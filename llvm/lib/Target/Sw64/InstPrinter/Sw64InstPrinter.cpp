//===-- Sw64InstPrinter.cpp - Convert Sw64 MCInst to assembly syntax ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an Sw64 MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "Sw64InstPrinter.h"
#include "MCTargetDesc/Sw64BaseInfo.h"
#include "MCTargetDesc/Sw64MCExpr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#include "Sw64GenAsmWriter.inc"

void Sw64InstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) const {
  OS << StringRef(getRegisterName(Reg)).lower();
}

void Sw64InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &OS) {
  printInstruction(MI, Address, OS);
  if (!Annot.empty()) {
    OS << "\t" << Annot;
  } else
    printAnnotation(OS, Annot);
}

void Sw64InstPrinter::printInlineJT(const MCInst *MI, int opNum,
                                    raw_ostream &O) {
  report_fatal_error("can't handle InlineJT");
}

void Sw64InstPrinter::printInlineJT32(const MCInst *MI, int opNum,
                                      raw_ostream &O) {
  report_fatal_error("can't handle InlineJT32");
}

void Sw64InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {

  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
    if (Op.getImm() > 65535) {
      O << formatHex(Op.getImm());
      return;
    }
    O << Op.getImm();
    return;
  }

  assert(Op.isExpr() && "unknown operand kind in printOperand");
  Op.getExpr()->print(O, &MAI, true);
}

void Sw64InstPrinter::printMemoryArg(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);

  if (Op.isExpr()) {
    const MCExpr *Expr = Op.getExpr();
    if (Expr->getKind() == MCExpr::Target) {
      const Sw64MCExpr *Sw64Expr = cast<Sw64MCExpr>(Expr);

      switch (Sw64Expr->getKind()) {
      default:
        break;
      case Sw64MCExpr::MEK_GPDISP_HI16:
      case Sw64MCExpr::MEK_GPDISP_LO16:
      case Sw64MCExpr::MEK_GPDISP:
        O << "0";
        return;
      }
    }
  }
  printOperand(MI, OpNo, O);
}

void Sw64InstPrinter::printMemOperand(const MCInst *MI, int opNum,
                                      raw_ostream &O) {
  // Load/Store memory operands -- imm($reg)

  if (MI->getOperand(opNum).isImm() && MI->getOperand(opNum + 1).isReg()) {
    printOperand(MI, opNum, O);
    O << "(";
    printOperand(MI, opNum + 1, O);
    O << ")";
  } else {
    printOperand(MI, opNum + 1, O);
    O << "(";
    printOperand(MI, opNum, O);
    O << ")";
  }
}

template <unsigned Bits, unsigned Offset>
void Sw64InstPrinter::printUImm(const MCInst *MI, int opNum, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm()) {
    uint64_t Imm = MO.getImm();
    Imm -= Offset;
    Imm &= (1 << Bits) - 1;
    Imm += Offset;
    if (MI->getOpcode() == Sw64::VLOGZZ)
      O << format("%x", Imm);
    else
      O << formatImm(Imm);
    return;
  }

  printOperand(MI, opNum, O);
}

// Only for Instruction VLOG
void Sw64InstPrinter::printHexImm(const MCInst *MI, int opNum, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm()) {
    uint64_t Imm = MO.getImm();
    O << format("%x", ((Imm >> 4) & 0xf)) << format("%x", (Imm & 0xf));
    return;
  }

  printOperand(MI, opNum, O);
}
