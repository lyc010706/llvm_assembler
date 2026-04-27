//===-- Sw64TargetTransformInfo.h - Sw64 specific TTI ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file a TargetTransformInfo::Concept conforming object specific to the
/// Sw64 target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_SW64TARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_SW64_SW64TARGETTRANSFORMINFO_H

#include "Sw64.h"
#include "Sw64TargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class Sw64TTIImpl : public BasicTTIImplBase<Sw64TTIImpl> {
  typedef BasicTTIImplBase<Sw64TTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const Sw64Subtarget *ST;
  const Sw64TargetLowering *TLI;

  const Sw64Subtarget *getST() const { return ST; }
  const Sw64TargetLowering *getTLI() const { return TLI; }

  unsigned const LIBCALL_COST = 30;

  bool isWideningInstruction(Type *Ty, unsigned Opcode,
                             ArrayRef<const Value *> Args);

public:
  explicit Sw64TTIImpl(const Sw64TargetMachine *TM, const Function &F)
      : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  unsigned getNumberOfRegisters(unsigned ClassID) const {
    bool Vector = (ClassID == 1);
    if (Vector) {
      if (ST->hasSIMD())
        return 32;
      return 0;
    }
    return 32;
  }

  unsigned getMaxInterleaveFactor(ElementCount VF);
  bool enableInterleavedAccessVectorization() { return true; }
  TypeSize getRegisterBitWidth(bool Vector) const;

  unsigned getCFInstrCost(unsigned Opcode, TTI::TargetCostKind CostKind,
                          const Instruction *I);

  InstructionCost getMemoryOpCost(
      unsigned Opcode, Type *Src, MaybeAlign Alignment, unsigned AddressSpace,
      TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo OpInfo = {TTI::OK_AnyValue, TTI::OP_None},
      const Instruction *I = nullptr);
  void getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                               TTI::UnrollingPreferences &UP,
                               OptimizationRemarkEmitter *ORE);

  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      ArrayRef<const Value *> Args = ArrayRef<const Value *>(),
      const Instruction *CxtI = nullptr);

  InstructionCost getIntImmCost(const APInt &Imm, Type *Ty,
                                TTI::TargetCostKind CostKind);

  InstructionCost getIntImmCostInst(unsigned Opcode, unsigned Idx,
                                    const APInt &Imm, Type *Ty,
                                    TTI::TargetCostKind CostKind,
                                    Instruction *Inst = nullptr);
  InstructionCost getIntImmCostIntrin(Intrinsic::ID IID, unsigned Idx,
                                      const APInt &Imm, Type *Ty,
                                      TTI::TargetCostKind CostKind);

  bool isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                     const TargetTransformInfo::LSRCost &C2);

  unsigned getNumberOfRegisters(bool Vector);

  unsigned getCacheLineSize() const override { return 128; }
  unsigned getPrefetchDistance() const override { return 524; }
  unsigned getMinPrefetchStride(unsigned NumMemAccesses,
                                unsigned NumStridedMemAccesses,
                                unsigned NumPrefetches,
                                bool HasCall) const override {
    return 1;
  }

  bool hasDivRemOp(Type *DataType, bool IsSigned);
  bool prefersVectorizedAddressing() { return false; }
  bool LSRWithInstrQueries() { return true; }
  bool supportsEfficientVectorElementLoadStore() { return true; }

  InstructionCost getShuffleCost(TTI::ShuffleKind Kind, VectorType *Tp,
                                 ArrayRef<int> Mask,
                                 TTI::TargetCostKind CostKind, int Index,
                                 VectorType *SubTp,
                                 ArrayRef<const Value *> Args = std::nullopt);
  unsigned getVectorTruncCost(Type *SrcTy, Type *DstTy);
  unsigned getVectorBitmaskConversionCost(Type *SrcTy, Type *DstTy);
  unsigned getBoolVecToIntConversionCost(unsigned Opcode, Type *Dst,
                                         const Instruction *I);
  InstructionCost getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                                   TTI::CastContextHint CCH,
                                   TTI::TargetCostKind CostKind,
                                   const Instruction *I = nullptr);
  InstructionCost getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy,
                                     CmpInst::Predicate VecPred,
                                     TTI::TargetCostKind CostKind,
                                     const Instruction *I = nullptr);
  bool isFoldableLoad(const LoadInst *Ld, const Instruction *&FoldedValue);

  TargetTransformInfo::PopcntSupportKind getPopcntSupport(unsigned TyWidth);
  /// @}
};

} // end namespace llvm

#endif
