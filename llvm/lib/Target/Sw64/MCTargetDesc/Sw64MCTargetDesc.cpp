//===-- Sw64MCTargetDesc.cpp - Sw64 Target Descriptions -------------------===//
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

#include "Sw64MCTargetDesc.h"
#include "InstPrinter/Sw64InstPrinter.h"
#include "Sw64AsmBackend.h"
#include "Sw64ELFStreamer.h"
#include "Sw64MCAsmInfo.h"
#include "Sw64TargetStreamer.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;
namespace llvm {

class MCInstrInfo;

} // end namespace llvm
#define GET_INSTRINFO_MC_DESC
#include "Sw64GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Sw64GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "Sw64GenRegisterInfo.inc"

/// Select the Sw64 CPU for the given triple and cpu name.
/// FIXME: Merge with the copy in Sw64Subtarget.cpp
StringRef SW64_MC::selectSw64CPU(const Triple &TT, StringRef CPU) {
  return CPU = "sw_64";
}

static MCInstrInfo *createSw64MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitSw64MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createSw64MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitSw64MCRegisterInfo(X, Sw64::R26);
  return X;
}

static MCSubtargetInfo *createSw64MCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  CPU = SW64_MC::selectSw64CPU(TT, CPU);
  return createSw64MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCAsmInfo *createSw64MCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new Sw64MCAsmInfo(TT, Options);

  unsigned SP = MRI.getDwarfRegNum(Sw64::R30, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCInstPrinter *createSw64MCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new Sw64InstPrinter(MAI, MII, MRI);
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter,
                                    bool RelaxAll) {
  MCStreamer *S;
  S = createSw64ELFStreamer(Context, std::move(MAB), std::move(OW),
                            std::move(Emitter), RelaxAll);
  return S;
}

static MCTargetStreamer *createSw64AsmTargetStreamer(MCStreamer &S,
                                                     formatted_raw_ostream &OS,
                                                     MCInstPrinter *InstPrint,
                                                     bool isVerboseAsm) {
  return new Sw64TargetAsmStreamer(S, OS);
}

static MCTargetStreamer *createSw64NullTargetStreamer(MCStreamer &S) {
  return new Sw64TargetStreamer(S);
}

static MCTargetStreamer *
createSw64ObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new Sw64TargetELFStreamer(S, STI);
}

namespace {

class Sw64MCInstrAnalysis : public MCInstrAnalysis {
public:
  Sw64MCInstrAnalysis(const MCInstrInfo *Info) : MCInstrAnalysis(Info) {}

  bool evaluateBranch(const MCInst &Inst, uint64_t Addr, uint64_t Size,
                      uint64_t &Target) const override {
    unsigned NumOps = Inst.getNumOperands();
    if (NumOps == 0)
      return false;
    if (Inst.getOpcode() == Sw64::JSR || Inst.getOpcode() == Sw64::JSR) {
      Target = Inst.getOperand(NumOps - 1).getImm() != 0
                   ? Inst.getOperand(NumOps - 2).getImm()
                   : Addr + 4;
      return true;
    }
    switch (Info->get(Inst.getOpcode()).operands()[NumOps - 1].OperandType) {
    default:
      return false;
    case MCOI::OPERAND_PCREL:
      Target = Addr + Inst.getOperand(NumOps - 1).getImm() * 4 + 4;
      return true;
    }
  }
};
} // namespace

static MCInstrAnalysis *createSw64MCInstrAnalysis(const MCInstrInfo *Info) {
  return new Sw64MCInstrAnalysis(Info);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSw64TargetMC() {
  Target *T = &getTheSw64Target();

  // Register the MC asm info.
  RegisterMCAsmInfoFn X(*T, createSw64MCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(*T, createSw64MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(*T, createSw64MCRegisterInfo);

  // Register the elf streamer.
  TargetRegistry::RegisterELFStreamer(*T, createMCStreamer);

  // Register the asm target streamer.
  TargetRegistry::RegisterAsmTargetStreamer(*T, createSw64AsmTargetStreamer);

  TargetRegistry::RegisterNullTargetStreamer(*T, createSw64NullTargetStreamer);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(*T, createSw64MCSubtargetInfo);

  // Register the MC instruction analyzer.
  TargetRegistry::RegisterMCInstrAnalysis(*T, createSw64MCInstrAnalysis);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(*T, createSw64MCInstPrinter);

  TargetRegistry::RegisterObjectTargetStreamer(*T,
                                               createSw64ObjectTargetStreamer);

  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(*T, createSw64AsmBackend);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(*T, createSw64MCCodeEmitterEL);
}
