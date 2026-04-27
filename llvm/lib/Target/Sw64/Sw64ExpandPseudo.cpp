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

#define DEBUG_TYPE "sw_64-pseudo"
namespace llvm {
extern const MCInstrDesc Sw64Insts[];
}

namespace {
class Sw64ExpandPseudo : public MachineFunctionPass {
public:
  static char ID;
  Sw64ExpandPseudo() : MachineFunctionPass(ID) {}

  const Sw64InstrInfo *TII;
  const Sw64Subtarget *STI;

  bool runOnMachineFunction(MachineFunction &Fn) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoVRegs);
  }

  StringRef getPassName() const override {
    return "Sw64 pseudo instruction expansion pass";
  }

private:
  bool expandAtomicCmpSwap(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI,
                           MachineBasicBlock::iterator &NextMBBI,
                           unsigned Size);
  bool expandAtomicCmpSwapSubword(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MBBI,
                                  MachineBasicBlock::iterator &NextMBBI);

  bool expandAtomicBinOp(MachineBasicBlock &BB, MachineBasicBlock::iterator I,
                         MachineBasicBlock::iterator &NMBBI, unsigned Size);
  bool expandAtomicBinOpSubword(MachineBasicBlock &BB,
                                MachineBasicBlock::iterator I,
                                MachineBasicBlock::iterator &NMBBI);
  bool expandCurGpdisp(MachineBasicBlock &MBB,
                       MachineBasicBlock::iterator MBBI);

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
  bool expandIntReduceSum(MachineBasicBlock &BB, MachineBasicBlock::iterator I,
                          MachineBasicBlock::iterator &NMBBI);
  bool expandFPReduceSum(MachineBasicBlock &BB, MachineBasicBlock::iterator I,
                         MachineBasicBlock::iterator &NMBBI);
};
char Sw64ExpandPseudo::ID = 0;
} // namespace

bool Sw64ExpandPseudo::expandAtomicCmpSwapSubword(
    MachineBasicBlock &BB, MachineBasicBlock::iterator I,
    MachineBasicBlock::iterator &NMBBI) {

  MachineFunction *MF = BB.getParent();
  DebugLoc DL = I->getDebugLoc();

  unsigned LL, SC, BEQ;
  unsigned BIC, BIS;
  unsigned EXTL, INSL, MASKL;
  unsigned mask;
  BIS = Sw64::BISr;
  BIC = Sw64::BICi;
  BEQ = Sw64::BEQ;
  LL = Sw64 ::LDQ_L;
  SC = Sw64::STQ_C;
  Register Dest = I->getOperand(0).getReg();
  Register Ptr = I->getOperand(1).getReg();
  Register OldVal = I->getOperand(2).getReg();
  Register NewVal = I->getOperand(3).getReg();
  // add
  Register Reg_bic = I->getOperand(4).getReg();
  Register Reg_ins = I->getOperand(5).getReg();
  Register LockVal = I->getOperand(6).getReg();
  Register Reg_cmp = I->getOperand(7).getReg();
  Register Reg_mas = I->getOperand(8).getReg();
  switch (I->getOpcode()) {
  case Sw64::ATOMIC_CMP_SWAP_I8_POSTRA:
    mask = 1;
    EXTL = Sw64::EXTLBr;
    INSL = Sw64::INSLBr;
    MASKL = Sw64::MASKLBr;
    break;
  case Sw64::ATOMIC_CMP_SWAP_I16_POSTRA:
    mask = 3;
    EXTL = Sw64::EXTLHr;
    INSL = Sw64::INSLHr;
    MASKL = Sw64::MASKLHr;
    break;
  default:
    llvm_unreachable("Unknown pseudo atomic!");
  }

  const BasicBlock *LLVM_BB = BB.getBasicBlock();
  MachineBasicBlock *loopMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *exitMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineFunction::iterator It = ++BB.getIterator();
  MF->insert(It, loopMBB);
  MF->insert(It, exitMBB);

  exitMBB->splice(exitMBB->begin(), &BB, std::next(I), BB.end());
  exitMBB->transferSuccessorsAndUpdatePHIs(&BB);

  BB.addSuccessor(loopMBB, BranchProbability::getOne());
  loopMBB->addSuccessor(loopMBB);
  loopMBB->addSuccessor(exitMBB);
  loopMBB->normalizeSuccProbs();

  // memb
  BuildMI(loopMBB, DL, TII->get(Sw64::MB));

  // bic
  BuildMI(loopMBB, DL, TII->get(BIC), Reg_bic).addReg(Ptr).addImm(7);

  // inslh
  BuildMI(loopMBB, DL, TII->get(INSL), Reg_ins).addReg(NewVal).addReg(Ptr);

  // lldl
  BuildMI(loopMBB, DL, TII->get(LL), LockVal).addImm(0).addReg(Reg_bic);

  // extlh
  BuildMI(loopMBB, DL, TII->get(EXTL), Dest).addReg(LockVal).addReg(Ptr);

  // cmpeq
  // zapnot
  BuildMI(loopMBB, DL, TII->get(Sw64::ZAPNOTi), OldVal)
      .addReg(OldVal)
      .addImm(mask);
  BuildMI(loopMBB, DL, TII->get(Sw64::ZAPNOTi), Dest).addReg(Dest).addImm(mask);
  BuildMI(loopMBB, DL, TII->get(Sw64::CMPEQr), Reg_cmp)
      .addReg(OldVal)
      .addReg(Dest);

  if (STI->hasCore4())
    // beq
    BuildMI(loopMBB, DL, TII->get(BEQ)).addReg(Reg_cmp).addMBB(exitMBB);
  else
    // wr_f
    BuildMI(loopMBB, DL, TII->get(Sw64::WR_F)).addReg(Reg_cmp);

  // masklh
  BuildMI(loopMBB, DL, TII->get(MASKL), Reg_mas).addReg(LockVal).addReg(Ptr);

  // bis
  BuildMI(loopMBB, DL, TII->get(BIS), Reg_ins).addReg(Reg_mas).addReg(Reg_ins);

  // lstw
  BuildMI(loopMBB, DL, TII->get(SC)).addReg(Reg_ins).addImm(0).addReg(Reg_bic);

  if (!STI->hasCore4())
    // rd_f
    BuildMI(loopMBB, DL, TII->get(Sw64::RD_F)).addReg(Reg_ins);

  // beq
  BuildMI(loopMBB, DL, TII->get(BEQ)).addReg(Reg_cmp).addMBB(exitMBB);

  // beq
  BuildMI(loopMBB, DL, TII->get(BEQ)).addReg(Reg_ins).addMBB(loopMBB);

  NMBBI = BB.end();
  I->eraseFromParent(); // The instruction is gone now.

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *loopMBB);
  computeAndAddLiveIns(LiveRegs, *exitMBB);
  return true;
}

bool Sw64ExpandPseudo::expandAtomicCmpSwap(MachineBasicBlock &BB,
                                           MachineBasicBlock::iterator I,
                                           MachineBasicBlock::iterator &NMBBI,
                                           unsigned Size) {
  MachineFunction *MF = BB.getParent();
  DebugLoc DL = I->getDebugLoc();
  unsigned LL, SC;
  unsigned BEQ = Sw64::BEQ;

  if (Size == 4) {
    LL = Sw64 ::LDL_L;
    SC = Sw64::STL_C;
  } else {
    LL = Sw64::LDQ_L;
    SC = Sw64::STQ_C;
  }

  Register Dest = I->getOperand(0).getReg();
  Register Ptr = I->getOperand(1).getReg();
  Register OldVal = I->getOperand(2).getReg();
  Register NewVal = I->getOperand(3).getReg();
  Register Scratch = I->getOperand(4).getReg();
  // add
  Register Reg_cmp = I->getOperand(5).getReg();

  // insert new blocks after the current block
  const BasicBlock *LLVM_BB = BB.getBasicBlock();
  MachineBasicBlock *loop1MBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *exitMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineFunction::iterator It = ++BB.getIterator();
  MF->insert(It, loop1MBB);
  MF->insert(It, exitMBB);

  // Transfer the remainder of BB and its successor edges to exitMBB.
  exitMBB->splice(exitMBB->begin(), &BB,
                  std::next(MachineBasicBlock::iterator(I)), BB.end());
  exitMBB->transferSuccessorsAndUpdatePHIs(&BB);

  //  thisMBB:
  //    ...
  //    fallthrough --> loop1MBB
  BB.addSuccessor(loop1MBB, BranchProbability::getOne());

  loop1MBB->addSuccessor(loop1MBB);
  loop1MBB->addSuccessor(exitMBB);
  loop1MBB->normalizeSuccProbs();

  // memb
  BuildMI(loop1MBB, DL, TII->get(Sw64::MB));

  // ldi
  BuildMI(loop1MBB, DL, TII->get(Sw64::LDA), Ptr).addImm(0).addReg(Ptr);

  // lldw
  BuildMI(loop1MBB, DL, TII->get(LL), Dest).addImm(0).addReg(Ptr);

  // zapnot
  if (Size == 4) {
    BuildMI(loop1MBB, DL, TII->get(Sw64::ZAPNOTi), OldVal)
        .addReg(OldVal)
        .addImm(15);
    BuildMI(loop1MBB, DL, TII->get(Sw64::ZAPNOTi), Dest)
        .addReg(Dest)
        .addImm(15);
  }

  // cmpeq
  BuildMI(loop1MBB, DL, TII->get(Sw64::CMPEQr))
      .addReg(Reg_cmp)
      .addReg(OldVal)
      .addReg(Dest);

  if (STI->hasCore4())
    // beq
    BuildMI(loop1MBB, DL, TII->get(BEQ)).addReg(Reg_cmp).addMBB(exitMBB);
  else
    // wr_f
    BuildMI(loop1MBB, DL, TII->get(Sw64::WR_F)).addReg(Reg_cmp);

  // mov
  BuildMI(loop1MBB, DL, TII->get(Sw64::BISr), Scratch)
      .addReg(NewVal)
      .addReg(NewVal);

  // lstw
  BuildMI(loop1MBB, DL, TII->get(SC)).addReg(Scratch).addImm(0).addReg(Ptr);

  if (!STI->hasCore4())
    // rd_f
    BuildMI(loop1MBB, DL, TII->get(Sw64::RD_F)).addReg(Scratch);

  // beq
  BuildMI(loop1MBB, DL, TII->get(BEQ)).addReg(Reg_cmp).addMBB(exitMBB);

  BuildMI(loop1MBB, DL, TII->get(BEQ)).addReg(Scratch).addMBB(loop1MBB);

  NMBBI = BB.end();
  I->eraseFromParent(); // The instruction is gone now.

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *loop1MBB);
  computeAndAddLiveIns(LiveRegs, *exitMBB);

  return true;
}

bool Sw64ExpandPseudo::expandAtomicBinOpSubword(
    MachineBasicBlock &BB, MachineBasicBlock::iterator I,
    MachineBasicBlock::iterator &NMBBI) {

  MachineFunction *MF = BB.getParent();
  DebugLoc DL = I->getDebugLoc();
  unsigned LL, SC, ZERO, BEQ;
  unsigned EXTL, INSL, MASKL;

  unsigned WR_F, RD_F, LDA, BIS, BIC;
  WR_F = Sw64::WR_F;
  RD_F = Sw64::RD_F;
  LDA = Sw64::LDA;
  BIS = Sw64::BISr;
  BIC = Sw64::BICi;
  LL = Sw64::LDQ_L;
  SC = Sw64::STQ_C;
  ZERO = Sw64::R31;
  BEQ = Sw64::BEQ;

  Register OldVal = I->getOperand(0).getReg();
  Register Ptr = I->getOperand(1).getReg();
  Register Incr = I->getOperand(2).getReg();
  Register StoreVal = I->getOperand(3).getReg();
  // add
  Register LockVal = I->getOperand(4).getReg();
  Register Reg_bic = I->getOperand(5).getReg();
  Register cmpres = I->getOperand(6).getReg();

  unsigned Opcode = 0;
  switch (I->getOpcode()) {
  case Sw64::ATOMIC_LOAD_ADD_I8_POSTRA:
    Opcode = Sw64::ADDLr;
    EXTL = Sw64::EXTLBr;
    INSL = Sw64::INSLBr;
    MASKL = Sw64::MASKLBr;
    break;
  case Sw64::ATOMIC_LOAD_SUB_I8_POSTRA:
    Opcode = Sw64::SUBLr;
    EXTL = Sw64::EXTLBr;
    INSL = Sw64::INSLBr;
    MASKL = Sw64::MASKLBr;
    break;
  case Sw64::ATOMIC_LOAD_AND_I8_POSTRA:
    Opcode = Sw64::ANDr;
    EXTL = Sw64::EXTLBr;
    INSL = Sw64::INSLBr;
    MASKL = Sw64::MASKLBr;
    break;
  case Sw64::ATOMIC_LOAD_OR_I8_POSTRA:
    Opcode = Sw64::BISr;
    EXTL = Sw64::EXTLBr;
    INSL = Sw64::INSLBr;
    MASKL = Sw64::MASKLBr;
    break;
  case Sw64::ATOMIC_LOAD_XOR_I8_POSTRA:
    Opcode = Sw64::XORr;
    EXTL = Sw64::EXTLBr;
    INSL = Sw64::INSLBr;
    MASKL = Sw64::MASKLBr;
    break;
  case Sw64::ATOMIC_SWAP_I8_POSTRA:
    EXTL = Sw64::EXTLBr;
    INSL = Sw64::INSLBr;
    MASKL = Sw64::MASKLBr;
    break;
  case Sw64::ATOMIC_LOAD_ADD_I16_POSTRA:
    Opcode = Sw64::ADDQr;
    EXTL = Sw64::EXTLHr;
    INSL = Sw64::INSLHr;
    MASKL = Sw64::MASKLHr;
    break;
  case Sw64::ATOMIC_LOAD_SUB_I16_POSTRA:
    Opcode = Sw64::SUBQr;
    EXTL = Sw64::EXTLHr;
    INSL = Sw64::INSLHr;
    MASKL = Sw64::MASKLHr;
    break;
  case Sw64::ATOMIC_LOAD_AND_I16_POSTRA:
    Opcode = Sw64::ANDr;
    EXTL = Sw64::EXTLHr;
    INSL = Sw64::INSLHr;
    MASKL = Sw64::MASKLHr;
    break;
  case Sw64::ATOMIC_LOAD_OR_I16_POSTRA:
    Opcode = Sw64::BISr;
    EXTL = Sw64::EXTLHr;
    INSL = Sw64::INSLHr;
    MASKL = Sw64::MASKLHr;
    break;
  case Sw64::ATOMIC_LOAD_XOR_I16_POSTRA:
    Opcode = Sw64::XORr;
    EXTL = Sw64::EXTLHr;
    INSL = Sw64::INSLHr;
    MASKL = Sw64::MASKLHr;
    break;
  case Sw64::ATOMIC_SWAP_I16_POSTRA:
    EXTL = Sw64::EXTLHr;
    INSL = Sw64::INSLHr;
    MASKL = Sw64::MASKLHr;
    break;
  case Sw64::ATOMIC_LOAD_UMAX_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_MAX_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_UMIN_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_MIN_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_NAND_I8_POSTRA:
    EXTL = Sw64::EXTLBr;
    INSL = Sw64::INSLBr;
    MASKL = Sw64::MASKLBr;
    break;
  case Sw64::ATOMIC_LOAD_UMAX_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_MAX_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_UMIN_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_MIN_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_NAND_I16_POSTRA:
    EXTL = Sw64::EXTLHr;
    INSL = Sw64::INSLHr;
    MASKL = Sw64::MASKLHr;
    break;
  default:
    llvm_unreachable("Unknown pseudo atomic!");
  }

  const BasicBlock *LLVM_BB = BB.getBasicBlock();
  MachineBasicBlock *loopMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *exitMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineFunction::iterator It = ++BB.getIterator();
  MF->insert(It, loopMBB);
  MF->insert(It, exitMBB);

  exitMBB->splice(exitMBB->begin(), &BB, std::next(I), BB.end());
  exitMBB->transferSuccessorsAndUpdatePHIs(&BB);

  BB.addSuccessor(loopMBB, BranchProbability::getOne());
  loopMBB->addSuccessor(loopMBB);
  loopMBB->addSuccessor(exitMBB);
  loopMBB->normalizeSuccProbs();

  // memb
  BuildMI(loopMBB, DL, TII->get(Sw64::MB));

  // bic
  BuildMI(loopMBB, DL, TII->get(BIC), Reg_bic).addReg(Ptr).addImm(7);

  // lldl
  BuildMI(loopMBB, DL, TII->get(LL), LockVal).addImm(0).addReg(Reg_bic);

  // ldi
  BuildMI(loopMBB, DL, TII->get(LDA), StoreVal).addImm(1).addReg(ZERO);

  if (!STI->hasCore4())
    // wr_f
    BuildMI(loopMBB, DL, TII->get(WR_F)).addReg(StoreVal);

  // extlh
  BuildMI(loopMBB, DL, TII->get(EXTL), OldVal).addReg(LockVal).addReg(Ptr);

  BuildMI(loopMBB, DL, TII->get(EXTL), OldVal).addReg(LockVal).addReg(Ptr);

  // BinOpcode
  // Use a tmp reg since the src and dst reg of ORNOT op shall not be the same
  // one for unknown reason.
  switch (I->getOpcode()) {
  case Sw64::ATOMIC_LOAD_UMAX_I8_POSTRA:
    // cmpult OldVal, Incr, cmpres          -- cmpres = OldVal < Incr ? 1 : 0
    // seleq cmpres, OldVal, Incr, StoreVal -- StoreVal = cmpres == 0 ? OldVal :
    // Incr
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPULTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    break;
  case Sw64::ATOMIC_LOAD_MAX_I8_POSTRA:
    // cmplt OldVal, Incr, cmpres
    // seleq cmpres, OldVal, Incr, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPLTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    break;
  case Sw64::ATOMIC_LOAD_UMIN_I8_POSTRA:
    // cmpult OldVal, Incr, cmpres          -- cmpres = OldVal < Incr ? 1 : 0
    // seleq cmpres, Incr, OldVal, StoreVal -- StoreVal = cmpres == 0 ? Incr :
    // OldVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPULTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(Incr)
        .addReg(OldVal);
    break;
  case Sw64::ATOMIC_LOAD_MIN_I8_POSTRA:
    // cmplt OldVal, Incr, cmpres
    // seleq cmpres, Incr, OldVal, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPLTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(Incr)
        .addReg(OldVal);
    break;
  case Sw64::ATOMIC_LOAD_NAND_I8_POSTRA:
    // and OldVal, Incr, andres
    // ornot andres, 0, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::ANDr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::ORNOTr), StoreVal)
        .addReg(Sw64::R31)
        .addReg(cmpres);
    break;
  case Sw64::ATOMIC_LOAD_UMAX_I16_POSTRA:
    // cmpult OldVal, Incr, cmpres          -- cmpres = OldVal < Incr ? 1 : 0
    // seleq cmpres, OldVal, Incr, StoreVal -- StoreVal = cmpres == 0 ? OldVal :
    // Incr
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPULTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    break;
  case Sw64::ATOMIC_LOAD_MAX_I16_POSTRA:
    // cmplt OldVal, Incr, cmpres
    // seleq cmpres, OldVal, Incr, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPLTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    break;
  case Sw64::ATOMIC_LOAD_UMIN_I16_POSTRA:
    // cmpult OldVal, Incr, cmpres          -- cmpres = OldVal < Incr ? 1 : 0
    // seleq cmpres, Incr, OldVal, StoreVal -- StoreVal = cmpres == 0 ? Incr :
    // OldVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPULTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(Incr)
        .addReg(OldVal);
    break;
  case Sw64::ATOMIC_LOAD_MIN_I16_POSTRA:
    // cmplt OldVal, Incr, cmpres
    // seleq cmpres, Incr, OldVal, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPLTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(Incr)
        .addReg(OldVal);
    break;
  case Sw64::ATOMIC_LOAD_NAND_I16_POSTRA:
    // and OldVal, Incr, andres
    // ornot andres, 0, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::ANDr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::ORNOTr), StoreVal)
        .addReg(Sw64::R31)
        .addReg(cmpres);
    break;
  default:
    if (Opcode) {
      BuildMI(loopMBB, DL, TII->get(Opcode), StoreVal)
          .addReg(OldVal)
          .addReg(Incr);
    } else {
      BuildMI(loopMBB, DL, TII->get(Sw64::BISr), StoreVal)
          .addReg(Incr)
          .addReg(Incr);
    }
  }

  // inslh
  BuildMI(loopMBB, DL, TII->get(INSL), StoreVal).addReg(StoreVal).addReg(Ptr);

  // masklh
  BuildMI(loopMBB, DL, TII->get(MASKL), LockVal).addReg(LockVal).addReg(Ptr);

  // bis
  BuildMI(loopMBB, DL, TII->get(BIS), LockVal).addReg(LockVal).addReg(StoreVal);

  // lstl
  BuildMI(loopMBB, DL, TII->get(SC)).addReg(LockVal).addImm(0).addReg(Reg_bic);

  if (!STI->hasCore4())
    // rd_f
    BuildMI(loopMBB, DL, TII->get(RD_F)).addReg(LockVal);

  // beq
  BuildMI(loopMBB, DL, TII->get(BEQ)).addReg(LockVal).addMBB(loopMBB);

  NMBBI = BB.end();
  I->eraseFromParent(); // The instruction is gone now.

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *loopMBB);
  computeAndAddLiveIns(LiveRegs, *exitMBB);

  return true;
}

bool Sw64ExpandPseudo::expandAtomicBinOp(MachineBasicBlock &BB,
                                         MachineBasicBlock::iterator I,
                                         MachineBasicBlock::iterator &NMBBI,
                                         unsigned Size) {
  MachineFunction *MF = BB.getParent();
  DebugLoc DL = I->getDebugLoc();
  unsigned LL, SC;
  unsigned LDA = Sw64::LDA;
  unsigned ZERO = Sw64::R31;
  unsigned BEQ = Sw64::BEQ;

  if (Size == 4) {
    LL = Sw64::LDL_L;
    SC = Sw64::STL_C;
  } else {
    LL = Sw64::LDQ_L;
    SC = Sw64::STQ_C;
  }

  Register OldVal = I->getOperand(0).getReg();
  Register Ptr = I->getOperand(1).getReg();
  Register Incr = I->getOperand(2).getReg();
  Register StoreVal = I->getOperand(3).getReg();
  Register Scratch1 = I->getOperand(4).getReg();
  Register cmpres = I->getOperand(5).getReg();

  unsigned Opcode = 0;
  switch (I->getOpcode()) {
  case Sw64::ATOMIC_LOAD_ADD_I32_POSTRA:
    Opcode = Sw64::ADDLr;
    break;
  case Sw64::ATOMIC_LOAD_SUB_I32_POSTRA:
    Opcode = Sw64::SUBLr;
    break;
  case Sw64::ATOMIC_LOAD_AND_I32_POSTRA:
    Opcode = Sw64::ANDr;
    break;
  case Sw64::ATOMIC_LOAD_OR_I32_POSTRA:
    Opcode = Sw64::BISr;
    break;
  case Sw64::ATOMIC_LOAD_XOR_I32_POSTRA:
    Opcode = Sw64::XORr;
    break;
  case Sw64::ATOMIC_SWAP_I32_POSTRA:
    break;
  case Sw64::ATOMIC_LOAD_ADD_I64_POSTRA:
    Opcode = Sw64::ADDQr;
    break;
  case Sw64::ATOMIC_LOAD_SUB_I64_POSTRA:
    Opcode = Sw64::SUBQr;
    break;
  case Sw64::ATOMIC_LOAD_AND_I64_POSTRA:
    Opcode = Sw64::ANDr;
    break;
  case Sw64::ATOMIC_LOAD_OR_I64_POSTRA:
    Opcode = Sw64::BISr;
    break;
  case Sw64::ATOMIC_LOAD_XOR_I64_POSTRA:
    Opcode = Sw64::XORr;
    break;
  case Sw64::ATOMIC_SWAP_I64_POSTRA:
    break;
  case Sw64::ATOMIC_LOAD_UMAX_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_MAX_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_UMIN_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_MIN_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_NAND_I32_POSTRA:

  case Sw64::ATOMIC_LOAD_UMAX_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_MAX_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_UMIN_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_MIN_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_NAND_I64_POSTRA:
    break;
  default:
    llvm_unreachable("Unknown pseudo atomic!");
  }

  // insert new blocks after the current block
  const BasicBlock *LLVM_BB = BB.getBasicBlock();
  MachineBasicBlock *loopMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *exitMBB = MF->CreateMachineBasicBlock(LLVM_BB);

  MachineFunction::iterator It = ++BB.getIterator();
  MF->insert(It, loopMBB);
  MF->insert(It, exitMBB);

  // Transfer the remainder of BB and its successor edges to exitMBB.
  exitMBB->splice(exitMBB->begin(), &BB, std::next(I), BB.end());
  exitMBB->transferSuccessorsAndUpdatePHIs(&BB);

  BB.addSuccessor(loopMBB, BranchProbability::getOne());
  loopMBB->addSuccessor(loopMBB);
  loopMBB->addSuccessor(exitMBB);
  loopMBB->normalizeSuccProbs();

  // memb
  BuildMI(loopMBB, DL, TII->get(Sw64::MB));

  // ldi
  BuildMI(loopMBB, DL, TII->get(Sw64::LDA), Ptr).addImm(0).addReg(Ptr);

  // lldw
  BuildMI(loopMBB, DL, TII->get(LL), OldVal).addImm(0).addReg(Ptr);

  // ldi
  BuildMI(loopMBB, DL, TII->get(LDA), Scratch1).addImm(1).addReg(ZERO);

  if (!STI->hasCore4())
    // wr_f
    BuildMI(loopMBB, DL, TII->get(Sw64::WR_F)).addReg(Scratch1);

  // BinOpcode

  // Use a tmp reg since the src and dst reg of ORNOT op shall not be the same
  // one for unknown reason.
  switch (I->getOpcode()) {
  case Sw64::ATOMIC_LOAD_UMAX_I64_POSTRA:
    // cmpult OldVal, Incr, cmpres          -- cmpres = OldVal < Incr ? 1 : 0
    // seleq cmpres, OldVal, Incr, StoreVal -- StoreVal = cmpres == 0 ? OldVal :
    // Incr
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPULTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    break;
  case Sw64::ATOMIC_LOAD_MAX_I64_POSTRA:
    // cmplt OldVal, Incr, cmpres
    // seleq cmpres, OldVal, Incr, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPLTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    break;
  case Sw64::ATOMIC_LOAD_UMIN_I64_POSTRA:
    // cmpult OldVal, Incr, cmpres          -- cmpres = OldVal < Incr ? 1 : 0
    // seleq cmpres, Incr, OldVal, StoreVal -- StoreVal = cmpres == 0 ? Incr :
    // OldVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPULTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(Incr)
        .addReg(OldVal);
    break;
  case Sw64::ATOMIC_LOAD_MIN_I64_POSTRA:
    // cmplt OldVal, Incr, cmpres
    // seleq cmpres, Incr, OldVal, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPLTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(Incr)
        .addReg(OldVal);
    break;
  case Sw64::ATOMIC_LOAD_NAND_I64_POSTRA:
    // and OldVal, Incr, cmpres
    // ornot cmpres, 0, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::ANDr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::ORNOTr), StoreVal)
        .addReg(Sw64::R31)
        .addReg(cmpres);
    break;
  case Sw64::ATOMIC_LOAD_UMAX_I32_POSTRA:
    // cmpult OldVal, Incr, cmpres          -- cmpres = OldVal < Incr ? 1 : 0
    // seleq cmpres, OldVal, Incr, StoreVal -- StoreVal = cmpres == 0 ? OldVal :
    // Incr
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPULTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    break;
  case Sw64::ATOMIC_LOAD_MAX_I32_POSTRA:
    // cmplt OldVal, Incr, cmpres
    // seleq cmpres, OldVal, Incr, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPLTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    break;
  case Sw64::ATOMIC_LOAD_UMIN_I32_POSTRA:
    // cmpult OldVal, Incr, cmpres          -- cmpres = OldVal < Incr ? 1 : 0
    // seleq cmpres, Incr, OldVal, StoreVal -- StoreVal = cmpres == 0 ? Incr :
    // OldVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPULTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(Incr)
        .addReg(OldVal);
    break;
  case Sw64::ATOMIC_LOAD_MIN_I32_POSTRA:
    // cmplt OldVal, Incr, cmpres
    // seleq cmpres, Incr, OldVal, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::CMPLTr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::SELEQr), StoreVal)
        .addReg(cmpres)
        .addReg(Incr)
        .addReg(OldVal);
    break;
  case Sw64::ATOMIC_LOAD_NAND_I32_POSTRA:
    // and OldVal, Incr, cmpres
    // ornot cmpres, 0, StoreVal
    BuildMI(loopMBB, DL, TII->get(Sw64::ANDr), cmpres)
        .addReg(OldVal)
        .addReg(Incr);
    BuildMI(loopMBB, DL, TII->get(Sw64::ORNOTr), StoreVal)
        .addReg(Sw64::R31)
        .addReg(cmpres);
    break;
  default:
    if (Opcode) {
      BuildMI(loopMBB, DL, TII->get(Opcode), StoreVal)
          .addReg(OldVal)
          .addReg(Incr);
    } else {
      BuildMI(loopMBB, DL, TII->get(Sw64::BISr), StoreVal)
          .addReg(Incr)
          .addReg(Incr);
    }
  }

  // lstw
  BuildMI(loopMBB, DL, TII->get(SC)).addReg(StoreVal).addImm(0).addReg(Ptr);

  if (!STI->hasCore4())
    // rd_f
    BuildMI(loopMBB, DL, TII->get(Sw64::RD_F)).addReg(StoreVal);

  // beq
  BuildMI(loopMBB, DL, TII->get(BEQ)).addReg(StoreVal).addMBB(loopMBB);

  NMBBI = BB.end();
  I->eraseFromParent(); // The instruction is gone now.

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *loopMBB);
  computeAndAddLiveIns(LiveRegs, *exitMBB);

  return true;
}

bool Sw64ExpandPseudo::expandMI(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MBBI,
                                MachineBasicBlock::iterator &NMBB) {

  bool Modified = false;

  switch (MBBI->getOpcode()) {
  case Sw64::ATOMIC_CMP_SWAP_I32_POSTRA:
    return expandAtomicCmpSwap(MBB, MBBI, NMBB, 4);
  case Sw64::ATOMIC_CMP_SWAP_I64_POSTRA:
    return expandAtomicCmpSwap(MBB, MBBI, NMBB, 8);

  case Sw64::ATOMIC_CMP_SWAP_I8_POSTRA:
  case Sw64::ATOMIC_CMP_SWAP_I16_POSTRA:
    return expandAtomicCmpSwapSubword(MBB, MBBI, NMBB);

  case Sw64::ATOMIC_SWAP_I8_POSTRA:
  case Sw64::ATOMIC_SWAP_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_ADD_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_ADD_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_SUB_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_SUB_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_AND_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_AND_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_OR_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_OR_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_XOR_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_XOR_I16_POSTRA:

  case Sw64::ATOMIC_LOAD_UMAX_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_MAX_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_UMIN_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_MIN_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_NAND_I16_POSTRA:
  case Sw64::ATOMIC_LOAD_UMAX_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_MAX_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_UMIN_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_MIN_I8_POSTRA:
  case Sw64::ATOMIC_LOAD_NAND_I8_POSTRA:
    return expandAtomicBinOpSubword(MBB, MBBI, NMBB);

  case Sw64::ATOMIC_LOAD_ADD_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_SUB_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_AND_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_OR_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_XOR_I32_POSTRA:
  case Sw64::ATOMIC_SWAP_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_UMAX_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_MAX_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_UMIN_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_MIN_I32_POSTRA:
  case Sw64::ATOMIC_LOAD_NAND_I32_POSTRA:
    return expandAtomicBinOp(MBB, MBBI, NMBB, 4);

  case Sw64::ATOMIC_LOAD_ADD_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_SUB_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_AND_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_OR_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_XOR_I64_POSTRA:
  case Sw64::ATOMIC_SWAP_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_UMAX_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_MAX_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_UMIN_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_MIN_I64_POSTRA:
  case Sw64::ATOMIC_LOAD_NAND_I64_POSTRA:
    return expandAtomicBinOp(MBB, MBBI, NMBB, 8);
  case Sw64::MOVProgPCGp:
  case Sw64::MOVaddrPCGp:
    return expandCurGpdisp(MBB, MBBI);
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
  default:
    return Modified;
  }
}

bool Sw64ExpandPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI, NMBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool Sw64ExpandPseudo::expandCurGpdisp(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MBBI) {

  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  MachineOperand addr = MI.getOperand(0);
  MachineOperand dstReg = MI.getOperand(2);

  BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDAH), Sw64::R29)
      .addGlobalAddress(addr.getGlobal(), 0, Sw64II::MO_GPDISP_HI)
      .add(dstReg);
  BuildMI(MBB, MBBI, DL, TII->get(Sw64::LDA), Sw64::R29)
      .addGlobalAddress(addr.getGlobal(), 0, Sw64II::MO_GPDISP_LO)
      .addReg(Sw64::R29);

  MI.eraseFromParent();
  return true;
}

bool Sw64ExpandPseudo::expandLoadCPAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  return expandLdihInstPair(MBB, MBBI, NextMBBI, Sw64II::MO_GPREL_HI,
                            Sw64::LDL);
}

bool Sw64ExpandPseudo::expandLoadGotAddress(
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

bool Sw64ExpandPseudo::expandLoadAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  return expandLdihInstPair(MBB, MBBI, NextMBBI, Sw64II::MO_GPREL_HI,
                            Sw64::LDA);
}

bool Sw64ExpandPseudo::expandLdihInstPair(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MBBI,
                                          MachineBasicBlock::iterator &NextMBBI,
                                          unsigned FlagsHi,
                                          unsigned SecondOpcode,
                                          unsigned FlagsLo, unsigned srcReg) {
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

bool Sw64ExpandPseudo::runOnMachineFunction(MachineFunction &MF) {
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
FunctionPass *llvm::createSw64ExpandPseudoPass() {
  return new Sw64ExpandPseudo();
}
