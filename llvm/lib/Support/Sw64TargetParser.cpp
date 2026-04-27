//===-- Sw64TargetParser - Parser for Sw64 features -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a target parser to recognise Sw64 hardware features
// such as FPU/CPU/ARCH and extension names.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Sw64TargetParser.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/TargetParser/Triple.h"
#include <cctype>

namespace llvm {
namespace Sw64 {

struct CPUInfo {
  StringLiteral Name;
  CPUKind Kind;
  unsigned Features;
  StringLiteral DefaultMarch;
  bool is64Bit() const { return (Features & FK_64BIT); }
};

constexpr CPUInfo Sw64CPUInfo[] = {
#define SW64_CPU(ENUM, NAME, FEATURES, DEFAULT_MARCH)                          \
  {NAME, CK_##ENUM, FEATURES, DEFAULT_MARCH},
#include "llvm/Support/Sw64TargetParser.def"
};

bool checkTuneCPUKind(CPUKind Kind, bool IsSw64) {
  if (Kind == CK_INVALID)
    return false;
  return Sw64CPUInfo[static_cast<unsigned>(Kind)].is64Bit() == IsSw64;
}

CPUKind parseARCHKind(StringRef CPU) {
  return llvm::StringSwitch<CPUKind>(CPU)
#define SW64_CPU(ENUM, NAME, FEATURES, DEFAULT_MARCH)                          \
  .Case(DEFAULT_MARCH, CK_##ENUM)
#include "llvm/Support/Sw64TargetParser.def"
      .Default(CK_INVALID);
}

StringRef resolveTuneCPUAlias(StringRef TuneCPU, bool IsSw64) {
  return llvm::StringSwitch<StringRef>(TuneCPU)
#define PROC_ALIAS(NAME, Sw64) .Case(NAME, StringRef(Sw64))
#include "llvm/Support/Sw64TargetParser.def"
      .Default(TuneCPU);
}

CPUKind parseTuneCPUKind(StringRef TuneCPU, bool IsSw64) {
  TuneCPU = resolveTuneCPUAlias(TuneCPU, IsSw64);

  return llvm::StringSwitch<CPUKind>(TuneCPU)
#define SW64_CPU(ENUM, NAME, FEATURES, DEFAULT_MARCH) .Case(NAME, CK_##ENUM)
#include "llvm/Support/Sw64TargetParser.def"
      .Default(CK_INVALID);
}

StringRef getMcpuFromMArch(StringRef CPU) {
  CPUKind Kind = parseARCHKind(CPU);
  return Sw64CPUInfo[static_cast<unsigned>(Kind)].Name;
}

void fillValidCPUArchList(SmallVectorImpl<StringRef> &Values, bool IsSw64) {
  for (const auto &C : Sw64CPUInfo) {
    if (C.Kind != CK_INVALID && IsSw64 == C.is64Bit())
      Values.emplace_back(C.Name);
  }
}

void fillValidTuneCPUArchList(SmallVectorImpl<StringRef> &Values, bool IsSw64) {
  for (const auto &C : Sw64CPUInfo) {
    if (C.Kind != CK_INVALID && IsSw64 == C.is64Bit())
      Values.emplace_back(C.Name);
  }

#define PROC_ALIAS(NAME, Sw64) Values.emplace_back(StringRef(NAME));
#include "llvm/Support/Sw64TargetParser.def"
}

CPUKind parseCPUArch(StringRef CPU) {
  return llvm::StringSwitch<CPUKind>(CPU)
#define SW64_CPU(ENUM, NAME, FEATURES, DEFAULT_MARCH) .Case(NAME, CK_##ENUM)
#include "llvm/Support/Sw64TargetParser.def"
      .Default(CK_INVALID);
}

} // namespace Sw64
} // namespace llvm
