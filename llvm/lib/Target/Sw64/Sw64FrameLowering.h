//===-- Sw64FrameLowering.h - Frame info for Sw64 Target ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains Sw64 frame information that doesn't fit anywhere else
// cleanly...
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_SW64_SW64FRAMELOWERING_H
#define LLVM_LIB_TARGET_SW64_SW64FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class Sw64Subtarget;

class Sw64FrameLowering : public TargetFrameLowering {

protected:
  const Sw64Subtarget &STI;

public:
  explicit Sw64FrameLowering(const Sw64Subtarget &sti)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(32), 0),
        STI(sti) {
    // Do nothing
  }

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool hasFP(const MachineFunction &MF) const override;
  bool hasReservedCallFrame(const MachineFunction &MF) const override;

  bool hasBP(const MachineFunction &MF) const;

private:
  void emitMieee(MachineFunction &MF) const;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS = nullptr) const override;

  void processFunctionBeforeFrameFinalized(
      MachineFunction &MF, RegScavenger *RS = nullptr) const override;

  //! Stack slot size (4 bytes)
  static int stackSlotSize() { return 4; }

  // Returns true if MF is a leaf procedure.
  bool isLeafProc(MachineFunction &MF) const;

protected:
  uint64_t estimateStackSize(const MachineFunction &MF) const;
};
} // namespace llvm
#endif
