//===-- Sw64BranchSelector.cpp - Convert Pseudo branchs ----------*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Replace Pseudo COND_BRANCH_* with their appropriate real branch
// Simplified version of the PPC Branch Selector
//
//===----------------------------------------------------------------------===//

#include "Sw64.h"
#include "Sw64InstrInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetMachine.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <utility>

using namespace llvm;

#define DEBUG_TYPE "sw_64-branch-expansion"

namespace {
class Sw64BranchSelection : public MachineFunctionPass {
public:
  static char ID;

  Sw64BranchSelection() : MachineFunctionPass(ID) {
    initializeSw64BranchSelectionPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Sw64 Branch Expansion Pass";
  }

  bool runOnMachineFunction(MachineFunction &F) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoVRegs);
  }
};
} // end of anonymous namespace

char Sw64BranchSelection::ID = 0;

INITIALIZE_PASS(Sw64BranchSelection, DEBUG_TYPE,
                "Expand out of range branch instructions and fix forbidden"
                " slot hazards",
                false, false)

/// Returns a pass that clears pipeline hazards.
FunctionPass *llvm::createSw64BranchSelection() {
  return new Sw64BranchSelection();
}

bool Sw64BranchSelection::runOnMachineFunction(MachineFunction &F) {

  return true;
}
