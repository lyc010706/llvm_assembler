//===-- Sw64FixupKinds.h - Sw64 Specific Fixup Entries ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64FIXUPKINDS_H
#define LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64FIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace Sw64 {
// Although most of the current fixup types reflect a unique relocation
// one can have multiple fixup types for a given relocation and thus need
// to be uniquely named.
//
// This table *must* be in the same order of
// MCFixupKindInfo Infos[Sw64::NumTargetFixupKinds]
// in Sw64AsmBackend.cpp.
//
enum Fixups {
  // Branch fixups resulting in R_SW64_NONE.
  fixup_SW64_NONE = FirstTargetFixupKind,

  // A 32 bit reference to a symbol.
  // resulting in R_SW_64_REFLONG.
  fixup_SW64_32,

  // A 64 bit reference to a symbol.
  // resulting in - R_SW_64_REFQUAD.
  fixup_SW64_64,

  // A 64 bit reference to a symbol.
  // resulting in - R_SW_64_REFQUAD.
  fixup_SW64_CTOR,

  // A 32 bit GP relative offset. This is just like REFLONG except
  // that when the value is used the value of the gp register will be
  // added in.
  // resulting in - R_SW_64_GPREL32.
  fixup_SW64_GPREL32,

  // Used for an instruction that refers to memory off the GP register
  // resulting in - R_SW_64_LITERAL.
  fixup_SW64_ELF_LITERAL,
  // This reloc only appears immediately following an ELF_LITERAL reloc.
  // It identifies a use of the literal.  The symbol index is special:
  // 1 means the literal address is in the base register of a memory
  // format instruction; 2 means the literal address is in the byte
  // offset register of a byte-manipulation instruction; 3 means the
  // literal address is in the target register of a jsr instruction.
  // This does not actually do any relocation.
  // resulting in - R_SW_64_LITUSE.
  fixup_SW64_LITUSE,

  // Load the gp register.  This is always used for a ldih instruction
  // which loads the upper 16 bits of the gp register.  The symbol
  // index of the GPDISP instruction is an offset in bytes to the lda
  // instruction that loads the lower 16 bits.  The value to use for
  // the relocation is the difference between the GP value and the
  // current location; the load will always be done against a register
  // holding the current address.
  // resulting in - R_SW_64_GPDISP.
  fixup_SW64_GPDISP,
  fixup_SW64_GPDISP_HI16,
  fixup_SW64_GPDISP_LO16,

  // A 21 bit branch.
  // resulting in - R_SW_64_BRADDR.
  fixup_SW64_23_PCREL_S2,
  // A hint for a jump to a register.
  // resulting in - R_SW_64_HINT.
  fixup_SW64_HINT,

  // 16 bit PC relative offset.
  // resulting in - R_SW_64_SREL16.
  fixup_SW64_16_PCREL,

  // 32 bit PC relative offset.
  // resulting in - R_SW_64_SREL32.
  fixup_SW64_32_PCREL,

  // 64 bit PC relative offset.
  // resulting in - R_SW_64_SREL64.
  fixup_SW64_64_PCREL,

  // The high 16 bits of the displacement from GP to the target
  // resulting in - R_SW_64_GPRELHIGH.
  fixup_SW64_GPREL_HI16,

  // The low 16 bits of the displacement from GP to the target
  // resulting in - R_SW_64_GPRELLOW.
  fixup_SW64_GPREL_LO16,

  //  A 16-bit displacement from the GP to the target
  //  resulting in - R_SW_64_GPREL16.
  fixup_SW64_GPREL16,
  // A 21 bit branch that adjusts for gp loads
  // resulting in - R_SW_64_BRSGP.
  fixup_SW64_BRSGP,

  // Creates a tls_index for the symbol in the got.
  // resulting in - R_SW_64_TLSGD.
  fixup_SW64_TLSGD,

  // Creates a tls_index for the (current) module in the got.
  // resulting in - R_SW_64_TLSLDM.
  fixup_SW64_TLSLDM,

  // A dynamic relocation for a DTP module entry.
  // resulting in - R_SW_64_DTPMOD64.
  fixup_SW64_DTPMOD64,

  // Creates a 64-bit offset in the got for the displacement from DTP to the
  // target.
  // resulting in - R_SW_64_GOTDTPREL.
  fixup_SW64_GOTDTPREL16,

  // A dynamic relocation for a displacement from DTP to the target.
  // resulting in - R_SW_64_DTPREL64.
  fixup_SW64_DTPREL64,

  // The high 16 bits of the displacement from DTP to the target.
  // resulting in - R_SW_64_DTPRELHI.
  fixup_SW64_DTPREL_HI16,
  // The low 16 bits of the displacement from DTP to the target.
  // resulting in - R_SW_64_DTPRELLO.
  fixup_SW64_DTPREL_LO16,

  // A 16-bit displacement from DTP to the target.
  // resulting in - R_SW_64_DTPREL16
  fixup_SW64_DTPREL16,

  // Creates a 64-bit offset in the got for the displacement from TP to the
  // target.
  // resulting in - R_SW_64_GOTTPREL
  fixup_SW64_GOTTPREL16,

  // A dynamic relocation for a displacement from TP to the target.
  // resulting in - R_SW_64_TPREL64
  fixup_SW64_TPREL64,

  //  The high 16 bits of the displacement from TP to the target.
  //  resulting in - R_SW_64_TPRELHI
  fixup_SW64_TPREL_HI16,

  // The low 16 bits of the displacement from TP to the target.
  // resulting in - R_SW_64_TPRELLO
  fixup_SW64_TPREL_LO16,

  // A 16-bit displacement from TP to the target.
  // resulting in - R_SW_64_TPREL16
  fixup_SW64_TPREL16,

  // Used for an instruction that refers to memory off the GP register
  // together with literal, expand call range to 32 bits offset
  // resulting in - R_SW_64_LITERAL_GOT
  fixup_SW64_ELF_LITERAL_GOT,

  // TODO: for literal sorting reloc
  fixup_SW64_LITERAL_BASE,
  fixup_SW64_LITUSE_JSRDIRECT,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
} // namespace Sw64
} // namespace llvm
#endif
