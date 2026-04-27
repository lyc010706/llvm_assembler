//===---- TargetInfo.cpp - Encapsulate target details -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// These classes wrap the information about a call or function
// definition used to handle ABI compliancy.
//
//===----------------------------------------------------------------------===//

#include "TargetInfo.h"
#include "ABIInfoImpl.h"
#include "clang/Basic/DiagnosticFrontend.h"
#include "llvm/ADT/SmallBitVector.h"

using namespace clang;
using namespace clang::CodeGen;

//===----------------------------------------------------------------------===//
// SW64 ABI Implementation.
//===----------------------------------------------------------------------===//

namespace {
class Sw64ABIInfo : public ABIInfo {
  /// Similar to llvm::CCState, but for Clang.
  struct CCState {
    CCState(CGFunctionInfo &FI)
        : IsPreassigned(FI.arg_size()), CC(FI.getCallingConvention()),
          Required(FI.getRequiredArgs()), IsDelegateCall(FI.isDelegateCall()) {}

    llvm::SmallBitVector IsPreassigned;
    unsigned CC = CallingConv::CC_C;
    unsigned FreeRegs = 0;
    unsigned FreeSSERegs = 0;
    RequiredArgs Required;
    bool IsDelegateCall = false;
  };
  unsigned MinABIStackAlignInBytes, StackAlignInBytes;
  void CoerceToIntArgs(uint64_t TySize,
                       SmallVectorImpl<llvm::Type *> &ArgList) const;
  llvm::Type *HandleAggregates(QualType Ty, uint64_t TySize) const;
  llvm::Type *returnAggregateInRegs(QualType RetTy, uint64_t Size) const;
  llvm::Type *getPaddingType(uint64_t Align, uint64_t Offset) const;

public:
  Sw64ABIInfo(CodeGenTypes &CGT)
      : ABIInfo(CGT), MinABIStackAlignInBytes(8), StackAlignInBytes(16) {}

  ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType RetTy, uint64_t &Offset,
                                  CCState &State) const;
  ABIArgInfo getIndirectResult(QualType Ty, bool ByVal, CCState &State) const;
  void computeInfo(CGFunctionInfo &FI) const override;
  Address EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                    QualType Ty) const override;
  ABIArgInfo extendType(QualType Ty) const;
};

class Sw64TargetCodeGenInfo : public TargetCodeGenInfo {
  unsigned SizeOfUnwindException;

public:
  Sw64TargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<Sw64ABIInfo>(CGT)),
        SizeOfUnwindException(32) {}

  int getDwarfEHStackPointer(CodeGen::CodeGenModule &CGM) const override {
    return 30;
  }

  void setTargetAttributes(const Decl *D, llvm::GlobalValue *GV,
                           CodeGen::CodeGenModule &CGM) const override {
    const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D);
    if (!FD)
      return;

    // Other attributes do not have a meaning for declarations.
    if (GV->isDeclaration())
      return;

    // FIXME:Interrupte Attr doesn`t write in SW64.
    // const auto *attr = FD->getAttr<Sw64IntteruptAttr>();
    // if(!attr)
    //   return
    // const char *Kind;
    // ...
    //
  }

  bool initDwarfEHRegSizeTable(CodeGen::CodeGenFunction &CGF,
                               llvm::Value *Address) const override;

  unsigned getSizeOfUnwindException() const override {
    return SizeOfUnwindException;
  }
};
} // namespace

void Sw64ABIInfo::CoerceToIntArgs(
    uint64_t TySize, SmallVectorImpl<llvm::Type *> &ArgList) const {
  llvm::IntegerType *IntTy =
      llvm::IntegerType::get(getVMContext(), MinABIStackAlignInBytes * 8);

  // Add (TySize / MinABIStackAlignInBytes) args of IntTy.
  for (unsigned N = TySize / (MinABIStackAlignInBytes * 8); N; --N)
    ArgList.push_back(IntTy);

  // If necessary, add one more integer type to ArgList.
  unsigned R = TySize % (MinABIStackAlignInBytes * 8);

  if (R)
    ArgList.push_back(llvm::IntegerType::get(getVMContext(), R));
}

// In N32/64, an aligned double precision floating point field is passed in
// a register.
llvm::Type *Sw64ABIInfo::HandleAggregates(QualType Ty, uint64_t TySize) const {
  SmallVector<llvm::Type *, 8> ArgList, IntArgList;

  if (Ty->isComplexType())
    return CGT.ConvertType(Ty);

  const RecordType *RT = Ty->getAs<RecordType>();

  // Unions/vectors are passed in integer registers.
  if (!RT || !RT->isStructureOrClassType()) {
    CoerceToIntArgs(TySize, ArgList);
    return llvm::StructType::get(getVMContext(), ArgList);
  }

  const RecordDecl *RD = RT->getDecl();
  const ASTRecordLayout &Layout = getContext().getASTRecordLayout(RD);
  assert(!(TySize % 8) && "Size of structure must be multiple of 8.");

  uint64_t LastOffset = 0;
  unsigned idx = 0;
  llvm::IntegerType *I64 = llvm::IntegerType::get(getVMContext(), 64);

  // Iterate over fields in the struct/class and check if there are any aligned
  // double fields.
  for (RecordDecl::field_iterator i = RD->field_begin(), e = RD->field_end();
       i != e; ++i, ++idx) {
    const QualType Ty = i->getType();
    const BuiltinType *BT = Ty->getAs<BuiltinType>();

    if (!BT || BT->getKind() != BuiltinType::Double)
      continue;

    uint64_t Offset = Layout.getFieldOffset(idx);
    if (Offset % 64) // Ignore doubles that are not aligned.
      continue;

    // Add ((Offset - LastOffset) / 64) args of type i64.
    for (unsigned j = (Offset - LastOffset) / 64; j > 0; --j)
      ArgList.push_back(I64);

    // Add double type.
    // ArgList.push_back(llvm::Type::getDoubleTy(getVMContext()));
    ArgList.push_back(llvm::Type::getInt64Ty(getVMContext()));
    LastOffset = Offset + 64;
  }

  CoerceToIntArgs(TySize - LastOffset, IntArgList);
  ArgList.append(IntArgList.begin(), IntArgList.end());

  return llvm::StructType::get(getVMContext(), ArgList);
}

llvm::Type *Sw64ABIInfo::getPaddingType(uint64_t OrigOffset,
                                        uint64_t Offset) const {
  if (OrigOffset + MinABIStackAlignInBytes > Offset)
    return nullptr;

  return llvm::IntegerType::get(getVMContext(), (Offset - OrigOffset) * 8);
}

ABIArgInfo Sw64ABIInfo::classifyArgumentType(QualType Ty) const {
  Ty = useFirstFieldIfTransparentUnion(Ty);
  if (isAggregateTypeForABI(Ty)) {
    // Records with non trivial destructors/constructors should not be passed
    // by value.
    if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI()))
      return getNaturalAlignIndirect(Ty, RAA == CGCXXABI::RAA_DirectInMemory);

    return getNaturalAlignIndirect(Ty);
  }

  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = Ty->getAs<EnumType>())
    Ty = EnumTy->getDecl()->getIntegerType();

  if (const BuiltinType *BuiltinTy = Ty->getAs<BuiltinType>()) {
    if (BuiltinTy->getKind() == BuiltinType::LongDouble &&
        getContext().getTypeSize(Ty) == 128)
      return getNaturalAlignIndirect(Ty, false);
  }
  return isPromotableIntegerTypeForABI(Ty) ? ABIArgInfo::getExtend(Ty)
                                           : ABIArgInfo::getDirect();
}
ABIArgInfo Sw64ABIInfo::getIndirectResult(QualType Ty, bool ByVal,
                                          CCState &State) const {
  if (!ByVal) {
    if (State.FreeRegs) {
      --State.FreeRegs; // Non-byval indirects just use one pointer.
      return getNaturalAlignIndirectInReg(Ty);
    }
    return getNaturalAlignIndirect(Ty, false);
  }

  // Compute the byval alignment.
  unsigned TypeAlign = getContext().getTypeAlign(Ty) / 8;
  return ABIArgInfo::getIndirect(CharUnits::fromQuantity(4), /*ByVal=*/true,
                                 /*Realign=*/TypeAlign >
                                     MinABIStackAlignInBytes);
}

ABIArgInfo Sw64ABIInfo::classifyArgumentType(QualType Ty, uint64_t &Offset,
                                             CCState &State) const {
  Ty = useFirstFieldIfTransparentUnion(Ty);
  // Check with the C++ ABI first.
  const RecordType *RT = Ty->getAs<RecordType>();
  if (RT) {
    CGCXXABI::RecordArgABI RAA = getRecordArgABI(RT, getCXXABI());
    if (RAA == CGCXXABI::RAA_Indirect) {
      return getIndirectResult(Ty, /*ByVal=*/false, State);
    } else if (RAA == CGCXXABI::RAA_DirectInMemory) {
      return getNaturalAlignIndirect(Ty, /*ByVal=*/true);
    }
  }

  if (Ty->isVectorType()) {
    uint64_t Size = getContext().getTypeSize(Ty);
    if (Size > 256)
      return getNaturalAlignIndirect(Ty, /*ByVal=*/false);
    else if (Size < 128) {
      llvm::Type *CoerceTy = llvm::IntegerType::get(getVMContext(), Size);
      return ABIArgInfo::getDirect(CoerceTy);
    }
  }

  if (Ty->isAnyComplexType()) {
    if (getContext().getTypeSize(Ty) <= 128) {
      return ABIArgInfo::getDirect();
    } else {
      return getNaturalAlignIndirect(Ty, false);
    }
  }

  uint64_t OrigOffset = Offset;
  uint64_t TySize = getContext().getTypeSize(Ty);
  uint64_t Align = getContext().getTypeAlign(Ty) / 8;

  Align = std::min(std::max(Align, (uint64_t)MinABIStackAlignInBytes),
                   (uint64_t)StackAlignInBytes);
  unsigned CurrOffset = llvm::alignTo(Offset, Align);
  Offset = CurrOffset + llvm::alignTo(TySize, Align * 8) / 8;

  if (isAggregateTypeForABI(Ty)) {
    // Ignore empty aggregates.
    if (TySize == 0)
      return ABIArgInfo::getIgnore();

    if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI())) {
      Offset = OrigOffset + MinABIStackAlignInBytes;
      return getNaturalAlignIndirect(Ty, RAA == CGCXXABI::RAA_DirectInMemory);
    }
    llvm::LLVMContext &LLVMContext = getVMContext();
    unsigned SizeInRegs = (getContext().getTypeSize(Ty) + 63) / 64;
    if (SizeInRegs <= State.FreeRegs) {
      llvm::IntegerType *Int64 = llvm::Type::getInt64Ty(LLVMContext);
      SmallVector<llvm::Type *, 6> Elements(SizeInRegs, Int64);
      llvm::Type *Result = llvm::StructType::get(LLVMContext, Elements);
      return ABIArgInfo::getDirectInReg(Result);
    } else {
      // If we have reached here, aggregates are passed directly by coercing to
      // another structure type. Padding is inserted if the offset of the
      // aggregate is unaligned.
      ABIArgInfo ArgInfo =
          ABIArgInfo::getDirect(HandleAggregates(Ty, TySize), 0,
                                getPaddingType(OrigOffset, CurrOffset));
      ArgInfo.setInReg(true);
      return ArgInfo;
    }
  }

  if (const BuiltinType *BuiltinTy = Ty->getAs<BuiltinType>()) {
    if (BuiltinTy->getKind() == BuiltinType::LongDouble &&
        getContext().getTypeSize(Ty) == 128)
      return getNaturalAlignIndirect(Ty, false);
  }

  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = Ty->getAs<EnumType>())
    Ty = EnumTy->getDecl()->getIntegerType();

  // All integral types are promoted to the GPR width.
  if (Ty->isIntegralOrEnumerationType())
    return extendType(Ty);

  return ABIArgInfo::getDirect(nullptr, 0,
                               getPaddingType(OrigOffset, CurrOffset));
}

llvm::Type *Sw64ABIInfo::returnAggregateInRegs(QualType RetTy,
                                               uint64_t Size) const {
  const RecordType *RT = RetTy->getAs<RecordType>();
  SmallVector<llvm::Type *, 8> RTList;

  if (RT && RT->isStructureOrClassType()) {
    const RecordDecl *RD = RT->getDecl();
    const ASTRecordLayout &Layout = getContext().getASTRecordLayout(RD);
    unsigned FieldCnt = Layout.getFieldCount();

    // N32/64 returns struct/classes in floating point registers if the
    // following conditions are met:
    // 1. The size of the struct/class is no larger than 128-bit.
    // 2. The struct/class has one or two fields all of which are floating
    //    point types.
    // 3. The offset of the first field is zero (this follows what gcc does).
    //
    // Any other composite results are returned in integer registers.
    //
    if (FieldCnt && (FieldCnt <= 2) && !Layout.getFieldOffset(0)) {
      RecordDecl::field_iterator b = RD->field_begin(), e = RD->field_end();
      for (; b != e; ++b) {
        const BuiltinType *BT = b->getType()->getAs<BuiltinType>();

        if (!BT || !BT->isFloatingPoint())
          break;

        RTList.push_back(CGT.ConvertType(b->getType()));
      }
      if (b == e)
        return llvm::StructType::get(getVMContext(), RTList,
                                     RD->hasAttr<PackedAttr>());

      RTList.clear();
    }
  }

  CoerceToIntArgs(Size, RTList);
  return llvm::StructType::get(getVMContext(), RTList);
}

ABIArgInfo Sw64ABIInfo::classifyReturnType(QualType RetTy) const {
  uint64_t Size = getContext().getTypeSize(RetTy);

  if (RetTy->isVoidType())
    return ABIArgInfo::getIgnore();

  // However, N32/N64 ignores zero sized return values.
  if (Size == 0)
    return ABIArgInfo::getIgnore();

  // Large vector types should be returned via memory.
  if (RetTy->isVectorType() && Size == 256)
    return ABIArgInfo::getDirect();

  if (const auto *BT = RetTy->getAs<BuiltinType>())
    if (BT->getKind() == BuiltinType::LongDouble || Size >= 128)
      return getNaturalAlignIndirect(RetTy);

  if (isAggregateTypeForABI(RetTy) || RetTy->isVectorType()) {
    if ((RetTy->hasFloatingRepresentation() && Size <= 128) ||
        (!RetTy->hasFloatingRepresentation() && Size <= 64)) {
      if (RetTy->isComplexType())
        return ABIArgInfo::getDirect();

      if (RetTy->isComplexIntegerType() ||
          (RetTy->isVectorType() && !RetTy->hasFloatingRepresentation())) {
        ABIArgInfo ArgInfo =
            ABIArgInfo::getDirect(returnAggregateInRegs(RetTy, Size));
        ArgInfo.setInReg(true);
        return ArgInfo;
      }
    }

    return getNaturalAlignIndirect(RetTy);
  }

  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = RetTy->getAs<EnumType>())
    RetTy = EnumTy->getDecl()->getIntegerType();

  if (isPromotableIntegerTypeForABI(RetTy))
    return ABIArgInfo::getExtend(RetTy);

  if ((RetTy->isUnsignedIntegerOrEnumerationType() ||
       RetTy->isSignedIntegerOrEnumerationType()) &&
      Size == 32)
    return ABIArgInfo::getSignExtend(RetTy);

  return ABIArgInfo::getDirect();
}

void Sw64ABIInfo::computeInfo(CGFunctionInfo &FI) const {

  CCState State(FI);
  if (FI.getHasRegParm()) {
    State.FreeRegs = FI.getRegParm();
  } else {
    State.FreeRegs = 6;
  }

  ABIArgInfo &RetInfo = FI.getReturnInfo();
  if (!getCXXABI().classifyReturnType(FI))
    RetInfo = classifyReturnType(FI.getReturnType());

  // Check if a pointer to an aggregate is passed as a hidden argument.
  uint64_t Offset = RetInfo.isIndirect() ? MinABIStackAlignInBytes : 0;

  for (auto &I : FI.arguments())
    I.info = classifyArgumentType(I.type, Offset, State);
}

Address Sw64ABIInfo::EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                               QualType OrigTy) const {

  QualType Ty = OrigTy;
  auto TyAlign = getContext().getTypeInfoInChars(Ty).Align;
  if (!Ty->isStructureOrClassType() && (TyAlign.getQuantity() <= 8)) {
    ABIArgInfo AI = classifyArgumentType(Ty);
    return EmitVAArgInstr(CGF, VAListAddr, OrigTy, AI);
  }

  bool DidPromote = false;
  auto TyInfo = getContext().getTypeInfoInChars(Ty);

  // The alignment of things in the argument area is never larger than
  // StackAlignInBytes.
  TyInfo.Align =
      std::min(TyInfo.Align, CharUnits::fromQuantity(StackAlignInBytes));

  bool IsIndirect = false;
  bool AllowHigherAlign = true;

  CharUnits DirectSize, DirectAlign;
  if (IsIndirect) {
    DirectAlign = CGF.getPointerAlign();
  } else {
    DirectAlign = TyInfo.Align;
  }
  // Cast the address we've calculated to the right type.
  llvm::Type *DirectTy = CGF.ConvertTypeForMem(Ty), *ElementTy = DirectTy;
  if (IsIndirect)
    DirectTy = DirectTy->getPointerTo(0);

  CharUnits SlotSize = CharUnits::fromQuantity(MinABIStackAlignInBytes);

  // Handle vaList specified on Sw64, struct{char *ptr, int offset}
  Address vaList_ptr_p = CGF.Builder.CreateStructGEP(VAListAddr, 0);
  llvm::Value *vaList_ptr = CGF.Builder.CreateLoad(vaList_ptr_p);
  Address vaList_offset_p = CGF.Builder.CreateStructGEP(VAListAddr, 1);
  llvm::Value *vaList_offset = CGF.Builder.CreateLoad(vaList_offset_p);

  uint64_t TySize = TyInfo.Width.getQuantity();
  llvm::Value *Offset = llvm::ConstantInt::get(CGF.Int32Ty, TySize);
  CGF.Builder.CreateStore(CGF.Builder.CreateAdd(vaList_offset, Offset),
                          vaList_offset_p);

  llvm::Value *GPAddr =
      CGF.Builder.CreateGEP(CGF.Int8Ty, vaList_ptr, vaList_offset);

  // If the CC aligns values higher than the slot size, do so if needed.
  Address Addr = Address::invalid();
  if (AllowHigherAlign && DirectAlign > SlotSize) {
    Addr = Address(emitRoundPointerUpToAlignment(CGF, GPAddr, DirectAlign),
                   CGF.Int8Ty, DirectAlign);
  } else {
    Addr = Address(GPAddr, CGF.Int8Ty, SlotSize);
  }

  Addr = Addr.withElementType(DirectTy);

  if (IsIndirect) {
    Addr = Address(CGF.Builder.CreateLoad(Addr), ElementTy, TyInfo.Align);
  }

  // If there was a promotion, "unpromote" into a temporary.
  // TODO: can we just use a pointer into a subset of the original slot?
  if (DidPromote) {
    Address Temp = CGF.CreateMemTemp(OrigTy, "vaarg.promotion-temp");
    llvm::Value *Promoted = CGF.Builder.CreateLoad(Addr);

    // Truncate down to the right width.
    llvm::Type *IntTy =
        (OrigTy->isIntegerType() ? Temp.getElementType() : CGF.IntPtrTy);
    llvm::Value *V = CGF.Builder.CreateTrunc(Promoted, IntTy);
    if (OrigTy->isPointerType())
      V = CGF.Builder.CreateIntToPtr(V, Temp.getElementType());

    CGF.Builder.CreateStore(V, Temp);
    Addr = Temp;
  }

  return Addr;
}

ABIArgInfo Sw64ABIInfo::extendType(QualType Ty) const {
  int TySize = getContext().getTypeSize(Ty);

  // SW64 ABI requires unsigned 32 bit integers to be sign extended.
  if (Ty->isUnsignedIntegerOrEnumerationType() && TySize == 32)
    return ABIArgInfo::getSignExtend(Ty);

  return ABIArgInfo::getExtend(Ty);
}

bool Sw64TargetCodeGenInfo::initDwarfEHRegSizeTable(
    CodeGen::CodeGenFunction &CGF, llvm::Value *Address) const {
  // SW have much different from Mips. This should be rewrite.

  // This information comes from gcc's implementation, which seems to
  // as canonical as it gets.

  // Everything on Sw64 is 4 bytes.  Double-precision FP registers
  // are aliased to pairs of single-precision FP registers.
  llvm::Value *Four8 = llvm::ConstantInt::get(CGF.Int8Ty, 4);

  // 0-31 are the general purpose registers, $0 - $31.
  // 32-63 are the floating-point registers, $f0 - $f31.
  // 64 and 65 are the multiply/divide registers, $hi and $lo.
  // 66 is the (notional, I think) register for signal-handler return.
  AssignToArrayRange(CGF.Builder, Address, Four8, 0, 65);

  // 67-74 are the floating-point status registers, $fcc0 - $fcc7.
  // They are one bit wide and ignored here.

  // 80-111 are the coprocessor 0 registers, $c0r0 - $c0r31.
  // (coprocessor 1 is the FP unit)
  // 112-143 are the coprocessor 2 registers, $c2r0 - $c2r31.
  // 144-175 are the coprocessor 3 registers, $c3r0 - $c3r31.
  // 176-181 are the DSP accumulator registers.
  AssignToArrayRange(CGF.Builder, Address, Four8, 80, 181);
  return false;
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createSw64TargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<Sw64TargetCodeGenInfo>(CGM.getTypes());
}
