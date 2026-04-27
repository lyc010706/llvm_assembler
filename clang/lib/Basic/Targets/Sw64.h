//===--- Sw64.h - Declare Sw64 target feature support ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares Sw64 TargetInfo objects.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_SW64_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_SW64_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Sw64TargetParser.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY Sw64TargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];
  bool HasCore3 = false;
  bool HasCore4 = false;

  // for futrure update
  // change data length
  void setDataLayout() {
    StringRef Layout;
    Layout =
        "e-m:e-p:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n64-S128-v256:256";
    resetDataLayout(Layout.str());
  }

  bool HasSIMD;

public:
  Sw64TargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple), HasSIMD(false) {
    NoAsmVariants = true;
    MCountName = "";
    setABI("sw_64");
    UseZeroLengthBitfieldAlignment = false;
    IntMaxType = SignedLong;
  }

  bool setABI(const std::string &Name) override {
    set64ABITypes();
    return true;
  }

  void set64ABITypes(void) {
    LongWidth = LongAlign = 64;
    PointerWidth = PointerAlign = 64;
    LongDoubleWidth = LongDoubleAlign = 128;
    LongDoubleFormat = &llvm::APFloat::IEEEquad();
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
    DoubleAlign = LongLongAlign = 64;
    SuitableAlign = 128;
    MaxVectorAlign = 256;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    WCharType = SignedInt;
    WIntType = UnsignedInt;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::Sw64ABIBuiltinVaList;
  }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  std::string_view getClobbers() const override { return ""; }

  bool hasFeature(StringRef Feature) const override;
  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override {
    for (const auto &Feature : Features) {
      if (Feature == "+simd")
        HasSIMD = true;
      if (Feature == "+core3b")
        HasCore3 = true;
      if (Feature == "+core4")
        HasCore4 = true;
    }
    setDataLayout();
    return true;
  };

  bool isValidCPUName(StringRef Name) const override;
  bool setCPU(const std::string &Name) override;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool isValidTuneCPUName(StringRef Name) const override;
  void fillValidTuneCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    default:
      return false;
    case 'I': // Signed 16-bit constant
    case 'J': // Integer 0
    case 'K': // Unsigned 16-bit constant
    case 'L': // Signed 32-bit constant, lower 16-bit zeros (for lui)
    case 'M': // Constants not loadable via lui, addiu, or ori
    case 'N': // Constant -1 to -65535
    case 'O': // A signed 15-bit constant
    case 'P': // A constant between 1 go 65535
      return true;
    }
  }
  // Return the register number that __builtin_eh_return_regno would return with
  // the specified argument.
  //
  // This corresponds with TargetLowering's getExceptionPointerRegister and
  // getExceptionSelectorRegister in the backend.
  int getEHDataRegisterNumber(unsigned RegNo) const override {
    if (RegNo == 0)
      return 16;
    if (RegNo == 1)
      return 17;
    return -1;
  }

  bool allowsLargerPreferedTypeAlignment() const override { return false; }
  bool hasBitIntType() const override { return true; }
};
} // namespace targets
} // namespace clang
#endif
