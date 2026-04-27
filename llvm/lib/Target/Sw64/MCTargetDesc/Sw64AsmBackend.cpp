//===-- Sw64AsmBackend.cpp - Sw64 Asm Backend  ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Sw64AsmBackend class.
//
//===----------------------------------------------------------------------===//
//

#include "MCTargetDesc/Sw64AsmBackend.h"
#include "MCTargetDesc/Sw64ABIInfo.h"
#include "MCTargetDesc/Sw64FixupKinds.h"
#include "MCTargetDesc/Sw64MCExpr.h"
#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Prepare value for the target space for it
static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {

  unsigned Kind = Fixup.getKind();
  switch (Kind) {
  default:
    return 0;
  case Sw64::fixup_SW64_32:
  case Sw64::fixup_SW64_64:
  case FK_Data_4:
  case FK_Data_8:
  case Sw64::fixup_SW64_GPREL32:
  case Sw64::fixup_SW64_LITUSE:
  case Sw64::fixup_SW64_GPREL_HI16:
  case Sw64::fixup_SW64_GPREL_LO16:
  case Sw64::fixup_SW64_GPREL16:
  case Sw64::fixup_SW64_TLSGD:
  case Sw64::fixup_SW64_TLSLDM:
  case Sw64::fixup_SW64_DTPMOD64:
  case Sw64::fixup_SW64_GOTDTPREL16:
  case Sw64::fixup_SW64_DTPREL64:
  case Sw64::fixup_SW64_DTPREL_HI16:
  case Sw64::fixup_SW64_DTPREL_LO16:
  case Sw64::fixup_SW64_DTPREL16:
  case Sw64::fixup_SW64_GOTTPREL16:
  case Sw64::fixup_SW64_TPREL64:
  case Sw64::fixup_SW64_TPREL_HI16:
  case Sw64::fixup_SW64_TPREL_LO16:
  case Sw64::fixup_SW64_TPREL16:
    break;
  case Sw64::fixup_SW64_23_PCREL_S2:
    // So far we are only using this type for branches.
    // For branches we start 1 instruction after the branch
    // so the displacement will be one instruction size less.
    Value -= 4;
    // The displacement is then divided by 4 to give us an 18 bit
    // address range.
    Value >>= 2;
    break;
  case Sw64::fixup_SW64_BRSGP:
    // So far we are only using this type for jumps.
    // The displacement is then divided by 4 to give us an 28 bit
    // address range.
    Value >>= 2;
    break;
  case Sw64::fixup_SW64_ELF_LITERAL:
    Value &= 0xffff;
    break;
  case Sw64::fixup_SW64_ELF_LITERAL_GOT:
    Value = ((Value + 0x8000) >> 16) & 0xffff;
    break;
  }
  return Value;
}

std::unique_ptr<MCObjectTargetWriter>
Sw64AsmBackend::createObjectTargetWriter() const {
  return createSw64ELFObjectWriter(TheTriple, IsS32);
}

/// ApplyFixup - Apply the Value for given Fixup into the provided
/// data fragment, at the offset specified by the fixup and following the
/// fixup kind as appropriate.
void Sw64AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                const MCValue &Target,
                                MutableArrayRef<char> Data, uint64_t Value,
                                bool IsResolved,
                                const MCSubtargetInfo *STI) const {
  MCFixupKind Kind = Fixup.getKind();
  MCContext &Ctx = Asm.getContext();
  Value = adjustFixupValue(Fixup, Value, Ctx);

  if (!Value)
    return; // Doesn't change encoding.

  // Where do we start in the object
  unsigned Offset = Fixup.getOffset();
  // Number of bytes we need to fixup
  unsigned NumBytes = (getFixupKindInfo(Kind).TargetSize + 7) / 8;
  // Used to point to big endian bytes
  unsigned FullSize;

  switch ((unsigned)Kind) {
  case Sw64::fixup_SW64_32:
    FullSize = 4;
    break;
  case Sw64::fixup_SW64_64:
    FullSize = 8;
    break;
  default:
    FullSize = 4;
    break;
  }

  // Grab current value, if any, from bits.
  uint64_t CurVal = 0;

  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned Idx = Endian == support::little ? i : (FullSize - 1 - i);
    CurVal |= (uint64_t)((uint8_t)Data[Offset + Idx]) << (i * 8);
  }

  uint64_t Mask = ((uint64_t)(-1) >> (64 - getFixupKindInfo(Kind).TargetSize));
  CurVal |= Value & Mask;

  // Write out the fixed up bytes back to the code/data bits.
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned Idx = Endian == support::little ? i : (FullSize - 1 - i);
    Data[Offset + Idx] = (uint8_t)((CurVal >> (i * 8)) & 0xff);
  }
}

std::optional<MCFixupKind> Sw64AsmBackend::getFixupKind(StringRef Name) const {
  return StringSwitch<std::optional<MCFixupKind>>(Name)
      .Case("R_SW_64_REFLONG", (MCFixupKind)Sw64::fixup_SW64_32)
      .Case("R_SW_64_REFQUAD", (MCFixupKind)Sw64::fixup_SW64_64)
      .Case("R_SW_64_REFQUAD", (MCFixupKind)Sw64::fixup_SW64_CTOR)
      .Case("R_SW_64_GPREL32", (MCFixupKind)Sw64::fixup_SW64_GPREL32)
      .Case("R_SW_64_LITERAL", (MCFixupKind)Sw64::fixup_SW64_ELF_LITERAL)
      .Case("R_SW_64_LITUSE", (MCFixupKind)Sw64::fixup_SW64_LITUSE)
      .Case("R_SW_64_GPDISP", (MCFixupKind)Sw64::fixup_SW64_GPDISP)
      .Case("R_SW_64_BRADDR", (MCFixupKind)Sw64::fixup_SW64_23_PCREL_S2)
      .Case("R_SW_64_HINT", (MCFixupKind)Sw64::fixup_SW64_HINT)
      .Case("R_SW_64_SREL16", (MCFixupKind)Sw64::fixup_SW64_16_PCREL)
      .Case("R_SW_64_SREL32", (MCFixupKind)Sw64::fixup_SW64_32_PCREL)
      .Case("R_SW_64_SREL64", (MCFixupKind)Sw64::fixup_SW64_64_PCREL)
      .Case("R_SW_64_GPRELHIGH", (MCFixupKind)Sw64::fixup_SW64_GPREL_HI16)
      .Case("R_SW_64_GPRELLOW", (MCFixupKind)Sw64::fixup_SW64_GPREL_LO16)
      .Case("R_SW_64_GPREL16", (MCFixupKind)Sw64::fixup_SW64_GPREL16)
      .Case("R_SW_64_BRSGP", (MCFixupKind)Sw64::fixup_SW64_BRSGP)
      .Case("R_SW_64_TLSGD", (MCFixupKind)Sw64::fixup_SW64_TLSGD)
      .Case("R_SW_64_TLSLDM", (MCFixupKind)Sw64::fixup_SW64_TLSLDM)
      .Case("R_SW_64_DTPMOD64", (MCFixupKind)Sw64::fixup_SW64_DTPMOD64)
      .Case("R_SW_64_GOTDTPREL", (MCFixupKind)Sw64::fixup_SW64_GOTDTPREL16)
      .Case("R_SW_64_DTPREL64", (MCFixupKind)Sw64::fixup_SW64_DTPREL64)
      .Case("R_SW_64_DTPRELHI", (MCFixupKind)Sw64::fixup_SW64_DTPREL_HI16)
      .Case("R_SW_64_DTPRELLO", (MCFixupKind)Sw64::fixup_SW64_DTPREL_LO16)
      .Case("R_SW_64_DTPREL16", (MCFixupKind)Sw64::fixup_SW64_DTPREL16)
      .Case("R_SW_64_GOTTPREL", (MCFixupKind)Sw64::fixup_SW64_GOTTPREL16)
      .Case("R_SW_64_TPREL64", (MCFixupKind)Sw64::fixup_SW64_TPREL64)
      .Case("R_SW_64_TPRELHI", (MCFixupKind)Sw64::fixup_SW64_TPREL_HI16)
      .Case("R_SW_64_TPRELLO", (MCFixupKind)Sw64::fixup_SW64_TPREL_LO16)
      .Case("R_SW_64_TPREL16", (MCFixupKind)Sw64::fixup_SW64_TPREL16)
      .Case("R_SW_64_LITERAL_GOT",
            (MCFixupKind)Sw64::fixup_SW64_ELF_LITERAL_GOT)
      .Default(MCAsmBackend::getFixupKind(Name));
}

const MCFixupKindInfo &
Sw64AsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo LittleEndianInfos[] = {
      // This table *must* be in same the order of fixup_* kinds in
      // Sw64FixupKinds.h.
      // name                    offset  bits  flags
      {"fixup_SW64_NONE", 0, 0, 0},
      {"fixup_SW64_32", 0, 32, 0},
      {"fixup_SW64_64", 0, 64, 0},
      {"fixup_SW64_CTOR", 0, 64, 0},
      {"fixup_SW64_GPREL32", 0, 32, 0},
      {"fixup_SW64_ELF_LITERAL", 0, 16, 0},
      {"fixup_SW64_LITUSE", 0, 32, 0},
      {"fixup_SW64_GPDISP", 0, 16, 0},
      {"fixup_SW64_GPDISP_HI16", 0, 16, 0},
      {"fixup_SW64_GPDISP_LO16", 0, 16, 0},
      {"fixup_SW64_23_PCREL_S2", 0, 21, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_SW64_HINT", 0, 14, 0},
      {"fixup_SW64_16_PCREL", 0, 16, 0},
      {"fixup_SW64_32_PCREL", 0, 32, 0},
      {"fixup_SW64_64_PCREL", 0, 64, 0},
      {"fixup_SW64_GPREL_HI16", 0, 16, 0},
      {"fixup_SW64_GPREL_LO16", 0, 16, 0},
      {"fixup_SW64_GPREL16", 0, 16, 0},
      {"fixup_SW64_BRSGP", 0, 21, 0},
      {"fixup_SW64_TLSGD", 0, 16, 0},
      {"fixup_SW64_TLSLDM", 0, 16, 0},
      {"fixup_SW64_DTPMOD64", 0, 64, 0},
      {"fixup_SW64_GOTDTPREL16", 0, 16, 0},
      {"fixup_SW64_DTPREL64", 0, 64, 0},
      {"fixup_SW64_DTPREL_HI16", 0, 16, 0},
      {"fixup_SW64_DTPREL_LO16", 0, 16, 0},
      {"fixup_SW64_DTPREL16", 0, 16, 0},
      {"fixup_SW64_GOTTPREL16", 0, 16, 0},
      {"fixup_SW64_TPREL64", 0, 64, 0},
      {"fixup_SW64_TPREL_HI16", 0, 16, 0},
      {"fixup_SW64_TPREL_LO16", 0, 16, 0},
      {"fixup_SW64_TPREL16", 0, 16, 0},
      {"fixup_SW64_ELF_LITERAL_GOT", 0, 16, 0},
      {"fixup_SW64_LITERAL_BASE", 0, 16, 0},
      {"fixup_SW64_LITUSE_JSRDIRECT", 0, 16, 0}};

  static_assert(std::size(LittleEndianInfos) == Sw64::NumTargetFixupKinds,
                "Not all SW64 little endian fixup kinds added!");

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");

  if (Endian == support::little)
    return LittleEndianInfos[Kind - FirstTargetFixupKind];
  else
    llvm_unreachable("sw_64 is not appoint litter endian.");
}

/// WriteNopData - Write an (optimal) nop sequence of Count bytes
/// to the given output. If the target cannot generate such a sequence,
/// it should return an error.
///
/// \return - True on success.
bool Sw64AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                  const MCSubtargetInfo *STI) const {
  // If the count is not 4-byte aligned, we must be writing data into the text
  // section (otherwise we have unaligned instructions, and thus have far
  // bigger problems), so just write zeros instead.
  OS.write_zeros(Count % 4);

  // We are properly aligned, so write NOPs as requested.
  Count /= 4;
  for (uint64_t i = 0; i != Count; ++i)
    support::endian::write<uint32_t>(OS, 0x43ff075f, support::little);
  return true;
}

bool Sw64AsmBackend::shouldForceRelocation(const MCAssembler &Asm,
                                           const MCFixup &Fixup,
                                           const MCValue &Target) {
  const unsigned FixupKind = Fixup.getKind();
  switch (FixupKind) {
  default:
    return false;
  // All these relocations require special processing
  // at linking time. Delegate this work to a linker.
  case Sw64::fixup_SW64_32:
  case Sw64::fixup_SW64_64:
  case Sw64::fixup_SW64_CTOR:
  case Sw64::fixup_SW64_GPREL32:
  case Sw64::fixup_SW64_ELF_LITERAL:
  case Sw64::fixup_SW64_LITUSE:
  case Sw64::fixup_SW64_GPDISP:
  case Sw64::fixup_SW64_GPDISP_HI16:
  case Sw64::fixup_SW64_HINT:
  case Sw64::fixup_SW64_16_PCREL:
  case Sw64::fixup_SW64_32_PCREL:
  case Sw64::fixup_SW64_64_PCREL:
  case Sw64::fixup_SW64_GPREL_HI16:
  case Sw64::fixup_SW64_GPREL_LO16:
  case Sw64::fixup_SW64_GPREL16:
  case Sw64::fixup_SW64_BRSGP:
  case Sw64::fixup_SW64_TLSGD:
  case Sw64::fixup_SW64_TLSLDM:
  case Sw64::fixup_SW64_DTPMOD64:
  case Sw64::fixup_SW64_GOTDTPREL16:
  case Sw64::fixup_SW64_DTPREL64:
  case Sw64::fixup_SW64_DTPREL_HI16:
  case Sw64::fixup_SW64_DTPREL_LO16:
  case Sw64::fixup_SW64_DTPREL16:
  case Sw64::fixup_SW64_GOTTPREL16:
  case Sw64::fixup_SW64_TPREL64:
  case Sw64::fixup_SW64_TPREL_HI16:
  case Sw64::fixup_SW64_TPREL_LO16:
  case Sw64::fixup_SW64_TPREL16:
  case Sw64::fixup_SW64_ELF_LITERAL_GOT:
    return true;
  case Sw64::fixup_SW64_23_PCREL_S2:
    return false;
  }
}

MCAsmBackend *llvm::createSw64AsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  Sw64ABIInfo ABI = Sw64ABIInfo::computeTargetABI(STI.getTargetTriple(),
                                                  STI.getCPU(), Options);
  return new Sw64AsmBackend(T, MRI, STI.getTargetTriple(), STI.getCPU(),
                            ABI.IsS64());
}
