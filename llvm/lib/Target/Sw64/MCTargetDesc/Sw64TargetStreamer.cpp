//===-- Sw64TargetStreamer.cpp - Sw64 Target Streamer Methods -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Sw64 specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "Sw64TargetStreamer.h"
#include "InstPrinter/Sw64InstPrinter.h"
#include "MCTargetDesc/Sw64ABIInfo.h"
#include "MCTargetDesc/Sw64BaseInfo.h"
#include "MCTargetDesc/Sw64MCExpr.h"
#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "Sw64ELFStreamer.h"
#include "Sw64MCExpr.h"
#include "Sw64MCTargetDesc.h"
#include "Sw64TargetObjectFile.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;
namespace llvm {
struct Sw64InstrTable {
  MCInstrDesc Insts[4445];
  MCOperandInfo OperandInfo[3026];
  MCPhysReg ImplicitOps[130];
};
extern const Sw64InstrTable Sw64Descs;
} // end namespace llvm

namespace {
static cl::opt<bool> RoundSectionSizes(
    "sw_64-round-section-sizes", cl::init(false),
    cl::desc("Round section sizes up to the section alignment"), cl::Hidden);
} // end anonymous namespace

Sw64TargetStreamer::Sw64TargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S), ModuleDirectiveAllowed(true) {
  GPRInfoSet = FPRInfoSet = FrameInfoSet = false;
}
void Sw64TargetStreamer::emitDirectiveSetReorder() { forbidModuleDirective(); }
void Sw64TargetStreamer::emitDirectiveSetNoReorder() {}
void Sw64TargetStreamer::emitDirectiveSetMacro() { forbidModuleDirective(); }
void Sw64TargetStreamer::emitDirectiveSetNoMacro() { forbidModuleDirective(); }
void Sw64TargetStreamer::emitDirectiveSetAt() { forbidModuleDirective(); }
void Sw64TargetStreamer::emitDirectiveSetNoAt() { forbidModuleDirective(); }
void Sw64TargetStreamer::emitDirectiveEnd(StringRef Name) {}
void Sw64TargetStreamer::emitDirectiveEnt(const MCSymbol &Symbol) {}
void Sw64TargetStreamer::emitDirectiveNaN2008() {}
void Sw64TargetStreamer::emitDirectiveNaNLegacy() {}
void Sw64TargetStreamer::emitDirectiveInsn() { forbidModuleDirective(); }
void Sw64TargetStreamer::emitFrame(unsigned StackReg, unsigned StackSize,
                                   unsigned ReturnReg) {}

void Sw64TargetStreamer::emitDirectiveSetCore3b() {}
void Sw64TargetStreamer::emitDirectiveSetCore4() {}

void Sw64TargetAsmStreamer::emitDirectiveSetCore3b() {
  OS << "\t.arch= \t core3b\n";
  forbidModuleDirective();
}
void Sw64TargetAsmStreamer::emitDirectiveSetCore4() {
  OS << "\t.arch= \t core4\n";
  forbidModuleDirective();
}

void Sw64TargetStreamer::emitDirectiveSetArch(StringRef Arch) {
  forbidModuleDirective();
}

void Sw64TargetStreamer::emitNop(SMLoc IDLoc, const MCSubtargetInfo *STI) {}

Sw64TargetAsmStreamer::Sw64TargetAsmStreamer(MCStreamer &S,
                                             formatted_raw_ostream &OS)
    : Sw64TargetStreamer(S), OS(OS) {}

void Sw64TargetAsmStreamer::emitDirectiveSetReorder() {
  Sw64TargetStreamer::emitDirectiveSetReorder();
}

void Sw64TargetAsmStreamer::emitDirectiveSetNoReorder() {
  forbidModuleDirective();
}

void Sw64TargetAsmStreamer::emitDirectiveSetMacro() {
  Sw64TargetStreamer::emitDirectiveSetMacro();
}

void Sw64TargetAsmStreamer::emitDirectiveSetNoMacro() {
  Sw64TargetStreamer::emitDirectiveSetNoMacro();
}

void Sw64TargetAsmStreamer::emitDirectiveSetAt() {
  Sw64TargetStreamer::emitDirectiveSetAt();
}

void Sw64TargetAsmStreamer::emitDirectiveSetNoAt() {
  Sw64TargetStreamer::emitDirectiveSetNoAt();
}

void Sw64TargetAsmStreamer::emitDirectiveEnd(StringRef Name) {
  OS << "\t.end\t" << Name << '\n';
}

void Sw64TargetAsmStreamer::emitDirectiveEnt(const MCSymbol &Symbol) {
  OS << "\t.ent\t" << Symbol.getName() << '\n';
}

void Sw64TargetAsmStreamer::emitDirectiveNaN2008() { OS << "\t.nan\t2008\n"; }

void Sw64TargetAsmStreamer::emitDirectiveNaNLegacy() {
  OS << "\t.nan\tlegacy\n";
}

void Sw64TargetAsmStreamer::emitDirectiveInsn() {
  Sw64TargetStreamer::emitDirectiveInsn();
  OS << "\t.insn\n";
}

void Sw64TargetAsmStreamer::emitFrame(unsigned StackReg, unsigned StackSize,
                                      unsigned ReturnReg) {
  OS << "\t.frame\t$"
     << StringRef(Sw64InstPrinter::getRegisterName(StackReg)).lower() << ","
     << StackSize << ",$"
     << StringRef(Sw64InstPrinter::getRegisterName(ReturnReg)).lower() << '\n';
}

void Sw64TargetAsmStreamer::emitDirectiveSetArch(StringRef Arch) {
  OS << "\t.set arch=" << Arch << "\n";
  Sw64TargetStreamer::emitDirectiveSetArch(Arch);
}

// This part is for ELF object output.
Sw64TargetELFStreamer::Sw64TargetELFStreamer(MCStreamer &S,
                                             const MCSubtargetInfo &STI)
    : Sw64TargetStreamer(S), STI(STI) {
  MCAssembler &MCA = getStreamer().getAssembler();

  // It's possible that MCObjectFileInfo isn't fully initialized at this point
  // due to an initialization order problem where LLVMTargetMachine creates the
  // target streamer before TargetLoweringObjectFile calls
  // InitializeMCObjectFileInfo. There doesn't seem to be a single place that
  // covers all cases so this statement covers most cases and direct object
  // emission must call setPic() once MCObjectFileInfo has been initialized. The
  // cases we don't handle here are covered by Sw64AsmPrinter.
  Pic = MCA.getContext().getObjectFileInfo()->isPositionIndependent();

  // Set the header flags that we can in the constructor.
  // FIXME: This is a fairly terrible hack. We set the rest
  // of these in the destructor. The problem here is two-fold:
  //
  // a: Some of the eflags can be set/reset by directives.
  // b: There aren't any usage paths that initialize the ABI
  //    pointer until after we initialize either an assembler
  //    or the target machine.
  // We can fix this by making the target streamer construct
  // the ABI, but this is fraught with wide ranging dependency
  // issues as well.
  unsigned EFlags = MCA.getELFHeaderEFlags();

  // FIXME: Fix a dependency issue by instantiating the ABI object to some
  // default based off the triple. The triple doesn't describe the target
  // fully, but any external user of the API that uses the MCTargetStreamer
  // would otherwise crash on assertion failure.

  ABI = Sw64ABIInfo(Sw64ABIInfo::S64());

  MCA.setELFHeaderEFlags(EFlags);
}

void Sw64TargetELFStreamer::emitLabel(MCSymbol *S) {
  auto *Symbol = cast<MCSymbolELF>(S);
  getStreamer().getAssembler().registerSymbol(*Symbol);
  uint8_t Type = Symbol->getType();
  if (Type != ELF::STT_FUNC)
    return;
}

void Sw64TargetELFStreamer::finish() {
  MCAssembler &MCA = getStreamer().getAssembler();
  const MCObjectFileInfo &OFI = *MCA.getContext().getObjectFileInfo();

  // .bss, .text and .data are always at least 16-byte aligned.
  MCSection &TextSection = *OFI.getTextSection();
  MCA.registerSection(TextSection);
  MCSection &DataSection = *OFI.getDataSection();
  MCA.registerSection(DataSection);
  MCSection &BSSSection = *OFI.getBSSSection();
  MCA.registerSection(BSSSection);

  TextSection.ensureMinAlignment(Align(16));
  DataSection.ensureMinAlignment(Align(16));
  BSSSection.ensureMinAlignment(Align(16));

  if (RoundSectionSizes) {
    // Make sections sizes a multiple of the alignment. This is useful for
    // verifying the output of IAS against the output of other assemblers but
    // it's not necessary to produce a correct object and increases section
    // size.
    MCStreamer &OS = getStreamer();
    for (MCSection &S : MCA) {
      MCSectionELF &Section = static_cast<MCSectionELF &>(S);

      Align Alignment = Section.getAlign();
      OS.switchSection(&Section);
      if (Section.useCodeAlign())
        OS.emitCodeAlignment(Alignment, &STI, Alignment.value());
      else
        OS.emitValueToAlignment(Alignment, 0, 1, Alignment.value());
    }
  }

  // Update e_header flags. See the FIXME and comment above in
  // the constructor for a full rundown on this.
  unsigned EFlags = MCA.getELFHeaderEFlags();

  if (Pic)
    EFlags |= ELF::EF_SW64_PIC | ELF::EF_SW64_CPIC;

  MCA.setELFHeaderEFlags(EFlags);

  // Emit all the option records.
  // At the moment we are only emitting .Sw64.options (ODK_REGINFO) and
  // .reginfo.
  Sw64ELFStreamer &MEF = static_cast<Sw64ELFStreamer &>(Streamer);
  MEF.EmitSw64OptionRecords();
}

MCELFStreamer &Sw64TargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}

void Sw64TargetELFStreamer::emitDirectiveSetNoReorder() {
  MCAssembler &MCA = getStreamer().getAssembler();
  unsigned Flags = MCA.getELFHeaderEFlags();
  Flags |= ELF::EF_SW64_NOREORDER;
  MCA.setELFHeaderEFlags(Flags);
  forbidModuleDirective();
}

void Sw64TargetELFStreamer::emitDirectiveEnt(const MCSymbol &Symbol) {
  GPRInfoSet = FPRInfoSet = FrameInfoSet = false;

  // .ent also acts like an implicit '.type symbol, STT_FUNC'
  static_cast<const MCSymbolELF &>(Symbol).setType(ELF::STT_FUNC);
}

void Sw64TargetELFStreamer::emitDirectiveNaN2008() {
  MCAssembler &MCA = getStreamer().getAssembler();
  unsigned Flags = MCA.getELFHeaderEFlags();
  Flags |= ELF::EF_SW64_NAN2008;
  MCA.setELFHeaderEFlags(Flags);
}

void Sw64TargetELFStreamer::emitDirectiveNaNLegacy() {
  MCAssembler &MCA = getStreamer().getAssembler();
  unsigned Flags = MCA.getELFHeaderEFlags();
  Flags &= ~ELF::EF_SW64_NAN2008;
  MCA.setELFHeaderEFlags(Flags);
}

void Sw64TargetELFStreamer::emitDirectiveInsn() {
  Sw64TargetStreamer::emitDirectiveInsn();
  Sw64ELFStreamer &MEF = static_cast<Sw64ELFStreamer &>(Streamer);
  MEF.createPendingLabelRelocs();
}

void Sw64TargetELFStreamer::emitFrame(unsigned StackReg, unsigned StackSize,
                                      unsigned ReturnReg_) {
  MCContext &Context = getStreamer().getAssembler().getContext();
  const MCRegisterInfo *RegInfo = Context.getRegisterInfo();

  FrameInfoSet = true;
  FrameReg = RegInfo->getEncodingValue(StackReg);
  FrameOffset = StackSize;
  ReturnReg = RegInfo->getEncodingValue(ReturnReg_);
}

static const char *getRelType(const MCExpr *Expr, const MCSubtargetInfo &STI) {
  const Sw64MCExpr *Sw64Expr = cast<Sw64MCExpr>(Expr);
  static int curgpdist = 0;
  switch (Sw64Expr->getKind()) {
  default:
    return "";
  case Sw64MCExpr::MEK_GPDISP_HI16:
  case Sw64MCExpr::MEK_GPDISP_LO16:
  case Sw64MCExpr::MEK_GPDISP: {
    std::string a =
        std::string("!gpdisp!") + std::to_string((curgpdist) / 2 + 1);
    curgpdist++;
    return strdup(a.c_str());
  }
  case Sw64MCExpr::MEK_ELF_LITERAL:
    return "!literal";
  case Sw64MCExpr::MEK_LITUSE_ADDR: /* !lituse_addr relocation.  */
    return "!lituse_addr";
  case Sw64MCExpr::MEK_LITUSE_BASE: /* !lituse_base relocation.  */
    return "!literal";
  case Sw64MCExpr::MEK_LITUSE_BYTOFF: /* !lituse_bytoff relocation.  */
    return "!lituse_bytoff";
  case Sw64MCExpr::MEK_LITUSE_JSR: /* !lituse_jsr relocation.  */
    return "!lituse_jsr";
  case Sw64MCExpr::MEK_LITUSE_TLSGD: /* !lituse_tlsgd relocation.  */
    return "!lituse_tlsgd";
  case Sw64MCExpr::MEK_LITUSE_TLSLDM: /* !lituse_tlsldm relocation.  */
    return "!lituse_tlsldm";
    //  case Sw64MCExpr::MEK_LITUSE_JSRDIRECT: /* !lituse_jsrdirect relocation.
    //  */
    //    return "!lituse_jsrdirect";
  case Sw64MCExpr::MEK_GPREL_HI16: /* !gprelhigh relocation.  */
    return "!gprelhigh";
  case Sw64MCExpr::MEK_GPREL_LO16: /* !gprellow relocation.  */
    return "!gprellow";
  case Sw64MCExpr::MEK_GPREL16: /* !gprel relocation.  */
    return "!gprel";
  case Sw64MCExpr::MEK_BRSGP: /* !samegp relocation.  */
    return "!samegp";
  case Sw64MCExpr::MEK_TLSGD: /* !tlsgd relocation.  */
    return "!tlsgd";
  case Sw64MCExpr::MEK_TLSLDM: /* !tlsldm relocation.  */
    return "!tlsldm";
  case Sw64MCExpr::MEK_GOTDTPREL16: /* !gotdtprel relocation.  */
    return "!gotdtprel";
  case Sw64MCExpr::MEK_DTPREL_HI16: /* !dtprelhi relocation.  */
    return "!dtprelhi";
  case Sw64MCExpr::MEK_DTPREL_LO16: /* !dtprello relocation.  */
    return "!dtprello";
  case Sw64MCExpr::MEK_DTPREL16: /* !dtprel relocation.  */
    return "!dtprel";
  case Sw64MCExpr::MEK_GOTTPREL16: /* !gottprel relocation.  */
    return "!gottprel";
  case Sw64MCExpr::MEK_TPREL_HI16: /* !tprelhi relocation.  */
    return "!tprelhi";
  case Sw64MCExpr::MEK_TPREL_LO16: /* !tprello relocation.  */
    return "!tprello";
  case Sw64MCExpr::MEK_TPREL16: /* !tprel relocation.  */
    return "!tprel";
  case Sw64MCExpr::MEK_ELF_LITERAL_GOT: /* !literal_got relocation.  */
    return "!literal_got";
  }
}

static void printRelocInst(MCInstPrinter &InstPrinter, const MCInst &Inst,
                           raw_ostream &OS, const MCSubtargetInfo &STI,
                           uint64_t Address) {
  MCOperand Op = Inst.getOperand(1);
  if (Op.isExpr()) {
    const MCExpr *Expr = Op.getExpr();
    if (Expr->getKind() == MCExpr::Target) {
      const char *RelName = getRelType(Expr, STI);
      InstPrinter.printInst(&Inst, Address, RelName, STI, OS);
      return;
    }
  }
  InstPrinter.printInst(&Inst, Address, "", STI, OS);
}

void Sw64TargetStreamer::prettyPrintAsm(MCInstPrinter &InstPrinter,
                                        uint64_t Address, const MCInst &Inst,
                                        const MCSubtargetInfo &STI,
                                        raw_ostream &OS) {
  const MCInstrDesc &MCID =
      Sw64Descs.Insts[Sw64::INSTRUCTION_LIST_END - 1 - Inst.getOpcode()];
  // while moving mayload flags for ldi/ldih
  // adding opcode determine here
  if (MCID.mayLoad() || MCID.mayStore() || Inst.getOpcode() == Sw64::LDAH ||
      Inst.getOpcode() == Sw64::LDA) {
    printRelocInst(InstPrinter, Inst, OS, STI, Address);
    return;
  }
  InstPrinter.printInst(&Inst, Address, "", STI, OS);
}
