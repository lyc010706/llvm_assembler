//===- Sw64OptionRecord.h - Abstraction for storing information -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Sw64OptionRecord - Abstraction for storing arbitrary information in
// ELF files. Arbitrary information (e.g. register usage) can be stored in Sw64
// specific ELF sections like .Sw64.options. Specific records should subclass
// Sw64OptionRecord and provide an implementation to EmitSw64OptionRecord which
// basically just dumps the information into an ELF section. More information
// about .Sw64.option can be found in the SysV ABI and the 64-bit ELF Object
// specification.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_SW64OPTIONRECORD_H
#define LLVM_LIB_TARGET_SW64_SW64OPTIONRECORD_H

#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCRegisterInfo.h"
#include <cstdint>

namespace llvm {

class Sw64ELFStreamer;

class Sw64OptionRecord {
public:
  virtual ~Sw64OptionRecord() = default;

  virtual void EmitSw64OptionRecord() = 0;
};

class Sw64RegInfoRecord : public Sw64OptionRecord {
public:
  Sw64RegInfoRecord(Sw64ELFStreamer *S, MCContext &Context)
      : Streamer(S), Context(Context) {

    const MCRegisterInfo *TRI = Context.getRegisterInfo();
    GPRCRegClass = &(TRI->getRegClass(Sw64::GPRCRegClassID));
    F4RCRegClass = &(TRI->getRegClass(Sw64::F4RCRegClassID));
    F8RCRegClass = &(TRI->getRegClass(Sw64::F8RCRegClassID));
    V256LRegClass = &(TRI->getRegClass(Sw64::V256LRegClassID));
  }

  ~Sw64RegInfoRecord() override = default;

  void EmitSw64OptionRecord() override;
  void SetPhysRegUsed(unsigned Reg, const MCRegisterInfo *MCRegInfo);

private:
  Sw64ELFStreamer *Streamer;
  MCContext &Context;
  const MCRegisterClass *GPRCRegClass;
  const MCRegisterClass *F4RCRegClass;
  const MCRegisterClass *F8RCRegClass;
  const MCRegisterClass *V256LRegClass;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SW64_SW64OPTIONRECORD_H
