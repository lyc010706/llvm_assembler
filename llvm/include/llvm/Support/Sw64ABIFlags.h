//===--- Sw64ABIFlags.h - SW64 ABI flags ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the constants for the ABI flags structure contained
// in the .Sw64.abiflags section.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_Sw64ABIFLAGS_H
#define LLVM_SUPPORT_Sw64ABIFLAGS_H

namespace llvm {
namespace Sw64 {

// Values for the xxx_size bytes of an ABI flags structure.
enum AFL_REG {
  AFL_REG_NONE = 0x00, // No registers
  AFL_REG_32 = 0x01,   // 32-bit registers
  AFL_REG_64 = 0x02,   // 64-bit registers
  AFL_REG_128 = 0x03   // 128-bit registers
};

// Values for the flags1 word of an ABI flags structure.
enum AFL_FLAGS1 { AFL_FLAGS1_ODDSPREG = 1 };

enum AFL_EXT {
  AFL_EXT_NONE = 0,  // None
  AFL_EXT_OCTEON = 5 // Cavium Networks Octeon
};
} // namespace Sw64
} // namespace llvm

#endif
