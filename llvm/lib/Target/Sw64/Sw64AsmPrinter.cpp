//===-- Sw64AsmPrinter.cpp - Sw64 LLVM assembly writer ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the XAS-format Sw64 assembly language.
//
//===----------------------------------------------------------------------===//

#include "InstPrinter/Sw64InstPrinter.h"
#include "MCTargetDesc/Sw64BaseInfo.h"
#include "Sw64.h"
#include "Sw64InstrInfo.h"
#include "Sw64MCInstLower.h"
#include "Sw64Subtarget.h"
#include "Sw64TargetMachine.h"
#include "Sw64TargetStreamer.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include <algorithm>
#include <cctype>
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
class Sw64AsmPrinter : public AsmPrinter {
  Sw64MCInstLower MCInstLowering;
  Sw64TargetStreamer &getTargetStreamer();
  /// InConstantPool - Maintain state when emitting a sequence of constant
  /// pool entries so we can properly mark them as data regions.
  bool InConstantPool = false;

public:
  explicit Sw64AsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), MCInstLowering(*this) {}

  StringRef getPassName() const override { return "Sw64 Assembly Printer"; }

  void printOp(const MachineOperand &MO, raw_ostream &O);
  void printOperand(const MachineInstr *MI, int opNum, raw_ostream &O);
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &O) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNum,
                             const char *ExtraCode, raw_ostream &O) override;

  void emitFunctionEntryLabel() override;
  void emitInstruction(const MachineInstr *MI) override;
  void emitFunctionBodyStart() override;
  void emitFunctionBodyEnd() override;
  void emitStartOfAsmFile(Module &M) override;
  bool isBlockOnlyReachableByFallthrough(
      const MachineBasicBlock *MBB) const override;
  bool runOnMachineFunction(MachineFunction &MF) override;

  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const {
    return LowerSw64MachineOperandToMCOperand(MO, MCOp, *this);
  }
};
} // end of anonymous namespace

bool Sw64AsmPrinter::runOnMachineFunction(MachineFunction &MF) {

  // Initialize TargetLoweringObjectFile.
  AsmPrinter::runOnMachineFunction(MF);
  return true;
}

bool Sw64AsmPrinter::isBlockOnlyReachableByFallthrough(
    const MachineBasicBlock *MBB) const {
  // The predecessor has to be immediately before this block.
  const MachineBasicBlock *Pred = *MBB->pred_begin();

  // If the predecessor is a switch statement, assume a jump table
  // implementation, so it is not a fall through.
  if (const BasicBlock *bb = Pred->getBasicBlock())
    if (isa<SwitchInst>(bb->getTerminator()))
      return false;

  // If this is a landing pad, it isn't a fall through.  If it has no preds,
  // then nothing falls through to it.
  if (MBB->isEHPad() || MBB->pred_empty())
    return false;

  // If there isn't exactly one predecessor, it can't be a fall through.
  MachineBasicBlock::const_pred_iterator PI = MBB->pred_begin(), PI2 = PI;
  ++PI2;

  if (PI2 != MBB->pred_end())
    return false;

  // The predecessor has to be immediately before this block.
  if (!Pred->isLayoutSuccessor(MBB))
    return false;

  // If the block is completely empty, then it definitely does fall through.
  if (Pred->empty())
    return true;

  // Otherwise, check the last instruction.
  // Check if the last terminator is an unconditional branch.
  MachineBasicBlock::const_iterator I = Pred->end();
  while (I != Pred->begin() && !(--I)->isTerminator())
    ;
  return false;
  //  return !I->isBarrier();
  // ;
}

Sw64TargetStreamer &Sw64AsmPrinter::getTargetStreamer() {
  return static_cast<Sw64TargetStreamer &>(*OutStreamer->getTargetStreamer());
}

//===----------------------------------------------------------------------===//
// Frame and Set directives
//===----------------------------------------------------------------------===//
/// EmitFunctionBodyStart - Targets can override this to emit stuff before
/// the first basic block in the function.
void Sw64AsmPrinter::emitFunctionBodyStart() {
  MCInstLowering.Initialize(&MF->getContext());
}

/// EmitFunctionBodyEnd - Targets can override this to emit stuff after
/// the last basic block in the function.
void Sw64AsmPrinter::emitFunctionBodyEnd() {
  // Emit function end directives
  Sw64TargetStreamer &TS = getTargetStreamer();

  // There are instruction for this macros, but they must
  // always be at the function end, and we can't emit and
  // break with BB logic.
  TS.emitDirectiveSetAt();
  TS.emitDirectiveSetMacro();
  TS.emitDirectiveSetReorder();

  TS.emitDirectiveEnd(CurrentFnSym->getName());
  // Make sure to terminate any constant pools that were at the end
  // of the function.
  if (!InConstantPool)
    return;
  InConstantPool = false;
  OutStreamer->emitDataRegion(MCDR_DataRegionEnd);
}

void Sw64AsmPrinter::emitFunctionEntryLabel() {
  Sw64TargetStreamer &TS = getTargetStreamer();

  TS.emitDirectiveEnt(*CurrentFnSym);
  OutStreamer->emitLabel(CurrentFnSym);
}

void Sw64AsmPrinter::printOperand(const MachineInstr *MI, int opNum,
                                  raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(opNum);

  if (MO.isReg()) {
    assert(Register::isPhysicalRegister(MO.getReg()) && "Not physreg??");
    O << Sw64InstPrinter::getRegisterName(MO.getReg());
  } else if (MO.isImm()) {
    O << MO.getImm();
  } else {
    printOp(MO, O);
  }
}
void Sw64AsmPrinter::printOp(const MachineOperand &MO, raw_ostream &O) {
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << Sw64InstPrinter::getRegisterName(MO.getReg());
    return;

  case MachineOperand::MO_Immediate:
    assert(0 && "printOp() does not handle immediate values");
    return;

  case MachineOperand::MO_MachineBasicBlock:
    MO.getMBB()->getSymbol()->print(O, MAI);
    return;

  case MachineOperand::MO_ConstantPoolIndex:
    O << MAI->getPrivateGlobalPrefix() << "CPI" << getFunctionNumber() << "_"
      << MO.getIndex();
    return;

  case MachineOperand::MO_ExternalSymbol:
    O << MO.getSymbolName();
    return;

  case MachineOperand::MO_GlobalAddress:
    getSymbol(MO.getGlobal())->print(O, MAI);
    return;
  case MachineOperand::MO_JumpTableIndex:
    O << MAI->getPrivateGlobalPrefix() << "JTI" << getFunctionNumber() << '_'
      << MO.getIndex();
    return;

  default:
    O << "<unknown operand type: "; //  << MO.getType() << ">";
    return;
  }
}

/// PrintAsmOperand - Print out an operand for an inline asm expression.
bool Sw64AsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                     const char *ExtraCode, raw_ostream &O) {
  // Print the operand if there is no operand modifier.
  if (!ExtraCode || !ExtraCode[0]) {
    printOperand(MI, OpNo, O);
    return false;
  }
  if (ExtraCode && ExtraCode[0])
    if (ExtraCode[1] != 0)
      return true;

  switch (ExtraCode[0]) {
  default:
    return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, O);
  case 'r':
    printOperand(MI, OpNo, O);
    return false;
  }
  // Otherwise fallback on the default implementation.
  return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, O);
}

void Sw64AsmPrinter::emitStartOfAsmFile(Module &M) {
  if (OutStreamer->hasRawTextSupport()) {
    OutStreamer->emitRawText(StringRef("\t.set noreorder"));
    OutStreamer->emitRawText(StringRef("\t.set volatile"));
    OutStreamer->emitRawText(StringRef("\t.set noat"));
    OutStreamer->emitRawText(StringRef("\t.set nomacro"));
  }
}

bool Sw64AsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                           unsigned OpNum,
                                           const char *ExtraCode,
                                           raw_ostream &O) {
  assert(OpNum + 1 < MI->getNumOperands() && "Insufficient operands");

  const MachineOperand &BaseMO = MI->getOperand(OpNum);

  assert(BaseMO.isReg() &&
         "Unexpected base pointer for inline asm memory operand.");

  if (ExtraCode && ExtraCode[0]) {
    return true; // Unknown modifier.
  }

  O << "0(" << Sw64InstPrinter::getRegisterName(BaseMO.getReg()) << ")";

  return false;
}

#include "Sw64GenMCPseudoLowering.inc"

void Sw64AsmPrinter::emitInstruction(const MachineInstr *MI) {
  if (MI->isDebugValue())
    return;
  SmallString<128> Str;
  raw_svector_ostream O(Str);

  if (emitPseudoExpansionLowering(*OutStreamer, MI))
    return;

  if (MI->getOpcode() == Sw64::STQ_C || MI->getOpcode() == Sw64::STL_C)
    OutStreamer->emitCodeAlignment(Align(8), &getSubtargetInfo());

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);

  EmitToStreamer(*OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSw64AsmPrinter() {
  RegisterAsmPrinter<Sw64AsmPrinter> X(getTheSw64Target());
}
