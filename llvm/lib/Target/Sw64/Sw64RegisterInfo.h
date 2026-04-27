//===-- Sw64RegisterInfo.h - Sw64 Register Information Impl ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sw64 implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_SW64_SW64REGISTERINFO_H
#define LLVM_LIB_TARGET_SW64_SW64REGISTERINFO_H

#include "Sw64.h"
#include "llvm/CodeGen/MachineBasicBlock.h"

#define GET_REGINFO_HEADER
#include "Sw64GenRegisterInfo.inc"

namespace llvm {

class TargetInstrInfo;
class TargetRegisterClass;

class Sw64RegisterInfo : public Sw64GenRegisterInfo {
public:
  Sw64RegisterInfo();
  enum class Sw64PtrClass {
    // The default register class for integer values.
    Default = 0,
    // The stack pointer only.
    StackPointer = 1,
    // The global pointer only.
    GlobalPointer = 2,
  };

  // Code Generation virtual methods...

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  // Eliminate virtual register which Prologue/Epilogue generate.
  bool requiresRegisterScavenging(const MachineFunction &MF) const override;
  bool trackLivenessAfterRegAlloc(const MachineFunction &MF) const override;
  bool useFPForScavengingIndex(const MachineFunction &MF) const override;
  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override;

  // Code Generation virtual methods...
  const TargetRegisterClass *getPointerRegClass(const MachineFunction &MF,
                                                unsigned Kind) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  // Debug information queries.
  Register getFrameRegister(const MachineFunction &MF) const override;

  const u_int32_t *getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID) const override;

  // Return whether to emit frame moves
  static bool needsFrameMoves(const MachineFunction &MF);
  // Exception handling queries.
  unsigned getEHExceptionRegister() const;
  unsigned getEHHandlerRegister() const;

  static std::string getPrettyName(unsigned reg);

private:
  void eliminateFI(MachineBasicBlock::iterator II, unsigned OpNo,
                   int FrameIndex, uint64_t StackSize, int64_t SPOffset) const;
};

} // end namespace llvm
#endif
