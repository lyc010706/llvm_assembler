//=== lib/CodeGen/GlobalISel/Sw64PreLegalizerCombiner.cpp --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass does combining of machine instructions at the generic MI level,
// before the legalizer.
//
//===----------------------------------------------------------------------===//

#include "Sw64TargetMachine.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "sw_64-prelegalizer-combiner"

using namespace llvm;

namespace {
class Sw64PreLegalizerCombinerInfo : public CombinerInfo {
public:
  Sw64PreLegalizerCombinerInfo()
      : CombinerInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, /*EnableOpt*/ false,
                     /*EnableOptSize*/ false, /*EnableMinSize*/ false) {}

  virtual bool combine(GISelChangeObserver &Observer, MachineInstr &MI,
                       MachineIRBuilder &B) const override;
};

bool Sw64PreLegalizerCombinerInfo::combine(GISelChangeObserver &Observer,
                                           MachineInstr &MI,
                                           MachineIRBuilder &B) const {
  return false;
}

// Pass boilerplate
// ================

class Sw64PreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  Sw64PreLegalizerCombiner();

  StringRef getPassName() const override { return "Sw64PreLegalizerCombiner"; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // end anonymous namespace

void Sw64PreLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  MachineFunctionPass::getAnalysisUsage(AU);
}

Sw64PreLegalizerCombiner::Sw64PreLegalizerCombiner() : MachineFunctionPass(ID) {
  initializeSw64PreLegalizerCombinerPass(*PassRegistry::getPassRegistry());
}

bool Sw64PreLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel))
    return false;
  auto *TPC = &getAnalysis<TargetPassConfig>();
  Sw64PreLegalizerCombinerInfo PCInfo;
  Combiner C(PCInfo, TPC);
  return C.combineMachineInstrs(MF, nullptr);
}

char Sw64PreLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(Sw64PreLegalizerCombiner, DEBUG_TYPE,
                      "Combine Sw64 machine instrs before legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_END(Sw64PreLegalizerCombiner, DEBUG_TYPE,
                    "Combine Sw64 machine instrs before legalization", false,
                    false)

namespace llvm {
FunctionPass *createSw64PreLegalizeCombiner() {
  return new Sw64PreLegalizerCombiner();
}
} // end namespace llvm
