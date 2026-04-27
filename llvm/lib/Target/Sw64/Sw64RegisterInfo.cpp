//===-- Sw64RegisterInfo.cpp - Sw64 Register Information ----------------===//
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

#include "Sw64RegisterInfo.h"
#include "MCTargetDesc/Sw64ABIInfo.h"
#include "Sw64.h"
#include "Sw64InstrInfo.h"
#include "Sw64MachineFunctionInfo.h"
#include "Sw64Subtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

#define DEBUG_TYPE "sw_64-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "Sw64GenRegisterInfo.inc"

static cl::opt<bool> EnableOptReg("enable-sw64-opt-reg",
                                  cl::desc("Enalbe R15/R28 reg alloc on SW64"),
                                  cl::init(true), cl::Hidden);

Sw64RegisterInfo::Sw64RegisterInfo() : Sw64GenRegisterInfo(Sw64::R26) {}

// helper functions
static long getUpper16(long l) {
  long y = l / Sw64::IMM_MULT;
  if (l % Sw64::IMM_MULT > Sw64::IMM_HIGH)
    ++y;
  return y;
}

static long getLower16(long l) {
  long h = getUpper16(l);
  return l - h * Sw64::IMM_MULT;
}

const uint16_t *
Sw64RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {

  return CSR_F64_SaveList;
}

BitVector Sw64RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const Sw64FrameLowering *TFI = getFrameLowering(MF);
  if (EnableOptReg) {
    if (TFI->hasFP(MF))
      Reserved.set(Sw64::R15);
  } else {
    Reserved.set(Sw64::R15);
    Reserved.set(Sw64::R28);
  }
  Reserved.set(Sw64::R29);
  Reserved.set(Sw64::R30);
  Reserved.set(Sw64::R31);
  Reserved.set(Sw64::F31);
  Reserved.set(Sw64::V31);
  for (size_t i = 0; i < Sw64::GPRCRegClass.getNumRegs(); ++i) {
    if (MF.getSubtarget<Sw64Subtarget>().isRegisterReserved(i)) {
      StringRef RegName("$" + std::to_string(i));
      Reserved.set(
          MF.getSubtarget<Sw64Subtarget>().getTargetLowering()->MatchRegName(
              RegName));
    }
  }

  // hasBP
  if (hasStackRealignment(MF) && MF.getFrameInfo().hasVarSizedObjects())
    Reserved.set(Sw64::R14);

  return Reserved;
}

const u_int32_t *
Sw64RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const {
  return CSR_F64_RegMask;
}

const TargetRegisterClass *
Sw64RegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                     unsigned Kind) const {
  Sw64PtrClass PtrClassKind = static_cast<Sw64PtrClass>(Kind);

  switch (PtrClassKind) {
  case Sw64PtrClass::Default:
    return &Sw64::GPRCRegClass;
  case Sw64PtrClass::StackPointer:
    return &Sw64::SP64RegClass;
  case Sw64PtrClass::GlobalPointer:
    return &Sw64::GP64RegClass;
  }

  llvm_unreachable("Unknown pointer kind");
}

bool Sw64RegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return true;
}
bool Sw64RegisterInfo::requiresFrameIndexScavenging(
    const MachineFunction &MF) const {
  return true;
}
bool Sw64RegisterInfo::trackLivenessAfterRegAlloc(
    const MachineFunction &MF) const {
  return true;
}

bool Sw64RegisterInfo::useFPForScavengingIndex(
    const MachineFunction &MF) const {
  return false;
}

void Sw64RegisterInfo::eliminateFI(MachineBasicBlock::iterator II,
                                   unsigned OpNo, int FrameIndex,
                                   uint64_t StackSize, int64_t SPOffset) const {
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  const Sw64InstrInfo &TII =
      *static_cast<const Sw64InstrInfo *>(MF.getSubtarget().getInstrInfo());
  const Sw64RegisterInfo *RegInfo = static_cast<const Sw64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());

  unsigned i = OpNo;
  int MinCSFI = 0;
  int MaxCSFI = -1;

  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();
  if (CSI.size()) {
    MinCSFI = CSI[0].getFrameIdx();
    MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
  }

  // The following stack frame objects are always referenced relative to $sp:
  //  1. Outgoing arguments.
  //  2. Pointer to dynamically allocated stack space.
  //  3. Locations for callee-saved registers.
  // Everything else is referenced relative to whatever register
  // getFrameRegister() returns.
  unsigned FrameReg;

  if (FrameIndex >= MinCSFI && FrameIndex <= MaxCSFI)
    FrameReg = Sw64::R30;
  else if (RegInfo->hasStackRealignment(MF)) {
    if (MFI.hasVarSizedObjects() && !MFI.isFixedObjectIndex(FrameIndex))
      FrameReg = Sw64::R14;
    else if (MFI.isFixedObjectIndex(FrameIndex))
      FrameReg = getFrameRegister(MF);
    else
      FrameReg = Sw64::R30;
  } else
    FrameReg = getFrameRegister(MF);

  // Calculate final offset.
  // - There is no need to change the offset if the frame object is one of the
  //   following: an outgoing argument, pointer to a dynamically allocated
  //   stack space or a $gp restore location,
  // - If the frame object is any of the following, its offset must be adjusted
  //   by adding the size of the stack:
  //   incoming argument, callee-saved register location or local variable.
  int64_t Offset = SPOffset + (int64_t)StackSize;
  const MCInstrDesc &MCID = TII.get(MI.getOpcode());
  if (MI.getNumOperands() > 2 && MI.getOperand(2).isImm()) {
    if (MCID.mayLoad() || MCID.mayStore())
      Offset += MI.getOperand(2).getImm();
  }

  if (MI.getOperand(1).isImm())
    Offset += MI.getOperand(1).getImm();

  if (MI.isDebugValue())
    MI.getOperand(i + 1).ChangeToRegister(FrameReg, false);
  else
    MI.getOperand(2).ChangeToRegister(FrameReg, false);

  LLVM_DEBUG(errs() << "Offset     : " << Offset << "\n"
                    << "<--------->\n");

  // Now add the frame object offset to the offset from the virtual frame index.
  if (Offset > Sw64::IMM_HIGH || Offset < Sw64::IMM_LOW) {
    LLVM_DEBUG(errs() << "Unconditionally using R28 for evil purposes Offset: "
                      << Offset << "\n");
    // so in this case, we need to use a temporary register, and move the
    // original inst off the SP/FP
    // fix up the old:
    MachineInstr *nMI;
    bool FrameRegIsKilled = false;
    // insert the new
    Register vreg = MF.getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
    if (MI.getOperand(1).getTargetFlags() == 15) {
      nMI = BuildMI(MF, MI.getDebugLoc(), TII.get(Sw64::LDAH), vreg)
                .addImm(getUpper16(Offset))
                .addReg(FrameReg);
      FrameRegIsKilled = true;
    } else {
      nMI = BuildMI(MF, MI.getDebugLoc(), TII.get(Sw64::LDAH), vreg)
                .addImm(getUpper16(Offset))
                .addReg(FrameReg);
      FrameRegIsKilled = true;
    }

    MBB.insert(II, nMI);
    MI.getOperand(2).ChangeToRegister(vreg, false, false, FrameRegIsKilled);
    MI.getOperand(1).ChangeToImmediate(getLower16(Offset));
  } else {
    if (MI.isDebugValue())
      MI.getOperand(i + 1).ChangeToImmediate(Offset);
    else
      MI.getOperand(1).ChangeToImmediate(Offset);
  }
}

// FrameIndex represent objects inside a abstract stack.
// We must replace FrameIndex with an stack/frame pointer
// direct reference.
bool Sw64RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();

  LLVM_DEBUG(errs() << "\nFunction : " << MF.getName() << "\n";
             errs() << "<--------->\n"
                    << MI);

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  uint64_t stackSize = MF.getFrameInfo().getStackSize();
  int64_t spOffset = MF.getFrameInfo().getObjectOffset(FrameIndex);

  LLVM_DEBUG(errs() << "FrameIndex : " << FrameIndex << "\n"
                    << "spOffset   : " << spOffset << "\n"
                    << "stackSize  : " << stackSize << "\n"
                    << "alignment  : "
                    << DebugStr(MF.getFrameInfo().getObjectAlign(FrameIndex))
                    << "\n");

  eliminateFI(MI, FIOperandNum, FrameIndex, stackSize, spOffset);
  return false;
}

Register Sw64RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const Sw64FrameLowering *TFI = getFrameLowering(MF);

  return TFI->hasFP(MF) ? Sw64::R15 : Sw64::R30;
}

unsigned Sw64RegisterInfo::getEHExceptionRegister() const {
  llvm_unreachable("What is the exception register");
  return 0;
}

unsigned Sw64RegisterInfo::getEHHandlerRegister() const {
  llvm_unreachable("What is the exception handler register");
  return 0;
}

std::string Sw64RegisterInfo::getPrettyName(unsigned reg) {
  std::string s("#reg_#-#");
  return s;
}

bool Sw64RegisterInfo::needsFrameMoves(const MachineFunction &MF) {
  return MF.getMMI().hasDebugInfo() || MF.getFunction().needsUnwindTableEntry();
}
