//===-- Sw64InstrInfo.cpp - Sw64 Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sw64 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "Sw64InstrInfo.h"
#include "Sw64.h"
#include "Sw64MachineFunctionInfo.h"
#include "Sw64OptionRecord.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineCombinerPattern.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/StackMaps.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "Sw64combinefma"

#define GET_INSTRINFO_CTOR_DTOR
#include "Sw64GenInstrInfo.inc"

// Pin the vtable to this file.
void Sw64InstrInfo::anchor() {}

Sw64InstrInfo::Sw64InstrInfo()
    : Sw64GenInstrInfo(Sw64::ADJUSTSTACKDOWN, Sw64::ADJUSTSTACKUP), RI() {}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned Sw64InstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                            int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case Sw64::LDL:
  case Sw64::LDW:
  case Sw64::LDHU:
  case Sw64::LDBU:
  case Sw64::LDS:
  case Sw64::LDD:
    if (MI.getOperand(1).isFI()) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  }

  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned Sw64InstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case Sw64::STL:
  case Sw64::STH:
  case Sw64::STB:
  case Sw64::STW:
  case Sw64::STS:
  case Sw64::STD:
    if (MI.getOperand(1).isFI()) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  }
  return 0;
}

unsigned Sw64InstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 2 || Cond.size() == 0) &&
         "Sw64 branch conditions have two components!");

  // Unconditional branch.
  if (Cond.empty()) {
    MachineInstr &MI = *BuildMI(&MBB, DL, get(Sw64::PseudoBR)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(MI);
    return 1;
  }

  // Either a one or two-way conditional branch.
  unsigned Opc = Cond[0].getImm();
  MachineInstr &CondMI = *BuildMI(&MBB, DL, get(Opc)).add(Cond[1]).addMBB(TBB);
  if (BytesAdded)
    *BytesAdded += getInstSizeInBytes(CondMI);

  // One-way conditional branch.
  if (!FBB)
    return 1;

  // Two-way conditional branch.
  MachineInstr &MI = *BuildMI(&MBB, DL, get(Sw64::PseudoBR)).addMBB(FBB);
  if (BytesAdded)
    *BytesAdded += getInstSizeInBytes(MI);
  return 2;
}

void Sw64InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, MCRegister DestReg,
                                MCRegister SrcReg, bool KillSrc) const {
  if ((Sw64::F4RCRegClass.contains(DestReg) ||
       Sw64::FPRC_loRegClass.contains(DestReg)) && // for rust and SIMD
      Sw64::GPRCRegClass.contains(SrcReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::ITOFS), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::F4RCRegClass.contains(SrcReg) && // for rust and SIMD
             Sw64::GPRCRegClass.contains(DestReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::FTOIS), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::FPRCRegClass.contains(SrcReg) && // for rust and SIMD
             Sw64::GPRCRegClass.contains(DestReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::FTOIT), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::FPRCRegClass.contains(DestReg) && // for rust and SIMD
             Sw64::GPRCRegClass.contains(SrcReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::ITOFT), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::FPRCRegClass.contains(DestReg) && // for rust and SIMD
             Sw64::FPRC_loRegClass.contains(SrcReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::CPYSD), DestReg)
        .addReg(SrcReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::FPRCRegClass.contains(SrcReg) && // for rust and SIMD
             Sw64::FPRC_loRegClass.contains(DestReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::CPYSD), DestReg)
        .addReg(SrcReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::GPRCRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::BISr), DestReg)
        .addReg(SrcReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::F4RCRegClass.contains(DestReg, SrcReg)) {
    unsigned int RC = MI->getOperand(1).getReg();
    unsigned int Opc = Sw64::CPYSS;
    for (MachineBasicBlock::iterator MBBI = MI; MBBI != MBB.begin(); --MBBI) {
      if (MBBI->getOpcode() == Sw64::VLDS || MBBI->getOpcode() == Sw64::VLDD) {
        unsigned int RD = MBBI->getOperand(0).getReg();
        if (RC == RD)
          Opc = Sw64::VCPYS;
        break;
      }
    }
    BuildMI(MBB, MI, DL, get(Opc), DestReg)
        .addReg(SrcReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::F8RCRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::CPYSD), DestReg)
        .addReg(SrcReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::FPRCRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::CPYSD), DestReg)
        .addReg(SrcReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Sw64::V256LRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Sw64::VOR), DestReg)
        .addReg(SrcReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else {
    llvm_unreachable("Attempt to copy register that is not GPR or FPR");
  }
}

void Sw64InstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool isKill, int FrameIdx, const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI, Register VReg) const {

  DebugLoc DL;
  if (MI != MBB.end())
    DL = MI->getDebugLoc();

  unsigned Opc = 0;

  if (RC == &Sw64::F4RCRegClass)
    Opc = Sw64::STS;
  else if (RC == &Sw64::F8RCRegClass)
    Opc = Sw64::STD;
  else if (RC == &Sw64::GPRCRegClass)
    Opc = Sw64::STL;
  else if (TRI->isTypeLegalForClass(*RC, MVT::i64) ||
           TRI->isTypeLegalForClass(*RC, MVT::f64))
    Opc = Sw64::STD;
  else if (TRI->isTypeLegalForClass(*RC, MVT::i32) ||
           TRI->isTypeLegalForClass(*RC, MVT::f32))
    Opc = Sw64::STS;
  else if (TRI->isTypeLegalForClass(*RC, MVT::v8i32))
    Opc = Sw64::VSTD;
  else if (TRI->isTypeLegalForClass(*RC, MVT::v4f32))
    Opc = Sw64::VSTS;
  else if (TRI->isTypeLegalForClass(*RC, MVT::v4i64))
    Opc = Sw64::VSTD;
  else if (TRI->isTypeLegalForClass(*RC, MVT::v4f64))
    Opc = Sw64::VSTD;
  else
    llvm_unreachable("Unhandled register class");

  BuildMI(MBB, MI, DL, get(Opc))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIdx)
      .addReg(Sw64::R31);
}

void Sw64InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DestReg, int FrameIdx,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI,
                                         Register VReg) const {
  DebugLoc DL;
  if (MI != MBB.end())
    DL = MI->getDebugLoc();

  unsigned Opc = 0;

  if (RC == &Sw64::F4RCRegClass)
    Opc = Sw64::LDS;
  else if (RC == &Sw64::F8RCRegClass)
    Opc = Sw64::LDD;
  else if (RC == &Sw64::GPRCRegClass)
    Opc = Sw64::LDL;
  else if (TRI->isTypeLegalForClass(*RC, MVT::i64) ||
           TRI->isTypeLegalForClass(*RC, MVT::f64))
    Opc = Sw64::LDD;
  else if (TRI->isTypeLegalForClass(*RC, MVT::i32) ||
           TRI->isTypeLegalForClass(*RC, MVT::f32))
    Opc = Sw64::LDS;
  else if (TRI->isTypeLegalForClass(*RC, MVT::v8i32))
    Opc = Sw64::VLDD;
  else if (TRI->isTypeLegalForClass(*RC, MVT::v4f32))
    Opc = Sw64::VLDS;
  else if (TRI->isTypeLegalForClass(*RC, MVT::v4i64))
    Opc = Sw64::VLDD;
  else if (TRI->isTypeLegalForClass(*RC, MVT::v4f64))
    Opc = Sw64::VLDD;
  else
    llvm_unreachable("Unhandled register class");

  BuildMI(MBB, MI, DL, get(Opc), DestReg)
      .addFrameIndex(FrameIdx)
      .addReg(Sw64::R31);
}

static unsigned Sw64RevCondCode(unsigned Opcode) {
  switch (Opcode) {
  case Sw64::BEQ:
    return Sw64::BNE;
  case Sw64::BNE:
    return Sw64::BEQ;
  case Sw64::BGE:
    return Sw64::BLT;
  case Sw64::BGT:
    return Sw64::BLE;
  case Sw64::BLE:
    return Sw64::BGT;
  case Sw64::BLT:
    return Sw64::BGE;
  case Sw64::BLBC:
    return Sw64::BLBS;
  case Sw64::BLBS:
    return Sw64::BLBC;
  case Sw64::FBEQ:
    return Sw64::FBNE;
  case Sw64::FBNE:
    return Sw64::FBEQ;
  case Sw64::FBGE:
    return Sw64::FBLT;
  case Sw64::FBGT:
    return Sw64::FBLE;
  case Sw64::FBLE:
    return Sw64::FBGT;
  case Sw64::FBLT:
    return Sw64::FBGE;
  default:
    llvm_unreachable("Unknown opcode");
  }
  return 0; // Not reached
}

//===----------------------------------------------------------------------===//
// Branch Analysis
//===----------------------------------------------------------------------===//
//

static bool isCondOpCode(unsigned Opcode) {
  switch (Opcode) {
  default:
    return false;
  case Sw64::BEQ:
  case Sw64::BNE:
  case Sw64::BGE:
  case Sw64::BGT:
  case Sw64::BLE:
  case Sw64::BLT:
  case Sw64::BLBC:
  case Sw64::BLBS:
  case Sw64::FBEQ:
  case Sw64::FBNE:
  case Sw64::FBGE:
  case Sw64::FBGT:
  case Sw64::FBLE:
  case Sw64::FBLT:
    return true;
  }
  return false; // Not reached
}

static bool isUnCondOpCode(unsigned Opcode) { return Opcode == Sw64::PseudoBR; }

static void parseCondBranch(MachineInstr *LastInst, MachineBasicBlock *&Target,
                            SmallVectorImpl<MachineOperand> &Cond) {

  Target = LastInst->getOperand(1).getMBB();
  Cond.push_back(MachineOperand::CreateImm(LastInst->getOpcode()));
  Cond.push_back(LastInst->getOperand(0));
}

bool Sw64InstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool AllowModify) const {
  // If the block has no terminators, it just falls into the block after it.
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return false;

  if (!isUnpredicatedTerminator(*I))
    return false;

  // Get the last instruction in the block.
  MachineInstr *LastInst = &*I;
  unsigned LastOpc = LastInst->getOpcode();
  // If there is only one terminator instruction, process it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
    if (isUnCondOpCode(LastOpc)) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    } else if (isCondOpCode(LastOpc)) {
      parseCondBranch(LastInst, TBB, Cond);
      return false;
    } // Otherwise, don't know what this is.
    return true;
  }

  // Get the instruction before it if it's a terminator.
  MachineInstr *SecondLastInst = &*I;
  unsigned SecondLastOpc = SecondLastInst->getOpcode();

  // If AllowModify is true and the block ends with two or more unconditional
  // branches, delete all but the first unconditional branch.
  if (AllowModify && isUnCondOpCode(LastOpc)) {
    while (isUnCondOpCode(SecondLastOpc)) {
      LastInst->eraseFromParent();
      LastInst = SecondLastInst;
      LastOpc = LastInst->getOpcode();
      if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
        TBB = LastInst->getOperand(0).getMBB();
        return false;
      } else {
        SecondLastInst = &*I;
        SecondLastOpc = SecondLastInst->getOpcode();
      }
    }
  }

  // If there are three terminators, we don't know what sort of block this is.
  if (SecondLastInst && I != MBB.begin() && isUnpredicatedTerminator(*--I))
    return true;

  if (isCondOpCode(SecondLastOpc) && isUnCondOpCode(LastOpc)) {
    parseCondBranch(SecondLastInst, TBB, Cond);
    FBB = LastInst->getOperand(0).getMBB();
    return false;
  }

  // If the block ends with two Sw64::BRs, handle it.  The second one is not
  // executed, so remove it.
  if (isUnCondOpCode(SecondLastOpc) && isUnCondOpCode(LastOpc)) {
    TBB = SecondLastInst->getOperand(0).getMBB();
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
    return false;
  }

  // Otherwise, can't handle this.
  return true;
}

unsigned Sw64InstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return false;

  if (I->getOpcode() != Sw64::PseudoBR && !isCondOpCode(I->getOpcode()))
    return 0;

  // Remove the branch.
  I->eraseFromParent();

  I = MBB.end();

  if (I == MBB.begin()) {
    if (BytesRemoved)
      *BytesRemoved = 4;
    return 1;
  }
  --I;
  if (!isCondOpCode(I->getOpcode())) {
    if (BytesRemoved)
      *BytesRemoved = 4;
    return 1;
  }

  // Remove the branch.
  I->eraseFromParent();
  if (BytesRemoved)
    *BytesRemoved = 8;
  return 2;
}

void Sw64InstrInfo::insertNoop(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MI) const {
  DebugLoc DL;
  BuildMI(MBB, MI, DL, get(Sw64::BISr), Sw64::R31)
      .addReg(Sw64::R31)
      .addReg(Sw64::R31);
}

bool Sw64InstrInfo::ReverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 2 && "Invalid Sw64 branch opcode!");
  Cond[0].setImm(Sw64RevCondCode(Cond[0].getImm()));
  return false;
}

/// getGlobalBaseReg - Return a virtual register initialized with the
/// the global base register value. Output instructions required to
/// initialize the register in the function entry block, if necessary.
///
unsigned Sw64InstrInfo::getGlobalBaseReg(MachineFunction *MF) const {
  Sw64MachineFunctionInfo *Sw64FI = MF->getInfo<Sw64MachineFunctionInfo>();
  unsigned GlobalBaseReg = Sw64FI->getGlobalBaseReg(*MF);
  if (GlobalBaseReg != 0)
    return GlobalBaseReg;

  // Insert the set of GlobalBaseReg into the first MBB of the function
  GlobalBaseReg = Sw64::R29;
  Sw64FI->setGlobalBaseReg(GlobalBaseReg);
  return GlobalBaseReg;
}

/// getGlobalRetAddr - Return a virtual register initialized with the
/// the global base register value. Output instructions required to
/// initialize the register in the function entry block, if necessary.
///
unsigned Sw64InstrInfo::getGlobalRetAddr(MachineFunction *MF) const {
  Sw64MachineFunctionInfo *Sw64FI = MF->getInfo<Sw64MachineFunctionInfo>();
  unsigned GlobalRetAddr = Sw64FI->getGlobalRetAddr(*MF);
  if (GlobalRetAddr != 0)
    return GlobalRetAddr;

  // Insert the set of GlobalRetAddr into the first MBB of the function
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  GlobalRetAddr = Sw64::R26;
  RegInfo.addLiveIn(Sw64::R26);
  Sw64FI->setGlobalRetAddr(GlobalRetAddr);
  return GlobalRetAddr;
}

MachineInstr *Sw64InstrInfo::emitFrameIndexDebugValue(MachineFunction &MF,
                                                      int FrameIx,
                                                      uint64_t Offset,
                                                      const MDNode *MDPtr,
                                                      DebugLoc DL) const {
  MachineInstrBuilder MIB = BuildMI(MF, DL, get(Sw64::DBG_VALUE))
                                .addFrameIndex(FrameIx)
                                .addImm(0)
                                .addImm(Offset)
                                .addMetadata(MDPtr);
  return &*MIB;
}

// for vector optimize.
// Utility routine that checks if \param MO is defined by an
// \param CombineOpc instruction in the basic block \param MBB
static bool canCombine(MachineBasicBlock &MBB, MachineOperand &MO,
                       unsigned CombineOpc) {
  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
  MachineInstr *MI = nullptr;

  if (MO.isReg() && Register::isVirtualRegister(MO.getReg()))
    MI = MRI.getUniqueVRegDef(MO.getReg());

  LLVM_DEBUG(dbgs() << "is MO reg?" << MO.isReg();
             dbgs() << "is Register Virtual?"
                    << Register::isVirtualRegister(MO.getReg()));

  // And it needs to be in the trace (otherwise, it won't have a depth).
  if (!MI || MI->getParent() != &MBB || (unsigned)MI->getOpcode() != CombineOpc)
    return false;

  // Must only used by the user we combine with.
  if (!MRI.hasOneNonDBGUse(MI->getOperand(0).getReg()))
    return false;

  return true;
}

//
// Is \param MO defined by a floating-point multiply and can be combined?
static bool canCombineWithFMUL(MachineBasicBlock &MBB, MachineOperand &MO,
                               unsigned MulOpc) {
  return canCombine(MBB, MO, MulOpc);
}

// TODO: There are many more machine instruction opcodes to match:
//       1. Other data types (integer, vectors)
//       2. Other math / logic operations (xor, or)
//       3. Other forms of the same operation (intrinsics and other variants)
bool Sw64InstrInfo::isAssociativeAndCommutative(const MachineInstr &Inst,
                                                bool Invert) const {
  if (Invert)
    return false;
  switch (Inst.getOpcode()) {
  case Sw64::ADDD:
  case Sw64::ADDS:
  case Sw64::MULD:
  case Sw64::MULS:
  case Sw64::VADDS:
  case Sw64::VADDD:
  case Sw64::VMULS:
  case Sw64::VMULD:
    return Inst.getParent()->getParent()->getTarget().Options.UnsafeFPMath;
  default:
    return false;
  }
}

// FP Opcodes that can be combined with a FMUL
static bool isCombineInstrCandidateFP(const MachineInstr &Inst) {
  switch (Inst.getOpcode()) {
  default:
    break;
  case Sw64::ADDS:
  case Sw64::ADDD:
  case Sw64::SUBS:
  case Sw64::SUBD: {
    TargetOptions Options = Inst.getParent()->getParent()->getTarget().Options;
    return (Options.UnsafeFPMath ||
            Options.AllowFPOpFusion == FPOpFusion::Fast);
  }
  case Sw64::VADDS:
  case Sw64::VADDD:
  case Sw64::VSUBS:
  case Sw64::VSUBD:
    return true;
  }
  return false;
}

/// Find instructions that can be turned into madd.
static bool getFMAPatterns(MachineInstr &Root,
                           SmallVectorImpl<MachineCombinerPattern> &Patterns) {

  if (!isCombineInstrCandidateFP(Root))
    return false;

  MachineBasicBlock &MBB = *Root.getParent();
  bool Found = false;

  switch (Root.getOpcode()) {
  default:
    assert(false && "Unsupported FP instruction in combiner\n");
    break;
  case Sw64::ADDS:
    assert(Root.getOperand(1).isReg() && Root.getOperand(2).isReg() &&
           "FADDS does not have register operands");
    if (canCombineWithFMUL(MBB, Root.getOperand(1), Sw64::MULS)) {
      Patterns.push_back(MachineCombinerPattern::FMULADDS_OP1);
      Found = true;
    }
    if (canCombineWithFMUL(MBB, Root.getOperand(2), Sw64::MULS)) {
      Patterns.push_back(MachineCombinerPattern::FMULADDS_OP2);
      Found = true;
    }
    break;

  case Sw64::ADDD:
    if (canCombineWithFMUL(MBB, Root.getOperand(1), Sw64::MULD)) {
      Patterns.push_back(MachineCombinerPattern::FMULADDD_OP1);
      Found = true;
    }
    if (canCombineWithFMUL(MBB, Root.getOperand(2), Sw64::MULD)) {
      Patterns.push_back(MachineCombinerPattern::FMULADDD_OP2);
      Found = true;
    }
    break;

  case Sw64::SUBS:
    if (canCombineWithFMUL(MBB, Root.getOperand(1), Sw64::MULS)) {
      Patterns.push_back(MachineCombinerPattern::FMULSUBS_OP1);
      Found = true;
    }
    if (canCombineWithFMUL(MBB, Root.getOperand(2), Sw64::MULS)) {
      Patterns.push_back(MachineCombinerPattern::FMULSUBS_OP2);
      Found = true;
    }
    break;

  case Sw64::SUBD:
    if (canCombineWithFMUL(MBB, Root.getOperand(1), Sw64::MULD)) {
      Patterns.push_back(MachineCombinerPattern::FMULSUBD_OP1);
      Found = true;
    }
    if (canCombineWithFMUL(MBB, Root.getOperand(2), Sw64::MULD)) {
      Patterns.push_back(MachineCombinerPattern::FMULSUBD_OP2);
      Found = true;
    }
    break;
  case Sw64::VADDS:
    assert(Root.getOperand(1).isReg() && Root.getOperand(2).isReg() &&
           "FADDS does not have register operands");
    if (canCombineWithFMUL(MBB, Root.getOperand(1), Sw64::VMULS)) {
      Patterns.push_back(MachineCombinerPattern::VMULADDS_OP1);
      Found = true;
    }
    if (canCombineWithFMUL(MBB, Root.getOperand(2), Sw64::VMULS)) {
      Patterns.push_back(MachineCombinerPattern::VMULADDS_OP2);
      Found = true;
    }
    break;

  case Sw64::VADDD:
    if (canCombineWithFMUL(MBB, Root.getOperand(1), Sw64::VMULD)) {
      Patterns.push_back(MachineCombinerPattern::VMULADDD_OP1);
      Found = true;
    }
    if (canCombineWithFMUL(MBB, Root.getOperand(2), Sw64::VMULD)) {
      Patterns.push_back(MachineCombinerPattern::VMULADDD_OP2);
      Found = true;
    }
    break;

  case Sw64::VSUBS:
    if (canCombineWithFMUL(MBB, Root.getOperand(1), Sw64::VMULS)) {
      Patterns.push_back(MachineCombinerPattern::VMULSUBS_OP1);
      Found = true;
    }
    if (canCombineWithFMUL(MBB, Root.getOperand(2), Sw64::VMULS)) {
      Patterns.push_back(MachineCombinerPattern::VMULSUBS_OP2);
      Found = true;
    }
    break;
  case Sw64::VSUBD:
    if (canCombineWithFMUL(MBB, Root.getOperand(1), Sw64::VMULD)) {
      Patterns.push_back(MachineCombinerPattern::VMULSUBD_OP1);
      Found = true;
    }
    if (canCombineWithFMUL(MBB, Root.getOperand(2), Sw64::VMULD)) {
      Patterns.push_back(MachineCombinerPattern::VMULSUBD_OP2);
      Found = true;
    }
    break;
  }
  return Found;
}

/// Return true when a code sequence can improve throughput. It
/// should be called only for instructions in loops.
/// \param Pattern - combiner pattern
bool Sw64InstrInfo::isThroughputPattern(MachineCombinerPattern Pattern) const {
  switch (Pattern) {
  default:
    break;
  case MachineCombinerPattern::FMULADDS_OP1:
  case MachineCombinerPattern::FMULADDS_OP2:
  case MachineCombinerPattern::FMULSUBS_OP1:
  case MachineCombinerPattern::FMULSUBS_OP2:
  case MachineCombinerPattern::FMULADDD_OP1:
  case MachineCombinerPattern::FMULADDD_OP2:
  case MachineCombinerPattern::FMULSUBD_OP1:
  case MachineCombinerPattern::FMULSUBD_OP2:
  case MachineCombinerPattern::FNMULSUBS_OP1:
  case MachineCombinerPattern::FNMULSUBD_OP1:
  case MachineCombinerPattern::VMULADDS_OP1:
  case MachineCombinerPattern::VMULADDS_OP2:
  case MachineCombinerPattern::VMULADDD_OP1:
  case MachineCombinerPattern::VMULADDD_OP2:
  case MachineCombinerPattern::VMULSUBS_OP1:
  case MachineCombinerPattern::VMULSUBS_OP2:
  case MachineCombinerPattern::VMULSUBD_OP1:
  case MachineCombinerPattern::VMULSUBD_OP2:
    return true;
  } // end switch (Pattern)
  return false;
}

/// Return true when there is potentially a faster code sequence for an
/// instruction chain ending in \p Root. All potential patterns are listed in
/// the \p Pattern vector. Pattern should be sorted in priority order since the
/// pattern evaluator stops checking as soon as it finds a faster sequence.
bool Sw64InstrInfo::getMachineCombinerPatterns(
    MachineInstr &Root, SmallVectorImpl<MachineCombinerPattern> &Patterns,
    bool DoRegPressureReduce) const {
  // Floating point patterns
  if (getFMAPatterns(Root, Patterns))
    return true;

  return TargetInstrInfo::getMachineCombinerPatterns(Root, Patterns,
                                                     DoRegPressureReduce);
}

enum class FMAInstKind { Default, Indexed, Accumulator };
/// genFusedMultiply - Generate fused multiply instructions.
/// This function supports both integer and floating point instructions.
/// A typical example:
///  F|MUL I=A,B,0
///  F|ADD R,I,C
///  ==> F|MADD R,A,B,C
/// \param MF Containing MachineFunction
/// \param MRI Register information
/// \param TII Target information
/// \param Root is the F|ADD instruction
/// \param [out] InsInstrs is a vector of machine instructions and will
/// contain the generated madd instruction
/// \param IdxMulOpd is index of operand in Root that is the result of
/// the F|MUL. In the example above IdxMulOpd is 1.
/// \param MaddOpc the opcode fo the f|madd instruction
/// \param RC Register class of operands
/// \param kind of fma instruction (addressing mode) to be generated
/// \param ReplacedAddend is the result register from the instruction
/// replacing the non-combined operand, if any.
static MachineInstr *
genFusedMultiply(MachineFunction &MF, MachineRegisterInfo &MRI,
                 const TargetInstrInfo *TII, MachineInstr &Root,
                 SmallVectorImpl<MachineInstr *> &InsInstrs, unsigned IdxMulOpd,
                 unsigned MaddOpc, const TargetRegisterClass *RC,
                 FMAInstKind kind = FMAInstKind::Default,
                 const unsigned *ReplacedAddend = nullptr) {
  assert(IdxMulOpd == 1 || IdxMulOpd == 2);

  LLVM_DEBUG(dbgs() << "creating fma insn \n");
  LLVM_DEBUG(dbgs() << MaddOpc);
  LLVM_DEBUG(dbgs() << "\n");

  unsigned IdxOtherOpd = IdxMulOpd == 1 ? 2 : 1;
  MachineInstr *MUL = MRI.getUniqueVRegDef(Root.getOperand(IdxMulOpd).getReg());
  unsigned ResultReg = Root.getOperand(0).getReg();
  unsigned SrcReg0 = MUL->getOperand(1).getReg();
  bool Src0IsKill = MUL->getOperand(1).isKill();
  unsigned SrcReg1 = MUL->getOperand(2).getReg();
  bool Src1IsKill = MUL->getOperand(2).isKill();

  unsigned SrcReg2;
  bool Src2IsKill;
  if (ReplacedAddend) {
    // If we just generated a new addend, we must be it's only use.
    SrcReg2 = *ReplacedAddend;
    Src2IsKill = true;
  } else {
    SrcReg2 = Root.getOperand(IdxOtherOpd).getReg();
    Src2IsKill = Root.getOperand(IdxOtherOpd).isKill();
  }
  if (Register::isVirtualRegister(ResultReg))
    MRI.constrainRegClass(ResultReg, RC);
  if (Register::isVirtualRegister(SrcReg0))
    MRI.constrainRegClass(SrcReg0, RC);
  if (Register::isVirtualRegister(SrcReg1))
    MRI.constrainRegClass(SrcReg1, RC);
  if (Register::isVirtualRegister(SrcReg2))
    MRI.constrainRegClass(SrcReg2, RC);

  MachineInstrBuilder MIB;
  if (kind == FMAInstKind::Default)
    MIB = BuildMI(MF, Root.getDebugLoc(), TII->get(MaddOpc), ResultReg)
              .addReg(SrcReg0, getKillRegState(Src0IsKill))
              .addReg(SrcReg1, getKillRegState(Src1IsKill))
              .addReg(SrcReg2, getKillRegState(Src2IsKill));
  else if (kind == FMAInstKind::Indexed)
    MIB = BuildMI(MF, Root.getDebugLoc(), TII->get(MaddOpc), ResultReg)
              .addReg(SrcReg2, getKillRegState(Src2IsKill))
              .addReg(SrcReg0, getKillRegState(Src0IsKill))
              .addReg(SrcReg1, getKillRegState(Src1IsKill))
              .addImm(MUL->getOperand(3).getImm());
  else if (kind == FMAInstKind::Accumulator)
    MIB = BuildMI(MF, Root.getDebugLoc(), TII->get(MaddOpc), ResultReg)
              .addReg(SrcReg2, getKillRegState(Src2IsKill))
              .addReg(SrcReg0, getKillRegState(Src0IsKill))
              .addReg(SrcReg1, getKillRegState(Src1IsKill));
  else
    assert(false && "Invalid FMA instruction kind \n");
  // Insert the MADD (MADD, FMA, FMS, FMLA, FMSL)
  InsInstrs.push_back(MIB);
  return MUL;
}

/// When getMachineCombinerPatterns() finds potential patterns,
/// this function generates the instructions that could replace the
/// original code sequence
void Sw64InstrInfo::genAlternativeCodeSequence(
    MachineInstr &Root, MachineCombinerPattern Pattern,
    SmallVectorImpl<MachineInstr *> &InsInstrs,
    SmallVectorImpl<MachineInstr *> &DelInstrs,
    DenseMap<unsigned, unsigned> &InstrIdxForVirtReg) const {

  LLVM_DEBUG(dbgs() << "combining float instring\n");
  MachineBasicBlock &MBB = *Root.getParent();
  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

  MachineInstr *MUL;
  const TargetRegisterClass *RC;
  unsigned Opc;
  switch (Pattern) {
  default:
    // Reassociate instructions.
    TargetInstrInfo::genAlternativeCodeSequence(Root, Pattern, InsInstrs,
                                                DelInstrs, InstrIdxForVirtReg);
    return;
  // Floating Point Support
  case MachineCombinerPattern::FMULADDS_OP1:
  case MachineCombinerPattern::FMULADDD_OP1:
    // FMUL I=A,B
    // FADD R,I,C
    // ==> FMAx R,A,B,C
    // --- Create(FMAx);
    if (Pattern == MachineCombinerPattern::FMULADDS_OP1) {
      Opc = Sw64::FMAS;
      RC = &Sw64::F4RCRegClass;
    } else {
      Opc = Sw64::FMAD;
      RC = &Sw64::F8RCRegClass;
    }
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 1, Opc, RC);
    break;
  case MachineCombinerPattern::FMULADDS_OP2:
  case MachineCombinerPattern::FMULADDD_OP2:
    // FMUL I=A,B
    // FADD R,C,I
    // ==> FMAx R,A,B,C
    // --- Create(FMAx);
    if (Pattern == MachineCombinerPattern::FMULADDS_OP2) {
      Opc = Sw64::FMAS;
      RC = &Sw64::F4RCRegClass;
    } else {
      Opc = Sw64::FMAD;
      RC = &Sw64::F8RCRegClass;
    }
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 2, Opc, RC);
    break;

  case MachineCombinerPattern::FMULSUBS_OP1:
  case MachineCombinerPattern::FMULSUBD_OP1: {
    // FMUL I=A,B,0
    // FSUB R,I,C
    // ==> FMSx R,A,B,C // = A*B - C
    // --- Create(FMSx);
    if (Pattern == MachineCombinerPattern::FMULSUBS_OP1) {
      Opc = Sw64::FMSS;
      RC = &Sw64::F4RCRegClass;
    } else {
      Opc = Sw64::FMSD;
      RC = &Sw64::F8RCRegClass;
    }
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 1, Opc, RC);
    break;
  }
  case MachineCombinerPattern::FMULSUBS_OP2:
  case MachineCombinerPattern::FMULSUBD_OP2: {
    // FMUL I=A,B,0
    // FSUB R,I,C
    // ==> FNMAx R,A,B,C // = -A*B + C
    // --- Create(FNMAx);
    if (Pattern == MachineCombinerPattern::FMULSUBS_OP2) {
      Opc = Sw64::FNMAS;
      RC = &Sw64::F4RCRegClass;
    } else {
      Opc = Sw64::FNMAD;
      RC = &Sw64::F8RCRegClass;
    }
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 2, Opc, RC);
    break;
  }

  case MachineCombinerPattern::FNMULSUBS_OP1:
  case MachineCombinerPattern::FNMULSUBD_OP1: {
    // FNMUL I=A,B,0
    // FSUB R,I,C
    // ==> FNMSx R,A,B,C // = -A*B - C
    // --- Create(FNMSx);
    if (Pattern == MachineCombinerPattern::FNMULSUBS_OP1) {
      Opc = Sw64::FNMSS;
      RC = &Sw64::F4RCRegClass;
    } else {
      Opc = Sw64::FNMSD;
      RC = &Sw64::F8RCRegClass;
    }
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 1, Opc, RC);
    break;
  }

  case MachineCombinerPattern::VMULADDS_OP1:
  case MachineCombinerPattern::VMULADDD_OP1: {
    // VMULx I=A,B
    // VADDx I,C,R
    // ==> VMAx A,B,C,R // = A*B+C
    // --- Create(VMAx);
    Opc = Pattern == MachineCombinerPattern::VMULADDS_OP1 ? Sw64::VMAS
                                                          : Sw64::VMAD;
    RC = &Sw64::V256LRegClass;
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 1, Opc, RC);
    break;
  }
  case MachineCombinerPattern::VMULADDS_OP2:
  case MachineCombinerPattern::VMULADDD_OP2: {
    // VMUL I=A,B
    // VADD C,R,I
    // ==> VMA A,B,C,R (computes C + A*B)
    // --- Create(FMSUB);
    Opc = Pattern == MachineCombinerPattern::VMULADDS_OP2 ? Sw64::VMAS
                                                          : Sw64::VMAD;
    RC = &Sw64::V256LRegClass;
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 2, Opc, RC);
    break;
  }

  case MachineCombinerPattern::VMULSUBS_OP1:
  case MachineCombinerPattern::VMULSUBD_OP1: {
    // VMULx I=A,B
    // VSUBx I,C,R
    // ==> VMSx A,B,C,R // = A*B-C
    // --- Create(VMSx);
    Opc = Pattern == MachineCombinerPattern::VMULSUBS_OP1 ? Sw64::VMSS
                                                          : Sw64::VMSD;
    RC = &Sw64::V256LRegClass;
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 1, Opc, RC);
    break;
  }
  case MachineCombinerPattern::VMULSUBS_OP2:
  case MachineCombinerPattern::VMULSUBD_OP2: {
    // FMUL I=A,B,0
    // FSUB R,C,I
    // ==> FMSUB R,A,B,C (computes C - A*B)
    // --- Create(FMSUB);
    Opc = Pattern == MachineCombinerPattern::VMULSUBS_OP2 ? Sw64::VNMAS
                                                          : Sw64::VNMAD;
    RC = &Sw64::V256LRegClass;
    MUL = genFusedMultiply(MF, MRI, TII, Root, InsInstrs, 2, Opc, RC);
    break;
  }
  } // end switch (Pattern)
  // Record MUL and ADD/SUB for deletion
  DelInstrs.push_back(MUL);
  DelInstrs.push_back(&Root);
}

bool Sw64InstrInfo::isSchedulingBoundary(const MachineInstr &MI,
                                         const MachineBasicBlock *MBB,
                                         const MachineFunction &MF) const {
  if (TargetInstrInfo::isSchedulingBoundary(MI, MBB, MF))
    return true;

  switch (MI.getOpcode()) {
  case Sw64::MOVProgPCGp:
  case Sw64::MOVaddrPCGp:
  case Sw64::WMEMB:
  case Sw64::IMEMB:
  case Sw64::MB:
    return true;
  }
  return false;
}
