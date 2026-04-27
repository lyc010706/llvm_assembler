//===-- Sw64.h - Top-level interface for Sw64 representation --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// Sw64 back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_SW64_H
#define LLVM_LIB_TARGET_SW64_SW64_H

#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
namespace Sw64 {
// These describe LDAx
static const int IMM_LOW = -32768;
static const int IMM_HIGH = 32767;
static const int IMM_MULT = 65536;
} // namespace Sw64

class FunctionPass;
class ModulePass;
class TargetMachine;
class Sw64TargetMachine;
class formatted_raw_ostream;

FunctionPass *createSw64ISelDag(Sw64TargetMachine &TM,
                                CodeGenOpt::Level OptLevel);

FunctionPass *createSw64LLRPPass(Sw64TargetMachine &tm);
FunctionPass *createSw64BranchSelectionPass();
FunctionPass *createSw64BranchSelection();
FunctionPass *createSw64PreLegalizeCombiner(); // for fmad
FunctionPass *createSw64ExpandPseudoPass();
FunctionPass *createSw64ExpandPseudo2Pass();
FunctionPass *createSw64CombineLSPass();
FunctionPass *createSw64IEEEConstraintPass();

bool LowerSw64MachineOperandToMCOperand(const MachineOperand &MO,
                                        MCOperand &MCOp, const AsmPrinter &AP);

void initializeSw64BranchSelectionPass(PassRegistry &);
void initializeSw64PreLegalizerCombinerPass(PassRegistry &); // for fmad
void initializeSw64DAGToDAGISelPass(PassRegistry &);
} // namespace llvm

#endif
