//===--- Sw64.cpp - Implement Sw64 target feature support ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements Sw64 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Sw64.h"
#include "Targets.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Sw64TargetParser.h"

using namespace clang;
using namespace clang::targets;

ArrayRef<const char *> Sw64TargetInfo::getGCCRegNames() const {
  static const char *const GCCRegNames[] = {
      "$0",   "$1",   "$2",   "$3",   "$4",   "$5",   "$6",   "$7",
      "$8",   "$9",   "$10",  "$11",  "$12",  "$13",  "$14",  "$15",
      "$16",  "$17",  "$18",  "$19",  "$20",  "$21",  "$22",  "$23",
      "$24",  "$25",  "$26",  "$27",  "$28",  "$29",  "$30",  "$31",
      "$f0",  "$f1",  "$f2",  "$f3",  "$f4",  "$f5",  "$f6",  "$f7",
      "$f8",  "$f9",  "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
      "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
      "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31"};
  return llvm::makeArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> Sw64TargetInfo::getGCCRegAliases() const {
  static const TargetInfo::GCCRegAlias GCCRegAliases[] = {
      {{"v0"}, "$0"},   {{"t0"}, "$1"},   {{"t1"}, "$2"},  {{"t2"}, "$3"},
      {{"t3"}, "$4"},   {{"t4"}, "$5"},   {{"t5"}, "$6"},  {{"t6"}, "$7"},
      {{"t7"}, "$8"},   {{"s0"}, "$9"},   {{"s1"}, "$10"}, {{"s2"}, "$11"},
      {{"s3"}, "$12"},  {{"s4"}, "$13"},  {{"s5"}, "$14"}, {{"fp"}, "$15"},
      {{"a0"}, "$16"},  {{"a1"}, "$17"},  {{"a2"}, "$18"}, {{"a3"}, "$19"},
      {{"a4"}, "$20"},  {{"a5"}, "$21"},  {{"t8"}, "$22"}, {{"t9"}, "$23"},
      {{"t10"}, "$24"}, {{"t11"}, "$25"}, {{"ra"}, "$26"}, {{"t12"}, "$27"},
      {{"at"}, "$28"},  {{"gp"}, "$29"},  {{"sp"}, "$30"}, {{"zero"}, "$31"}};
  return llvm::makeArrayRef(GCCRegAliases);
}

const Builtin::Info Sw64TargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, HEADER, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
#include "clang/Basic/BuiltinsSw64.def"
};

void Sw64TargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  llvm::Sw64::fillValidCPUArchList(Values, true);
}

bool Sw64TargetInfo::isValidTuneCPUName(StringRef Name) const {
  return llvm::Sw64::checkTuneCPUKind(llvm::Sw64::parseTuneCPUKind(Name, true),
                                      /*Is64Bit=*/true);
}

void Sw64TargetInfo::fillValidTuneCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  llvm::Sw64::fillValidTuneCPUArchList(Values, true);
}

bool Sw64TargetInfo::isValidCPUName(StringRef Name) const {
  return llvm::Sw64::parseCPUArch(Name) != llvm::Sw64::CK_INVALID;
}

bool Sw64TargetInfo::setCPU(const std::string &Name) {
  return isValidCPUName(Name);
}

void Sw64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "sw_64", Opts);

  Builder.defineMacro("__REGISTER_PREFIX__", "");
  Builder.defineMacro("__LONG_DOUBLE_128__");

  Builder.defineMacro("__ELF__");
  Builder.defineMacro("__sw_64__");
  Builder.defineMacro("__sw_64_sw6a__");
  Builder.defineMacro("__sw_64");
  // Consistent with GCC
  Builder.defineMacro("__gnu_linux__");

  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8");

  DefineStd(Builder, "unix", Opts);
  DefineStd(Builder, "linux", Opts);

  if (HasCore4)
    Builder.defineMacro("__sw_64_sw8a__");

  if (Opts.CPlusPlus)
    Builder.defineMacro("_GNU_SOURCE");
}

/// Return true if has this feature, need to sync with handleTargetFeatures.
bool Sw64TargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
      .Case("sw_64", true)
      .Case("core3b", HasCore3)
      .Case("core4", HasCore4)
      .Case("simd", HasSIMD)
      .Default(false);
}

ArrayRef<Builtin::Info> Sw64TargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, clang::Sw64::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}
