//===- Sw64OptionRecord.cpp - Abstraction for storing information ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Sw64OptionRecord.h"
#include "Sw64ABIInfo.h"
#include "Sw64ELFStreamer.h"
#include "Sw64TargetStreamer.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionELF.h"
#include <cassert>

using namespace llvm;

void Sw64RegInfoRecord::EmitSw64OptionRecord() {

  // We need to distinguish between S64 and the rest because at the moment
  // we don't emit .Sw64.options for other ELFs other than S64.
  // Since .reginfo has the same information as .Sw64.options (ODK_REGINFO),
  // we can use the same abstraction (Sw64RegInfoRecord class) to handle both.
}

void Sw64RegInfoRecord::SetPhysRegUsed(unsigned Reg,
                                       const MCRegisterInfo *MCRegInfo) {}
