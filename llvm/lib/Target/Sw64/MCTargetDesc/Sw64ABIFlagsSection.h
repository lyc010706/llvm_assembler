//===- Sw64ABIFlagsSection.h - Sw64 ELF ABI Flags Section -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64ABIFLAGSSECTION_H
#define LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64ABIFLAGSSECTION_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Sw64ABIFlags.h"
#include <cstdint>

namespace llvm {

class MCStreamer;

struct Sw64ABIFlagsSection {
  // Internal representation of the fp_abi related values used in .module.
  enum class FpABIKind { ANY, XX, S32, S64, SOFT };

  // Version of flags structure.
  uint16_t Version = 0;
  // The level of the ISA: 1-5, 32, 64.
  uint8_t ISALevel = 0;
  // The revision of ISA: 0 for SW64 V and below, 1-n otherwise.
  uint8_t ISARevision = 0;
  // The size of general purpose registers.
  Sw64::AFL_REG GPRSize = Sw64::AFL_REG_NONE;
  // The size of co-processor 1 registers.
  Sw64::AFL_REG CPR1Size = Sw64::AFL_REG_NONE;
  // The size of co-processor 2 registers.
  Sw64::AFL_REG CPR2Size = Sw64::AFL_REG_NONE;
  // Processor-specific extension.
  Sw64::AFL_EXT ISAExtension = Sw64::AFL_EXT_NONE;
  // Mask of ASEs used.
  uint32_t ASESet = 0;

  bool OddSPReg = false;

protected:
  // The floating-point ABI.
  FpABIKind FpABI = FpABIKind::ANY;

public:
  Sw64ABIFlagsSection() = default;

  uint16_t getVersionValue() { return (uint16_t)Version; }
  uint8_t getISALevelValue() { return (uint8_t)ISALevel; }
  uint8_t getISARevisionValue() { return (uint8_t)ISARevision; }
  uint8_t getGPRSizeValue() { return (uint8_t)GPRSize; }
  uint8_t getCPR1SizeValue();
  uint8_t getCPR2SizeValue() { return (uint8_t)CPR2Size; }
  uint8_t getFpABIValue();
  uint32_t getISAExtensionValue() { return (uint32_t)ISAExtension; }
  uint32_t getASESetValue() { return (uint32_t)ASESet; }

  uint32_t getFlags1Value() {
    uint32_t Value = 0;

    if (OddSPReg)
      Value |= (uint32_t)Sw64::AFL_FLAGS1_ODDSPREG;

    return Value;
  }

  uint32_t getFlags2Value() { return 0; }

  FpABIKind getFpABI() { return FpABI; }
  void setFpABI(FpABIKind Value) {
    FpABI = Value;
  }

  StringRef getFpABIString(FpABIKind Value);

  template <class PredicateLibrary>
  void setGPRSizeFromPredicates(const PredicateLibrary &P) {
    GPRSize = P.isGP64bit() ? Sw64::AFL_REG_64 : Sw64::AFL_REG_32;
  }

  template <class PredicateLibrary>
  void setCPR1SizeFromPredicates(const PredicateLibrary &P) {
    if (P.useSoftFloat())
      CPR1Size = Sw64::AFL_REG_NONE;
    else if (P.hasMSA())
      CPR1Size = Sw64::AFL_REG_128;
    else
      CPR1Size = P.isFP64bit() ? Sw64::AFL_REG_64 : Sw64::AFL_REG_32;
  }

  template <class PredicateLibrary>
  void setISAExtensionFromPredicates(const PredicateLibrary &P) {
    if (P.hasCnSw64())
      ISAExtension = Sw64::AFL_EXT_OCTEON;
    else
      ISAExtension = Sw64::AFL_EXT_NONE;
  }

  template <class PredicateLibrary>
  void setFpAbiFromPredicates(const PredicateLibrary &P) {
    FpABI = FpABIKind::ANY;
    if (P.useSoftFloat())
      FpABI = FpABIKind::SOFT;

    if (P.isABI_S64())
      FpABI = FpABIKind::S64;
  }

  template <class PredicateLibrary>
  void setAllFromPredicates(const PredicateLibrary &P) {
    setGPRSizeFromPredicates(P);
    setCPR1SizeFromPredicates(P);
    setISAExtensionFromPredicates(P);
    setFpAbiFromPredicates(P);
    OddSPReg = P.useOddSPReg();
  }
};

MCStreamer &operator<<(MCStreamer &OS, Sw64ABIFlagsSection &ABIFlagsSection);

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64ABIFLAGSSECTION_H
