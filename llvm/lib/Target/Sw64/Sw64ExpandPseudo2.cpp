//===-- Sw64ExpandPseudoInsts.cpp - Expand pseudo instructions ------------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a pass that expands pseudo instructions into target
// instructions to allow proper scheduling, if-conversion, and other late
// optimizations. This pass should be run after register allocation but before
// the post-regalloc scheduling pass.
//
// This is currently only used for expanding atomic pseudos after register
// allocation. We do this to avoid the fast register allocator introducing
// spills between ll and sc. These stores cause some other implementations to
// abort the atomic RMW sequence.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Sw64BaseInfo.h"
#include "Sw64.h"
#include "Sw64InstrInfo.h"
#include "Sw64Subtarget.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "sw_64-pseudo2"
namespace llvm {
extern const MCInstrDesc Sw64Insts[];
}

static cl::opt<bool>
    ExpandPre("expand-presched",
              cl::desc("Expand pseudo Inst before PostRA schedule"),
              cl::init(true), cl::Hidden);

namespace {
class Sw64ExpandPseudo2 : public MachineFunctionPass {
public:
  static char ID;
  Sw64ExpandPseudo2() : MachineFunctionPass(ID) {}

  const Sw64InstrInfo *TII;
  const Sw64Subtarget *STI;

  bool runOnMachineFunction(MachineFunction &Fn) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoVRegs);
  }

  StringRef getPassName() const override {
    return "Sw64 pseudo instruction expansion pass2";
  }

private:
  bool expandPseudoCall(MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MBBI,
                        MachineBasicBlock::iterator &NextMBBI);

  bool expandLoadAddress(MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MBBI,
                         MachineBasicBlock::iterator &NextMBBI);

  bool expandLoadCPAddress(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI,
                           MachineBasicBlock::iterator &NextMBBI);

  bool expandLdihInstPair(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MBBI,
                          MachineBasicBlock::iterator &NextMBBI,
                          unsigned FlagsHi, unsigned SecondOpcode,
                          unsigned FlagsLo = Sw64II::MO_GPREL_LO,
                          unsigned srcReg = Sw64::R29);

  bool expandLoadGotAddress(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI,
                            MachineBasicBlock::iterator &NextMBBI);

  bool expandMI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                MachineBasicBlock::iterator &NMBB);

  bool expandMBB(MachineBasicBlock &MBB);
};
char Sw64ExpandPseudo2::ID = 0;
} // namespace

bool Sw64ExpandPseudo2::expandMI(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI,
                                 MachineBasicBlock::iterator &NMBB) {
  bool Modified = false;

  if (ExpandPre) {
    switch (MBBI->getOpcode()) {
    case Sw64::LOADlitSym:
    case Sw64::LOADlit:
      return expandLoadGotAddress(MBB, MBBI, NMBB);
    case Sw64::LOADconstant:
      return expandLoadCPAddress(MBB, MBBI, NMBB);
    case Sw64::MOVaddrCP:
    case Sw64::MOVaddrBA:
    case Sw64::MOVaddrGP:
    case Sw64::MOVaddrEXT:
    case Sw64::MOVaddrJT:
      return expandLoadAddress(MBB, MBBI, NMBB);
    case Sw64::PseudoCall:
      return expandPseudoCall(MBB, MBBI, NMBB);
    default:
      return Modified;
    }
  } else {
    switch (MBBI->getOpcode()) {
    case Sw64::PseudoCall:
      return expandPseudoCall(MBB, MBBI, NMBB);
    default:
      return Modified;
    }
  }
}

bool Sw64ExpandPseudo2::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI, NMBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool Sw64ExpandPseudo2::expandLoadCPAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  return expandLdihInstPair(MBB, MBBI, NextMBBI, Sw64II::MO_GPREL_HI,
                            Sw64::LDL);
}

bool Sw64ExpandPseudo2::expandLoadAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  return expandLdihInstPair(MBB, MBBI, NextMBBI, Sw64II::MO_GPREL_HI,
                            Sw64::LDA);
}

bool Sw64ExpandPseudo2::expandLdihInstPair(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI, unsigned FlagsHi,
    unsigned SecondOpcode, unsigned FlagsLo, unsigned srcReg) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  unsigned DestReg = MI.getOperand(0).getReg();
  const MachineOperand &Symbol = MI.getOperand(1);

  MachineInstrBuilder MIB =
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDAH), DestReg)
          .add(Symbol)
          .addReg(srcReg);
  MachineInstrBuilder MIB1 =
      BuildMI(MBB, MBBI, DL, TII->get(SecondOpcode), DestReg)
          .add(Symbol)
          .addReg(DestReg);

  MachineInstr *tmpInst = MIB.getInstr();
  MachineInstr *tmpInst1 = MIB1.getInstr();

  MachineOperand &SymbolHi = tmpInst->getOperand(1);
  MachineOperand &SymbolLo = tmpInst1->getOperand(1);

  SymbolHi.addTargetFlag(FlagsHi);
  SymbolLo.addTargetFlag(FlagsLo);

  MI.eraseFromParent();
  return true;
}

// while expanding call, we can choose adding lituse
// for linker relax or not. Adding flags for sortRelocs
bool Sw64ExpandPseudo2::expandPseudoCall(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  LLVM_DEBUG(dbgs() << "expand PseudoCall" << *MBBI);

  MachineFunction *MF = MBB.getParent();
  const auto &STI = MF->getSubtarget<Sw64Subtarget>();
  const Sw64FrameLowering *SFL = STI.getFrameLowering();

  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();
  unsigned Lflags = 0; // load flags
  unsigned Cflags = 0; // Call flags

  MachineOperand Symbol = MI.getOperand(0);
  switch (MF->getTarget().getCodeModel()) {
  default:
    report_fatal_error("Unsupported code model for lowering");
  case CodeModel::Small: {
    if (Symbol.isGlobal()) {
      int64_t Offs = Symbol.getOffset();
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDL), Sw64::R27)
          .addGlobalAddress(Symbol.getGlobal(), Offs,
                            Lflags | Sw64II::MO_LITERAL |
                                Sw64II::MO_LITERAL_BASE)
          .addReg(Sw64::R29);
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::JSR), Sw64::R26)
          .addReg(Sw64::R27)
          .addGlobalAddress(Symbol.getGlobal(), 0,
                            Cflags | Sw64II::MO_HINT | Sw64II::MO_LITUSE);
    } else if (Symbol.isSymbol()) {
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDL), Sw64::R27)
          .addExternalSymbol(Symbol.getSymbolName(), Sw64II::MO_LITERAL)
          .addReg(Sw64::R29);
      const Sw64TargetLowering *STL = STI.getTargetLowering();
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::JSR), Sw64::R26)
          .addReg(Sw64::R27)
          .addExternalSymbol(Symbol.getSymbolName());
    }
    break;
  }

  case CodeModel::Medium: {
    if (Symbol.isGlobal()) {
      int64_t Offs = Symbol.getOffset();
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDAH), Sw64::R27)
          .addGlobalAddress(Symbol.getGlobal(), Offs, Sw64II::MO_LITERAL_GOT)
          .addReg(Sw64::R29);
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDL), Sw64::R27)
          .addGlobalAddress(Symbol.getGlobal(), Offs,
                            Lflags | Sw64II::MO_LITERAL |
                                Sw64II::MO_LITERAL_BASE)
          .addReg(Sw64::R27);
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::JSR), Sw64::R26)
          .addReg(Sw64::R27)
          .addGlobalAddress(Symbol.getGlobal(), 0,
                            Cflags | Sw64II::MO_HINT | Sw64II::MO_LITUSE);
    } else if (Symbol.isSymbol()) {
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDAH), Sw64::R27)
          .addExternalSymbol(Symbol.getSymbolName(), Sw64II::MO_LITERAL_GOT)
          .addReg(Sw64::R29);
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDL), Sw64::R27)
          .addExternalSymbol(Symbol.getSymbolName(), Sw64II::MO_LITERAL)
          .addReg(Sw64::R27);
      const Sw64TargetLowering *STL = STI.getTargetLowering();
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::JSR), Sw64::R26)
          .addReg(Sw64::R27)
          .addExternalSymbol(Symbol.getSymbolName());
    }
    break;
  }
  }

  MI.eraseFromParent();
  return true;
}

bool Sw64ExpandPseudo2::expandLoadGotAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  LLVM_DEBUG(dbgs() << "expand Loadlit LoadlitSym" << *MBBI);
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  unsigned DestReg = MI.getOperand(0).getReg();
  const MachineOperand &Symbol = MI.getOperand(1);

  MachineFunction *MF = MBB.getParent();
  switch (MF->getTarget().getCodeModel()) {
  default:
    report_fatal_error("Unsupported code model for lowering");
  case CodeModel::Small: {
    if (Symbol.isSymbol())
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDL), DestReg)
          .addExternalSymbol(Symbol.getSymbolName(), Sw64II::MO_LITERAL)
          .addReg(Sw64::R29);
    else
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDL), DestReg)
          .addDisp(Symbol, 0, Sw64II::MO_LITERAL)
          .addReg(Sw64::R29);
    break;
  }

  case CodeModel::Medium: {
    if (Symbol.isSymbol()) {
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDAH), DestReg)
          .addExternalSymbol(Symbol.getSymbolName(), Sw64II::MO_LITERAL_GOT)
          .addReg(Sw64::R29);
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDL), DestReg)
          .addExternalSymbol(Symbol.getSymbolName(), Sw64II::MO_LITERAL)
          .addReg(DestReg);
    } else {
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDAH), DestReg)
          .addDisp(Symbol, 0, Sw64II::MO_LITERAL_GOT)
          .addReg(Sw64::R29);
      BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDL), DestReg)
          .addDisp(Symbol, 0, Sw64II::MO_LITERAL)
          .addReg(DestReg);
    }
    break;
  }
  }
  MI.eraseFromParent();
  return true;
}

bool Sw64ExpandPseudo2::runOnMachineFunction(MachineFunction &MF) {
  STI = &static_cast<const Sw64Subtarget &>(MF.getSubtarget());
  TII = STI->getInstrInfo();

  bool Modified = false;
  for (MachineFunction::iterator MFI = MF.begin(), E = MF.end(); MFI != E;
       ++MFI)
    Modified |= expandMBB(*MFI);

  if (Modified)
    MF.RenumberBlocks();

  return Modified;
}

/// createSw64ExpandPseudoPass - returns an instance of the pseudo instruction
/// expansion pass.
FunctionPass *llvm::createSw64ExpandPseudo2Pass() {
  return new Sw64ExpandPseudo2();
}
