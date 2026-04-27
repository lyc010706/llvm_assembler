//===-- Sw64TargetInfo.cpp - Sw64 Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/Sw64TargetInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheSw64Target() {
  static Target TheSw64Target;
  return TheSw64Target;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSw64TargetInfo() {
  RegisterTarget<Triple::sw_64,
                 /*HasJIT=*/true>
      X(getTheSw64Target(), "sw_64", "Sw64", "Sw64");
}
