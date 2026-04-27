//===-- Sw64TargetParser - Parser for Sw64 features -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a target parser to recognise SW64 hardware features
// such as FPU/CPU/ARCH and extension names.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_SW64TARGETPARSER_H
#define LLVM_SUPPORT_SW64TARGETPARSER_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <vector>

// FIXME:This should be made into class design,to avoid dupplication.
namespace llvm {
class StringRef;

namespace Sw64 {

enum CPUKind : unsigned { CK_INVALID = 0, CK_SW6B, CK_SW4D, CK_SW8A };

enum FeatureKind : unsigned {
  FK_INVALID = 0,
  FK_NONE = 1,
  FK_STDEXTM = 1 << 2,
  FK_STDEXTA = 1 << 3,
  FK_STDEXTF = 1 << 4,
  FK_STDEXTD = 1 << 5,
  FK_STDEXTC = 1 << 6,
  FK_64BIT = 1 << 7,
};

bool checkCPUKind(CPUKind Kind, bool IsSw64);
bool checkTuneCPUKind(CPUKind Kind, bool IsSw64);
CPUKind parseARCHKind(StringRef CPU);
CPUKind parseTuneCPUKind(StringRef CPU, bool IsSw64);
StringRef getMcpuFromMArch(StringRef CPU);
void fillValidCPUArchList(SmallVectorImpl<StringRef> &Values, bool IsSw64);
void fillValidTuneCPUArchList(SmallVectorImpl<StringRef> &Values, bool IsSw64);
StringRef resolveTuneCPUAlias(StringRef TuneCPU, bool IsSw64);
CPUKind parseCPUArch(StringRef CPU);

} // namespace Sw64
} // namespace llvm

#endif
