//===-- Sw64TargetStreamer.h - Sw64 Target Streamer ------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_SW64TARGETSTREAMER_H
#define LLVM_LIB_TARGET_SW64_SW64TARGETSTREAMER_H

#include "MCTargetDesc/Sw64ABIFlagsSection.h"
#include "MCTargetDesc/Sw64ABIInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/FormattedStream.h"
#include <optional>

namespace llvm {

struct Sw64ABIFlagsSection;

class Sw64TargetStreamer : public MCTargetStreamer {
public:
  Sw64TargetStreamer(MCStreamer &S);

  virtual void setPic(bool Value) {}

  virtual void emitDirectiveSetReorder();
  virtual void emitDirectiveSetNoReorder();
  virtual void emitDirectiveSetMacro();
  virtual void emitDirectiveSetNoMacro();
  virtual void emitDirectiveSetAt();
  virtual void emitDirectiveSetNoAt();
  virtual void emitDirectiveEnd(StringRef Name);

  virtual void emitDirectiveEnt(const MCSymbol &Symbol);
  virtual void emitDirectiveNaN2008();
  virtual void emitDirectiveNaNLegacy();
  virtual void emitDirectiveInsn();
  virtual void emitDirectiveSetCore3b();
  virtual void emitDirectiveSetCore4();
  virtual void emitFrame(unsigned StackReg, unsigned StackSize,
                         unsigned ReturnReg);
  virtual void emitDirectiveSetArch(StringRef Arch);

  void prettyPrintAsm(MCInstPrinter &InstPrinter, uint64_t Address,
                      const MCInst &Inst, const MCSubtargetInfo &STI,
                      raw_ostream &OS) override;

  void emitNop(SMLoc IDLoc, const MCSubtargetInfo *STI);

  void forbidModuleDirective() { ModuleDirectiveAllowed = false; }
  void reallowModuleDirective() { ModuleDirectiveAllowed = true; }
  bool isModuleDirectiveAllowed() { return ModuleDirectiveAllowed; }

  // This method enables template classes to set internal abi flags
  // structure values.
  template <class PredicateLibrary>
  void updateABIInfo(const PredicateLibrary &P) {
    ABI = P.getABI();
    ABIFlagsSection.setAllFromPredicates(P);
  }

  Sw64ABIFlagsSection &getABIFlagsSection() { return ABIFlagsSection; }
  const Sw64ABIInfo &getABI() const {
    assert(ABI && "ABI hasn't been set!");
    return *ABI;
  }

protected:
  std::optional<Sw64ABIInfo> ABI;
  Sw64ABIFlagsSection ABIFlagsSection;

  bool GPRInfoSet;
  unsigned GPRBitMask;
  int GPROffset;

  bool FPRInfoSet;
  unsigned FPRBitMask;
  int FPROffset;

  bool FrameInfoSet;
  int FrameOffset;
  unsigned FrameReg;
  unsigned ReturnReg;

private:
  bool ModuleDirectiveAllowed;
};

// This part is for ascii assembly output
class Sw64TargetAsmStreamer : public Sw64TargetStreamer {
  formatted_raw_ostream &OS;

public:
  Sw64TargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);

  void emitDirectiveSetReorder() override;
  void emitDirectiveSetNoReorder() override;
  void emitDirectiveSetMacro() override;
  void emitDirectiveSetNoMacro() override;
  void emitDirectiveSetAt() override;
  void emitDirectiveSetNoAt() override;
  void emitDirectiveEnd(StringRef Name) override;

  void emitDirectiveEnt(const MCSymbol &Symbol) override;
  void emitDirectiveNaN2008() override;
  void emitDirectiveNaNLegacy() override;
  void emitDirectiveInsn() override;
  void emitFrame(unsigned StackReg, unsigned StackSize,
                 unsigned ReturnReg) override;
  void emitDirectiveSetCore3b() override;
  void emitDirectiveSetCore4() override;

  void emitDirectiveSetArch(StringRef Arch) override;
};

// This part is for ELF object output
class Sw64TargetELFStreamer : public Sw64TargetStreamer {
  bool MicroSw64Enabled;
  const MCSubtargetInfo &STI;
  bool Pic;

public:
  MCELFStreamer &getStreamer();
  Sw64TargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

  void setPic(bool Value) override { Pic = Value; }

  void emitLabel(MCSymbol *Symbol) override;
  void finish() override;

  void emitDirectiveSetNoReorder() override;

  void emitDirectiveEnt(const MCSymbol &Symbol) override;
  void emitDirectiveNaN2008() override;
  void emitDirectiveNaNLegacy() override;
  void emitDirectiveInsn() override;
  void emitFrame(unsigned StackReg, unsigned StackSize,
                 unsigned ReturnReg) override;

  void emitSw64AbiFlags();
};
} // namespace llvm
#endif
