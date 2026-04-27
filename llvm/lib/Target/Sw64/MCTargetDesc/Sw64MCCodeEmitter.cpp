//===-- Sw64MCCodeEmitter.cpp - Convert Sw64 Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Sw64MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "Sw64MCCodeEmitter.h"
#include "MCTargetDesc/Sw64FixupKinds.h"
#include "MCTargetDesc/Sw64MCExpr.h"
#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

#define GET_INSTRMAP_INFO
#include "Sw64GenInstrInfo.inc"
#undef GET_INSTRMAP_INFO

namespace llvm {

MCCodeEmitter *createSw64MCCodeEmitterEB(const MCInstrInfo &MCII,
                                         MCContext &Ctx) {
  return new Sw64MCCodeEmitter(MCII, Ctx, false);
}

MCCodeEmitter *createSw64MCCodeEmitterEL(const MCInstrInfo &MCII,
                                         MCContext &Ctx) {
  return new Sw64MCCodeEmitter(MCII, Ctx, true);
}

} // end namespace llvm

MCInst Sw64MCCodeEmitter::LowerCompactBranch(MCInst TmpInst) const {
  // <MCInst 194 <MCOperand Imm:0> <MCOperand Reg:33> <MCOperand
  // Expr:(.LBB0_2)>>
  // ==> <MCInst 194 <MCOperand Reg:33> <MCOperand Expr:(.LBB0_2)>>

  MCInst TI;
  unsigned int Size = TmpInst.getNumOperands();
  // for test op is or not a imm
  // as "bsr $RA,disp" will be convert to " bsr disp" will be an error
  TI.setOpcode(TmpInst.getOpcode());
  if (TmpInst.getOperand(0).isImm())
    for (unsigned int i = 0; i < Size; i++) {
      if (i == 0)
        continue;
      TI.addOperand(TmpInst.getOperand(i));
    }
  else {
    return TmpInst;
  }

  return TI;
}

void Sw64MCCodeEmitter::EmitByte(unsigned char C, raw_ostream &OS) const {
  OS << (char)C;
}

void Sw64MCCodeEmitter::EmitInstruction(uint64_t Val, unsigned Size,
                                        const MCSubtargetInfo &STI,
                                        raw_ostream &OS) const {
  // Output the instruction encoding in little endian byte order.
  // Little-endian byte ordering:
  // sw_64:   4 | 3 | 2 | 1
  for (unsigned i = 0; i < Size; ++i) {
    unsigned Shift = IsLittleEndian ? i * 8 : (Size - 1 - i) * 8;
    EmitByte((Val >> Shift) & 0xff, OS);
  }
}

/// encodeInstruction - Emit the instruction.
/// Size the instruction with Desc.getSize().
void Sw64MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  // Non-pseudo instructions that get changed for direct object
  // only based on operand values.
  // If this list of instructions get much longer we will move
  // the check to a function call. Until then, this is more efficient.
  MCInst TmpInst = MI;

  switch (MI.getOpcode()) {
  // If shift amount is >= 32 it the inst needs to be lowered further
  case Sw64::BEQ:
  case Sw64::BGE:
  case Sw64::BGT:
  case Sw64::BLBC:
  case Sw64::BLBS:
  case Sw64::BLE:
  case Sw64::BLT:
  case Sw64::BNE:
  case Sw64::BR:
  case Sw64::BSR:
  case Sw64::FBEQ:
  case Sw64::FBGE:
  case Sw64::FBGT:
  case Sw64::FBLE:
  case Sw64::FBLT:
  case Sw64::FBNE:
    break;
  case Sw64::ALTENT:
    return;
  }

  uint32_t Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);

  const MCInstrDesc &Desc = MCII.get(TmpInst.getOpcode());

  // Get byte count of instruction
  unsigned Size = Desc.getSize();
  if (!Size)
    llvm_unreachable("Desc.getSize() returns 0");

  EmitInstruction(Binary, Size, STI, OS);
}

/// getBranchTargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// [(store F4RC:$RA, (Sw64_gprello tglobaladdr:$DISP,
/// GPRC:$RB))], s_ild_lo>;
/// record the relocation and return zero.
unsigned
Sw64MCCodeEmitter::getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  // If the destination is an immediate, divide by 4.
  if (MO.isImm())
    return MO.getImm() >> 2;

  assert(MO.isExpr() &&
         "getBranchTargetOpValue expects only expressions or immediates");

  const MCExpr *FixupExpression = MO.getExpr();

  Fixups.push_back(MCFixup::create(0, FixupExpression,
                                   MCFixupKind(Sw64::fixup_SW64_23_PCREL_S2)));
  return 0;
}

/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned
Sw64MCCodeEmitter::getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  // If the destination is an immediate, divide by 4.
  if (MO.isImm())
    return MO.getImm() >> 2;

  assert(MO.isExpr() &&
         "getJumpTargetOpValue expects only expressions or an immediate");

  const MCExpr *FixupExpression = MO.getExpr();

  Fixups.push_back(MCFixup::create(0, FixupExpression,
                                   MCFixupKind(Sw64::fixup_SW64_23_PCREL_S2)));
  return 0;
}

static MCOperand createLituse(MCContext *Ctx) {
  const MCSymbol *Sym = Ctx->getOrCreateSymbol(".text");
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, *Ctx);

  return MCOperand::createExpr(
      Sw64MCExpr::create(Sw64MCExpr::MEK_LITUSE_JSR, Expr, *Ctx));
}

unsigned Sw64MCCodeEmitter::getExprOpValue(const MCExpr *Expr,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {

  MCExpr::ExprKind Kind = Expr->getKind();
  if (Kind == MCExpr::Constant) {
    return cast<MCConstantExpr>(Expr)->getValue();
  }

  if (Kind == MCExpr::Binary) {
    unsigned Res =
        getExprOpValue(cast<MCBinaryExpr>(Expr)->getLHS(), Fixups, STI);
    Res += getExprOpValue(cast<MCBinaryExpr>(Expr)->getRHS(), Fixups, STI);
    return Res;
  }

  if (Kind == MCExpr::Target) {
    const Sw64MCExpr *Sw64Expr = cast<Sw64MCExpr>(Expr);

    Sw64::Fixups FixupKind = Sw64::Fixups(0);
    switch (Sw64Expr->getKind()) {
    default:
      llvm_unreachable("Unknown fixup kind!");
      break;
    case Sw64MCExpr::MEK_LITUSE_BASE:
      FixupKind = Sw64::fixup_SW64_LITERAL_BASE;
      break;
    case Sw64MCExpr::MEK_LITUSE_JSRDIRECT:
      FixupKind = Sw64::fixup_SW64_LITUSE_JSRDIRECT;
      Fixups.push_back(
          MCFixup::create(0, Sw64Expr, MCFixupKind(Sw64::fixup_SW64_HINT)));
      break;
    case Sw64MCExpr::MEK_ELF_LITERAL:
      FixupKind = Sw64::fixup_SW64_ELF_LITERAL;
      break;
    case Sw64MCExpr::MEK_LITUSE_ADDR:
      FixupKind = Sw64::fixup_SW64_LITUSE;
      break;
    case Sw64MCExpr::MEK_LITUSE_BYTOFF:
      FixupKind = Sw64::fixup_SW64_LITUSE;
      break;
    case Sw64MCExpr::MEK_LITUSE_JSR:
      FixupKind = Sw64::fixup_SW64_LITUSE;
      break;
    case Sw64MCExpr::MEK_LITUSE_TLSGD:
      FixupKind = Sw64::fixup_SW64_LITUSE;
      break;
    case Sw64MCExpr::MEK_LITUSE_TLSLDM:
      FixupKind = Sw64::fixup_SW64_LITUSE;
      break;
    case Sw64MCExpr::MEK_HINT:
      FixupKind = Sw64::fixup_SW64_HINT;
      break;
    case Sw64MCExpr::MEK_GPDISP:
      FixupKind = Sw64::fixup_SW64_GPDISP;
      break;
    case Sw64MCExpr::MEK_GPDISP_HI16:
      FixupKind = Sw64::fixup_SW64_GPDISP_HI16;
      break;
    case Sw64MCExpr::MEK_GPDISP_LO16:
      return 0;
    case Sw64MCExpr::MEK_GPREL_HI16:
      FixupKind = Sw64::fixup_SW64_GPREL_HI16;
      break;
    case Sw64MCExpr::MEK_GPREL_LO16:
      FixupKind = Sw64::fixup_SW64_GPREL_LO16;
      break;
    case Sw64MCExpr::MEK_GPREL16:
      FixupKind = Sw64::fixup_SW64_GPREL16;
      break;
    case Sw64MCExpr::MEK_BRSGP:
      FixupKind = Sw64::fixup_SW64_BRSGP;
      break;
    case Sw64MCExpr::MEK_TLSGD:
      FixupKind = Sw64::fixup_SW64_TLSGD;
      break;
    case Sw64MCExpr::MEK_TLSLDM:
      FixupKind = Sw64::fixup_SW64_TLSLDM;
      break;
    case Sw64MCExpr::MEK_GOTDTPREL16:
      FixupKind = Sw64::fixup_SW64_GOTDTPREL16;
      break;
    case Sw64MCExpr::MEK_DTPREL_HI16:
      FixupKind = Sw64::fixup_SW64_DTPREL_HI16;
      break;
    case Sw64MCExpr::MEK_DTPREL_LO16:
      FixupKind = Sw64::fixup_SW64_DTPREL_LO16;
      break;
    case Sw64MCExpr::MEK_DTPREL16:
      FixupKind = Sw64::fixup_SW64_DTPREL16;
      break;
    case Sw64MCExpr::MEK_GOTTPREL16:
      FixupKind = Sw64::fixup_SW64_GOTTPREL16;
      break;
    case Sw64MCExpr::MEK_TPREL_HI16:
      FixupKind = Sw64::fixup_SW64_TPREL_HI16;
      break;
    case Sw64MCExpr::MEK_TPREL_LO16:
      FixupKind = Sw64::fixup_SW64_TPREL_LO16;
      break;
    case Sw64MCExpr::MEK_TPREL16:
      FixupKind = Sw64::fixup_SW64_TPREL16;
      break;
    case Sw64MCExpr::MEK_ELF_LITERAL_GOT:
      FixupKind = Sw64::fixup_SW64_ELF_LITERAL_GOT;
      break;
    } // switch

    Fixups.push_back(MCFixup::create(0, Sw64Expr, MCFixupKind(FixupKind)));
    return 0;
  }

  return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned
Sw64MCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = Ctx.getRegisterInfo()->getEncodingValue(Reg);
    return RegNo;
  } else if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  } else if (MO.isDFPImm()) {
    return static_cast<unsigned>(bit_cast<double>(MO.getDFPImm()));
  }

  // beq         op1    op2
  // to
  // beq  opc    op1    op2
  if (MCII.get(MI.getOpcode()).isBranch() && MI.getNumOperands() == 3) {
    // for beq/bne/fbeq ....
    return getBranchTargetOpValue(MI, 2, Fixups, STI);
  } else if (MCII.get(MI.getOpcode()).isBranch() && MI.getNumOperands() == 2) {
    // for br/bsr
    return getJumpTargetOpValue(MI, 1, Fixups, STI);
  }

  // MO must be an Expr.
  assert(MO.isExpr());
  return getExprOpValue(MO.getExpr(), Fixups, STI);
}

/// Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
template <unsigned ShiftAmount>
unsigned Sw64MCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  unsigned RegBits; // Base register is encoded in bits 20-16.
  unsigned OffBits; // offset is encoded in bits 15-0.

  if (MI.getOperand(OpNo).isImm()) { // vload
    RegBits = getMachineOpValue(MI, MI.getOperand(OpNo + 1), Fixups, STI) << 16;
    OffBits = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI);
  } else { // vstore
    RegBits = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI) << 16;
    OffBits = getMachineOpValue(MI, MI.getOperand(OpNo + 1), Fixups, STI);
  }

  // Apply the scale factor if there is one.
  // OffBits >>= ShiftAmount;

  return (OffBits & 0xFFFF) | RegBits;
}

// FIXME: should be called getMSBEncoding
unsigned
Sw64MCCodeEmitter::getSizeInsEncoding(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  assert(MI.getOperand(OpNo - 1).isImm());
  assert(MI.getOperand(OpNo).isImm());
  unsigned Position =
      getMachineOpValue(MI, MI.getOperand(OpNo - 1), Fixups, STI);
  unsigned Size = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI);

  return Position + Size - 1;
}

unsigned Sw64MCCodeEmitter::getUImm4AndValue(const MCInst &MI, unsigned OpNo,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  assert(MI.getOperand(OpNo).isImm());
  const MCOperand &MO = MI.getOperand(OpNo);
  unsigned Value = MO.getImm();
  switch (Value) {
  case 128:
    return 0x0;
  case 1:
    return 0x1;
  case 2:
    return 0x2;
  case 3:
    return 0x3;
  case 4:
    return 0x4;
  case 7:
    return 0x5;
  case 8:
    return 0x6;
  case 15:
    return 0x7;
  case 16:
    return 0x8;
  case 31:
    return 0x9;
  case 32:
    return 0xa;
  case 63:
    return 0xb;
  case 64:
    return 0xc;
  case 255:
    return 0xd;
  case 32768:
    return 0xe;
  case 65535:
    return 0xf;
  }
  llvm_unreachable("Unexpected value");
}

unsigned
Sw64MCCodeEmitter::getRegisterListOpValue(const MCInst &MI, unsigned OpNo,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  unsigned res = 0;

  // Register list operand is always first operand of instruction and it is
  // placed before memory operand (register + imm).

  for (unsigned I = OpNo, E = MI.getNumOperands() - 2; I < E; ++I) {
    unsigned Reg = MI.getOperand(I).getReg();
    unsigned RegNo = Ctx.getRegisterInfo()->getEncodingValue(Reg);
    if (RegNo != 31)
      res++;
    else
      res |= 0x10;
  }
  return res;
}

unsigned
Sw64MCCodeEmitter::getRegisterListOpValue16(const MCInst &MI, unsigned OpNo,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  return (MI.getNumOperands() - 4);
}

#include "Sw64GenMCCodeEmitter.inc"
