//=====- Sw64FrameLowering.cpp - Sw64 Frame Information ------*- C++ -*-====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sw64 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//
#include "Sw64FrameLowering.h"
#include "MCTargetDesc/Sw64BaseInfo.h"
#include "Sw64.h"
#include "Sw64InstrInfo.h"
#include "Sw64MachineFunctionInfo.h"
#include "Sw64Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm> // std::sort

using namespace llvm;

cl::opt<bool> Sw64PG("pg", cl::desc("Support the pg"), cl::init(false));

static long getUpper16(long l) {
  long y = l / Sw64::IMM_MULT;
  if (l % Sw64::IMM_MULT > Sw64::IMM_HIGH)
    ++y;
  else if (l % Sw64::IMM_MULT < Sw64::IMM_LOW)
    --y;
  return y;
}

static long getLower16(long l) {
  long h = getUpper16(l);
  return l - h * Sw64::IMM_MULT;
}

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
//
bool Sw64FrameLowering::hasFP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();

  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken() ||
         TRI->hasStackRealignment(MF);
}

// hasReservedCallFrame - Under normal circumstances, when a frame pointer is
// not required, we reserve argument space for call sites in the function
// immediately on entry to the current function.  This eliminates the need for
// add/sub sp brackets around call sites.  Returns true if the call frame is
// included as part of the stack frame.
bool Sw64FrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  return !MF.getFrameInfo().hasVarSizedObjects();
}

bool Sw64FrameLowering::isLeafProc(MachineFunction &MF) const {
  MachineRegisterInfo &MRI = MF.getRegInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  return !MRI.isPhysRegUsed(Sw64::R29);
}

bool Sw64FrameLowering::hasBP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();

  return MFI.hasVarSizedObjects() && TRI->hasStackRealignment(MF);
}

void Sw64FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");

  MachineBasicBlock::iterator MBBI = MBB.begin(); // Prolog goes in entry BB
  MachineFrameInfo &MFI = MF.getFrameInfo();

  const Sw64InstrInfo &TII = *MF.getSubtarget<Sw64Subtarget>().getInstrInfo();
  const Sw64RegisterInfo &RegInfo = *static_cast<const Sw64RegisterInfo *>(
      MF.getSubtarget<Sw64Subtarget>().getRegisterInfo());
  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc dl;

  // First, compute final stack size.
  uint64_t StackSize = MFI.getStackSize();

  MachineModuleInfo &MMI = MF.getMMI();
  const MCRegisterInfo *MRI = MMI.getContext().getRegisterInfo();

  MBB.addLiveIn(Sw64::R27);
  int curgpdist = STI.getCurgpdist();
  // Handle GOT offset
  // Now sw_64 won't emit this unless it is necessary.
  // While it is also useful for DebugInfo test.
  if (!isLeafProc(MF)) {
    BuildMI(MBB, MBBI, dl, TII.get(Sw64::MOVProgPCGp))
        .addGlobalAddress(&(MF.getFunction()))
        .addImm(++curgpdist)
        .addReg(Sw64::R27);

    BuildMI(MBB, MBBI, dl, TII.get(Sw64::ALTENT))
        .addGlobalAddress(&(MF.getFunction()));
  }

  // No need to allocate space on the stack.
  if (StackSize == 0 && !MFI.adjustsStack())
    return;

  if (Sw64Mieee) {
    if (!Sw64DeleteNop)
      BuildMI(MBB, MBBI, dl, TII.get(Sw64::NOP));
  }
  if (Sw64PG) {
    BuildMI(MBB, MBBI, dl, TII.get(Sw64::LDL), Sw64::R28)
        .addExternalSymbol("_mcount")
        .addReg(Sw64::R29);
    if (Sw64Mieee) {
      if (!Sw64DeleteNop)
        BuildMI(MBB, MBBI, dl, TII.get(Sw64::NOP));
      BuildMI(MBB, MBBI, dl, TII.get(Sw64::JSR))
          .addReg(Sw64::R28)
          .addReg(Sw64::R28)
          .addExternalSymbol("_mcount");
      if (!Sw64DeleteNop)
        BuildMI(MBB, MBBI, dl, TII.get(Sw64::NOP));
    } else
      BuildMI(MBB, MBBI, dl, TII.get(Sw64::JSR))
          .addReg(Sw64::R28)
          .addReg(Sw64::R28)
          .addExternalSymbol("_mcount");
  }

  unsigned Align = getStackAlignment();
  StackSize = (StackSize + Align - 1) / Align * Align;

  // Update frame info to pretend that this is part of the stack...
  MFI.setStackSize(StackSize);

  // adjust stack pointer: r30 -= numbytes
  int AdjustStackSize = -StackSize;
  if (AdjustStackSize >= Sw64::IMM_LOW) {
    BuildMI(MBB, MBBI, dl, TII.get(Sw64::LDA), Sw64::R30)
        .addImm(AdjustStackSize)
        .addReg(Sw64::R30);
  } else if (getUpper16(AdjustStackSize) >= Sw64::IMM_LOW) {
    BuildMI(MBB, MBBI, dl, TII.get(Sw64::LDAH), Sw64::R30)
        .addImm(getUpper16(AdjustStackSize))
        .addReg(Sw64::R30);
    BuildMI(MBB, MBBI, dl, TII.get(Sw64::LDA), Sw64::R30)
        .addImm(getLower16(AdjustStackSize))
        .addReg(Sw64::R30);
  } else {
    report_fatal_error("Too big a stack frame at " + Twine(-AdjustStackSize));
  }

  // emit ".cfi_def_cfa_offset StackSize"
  unsigned CFIIndex = MF.addFrameInst(
      MCCFIInstruction::cfiDefCfaOffset(nullptr, -AdjustStackSize));
  BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex);

  std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();

  if (!CSI.empty()) {
    // Find the instruction past the last instruction that saves a
    // callee-saved register to the stack.
    for (unsigned i = 0; i < CSI.size(); ++i)
      ++MBBI;

    // Iterate over list of callee-saved registers and emit .cfi_offset
    // directives.
    for (std::vector<CalleeSavedInfo>::const_iterator I = CSI.begin(),
                                                      E = CSI.end();
         I != E; ++I) {
      int64_t Offset = MFI.getObjectOffset(I->getFrameIdx());
      unsigned Reg = I->getReg();
      unsigned DReg = MRI->getDwarfRegNum(Reg, true);
      unsigned CFIIndex = MF.addFrameInst(
          MCCFIInstruction::createOffset(nullptr, DReg, Offset));

      BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
          .addCFIIndex(CFIIndex);
    }
  }

  // if framepointer enabled, set it to point to the stack pointer.
  // Now if we need to, save the old FP and set the new
  if (hasFP(MF)) {
    // This must be the last instr in the prolog
    BuildMI(MBB, MBBI, dl, TII.get(Sw64::BISr), Sw64::R15)
        .addReg(Sw64::R30)
        .addReg(Sw64::R30);

    // emit ".cfi_def_cfa_register $fp"
    unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createDefCfaRegister(
        nullptr, MRI->getDwarfRegNum(Sw64::R15, true)));
    BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex);

    if (RegInfo.hasStackRealignment(MF)) {
      // ldi -MaxAlign
      // and -MaxAlign for sp
      Register VR = MF.getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);

      assert((Log2(MFI.getMaxAlign()) < 16) &&
             "Function's alignment size requirement is not supported.");
      int64_t MaxAlign = -(int64_t)MFI.getMaxAlign().value();
      BuildMI(MBB, MBBI, dl, TII.get(Sw64::LDA), VR)
          .addImm(MaxAlign)
          .addReg(Sw64::R31);
      BuildMI(MBB, MBBI, dl, TII.get(Sw64::ANDr), Sw64::R30)
          .addReg(Sw64::R30)
          .addReg(VR);

      if (hasBP(MF))
        // mov $sp, $14
        BuildMI(MBB, MBBI, dl, TII.get(Sw64::BISr), Sw64::R14)
            .addReg(Sw64::R30)
            .addReg(Sw64::R30);
    }
  }
}

void Sw64FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {

  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const Sw64InstrInfo &TII = *MF.getSubtarget<Sw64Subtarget>().getInstrInfo();
  DebugLoc dl = MBBI->getDebugLoc();

  assert((MBBI->getOpcode() == Sw64::PseudoRet) &&
         "Can only insert epilog into returning blocks");

  // Get the number of bytes allocated from the FrameInfo...
  uint64_t StackSize = MFI.getStackSize();
  // now if we need to, restore the old FP
  if (hasFP(MF)) {
    // Find the first instruction that restores a callee-saved register.
    MachineBasicBlock::iterator I = MBBI;
    for (unsigned i = 0; i < MFI.getCalleeSavedInfo().size(); ++i) {
      --I;
    }

    // copy the FP into the SP (discards allocas)
    BuildMI(MBB, I, dl, TII.get(Sw64::BISr), Sw64::R30)
        .addReg(Sw64::R15)
        .addReg(Sw64::R15);
  }

  if (StackSize != 0) {
    if (StackSize <= Sw64::IMM_HIGH) {
      BuildMI(MBB, MBBI, dl, TII.get(Sw64::LDA), Sw64::R30)
          .addImm(StackSize)
          .addReg(Sw64::R30);
    } else if (getUpper16(StackSize) <= Sw64::IMM_HIGH) {
      BuildMI(MBB, MBBI, dl, TII.get(Sw64::LDAH), Sw64::R30)
          .addImm(getUpper16(StackSize))
          .addReg(Sw64::R30);
      BuildMI(MBB, MBBI, dl, TII.get(Sw64::LDA), Sw64::R30)
          .addImm(getLower16(StackSize))
          .addReg(Sw64::R30);
    } else {
      report_fatal_error("Too big a stack frame at " + Twine(StackSize));
    }
  }
}

StackOffset
Sw64FrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                          Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  if (MFI.isFixedObjectIndex(FI))
    FrameReg = hasFP(MF) ? Sw64::R15 : Sw64::R30;
  else
    FrameReg = hasBP(MF) ? Sw64::R14 : Sw64::R30;

  return StackOffset::getFixed(MFI.getObjectOffset(FI) + MFI.getStackSize() -
                               getOffsetOfLocalArea() +
                               MFI.getOffsetAdjustment());
}

// TODO: must be rewrite.
bool Sw64FrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  const TargetInstrInfo &TII = *STI.getInstrInfo();

  DebugLoc DL;
  if (MI != MBB.end() && !MI->isDebugInstr())
    DL = MI->getDebugLoc();
  for (unsigned i = 0, e = CSI.size(); i != e; ++i) {
    unsigned Reg = CSI[i].getReg();
    MBB.addLiveIn(Reg);
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.storeRegToStackSlot(MBB, MI, Reg, true, CSI[i].getFrameIdx(), RC, TRI,
                            Register());
  }
  return true;
}

bool Sw64FrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    MutableArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  MachineFunction *MF = MBB.getParent();
  const TargetInstrInfo &TII = *MF->getSubtarget().getInstrInfo();
  bool AtStart = MI == MBB.begin();
  MachineBasicBlock::iterator BeforeI = MI;
  if (!AtStart)
    --BeforeI;
  for (unsigned i = 0, e = CSI.size(); i != e; ++i) {
    unsigned Reg = CSI[i].getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.loadRegFromStackSlot(MBB, MI, Reg, CSI[i].getFrameIdx(), RC, TRI,
                             Register());
    assert(MI != MBB.begin() && "loadRegFromStackSlot didn't insert any code!");
    // Insert in reverse order.  loadRegFromStackSlot can insert multiple
    // instructions.
    if (AtStart)
      MI = MBB.begin();
    else {
      MI = BeforeI;
      ++MI;
    }
  }
  return true;
}

// This function eliminates ADJCALLSTACKDOWN,
// ADJCALLSTACKUP pseudo instructions
MachineBasicBlock::iterator Sw64FrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {

  const Sw64InstrInfo &TII = *MF.getSubtarget<Sw64Subtarget>().getInstrInfo();

  if (!hasReservedCallFrame(MF)) {
    // Turn the adjcallstackdown instruction into 'ldi sp,-<amt>sp' and the
    // adjcallstackup instruction into 'ldi sp,<amt>sp'
    MachineInstr &Old = *I;
    // FIXME: temporary modify the old value is: Old.getOperand(0).getImm();
    uint64_t Amount = Old.getOperand(0).getImm();
    if (Amount != 0) {
      // We need to keep the stack aligned properly.  To do this, we round the
      // amount of space needed for the outgoing arguments up to the next
      // alignment boundary.
      unsigned Align = getStackAlignment();
      Amount = (Amount + Align - 1) / Align * Align;

      MachineInstr *New;
      if (Old.getOpcode() == Sw64::ADJUSTSTACKDOWN) {
        New = BuildMI(MF, Old.getDebugLoc(), TII.get(Sw64::LDA), Sw64::R30)
                  .addImm(-Amount)
                  .addReg(Sw64::R30);
      } else {
        assert(Old.getOpcode() == Sw64::ADJUSTSTACKUP);
        New = BuildMI(MF, Old.getDebugLoc(), TII.get(Sw64::LDA), Sw64::R30)
                  .addImm(Amount)
                  .addReg(Sw64::R30);
      }
      // Replace the pseudo instruction with a new instruction...
      MBB.insert(I, New);
    }
  }

  return MBB.erase(I);
}

/// Mark \p Reg and all registers aliasing it in the bitset.
static void setAliasRegs(MachineFunction &MF, BitVector &SavedRegs,
                         unsigned Reg) {
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  for (MCRegAliasIterator AI(Reg, TRI, true); AI.isValid(); ++AI)
    SavedRegs.set(*AI);
}

// TODO: must be rewrite.
void Sw64FrameLowering::determineCalleeSaves(MachineFunction &MF,
                                             BitVector &SavedRegs,
                                             RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  // Mark $fp as used if function has dedicated frame pointer.
  if (hasFP(MF))
    setAliasRegs(MF, SavedRegs, Sw64::R15);
  if (hasBP(MF))
    setAliasRegs(MF, SavedRegs, Sw64::R14);

  // Set scavenging frame index if necessary.
  uint64_t MaxSPOffset = estimateStackSize(MF);

  // If there is a variable sized object on the stack, the estimation cannot
  // account for it.
  if (isIntN(16, MaxSPOffset) && !MF.getFrameInfo().hasVarSizedObjects())
    return;
}

// Estimate the size of the stack, including the incoming arguments. We need to
// account for register spills, local objects, reserved call frame and incoming
// arguments. This is required to determine the largest possible positive offset
// from $sp so that it can be determined if an emergency spill slot for stack
// addresses is required.
uint64_t Sw64FrameLowering::estimateStackSize(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo &TRI = *STI.getRegisterInfo();

  int64_t Size = 0;

  // Iterate over fixed sized objects which are incoming arguments.
  for (int I = MFI.getObjectIndexBegin(); I != 0; ++I)
    if (MFI.getObjectOffset(I) > 0)
      Size += MFI.getObjectSize(I);

  // Conservatively assume all callee-saved registers will be saved.
  for (const MCPhysReg *R = TRI.getCalleeSavedRegs(&MF); *R; ++R) {
    unsigned RegSize = TRI.getSpillSize(*TRI.getMinimalPhysRegClass(*R));
    Size = alignTo(Size + RegSize, RegSize);
  }

  // Get the size of the rest of the frame objects and any possible reserved
  // call frame, accounting for alignment.
  return Size + MFI.estimateStackSize(MF);
}

void Sw64FrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {
  const Sw64RegisterInfo *RegInfo =
      MF.getSubtarget<Sw64Subtarget>().getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterClass *RC = &Sw64::GPRCRegClass;
  if (!isInt<16>(MFI.estimateStackSize(MF))) {
    int RegScavFI = MFI.CreateStackObject(RegInfo->getSpillSize(*RC),
                                          RegInfo->getSpillAlign(*RC), false);
    RS->addScavengingFrameIndex(RegScavFI);
  }
  assert(RS && "requiresRegisterScavenging failed");
}
