//===- Sw64MCExpr.h - Sw64 specific MC expression classes -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64MCEXPR_H
#define LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64MCEXPR_H

#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"

namespace llvm {
class Sw64MCExpr : public MCTargetExpr {
public:
  // for linker relax, add complex relocation
  // exprkind here
  enum Sw64ExprKind {
    // use for relax
    MEK_HINT = 0x100,
    MEK_LITERAL = 0x200,
    MEK_LITUSE = 0x400,

    // do complex relocation
    MEK_LITUSE_BASE = MEK_LITERAL | MEK_LITUSE,
    MEK_LITUSE_JSRDIRECT = MEK_HINT | MEK_LITUSE,

    // None
    MEK_None = 0x000,

    // final reloc
    MEK_ELF_LITERAL,     /* !literal relocation.  */
    MEK_ELF_LITERAL_GOT, /* !literal_got relocation */
    MEK_LITUSE_ADDR,     /* !lituse_addr relocation.  */
    MEK_LITUSE_BYTOFF, /* !lituse_bytoff relocation.  */
    MEK_LITUSE_JSR,    /* !lituse_jsr relocation.  */
    MEK_LITUSE_TLSGD,  /* !lituse_tlsgd relocation.  */
    MEK_LITUSE_TLSLDM, /* !lituse_tlsldm relocation.  */
    MEK_GPDISP, /* !gpdisp relocation.  */
    MEK_GPDISP_HI16,
    MEK_GPDISP_LO16,
    MEK_GPREL_HI16,  /* !gprelhigh relocation.  */
    MEK_GPREL_LO16,  /* !gprellow relocation.  */
    MEK_GPREL16,     /* !gprel relocation.  */
    MEK_BRSGP,       /* !samegp relocation.  */
    MEK_TLSGD,       /* !tlsgd relocation.  */
    MEK_TLSLDM,      /* !tlsldm relocation.  */
    MEK_GOTDTPREL16, /* !gotdtprel relocation.  */
    MEK_DTPREL_HI16, /* !dtprelhi relocation.  */
    MEK_DTPREL_LO16, /* !dtprello relocation.  */
    MEK_DTPREL16,    /* !dtprel relocation.  */
    MEK_GOTTPREL16,  /* !gottprel relocation.  */
    MEK_TPREL_HI16,  /* !tprelhi relocation.  */
    MEK_TPREL_LO16,  /* !tprello relocation.  */
    MEK_TPREL16,     /* !tprel relocation.  */
  };

private:
  const Sw64ExprKind Kind;
  const MCExpr *Expr;

  explicit Sw64MCExpr(Sw64ExprKind Kind, const MCExpr *Expr)
      : Kind(Kind), Expr(Expr) {}

public:
  static const Sw64MCExpr *create(Sw64ExprKind Kind, const MCExpr *Expr,
                                  MCContext &Ctx);

  // Get the kind of this expression.
  Sw64ExprKind getKind() const { return Kind; }

  // Get the child of this expression.
  const MCExpr *getSubExpr() const { return Expr; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;

  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64MCEXPR_H
