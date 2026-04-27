//===-- Sw64MCInstLower.h - Lower MachineInstr to MCInst ------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_SW64MCINSTLOWER_H
#define LLVM_LIB_TARGET_SW64_SW64MCINSTLOWER_H
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class MCContext;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineFunction;
class Mangler;
class AsmPrinter;

typedef MachineOperand::MachineOperandType MachineOperandType;
// This class is used to lower an MachineInstr into an MCInst.
class LLVM_LIBRARY_VISIBILITY Sw64MCInstLower {
  MCContext *Ctx;
  AsmPrinter &Printer;

public:
  Sw64MCInstLower(class AsmPrinter &asmprinter);
  void Initialize(MCContext *C);
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
  MCOperand LowerOperand(const MachineOperand &MO, unsigned offset = 0) const;

  void lowerMemory(const MachineInstr *MI, MCInst &OutMI) const;

private:
  MCOperand LowerSymbolOperand(const MachineOperand &MO,
                               MachineOperandType MOTy, unsigned Offset) const;
};
} // namespace llvm

#endif
