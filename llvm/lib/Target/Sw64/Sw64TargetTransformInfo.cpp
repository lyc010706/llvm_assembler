//===-- Sw64TargetTransformInfo.cpp - Sw64-specific TTI -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a TargetTransformInfo analysis pass specific to the
// Sw64 target machine. It uses the target's detailed information to provide
// more precise answers to certain TTI queries, while letting the target
// independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#include "Sw64TargetTransformInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/CodeGen/CostTable.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/Debug.h"
using namespace llvm;

#define DEBUG_TYPE "sw64tti"

//===----------------------------------------------------------------------===//
//
// Sw64 cost model.
//
//===----------------------------------------------------------------------===//

InstructionCost Sw64TTIImpl::getIntImmCost(const APInt &Imm, Type *Ty,
                                           TTI::TargetCostKind CostKind) {
  assert(Ty->isIntegerTy());

  unsigned BitSize = Ty->getPrimitiveSizeInBits();
  // There is no cost model for constants with a bit size of 0. Return TCC_Free
  // here, so that constant hoisting will ignore this constant.
  if (BitSize == 0)
    return TTI::TCC_Free;
  // No cost model for operations on integers larger than 64 bit implemented
  // yet.
  if (BitSize > 64)
    return TTI::TCC_Free;

  if (Imm == 0)
    return TTI::TCC_Free;

  if (Imm.getBitWidth() <= 64) {
    // Constants loaded via lgfi.
    if (isInt<32>(Imm.getSExtValue()))
      return TTI::TCC_Basic;
    // Constants loaded via llilf.
    if (isUInt<32>(Imm.getZExtValue()))
      return TTI::TCC_Basic;
    // Constants loaded via llihf:
    if ((Imm.getZExtValue() & 0xffffffff) == 0)
      return TTI::TCC_Basic;

    return 2 * TTI::TCC_Basic;
  }

  return 4 * TTI::TCC_Basic;
}

InstructionCost Sw64TTIImpl::getIntImmCostInst(unsigned Opcode, unsigned Idx,
                                               const APInt &Imm, Type *Ty,
                                               TTI::TargetCostKind CostKind,
                                               Instruction *Inst) {
  assert(Ty->isIntegerTy());

  unsigned BitSize = Ty->getPrimitiveSizeInBits();
  // There is no cost model for constants with a bit size of 0. Return TCC_Free
  // here, so that constant hoisting will ignore this constant.
  if (BitSize == 0)
    return TTI::TCC_Free;
  // No cost model for operations on integers larger than 64 bit implemented
  // yet.
  if (BitSize > 64)
    return TTI::TCC_Free;

  switch (Opcode) {
  default:
    return TTI::TCC_Free;
  case Instruction::GetElementPtr:
    // Always hoist the base address of a GetElementPtr. This prevents the
    // creation of new constants for every base constant that gets constant
    // folded with the offset.
    if (Idx == 0)
      return 2 * TTI::TCC_Basic;
    return TTI::TCC_Free;
  case Instruction::Store:
    return TTI::TCC_Basic;
  case Instruction::ICmp:
  case Instruction::Add:
  case Instruction::Sub:
    if (Idx == 1 && Imm.getBitWidth() <= 64) {
      // We use algfi/slgfi to add/subtract 32-bit unsigned immediates.
      if (isUInt<32>(Imm.getZExtValue()))
        return TTI::TCC_Free;
      // Or their negation, by swapping addition vs. subtraction.
      if (isUInt<32>(-Imm.getSExtValue()))
        return TTI::TCC_Free;
    }
    break;
  case Instruction::Mul:
  case Instruction::Or:
  case Instruction::Xor:
    if (Idx == 1 && Imm.getBitWidth() <= 64) {
      // Masks supported by oilf/xilf.
      if (isUInt<32>(Imm.getZExtValue()))
        return TTI::TCC_Free;
      // Masks supported by oihf/xihf.
      if ((Imm.getZExtValue() & 0xffffffff) == 0)
        return TTI::TCC_Free;
    }
    break;
  case Instruction::And:
    if (Idx == 1 && Imm.getBitWidth() <= 64) {
      // Any 32-bit AND operation can by implemented via nilf.
      if (BitSize <= 32)
        return TTI::TCC_Free;
      // 64-bit masks supported by nilf.
      if (isUInt<32>(~Imm.getZExtValue()))
        return TTI::TCC_Free;
      // 64-bit masks supported by nilh.
      if ((Imm.getZExtValue() & 0xffffffff) == 0xffffffff)
        return TTI::TCC_Free;
    }
    break;
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::URem:
  case Instruction::SRem:
  case Instruction::Trunc:
  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::IntToPtr:
  case Instruction::PtrToInt:
  case Instruction::BitCast:
  case Instruction::PHI:
  case Instruction::Call:
  case Instruction::Select:
  case Instruction::Ret:
  case Instruction::Load:
    break;
  }

  return Sw64TTIImpl::getIntImmCost(Imm, Ty, CostKind);
}

InstructionCost Sw64TTIImpl::getIntImmCostIntrin(Intrinsic::ID IID,
                                                 unsigned Idx, const APInt &Imm,
                                                 Type *Ty,
                                                 TTI::TargetCostKind CostKind) {
  assert(Ty->isIntegerTy());

  unsigned BitSize = Ty->getPrimitiveSizeInBits();
  // There is no cost model for constants with a bit size of 0. Return TCC_Free
  // here, so that constant hoisting will ignore this constant.
  if (BitSize == 0)
    return TTI::TCC_Free;
  // No cost model for operations on integers larger than 64 bit implemented
  // yet.
  if (BitSize > 64)
    return TTI::TCC_Free;

  switch (IID) {
  default:
    return TTI::TCC_Free;
  case Intrinsic::sadd_with_overflow:
  case Intrinsic::uadd_with_overflow:
  case Intrinsic::ssub_with_overflow:
  case Intrinsic::usub_with_overflow:
    // These get expanded to include a normal addition/subtraction.
    if (Idx == 1 && Imm.getBitWidth() <= 64) {
      if (isUInt<32>(Imm.getZExtValue()))
        return TTI::TCC_Free;
      if (isUInt<32>(-Imm.getSExtValue()))
        return TTI::TCC_Free;
    }
    break;
  case Intrinsic::smul_with_overflow:
  case Intrinsic::umul_with_overflow:
    // These get expanded to include a normal multiplication.
    if (Idx == 1 && Imm.getBitWidth() <= 64) {
      if (isInt<32>(Imm.getSExtValue()))
        return TTI::TCC_Free;
    }
    break;
  case Intrinsic::experimental_stackmap:
    if ((Idx < 2) || (Imm.getBitWidth() <= 64 && isInt<64>(Imm.getSExtValue())))
      return TTI::TCC_Free;
    break;
  case Intrinsic::experimental_patchpoint_void:
  case Intrinsic::experimental_patchpoint_i64:
    if ((Idx < 4) || (Imm.getBitWidth() <= 64 && isInt<64>(Imm.getSExtValue())))
      return TTI::TCC_Free;
    break;
  }
  return Sw64TTIImpl::getIntImmCost(Imm, Ty, CostKind);
}

bool Sw64TTIImpl::isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                                const TargetTransformInfo::LSRCost &C2) {
  // check instruction count (first), and don't care about
  // ImmCost, since offsets are checked explicitly.
  return std::tie(C1.Insns, C1.NumRegs, C1.AddRecCost, C1.NumIVMuls,
                  C1.NumBaseAdds, C1.ScaleCost, C1.SetupCost) <
         std::tie(C2.Insns, C2.NumRegs, C2.AddRecCost, C2.NumIVMuls,
                  C2.NumBaseAdds, C2.ScaleCost, C2.SetupCost);
}

unsigned Sw64TTIImpl::getNumberOfRegisters(bool Vector) {
  if (Vector) {
    return 0;
  }
  return 12;
}

bool Sw64TTIImpl::hasDivRemOp(Type *DataType, bool IsSigned) {
  EVT VT = TLI->getValueType(DL, DataType);
  return (VT.isScalarInteger() && TLI->isTypeLegal(VT));
}

void Sw64TTIImpl::getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                                          TTI::UnrollingPreferences &UP,
                                          OptimizationRemarkEmitter *ORE) {
  // Find out if L contains a call, what the machine instruction count
  // estimate is, and how many stores there are.
  bool HasCall = false;
  InstructionCost NumStores = 0;
  for (auto &BB : L->blocks())
    for (auto &I : *BB) {
      if (isa<CallInst>(&I) || isa<InvokeInst>(&I)) {
        if (const Function *F = cast<CallBase>(I).getCalledFunction()) {
          if (isLoweredToCall(F))
            HasCall = true;
          if (F->getIntrinsicID() == Intrinsic::memcpy ||
              F->getIntrinsicID() == Intrinsic::memset)
            NumStores++;
        } else { // indirect call.
          HasCall = true;
        }
      }
      if (isa<StoreInst>(&I)) {
        Type *MemAccessTy = I.getOperand(0)->getType();
        NumStores += getMemoryOpCost(Instruction::Store, MemAccessTy,
                                     std::nullopt, 0, TTI::TCK_RecipThroughput);
      }
    }

  // The processor will run out of store tags if too many stores
  // are fed into it too quickly. Therefore make sure there are not
  // too many stores in the resulting unrolled loop.
  unsigned const NumStoresVal = *NumStores.getValue();
  unsigned const Max = (NumStoresVal ? (12 / NumStoresVal) : UINT_MAX);

  if (HasCall) {
    // Only allow full unrolling if loop has any calls.
    UP.FullUnrollMaxCount = Max;
    UP.MaxCount = 1;
    return;
  }

  UP.MaxCount = Max;
  if (UP.MaxCount <= 1)
    return;

  // Allow partial and runtime trip count unrolling.
  UP.Partial = UP.Runtime = true;

  UP.PartialThreshold = 75;
  if (L->getLoopDepth() > 1)
    UP.PartialThreshold *= 2;

  UP.DefaultUnrollRuntimeCount = 4;

  // Allow expensive instructions in the pre-header of the loop.
  UP.AllowExpensiveTripCount = true;
  UP.UnrollAndJam = true;

  UP.Force = true;
}

// Return the bit size for the scalar type or vector element
// type. getScalarSizeInBits() returns 0 for a pointer type.
static unsigned getScalarSizeInBits(Type *Ty) {
  unsigned Size = (Ty->isPtrOrPtrVectorTy() ? 64U : Ty->getScalarSizeInBits());
  assert(Size > 0 && "Element must have non-zero size.");
  return Size;
}

// getNumberOfParts() calls getTypeLegalizationCost() which splits the vector
// type until it is legal. This would e.g. return 4 for <6 x i64>, instead of
// 3.
static unsigned getNumVectorRegs(Type *Ty) { return 0; }

unsigned Sw64TTIImpl::getMaxInterleaveFactor(ElementCount VF) {
  return ST->getMaxInterleaveFactor();
}

TypeSize Sw64TTIImpl::getRegisterBitWidth(bool Vector) const {
  return TypeSize::getFixed(64);
}

unsigned Sw64TTIImpl::getCFInstrCost(unsigned Opcode,
                                     TTI::TargetCostKind CostKind,
                                     const Instruction *I) {
  if (CostKind != TTI::TCK_RecipThroughput)
    return Opcode == Instruction::PHI ? 0 : 1;
  assert(CostKind == TTI::TCK_RecipThroughput && "unexpected CostKind");
  // Branches are assumed to be predicted.
  return 0;
}

bool Sw64TTIImpl::isWideningInstruction(Type *DstTy, unsigned Opcode,
                                        ArrayRef<const Value *> Args) {

  // A helper that returns a vector type from the given type. The number of
  // elements in type Ty determine the vector width.
  auto toVectorTy = [&](Type *ArgTy) {
    return FixedVectorType::get(ArgTy->getScalarType(),
                                cast<FixedVectorType>(DstTy)->getNumElements());
  };

  // Exit early if DstTy is not a vector type whose elements are at least
  // 16-bits wide.
  if (!DstTy->isVectorTy() || DstTy->getScalarSizeInBits() < 16)
    return false;

  // Determine if the operation has a widening variant. We consider both the
  // "long" (e.g., usubl) and "wide" (e.g., usubw) versions of the
  // instructions.
  //
  // TODO: Add additional widening operations (e.g., mul, shl, etc.) once we
  //       verify that their extending operands are eliminated during code
  //       generation.
  switch (Opcode) {
  case Instruction::Add: // UADDL(2), SADDL(2), UADDW(2), SADDW(2).
  case Instruction::Sub: // USUBL(2), SSUBL(2), USUBW(2), SSUBW(2).
    break;
  default:
    return false;
  }

  // To be a widening instruction (either the "wide" or "long" versions), the
  // second operand must be a sign- or zero extend having a single user. We
  // only consider extends having a single user because they may otherwise not
  // be eliminated.
  if (Args.size() != 2 ||
      (!isa<SExtInst>(Args[1]) && !isa<ZExtInst>(Args[1])) ||
      !Args[1]->hasOneUse())
    return false;
  auto *Extend = cast<CastInst>(Args[1]);

  // Legalize the destination type and ensure it can be used in a widening
  // operation.
  auto DstTyL = getTypeLegalizationCost(DstTy);
  unsigned DstElTySize = DstTyL.second.getScalarSizeInBits();
  if (!DstTyL.second.isVector() || DstElTySize != DstTy->getScalarSizeInBits())
    return false;

  // Legalize the source type and ensure it can be used in a widening
  // operation.
  auto *SrcTy = toVectorTy(Extend->getSrcTy());
  auto SrcTyL = getTypeLegalizationCost(SrcTy);
  unsigned SrcElTySize = SrcTyL.second.getScalarSizeInBits();
  if (!SrcTyL.second.isVector() || SrcElTySize != SrcTy->getScalarSizeInBits())
    return false;

  // Get the total number of vector elements in the legalized types.
  InstructionCost NumDstEls =
      DstTyL.first * DstTyL.second.getVectorMinNumElements();
  InstructionCost NumSrcEls =
      SrcTyL.first * SrcTyL.second.getVectorMinNumElements();

  // Return true if the legalized types have the same number of vector elements
  // and the destination element type size is twice that of the source type.
  return NumDstEls == NumSrcEls && 2 * SrcElTySize == DstElTySize;
}

InstructionCost Sw64TTIImpl::getArithmeticInstrCost(
    unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
    TTI::OperandValueInfo Op1Info, TTI::OperandValueInfo Op2Info,
    ArrayRef<const Value *> Args, const Instruction *CxtI) {
  // TODO: Handle more cost kinds.
  if (CostKind != TTI::TCK_RecipThroughput)
    return BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info, Op2Info,
                                         Args, CxtI);

  // Legalize the type.
  std::pair<InstructionCost, MVT> LT = getTypeLegalizationCost(Ty);

  // If the instruction is a widening instruction (e.g., uaddl, saddw, etc.),
  // add in the widening overhead specified by the sub-target. Since the
  // extends feeding widening instructions are performed automatically, they
  // aren't present in the generated code and have a zero cost. By adding a
  // widening overhead here, we attach the total cost of the combined operation
  // to the widening instruction.
  InstructionCost Cost = 0;
  if (isWideningInstruction(Ty, Opcode, Args))
    Cost += ST->getWideningBaseCost();

  int ISD = TLI->InstructionOpcodeToISD(Opcode);

  switch (ISD) {
  default:
    return Cost + BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info,
                                                Op2Info);
  case ISD::SDIV:
    if (Op2Info.isConstant() && Op2Info.isUniform() && Op2Info.isPowerOf2()) {
      // On Sw64, scalar signed division by constants power-of-two are
      // normally expanded to the sequence ADD + CMP + SELECT + SRA.
      // The OperandValue properties many not be same as that of previous
      // operation; conservatively assume OP_None.
      Cost +=
          getArithmeticInstrCost(Instruction::Add, Ty, CostKind,
                                 Op1Info.getNoProps(), Op2Info.getNoProps());
      Cost +=
          getArithmeticInstrCost(Instruction::Sub, Ty, CostKind,
                                 Op1Info.getNoProps(), Op2Info.getNoProps());
      Cost +=
          getArithmeticInstrCost(Instruction::Select, Ty, CostKind,
                                 Op1Info.getNoProps(), Op2Info.getNoProps());
      Cost +=
          getArithmeticInstrCost(Instruction::AShr, Ty, CostKind,
                                 Op1Info.getNoProps(), Op2Info.getNoProps());
      return Cost;
    }
    [[fallthrough]];
  case ISD::UDIV:
    if (Op2Info.isConstant() && Op2Info.isUniform()) {
      auto VT = TLI->getValueType(DL, Ty);
      if (TLI->isOperationLegalOrCustom(ISD::MULHU, VT)) {
        // Vector signed division by constant are expanded to the
        // sequence MULHS + ADD/SUB + SRA + SRL + ADD, and unsigned division
        // to MULHS + SUB + SRL + ADD + SRL.
        InstructionCost MulCost =
            getArithmeticInstrCost(Instruction::Mul, Ty, CostKind,
                                   Op1Info.getNoProps(), Op2Info.getNoProps());
        InstructionCost AddCost =
            getArithmeticInstrCost(Instruction::Add, Ty, CostKind,
                                   Op1Info.getNoProps(), Op2Info.getNoProps());
        InstructionCost ShrCost =
            getArithmeticInstrCost(Instruction::AShr, Ty, CostKind,
                                   Op1Info.getNoProps(), Op2Info.getNoProps());
        return MulCost * 2 + AddCost * 2 + ShrCost * 2 + 1;
      }
    }

    Cost +=
        BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info, Op2Info);
    if (Ty->isVectorTy()) {
      // On Sw64, vector divisions are not supported natively and are
      // expanded into scalar divisions of each pair of elements.
      Cost += getArithmeticInstrCost(Instruction::ExtractElement, Ty, CostKind,
                                     Op1Info, Op2Info);
      Cost += getArithmeticInstrCost(Instruction::InsertElement, Ty, CostKind,
                                     Op1Info, Op2Info);
      // TODO: if one of the arguments is scalar, then it's not necessary to
      // double the cost of handling the vector elements.
      Cost += Cost;
    }
    return Cost;

  case ISD::ADD:
  case ISD::MUL:
  case ISD::XOR:
  case ISD::OR:
  case ISD::AND:
    // These nodes are marked as 'custom' for combining purposes only.
    // We know that they are legal. See LowerAdd in ISelLowering.
    return (Cost + 1) * LT.first;

  case ISD::FADD:
    // These nodes are marked as 'custom' just to lower them to SVE.
    // We know said lowering will incur no additional cost.
    if (isa<FixedVectorType>(Ty) && !Ty->getScalarType()->isFP128Ty())
      return (Cost + 2) * LT.first;

    return Cost + BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info,
                                                Op2Info);
  }
}
InstructionCost Sw64TTIImpl::getShuffleCost(TTI::ShuffleKind Kind,
                                            VectorType *Tp, ArrayRef<int> Mask,
                                            TTI::TargetCostKind CostKind,
                                            int Index, VectorType *SubTp,
                                            ArrayRef<const Value *> Args) {
  Kind = improveShuffleKindFromMask(Kind, Mask);
  return BaseT::getShuffleCost(Kind, Tp, Mask, CostKind, Index, SubTp);
}
// Return the log2 difference of the element sizes of the two vector types.
static unsigned getElSizeLog2Diff(Type *Ty0, Type *Ty1) {
  unsigned Bits0 = Ty0->getScalarSizeInBits();
  unsigned Bits1 = Ty1->getScalarSizeInBits();

  if (Bits1 > Bits0)
    return (Log2_32(Bits1) - Log2_32(Bits0));

  return (Log2_32(Bits0) - Log2_32(Bits1));
}

// Return the number of instructions needed to truncate SrcTy to DstTy.
unsigned Sw64TTIImpl::getVectorTruncCost(Type *SrcTy, Type *DstTy) { return 1; }

// Return the cost of converting a vector bitmask produced by a compare
// (SrcTy), to the type of the select or extend instruction (DstTy).
unsigned Sw64TTIImpl::getVectorBitmaskConversionCost(Type *SrcTy, Type *DstTy) {
  assert(SrcTy->isVectorTy() && DstTy->isVectorTy() &&
         "Should only be called with vector types.");

  unsigned PackCost = 0;
  unsigned SrcScalarBits = SrcTy->getScalarSizeInBits();
  unsigned DstScalarBits = DstTy->getScalarSizeInBits();
  unsigned Log2Diff = getElSizeLog2Diff(SrcTy, DstTy);
  if (SrcScalarBits > DstScalarBits)
    // The bitmask will be truncated.
    PackCost = getVectorTruncCost(SrcTy, DstTy);
  else if (SrcScalarBits < DstScalarBits) {
    unsigned DstNumParts = getNumVectorRegs(DstTy);
    // Each vector select needs its part of the bitmask unpacked.
    PackCost = Log2Diff * DstNumParts;
    // Extra cost for moving part of mask before unpacking.
    PackCost += DstNumParts - 1;
  }

  return PackCost;
}

// Return the type of the compared operands. This is needed to compute the
// cost for a Select / ZExt or SExt instruction.
static Type *getCmpOpsType(const Instruction *I, unsigned VF = 1) {
  Type *OpTy = nullptr;
  if (CmpInst *CI = dyn_cast<CmpInst>(I->getOperand(0)))
    OpTy = CI->getOperand(0)->getType();
  else if (Instruction *LogicI = dyn_cast<Instruction>(I->getOperand(0)))
    if (LogicI->getNumOperands() == 2)
      if (CmpInst *CI0 = dyn_cast<CmpInst>(LogicI->getOperand(0)))
        if (isa<CmpInst>(LogicI->getOperand(1)))
          OpTy = CI0->getOperand(0)->getType();

  return nullptr;
}

unsigned Sw64TTIImpl::getBoolVecToIntConversionCost(unsigned Opcode, Type *Dst,
                                                    const Instruction *I) {
  unsigned Cost = 0;
  return Cost;
}

InstructionCost Sw64TTIImpl::getCastInstrCost(unsigned Opcode, Type *Dst,
                                              Type *Src,
                                              TTI::CastContextHint CCH,
                                              TTI::TargetCostKind CostKind,
                                              const Instruction *I) {
  // FIXME: Can the logic below also be used for these cost kinds?
  if (CostKind == TTI::TCK_CodeSize || CostKind == TTI::TCK_SizeAndLatency) {
    auto BaseCost = BaseT::getCastInstrCost(Opcode, Dst, Src, CCH, CostKind, I);
    return BaseCost == 0 ? BaseCost : 1;
  }

  unsigned DstScalarBits = Dst->getScalarSizeInBits();
  unsigned SrcScalarBits = Src->getScalarSizeInBits();

  if (!Src->isVectorTy()) {
    assert(!Dst->isVectorTy());

    if (Opcode == Instruction::SIToFP || Opcode == Instruction::UIToFP) {
      if (SrcScalarBits >= 32 ||
          (I != nullptr && isa<LoadInst>(I->getOperand(0))))
        return 1;
      return SrcScalarBits > 1 ? 2 /*i8/i16 extend*/ : 5 /*branch seq.*/;
    }

    if ((Opcode == Instruction::ZExt || Opcode == Instruction::SExt) &&
        Src->isIntegerTy(1)) {

      // This should be extension of a compare i1 result, which is done with
      // ipm and a varying sequence of instructions.
      unsigned Cost = 0;
      if (Opcode == Instruction::SExt)
        Cost = (DstScalarBits < 64 ? 3 : 4);
      if (Opcode == Instruction::ZExt)
        Cost = 3;
      Type *CmpOpTy = ((I != nullptr) ? getCmpOpsType(I) : nullptr);
      if (CmpOpTy != nullptr && CmpOpTy->isFloatingPointTy())
        // If operands of an fp-type was compared, this costs +1.
        Cost++;
      return Cost;
    }
  }

  return BaseT::getCastInstrCost(Opcode, Dst, Src, CCH, CostKind, I);
}

// Scalar i8 / i16 operations will typically be made after first extending
// the operands to i32.
static unsigned getOperandsExtensionCost(const Instruction *I) {
  unsigned ExtCost = 0;
  for (Value *Op : I->operands())
    // A load of i8 or i16 sign/zero extends to i32.
    if (!isa<LoadInst>(Op) && !isa<ConstantInt>(Op))
      ExtCost++;

  return ExtCost;
}

InstructionCost Sw64TTIImpl::getCmpSelInstrCost(unsigned Opcode, Type *ValTy,
                                                Type *CondTy,
                                                CmpInst::Predicate VecPred,
                                                TTI::TargetCostKind CostKind,
                                                const Instruction *I) {
  if (CostKind != TTI::TCK_RecipThroughput)
    return BaseT::getCmpSelInstrCost(Opcode, ValTy, CondTy, VecPred, CostKind);

  if (!ValTy->isVectorTy()) {
    switch (Opcode) {
    case Instruction::ICmp: {
      // A loaded value compared with 0 with multiple users becomes Load and
      // Test. The load is then not foldable, so return 0 cost for the ICmp.
      unsigned ScalarBits = ValTy->getScalarSizeInBits();
      if (I != nullptr && ScalarBits >= 32)
        if (LoadInst *Ld = dyn_cast<LoadInst>(I->getOperand(0)))
          if (const ConstantInt *C = dyn_cast<ConstantInt>(I->getOperand(1)))
            if (!Ld->hasOneUse() && Ld->getParent() == I->getParent() &&
                C->isZero())
              return 0;

      unsigned Cost = 1;
      if (ValTy->isIntegerTy() && ValTy->getScalarSizeInBits() <= 16)
        Cost += (I != nullptr ? getOperandsExtensionCost(I) : 2);
      return Cost;
    }
    case Instruction::Select:
      if (ValTy->isFloatingPointTy())
        return 4; // No load on condition for FP - costs a conditional jump.
      return 1;   // Load On Condition / Select Register.
    }
  }

  return BaseT::getCmpSelInstrCost(Opcode, ValTy, CondTy, VecPred, CostKind);
}

// Check if a load may be folded as a memory operand in its user.
bool Sw64TTIImpl::isFoldableLoad(const LoadInst *Ld,
                                 const Instruction *&FoldedValue) {
  if (!Ld->hasOneUse())
    return false;
  FoldedValue = Ld;
  const Instruction *UserI = cast<Instruction>(*Ld->user_begin());
  unsigned LoadedBits = getScalarSizeInBits(Ld->getType());
  unsigned TruncBits = 0;
  unsigned SExtBits = 0;
  unsigned ZExtBits = 0;
  if (UserI->hasOneUse()) {
    unsigned UserBits = UserI->getType()->getScalarSizeInBits();
    if (isa<TruncInst>(UserI))
      TruncBits = UserBits;
    else if (isa<SExtInst>(UserI))
      SExtBits = UserBits;
    else if (isa<ZExtInst>(UserI))
      ZExtBits = UserBits;
  }
  if (TruncBits || SExtBits || ZExtBits) {
    FoldedValue = UserI;
    UserI = cast<Instruction>(*UserI->user_begin());
    // Load (single use) -> trunc/extend (single use) -> UserI
  }
  if ((UserI->getOpcode() == Instruction::Sub ||
       UserI->getOpcode() == Instruction::SDiv ||
       UserI->getOpcode() == Instruction::UDiv) &&
      UserI->getOperand(1) != FoldedValue)
    return false; // Not commutative, only RHS foldable.
  // LoadOrTruncBits holds the number of effectively loaded bits, but 0 if an
  // extension was made of the load.
  unsigned LoadOrTruncBits =
      ((SExtBits || ZExtBits) ? 0 : (TruncBits ? TruncBits : LoadedBits));
  switch (UserI->getOpcode()) {
  case Instruction::Add: // SE: 16->32, 16/32->64, z14:16->64. ZE: 32->64
  case Instruction::Sub:
  case Instruction::ICmp:
    if (LoadedBits == 32 && ZExtBits == 64)
      return true;
    LLVM_FALLTHROUGH;
  case Instruction::Mul: // SE: 16->32, 32->64, z14:16->64
    if (UserI->getOpcode() != Instruction::ICmp) {
      if (LoadedBits == 16 && SExtBits == 32)
        return true;
      if (LoadOrTruncBits == 16)
        return true;
    }
    LLVM_FALLTHROUGH;
  case Instruction::SDiv: // SE: 32->64
    if (LoadedBits == 32 && SExtBits == 64)
      return true;
    LLVM_FALLTHROUGH;
  case Instruction::UDiv:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
    // All possible extensions of memory checked above.
    // Comparison between memory and immediate.
    if (UserI->getOpcode() == Instruction::ICmp)
      if (ConstantInt *CI = dyn_cast<ConstantInt>(UserI->getOperand(1)))
        if (CI->getValue().isIntN(16))
          return true;
    return (LoadOrTruncBits == 32 || LoadOrTruncBits == 64);
    break;
  }
  return false;
}

static bool isBswapIntrinsicCall(const Value *V) {
  if (const Instruction *I = dyn_cast<Instruction>(V))
    if (auto *CI = dyn_cast<CallInst>(I))
      if (auto *F = CI->getCalledFunction())
        if (F->getIntrinsicID() == Intrinsic::bswap)
          return true;
  return false;
}

InstructionCost Sw64TTIImpl::getMemoryOpCost(unsigned Opcode, Type *Ty,
                                             MaybeAlign Alignment,
                                             unsigned AddressSpace,
                                             TTI::TargetCostKind CostKind,
                                             TTI::OperandValueInfo OpInfo,
                                             const Instruction *I) {
  assert(!Ty->isVoidTy() && "Invalid type");

  // TODO: Handle other cost kinds.
  if (CostKind != TTI::TCK_RecipThroughput)
    return 1;

  // Type legalization can't handle structs
  if (TLI->getValueType(DL, Ty, true) == MVT::Other)
    return BaseT::getMemoryOpCost(Opcode, Ty, Alignment, AddressSpace,
                                  CostKind);

  auto LT = getTypeLegalizationCost(Ty);

  if (ST->isMisaligned256StoreSlow() && Opcode == Instruction::Store &&
      LT.second.is256BitVector() && (!Alignment || *Alignment < Align(32))) {
    // Unaligned stores are extremely inefficient. We don't split all
    // unaligned 128-bit stores because the negative impact that has shown in
    // practice on inlined block copy code.
    // We make such stores expensive so that we will only vectorize if there
    // are 6 other instructions getting vectorized.
    const int AmortizationCost = 6;

    return LT.first * 2 * AmortizationCost;
  }

  if (Ty->isVectorTy() &&
      cast<VectorType>(Ty)->getElementType()->isIntegerTy(8)) {
    unsigned ProfitableNumElements;
    if (Opcode == Instruction::Store)
      // We use a custom trunc store lowering so v.4b should be profitable.
      ProfitableNumElements = 4;
    else
      // We scalarize the loads because there is not v.4b register and we
      // have to promote the elements to v.2.
      ProfitableNumElements = 8;

    if (cast<FixedVectorType>(Ty)->getNumElements() < ProfitableNumElements) {
      unsigned NumVecElts = cast<FixedVectorType>(Ty)->getNumElements();
      unsigned NumVectorizableInstsToAmortize = NumVecElts * 2;
      // We generate 2 instructions per vector element.
      return NumVectorizableInstsToAmortize * NumVecElts * 2;
    }
  }
  return LT.first;
}

TargetTransformInfo::PopcntSupportKind
Sw64TTIImpl::getPopcntSupport(unsigned TyWidth) {
  assert(isPowerOf2_32(TyWidth) && "Ty width must be power of 2");
  // Sw64 only support 64 Bit Pop County
  if (TyWidth == 32 || TyWidth == 64)
    return TTI::PSK_FastHardware;
  return TTI::PSK_Software;
}
