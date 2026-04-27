//===-------- Sw64ELFStreamer.cpp - ELF Object Output ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Sw64ELFStreamer.h"
#include "Sw64OptionRecord.h"
#include "Sw64TargetStreamer.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

Sw64ELFStreamer::Sw64ELFStreamer(MCContext &Context,
                                 std::unique_ptr<MCAsmBackend> MAB,
                                 std::unique_ptr<MCObjectWriter> OW,
                                 std::unique_ptr<MCCodeEmitter> Emitter)
    : MCELFStreamer(Context, std::move(MAB), std::move(OW),
                    std::move(Emitter)) {
  RegInfoRecord = new Sw64RegInfoRecord(this, Context);
  Sw64OptionRecords.push_back(
      std::unique_ptr<Sw64RegInfoRecord>(RegInfoRecord));
}

void Sw64ELFStreamer::emitInstruction(const MCInst &Inst,
                                      const MCSubtargetInfo &STI) {
  MCELFStreamer::emitInstruction(Inst, STI);

  MCContext &Context = getContext();
  const MCRegisterInfo *MCRegInfo = Context.getRegisterInfo();

  for (unsigned OpIndex = 0; OpIndex < Inst.getNumOperands(); ++OpIndex) {
    const MCOperand &Op = Inst.getOperand(OpIndex);

    if (!Op.isReg())
      continue;

    unsigned Reg = Op.getReg();
    RegInfoRecord->SetPhysRegUsed(Reg, MCRegInfo);
  }

  createPendingLabelRelocs();
}

void Sw64ELFStreamer::emitCFIStartProcImpl(MCDwarfFrameInfo &Frame) {
  Frame.Begin = getContext().createTempSymbol();
  MCELFStreamer::emitLabel(Frame.Begin);
}

MCSymbol *Sw64ELFStreamer::emitCFILabel() {
  MCSymbol *Label = getContext().createTempSymbol("cfi", true);
  MCELFStreamer::emitLabel(Label);
  return Label;
}

void Sw64ELFStreamer::emitCFIEndProcImpl(MCDwarfFrameInfo &Frame) {
  Frame.End = getContext().createTempSymbol();
  MCELFStreamer::emitLabel(Frame.End);
}

void Sw64ELFStreamer::createPendingLabelRelocs() { Labels.clear(); }

void Sw64ELFStreamer::emitLabel(MCSymbol *Symbol, SMLoc Loc) {
  MCELFStreamer::emitLabel(Symbol);
  Labels.push_back(Symbol);
}

void Sw64ELFStreamer::switchSection(MCSection *Section,
                                    const MCExpr *Subsection) {
  MCELFStreamer::switchSection(Section, Subsection);
  Labels.clear();
}

void Sw64ELFStreamer::emitValueImpl(const MCExpr *Value, unsigned Size,
                                    SMLoc Loc) {
  MCELFStreamer::emitValueImpl(Value, Size, Loc);
  Labels.clear();
}

void Sw64ELFStreamer::emitIntValue(uint64_t Value, unsigned Size) {
  MCELFStreamer::emitIntValue(Value, Size);
  Labels.clear();
}

void Sw64ELFStreamer::EmitSw64OptionRecords() {
  for (const auto &I : Sw64OptionRecords)
    I->EmitSw64OptionRecord();
}

MCELFStreamer *llvm::createSw64ELFStreamer(
    MCContext &Context, std::unique_ptr<MCAsmBackend> MAB,
    std::unique_ptr<MCObjectWriter> OW, std::unique_ptr<MCCodeEmitter> Emitter,
    bool RelaxAll) {
  return new Sw64ELFStreamer(Context, std::move(MAB), std::move(OW),
                             std::move(Emitter));
}
