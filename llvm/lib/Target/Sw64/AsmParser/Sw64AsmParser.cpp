//===-- Sw64AsmParser.cpp - Parse Sw64 assembly to MCInst instructions ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Sw64ABIFlagsSection.h"
#include "MCTargetDesc/Sw64ABIInfo.h"
#include "MCTargetDesc/Sw64BaseInfo.h"
#include "MCTargetDesc/Sw64MCExpr.h"
#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "Sw64TargetStreamer.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCAsmParserExtension.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/SubtargetFeature.h"
#include "llvm/TargetParser/Triple.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

using namespace llvm;

#define DEBUG_TYPE "sw_64-asm-parser"

static const StringRef RelocTable[] = {
    "literal",   "lituse_addr", "lituse_jsr", "gpdisp",
    "gprelhigh", "gprellow",    "gprel",      "tlsgd",
    "tlsldm",    "gotdtprel",   "dtprelhi",   "dtprello",
    "gottprel",  "tprelhi",     "tprello",    "tprel"};

namespace llvm {

class MCInstrInfo;

} // end namespace llvm

namespace {

class Sw64AssemblerOptions {
public:
  Sw64AssemblerOptions(const FeatureBitset &Features_) : Features(Features_) {}

  Sw64AssemblerOptions(const Sw64AssemblerOptions *Opts) {
    ATReg = Opts->getATRegIndex();
    Reorder = Opts->isReorder();
    Macro = Opts->isMacro();
    Features = Opts->getFeatures();
  }

  unsigned getATRegIndex() const { return ATReg; }
  bool setATRegIndex(unsigned Reg) {
    if (Reg > 31)
      return false;

    ATReg = Reg;
    return true;
  }

  bool isReorder() const { return Reorder; }
  void setReorder() { Reorder = true; }
  void setNoReorder() { Reorder = false; }

  bool isMacro() const { return Macro; }
  void setMacro() { Macro = true; }
  void setNoMacro() { Macro = false; }

  const FeatureBitset &getFeatures() const { return Features; }
  void setFeatures(const FeatureBitset &Features_) { Features = Features_; }

  // Set of features that are either architecture features or referenced
  // by them (e.g.: FeatureNaN2008 implied by FeatureSw6432r6).
  // The full table can be found in Sw64GenSubtargetInfo.inc (Sw64FeatureKV[]).
  // The reason we need this mask is explained in the selectArch function.
  // FIXME: Ideally we would like TableGen to generate this information.
  static const FeatureBitset AllArchRelatedMask;

private:
  unsigned ATReg = 1;
  bool Reorder = true;
  bool Macro = true;
  FeatureBitset Features;
};

} // end anonymous namespace

const FeatureBitset Sw64AssemblerOptions::AllArchRelatedMask = {
    Sw64::FeatureCIX, Sw64::Featurecore3b, Sw64::Featurecore4,
    Sw64::FeatureRelax, Sw64::FeatureEv};

namespace {

class Sw64AsmParser : public MCTargetAsmParser {
  Sw64TargetStreamer &getTargetStreamer() {
    MCTargetStreamer &TS = *getParser().getStreamer().getTargetStreamer();
    return static_cast<Sw64TargetStreamer &>(TS);
  }

  Sw64ABIInfo ABI;
  SmallVector<std::unique_ptr<Sw64AssemblerOptions>, 2> AssemblerOptions;
  MCSymbol *CurrentFn; // Pointer to the function being parsed. It may be a
                       // nullptr, which indicates that no function is currently
                       // selected. This usually happens after an '.end func'
                       // directive.
  bool IsLittleEndian;
  bool IsPicEnabled;
  bool IsCpRestoreSet;
  int CpRestoreOffset;
  unsigned CpSaveLocation;
  // If true, then CpSaveLocation is a register, otherwise it's an offset.
  bool CpSaveLocationIsRegister;

  // Map of register aliases created via the .set directive.
  StringMap<AsmToken> RegisterSets;

#define GET_ASSEMBLER_HEADER
#include "Sw64GenAsmMatcher.inc"

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  // Parse a register as used in CFI directives
  bool parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                     SMLoc &EndLoc) override;

  OperandMatchResultTy tryParseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;

  bool parseParenSuffix(StringRef Name, OperandVector &Operands);

  bool mnemonicIsValid(StringRef Mnemonic, unsigned VariantID);

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool ParseDirective(AsmToken DirectiveID) override;

  OperandMatchResultTy
  matchAnyRegisterNameWithoutDollar(OperandVector &Operands,
                                    StringRef Identifier, SMLoc S);
  OperandMatchResultTy matchAnyRegisterWithoutDollar(OperandVector &Operands,
                                                     const AsmToken &Token,
                                                     SMLoc S);
  OperandMatchResultTy matchAnyRegisterWithoutDollar(OperandVector &Operands,
                                                     SMLoc S);
  OperandMatchResultTy parseAnyRegister(OperandVector &Operands);
  OperandMatchResultTy parseMemOperand(OperandVector &Operands);
  OperandMatchResultTy parseMemOperands(OperandVector &Operands);
  OperandMatchResultTy parseJmpImm(OperandVector &Operands);

  bool searchSymbolAlias(OperandVector &Operands);

  bool parseOperand(OperandVector &, StringRef Mnemonic);

  void ParsingFixupOperands(std::pair<StringRef, unsigned> reloc);

  enum MacroExpanderResultTy {
    MER_NotAMacro,
    MER_Success,
    MER_Fail,
  };

  unsigned validateTargetOperandClass(MCParsedAsmOperand &Op,
                                      unsigned Kind) override;

  bool loadAndAddSymbolAddress(const MCExpr *SymExpr, unsigned DstReg,
                               unsigned SrcReg, bool Is32BitSym, SMLoc IDLoc,
                               MCStreamer &Out, const MCSubtargetInfo *STI);

  void expandMemInst(MCInst &Inst, SMLoc IDLoc, MCStreamer &Out,
                     const MCSubtargetInfo *STI, bool IsLoad);

  bool reportParseError(Twine ErrorMsg);

  bool parseMemOffset(const MCExpr *&Res, bool isParenExpr);

  bool isEvaluated(const MCExpr *Expr);
  bool parseSetArchDirective();
  bool parseDirectiveSet();

  bool parseSetAtDirective();
  bool parseSetNoAtDirective();
  bool parseSetMacroDirective();
  bool parseSetNoMacroDirective();
  bool parseSetReorderDirective();
  bool parseSetNoReorderDirective();

  bool parseSetAssignment();

  bool parseFpABIValue(Sw64ABIFlagsSection::FpABIKind &FpABI,
                       StringRef Directive);

  int matchCPURegisterName(StringRef Symbol);

  int matchFPURegisterName(StringRef Name);

  bool processInstruction(MCInst &Inst, SMLoc IDLoc, MCStreamer &Out,
                          const MCSubtargetInfo *STI);

  // Helper function that checks if the value of a vector index is within the
  // boundaries of accepted values for each RegisterKind
  // Example: INSERT.B $w0[n], $1 => 16 > n >= 0
  bool validateMSAIndex(int Val, int RegKind);

  // Selects a new architecture by updating the FeatureBits with the necessary
  // info including implied dependencies.
  // Internally, it clears all the feature bits related to *any* architecture
  // and selects the new one using the ToggleFeature functionality of the
  // MCSubtargetInfo object that handles implied dependencies. The reason we
  // clear all the arch related bits manually is because ToggleFeature only
  // clears the features that imply the feature being cleared and not the
  // features implied by the feature being cleared. This is easier to see
  // with an example:
  //  --------------------------------------------------
  // | Feature         | Implies                        |
  // | -------------------------------------------------|
  // | FeatureCIX      |                                |
  // | FeatureEV       |                                |
  // | FeatureSw6a     |                                |
  // | FeatureSw6b     |                                |
  // | ...             |                                |
  //  --------------------------------------------------
  //
  // Setting Sw643 is equivalent to set: (FeatureSw643 | FeatureSw642 |
  // FeatureSw64GP64 | FeatureSw641)
  // Clearing Sw643 is equivalent to clear (FeatureSw643 | FeatureSw644).
  void selectArch(StringRef ArchFeature) {
    MCSubtargetInfo &STI = copySTI();
    FeatureBitset FeatureBits = STI.getFeatureBits();
    FeatureBits &= ~Sw64AssemblerOptions::AllArchRelatedMask;
    STI.setFeatureBits(FeatureBits);
    setAvailableFeatures(
        ComputeAvailableFeatures(STI.ToggleFeature(ArchFeature)));
    AssemblerOptions.back()->setFeatures(STI.getFeatureBits());
  }

  void setFeatureBits(uint64_t Feature, StringRef FeatureString) {
    if (!(getSTI().getFeatureBits()[Feature])) {
      MCSubtargetInfo &STI = copySTI();
      setAvailableFeatures(
          ComputeAvailableFeatures(STI.ToggleFeature(FeatureString)));
      AssemblerOptions.back()->setFeatures(STI.getFeatureBits());
    }
  }

  void clearFeatureBits(uint64_t Feature, StringRef FeatureString) {
    if (getSTI().getFeatureBits()[Feature]) {
      MCSubtargetInfo &STI = copySTI();
      setAvailableFeatures(
          ComputeAvailableFeatures(STI.ToggleFeature(FeatureString)));
      AssemblerOptions.back()->setFeatures(STI.getFeatureBits());
    }
  }

  void setModuleFeatureBits(uint64_t Feature, StringRef FeatureString) {
    setFeatureBits(Feature, FeatureString);
    AssemblerOptions.front()->setFeatures(getSTI().getFeatureBits());
  }

  void clearModuleFeatureBits(uint64_t Feature, StringRef FeatureString) {
    clearFeatureBits(Feature, FeatureString);
    AssemblerOptions.front()->setFeatures(getSTI().getFeatureBits());
  }

public:
  MCFixupKind FixupKind;

  enum Sw64MatchResultTy {
    Match_RequiresDifferentSrcAndDst = FIRST_TARGET_MATCH_RESULT_TY,
    Match_RequiresDifferentOperands,
    Match_RequiresNoZeroRegister,
    Match_RequiresSameSrcAndDst,
    Match_NoFCCRegisterForCurrentISA,
    Match_NonZeroOperandForSync,
    Match_NonZeroOperandForMTCX,
    Match_RequiresPosSizeRange0_32,
    Match_RequiresPosSizeRange33_64,
    Match_RequiresPosSizeUImm6,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "Sw64GenAsmMatcher.inc"
#undef GET_OPERAND_DIAGNOSTIC_TYPES
  };

  Sw64AsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser,
                const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, sti, MII),
        ABI(Sw64ABIInfo::computeTargetABI(Triple(sti.getTargetTriple()),
                                          sti.getCPU(), Options)) {
    FixupKind = llvm::FirstTargetFixupKind;

    MCAsmParserExtension::Initialize(parser);
    parser.addAliasForDirective(".asciiz", ".asciz");
    parser.addAliasForDirective(".hword", ".2byte");
    parser.addAliasForDirective(".word", ".4byte");
    parser.addAliasForDirective(".dword", ".8byte");

    // Initialize the set of available features.
    setAvailableFeatures(ComputeAvailableFeatures(getSTI().getFeatureBits()));

    // Remember the initial assembler options. The user can not modify these.
    AssemblerOptions.push_back(
        std::make_unique<Sw64AssemblerOptions>(getSTI().getFeatureBits()));

    // Create an assembler options environment for the user to modify.
    AssemblerOptions.push_back(
        std::make_unique<Sw64AssemblerOptions>(getSTI().getFeatureBits()));

    CurrentFn = nullptr;

    IsPicEnabled = getContext().getObjectFileInfo()->isPositionIndependent();

    IsCpRestoreSet = false;
    CpRestoreOffset = -1;
  }

  const Sw64ABIInfo &getABI() const { return ABI; }

  const MCExpr *createTargetUnaryExpr(const MCExpr *E,
                                      AsmToken::TokenKind OperatorToken,
                                      MCContext &Ctx) override {
    switch (OperatorToken) {
    default:
      return nullptr;
    case AsmToken::PercentGp_Rel:
      return Sw64MCExpr::create(Sw64MCExpr::MEK_ELF_LITERAL, E, Ctx);
    case AsmToken::PercentDtprel_Hi:
      return Sw64MCExpr::create(Sw64MCExpr::MEK_GPREL_HI16, E, Ctx);
    case AsmToken::PercentDtprel_Lo:
      return Sw64MCExpr::create(Sw64MCExpr::MEK_GPREL_LO16, E, Ctx);
    case AsmToken::PercentGot_Hi:
      return Sw64MCExpr::create(Sw64MCExpr::MEK_GPDISP_HI16, E, Ctx);
    case AsmToken::PercentGot_Lo:
      return Sw64MCExpr::create(Sw64MCExpr::MEK_GPDISP_LO16, E, Ctx);

    case AsmToken::PercentTprel_Hi:
      return Sw64MCExpr::create(Sw64MCExpr::MEK_TPREL_HI16, E, Ctx);
    case AsmToken::PercentTprel_Lo:
      return Sw64MCExpr::create(Sw64MCExpr::MEK_TPREL_LO16, E, Ctx);
    }
  }
};

// Sw64Operand - Instances of this class represent a parsed Sw64 machine
// instruction.
class Sw64Operand : public MCParsedAsmOperand {
public:
  // Broad categories of register classes
  // The exact class is finalized by the render method.
  enum RegKind {
    RegKind_GPR = 1,   // Sw64 GPR Register
    RegKind_FPR = 2,   // Sw64 FPR Register
    RegKind_TC = 4,    //  Sw64 Time counter
    RegKind_CSR = 8,   // Sw64 Control & Status Register
    RegKind_FPCR = 16, // Sw64 Floating-point Control Register
                       // Potentially any (e.g. $1)
    RegKind_Numeric =
        RegKind_GPR | RegKind_FPR | RegKind_TC | RegKind_CSR | RegKind_FPCR
  };

private:
  enum KindTy {
    k_Immediate,     // An immediate (possibly involving symbol references)
    k_Memory,        // Base + Offset Memory Address
    k_Register,      // A RegKind.
    k_RegisterIndex, // A register index in one or more RegKind.
    k_Token          // A simple token
  } Kind;

public:
  Sw64Operand(KindTy K, Sw64AsmParser &Parser)
      : MCParsedAsmOperand(), Kind(K), AsmParser(Parser) {}

  ~Sw64Operand() override {
    switch (Kind) {
    case k_Immediate:
      break;
    case k_Memory:
      delete Mem.Base;
      break;
    case k_Register:
    case k_RegisterIndex:
    case k_Token:
      break;
    }
  }

private:
  // For diagnostics, and checking the assembler temporary
  Sw64AsmParser &AsmParser;

  struct Token {
    const char *Data;
    unsigned Length;
  };

  struct RegIdxOp {
    unsigned Index;   // Index into the register class
    RegKind Kind;     // Bitfield of the kinds it could possibly be
    struct Token Tok; // The input token this operand originated from.
    const MCRegisterInfo *RegInfo;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  struct MemOp {
    Sw64Operand *Base;
    const MCExpr *Off;
  };

  struct RegListOp {
    SmallVector<unsigned, 10> *List;
  };

  union {
    struct Token Tok;
    struct RegIdxOp RegIdx;
    struct ImmOp Imm;
    struct MemOp Mem;
    struct RegListOp RegList;
  };

  SMLoc StartLoc, EndLoc;

  // Internal constructor for register kinds
  static std::unique_ptr<Sw64Operand> CreateReg(unsigned Index, StringRef Str,
                                                RegKind RegKind,
                                                const MCRegisterInfo *RegInfo,
                                                SMLoc S, SMLoc E,
                                                Sw64AsmParser &Parser) {
    auto Op = std::make_unique<Sw64Operand>(k_Register, Parser);
    Op->RegIdx.Index = Index;
    Op->RegIdx.RegInfo = RegInfo;
    Op->RegIdx.Kind = RegKind;
    Op->RegIdx.Tok.Data = Str.data();
    Op->RegIdx.Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

public:
  // Coerce the register to GPR64 and return the real register for the current
  // target.
  unsigned getGPRReg() const {
    assert(isRegIdx() && (RegIdx.Kind & RegKind_GPR) && "Invalid access!");
    return RegIdx.Index;
  }

  bool isV256AsmReg() const {
    return isRegIdx() && RegIdx.Kind & RegKind_FPR &&
           RegIdx.Index <= Sw64::F31 && RegIdx.Index >= Sw64::F0;
  }

  void addMemOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands!");

    Inst.addOperand(MCOperand::createReg(getMemBase()->getGPR64Reg()));

    const MCExpr *Expr = getMemOff();
    addExpr(Inst, Expr);
  }

private:
  // Coerce the register to FPR64 and return the real register for the current
  // target.
  unsigned getFPR64Reg() const {
    assert(isRegIdx() && (RegIdx.Kind & RegKind_FPR) && "Invalid access!");
    return RegIdx.Index;
  }

public:
  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    // Add as immediate when possible.  Null MCExpr = 0.
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    if (RegIdx.Index > 32)
      Inst.addOperand(MCOperand::createReg(getGPRReg()));
    else
      Inst.addOperand(MCOperand::createReg(getFPR64Reg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr *Expr = getImm();
    addExpr(Inst, Expr);
  }

  bool isReg() const override {
    // As a special case until we sort out the definition of div/divu, accept
    // $0/$zero here so that MCK_ZERO works correctly.
    return isGPRAsmReg() || isFPRAsmReg();
  }

  bool isRegIdx() const { return Kind == k_Register; } // Operand.Kind
  bool isImm() const override { return Kind == k_Immediate; }

  bool isConstantImm() const {
    int64_t Res;
    return isImm() && getImm()->evaluateAsAbsolute(Res);
  }

  bool isToken() const override {
    // Note: It's not possible to pretend that other operand kinds are tokens.
    // The matcher emitter checks tokens first.
    return Kind == k_Token;
  }

  bool isMem() const override { return Kind == k_Memory; }

  StringRef getToken() const {
    assert(Kind == k_Token && "Invalid access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  unsigned getReg() const override {
    // As a special case until we sort out the definition of div/divu, accept
    // $0/$zero here so that MCK_ZERO works correctly.
    if (Kind == k_Register && RegIdx.Kind & RegKind_GPR)
      return getGPRReg(); // FIXME: GPR64 too

    if (Kind == k_Register && RegIdx.Kind & RegKind_FPR)
      return getFPR64Reg(); // FIXME: GPR64 too

    llvm_unreachable("Invalid access!");
    return 0;
  }

  const MCExpr *getImm() const {
    assert((Kind == k_Immediate) && "Invalid access!");
    return Imm.Val;
  }

  int64_t getConstantImm() const {
    const MCExpr *Val = getImm();
    int64_t Value = 0;
    (void)Val->evaluateAsAbsolute(Value);
    return Value;
  }

  Sw64Operand *getMemBase() const {
    assert((Kind == k_Memory) && "Invalid access!");
    return Mem.Base;
  }

  const MCExpr *getMemOff() const {
    assert((Kind == k_Memory) && "Invalid access!");
    return Mem.Off;
  }

  int64_t getConstantMemOff() const {
    return static_cast<const MCConstantExpr *>(getMemOff())->getValue();
  }

  static std::unique_ptr<Sw64Operand> CreateToken(StringRef Str, SMLoc S,
                                                  Sw64AsmParser &Parser) {
    auto Op = std::make_unique<Sw64Operand>(k_Token, Parser);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  // Create a numeric register (e.g. $1). The exact register remains
  // unresolved until an instruction successfully matches
  static std::unique_ptr<Sw64Operand>
  createNumericReg(unsigned Index, StringRef Str, const MCRegisterInfo *RegInfo,
                   SMLoc S, SMLoc E, Sw64AsmParser &Parser) {
    LLVM_DEBUG(dbgs() << "createNumericReg(" << Index + 65 << ", ...)\n");
    return CreateReg(Index + 65, Str, RegKind_Numeric, RegInfo, S, E, Parser);
  }

  // Create a register that is definitely a GPR.
  // This is typically only used for named registers such as $gp.
  static std::unique_ptr<Sw64Operand>
  createGPRReg(unsigned Index, StringRef Str, const MCRegisterInfo *RegInfo,
               SMLoc S, SMLoc E, Sw64AsmParser &Parser) {
    return CreateReg(Index, Str, RegKind_GPR, RegInfo, S, E, Parser);
  }

  // Create a register that is definitely a FPR.
  // This is typically only used for named registers such as $f0.
  static std::unique_ptr<Sw64Operand>
  createFPRReg(unsigned Index, StringRef Str, const MCRegisterInfo *RegInfo,
               SMLoc S, SMLoc E, Sw64AsmParser &Parser) {
    return CreateReg(Index, Str, RegKind_FPR, RegInfo, S, E, Parser);
  }

  static std::unique_ptr<Sw64Operand>
  CreateImm(const MCExpr *Val, SMLoc S, SMLoc E, Sw64AsmParser &Parser) {
    auto Op = std::make_unique<Sw64Operand>(k_Immediate, Parser);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<Sw64Operand>
  CreateMem(std::unique_ptr<Sw64Operand> Base, const MCExpr *Off, SMLoc S,
            SMLoc E, Sw64AsmParser &Parser) {
    auto Op = std::make_unique<Sw64Operand>(k_Memory, Parser);
    Op->Mem.Base = Base.release();
    Op->Mem.Off = Off;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  bool isGPRAsmReg() const {
    return isRegIdx() && RegIdx.Kind & RegKind_GPR &&
           RegIdx.Index <= Sw64::R31 && RegIdx.Index >= Sw64::R0;
  }

  bool isFPRAsmReg() const {
    // AFPR64 is $0-$15 but we handle this in getAFGR64()
    return isRegIdx() && RegIdx.Kind & RegKind_FPR &&
           RegIdx.Index <= Sw64::F31 && RegIdx.Index >= Sw64::F0;
    // return isRegIdx() && RegIdx.Kind & RegKind_GPR && RegIdx.Index <= 64 &&
    //        RegIdx.Index >= 33;
  }

  // Coerce the register to GPR64 and return the real register for the current
  // target.
  unsigned getGPR64Reg() const {
    assert(isRegIdx() && (RegIdx.Kind & RegKind_GPR) && "Invalid access!");
    return RegIdx.Index;
  }

  unsigned getFGR64Reg() const {
    assert(isRegIdx() && (RegIdx.Kind & RegKind_FPR) && "Invalid access!");
    return RegIdx.Index;
  }

  void addF4RCAsmRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getFGR64Reg()));
  }

  void addF8RCAsmRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getFGR64Reg()));
  }

  bool isFGRAsmReg() const {
    return isRegIdx() && RegIdx.Kind & RegKind_FPR && RegIdx.Index <= 32;
  }

  // getStartLoc - Get the location of the first token of this operand.
  SMLoc getStartLoc() const override { return StartLoc; }
  // getEndLoc - Get the location of the last token of this operand.
  SMLoc getEndLoc() const override { return EndLoc; }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case k_Immediate:
      OS << "Imm<";
      OS << *Imm.Val;
      OS << ">";
      break;
    case k_Memory:
      OS << "Mem<";
      Mem.Base->print(OS);
      OS << ", ";
      OS << *Mem.Off;
      OS << ">";
      break;
    case k_Register:
      OS << "Reg<" << RegIdx.Kind << ", "
         << StringRef(RegIdx.Tok.Data, RegIdx.Tok.Length) << ">";
      break;
    case k_RegisterIndex:
      OS << "RegIdx<" << RegIdx.Index << ":" << RegIdx.Kind << ", "
         << StringRef(RegIdx.Tok.Data, RegIdx.Tok.Length) << ">";
      break;
    case k_Token:
      OS << getToken();
      break;
    }
  }

  bool isValidForTie(const Sw64Operand &Other) const {
    if (Kind != Other.Kind)
      return false;

    switch (Kind) {
    default:
      llvm_unreachable("Unexpected kind");
      return false;
    case k_RegisterIndex: {
      StringRef Token(RegIdx.Tok.Data, RegIdx.Tok.Length);
      StringRef OtherToken(Other.RegIdx.Tok.Data, Other.RegIdx.Tok.Length);
      return Token == OtherToken;
    }
    }
  }

  template <unsigned Bits, unsigned ShiftLeftAmount> bool isScaledSImm() const {
    if (isConstantImm() &&
        isShiftedInt<Bits, ShiftLeftAmount>(getConstantImm()))
      return true;
    // Operand can also be a symbol or symbol plus
    // offset in case of relocations.
    if (Kind != k_Immediate)
      return false;
    MCValue Res;
    bool Success = getImm()->evaluateAsRelocatable(Res, nullptr, nullptr);
    return Success && isShiftedInt<Bits, ShiftLeftAmount>(Res.getConstant());
  }

  template <unsigned Bits, int Offset = 0, int AdjustOffset = 0>
  void addConstantSImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    int64_t Imm = getConstantImm() - Offset;
    Imm = SignExtend64<Bits>(Imm);
    Imm += Offset;
    Imm += AdjustOffset;
    Inst.addOperand(MCOperand::createImm(Imm));
  }

  template <unsigned Bits, int Offset = 0, int AdjustOffset = 0>
  void addConstantUImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    uint64_t Imm = getConstantImm() - Offset;
    Imm &= (1ULL << Bits) - 1;
    Imm += Offset;
    Imm += AdjustOffset;
    Inst.addOperand(MCOperand::createImm(Imm));
  }

  template <unsigned Bottom, unsigned Top> bool isConstantUImmRange() const {
    return isConstantImm() && getConstantImm() >= Bottom &&
           getConstantImm() <= Top;
  }

  template <unsigned Bits, unsigned ShiftLeftAmount> bool isScaledUImm() const {
    return isConstantImm() &&
           isShiftedUInt<Bits, ShiftLeftAmount>(getConstantImm());
  }

  template <unsigned Bits, int Offset = 0> bool isConstantSImm() const {
    return isConstantImm() && isInt<Bits>(getConstantImm() - Offset);
  }

  template <unsigned Bits, int Offset = 0> bool isConstantUImm() const {
    return isConstantImm() && isUInt<Bits>(getConstantImm() - Offset);
  }

  // Coerce the register to SIMD and return the real register for the current
  // target.
  unsigned getV256Reg() const {
    assert(isRegIdx() && (RegIdx.Kind & RegKind_FPR) && "Invalid access!");
    // It doesn't matter which of the MSA128[BHWD] classes we use. They are all
    // identical
    unsigned ClassID = Sw64::V256LRegClassID;
    // RegIdx.Index should be sub 1, or it will be error. such as: $f1 -> $f2
    return RegIdx.RegInfo->getRegClass(ClassID).getRegister(RegIdx.Index - 1);
  }

  void addV256AsmRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getV256Reg()));
  }

  bool isConstantMemOff() const {
    return isMem() && isa<MCConstantExpr>(getMemOff());
  }

  // Allow relocation operators.
  // FIXME: This predicate and others need to look through binary expressions
  //        and determine whether a Value is a constant or not.
  template <unsigned Bits, unsigned ShiftAmount = 0>
  bool isMemWithSimmOffset() const {
    if (!isMem())
      return false;
    if (!getMemBase()->isGPRAsmReg())
      return false;
    if (isa<MCTargetExpr>(getMemOff()) ||
        (isConstantMemOff() &&
         isShiftedInt<Bits, ShiftAmount>(getConstantMemOff())))
      return true;
    MCValue Res;
    bool IsReloc = getMemOff()->evaluateAsRelocatable(Res, nullptr, nullptr);
    return IsReloc && isShiftedInt<Bits, ShiftAmount>(Res.getConstant());
  }

  template <unsigned Bits> bool isSImm() const {
    return isConstantImm() ? isInt<Bits>(getConstantImm()) : isImm();
  }

  template <unsigned Bits> bool isUImm() const {
    return isConstantImm() ? isUInt<Bits>(getConstantImm()) : isImm();
  }

  template <unsigned Bits> bool isAnyImm() const {
    return isConstantImm() ? (isInt<Bits>(getConstantImm()) ||
                              isUInt<Bits>(getConstantImm()))
                           : isImm();
  }

}; // class Sw64Operand

} // end anonymous namespace

namespace llvm {} // end namespace llvm

bool Sw64AsmParser::processInstruction(MCInst &Inst, SMLoc IDLoc,
                                       MCStreamer &Out,
                                       const MCSubtargetInfo *STI) {
  const MCInstrDesc &MCID = MII.get(Inst.getOpcode());
  Inst.setLoc(IDLoc);

  if (MCID.mayLoad() || MCID.mayStore()) {
    // Check the offset of memory operand, if it is a symbol
    // reference or immediate we may have to expand instructions.
    const MCOperandInfo &OpInfo = MCID.operands()[1];
    if ((OpInfo.OperandType == MCOI::OPERAND_MEMORY) ||
        (OpInfo.OperandType == MCOI::OPERAND_UNKNOWN)) {
      MCOperand &Op = Inst.getOperand(1);
      if (Op.isImm()) {
        const unsigned Opcode = Inst.getOpcode();
        switch (Opcode) {
        default:
          break;
        }

        int64_t MemOffset = Op.getImm();
        if (MemOffset < -32768 || MemOffset > 32767) {
          // Offset can't exceed 16bit value.
          expandMemInst(Inst, IDLoc, Out, STI, MCID.mayLoad());
          return getParser().hasPendingError();
        }
      } else if (Op.isExpr()) {
        const MCExpr *Expr = Op.getExpr();
        if (Expr->getKind() == MCExpr::SymbolRef) {
          const MCSymbolRefExpr *SR =
              static_cast<const MCSymbolRefExpr *>(Expr);
          if (SR->getKind() == MCSymbolRefExpr::VK_None) {
            // Expand symbol.
            expandMemInst(Inst, IDLoc, Out, STI, MCID.mayLoad());
            return getParser().hasPendingError();
          }
        } else if (!isEvaluated(Expr)) {
          expandMemInst(Inst, IDLoc, Out, STI, MCID.mayLoad());
          return getParser().hasPendingError();
        }
      }
    }
  } // if load/store
  static int lockReg = -1;
  if (Inst.getOpcode() == Sw64::STQ_C || Inst.getOpcode() == Sw64::STL_C) {
    lockReg = Inst.getOperand(0).getReg();
  }

  if (Inst.getOpcode() == Sw64::RD_F) {
    if (lockReg != Inst.getOperand(0).getReg() && lockReg != -1) {
      Error(IDLoc, "lstX and rd_f must use the same reg!");
      lockReg = -1;
      return false;
    }
  }

  Out.emitInstruction(Inst, *STI);
  return true;
}

// Can the value be represented by a unsigned N-bit value and a shift left?
template <unsigned N> static bool isShiftedUIntAtAnyPosition(uint64_t x) {
  return x && isUInt<N>(x >> llvm::countr_zero(x));
}

OperandMatchResultTy Sw64AsmParser::parseJmpImm(OperandVector &Operands) {
  MCAsmParser &Parser = getParser();
  LLVM_DEBUG(dbgs() << "parseJumpTarget\n");

  SMLoc S = getLexer().getLoc();

  // Registers are a valid target and have priority over symbols.
  OperandMatchResultTy ResTy = parseAnyRegister(Operands);
  if (ResTy != MatchOperand_NoMatch)
    return ResTy;

  // Integers and expressions are acceptable
  const MCExpr *Expr = nullptr;
  if (Parser.parseExpression(Expr)) {
    // We have no way of knowing if a symbol was consumed so we must ParseFail
    return MatchOperand_ParseFail;
  }
  Operands.push_back(
      Sw64Operand::CreateImm(Expr, S, getLexer().getLoc(), *this));
  return MatchOperand_Success;
}

OperandMatchResultTy Sw64AsmParser::parseMemOperands(OperandVector &Operands) {
  LLVM_DEBUG(dbgs() << "Parsing Memory Operand for store/load\n");
  SMLoc S = getParser().getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  const AsmToken &Tok = getParser().getTok();
  switch (Tok.getKind()) {
  default:
    return MatchOperand_NoMatch;
  case AsmToken::EndOfStatement:
    // Zero register assumed, add a memory operand with ZERO as its base.
    //  "Base" will be managed by k_Memory.
    auto Base = Sw64Operand::createGPRReg(
        0, "0", getContext().getRegisterInfo(), S, E, *this);
    Operands.push_back(
        Sw64Operand::CreateMem(std::move(Base), nullptr, S, E, *this));
    return MatchOperand_Success;
  }

  return MatchOperand_NoMatch;
}

void Sw64AsmParser::expandMemInst(MCInst &Inst, SMLoc IDLoc, MCStreamer &Out,
                                  const MCSubtargetInfo *STI, bool IsLoad) {
  // ldl $0,a($gp)        Op0                 Op1              Op2
  //<MCInst 295 <MCOperand Reg:33> <MCOperand Expr:(a)> <MCOperand Reg:62>>

  const MCSymbolRefExpr *SR;
  MCInst TempInst;
  unsigned ImmOffset, HiOffset, LoOffset;
  const MCExpr *ExprOffset;

  // 1st operand is either the source or destination register.
  assert(Inst.getOperand(0).isReg() && "expected register operand kind");
  unsigned RegOpNum = Inst.getOperand(0).getReg();

  // 3nd operand is the base register.
  assert(Inst.getOperand(2).isReg() && "expected register operand kind");
  unsigned BaseRegNum = Inst.getOperand(2).getReg();
  const MCOperand &OffsetOp = Inst.getOperand(1);

  // 2rd operand is either an immediate or expression.
  if (OffsetOp.isImm()) {
    assert(Inst.getOperand(1).isImm() && "expected immediate operand kind");
    ImmOffset = Inst.getOperand(2).getImm();
    LoOffset = ImmOffset & 0x0000ffff;
    HiOffset = (ImmOffset & 0xffff0000) >> 16;
    // If msb of LoOffset is 1(negative number) we must increment HiOffset.
    if (LoOffset & 0x8000)
      HiOffset++;
  } else
    ExprOffset = Inst.getOperand(1).getExpr();
  // All instructions will have the same location.
  TempInst.setLoc(IDLoc);
  TempInst.setOpcode(Inst.getOpcode());
  TempInst.addOperand(MCOperand::createReg(RegOpNum));
  if (OffsetOp.isImm())
    TempInst.addOperand(MCOperand::createImm(ImmOffset));
  else {
    if (ExprOffset->getKind() == MCExpr::SymbolRef) {
      SR = static_cast<const MCSymbolRefExpr *>(ExprOffset);

      TempInst.addOperand(MCOperand::createExpr(SR));
    } else {
      llvm_unreachable("Memory offset is not SymbolRef!");
    }
  }
  TempInst.addOperand(MCOperand::createReg(BaseRegNum));
  Out.emitInstruction(TempInst, *STI);
  // Prepare TempInst for next instruction.
  TempInst.clear();
}

// Expand a integer division macro.
//
// Notably we don't have to emit a warning when encountering $rt as the $zero
// register, or 0 as an immediate. processInstruction() has already done that.
//
// The destination register can only be $zero when expanding (S)DivIMacro or
// D(S)DivMacro.

bool Sw64AsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);

  switch (MatchResult) {
  case Match_Success:
    if (processInstruction(Inst, IDLoc, Out, STI))
      return true;
    return false;
  case Match_MissingFeature:
    Error(IDLoc, "instruction requires a CPU feature not currently enabled");
    return true;
  case Match_InvalidTiedOperand:
    Error(IDLoc, "operand must match destination register");
    return true;
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL) {
      if (ErrorInfo >= Operands.size())
        return Error(IDLoc, "too few operands for instruction");

      ErrorLoc = Operands[ErrorInfo]->getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }

    return Error(ErrorLoc, "invalid operand for instruction");
  }
  case Match_MnemonicFail:
    return Error(IDLoc, "invalid instruction");
  }
  llvm_unreachable("Implement any new match types added!");
}

int Sw64AsmParser::matchCPURegisterName(StringRef Name) {
  int CC;
  CC = StringSwitch<unsigned>(Name)
           .Cases("v0", "r0", Sw64::R0)
           .Cases("t0", "r1", Sw64::R1)
           .Cases("t1", "r2", Sw64::R2)
           .Cases("t2", "r3", Sw64::R3)
           .Cases("t3", "r4", Sw64::R4)
           .Cases("t4", "r5", Sw64::R5)
           .Cases("t5", "r6", Sw64::R6)
           .Cases("t6", "r7", Sw64::R7)
           .Cases("t7", "r8", Sw64::R8)
           .Cases("s0", "r9", Sw64::R9)
           .Cases("s1", "r10", Sw64::R10)
           .Cases("s2", "r11", Sw64::R11)
           .Cases("s3", "r12", Sw64::R12)
           .Cases("s4", "r13", Sw64::R13)
           .Cases("s5", "r14", Sw64::R14)
           .Cases("fp", "r15", Sw64::R15)
           .Cases("a0", "r16", Sw64::R16)
           .Cases("a1", "r17", Sw64::R17)
           .Cases("a2", "r18", Sw64::R18)
           .Cases("a3", "r19", Sw64::R19)
           .Cases("a4", "r20", Sw64::R20)
           .Cases("a5", "r21", Sw64::R21)
           .Cases("t8", "r22", Sw64::R22)
           .Cases("t9", "r23", Sw64::R23)
           .Cases("t10", "r24", Sw64::R24)
           .Cases("t11", "r25", Sw64::R25)
           .Cases("ra", "r26", Sw64::R26)
           .Cases("pv", "r27", Sw64::R27)
           .Cases("at", "r28", Sw64::R28)
           .Cases("gp", "r29", Sw64::R29)
           .Cases("sp", "r30", Sw64::R30)
           .Cases("zero", "r31", Sw64::R31)
           .Default(-1);

  return CC;
}

int Sw64AsmParser::matchFPURegisterName(StringRef Name) {
  if (Name[0] == 'f') {
    StringRef NumString = Name.substr(1);
    unsigned IntVal;
    if (NumString.getAsInteger(10, IntVal))
      return -1;     // This is not an integer.
    if (IntVal > 31) // Maximum index for fpu register.
      return -1;
    return IntVal + 1;
  }
  return -1;
}

bool Sw64AsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {
  MCAsmParser &Parser = getParser();
  LLVM_DEBUG(dbgs() << "parseOperand\n");

  // Check if the current operand has a custom associated parser, if so, try to
  // custom parse the operand, or fallback to the general approach.
  OperandMatchResultTy ResTy = MatchOperandParserImpl(Operands, Mnemonic);
  if (ResTy == MatchOperand_Success)
    return false;
  // If there wasn't a custom match, try the generic matcher below. Otherwise,
  // there was a match, but an error occurred, in which case, just return that
  // the operand parsing failed.
  if (ResTy == MatchOperand_ParseFail)
    return true;

  if (parseMemOperands(Operands) == MatchOperand_Success)
    return false;

  LLVM_DEBUG(dbgs() << ".. Generic Parser\n");

  switch (getLexer().getKind()) {
  case AsmToken::Dollar: {
    // Parse the register.
    SMLoc S = Parser.getTok().getLoc();

    // Almost all registers have been parsed by custom parsers. There is only
    // one exception to this. $zero (and it's alias $0) will reach this point
    // for div, divu, and similar instructions because it is not an operand
    // to the instruction definition but an explicit register. Special case
    // this situation for now.
    if (parseAnyRegister(Operands) != MatchOperand_NoMatch)
      return false;

    // Maybe it is a symbol reference.
    StringRef Identifier;
    if (Parser.parseIdentifier(Identifier))
      return true;

    SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
    MCSymbol *Sym = getContext().getOrCreateSymbol(Identifier);

    // Otherwise create a symbol reference.
    const MCExpr *Res =
        MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, getContext());

    Operands.push_back(Sw64Operand::CreateImm(Res, S, E, *this));
    return false;
  }
  // parse jmp & ret: ($GPRC)
  case AsmToken::LParen: {
    return parseParenSuffix(Mnemonic, Operands);
  }
  case AsmToken::Minus:
  case AsmToken::Plus:
  case AsmToken::String:
  case AsmToken::Integer: {
    LLVM_DEBUG(dbgs() << ".. generic integer expression\n");
    const MCExpr *IdVal;
    SMLoc S = Parser.getTok().getLoc(); // Start location of the operand.
    if (getParser().parseExpression(IdVal))
      return true;

    std::string Reloc;
    const MCExpr *Expr;
    const char *Mnem = Mnemonic.data();
    AsmToken::TokenKind FirstTokenKind;
    MCContext &Ctx = getStreamer().getContext();
    std::string Stxt = S.getPointer();
    size_t a = Stxt.find_first_of('!');
    size_t c = Stxt.find_first_of('\n');

    if (a != 0 && a < c) {
      std::string Reloc1 = Stxt.substr(a + 1, c - a - 1);
      size_t b = Reloc1.find_last_of('!');

      Reloc = Reloc1.substr(0, b);

      if (Reloc == "gpdisp") {
        if (strcmp(Mnem, "ldih") == 0)
          FirstTokenKind = AsmToken::TokenKind::PercentGot_Hi;
        else if (strcmp(Mnem, "ldi") == 0)
          FirstTokenKind = AsmToken::TokenKind::PercentGot_Lo;

        Expr = createTargetUnaryExpr(IdVal, FirstTokenKind, Ctx);
      }
      SMLoc E =
          SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

      Operands.push_back(Sw64Operand::CreateImm(Expr, S, E, *this));
      return false;
    }

    SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
    Operands.push_back(Sw64Operand::CreateImm(IdVal, S, E, *this));
    return false;
  }
  default: {
    LLVM_DEBUG(dbgs() << ".. generic expr expression\n");

    const MCExpr *Expr;
    SMLoc S = Parser.getTok().getLoc();
    if (getParser().parseExpression(Expr))
      return true;

    std::string Reloc;
    AsmToken::TokenKind FirstTokenKind;
    MCContext &Ctx = getStreamer().getContext();
    std::string Stxt = S.getPointer();
    size_t a = Stxt.find_first_of('!');
    size_t b = Stxt.find_first_of('\n');
    Reloc = Stxt.substr(a + 1, b - a - 1);

    if (a < b) {
      if (Reloc == "literal")
        FirstTokenKind = AsmToken::TokenKind::PercentGp_Rel;
      else if (Reloc == "gprelhigh")
        FirstTokenKind = AsmToken::TokenKind::PercentDtprel_Hi;
      else if (Reloc == "gprellow")
        FirstTokenKind = AsmToken::TokenKind::PercentDtprel_Lo;
      else if (Reloc == "tprelhi")
        FirstTokenKind = AsmToken::TokenKind::PercentTprel_Hi;
      else if (Reloc == "tprello")
        FirstTokenKind = AsmToken::TokenKind::PercentTprel_Lo;

      Expr = createTargetUnaryExpr(Expr, FirstTokenKind, Ctx);
    }

    SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

    Operands.push_back(Sw64Operand::CreateImm(Expr, S, E, *this));
    return false;
  }
  }
  return true;
}

bool Sw64AsmParser::parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                  SMLoc &EndLoc) {
  return tryParseRegister(RegNo, StartLoc, EndLoc) != MatchOperand_Success;
}

OperandMatchResultTy Sw64AsmParser::tryParseRegister(MCRegister &RegNo,
                                                     SMLoc &StartLoc,
                                                     SMLoc &EndLoc) {
  SmallVector<std::unique_ptr<MCParsedAsmOperand>, 1> Operands;
  OperandMatchResultTy ResTy = parseAnyRegister(Operands);
  if (ResTy == MatchOperand_Success) {
    assert(Operands.size() == 1);
    Sw64Operand &Operand = static_cast<Sw64Operand &>(*Operands.front());
    StartLoc = Operand.getStartLoc();
    EndLoc = Operand.getEndLoc();

    // AFAIK, we only support numeric registers and named GPR's in CFI
    // directives.
    // Don't worry about eating tokens before failing. Using an unrecognised
    // register is a parse error.
    if (Operand.isGPRAsmReg()) {
      // Resolve to GPR32 or GPR64 appropriately.
      RegNo = Operand.getGPRReg();
    }

    return (RegNo == (unsigned)-1) ? MatchOperand_NoMatch
                                   : MatchOperand_Success;
  }

  assert(Operands.size() == 0);
  return (RegNo == (unsigned)-1) ? MatchOperand_NoMatch : MatchOperand_Success;
}

bool Sw64AsmParser::isEvaluated(const MCExpr *Expr) {
  switch (Expr->getKind()) {
  case MCExpr::Constant:
    return true;
  case MCExpr::SymbolRef:
    return (cast<MCSymbolRefExpr>(Expr)->getKind() != MCSymbolRefExpr::VK_None);
  case MCExpr::Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(Expr);
    if (!isEvaluated(BE->getLHS()))
      return false;
    return isEvaluated(BE->getRHS());
  }
  case MCExpr::Unary:
    return isEvaluated(cast<MCUnaryExpr>(Expr)->getSubExpr());
  case MCExpr::Target:
    return true;
  }
  return false;
}

bool Sw64AsmParser::parseMemOffset(const MCExpr *&Res, bool isParenExpr) {
  SMLoc S;

  if (isParenExpr)
    return getParser().parseParenExprOfDepth(0, Res, S);
  return getParser().parseExpression(Res);
}

OperandMatchResultTy Sw64AsmParser::parseMemOperand(OperandVector &Operands) {
  MCAsmParser &Parser = getParser();
  LLVM_DEBUG(dbgs() << "parseMemOperand\n");
  const MCExpr *IdVal = nullptr;
  SMLoc S;
  bool isParenExpr = false;
  OperandMatchResultTy Res = MatchOperand_NoMatch;
  // First operand is the offset.
  S = Parser.getTok().getLoc();

  if (getLexer().getKind() == AsmToken::LParen) {
    Parser.Lex();
    isParenExpr = true;
  }

  if (getLexer().getKind() != AsmToken::Dollar) {
    if (parseMemOffset(IdVal, isParenExpr))
      return MatchOperand_ParseFail;

    const AsmToken &Tok = Parser.getTok(); // Get the next token.
    if (Tok.isNot(AsmToken::LParen)) {
      Sw64Operand &Mnemonic = static_cast<Sw64Operand &>(*Operands[0]);
      if (Mnemonic.getToken() == "la" || Mnemonic.getToken() == "dla") {
        SMLoc E =
            SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
        Operands.push_back(Sw64Operand::CreateImm(IdVal, S, E, *this));
        return MatchOperand_Success;
      }
      if (Tok.is(AsmToken::EndOfStatement)) {
        SMLoc E =
            SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

        // Zero register assumed, add a memory operand with ZERO as its base.
        // "Base" will be managed by k_Memory.
        auto Base = Sw64Operand::createGPRReg(
            0, "0", getContext().getRegisterInfo(), S, E, *this);
        Operands.push_back(
            Sw64Operand::CreateMem(std::move(Base), IdVal, S, E, *this));
        return MatchOperand_Success;
      }

      MCBinaryExpr::Opcode Opcode;
      // GAS and LLVM treat comparison operators different. GAS will generate -1
      // or 0, while LLVM will generate 0 or 1. Since a comparsion operator is
      // highly unlikely to be found in a memory offset expression, we don't
      // handle them.
      switch (Tok.getKind()) {
      case AsmToken::Plus:
        Opcode = MCBinaryExpr::Add;
        Parser.Lex();
        break;
      case AsmToken::Minus:
        Opcode = MCBinaryExpr::Sub;
        Parser.Lex();
        break;
      case AsmToken::Star:
        Opcode = MCBinaryExpr::Mul;
        Parser.Lex();
        break;
      case AsmToken::Pipe:
        Opcode = MCBinaryExpr::Or;
        Parser.Lex();
        break;
      case AsmToken::Amp:
        Opcode = MCBinaryExpr::And;
        Parser.Lex();
        break;
      case AsmToken::LessLess:
        Opcode = MCBinaryExpr::Shl;
        Parser.Lex();
        break;
      case AsmToken::GreaterGreater:
        Opcode = MCBinaryExpr::LShr;
        Parser.Lex();
        break;
      case AsmToken::Caret:
        Opcode = MCBinaryExpr::Xor;
        Parser.Lex();
        break;
      case AsmToken::Slash:
        Opcode = MCBinaryExpr::Div;
        Parser.Lex();
        break;
      case AsmToken::Percent:
        Opcode = MCBinaryExpr::Mod;
        Parser.Lex();
        break;
      default:
        Error(Parser.getTok().getLoc(), "'(' or expression expected");
        return MatchOperand_ParseFail;
      }
      const MCExpr *NextExpr;
      if (getParser().parseExpression(NextExpr))
        return MatchOperand_ParseFail;
      IdVal = MCBinaryExpr::create(Opcode, IdVal, NextExpr, getContext());
    }

    Parser.Lex(); // Eat the '(' token.
  }

  Res = parseAnyRegister(Operands);
  if (Res != MatchOperand_Success)
    return Res;

  if (Parser.getTok().isNot(AsmToken::RParen)) {
    Error(Parser.getTok().getLoc(), "')' expected");
    return MatchOperand_ParseFail;
  }

  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  Parser.Lex(); // Eat the ')' token.

  if (!IdVal)
    IdVal = MCConstantExpr::create(0, getContext());

  // Replace the register operand with the memory operand.
  std::unique_ptr<Sw64Operand> op(
      static_cast<Sw64Operand *>(Operands.back().release()));
  // Remove the register from the operands.
  // "op" will be managed by k_Memory.
  Operands.pop_back();

  // Add the memory operand.
  if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(IdVal)) {
    int64_t Imm;
    if (IdVal->evaluateAsAbsolute(Imm))
      IdVal = MCConstantExpr::create(Imm, getContext());
    else if (BE->getLHS()->getKind() != MCExpr::SymbolRef)
      IdVal = MCBinaryExpr::create(BE->getOpcode(), BE->getRHS(), BE->getLHS(),
                                   getContext());
  }

  Operands.push_back(Sw64Operand::CreateMem(std::move(op), IdVal, S, E, *this));
  return MatchOperand_Success;
}

bool Sw64AsmParser::searchSymbolAlias(OperandVector &Operands) {
  MCAsmParser &Parser = getParser();
  MCSymbol *Sym = getContext().lookupSymbol(Parser.getTok().getIdentifier());
  if (!Sym)
    return false;

  SMLoc S = Parser.getTok().getLoc();
  if (Sym->isVariable()) {
    const MCExpr *Expr = Sym->getVariableValue();
    if (Expr->getKind() == MCExpr::SymbolRef) {
      const MCSymbolRefExpr *Ref = static_cast<const MCSymbolRefExpr *>(Expr);
      StringRef DefSymbol = Ref->getSymbol().getName();
      if (DefSymbol.startswith("$")) {
        OperandMatchResultTy ResTy =
            matchAnyRegisterNameWithoutDollar(Operands, DefSymbol.substr(1), S);
        if (ResTy == MatchOperand_Success) {
          Parser.Lex();
          return true;
        }
        if (ResTy == MatchOperand_ParseFail)
          llvm_unreachable("Should never ParseFail");
      }
    }
  } else if (Sym->isUnset()) {
    // If symbol is unset, it might be created in the `parseSetAssignment`
    // routine as an alias for a numeric register name.
    // Lookup in the aliases list.
    auto Entry = RegisterSets.find(Sym->getName());
    if (Entry != RegisterSets.end()) {
      OperandMatchResultTy ResTy =
          matchAnyRegisterWithoutDollar(Operands, Entry->getValue(), S);
      if (ResTy == MatchOperand_Success) {
        Parser.Lex();
        return true;
      }
    }
  }

  return false;
}

OperandMatchResultTy Sw64AsmParser::matchAnyRegisterNameWithoutDollar(
    OperandVector &Operands, StringRef Identifier, SMLoc S) {
  int Index = matchCPURegisterName(Identifier);
  if (Index != -1) {
    Operands.push_back(Sw64Operand::createGPRReg(
        Index, Identifier, getContext().getRegisterInfo(), S,
        getLexer().getLoc(), *this));
    return MatchOperand_Success;
  }
  Index = matchFPURegisterName(Identifier);
  if (Index != -1) {
    Operands.push_back(Sw64Operand::createFPRReg(
        Index, Identifier, getContext().getRegisterInfo(), S,
        getLexer().getLoc(), *this));
    return MatchOperand_Success;
  }
  return MatchOperand_NoMatch;
}

OperandMatchResultTy
Sw64AsmParser::matchAnyRegisterWithoutDollar(OperandVector &Operands,
                                             const AsmToken &Token, SMLoc S) {
  if (Token.is(AsmToken::Identifier)) {
    LLVM_DEBUG(dbgs() << ".. identifier\n");
    StringRef Identifier = Token.getIdentifier();
    OperandMatchResultTy ResTy =
        matchAnyRegisterNameWithoutDollar(Operands, Identifier, S);
    return ResTy;
  } else if (Token.is(AsmToken::Integer)) {
    LLVM_DEBUG(dbgs() << ".. integer\n");
    int64_t RegNum = Token.getIntVal();
    Operands.push_back(Sw64Operand::createNumericReg(
        RegNum, Token.getString(), getContext().getRegisterInfo(), S,
        Token.getLoc(), *this));
    return MatchOperand_Success;
  }

  LLVM_DEBUG(dbgs() << Token.getKind() << "\n");

  return MatchOperand_NoMatch;
}

OperandMatchResultTy
Sw64AsmParser::matchAnyRegisterWithoutDollar(OperandVector &Operands, SMLoc S) {
  auto Token = getLexer().peekTok(false);
  return matchAnyRegisterWithoutDollar(Operands, Token, S);
}

OperandMatchResultTy Sw64AsmParser::parseAnyRegister(OperandVector &Operands) {
  MCAsmParser &Parser = getParser();
  LLVM_DEBUG(dbgs() << "parseAnyRegister\n");

  auto Token = Parser.getTok();

  SMLoc S = Token.getLoc();

  if (Token.isNot(AsmToken::Dollar)) {
    LLVM_DEBUG(dbgs() << ".. !$ -> try sym aliasing\n");
    if (Token.is(AsmToken::Identifier)) {
      if (searchSymbolAlias(Operands))
        return MatchOperand_Success;
    }
    LLVM_DEBUG(dbgs() << ".. !symalias -> NoMatch\n");
    return MatchOperand_NoMatch;
  }
  LLVM_DEBUG(dbgs() << ".. $\n");

  OperandMatchResultTy ResTy = matchAnyRegisterWithoutDollar(Operands, S);
  if (ResTy == MatchOperand_Success) {
    Parser.Lex(); // $
    Parser.Lex(); // identifier
  }
  return ResTy;
}

bool Sw64AsmParser::parseParenSuffix(StringRef Name, OperandVector &Operands) {
  MCAsmParser &Parser = getParser();
  if (getLexer().is(AsmToken::LParen)) {
    Operands.push_back(
        Sw64Operand::CreateToken("(", getLexer().getLoc(), *this));
    Parser.Lex();
    if (Name == "ret") {
      Operands.push_back(
          Sw64Operand::CreateToken("$26)", getLexer().getLoc(), *this));
      Parser.Lex(); // eat "$"
      Parser.Lex(); // eat "26"
      Parser.Lex(); // eat ")"
    } else {
      if (parseOperand(Operands, Name)) {
        SMLoc Loc = getLexer().getLoc();
        return Error(Loc, "unexpected token in argument list");
      }
      if (Parser.getTok().isNot(AsmToken::RParen)) {
        SMLoc Loc = getLexer().getLoc();
        return Error(Loc, "unexpected token, expected ')'");
      }
      Operands.push_back(
          Sw64Operand::CreateToken(")", getLexer().getLoc(), *this));
      Parser.Lex();
    }
  }
  return false;
}

bool Sw64AsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                     SMLoc NameLoc, OperandVector &Operands) {
  MCAsmParser &Parser = getParser();
  LLVM_DEBUG(dbgs() << "ParseInstruction\n");

  std::pair<StringRef, unsigned> RelocOperands;
  // We have reached first instruction, module directive are now forbidden.
  // getTargetStreamer().forbidModuleDirective();

  // Check if we have valid mnemonic
  if (!mnemonicIsValid(Name, 0)) {
    return Error(NameLoc, "unknown instruction");
  }
  // First operand in MCInst is instruction mnemonic.
  Operands.push_back(Sw64Operand::CreateToken(Name, NameLoc, *this));

  // Read the remaining operands.
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    // Read the first operand.
    if (parseOperand(Operands, Name)) {
      SMLoc Loc = getLexer().getLoc();
      return Error(Loc, "unexpected token in argument list");
    }

    while (getLexer().is(AsmToken::Comma)) {
      Parser.Lex(); // Eat the comma.
      // Parse and remember the operand.
      if (parseOperand(Operands, Name)) {
        SMLoc Loc = getLexer().getLoc();
        return Error(Loc, "unexpected token in argument list");
      }
      // Parse parenthesis suffixes before we iterate
      if (getLexer().is(AsmToken::LParen) && parseParenSuffix(Name, Operands))
        return true;
    }
  }
  while (Parser.getTok().is(AsmToken::Exclaim)) {
    if (false) {
      LLVM_DEBUG(dbgs() << ".. Skip Parse " << Name << " Relocation Symbol\n");
      Parser.Lex(); // Eat !
      Parser.Lex(); // Eat reloction symbol.
    } else {
      LLVM_DEBUG(dbgs() << ".. Parse \"!");
      Parser.Lex(); // Eat !

      if (Parser.getTok().is(AsmToken::Identifier)) {
        // Parse Relocation Symbol ,Add Rel Kind Here !
        StringRef Identifier = Parser.getTok().getIdentifier();
        LLVM_DEBUG(dbgs() << Identifier << "\"\n");
        RelocOperands.first = Identifier;
      }
      if (Parser.getTok().is(AsmToken::Integer)) {
        int64_t RelNum = Parser.getTok().getIntVal();
        LLVM_DEBUG(dbgs() << RelNum << "\"\n");
        RelocOperands.second = RelNum;
      }
      ParsingFixupOperands(RelocOperands);
      Parser.Lex(); // Eat reloction symbol.
    }
  }
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    return Error(Loc, "unexpected token in argument list");
  }
  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

// FIXME: Given that these have the same name, these should both be
// consistent on affecting the Parser.
bool Sw64AsmParser::reportParseError(Twine ErrorMsg) {
  SMLoc Loc = getLexer().getLoc();
  return Error(Loc, ErrorMsg);
}

bool Sw64AsmParser::parseSetNoAtDirective() {
  MCAsmParser &Parser = getParser();
  // Line should look like: ".set noat".

  // Set the $at register to $0.
  AssemblerOptions.back()->setATRegIndex(0);

  Parser.Lex(); // Eat "noat".

  // If this is not the end of the statement, report an error.
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    reportParseError("unexpected token, expected end of statement");
    return false;
  }

  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

bool Sw64AsmParser::parseSetAtDirective() {
  // Line can be: ".set at", which sets $at to $1
  //          or  ".set at=$reg", which sets $at to $reg.
  MCAsmParser &Parser = getParser();
  Parser.Lex(); // Eat "at".

  if (getLexer().is(AsmToken::EndOfStatement)) {
    // No register was specified, so we set $at to $1.
    AssemblerOptions.back()->setATRegIndex(1);

    Parser.Lex(); // Consume the EndOfStatement.
    return false;
  }

  if (getLexer().isNot(AsmToken::Equal)) {
    reportParseError("unexpected token, expected equals sign");
    return false;
  }
  Parser.Lex(); // Eat "=".

  if (getLexer().isNot(AsmToken::Dollar)) {
    if (getLexer().is(AsmToken::EndOfStatement)) {
      reportParseError("no register specified");
      return false;
    } else {
      reportParseError("unexpected token, expected dollar sign '$'");
      return false;
    }
  }
  Parser.Lex(); // Eat "$".

  // Find out what "reg" is.
  unsigned AtRegNo;
  const AsmToken &Reg = Parser.getTok();
  if (Reg.is(AsmToken::Identifier)) {
    AtRegNo = matchCPURegisterName(Reg.getIdentifier());
  } else if (Reg.is(AsmToken::Integer)) {
    AtRegNo = Reg.getIntVal();
  } else {
    reportParseError("unexpected token, expected identifier or integer");
    return false;
  }

  // Check if $reg is a valid register. If it is, set $at to $reg.
  if (!AssemblerOptions.back()->setATRegIndex(AtRegNo)) {
    reportParseError("invalid register");
    return false;
  }
  Parser.Lex(); // Eat "reg".

  // If this is not the end of the statement, report an error.
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    reportParseError("unexpected token, expected end of statement");
    return false;
  }

  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

bool Sw64AsmParser::parseSetReorderDirective() {
  MCAsmParser &Parser = getParser();
  Parser.Lex();
  // If this is not the end of the statement, report an error.
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    reportParseError("unexpected token, expected end of statement");
    return false;
  }
  AssemblerOptions.back()->setReorder();
  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

bool Sw64AsmParser::parseSetNoReorderDirective() {
  MCAsmParser &Parser = getParser();
  Parser.Lex();
  // If this is not the end of the statement, report an error.
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    reportParseError("unexpected token, expected end of statement");
    return false;
  }
  AssemblerOptions.back()->setNoReorder();
  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

bool Sw64AsmParser::parseSetMacroDirective() {
  MCAsmParser &Parser = getParser();
  Parser.Lex();
  // If this is not the end of the statement, report an error.
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    reportParseError("unexpected token, expected end of statement");
    return false;
  }
  AssemblerOptions.back()->setMacro();
  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

bool Sw64AsmParser::parseSetNoMacroDirective() {
  MCAsmParser &Parser = getParser();
  Parser.Lex();
  // If this is not the end of the statement, report an error.
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    reportParseError("unexpected token, expected end of statement");
    return false;
  }
  if (AssemblerOptions.back()->isReorder()) {
    reportParseError("`noreorder' must be set before `nomacro'");
    return false;
  }
  AssemblerOptions.back()->setNoMacro();
  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

bool Sw64AsmParser::parseSetAssignment() {
  StringRef Name;
  const MCExpr *Value;
  MCAsmParser &Parser = getParser();

  if (Parser.parseIdentifier(Name))
    return reportParseError("expected identifier after .set");

  if (getLexer().isNot(AsmToken::Comma))
    return reportParseError("unexpected token, expected comma");
  Lex(); // Eat comma

  if (getLexer().is(AsmToken::Dollar) &&
      getLexer().peekTok().is(AsmToken::Integer)) {
    // Parse assignment of a numeric register:
    //   .set r1,$1
    Parser.Lex(); // Eat $.
    RegisterSets[Name] = Parser.getTok();
    Parser.Lex(); // Eat identifier.
    getContext().getOrCreateSymbol(Name);
  } else if (!Parser.parseExpression(Value)) {
    // Parse assignment of an expression including
    // symbolic registers:
    //   .set  $tmp, $BB0-$BB1
    //   .set  r2, $f2
    MCSymbol *Sym = getContext().getOrCreateSymbol(Name);
    Sym->setVariableValue(Value);
  } else {
    return reportParseError("expected valid expression after comma");
  }

  return false;
}

bool Sw64AsmParser::parseSetArchDirective() {
  MCAsmParser &Parser = getParser();

  StringRef Arch;
  if (Parser.parseIdentifier(Arch))
    return reportParseError("expected arch identifier");

  StringRef ArchFeatureName = StringSwitch<StringRef>(Arch)
                                  .Case("sw_64", "sw_64")
                                  .Case("core3b", "core3b")
                                  .Case("core4", "core4")
                                  .Default("");

  if (ArchFeatureName.empty())
    return reportParseError("unsupported architecture");

  selectArch(ArchFeatureName);
  return false;
}

bool Sw64AsmParser::parseDirectiveSet() {
  const AsmToken &Tok = getParser().getTok();
  StringRef IdVal = Tok.getString();

  if (IdVal == "noat")
    return parseSetNoAtDirective();
  if (IdVal == "at")
    return parseSetAtDirective();
  if (IdVal == "arch")
    return parseSetArchDirective();

  if (Tok.getString() == "reorder") {
    return parseSetReorderDirective();
  }
  if (Tok.getString() == "noreorder") {
    return parseSetNoReorderDirective();
  }
  if (Tok.getString() == "macro") {
    return parseSetMacroDirective();
  }
  if (Tok.getString() == "nomacro") {
    return parseSetNoMacroDirective();
  }
  // TODO: temp write
  if (Tok.getString() == "volatile") {
    return parseSetNoMacroDirective();
  }
  // It is just an identifier, look for an assignment.
  return parseSetAssignment();
}

bool Sw64AsmParser::ParseDirective(AsmToken DirectiveID) {
  // This returns false if this function recognizes the directive
  // regardless of whether it is successfully handles or reports an
  // error. Otherwise it returns true to give the generic parser a
  // chance at recognizing it.

  MCAsmParser &Parser = getParser();
  StringRef IDVal = DirectiveID.getString();

  if (IDVal == ".ent") {
    // Ignore this directive for now.
    Parser.Lex();
    return false;
  }

  if (IDVal == ".end") {
    // Ignore this directive for now.
    Parser.Lex();
    return false;
  }

  if (IDVal == ".frame") {
    // Ignore this directive for now.
    Parser.eatToEndOfStatement();
    return false;
  }

  if (IDVal == ".set") {
    parseDirectiveSet();
    return false;
  }

  if (IDVal == ".mask" || IDVal == ".fmask") {
    // Ignore this directive for now.
    Parser.eatToEndOfStatement();
    return false;
  }
  if (IDVal == ".arch") {
    // Ignore this directive for now.
    parseSetArchDirective();
    Parser.eatToEndOfStatement();
    return false;
  }
  if (IDVal == ".word") {
    // Ignore this directive for now.
    Parser.eatToEndOfStatement();
  }
  return true;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSw64AsmParser() {
  RegisterMCAsmParser<Sw64AsmParser> X(getTheSw64Target());
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "Sw64GenAsmMatcher.inc"

bool Sw64AsmParser::mnemonicIsValid(StringRef Mnemonic, unsigned VariantID) {
  // Find the appropriate table for this asm variant.
  const MatchEntry *Start, *End;
  switch (VariantID) {
  default:
    llvm_unreachable("invalid variant!");
  case 0:
    Start = std::begin(MatchTable0);
    End = std::end(MatchTable0);
    break;
  }
  // Search the table.
  auto MnemonicRange = std::equal_range(Start, End, Mnemonic, LessOpcode());
  return MnemonicRange.first != MnemonicRange.second;
}

unsigned Sw64AsmParser::validateTargetOperandClass(MCParsedAsmOperand &AsmOp,
                                                   unsigned Kind) {
  Sw64Operand &Op = static_cast<Sw64Operand &>(AsmOp);
  int64_t ExpectedVal;

  switch (Kind) {
  default:
    return Match_InvalidOperand;
  }

  if (!Op.isReg())
    return Match_InvalidOperand;

  if (Op.getReg() == ExpectedVal)
    return Match_Success;
  return Match_InvalidOperand;
}

void Sw64AsmParser::ParsingFixupOperands(std::pair<StringRef, unsigned> reloc) {
  for (auto i : RelocTable) {
    if (reloc.first.startswith(i))
      FixupKind =
          StringSwitch<MCFixupKind>(i)
              .Case("literal", (MCFixupKind)Sw64::fixup_SW64_ELF_LITERAL)
              .Case("literal_got",
                    (MCFixupKind)Sw64::fixup_SW64_ELF_LITERAL_GOT)
              .Case("lituse_addr", (MCFixupKind)Sw64::fixup_SW64_LITUSE)
              .Case("lituse_jsr", (MCFixupKind)Sw64::fixup_SW64_HINT)
              .Case("gpdisp", (MCFixupKind)Sw64::fixup_SW64_GPDISP)
              .Case("gprelhigh", (MCFixupKind)Sw64::fixup_SW64_GPDISP_HI16)
              .Case("gprellow", (MCFixupKind)Sw64::fixup_SW64_GPDISP_LO16)
              .Case("gprel", (MCFixupKind)Sw64::fixup_SW64_GPREL16)
              .Case("tlsgd", (MCFixupKind)Sw64::fixup_SW64_TLSGD)
              .Case("tlsldm", (MCFixupKind)Sw64::fixup_SW64_TLSLDM)
              .Case("gotdtprel", (MCFixupKind)Sw64::fixup_SW64_GOTDTPREL16)
              .Case("dtprelhi", (MCFixupKind)Sw64::fixup_SW64_DTPREL_HI16)
              .Case("dtprello", (MCFixupKind)Sw64::fixup_SW64_DTPREL_LO16)
              .Case("gottprel", (MCFixupKind)Sw64::fixup_SW64_GOTTPREL16)
              .Case("tprelhi", (MCFixupKind)Sw64::fixup_SW64_TPREL_HI16)
              .Case("tprello", (MCFixupKind)Sw64::fixup_SW64_TPREL_LO16)
              .Case("tprel", (MCFixupKind)Sw64::fixup_SW64_TPREL16)
              .Default(llvm::FirstTargetFixupKind);
  }
}
