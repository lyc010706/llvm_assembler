//===-- Sw64MCExpr.cpp - Sw64 specific MC expression classes --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Sw64MCExpr.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "sw64mcexpr"

const Sw64MCExpr *Sw64MCExpr::create(Sw64MCExpr::Sw64ExprKind Kind,
                                     const MCExpr *Expr, MCContext &Ctx) {
  return new (Ctx) Sw64MCExpr(Kind, Expr);
}

void Sw64MCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  int64_t AbsVal;
  // FIXME: the end "(" need match
  if (Expr->evaluateAsAbsolute(AbsVal))
    OS << AbsVal;
  else
    Expr->print(OS, MAI, true);
}

bool Sw64MCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                           const MCAsmLayout *Layout,
                                           const MCFixup *Fixup) const {
  if (!getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup))
    return false;

  if (Res.getRefKind() != MCSymbolRefExpr::VK_None)
    return false;

  // evaluateAsAbsolute() and evaluateAsValue() require that we evaluate the
  // %hi/%lo/etc. here. Fixup is a null pointer when either of these is the
  // caller.
  if (Res.isAbsolute() && Fixup == nullptr) {
    int64_t AbsVal = Res.getConstant();
    switch (Kind) {
    case MEK_None:
      llvm_unreachable("MEK_None is invalid");
    case MEK_DTPREL16:
      // MEK_DTPREL is used for marking TLS DIEExpr only
      // and contains a regular sub-expression.
      return getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup);
    case MEK_ELF_LITERAL:      /* !literal relocation.  */
    case MEK_LITUSE_ADDR:      /* !lituse_addr relocation.  */
    case MEK_LITUSE_BASE:      /* !lituse_base relocation.  */
    case MEK_LITUSE_BYTOFF:    /* !lituse_bytoff relocation.  */
    case MEK_LITUSE_JSR:       /* !lituse_jsr relocation.  */
    case MEK_LITUSE_TLSGD:     /* !lituse_tlsgd relocation.  */
    case MEK_LITUSE_TLSLDM:    /* !lituse_tlsldm relocation.  */
    case MEK_LITUSE_JSRDIRECT: /* !lituse_jsrdirect relocation.  */
    case MEK_GPDISP:           /* !gpdisp relocation.  */
    case MEK_GPDISP_HI16:
    case MEK_GPDISP_LO16:
    case MEK_GPREL_HI16:      /* !gprelhigh relocation.  */
    case MEK_GPREL_LO16:      /* !gprellow relocation.  */
    case MEK_GPREL16:         /* !gprel relocation.  */
    case MEK_BRSGP:           /* !samegp relocation.  */
    case MEK_TLSGD:           /* !tlsgd relocation.  */
    case MEK_TLSLDM:          /* !tlsldm relocation.  */
    case MEK_GOTDTPREL16:     /* !gotdtprel relocation.  */
    case MEK_DTPREL_HI16:     /* !dtprelhi relocation.  */
    case MEK_DTPREL_LO16:     /* !dtprello relocation.  */
    case MEK_GOTTPREL16:      /* !gottprel relocation.  */
    case MEK_TPREL_HI16:      /* !tprelhi relocation.  */
    case MEK_TPREL_LO16:      /* !tprello relocation.  */
    case MEK_TPREL16:         /* !tprel relocation.  */
    case MEK_ELF_LITERAL_GOT: /* !literal_got relocation.  */
      return false;
    }
    Res = MCValue::get(AbsVal);
    return true;
  }
  // We want to defer it for relocatable expressions since the constant is
  // applied to the whole symbol value.
  //
  // The value of getKind() that is given to MCValue is only intended to aid
  // debugging when inspecting MCValue objects. It shouldn't be relied upon
  // for decision making.
  Res =
      MCValue::get(Res.getSymA(), Res.getSymB(), Res.getConstant(), getKind());

  return true;
}

void Sw64MCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

static void fixELFSymbolsInTLSFixupsImpl(const MCExpr *Expr, MCAssembler &Asm) {
  switch (Expr->getKind()) {
  case MCExpr::Target:
    fixELFSymbolsInTLSFixupsImpl(cast<Sw64MCExpr>(Expr)->getSubExpr(), Asm);
    break;
  case MCExpr::Constant:
    break;
  case MCExpr::Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(Expr);
    fixELFSymbolsInTLSFixupsImpl(BE->getLHS(), Asm);
    fixELFSymbolsInTLSFixupsImpl(BE->getRHS(), Asm);
    break;
  }
  case MCExpr::SymbolRef: {
    // We're known to be under a TLS fixup, so any symbol should be
    // modified. There should be only one.
    const MCSymbolRefExpr &SymRef = *cast<MCSymbolRefExpr>(Expr);
    cast<MCSymbolELF>(SymRef.getSymbol()).setType(ELF::STT_TLS);
    break;
  }
  case MCExpr::Unary:
    fixELFSymbolsInTLSFixupsImpl(cast<MCUnaryExpr>(Expr)->getSubExpr(), Asm);
    break;
  }
}

// For lituse relocation, we don't need to change symbol type
// to tls.
void Sw64MCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {
  switch (getKind()) {
  case MEK_None:
    llvm_unreachable("MEK_None and MEK_Special are invalid");
    break;
  case MEK_GPDISP:
  case MEK_LITUSE_BASE:      /* !lituse_base relocation.  */
  case MEK_LITUSE_JSRDIRECT: /* !lituse_jsrdirect relocation.  */
  case MEK_GPDISP_HI16:
  case MEK_GPDISP_LO16:
  case MEK_ELF_LITERAL:
  case MEK_ELF_LITERAL_GOT:
  case MEK_GPREL_HI16:
  case MEK_GPREL_LO16:
  case MEK_GPREL16:
  case MEK_BRSGP:
    // If we do have nested target-specific expressions, they will be in
    // a consecutive chain.
    if (const Sw64MCExpr *E = dyn_cast<const Sw64MCExpr>(getSubExpr()))
      E->fixELFSymbolsInTLSFixups(Asm);
    break;
  case MEK_DTPREL16:
  case MEK_LITUSE_ADDR:   /* !lituse_addr relocation.  */
  case MEK_LITUSE_BYTOFF: /* !lituse_bytoff relocation.  */
  case MEK_LITUSE_JSR:    /* !lituse_jsr relocation.  */
  case MEK_LITUSE_TLSGD:  /* !lituse_tlsgd relocation.  */
  case MEK_LITUSE_TLSLDM: /* !lituse_tlsldm relocation.  */
  case MEK_TLSGD:         /* !tlsgd relocation.  */
  case MEK_TLSLDM:        /* !tlsldm relocation.  */
  case MEK_GOTDTPREL16:   /* !gotdtprel relocation.  */
  case MEK_DTPREL_HI16:   /* !dtprelhi relocation.  */
  case MEK_DTPREL_LO16:   /* !dtprello relocation.  */
  case MEK_GOTTPREL16:    /* !gottprel relocation.  */
  case MEK_TPREL_HI16:    /* !tprelhi relocation.  */
  case MEK_TPREL_LO16:    /* !tprello relocation.  */
  case MEK_TPREL16:       /* !tprel relocation.  */
    fixELFSymbolsInTLSFixupsImpl(getSubExpr(), Asm);
    break;
  }
}
