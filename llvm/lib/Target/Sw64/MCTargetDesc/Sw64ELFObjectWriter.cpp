//===-- Sw64ELFObjectWriter.cpp - Sw64 ELF Writer -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Sw64FixupKinds.h"
#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <list>
#include <utility>

#define DEBUG_TYPE "sw_64-elf-object-writer"

using namespace llvm;

namespace {

// Holds additional information needed by the relocation ordering algorithm.
struct Sw64RelocationEntry {
  const ELFRelocationEntry R; // < The relocation.
  bool Matched = false;       // < Is this relocation part of a match.

  Sw64RelocationEntry(const ELFRelocationEntry &R) : R(R) {}

  void print(raw_ostream &Out) const {
    R.print(Out);
    Out << ", Matched=" << Matched;
  }
};

#ifndef NDEBUG
raw_ostream &operator<<(raw_ostream &OS, const Sw64RelocationEntry &RHS) {
  RHS.print(OS);
  return OS;
}
#endif

class Sw64ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  Sw64ELFObjectWriter(uint8_t OSABI, bool HasRelocationAddend, bool Is64);

  ~Sw64ELFObjectWriter() override = default;

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCSymbol &Sym,
                               unsigned Type) const override;
  void sortRelocs(const MCAssembler &Asm,
                  std::vector<ELFRelocationEntry> &Relocs) override;
};

// The possible results of the Predicate function used by find_best.
enum FindBestPredicateResult {
  FindBest_NoMatch = 0,  // < The current element is not a match.
  FindBest_Match,        // < The current element is a match but better ones are
                         //   possible.
  FindBest_PerfectMatch, // < The current element is an unbeatable match.
};

} // end anonymous namespace

// Copy elements in the range [First, Last) to d1 when the predicate is true or
// d2 when the predicate is false. This is essentially both std::copy_if and
// std::remove_copy_if combined into a single pass.
template <class InputIt, class OutputIt1, class OutputIt2, class UnaryPredicate>
static std::pair<OutputIt1, OutputIt2> copy_if_else(InputIt First, InputIt Last,
                                                    OutputIt1 d1, OutputIt2 d2,
                                                    UnaryPredicate Predicate) {
  for (InputIt I = First; I != Last; ++I) {
    if (Predicate(*I)) {
      *d1 = *I;
      d1++;
    } else {
      *d2 = *I;
      d2++;
    }
  }

  return std::make_pair(d1, d2);
}

// Find the best match in the range [First, Last).
//
// An element matches when Predicate(X) returns FindBest_Match or
// FindBest_PerfectMatch. A value of FindBest_PerfectMatch also terminates
// the search. BetterThan(A, B) is a comparator that returns true when A is a
// better match than B. The return value is the position of the best match.
//
// This is similar to std::find_if but finds the best of multiple possible
// matches.
template <class InputIt, class UnaryPredicate>
static InputIt find_best(InputIt First, InputIt Last,
                         UnaryPredicate Predicate) {
  InputIt Best = Last;

  for (InputIt I = First; I != Last; ++I) {
    unsigned Matched = Predicate(*I);
    if (Matched != FindBest_NoMatch) {
      LLVM_DEBUG(dbgs() << std::distance(First, I) << " is a match (";
                 I->print(dbgs()); dbgs() << ")\n");
      if (Best == Last) {
        LLVM_DEBUG(dbgs() << ".. and it beats the last one\n");
        Best = I;
      }
    }
    if (Matched == FindBest_PerfectMatch) {
      LLVM_DEBUG(dbgs() << ".. and it is unbeatable\n");
      break;
    }
  }

  return Best;
}

#ifndef NDEBUG
// Print all the relocations.
template <class Container>
static void dumpRelocs(const char *Prefix, const Container &Relocs) {
  for (const auto &R : Relocs) {
    dbgs() << Prefix;
    R.print(dbgs());
    dbgs() << "\n";
  }
}
#endif

Sw64ELFObjectWriter::Sw64ELFObjectWriter(uint8_t OSABI,
                                         bool HasRelocationAddend, bool Is64)
    : MCELFObjectTargetWriter(Is64, OSABI, ELF::EM_SW64, HasRelocationAddend) {}

unsigned Sw64ELFObjectWriter::getRelocType(MCContext &Ctx,
                                           const MCValue &Target,
                                           const MCFixup &Fixup,
                                           bool IsPCRel) const {
  // Determine the type of the relocation.
  unsigned Kind = (unsigned)Fixup.getKind();
  switch (Kind) {
  case Sw64::fixup_SW64_NONE:
    return ELF::R_SW_64_NONE;
  case FK_Data_1:
  case FK_Data_2:
    Ctx.reportError(Fixup.getLoc(),
                    "SW64 does not support one byte relocations");
    return ELF::R_SW_64_NONE;
  case FK_Data_4:
    if (Fixup.getValue()->getKind() == MCExpr::Binary)
      return ELF::R_SW_64_SREL32; // .cfi_startproc
    else
      return ELF::R_SW_64_REFLONG; // R_SW_64_32
    break;
  case FK_Data_8: // .8byte ($.str)
    if (IsPCRel)
      return ELF::R_SW_64_SREL64;
    else
      return ELF::R_SW_64_REFQUAD; // R_SW_64_64
    break;
  case Sw64::fixup_SW64_32:
    return ELF::R_SW_64_REFLONG;
    break;
  case Sw64::fixup_SW64_64:
  case Sw64::fixup_SW64_CTOR:
    return ELF::R_SW_64_REFQUAD;
    break;
  case Sw64::fixup_SW64_GPREL32:
    return ELF::R_SW_64_GPREL32;
    break;

  case Sw64::fixup_SW64_ELF_LITERAL:
    return ELF::R_SW_64_LITERAL;
    break;
  case Sw64::fixup_SW64_LITUSE:
    return ELF::R_SW_64_LITUSE;
    break;
  case Sw64::fixup_SW64_LITERAL_BASE:
    return ELF::R_SW_64_DUMMY_LITERAL;
    break;
  case Sw64::fixup_SW64_LITUSE_JSRDIRECT:
    return ELF::R_SW_64_DUMMY_LITUSE;
    break;
  case Sw64::fixup_SW64_GPDISP:
    return ELF::R_SW_64_GPDISP;
    break;
  case Sw64::fixup_SW64_GPDISP_HI16:
    return ELF::R_SW_64_GPDISP;
    break;
  case Sw64::fixup_SW64_GPDISP_LO16:
    return ELF::R_SW_64_GPDISP;
    break;
  case Sw64::fixup_SW64_23_PCREL_S2:
    return ELF::R_SW_64_BRADDR;
    break;
  case Sw64::fixup_SW64_HINT:
    return ELF::R_SW_64_HINT;
    break;
  case Sw64::fixup_SW64_16_PCREL:
    return ELF::R_SW_64_SREL16;
    break;
  case Sw64::fixup_SW64_32_PCREL:
    return ELF::R_SW_64_SREL32;
    break;
  case Sw64::fixup_SW64_64_PCREL:
    return ELF::R_SW_64_SREL64;
    break;
  case Sw64::fixup_SW64_GPREL_HI16:
    return ELF::R_SW_64_GPRELHIGH;
    break;
  case Sw64::fixup_SW64_GPREL_LO16:
    return ELF::R_SW_64_GPRELLOW;
    break;
  case Sw64::fixup_SW64_GPREL16:
    return ELF::R_SW_64_GPREL16;
    break;
  case Sw64::fixup_SW64_BRSGP:
    return ELF::R_SW_64_BRSGP;
    break;
  case Sw64::fixup_SW64_TLSGD:
    return ELF::R_SW_64_TLSGD;
    break;
  case Sw64::fixup_SW64_TLSLDM:
    return ELF::R_SW_64_TLSLDM;
    break;
  case Sw64::fixup_SW64_DTPMOD64:
    return ELF::R_SW_64_DTPMOD64;
    break;
  case Sw64::fixup_SW64_GOTDTPREL16:
    return ELF::R_SW_64_GOTDTPREL;
    break;
  case Sw64::fixup_SW64_DTPREL64:
    return ELF::R_SW_64_DTPREL64;
    break;
  case Sw64::fixup_SW64_DTPREL_HI16:
    return ELF::R_SW_64_DTPRELHI;
    break;
  case Sw64::fixup_SW64_DTPREL_LO16:
    return ELF::R_SW_64_DTPRELLO;
    break;
  case Sw64::fixup_SW64_DTPREL16:
    return ELF::R_SW_64_DTPREL16;
    break;
  case Sw64::fixup_SW64_GOTTPREL16:
    return ELF::R_SW_64_GOTTPREL;
    break;
  case Sw64::fixup_SW64_TPREL64:
    return ELF::R_SW_64_TPREL64;
    break;
  case Sw64::fixup_SW64_TPREL_HI16:
    return ELF::R_SW_64_TPRELHI;
    break;
  case Sw64::fixup_SW64_TPREL_LO16:
    return ELF::R_SW_64_TPRELLO;
    break;
  case Sw64::fixup_SW64_TPREL16:
    return ELF::R_SW_64_TPREL16;
    break;
  case Sw64::fixup_SW64_ELF_LITERAL_GOT:
    return ELF::R_SW_64_LITERAL_GOT;
    break;
  }
  llvm_unreachable("invalid fixup kind!");
}

// Determine whether a relocation (X) matches the one given in R.
//
// A relocation matches if:
// - It's type matches that of a corresponding low part. This is provided in
//   MatchingType for efficiency.
// - It's based on the same symbol.
// - It's offset of greater or equal to that of the one given in R.
//   It should be noted that this rule assumes the programmer does not use
//   offsets that exceed the alignment of the symbol. The carry-bit will be
//   incorrect if this is not true.
//
// A matching relocation is unbeatable if:
// - It is not already involved in a match.
// - It's offset is exactly that of the one given in R.
static FindBestPredicateResult isMatchingReloc(const Sw64RelocationEntry &X,
                                               const ELFRelocationEntry &R,
                                               unsigned MatchingType) {
  if (X.R.Type == MatchingType && X.R.OriginalSymbol == R.OriginalSymbol) {
    if (!X.Matched && X.R.OriginalAddend == R.OriginalAddend)
      return FindBest_PerfectMatch;
  }
  return FindBest_NoMatch;
}

// Rewrite Reloc Target And Type
static ELFRelocationEntry RewriteTypeReloc(const ELFRelocationEntry R,
                                           const MCSymbolELF *RenamedSymA) {
  ELFRelocationEntry Entry = R;
  switch (R.Type) {
  default:
    break;
  case ELF::R_SW_64_DUMMY_LITUSE:
    Entry.Type = ELF::R_SW_64_LITUSE;
    Entry.Symbol = RenamedSymA;
    Entry.Addend = 0x3;
    break;
  case ELF::R_SW_64_DUMMY_LITERAL:
    Entry.Type = ELF::R_SW_64_LITERAL;
    break;
  case ELF::R_SW_64_GPDISP:
    Entry.Symbol = RenamedSymA;
    Entry.Addend = 0x4;
    break;
  }
  return Entry;
}

void Sw64ELFObjectWriter::sortRelocs(const MCAssembler &Asm,
                                     std::vector<ELFRelocationEntry> &Relocs) {
  if (Relocs.size() < 2)
    return;

  MCContext &Ctx = Asm.getContext();
  std::list<Sw64RelocationEntry> Sorted;
  std::list<ELFRelocationEntry> Remainder;
  std::list<ELFRelocationEntry> Orig;
  const auto *RenamedSymA = cast<MCSymbolELF>(Ctx.getOrCreateSymbol(".text"));

  LLVM_DEBUG(dumpRelocs("R: ", Relocs));

  // Sort relocations by the address they are applied to.
  llvm::sort(Relocs,
             [](const ELFRelocationEntry &A, const ELFRelocationEntry &B) {
               return A.Offset < B.Offset;
             });

  // copy all reloc entry into remainder, except lituse.
  // all lituse will be insert literal->next later.
  copy_if_else(Relocs.begin(), Relocs.end(), std::back_inserter(Remainder),
               std::back_inserter(Sorted), [](const ELFRelocationEntry &Reloc) {
                 return Reloc.Type == ELF::R_SW_64_DUMMY_LITUSE;
               });

  // Separate the movable relocations (AHL relocations using the high bits) from
  // the immobile relocations (everything else). This does not preserve high/low
  // matches that already existed in the input.
  for (auto &R : Remainder) {
    LLVM_DEBUG(dbgs() << "Matching: " << R << "\n");

    auto InsertionPoint = find_best(
        Sorted.begin(), Sorted.end(), [&R](const Sw64RelocationEntry &X) {
          return isMatchingReloc(X, R, ELF::R_SW_64_DUMMY_LITERAL);
        });

    if (InsertionPoint != Sorted.end()) {
      // if lit_use and literal correctly matched,
      // InsertPoint is the reloc entry next to the literal
      InsertionPoint->Matched = true;
      InsertionPoint = std::next(InsertionPoint, 1);
    }
    Sorted.insert(InsertionPoint, R)->Matched = true;
  }
  assert(Relocs.size() == Sorted.size() && "Some relocs were not consumed");

  // Overwrite the original vector with the sorted elements. The caller expects
  // them in reverse order.
  unsigned CopyTo = 0;
  for (const auto &R : reverse(Sorted)) {
    ELFRelocationEntry Entry = RewriteTypeReloc(R.R, RenamedSymA);
    Relocs[CopyTo++] = Entry;
  }
}

bool Sw64ELFObjectWriter::needsRelocateWithSymbol(const MCSymbol &Sym,
                                                  unsigned Type) const {
  if (!isUInt<8>(Type))
    return needsRelocateWithSymbol(Sym, Type & 0xff) ||
           needsRelocateWithSymbol(Sym, (Type >> 8) & 0xff) ||
           needsRelocateWithSymbol(Sym, (Type >> 16) & 0xff);

  switch (Type) {
  default:
    errs() << Type << "\n";
    llvm_unreachable("Unexpected relocation");
    return true;

  // This relocation doesn't affect the section data.
  case ELF::R_SW_64_NONE:
    return false;
  // On REL ABI's (e.g. S32), these relocations form pairs. The pairing is done
  // by the static linker by matching the symbol and offset.
  // We only see one relocation at a time but it's still safe to relocate with
  // the section so long as both relocations make the same decision.
  //
  // Some older linkers may require the symbol for particular cases. Such cases
  // are not supported yet but can be added as required.
  case ELF::R_SW_64_REFLONG:
  case ELF::R_SW_64_REFQUAD:
  case ELF::R_SW_64_GPREL32:
  case ELF::R_SW_64_LITERAL:
  case ELF::R_SW_64_DUMMY_LITERAL:
  case ELF::R_SW_64_DUMMY_LITUSE:
  case ELF::R_SW_64_LITUSE:
  case ELF::R_SW_64_BRADDR:
  case ELF::R_SW_64_HINT:
  case ELF::R_SW_64_SREL16:
  case ELF::R_SW_64_SREL32:
  case ELF::R_SW_64_SREL64:
  case ELF::R_SW_64_GPRELHIGH:
  case ELF::R_SW_64_GPRELLOW:
  case ELF::R_SW_64_GPREL16:
  case ELF::R_SW_64_COPY:
  case ELF::R_SW_64_GLOB_DAT:
  case ELF::R_SW_64_JMP_SLOT:
  case ELF::R_SW_64_RELATIVE:
  case ELF::R_SW_64_BRSGP:
  case ELF::R_SW_64_TLSGD:
  case ELF::R_SW_64_TLSLDM:
  case ELF::R_SW_64_DTPMOD64:
  case ELF::R_SW_64_GOTDTPREL:
  case ELF::R_SW_64_DTPREL64:
  case ELF::R_SW_64_DTPRELHI:
  case ELF::R_SW_64_DTPRELLO:
  case ELF::R_SW_64_DTPREL16:
  case ELF::R_SW_64_GOTTPREL:
  case ELF::R_SW_64_TPREL64:
  case ELF::R_SW_64_TPRELHI:
  case ELF::R_SW_64_TPRELLO:
  case ELF::R_SW_64_TPREL16:
  case ELF::R_SW_64_NUM:
  case ELF::R_SW_64_LITERAL_GOT:
  case ELF::R_SW_64_PC32:
  case ELF::R_SW_64_EH:
    return false;

  case ELF::R_SW_64_GPDISP:
    return true;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createSw64ELFObjectWriter(const Triple &TT, bool IsS32) {
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  bool IsS64 = true;
  bool HasRelocationAddend = TT.isArch64Bit();
  return std::make_unique<Sw64ELFObjectWriter>(OSABI, HasRelocationAddend,
                                               IsS64);
}
