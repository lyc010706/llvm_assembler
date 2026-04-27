//===-- Sw64LLRP.cpp - Sw64 Load Load Replay Trap elimination pass. -- --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Here we check for potential replay traps introduced by the spiller
// We also align some branch targets if we can do so for free.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sw_64-nops"
#include "MCTargetDesc/Sw64BaseInfo.h"
#include "Sw64.h"
#include "Sw64FrameLowering.h"
#include "Sw64Subtarget.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;
cl::opt<bool> Sw64Mieee("mieee", cl::desc("Support the IEEE754"),
                        cl::init(true));

cl::opt<bool> Sw64DeleteNop("sw64-delete-nop", cl::desc("Delete NOP"),
                            cl::init(true));

STATISTIC(nopintro, "Number of nops inserted");
STATISTIC(nopalign, "Number of nops inserted for alignment");
namespace llvm {
cl::opt<bool> AlignAll("sw_64-align-all", cl::Hidden,
                       cl::desc("Align all blocks"));

struct Sw64LLRPPass : public MachineFunctionPass {
  // Target machine description which we query for reg. names, data
  // layout, etc.
  //
  Sw64TargetMachine &TM;

  static char ID;
  Sw64LLRPPass(Sw64TargetMachine &tm) : MachineFunctionPass(ID), TM(tm) {}

  StringRef getPassName() const { return "Sw64 NOP inserter"; }

  bool runOnMachineFunction(MachineFunction &F) {
    const TargetInstrInfo *TII = F.getSubtarget().getInstrInfo();
    bool flag = false; // hasJSR ?
    bool Changed = false;
    MachineInstr *prev[3] = {0, 0, 0};
    unsigned count = 0;

    DebugLoc dl;
    const Sw64Subtarget &Subtarget = F.getSubtarget<Sw64Subtarget>();
    int curgpdist = Subtarget.getCurgpdist();

    SmallVector<MachineInstr *, 4> Ops;
    for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE;
         ++FI) {
      MachineBasicBlock &MBB = *FI;
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr *MI = &*MII;
        ++MII;
        if (MII == MIE)
          break;
        MachineInstr *MINext = &*MII;
        if (MINext->getOpcode() == Sw64::FILLCS ||
            MINext->getOpcode() == Sw64::FILLDE) {
          if (MI->getOpcode() == Sw64::LDA &&
              (MI->getOperand(1).getImm() == MINext->getOperand(0).getImm())) {
            bool isRead = false;
            for (MachineBasicBlock::iterator M1 = MII; M1 != MIE;) {
              MachineInstr *Mtest = &*M1;
              if (Mtest->getOpcode() == Sw64::LDA ||
                  Mtest->getOpcode() == Sw64::LDAH ||
                  Mtest->getOpcode() == Sw64::LDL ||
                  Mtest->getOpcode() == Sw64::LDW ||
                  Mtest->getOpcode() == Sw64::LDHU ||
                  Mtest->getOpcode() == Sw64::LDBU) {
                if (Mtest->getOperand(0).getReg() ==
                        MI->getOperand(0).getReg() &&
                    !isRead) {
                  Ops.push_back(MI);
                  break;
                }
              }
              if (Mtest->getOpcode() == Sw64::STL ||
                  Mtest->getOpcode() == Sw64::STW ||
                  Mtest->getOpcode() == Sw64::STH ||
                  Mtest->getOpcode() == Sw64::STB) {
                if (Mtest->getOperand(2).getReg() ==
                        MI->getOperand(0).getReg() ||
                    Mtest->getOperand(0).getReg() ==
                        MI->getOperand(0).getReg()) {
                  isRead = true;
                }
              }
              ++M1;
            }
          }
        }
      }
      for (auto *PrefMI : Ops)
        PrefMI->eraseFromParent();
      Ops.clear();
    }

    // Remove all duplicate prefetch instr
    SmallVector<MachineInstr *, 12> FILL;
    int Dul;
    for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE;
         ++FI) {
      MachineBasicBlock &MBB = *FI;
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr *MI = &*MII;
        ++MII;
        Dul = 1;
        if (MII == MIE)
          break;
        if (MI->getOpcode() == Sw64::FILLCS ||
            MI->getOpcode() == Sw64::FILLCS_E ||
            MI->getOpcode() == Sw64::FILLDE ||
            MI->getOpcode() == Sw64::FILLDE_E ||
            MI->getOpcode() == Sw64::S_FILLDE ||
            MI->getOpcode() == Sw64::S_FILLCS) {
          if (!FILL.empty()) {
            for (auto *PrefMI : FILL) {
              if (PrefMI->getOperand(1).getReg() ==
                  MI->getOperand(1).getReg()) {
                Dul = 2;
                break;
              }
            }
          }
          if (Dul == 1) {
            for (MachineBasicBlock::iterator M1 = MII; M1 != MIE;) {
              MachineInstr *Mtest = &*M1;
              if (Mtest->getOpcode() == Sw64::FILLCS ||
                  Mtest->getOpcode() == Sw64::FILLCS_E ||
                  Mtest->getOpcode() == Sw64::FILLDE ||
                  Mtest->getOpcode() == Sw64::FILLDE_E ||
                  Mtest->getOpcode() == Sw64::S_FILLCS ||
                  Mtest->getOpcode() == Sw64::S_FILLDE) {
                if (Mtest->getOperand(1).getReg() ==
                    MI->getOperand(1).getReg()) {
                  FILL.push_back(Mtest);
                }
              }
              ++M1;
            }
          }
        }
      }
      if (!FILL.empty()) {
        for (auto *PrefMI1 : FILL)
          PrefMI1->eraseFromParent();
      }
      FILL.clear();
    }

    // If read and write, use fillde
    int N = 0;
    for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE;) {
      MachineBasicBlock &MBB = *FI;
      ++FI;
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr *MI = &*MII;
        ++MII;
        if (MII == MIE)
          break;
        if (MI->getOpcode() == Sw64::FILLCS ||
            MI->getOpcode() == Sw64::S_FILLCS) {
          for (MachineBasicBlock::iterator M1 = MII; M1 != MIE;) {
            MachineInstr *Mtest = &*M1;
            if (Mtest->getOpcode() == Sw64::LDA ||
                Mtest->getOpcode() == Sw64::LDAH ||
                Mtest->getOpcode() == Sw64::LDL ||
                Mtest->getOpcode() == Sw64::LDW ||
                Mtest->getOpcode() == Sw64::LDHU ||
                Mtest->getOpcode() == Sw64::LDBU) {
              if (Mtest->getOperand(0).getReg() == MI->getOperand(1).getReg()) {
                N = 1;
              }
            }
            ++M1;
          }
          if (FI == FE)
            break;
          MachineBasicBlock &MBB1 = *FI;
          for (MachineBasicBlock::iterator MII1 = MBB1.begin(),
                                           MIE1 = MBB1.end();
               MII1 != MIE1;) {
            MachineInstr *MI1 = &*MII1;
            if (MI1->getOpcode() == Sw64::STL ||
                MI1->getOpcode() == Sw64::STW ||
                MI1->getOpcode() == Sw64::STB ||
                MI1->getOpcode() == Sw64::STH) {
              if (MI1->getOperand(2).getReg() == MI->getOperand(1).getReg() &&
                  N == 0) {
                if (MI->getOpcode() == Sw64::FILLCS)
                  MI->setDesc(TII->get(Sw64::FILLDE));
                if (MI->getOpcode() == Sw64::S_FILLCS)
                  MI->setDesc(TII->get(Sw64::S_FILLDE));
                N = 0;
              }
            }
            ++MII1;
          }
        }
      }
    }

    const TargetRegisterInfo *TRI = F.getSubtarget().getRegisterInfo();
    for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE;
         ++FI) {
      MachineBasicBlock &MBB = *FI;
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr *MI = &*MII;
        ++MII;
        if (MII == MIE)
          break;
        if (MI->getOpcode() == Sw64::FILLCS ||
            MI->getOpcode() == Sw64::FILLDE) {
          int N = 0;
          int isDul = 0;
          for (MachineBasicBlock::iterator MIT = MII; MIT != MIE;) {
            MachineInstr *MITT = &*MIT;
            if (MITT->readsRegister(MI->getOperand(1).getReg(), TRI)) {
              N++;
            }
            if (MITT->getOpcode() == Sw64::FILLCS ||
                MITT->getOpcode() == Sw64::FILLDE ||
                MITT->getOpcode() == Sw64::FILLCS_E ||
                MITT->getOpcode() == Sw64::FILLDE_E)
              isDul++;
            ++MIT;
          }
          if (N == 1 && isDul > 0) {
            if (MI->getOpcode() == Sw64::FILLCS)
              MI->setDesc(TII->get(Sw64::FILLCS_E));
            if (MI->getOpcode() == Sw64::FILLDE) {
              MI->setDesc(TII->get(Sw64::FILLDE_E));
            }
          }
        }
      }
    }

    for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE;
         ++FI) {
      MachineBasicBlock &MBB = *FI;
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr *MI = &*MII;
        if (MI->getOpcode() == Sw64::FILLCS ||
            MI->getOpcode() == Sw64::S_FILLCS) {
          for (MachineBasicBlock::iterator M1 = MII; M1 != MIE;) {
            MachineInstr *Mtest = &*M1;
            if (Mtest->getOpcode() == Sw64::STL ||
                Mtest->getOpcode() == Sw64::STW ||
                Mtest->getOpcode() == Sw64::STH ||
                Mtest->getOpcode() == Sw64::STB) {
              if (Mtest->getOperand(2).getReg() == MI->getOperand(1).getReg()) {
                if (MI->getOpcode() == Sw64::FILLCS)
                  MI->setDesc(TII->get(Sw64::FILLDE));
                if (MI->getOpcode() == Sw64::S_FILLCS)
                  MI->setDesc(TII->get(Sw64::S_FILLDE));
              }
            }
            ++M1;
          }
        }
        ++MII;
      }
    }

    for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE;
         ++FI) {
      MachineBasicBlock &MBB = *FI;

      int count = 0;
      bool isLable = 0;
      if (MBB.getBasicBlock() && MBB.getBasicBlock()->isLandingPad()) {
        MachineBasicBlock::iterator MBBI = MBB.begin();
        for (MBBI = MBB.begin(); MBBI != MBB.end(); ++MBBI, ++count) {
          if (count == 0 && MBBI->isLabel())
            isLable = true;
          if (count == 1 && isLable) {
            BuildMI(MBB, MBBI, dl, TII->get(Sw64::MOVaddrPCGp))
                .addGlobalAddress(&(F.getFunction()))
                .addImm(++curgpdist)
                .addReg(Sw64::R26);
            isLable = false;
          }
        }
        if (count == 1 && isLable) {
          BuildMI(MBB, MBBI, dl, TII->get(Sw64::MOVaddrPCGp))
              .addGlobalAddress(&(F.getFunction()))
              .addImm(++curgpdist)
              .addReg(Sw64::R26);
          isLable = false;
        }
      }

      MachineBasicBlock::iterator I;
      for (I = MBB.begin(); I != MBB.end(); ++I) {
        if (flag) {
          BuildMI(MBB, I, dl, TII->get(Sw64::MOVaddrPCGp))
              .addGlobalAddress(&(F.getFunction()))
              .addImm(++curgpdist)
              .addReg(Sw64::R26);
          if (Sw64Mieee) {
            if (!Sw64DeleteNop)
              BuildMI(MBB, I, dl, TII->get(Sw64::NOP));
          }
          flag = false;
        }
        if (I->getOpcode() == Sw64::JSR ||
            I->getOpcode() == Sw64::PseudoCallIndirect) {
          dl = MBB.findDebugLoc(I);
          if (Sw64Mieee) {
            if (!Sw64DeleteNop)
              BuildMI(MBB, I, dl, TII->get(Sw64::NOP));
          }
          flag = true;
        }
      }
      if (flag) {
        BuildMI(MBB, I, dl, TII->get(Sw64::MOVaddrPCGp))
            .addGlobalAddress(&(F.getFunction()))
            .addImm(++curgpdist)
            .addReg(Sw64::R26);
        if (Sw64Mieee) {
          if (!Sw64DeleteNop)
            BuildMI(MBB, I, dl, TII->get(Sw64::NOP));
        }
        flag = false;
      }
    }

    if (!Sw64DeleteNop) {
      for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE;
           ++FI) {
        MachineBasicBlock &MBB = *FI;
        bool ub = false;
        for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end();) {
          if (count % 4 == 0)
            prev[0] = prev[1] = prev[2] = 0; // Slots cleared at fetch boundary
          ++count;
          MachineInstr *MI = &(*I);
          I++;
          switch (MI->getOpcode()) {
          case Sw64::LDL:
          case Sw64::LDW:
          case Sw64::LDHU:
          case Sw64::LDBU:
          case Sw64::LDD:
          case Sw64::LDS:
          case Sw64::STL:
          case Sw64::STW:
          case Sw64::STH:
          case Sw64::STB:
          case Sw64::STD:
          case Sw64::STS:
            dl = MBB.findDebugLoc(MI);
            if (MI->getOperand(2).getReg() == Sw64::R30) {
              if (prev[0] &&
                  prev[0]->getOperand(2).getReg() ==
                      MI->getOperand(2).getReg() &&
                  prev[0]->getOperand(1).getImm() ==
                      MI->getOperand(1).getImm()) {
                prev[0] = prev[1];
                prev[1] = prev[2];
                prev[2] = 0;
                BuildMI(MBB, MI, dl, TII->get(Sw64::BISr), Sw64::R31)
                    .addReg(Sw64::R31)
                    .addReg(Sw64::R31);
                Changed = true;
                nopintro += 1;
                count += 1;
              } else if (prev[1] &&
                         prev[1]->getOperand(2).getReg() ==
                             MI->getOperand(2).getReg() &&
                         prev[1]->getOperand(1).getImm() ==
                             MI->getOperand(1).getImm()) {
                prev[0] = prev[2];
                prev[1] = prev[2] = 0;
                BuildMI(MBB, MI, dl, TII->get(Sw64::BISr), Sw64::R31)
                    .addReg(Sw64::R31)
                    .addReg(Sw64::R31);
                BuildMI(MBB, MI, dl, TII->get(Sw64::BISr), Sw64::R31)
                    .addReg(Sw64::R31)
                    .addReg(Sw64::R31);
                Changed = true;
                nopintro += 2;
                count += 2;
              } else if (prev[2] &&
                         prev[2]->getOperand(2).getReg() ==
                             MI->getOperand(2).getReg() &&
                         prev[2]->getOperand(1).getImm() ==
                             MI->getOperand(1).getImm()) {
                prev[0] = prev[1] = prev[2] = 0;
                BuildMI(MBB, MI, dl, TII->get(Sw64::BISr), Sw64::R31)
                    .addReg(Sw64::R31)
                    .addReg(Sw64::R31);
                BuildMI(MBB, MI, dl, TII->get(Sw64::BISr), Sw64::R31)
                    .addReg(Sw64::R31)
                    .addReg(Sw64::R31);
                BuildMI(MBB, MI, dl, TII->get(Sw64::BISr), Sw64::R31)
                    .addReg(Sw64::R31)
                    .addReg(Sw64::R31);
                Changed = true;
                nopintro += 3;
                count += 3;
              }
              prev[0] = prev[1];
              prev[1] = prev[2];
              prev[2] = MI;
              break;
            }
            prev[0] = prev[1];
            prev[1] = prev[2];
            prev[2] = 0;
            break;
          case Sw64::ALTENT:
          case Sw64::MEMLABEL:
          case Sw64::PCLABEL:
            --count;
            break;
          case Sw64::BR:
          case Sw64::PseudoBR:
          case Sw64::JMP:
            ub = true;
          // fall through
          default:
            prev[0] = prev[1];
            prev[1] = prev[2];
            prev[2] = 0;
            break;
          }
        }
        if (ub || AlignAll) {
          // we can align stuff for free at this point
          while (count % 4) {
            BuildMI(MBB, MBB.end(), dl, TII->get(Sw64::BISr), Sw64::R31)
                .addReg(Sw64::R31)
                .addReg(Sw64::R31);
            ++count;
            ++nopalign;
            prev[0] = prev[1];
            prev[1] = prev[2];
            prev[2] = 0;
          }
        }
      }
    }
    return Changed;
  }
};
char Sw64LLRPPass::ID = 0;
} // namespace llvm

FunctionPass *llvm::createSw64LLRPPass(Sw64TargetMachine &tm) {
  return new Sw64LLRPPass(tm);
}
