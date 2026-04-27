//===-- Sw64MCInstLower.cpp - Convert Sw64 MachineInstr to MCInst -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower Sw64 MachineInstrs to their
// corresponding MCInst records.
//
//===----------------------------------------------------------------------===//
#include "Sw64MCInstLower.h"
#include "MCTargetDesc/Sw64BaseInfo.h"
#include "MCTargetDesc/Sw64MCExpr.h"
#include "Sw64.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"

using namespace llvm;

#include "Sw64GenInstrInfo.inc"

namespace llvm {
struct Sw64InstrTable {
  MCInstrDesc Insts[1000];
};
extern const Sw64InstrTable Sw64Descs;
} // namespace llvm

Sw64MCInstLower::Sw64MCInstLower(class AsmPrinter &asmprinter)
    : Printer(asmprinter) {}

void Sw64MCInstLower::Initialize(MCContext *C) { Ctx = C; }

static bool lowerLitUseMOp(const MachineOperand &MO,
                           Sw64MCExpr::Sw64ExprKind &Kind) {
  Sw64MCExpr::Sw64ExprKind TargetKind = Sw64MCExpr::MEK_None;
  unsigned flags = MO.getTargetFlags();
  if (flags & Sw64II::MO_LITERAL && flags & Sw64II::MO_LITERAL_BASE) {
    TargetKind = Sw64MCExpr::MEK_LITUSE_BASE;
  } else if (flags & Sw64II::MO_HINT && flags & Sw64II::MO_LITUSE) {
    TargetKind = Sw64MCExpr::MEK_LITUSE_JSRDIRECT;
  } else
    return false;

  Kind = TargetKind;
  return true;
}

MCOperand Sw64MCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MachineOperandType MOTy,
                                              unsigned Offset) const {
  MCSymbolRefExpr::VariantKind Kind = MCSymbolRefExpr::VK_None;
  Sw64MCExpr::Sw64ExprKind TargetKind = Sw64MCExpr::MEK_None;
  const MCSymbol *Symbol;

  switch (MO.getTargetFlags()) {
  default:
    if (lowerLitUseMOp(MO, TargetKind))
      break;
    llvm_unreachable("Invalid target flag!");
  case Sw64II::MO_NO_FLAG:
    TargetKind = Sw64MCExpr::MEK_None;
    break;
  case Sw64II::MO_GPDISP_HI:
    TargetKind = Sw64MCExpr::MEK_GPDISP_HI16;
    break;
  case Sw64II::MO_GPDISP_LO:
    TargetKind = Sw64MCExpr::MEK_GPDISP_LO16;
    break;
  case Sw64II::MO_GPREL_HI:
    TargetKind = Sw64MCExpr::MEK_GPREL_HI16;
    break;
  case Sw64II::MO_GPREL_LO:
    TargetKind = Sw64MCExpr::MEK_GPREL_LO16;
    break;
  case Sw64II::MO_ABS_LO:
  case Sw64II::MO_LITERAL:
    TargetKind = Sw64MCExpr::MEK_ELF_LITERAL;
    break;
  case Sw64II::MO_LITERAL_GOT:
    TargetKind = Sw64MCExpr::MEK_ELF_LITERAL_GOT;
    break;
  case Sw64II::MO_TPREL_HI:
    TargetKind = Sw64MCExpr::MEK_TPREL_HI16;
    break;
  case Sw64II::MO_TPREL_LO:
    TargetKind = Sw64MCExpr::MEK_TPREL_LO16;
    break;
  case Sw64II::MO_TLSGD:
    TargetKind = Sw64MCExpr::MEK_TLSGD;
    break;
  case Sw64II::MO_TLSLDM:
    TargetKind = Sw64MCExpr::MEK_TLSLDM;
    break;
  case Sw64II::MO_GOTTPREL:
    TargetKind = Sw64MCExpr::MEK_GOTTPREL16;
    break;
  case Sw64II::MO_DTPREL_HI:
    TargetKind = Sw64MCExpr::MEK_DTPREL_HI16;
    break;
  case Sw64II::MO_DTPREL_LO:
    TargetKind = Sw64MCExpr::MEK_DTPREL_LO16;
    break;
  case Sw64II::MO_HINT:
    TargetKind = Sw64MCExpr::MEK_HINT;
  }

  switch (MOTy) {
  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;
  case MachineOperand::MO_GlobalAddress:
    Symbol = Printer.getSymbol(MO.getGlobal());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_BlockAddress:
    Symbol = Printer.GetBlockAddressSymbol(MO.getBlockAddress());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_ExternalSymbol:
    Symbol = Printer.GetExternalSymbolSymbol(MO.getSymbolName());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_JumpTableIndex:
    Symbol = Printer.GetJTISymbol(MO.getIndex());
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    Symbol = Printer.GetCPISymbol(MO.getIndex());
    Offset += MO.getOffset();
    break;
  default:
    llvm_unreachable("<unknown operand type>");
  }

  const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, Kind, *Ctx);

  if (Offset) {
    // Assume offset is never negative.
    assert(Offset > 0);

    Expr = MCBinaryExpr::createAdd(Expr, MCConstantExpr::create(Offset, *Ctx),
                                   *Ctx);
  }

  if (TargetKind != Sw64MCExpr::MEK_None)
    Expr = Sw64MCExpr::create(TargetKind, Expr, *Ctx);

  return MCOperand::createExpr(Expr);
}

MCOperand Sw64MCInstLower::LowerOperand(const MachineOperand &MO,
                                        unsigned offset) const {
  MachineOperandType MOTy = MO.getType();

  switch (MOTy) {
  default:
    llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      break;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm() + offset);
  case MachineOperand::MO_MachineBasicBlock:
  case MachineOperand::MO_GlobalAddress:
  case MachineOperand::MO_ExternalSymbol:
  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_ConstantPoolIndex:
  case MachineOperand::MO_BlockAddress:
    return LowerSymbolOperand(MO, MOTy, offset);
  case MachineOperand::MO_RegisterMask:
    break;
  }

  return MCOperand();
}

void Sw64MCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    MCOperand MCOp = LowerOperand(MO);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}

static MCOperand lowerSymbolOperand(const MachineOperand &MO,
                                    MachineOperandType MOTy, unsigned Offset,
                                    const AsmPrinter &AP) {
  MCSymbolRefExpr::VariantKind Kind = MCSymbolRefExpr::VK_None;
  Sw64MCExpr::Sw64ExprKind TargetKind = Sw64MCExpr::MEK_None;
  const MCSymbol *Symbol;
  MCContext &Ctx = AP.OutContext;

  switch (MOTy) {
  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;
  case MachineOperand::MO_GlobalAddress:
    Symbol = AP.getSymbol(MO.getGlobal());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_BlockAddress:
    Symbol = AP.GetBlockAddressSymbol(MO.getBlockAddress());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_ExternalSymbol:
    Symbol = AP.GetExternalSymbolSymbol(MO.getSymbolName());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_JumpTableIndex:
    Symbol = AP.GetJTISymbol(MO.getIndex());
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    Symbol = AP.GetCPISymbol(MO.getIndex());
    Offset += MO.getOffset();
    break;
  default:
    llvm_unreachable("<unknown operand type>");
  }

  const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, Kind, Ctx);

  if (Offset) {
    // Assume offset is never negative.
    assert(Offset > 0);

    Expr =
        MCBinaryExpr::createAdd(Expr, MCConstantExpr::create(Offset, Ctx), Ctx);
  }

  if (TargetKind != Sw64MCExpr::MEK_None)
    Expr = Sw64MCExpr::create(TargetKind, Expr, Ctx);

  return MCOperand::createExpr(Expr);
}

bool llvm::LowerSw64MachineOperandToMCOperand(const MachineOperand &MO,
                                              MCOperand &MCOp,
                                              const AsmPrinter &AP) {
  switch (MO.getType()) {
  default:
    report_fatal_error("LowerSw64MachineInstrToMCInst: unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
    break;
  case MachineOperand::MO_RegisterMask:
    // Regmasks are like implicit defs.
    return false;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    break;
    return false;
  case MachineOperand::MO_MachineBasicBlock:
  case MachineOperand::MO_GlobalAddress:
  case MachineOperand::MO_ExternalSymbol:
  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_ConstantPoolIndex:
  case MachineOperand::MO_BlockAddress:
    MCOp = lowerSymbolOperand(MO, MO.getType(), 0, AP);
    return false;
  }
  return true;
}
