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

#define DEBUG_TYPE "sw_64-ieee-contrain"

using namespace llvm;

namespace llvm {

struct Sw64IEEEConstraint : public MachineFunctionPass {
  /// Target machine description which we query for reg. names, data
  /// layout, etc.
  static char ID;
  Sw64IEEEConstraint() : MachineFunctionPass(ID) {}

  StringRef getPassName() const { return "Sw64 Add IEEE Contrain"; }

  bool runOnMachineFunction(MachineFunction &F);
};
char Sw64IEEEConstraint::ID = 0;
} // end namespace llvm

static bool isNeedIEEEConstraint(unsigned opcode) {
  switch (opcode) {
  case Sw64::ADDS:
  case Sw64::SUBS:
  case Sw64::MULS:
  case Sw64::DIVS:
  case Sw64::FMAS:
  case Sw64::FMSS:
  case Sw64::FNMAS:
  case Sw64::FNMSS:
  case Sw64::ADDD:
  case Sw64::SUBD:
  case Sw64::MULD:
  case Sw64::DIVD:
  case Sw64::FMAD:
  case Sw64::FMSD:
  case Sw64::FNMAD:
  case Sw64::FNMSD:
  case Sw64::CVTQS:
  case Sw64::CVTQT:
  case Sw64::CVTTQ:
  case Sw64::CVTTS:
  case Sw64::CVTST:
  case Sw64::FCVTWL:
  case Sw64::FCVTLW:
  case Sw64::VADDS:
  case Sw64::VADDD:
  case Sw64::VSUBS:
  case Sw64::VSUBD:
  case Sw64::VMULS:
  case Sw64::VMULD:
  case Sw64::VDIVS:
  case Sw64::VDIVD:
  case Sw64::VSQRTS:
  case Sw64::VSQRTD:
  case Sw64::SQRTSS:
  case Sw64::SQRTSD:
  case Sw64::CMPTEQ:
  case Sw64::CMPTLE:
  case Sw64::CMPTLT:
  case Sw64::CMPTUN:
  case Sw64::VFCMPEQ:
  case Sw64::VFCMPLE:
  case Sw64::VFCMPLT:
  case Sw64::VFCMPUN:
  case Sw64::VMAS:
  case Sw64::VMAD:
  case Sw64::VMSS:
  case Sw64::VMSD:
  case Sw64::VNMAS:
  case Sw64::VNMAD:
  case Sw64::VNMSS:
  case Sw64::VNMSD:
  case Sw64::FSELEQS:
  case Sw64::FSELNES:
  case Sw64::FSELLTS:
  case Sw64::FSELLES:
  case Sw64::FSELGTS:
  case Sw64::FSELGES:
  case Sw64::FSELEQD:
  case Sw64::FSELNED:
  case Sw64::FSELLTD:
  case Sw64::FSELLED:
  case Sw64::FSELGTD:
  case Sw64::FSELGED:
  case Sw64::FCTTDL_G:
  case Sw64::FCTTDL_P:
  case Sw64::FCTTDL_N:
  case Sw64::FCTTDL:
    return true;
  }
  return false;
}

bool Sw64IEEEConstraint::runOnMachineFunction(MachineFunction &F) {
  const Sw64Subtarget &ST = F.getSubtarget<Sw64Subtarget>();
  if (ST.hasCore4())
    return false;

  for (MachineFunction::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
    MachineBasicBlock &MBB = *FI;
    MachineBasicBlock::iterator MBBI = MBB.begin();
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    NMBBI++;
    for (; MBBI != MBB.end(); MBBI++) {
      if (isNeedIEEEConstraint(MBBI->getOpcode())) {
        MachineOperand &MO = MBBI->getOperand(0);
        if (MO.isEarlyClobber()) {
          LLVM_DEBUG(dbgs() << "getting is EarlyClobber Flag"
                            << MO.isEarlyClobber() << "\n";
                     MBBI->dump());
          continue;
        }

        MO.setIsEarlyClobber();
        LLVM_DEBUG(dbgs() << "setting is EarlyClobber Flag"
                          << MBBI->getOperand(0).isEarlyClobber() << "\n";
                   MBBI->dump());
      }
    }
  }
  return true;
}

FunctionPass *llvm::createSw64IEEEConstraintPass() {
  return new Sw64IEEEConstraint();
}
