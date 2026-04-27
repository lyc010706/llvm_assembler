//===- Sw64MacroFusion.cpp - Sw64 Macro Fusion ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sw64 implementation of the DAG scheduling
// mutation to pair instructions back to back.
//
//===----------------------------------------------------------------------===//

#include "Sw64MacroFusion.h"
#include "Sw64Subtarget.h"
#include "llvm/CodeGen/MacroFusion.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

using namespace llvm;

// CMPxx followed by BEQ/BNE
static bool isCmpBqPair(const MachineInstr *FirstMI,
                        const MachineInstr &SecondMI) {
  if (SecondMI.getOpcode() != Sw64::BEQ && SecondMI.getOpcode() != Sw64::BNE)
    return false;

  // Assume the 1st instr to be a wildcard if it is unspecified.
  if (FirstMI == nullptr)
    return true;

  switch (FirstMI->getOpcode()) {
  case Sw64::CMPEQr:
  case Sw64::CMPEQi:
  case Sw64::CMPLTr:
  case Sw64::CMPLTi:
  case Sw64::CMPLEr:
  case Sw64::CMPLEi:
  case Sw64::CMPULTr:
  case Sw64::CMPULTi:
  case Sw64::CMPULEr:
  case Sw64::CMPULEi:
    return true;
  }

  return false;
}

// Check if the instr pair, FirstMI and SecondMI, should be fused
// together. Given SecondMI, when FirstMI is unspecified, then check if
// SecondMI may be part of a fused pair at all.
static bool shouldScheduleAdjacent(const TargetInstrInfo &TII,
                                   const TargetSubtargetInfo &TSI,
                                   const MachineInstr *FirstMI,
                                   const MachineInstr &SecondMI) {
  const Sw64Subtarget &ST = static_cast<const Sw64Subtarget &>(TSI);

  if (ST.hasCore4() && isCmpBqPair(FirstMI, SecondMI))
    return true;

  return false;
}

std::unique_ptr<ScheduleDAGMutation> llvm::createSw64MacroFusionDAGMutation() {
  return createMacroFusionDAGMutation(shouldScheduleAdjacent);
}
