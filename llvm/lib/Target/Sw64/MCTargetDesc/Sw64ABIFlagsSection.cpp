//===- Sw64ABIFlagsSection.cpp - Sw64 ELF ABI Flags Section ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Sw64ABIFlagsSection.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Sw64ABIFlags.h"

using namespace llvm;

uint8_t Sw64ABIFlagsSection::getFpABIValue() {
  llvm_unreachable("unexpected fp abi value");
}

StringRef Sw64ABIFlagsSection::getFpABIString(FpABIKind Value) {
  llvm_unreachable("unsupported fp abi value");
}
namespace llvm {

MCStreamer &operator<<(MCStreamer &OS, Sw64ABIFlagsSection &ABIFlagsSection) {
  return OS;
}

} // end namespace llvm
