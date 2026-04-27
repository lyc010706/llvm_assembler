//===---- Sw64ABIInfo.cpp - Information about SW64 ABI's ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Sw64ABIInfo.h"
#include "Sw64RegisterInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCTargetOptions.h"

using namespace llvm;
Sw64ABIInfo Sw64ABIInfo::computeTargetABI(const Triple &TT, StringRef CPU,
                                          const MCTargetOptions &Options) {
  if (Options.getABIName().startswith("n64"))
    return Sw64ABIInfo::S64();

  assert(Options.getABIName().empty() && "Unknown ABI option for SW64");

  if (TT.isSw64())
    return Sw64ABIInfo::S64();
  else
    assert(!TT.isSw64() && "sw_64 ABI is not appoint 64 bit.");
  return Sw64ABIInfo::S64();
}
