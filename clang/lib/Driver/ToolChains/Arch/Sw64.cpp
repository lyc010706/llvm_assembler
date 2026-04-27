//===--------- Sw64.cpp - Sw64 Helpers for Tools ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Sw64.h"
#include "ToolChains/CommonArgs.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Sw64TargetParser.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

const char *Sw64::getSw64TargetCPU(const ArgList &Args) {
  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_march_EQ)) {
    StringRef Mcpu = llvm::Sw64::getMcpuFromMArch(A->getValue());
    if (Mcpu != "")
      return Mcpu.data();
    else
      return A->getValue();
  }
  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_mcpu_EQ))
    return A->getValue();
  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_mtune_EQ))
    return A->getValue();
  return "sw6b";
}

void Sw64::getSw64TargetFeatures(const Driver &D, const ArgList &Args,
                                 std::vector<llvm::StringRef> &Features) {
  // -m(no-)simd overrides use of the vector facility.
  AddTargetFeature(Args, Features, options::OPT_msimd, options::OPT_mno_simd,
                   "simd");

  if (const Arg *A = Args.getLastArg(options::OPT_mcpu_EQ)) {
    StringRef Mcpu = A->getValue();
    if (Mcpu.startswith("sw6b") || Mcpu.startswith("sw4d"))
      Features.push_back("+core3b");
    else if (Mcpu.startswith("sw8a"))
      Features.push_back("+core4");
  }

  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ)) {
    StringRef March = A->getValue();
    if (March.startswith("core3b"))
      Features.push_back("+core3b");
    else if (March.startswith("core4"))
      Features.push_back("+core4");
  }

  if (Args.hasArg(options::OPT_ffixed_sw_1))
    Features.push_back("+reserve-r1");
  if (Args.hasArg(options::OPT_ffixed_sw_2))
    Features.push_back("+reserve-r2");
  if (Args.hasArg(options::OPT_ffixed_sw_3))
    Features.push_back("+reserve-r3");
  if (Args.hasArg(options::OPT_ffixed_sw_4))
    Features.push_back("+reserve-r4");
  if (Args.hasArg(options::OPT_ffixed_sw_5))
    Features.push_back("+reserve-r5");
  if (Args.hasArg(options::OPT_ffixed_sw_6))
    Features.push_back("+reserve-r6");
  if (Args.hasArg(options::OPT_ffixed_sw_7))
    Features.push_back("+reserve-r7");
  if (Args.hasArg(options::OPT_ffixed_sw_8))
    Features.push_back("+reserve-r8");
  if (Args.hasArg(options::OPT_ffixed_sw_9))
    Features.push_back("+reserve-r9");
  if (Args.hasArg(options::OPT_ffixed_sw_10))
    Features.push_back("+reserve-r10");
  if (Args.hasArg(options::OPT_ffixed_sw_11))
    Features.push_back("+reserve-r11");
  if (Args.hasArg(options::OPT_ffixed_sw_12))
    Features.push_back("+reserve-r12");
  if (Args.hasArg(options::OPT_ffixed_sw_13))
    Features.push_back("+reserve-r13");
  if (Args.hasArg(options::OPT_ffixed_sw_14))
    Features.push_back("+reserve-r14");
  if (Args.hasArg(options::OPT_ffixed_sw_22))
    Features.push_back("+reserve-r22");
  if (Args.hasArg(options::OPT_ffixed_sw_23))
    Features.push_back("+reserve-r23");
  if (Args.hasArg(options::OPT_ffixed_sw_24))
    Features.push_back("+reserve-r24");
  if (Args.hasArg(options::OPT_ffixed_sw_25))
    Features.push_back("+reserve-r25");
}
