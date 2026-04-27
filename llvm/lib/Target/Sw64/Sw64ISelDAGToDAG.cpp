//===-- Sw64ISelDAGToDAG.cpp - Sw64 pattern matching inst selector ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines a pattern matching instruction selector for Sw64,
// converting from a legalized dag to a Sw64 dag.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "sw_64-isel"
#define PASS_NAME "Sw64 DAG->DAG Pattern Instruction Selection"

#include "MCTargetDesc/Sw64BaseInfo.h"
#include "Sw64.h"
#include "Sw64MachineFunctionInfo.h"
#include "Sw64Subtarget.h"
#include "Sw64TargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm>
using namespace llvm;

namespace {

//===--------------------------------------------------------------------===//
/// Sw64DAGToDAGISel - Sw64 specific code to select Sw64 machine
/// instructions for SelectionDAG operations.
class Sw64DAGToDAGISel : public SelectionDAGISel {
  const Sw64Subtarget *Subtarget;

  static const int64_t IMM_LOW = -32768;
  static const int64_t IMM_HIGH = 32767;
  static const int64_t IMM_MULT = 65536;
  static const int64_t IMM_FULLHIGH = IMM_HIGH + IMM_HIGH * IMM_MULT;
  static const int64_t IMM_FULLLOW = IMM_LOW + IMM_LOW * IMM_MULT;

  static int64_t get_ldah16(int64_t x) {
    int64_t y = x / IMM_MULT;
    if (x % IMM_MULT > IMM_HIGH)
      ++y;
    if (x % IMM_MULT < IMM_LOW)
      --y;
    return y;
  }

  static int64_t get_lda16(int64_t x) { return x - get_ldah16(x) * IMM_MULT; }

  /// get_zapImm - Return a zap mask if X is a valid immediate for a zapnot
  /// instruction (if not, return 0).  Note that this code accepts partial
  /// zap masks.  For example (and LHS, 1) is a valid zap, as long we know
  /// that the bits 1-7 of LHS are already zero.  If LHS is non-null, we are
  /// in checking mode.  If LHS is null, we assume that the mask has already
  /// been validated before.
  uint64_t get_zapImm(SDValue LHS, uint64_t Constant) const {
    uint64_t BitsToCheck = 0;
    unsigned Result = 0;
    for (unsigned i = 0; i != 8; ++i) {
      if (((Constant >> 8 * i) & 0xFF) == 0) {
        // nothing to do.
      } else {
        Result |= 1 << i;
        if (((Constant >> 8 * i) & 0xFF) == 0xFF) {
          // If the entire byte is set, zapnot the byte.
        } else if (LHS.getNode() == 0) {
          // Otherwise, if the mask was previously validated, we know its okay
          // to zapnot this entire byte even though all the bits aren't set.
        } else {
          // Otherwise we don't know that the it's okay to zapnot this entire
          // byte.  Only do this iff we can prove that the missing bits are
          // already null, so the bytezap doesn't need to really null them.
          BitsToCheck |= ~Constant & (0xFFULL << 8 * i);
        }
      }
    }

    // If there are missing bits in a byte (for example, X & 0xEF00), check to
    // see if the missing bits (0x1000) are already known zero if not, the zap
    // isn't okay to do, as it won't clear all the required bits.
    if (BitsToCheck && !CurDAG->MaskedValueIsZero(
                           LHS, APInt(LHS.getValueSizeInBits(), BitsToCheck)))
      return 0;

    return Result;
  }

  static uint64_t get_zapImm(uint64_t x) {
    unsigned build = 0;
    for (int i = 0; i != 8; ++i) {
      if ((x & 0x00FF) == 0x00FF)
        build |= 1 << i;
      else if ((x & 0x00FF) != 0)
        return 0;
      x >>= 8;
    }
    return build;
  }

  static uint64_t getNearPower2(uint64_t x) {
    if (!x)
      return 0;
    unsigned at = __builtin_clzll(x);
    uint64_t complow = 1ULL << (63 - at);
    uint64_t comphigh = complow << 1;
    if (x - complow <= comphigh - x)
      return complow;
    else
      return comphigh;
  }

  static bool chkRemNearPower2(uint64_t x, uint64_t r, bool swap) {
    uint64_t y = getNearPower2(x);
    if (swap)
      return (y - x) == r;
    else
      return (x - y) == r;
  }

public:
  static char ID;

  Sw64DAGToDAGISel() = delete;

  explicit Sw64DAGToDAGISel(Sw64TargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(ID, TM, OptLevel), Subtarget(nullptr) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<Sw64Subtarget>();
    return SelectionDAGISel::runOnMachineFunction(MF);
  }
  /// getI64Imm - Return a target constant with the specified value, of type
  /// i64.
  inline SDValue getI64Imm(int64_t Imm, const SDLoc &dl) {
    return CurDAG->getTargetConstant(Imm, dl, MVT::i64);
  }

  inline SDValue getI32Imm(unsigned Imm, const SDLoc &dl) {
    return CurDAG->getTargetConstant(Imm, dl, MVT::i32);
  }

  static SDNode *selectImm(SelectionDAG *CurDAG, const SDLoc &DL, int64_t Imm);
  // Select - Convert the specified operand from a target-independent to a
  // target-specific node if it hasn't already been changed.
  void Select(SDNode *N) override;
  StringRef getPassName() const override {
    return "Sw64 DAG->DAG Pattern Instruction Selection";
  }

  /// SelectInlineAsmMemoryOperand - Implement addressing mode selection for
  /// inline asm expressions.
  bool SelectInlineAsmMemoryOperand(const SDValue &Op, unsigned ConstraintID,
                                    std::vector<SDValue> &OutOps) override;

  template <MVT::SimpleValueType VT>
  bool SelectAddSubImm(SDValue N, SDValue &Imm) {
    return SelectAddSubImm(N, VT, Imm);
  }

  bool selectAddrFrameIndex(SDValue Addr, SDValue &Base, SDValue &Offset) const;
  bool selectAddrFrameIndexOffset(SDValue Addr, SDValue &Base, SDValue &Offset,
                                  unsigned OffsetBits,
                                  unsigned ShiftAmount) const;
  bool selectAddrRegImm9(SDValue Addr, SDValue &Base, SDValue &Offset) const;
  bool selectAddrRegImm16(SDValue Addr, SDValue &Base, SDValue &Offset) const;

  /// abs64 - absolute value of a 64-bit int.  Not all environments support
  /// "abs" on whatever their name for the 64-bit int type is.  The absolute
  /// value of the largest negative number is undefined, as with "abs".
  inline int64_t abs64(int64_t x) { return (x < 0) ? -x : x; }

// Include the pieces autogenerated from the target description.
#include "Sw64GenDAGISel.inc"

private:
  /// getTargetMachine - Return a reference to the TargetMachine, casted
  /// to the target-specific type.
  const Sw64TargetMachine &getTargetMachine() {
    return static_cast<const Sw64TargetMachine &>(TM);
  }

  bool SelectAddSubImm(SDValue N, MVT VT, SDValue &Imm);
  bool SelectComplexImm(SDValue N, SDValue &Imm);

  SDNode *getGlobalBaseReg();
  SDNode *getGlobalRetAddr();
  void SelectCALL(SDNode *Op);
  bool tryIndexedLoad(SDNode *N);
  bool tryIndexedStore(SDNode *N);
  bool selectSExti32(SDValue N, SDValue &Val);
  bool selectZExti32(SDValue N, SDValue &Val);

  /// Select constant vector splats.
  bool selectVSplat(SDNode *N, APInt &Imm, unsigned MinSizeInBits) const;
  /// Select constant vector splats whose value fits in a given integer.
  bool selectVSplatCommon(SDValue N, SDValue &Imm, bool Signed,
                          unsigned ImmBitSize) const;
  /// Select constant vector splats whose value fits in a uimm8.
  bool selectVSplatUimm8(SDValue N, SDValue &Imm) const;

  bool selectVSplatSimm8(SDValue N, SDValue &Imm) const;
  bool selectAddrDefault(SDValue Addr, SDValue &Base, SDValue &Offset) const;

  bool selectIntAddrSImm16(SDValue Addr, SDValue &Base, SDValue &Offset) const;

  bool selectIntAddrSImm12(SDValue Addr, SDValue &Base, SDValue &Offset) const;

  bool SelectAddrFI(SDValue Addr, SDValue &Base);
};
} // end anonymous namespace
char Sw64DAGToDAGISel::ID = 0;

INITIALIZE_PASS(Sw64DAGToDAGISel, DEBUG_TYPE, PASS_NAME, false, false)

/// getGlobalBaseReg - Output the instructions required to put the
/// GOT address into a register.
///
SDNode *Sw64DAGToDAGISel::getGlobalBaseReg() {
  unsigned GlobalBaseReg = Subtarget->getInstrInfo()->getGlobalBaseReg(MF);
  return CurDAG
      ->getRegister(GlobalBaseReg,
                    getTargetLowering()->getPointerTy(CurDAG->getDataLayout()))
      .getNode();
}

/// getGlobalRetAddr - Grab the return address.
///
SDNode *Sw64DAGToDAGISel::getGlobalRetAddr() {
  unsigned GlobalRetAddr = Subtarget->getInstrInfo()->getGlobalRetAddr(MF);
  return CurDAG
      ->getRegister(GlobalRetAddr,
                    getTargetLowering()->getPointerTy(CurDAG->getDataLayout()))
      .getNode();
}

bool Sw64DAGToDAGISel::SelectAddrFI(SDValue Addr, SDValue &Base) {
  if (auto FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i64);
    return true;
  }

  return false;
}

// Select - Convert the specified operand from a target-independent to a
// target-specific node if it hasn't already been changed.
void Sw64DAGToDAGISel::Select(SDNode *N) {

  // Dump information about the Node being selected
  LLVM_DEBUG(errs() << "Selecting: "; N->dump(CurDAG); errs() << "\n");

  // If we have a custom node, we already have selected!
  if (N->isMachineOpcode()) {
    LLVM_DEBUG(errs() << "== "; N->dump(CurDAG); errs() << "\n");
    return;
  }
  SDLoc dl(N);
  switch (N->getOpcode()) {
  default:
    break;
  case ISD::LOAD:
    if (tryIndexedLoad(N))
      return;
    // Other cases are autogenerated.
    break;
  case ISD::STORE:
    if (tryIndexedStore(N))
      return;
    // Other cases are autogenerated.
    break;
  case Sw64ISD::CALL:
    SelectCALL(N);
    if (N->use_empty()) // Don't delete EntryToken, etc.
      CurDAG->RemoveDeadNode(N);
    return;
  case ISD::FrameIndex: {
    assert(N->getValueType(0) == MVT::i64);
    int FI = cast<FrameIndexSDNode>(N)->getIndex();
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i32);
    if (N->hasOneUse()) {
      N->setDebugLoc((*(N->use_begin()))->getDebugLoc());
      CurDAG->SelectNodeTo(N, Sw64::LDA, MVT::i64, TFI,
                           CurDAG->getTargetConstant(0, dl, MVT::i64));
      return;
    }
    ReplaceNode(
        N, CurDAG->getMachineNode(Sw64::LDA, dl, MVT::i64, TFI,
                                  CurDAG->getTargetConstant(0, dl, MVT::i64)));
    return;
  }
  case ISD::GLOBAL_OFFSET_TABLE:
    ReplaceNode(N, getGlobalBaseReg());
    return;
  case Sw64ISD::GlobalRetAddr:
    ReplaceNode(N, getGlobalRetAddr());
    return;

  case Sw64ISD::DivCall: {
    SDValue Chain = CurDAG->getEntryNode();
    SDValue N0 = N->getOperand(0);
    SDValue N1 = N->getOperand(1);
    SDValue N2 = N->getOperand(2);
    Chain = CurDAG->getCopyToReg(Chain, dl, Sw64::R24, N1, SDValue(0, 0));
    Chain = CurDAG->getCopyToReg(Chain, dl, Sw64::R25, N2, Chain.getValue(1));
    Chain = CurDAG->getCopyToReg(Chain, dl, Sw64::R27, N0, Chain.getValue(1));
    SDNode *CNode = CurDAG->getMachineNode(Sw64::PseudoCallDiv, dl, MVT::Other,
                                           MVT::Glue, Chain, Chain.getValue(1));
    Chain = CurDAG->getCopyFromReg(Chain, dl, Sw64::R27, MVT::i64,
                                   SDValue(CNode, 1));
    ReplaceNode(N,
                CurDAG->getMachineNode(Sw64::BISr, dl, MVT::i64, Chain, Chain));
    return;
  }

  case ISD::READCYCLECOUNTER: {
    SDValue Chain = N->getOperand(0);
    ReplaceNode(
        N, CurDAG->getMachineNode(Sw64::RPCC, dl, MVT::i64, MVT::Other, Chain));
    return;
  }

  case ISD::Constant: {
    auto ConstNode = cast<ConstantSDNode>(N);
    if (ConstNode->isZero()) {
      SDValue Result = CurDAG->getCopyFromReg(CurDAG->getEntryNode(), dl,
                                              Sw64::R31, MVT::i64);
      ReplaceUses(SDValue(N, 0), Result);
      return;
    }
    uint64_t uval = cast<ConstantSDNode>(N)->getZExtValue();
    int64_t Imm = ConstNode->getSExtValue();
    int64_t val = Imm;
    int32_t val32 = (int32_t)val;
    if (val <= IMM_HIGH + IMM_HIGH * IMM_MULT &&
        val >= IMM_LOW + IMM_LOW * IMM_MULT)
      break;                 //(LDAH (LDA))
    if ((uval >> 32) == 0 && // empty upper bits
        val32 <= IMM_HIGH + IMM_HIGH * IMM_MULT)
      break; //(zext (LDAH (LDA)))
    // Else use the constant pool

    ConstantInt *C =
        ConstantInt::get(Type::getInt64Ty(*CurDAG->getContext()), uval);
    SDValue CPI = CurDAG->getTargetConstantPool(C, MVT::i64);
    SDNode *Load =
        CurDAG->getMachineNode(Sw64::LOADconstant, dl, MVT::i64, CPI);
    ReplaceNode(N, Load);

    return;
  }
  case ISD::TargetConstantFP:
  case ISD::ConstantFP: {
    ConstantFPSDNode *CN = cast<ConstantFPSDNode>(N);
    bool isDouble = N->getValueType(0) == MVT::f64;
    EVT T = isDouble ? MVT::f64 : MVT::f32;
    if (CN->getValueAPF().isPosZero()) {
      ReplaceNode(
          N, CurDAG->getMachineNode(isDouble ? Sw64::CPYSD : Sw64::CPYSS, dl, T,
                                    CurDAG->getRegister(Sw64::F31, T),
                                    CurDAG->getRegister(Sw64::F31, T)));
      return;
    } else if (CN->getValueAPF().isNegZero()) {
      ReplaceNode(
          N, CurDAG->getMachineNode(isDouble ? Sw64::CPYSND : Sw64::CPYSNS, dl,
                                    T, CurDAG->getRegister(Sw64::F31, T),
                                    CurDAG->getRegister(Sw64::F31, T)));
      return;
    } else {
      report_fatal_error("Unhandled FP constant type");
    }
    break;
  }

  case ISD::SETCC:
    if (N->getSimpleValueType(0).SimpleTy == MVT::v4i64)
      break;
    if (N->getOperand(0).getNode()->getValueType(0).isFloatingPoint()) {
      ISD::CondCode CC = cast<CondCodeSDNode>(N->getOperand(2))->get();

      unsigned Opc = Sw64::WTF;
      bool rev = false;
      bool inv = false;
      bool ordonly = false;
      if (Sw64Mieee) {
        switch (CC) {
        default:
          LLVM_DEBUG(N->dump(CurDAG));
          llvm_unreachable("Unknown FP comparison!");
        case ISD::SETEQ:
        case ISD::SETOEQ:
        case ISD::SETUEQ:
          Opc = Sw64::CMPTEQ;
          break;
        case ISD::SETLT:
        case ISD::SETOLT:
        case ISD::SETULT:
          Opc = Sw64::CMPTLT;
          break;
        case ISD::SETLE:
        case ISD::SETOLE:
        case ISD::SETULE:
          Opc = Sw64::CMPTLE;
          break;
        case ISD::SETGT:
        case ISD::SETOGT:
        case ISD::SETUGT:
          Opc = Sw64::CMPTLT;
          rev = true;
          break;
        case ISD::SETGE:
        case ISD::SETOGE:
        case ISD::SETUGE:
          Opc = Sw64::CMPTLE;
          rev = true;
          break;
        case ISD::SETNE:
        case ISD::SETONE:
        case ISD::SETUNE:
          Opc = Sw64::CMPTEQ;
          inv = true;
          break;
        case ISD::SETO:
          Opc = Sw64::CMPTUN;
          inv = true;
          ordonly = true;
          break;
        case ISD::SETUO:
          Opc = Sw64::CMPTUN;
          ordonly = true;
          break;
        };

        /*
           unordered:
           FCMPUN $f1, $f2, $f3
           FCMPxx $f1, $f2, $f3
           FSELNE $f3, $f3, $f4, $f4

           ordered:
           FCMPUN $f1, $f2, $f3
           FCMPxx $f1, $f2, $f3
           FSELEQ $f3, $f4, $f31, $f4

           SETO/SETUO:
           FCMPxx $f1, $f2, $f3
        */
        bool ordered = true;
        switch (CC) {
        case ISD::SETUEQ:
        case ISD::SETULT:
        case ISD::SETULE:
        case ISD::SETUNE:
        case ISD::SETUGT:
        case ISD::SETUGE:
          ordered = false;
          break;
        default:
          break;
        }
        SDValue opr0 = N->getOperand(rev ? 1 : 0);
        SDValue opr1 = N->getOperand(rev ? 0 : 1);
        SDNode *cmpu =
            CurDAG->getMachineNode(Sw64::CMPTUN, dl, MVT::f64, opr0, opr1);
        SDNode *cmp = CurDAG->getMachineNode(Opc, dl, MVT::f64, opr0, opr1);
        if (inv)
          cmp = CurDAG->getMachineNode(
              Sw64::CMPTEQ, dl, MVT::f64, SDValue(cmp, 0),
              CurDAG->getRegister(Sw64::F31, MVT::f64));

        SDNode *sel = NULL;
        if (ordonly)
          sel = cmp;
        else if (ordered)
          sel = CurDAG->getMachineNode(Sw64::FSELEQD, dl, MVT::f64,
                                       CurDAG->getRegister(Sw64::F31, MVT::f64),
                                       SDValue(cmp, 0), SDValue(cmpu, 0));
        else
          sel = CurDAG->getMachineNode(Sw64::FSELNED, dl, MVT::f64,
                                       SDValue(cmp, 0), SDValue(cmpu, 0),
                                       SDValue(cmpu, 0));

        MVT VT = N->getSimpleValueType(0).SimpleTy == MVT::v4i64 ? MVT::v4i64
                                                                 : MVT::i64;
        SDNode *LD =
            CurDAG->getMachineNode(Sw64::FTOIT, dl, VT, SDValue(sel, 0));

        ReplaceNode(N, CurDAG->getMachineNode(
                           Sw64::CMPULTr, dl, VT,
                           CurDAG->getRegister(Sw64::R31, VT), SDValue(LD, 0)));
        return;
      } else {
        switch (CC) {
        default:
          LLVM_DEBUG(N->dump(CurDAG));
          llvm_unreachable("Unknown FP comparison!");
        case ISD::SETEQ:
        case ISD::SETOEQ:
        case ISD::SETUEQ:
          Opc = Sw64::CMPTEQ;
          break;
        case ISD::SETLT:
        case ISD::SETOLT:
        case ISD::SETULT:
          Opc = Sw64::CMPTLT;
          break;
        case ISD::SETLE:
        case ISD::SETOLE:
        case ISD::SETULE:
          Opc = Sw64::CMPTLE;
          break;
        case ISD::SETGT:
        case ISD::SETOGT:
        case ISD::SETUGT:
          Opc = Sw64::CMPTLT;
          rev = true;
          break;
        case ISD::SETGE:
        case ISD::SETOGE:
        case ISD::SETUGE:
          Opc = Sw64::CMPTLE;
          rev = true;
          break;
        case ISD::SETNE:
        case ISD::SETONE:
        case ISD::SETUNE:
          Opc = Sw64::CMPTEQ;
          inv = true;
          break;
        case ISD::SETO:
          Opc = Sw64::CMPTUN;
          inv = true;
          break;
        case ISD::SETUO:
          Opc = Sw64::CMPTUN;
          break;
        };
        SDValue tmp1 = N->getOperand(rev ? 1 : 0);
        SDValue tmp2 = N->getOperand(rev ? 0 : 1);
        SDNode *cmp = CurDAG->getMachineNode(Opc, dl, MVT::f64, tmp1, tmp2);
        if (inv)
          cmp = CurDAG->getMachineNode(
              Sw64::CMPTEQ, dl, MVT::f64, SDValue(cmp, 0),
              CurDAG->getRegister(Sw64::F31, MVT::f64));
        switch (CC) {
        case ISD::SETUEQ:
        case ISD::SETULT:
        case ISD::SETULE:
        case ISD::SETUNE:
        case ISD::SETUGT:
        case ISD::SETUGE: {
          SDNode *cmp2 =
              CurDAG->getMachineNode(Sw64::CMPTUN, dl, MVT::f64, tmp1, tmp2);
          cmp = CurDAG->getMachineNode(Sw64::ADDD, dl, MVT::f64,
                                       SDValue(cmp2, 0), SDValue(cmp, 0));
          break;
        }
        default:
          break;
        }
        SDNode *LD =
            CurDAG->getMachineNode(Sw64::FTOIT, dl, MVT::i64, SDValue(cmp, 0));

        ReplaceNode(
            N, CurDAG->getMachineNode(Sw64::CMPULTr, dl, MVT::i64,
                                      CurDAG->getRegister(Sw64::R31, MVT::i64),
                                      SDValue(LD, 0)));
        return;
      }
    }
    break;
  case ISD::AND: {
    ConstantSDNode *SC = NULL;
    ConstantSDNode *MC = NULL;
    if (N->getOperand(0).getOpcode() == ISD::SRL &&
        (MC = dyn_cast<ConstantSDNode>(N->getOperand(1))) &&
        (SC = dyn_cast<ConstantSDNode>(N->getOperand(0).getOperand(1)))) {
      uint64_t sval = SC->getZExtValue();
      uint64_t mval = MC->getZExtValue();
      // If the result is a zap, let the autogened stuff handle it.
      if (get_zapImm(N->getOperand(0), mval))
        break;
      // given mask X, and shift S, we want to see if there is any zap in the
      // mask if we play around with the botton S bits
      uint64_t dontcare = (~0ULL) >> (64 - sval);
      uint64_t mask = mval << sval;

      if (get_zapImm(mask | dontcare))
        mask = mask | dontcare;

      if (get_zapImm(mask)) {
        SDValue Z =
            SDValue(CurDAG->getMachineNode(Sw64::ZAPNOTi, dl, MVT::i64,
                                           N->getOperand(0).getOperand(0),
                                           getI64Imm(get_zapImm(mask), dl)),
                    0);
        ReplaceNode(N, CurDAG->getMachineNode(Sw64::SRLi, dl, MVT::i64, Z,
                                              getI64Imm(sval, dl)));
        return;
      }
    }
    break;
  }
  case ISD::BUILD_VECTOR: {

    BuildVectorSDNode *BVN = cast<BuildVectorSDNode>(N);
    APInt SplatValue, SplatUndef;
    unsigned SplatBitSize;
    bool HasAnyUndefs;
    EVT ViaVecTy;

    if (!Subtarget->hasSIMD() || !BVN->getValueType(0).is256BitVector())
      return;

    if (!BVN->isConstantSplat(SplatValue, SplatUndef, SplatBitSize,
                              HasAnyUndefs, 8, false))
      break;
  }
  }
  // Select the default instruction
  SelectCode(N);
}

void Sw64DAGToDAGISel::SelectCALL(SDNode *N) {
  // TODO: add flag stuff to prevent nondeturministic breakage!

  SDValue Chain = N->getOperand(0);
  SDValue Addr = N->getOperand(1);
  SDValue InFlag = N->getOperand(N->getNumOperands() - 1);
  SDLoc dl(N);
  if (Addr.getOpcode() == Sw64ISD::GPRelLo) {
    SDValue GOT = SDValue(getGlobalBaseReg(), 0);
    Chain = CurDAG->getCopyToReg(Chain, dl, Sw64::R29, GOT, InFlag);
    InFlag = Chain.getValue(1);
    Chain = SDValue(CurDAG->getMachineNode(Sw64::BSR, dl, MVT::Other, MVT::Glue,
                                           Addr.getOperand(0), Chain, InFlag),
                    0);
  } else {
    Chain = CurDAG->getCopyToReg(Chain, dl, Sw64::R27, Addr, InFlag);
    InFlag = Chain.getValue(1);
    SDValue Ops[] = {Chain, CurDAG->getRegister(Sw64::R27, MVT::i64),
                     N->getOperand(2), InFlag};
    Chain = SDValue(
        CurDAG->getMachineNode(Sw64::JSR, dl, MVT::Other, MVT::Glue, Ops), 0);
  }
  InFlag = Chain.getValue(1);

  ReplaceUses(SDValue(N, 0), Chain);
  ReplaceUses(SDValue(N, 1), InFlag);
}

/// Match frameindex
bool Sw64DAGToDAGISel::selectAddrFrameIndex(SDValue Addr, SDValue &Base,
                                            SDValue &Offset) const {
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    EVT ValTy = Addr.getValueType();

    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), ValTy);
    Offset = CurDAG->getTargetConstant(0, SDLoc(Addr), ValTy);
    return true;
  }
  return false;
}

/// Match frameindex+offset and frameindex|offset
bool Sw64DAGToDAGISel::selectAddrFrameIndexOffset(
    SDValue Addr, SDValue &Base, SDValue &Offset, unsigned OffsetBits,
    unsigned ShiftAmount = 0) const {
  if (CurDAG->isBaseWithConstantOffset(Addr)) {
    ConstantSDNode *CN = dyn_cast<ConstantSDNode>(Addr.getOperand(1));
    if (isIntN(OffsetBits + ShiftAmount, CN->getSExtValue())) {
      EVT ValTy = Addr.getValueType();

      // If the first operand is a FI, get the TargetFI Node
      if (FrameIndexSDNode *FIN =
              dyn_cast<FrameIndexSDNode>(Addr.getOperand(0)))
        Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), ValTy);
      else {
        Base = Addr.getOperand(0);
        // If base is a FI, additional offset calculation is done in
        // eliminateFrameIndex, otherwise we need to check the alignment
        const Align Alignment(1ULL << ShiftAmount);
        if (!isAligned(Alignment, CN->getZExtValue()))
          return false;
      }

      Offset =
          CurDAG->getTargetConstant(CN->getZExtValue(), SDLoc(Addr), ValTy);
      return true;
    }
  }
  return false;
}

bool Sw64DAGToDAGISel::selectAddrRegImm9(SDValue Addr, SDValue &Base,
                                         SDValue &Offset) const {
  if (selectAddrFrameIndex(Addr, Base, Offset))
    return true;

  if (selectAddrFrameIndexOffset(Addr, Base, Offset, 9))
    return true;

  return false;
}

bool Sw64DAGToDAGISel::selectAddrRegImm16(SDValue Addr, SDValue &Base,
                                          SDValue &Offset) const {
  if (selectAddrFrameIndex(Addr, Base, Offset))
    return true;

  if (selectAddrFrameIndexOffset(Addr, Base, Offset, 16))
    return true;

  return false;
}

bool Sw64DAGToDAGISel::SelectInlineAsmMemoryOperand(
    const SDValue &Op, unsigned ConstraintID, std::vector<SDValue> &OutOps) {
  SDValue Base, Offset;

  switch (ConstraintID) {
  default:
    llvm_unreachable("Unexpected asm memory constraint");
  case InlineAsm::Constraint_i:
  case InlineAsm::Constraint_m:
  case InlineAsm::Constraint_Q:
    // We need to make sure that this one operand does not end up in XZR, thus
    //  require the address to be in a PointerRegClass register.
    const TargetRegisterInfo *TRI = Subtarget->getRegisterInfo();
    const TargetRegisterClass *TRC = TRI->getPointerRegClass(*MF);
    SDLoc dl(Op);
    SDValue RC = CurDAG->getTargetConstant(TRC->getID(), dl, MVT::i64);
    SDValue NewOp =
        SDValue(CurDAG->getMachineNode(TargetOpcode::COPY_TO_REGCLASS, dl,
                                       Op.getValueType(), Op, RC),
                0);
    OutOps.push_back(NewOp);
    return false;
  }
  return true;
}

bool Sw64DAGToDAGISel::tryIndexedLoad(SDNode *N) {
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::MemIndexedMode AM = LD->getAddressingMode();
  if (AM != ISD::POST_INC)
    return false;
  SDLoc dl(N);
  MVT VT = LD->getMemoryVT().getSimpleVT();
  bool isFloat = false;
  unsigned Opcode = 0;
  switch (VT.SimpleTy) {
  case MVT::i8:
    Opcode = Sw64::LDBU_A;
    break;
  case MVT::i16:
    Opcode = Sw64::LDHU_A;
    break;
  case MVT::i32:
    Opcode = Sw64::LDW_A;
    break;
  case MVT::i64:
    Opcode = Sw64::LDL_A;
    break;
  case MVT::f32:
    Opcode = Sw64::LDS_A;
    isFloat = true;
    break;
  case MVT::f64:
    Opcode = Sw64::LDD_A;
    isFloat = true;
    break;
  default:
    return false;
  }
  SDValue Offset = LD->getOffset();
  int64_t Inc = cast<ConstantSDNode>(Offset.getNode())->getSExtValue();
  ReplaceNode(
      N, CurDAG->getMachineNode(Opcode, SDLoc(N), isFloat ? VT : MVT::i64,
                                MVT::i64, MVT::Other, LD->getBasePtr(),
                                CurDAG->getTargetConstant(Inc, dl, MVT::i64),
                                LD->getChain()));
  return true;
}

bool Sw64DAGToDAGISel::tryIndexedStore(SDNode *N) {
  StoreSDNode *ST = cast<StoreSDNode>(N);
  ISD::MemIndexedMode AM = ST->getAddressingMode();
  if (AM != ISD::POST_INC)
    return false;
  SDLoc dl(N);
  MVT VT = ST->getMemoryVT().getSimpleVT();
  unsigned Opcode = 0;
  switch (VT.SimpleTy) {
  case MVT::i8:
    Opcode = Sw64::STB_A;
    break;
  case MVT::i16:
    Opcode = Sw64::STH_A;
    break;
  case MVT::i32:
    Opcode = Sw64::STW_A;
    break;
  case MVT::i64:
    Opcode = Sw64::STL_A;
    break;
  case MVT::f32:
    Opcode = Sw64::STS_A;
    break;
  case MVT::f64:
    Opcode = Sw64::STD_A;
    break;
  default:
    return false;
  }
  MachineMemOperand *MemOp = ST->getMemOperand();
  SDValue From[2] = {SDValue(ST, 0), SDValue(ST, 1)};
  SDValue To[2];
  int64_t Inc = cast<ConstantSDNode>(ST->getOffset().getNode())->getSExtValue();
  SDValue Ops[] = {ST->getValue(), ST->getBasePtr(),
                   CurDAG->getTargetConstant(Inc, dl, MVT::i64),
                   ST->getChain()};
  MachineSDNode *S =
      CurDAG->getMachineNode(Opcode, dl, MVT::i64, MVT::Other, Ops);
  CurDAG->setNodeMemRefs(S, {MemOp});
  To[0] = SDValue(S, 0);
  To[1] = SDValue(S, 1);
  ReplaceUses(From, To, 2);
  CurDAG->RemoveDeadNode(ST);
  return true;
}

/// ComplexPattern used on Sw64InstrInfo
/// Used on Sw64 Load/Store instructions
bool Sw64DAGToDAGISel::selectAddrDefault(SDValue Addr, SDValue &Base,
                                         SDValue &Offset) const {
  Base = Addr;
  Offset = CurDAG->getTargetConstant(0, SDLoc(Addr), Addr.getValueType());
  return true;
}

// Select constant vector splats.
//
// Returns true and sets Imm if:
// * MSA is enabled
// * N is a ISD::BUILD_VECTOR representing a constant splat
bool Sw64DAGToDAGISel::selectVSplat(SDNode *N, APInt &Imm,
                                    unsigned MinSizeInBits) const {
  BuildVectorSDNode *Node = dyn_cast<BuildVectorSDNode>(N);

  if (!Node)
    return false;

  APInt SplatValue, SplatUndef;
  unsigned SplatBitSize;
  bool HasAnyUndefs;

  if (!Node->isConstantSplat(SplatValue, SplatUndef, SplatBitSize, HasAnyUndefs,
                             MinSizeInBits, false))
    return false;

  Imm = SplatValue;

  return true;
}

bool Sw64DAGToDAGISel::selectVSplatCommon(SDValue N, SDValue &Imm, bool Signed,
                                          unsigned ImmBitSize) const {
  APInt ImmValue;
  EVT EltTy = N->getValueType(0).getVectorElementType();

  if (N->getOpcode() == ISD::BITCAST)
    N = N->getOperand(0);

  if (selectVSplat(N.getNode(), ImmValue, EltTy.getSizeInBits()) &&
      ImmValue.getBitWidth() == EltTy.getSizeInBits()) {

    if ((Signed && ImmValue.isSignedIntN(ImmBitSize)) ||
        (!Signed && ImmValue.isIntN(ImmBitSize))) {
      Imm = CurDAG->getTargetConstant(ImmValue, SDLoc(N), EltTy);
      return true;
    }
  }

  return false;
}

// Select constant vector splats.
bool Sw64DAGToDAGISel::selectVSplatSimm8(SDValue N, SDValue &Imm) const {
  return selectVSplatCommon(N, Imm, true, 8);
}

bool Sw64DAGToDAGISel::selectVSplatUimm8(SDValue N, SDValue &Imm) const {
  return selectVSplatCommon(N, Imm, false, 8);
}

bool Sw64DAGToDAGISel::selectIntAddrSImm16(SDValue Addr, SDValue &Base,
                                           SDValue &Offset) const {
  if (selectAddrFrameIndex(Addr, Base, Offset))
    return true;

  if (selectAddrFrameIndexOffset(Addr, Base, Offset, 10, 2))
    return true;

  return selectAddrDefault(Addr, Base, Offset);
}

bool Sw64DAGToDAGISel::selectIntAddrSImm12(SDValue Addr, SDValue &Base,
                                           SDValue &Offset) const {
  if (selectAddrFrameIndex(Addr, Base, Offset))
    return true;

  if (selectAddrFrameIndexOffset(Addr, Base, Offset, 10, 3))
    return true;

  return selectAddrDefault(Addr, Base, Offset);
}

bool Sw64DAGToDAGISel::SelectAddSubImm(SDValue N, MVT VT, SDValue &Imm) {
  if (auto CNode = dyn_cast<ConstantSDNode>(N)) {
    const int64_t ImmVal = CNode->getSExtValue();
    SDLoc DL(N);

    switch (VT.SimpleTy) {
    case MVT::i8:
      // Can always select i8s, no shift, mask the immediate value to
      // deal with sign-extended value from lowering.
      if (!isUInt<8>(ImmVal))
        return false;
      Imm = CurDAG->getTargetConstant(ImmVal & 0xFF, DL, MVT::i64);
      return true;
    case MVT::i16:
      // i16 values get sign-extended to 32-bits during lowering.
      Imm = CurDAG->getTargetConstant(ImmVal, DL, MVT::i64);
      return true;
      break;
    case MVT::i32:
    case MVT::i64:
      return false;
      break;
    default:
      break;
    }
  }

  return false;
}

bool Sw64DAGToDAGISel::SelectComplexImm(SDValue N, SDValue &Imm) {
  if (auto CNode = dyn_cast<ConstantSDNode>(N)) {
    const int64_t ImmVal = CNode->getSExtValue();
    SDLoc DL(N);
    if (!isUInt<5>(ImmVal))
      return false;
    Imm = CurDAG->getTargetConstant(ImmVal & 0x1F, DL, MVT::i64);
    return true;
  }
  return false;
}

/// createSw64ISelDag - This pass converts a legalized DAG into a
/// Sw64-specific DAG, ready for instruction scheduling.
///
FunctionPass *llvm::createSw64ISelDag(Sw64TargetMachine &TM,
                                      CodeGenOpt::Level OptLevel) {
  return new Sw64DAGToDAGISel(TM, OptLevel);
}

bool Sw64DAGToDAGISel::selectSExti32(SDValue N, SDValue &Val) {
  if (N.getOpcode() == ISD::SIGN_EXTEND_INREG &&
      cast<VTSDNode>(N.getOperand(1))->getVT() == MVT::i32) {
    Val = N.getOperand(0);
    return true;
  }
  MVT VT = N.getSimpleValueType();
  if (CurDAG->ComputeNumSignBits(N) > (VT.getSizeInBits() - 32)) {
    Val = N;
    return true;
  }

  return false;
}

bool Sw64DAGToDAGISel::selectZExti32(SDValue N, SDValue &Val) {
  if (N.getOpcode() == ISD::AND) {
    auto *C = dyn_cast<ConstantSDNode>(N.getOperand(1));
    if (C && C->getZExtValue() == UINT64_C(0xFFFFFFFF)) {
      Val = N.getOperand(0);
      return true;
    }
  }
  MVT VT = N.getSimpleValueType();
  APInt Mask = APInt::getHighBitsSet(VT.getSizeInBits(), 32);
  if (CurDAG->MaskedValueIsZero(N, Mask)) {
    Val = N;
    return true;
  }

  return false;
}
