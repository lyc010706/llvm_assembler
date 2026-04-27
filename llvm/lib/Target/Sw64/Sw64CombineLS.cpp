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

#define DEBUG_TYPE "sw_64-combineLS"

using namespace llvm;

namespace llvm {

struct Sw64CombineLS : public MachineFunctionPass {
  /// Target machine description which we query for reg. names, data
  /// layout, etc.
  static char ID;
  Sw64CombineLS() : MachineFunctionPass(ID) {}

  StringRef getPassName() const { return "Sw64 Combine Load Store insn"; }

  bool runOnMachineFunction(MachineFunction &F) {
    for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE;
         ++FI) {
      MachineBasicBlock &MBB = *FI;
      MachineBasicBlock::iterator MBBI = MBB.begin();
      MachineBasicBlock::iterator NMBBI = std::next(MBBI);
      NMBBI++;
      for (; NMBBI != MBB.end(); MBBI++, NMBBI++) {

        MachineInstr &MI = *MBBI, &NMI = *NMBBI;
        DebugLoc DL = MI.getDebugLoc();
        const MCInstrDesc &MCID = NMI.getDesc();

        if (MI.getOpcode() == Sw64::LDA &&
            (MCID.mayLoad() || MCID.mayStore())) {
          LLVM_DEBUG(dbgs() << "combining Load/Store instr\n"; MI.dump();
                     dbgs() << "\n"; NMI.dump(); dbgs() << "\n");

          if (MI.getOperand(0).getReg() == NMI.getOperand(2).getReg() &&
              NMI.getOperand(2).getReg() != Sw64::R30) {
            BuildMI(MBB, MBBI, DL, MCID)
                .add(NMI.getOperand(0))
                .add(MI.getOperand(1))
                .add(MI.getOperand(0));
            NMI.eraseFromParent();
            MI.eraseFromParent();
          }
        }
      }
    }
    return true;
  }
};
char Sw64CombineLS::ID = 0;
} // end namespace llvm

FunctionPass *llvm::createSw64CombineLSPass() { return new Sw64CombineLS(); }
