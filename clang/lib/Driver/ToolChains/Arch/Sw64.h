//===--- Sw64.h - Sw64-specific Tool Helpers --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_SW64_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_SW64_H

#include "clang/Driver/Driver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"
#include "llvm/TargetParser/Triple.h"
#include <vector>

namespace clang {
namespace driver {
namespace tools {
namespace Sw64 {

const char *getSw64TargetCPU(const llvm::opt::ArgList &Args);

void getSw64TargetFeatures(const Driver &D, const llvm::opt::ArgList &Args,
                           std::vector<llvm::StringRef> &Features);

} // end namespace Sw64
} // end namespace tools
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_SW64_H
