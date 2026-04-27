//===- Sw64Relocations.h - Sw64 Code Relocations --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Sw64 target-specific relocation types.
//
//===----------------------------------------------------------------------===//

#ifndef Sw64RELOCATIONS_H
#define Sw64RELOCATIONS_H

#include "llvm/CodeGen/MachineRelocation.h"

namespace llvm {
namespace Sw64 {
enum RelocationType {
  reloc_literal,
  reloc_gprellow,
  reloc_gprelhigh,
  reloc_gpdist,
  reloc_bsr
};
}
} // namespace llvm
#endif
