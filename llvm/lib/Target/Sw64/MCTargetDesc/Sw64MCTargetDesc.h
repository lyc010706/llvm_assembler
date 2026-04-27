//===-- Sw64MCTargetDesc.h - Sw64 Target Descriptions -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Sw64 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64MCTARGETDESC_H
#define LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64MCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;
class Triple;
class raw_ostream;
class raw_pwrite_stream;

Target &getTheSw64Target();

MCCodeEmitter *createSw64MCCodeEmitterEL(const MCInstrInfo &MCII,
                                         MCContext &Ctx);

MCAsmBackend *createSw64AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter>
createSw64ELFObjectWriter(const Triple &TT, bool IsS32);

namespace SW64_MC {
StringRef selectSw64CPU(const Triple &TT, StringRef CPU);
}

} // namespace llvm

// Defines symbolic names for Sw64 registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "Sw64GenRegisterInfo.inc"

// Defines symbolic names for the Sw64 instructions.
#define GET_INSTRINFO_ENUM
#include "Sw64GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "Sw64GenSubtargetInfo.inc"

#endif
