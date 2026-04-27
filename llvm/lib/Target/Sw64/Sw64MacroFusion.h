//===- Sw64MacroFusion.h - Sw64 Macro Fusion ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sw64 definition of the DAG scheduling
// mutation to pair instructions back to back.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_SW64MACROFUSION_H
#define LLVM_LIB_TARGET_SW64_SW64MACROFUSION_H

#include "llvm/CodeGen/MachineScheduler.h"

namespace llvm {

// Note that you have to add:
// DAG.addMutation(createSw64MacroFusionDAGMutation());
// to Sw64PassConfig::createMachineScheduler() to have an effect.
std::unique_ptr<ScheduleDAGMutation> createSw64MacroFusionDAGMutation();

} // namespace llvm

#endif // LLVM_LIB_TARGET_SW64_SW64MACROFUSION_H
