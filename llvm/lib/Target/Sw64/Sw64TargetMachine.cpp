//===-- Sw64TargetMachine.cpp - Define TargetMachine for Sw64 -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "Sw64TargetMachine.h"
#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "Sw64.h"
#include "Sw64MachineFunctionInfo.h"
#include "Sw64MacroFusion.h"
#include "Sw64TargetObjectFile.h"
#include "Sw64TargetTransformInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Transforms/Scalar.h"
#include <optional>

using namespace llvm;

static cl::opt<bool> EnableMCR("sw_64-enable-mcr",
                               cl::desc("Enable the machine combiner pass"),
                               cl::init(true), cl::Hidden);

static cl::opt<bool>
    EnablePrefetch("enable-sw64-prefetching",
                   cl::desc("Enable software prefetching on SW64"),
                   cl::init(true), cl::Hidden);

cl::opt<bool> FS_LOAD("fastload",
                      cl::desc("Enable fast/load optimize(developing)"),
                      cl::init(false), cl::Hidden);

static Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                           std::optional<Reloc::Model> RM) {
  if (!RM)
    return Reloc::Static;
  return *RM;
}

static CodeModel::Model
getEffectiveSw64CodeModel(std::optional<CodeModel::Model> CM) {
  if (CM) {
    if (*CM != CodeModel::Small && *CM != CodeModel::Medium &&
        *CM != CodeModel::Large)
      report_fatal_error(
          "Target only supports CodeModel Small, Medium or Large");
    return *CM;
  }
  return CodeModel::Small;
}

// Create an ILP32 architecture model
Sw64TargetMachine::Sw64TargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(
          T,
          "e-m:e-p:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n64-S128-v256:256",
          TT, CPU, FS, Options, getEffectiveRelocModel(TT, RM),
          getEffectiveSw64CodeModel(CM), OL),
      TLOF(std::make_unique<Sw64TargetObjectFile>()),
      ABI(Sw64ABIInfo::computeTargetABI(TT, CPU, Options.MCOptions)),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

Sw64TargetMachine::~Sw64TargetMachine() = default;

namespace {

// Sw64 Code Generator Pass Configuration Options.
class Sw64PassConfig : public TargetPassConfig {
public:
  Sw64PassConfig(Sw64TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {
    if (TM.getOptLevel() != CodeGenOpt::None)
      substitutePass(&PostRASchedulerID, &PostMachineSchedulerID);
  }

  Sw64TargetMachine &getSw64TargetMachine() const {
    return getTM<Sw64TargetMachine>();
  }
  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override {
    ScheduleDAGMILive *DAG = createGenericSchedLive(C);
    DAG->addMutation(createSw64MacroFusionDAGMutation());
    return DAG;
  }

  ScheduleDAGInstrs *
  createPostMachineScheduler(MachineSchedContext *C) const override {
    ScheduleDAGMI *DAG = createGenericSchedPostRA(C);
    DAG->addMutation(createSw64MacroFusionDAGMutation());
    return DAG;
  }

  void addIRPasses() override;
  bool addILPOpts() override;
  bool addInstSelector() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
  void addPreRegAlloc() override;
  void addPreLegalizeMachineIR() override;
  // for Inst Selector.
  bool addGlobalInstructionSelect() override;
};

} // end anonymous namespace

TargetPassConfig *Sw64TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new Sw64PassConfig(*this, PM);
}

void Sw64PassConfig::addIRPasses() {
  addPass(createAtomicExpandPass());

  if (EnablePrefetch)
    addPass(createLoopDataPrefetchPass());

  TargetPassConfig::addIRPasses();
}

void Sw64PassConfig::addPreLegalizeMachineIR() {
  addPass(createSw64PreLegalizeCombiner());
}

void Sw64PassConfig::addPreSched2() { addPass(createSw64ExpandPseudo2Pass()); }

bool Sw64PassConfig::addInstSelector() {
  addPass(createSw64ISelDag(getSw64TargetMachine(), getOptLevel()));
  return false;
}

void Sw64PassConfig::addPreRegAlloc() {
  addPass(createSw64IEEEConstraintPass());
}

void Sw64PassConfig::addPreEmitPass() {
  addPass(createSw64BranchSelection());
  addPass(createSw64LLRPPass(getSw64TargetMachine()));
  addPass(createSw64ExpandPseudoPass());
}

bool Sw64PassConfig::addILPOpts() {

  if (EnableMCR)
    addPass(&MachineCombinerID);

  return true;
}

bool Sw64PassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect());
  return false;
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSw64Target() {
  RegisterTargetMachine<Sw64TargetMachine> X(getTheSw64Target());

  PassRegistry *PR = PassRegistry::getPassRegistry();
  initializeSw64BranchSelectionPass(*PR);
  initializeSw64PreLegalizerCombinerPass(*PR);
  initializeSw64DAGToDAGISelPass(*PR);
}

TargetTransformInfo
Sw64TargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(Sw64TTIImpl(this, F));
}

MachineFunctionInfo *Sw64TargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return Sw64MachineFunctionInfo::create<Sw64MachineFunctionInfo>(Allocator, F,
                                                                  STI);
}
