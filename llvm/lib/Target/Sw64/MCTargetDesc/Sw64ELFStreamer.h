//===- Sw64ELFStreamer.h - ELF Object Output --------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is a custom MCELFStreamer which allows us to insert some hooks before
// emitting data into an actual object file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64ELFSTREAMER_H
#define LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64ELFSTREAMER_H

#include "Sw64OptionRecord.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCELFStreamer.h"
#include <memory>

namespace llvm {

class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCSubtargetInfo;
struct MCDwarfFrameInfo;

class Sw64ELFStreamer : public MCELFStreamer {
  SmallVector<std::unique_ptr<Sw64OptionRecord>, 8> Sw64OptionRecords;
  Sw64RegInfoRecord *RegInfoRecord;
  SmallVector<MCSymbol *, 4> Labels;

public:
  Sw64ELFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> MAB,
                  std::unique_ptr<MCObjectWriter> OW,
                  std::unique_ptr<MCCodeEmitter> Emitter);

  // Overriding this function allows us to add arbitrary behaviour before the
  // Inst is actually emitted. For example, we can inspect the operands and
  // gather sufficient information that allows us to reason about the register
  // usage for the translation unit.
  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;

  // Overriding this function allows us to record all labels that should be
  // marked as microSW64. Based on this data marking is done in
  // EmitInstruction.
  void emitLabel(MCSymbol *Symbol, SMLoc Loc = SMLoc()) override;

  // Overriding this function allows us to dismiss all labels that are
  // candidates for marking as microSW64 when .section directive is processed.
  void switchSection(MCSection *Section,
                     const MCExpr *Subsection = nullptr) override;

  // Overriding these functions allows us to dismiss all labels that are
  // candidates for marking as microSW64 when .word/.long/.4byte etc
  // directives are emitted.
  void emitValueImpl(const MCExpr *Value, unsigned Size, SMLoc Loc) override;
  void emitIntValue(uint64_t Value, unsigned Size) override;

  // Overriding these functions allows us to avoid recording of these labels
  // in EmitLabel and later marking them as microSW64.
  void emitCFIStartProcImpl(MCDwarfFrameInfo &Frame) override;
  void emitCFIEndProcImpl(MCDwarfFrameInfo &Frame) override;
  MCSymbol *emitCFILabel() override;

  // Emits all the option records stored up until the point it's called.
  void EmitSw64OptionRecords();

  // Mark labels as microSW64, if necessary for the subtarget.
  void createPendingLabelRelocs();
};

MCELFStreamer *createSw64ELFStreamer(MCContext &Context,
                                     std::unique_ptr<MCAsmBackend> MAB,
                                     std::unique_ptr<MCObjectWriter> OW,
                                     std::unique_ptr<MCCodeEmitter> Emitter,
                                     bool RelaxAll);
} // end namespace llvm

#endif // LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64ELFSTREAMER_H
