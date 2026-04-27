//===-- Sw64TargetMachine.h - Define TargetMachine for Sw64 ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Sw64 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_SW64_SW64TARGETMACHINE_H
#define LLVM_LIB_TARGET_SW64_SW64TARGETMACHINE_H

#include "MCTargetDesc/Sw64ABIInfo.h"
#include "Sw64Subtarget.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetMachine.h"
#include <memory>
#include <optional>

namespace llvm {

class Sw64TargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  Sw64ABIInfo ABI;
  Sw64Subtarget Subtarget;

public:
  Sw64TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOpt::Level OL,
                    bool JIT);
  ~Sw64TargetMachine() override;

  const Sw64ABIInfo &getABI() const { return ABI; }
  const Sw64Subtarget *getSubtargetImpl() const { return &Subtarget; }
  const Sw64Subtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // end namespace llvm
#endif
