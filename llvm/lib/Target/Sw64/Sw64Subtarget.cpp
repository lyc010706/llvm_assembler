//===-- Sw64Subtarget.cpp - Sw64 Subtarget Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Sw64 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "Sw64Subtarget.h"
#include "Sw64.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "sw_64-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "Sw64GenSubtargetInfo.inc"

static cl::opt<bool> Sw64IntArith("sw-int-divmod", cl::init(true),
                                  cl::desc("Enable sw64 core4 integer"
                                           "arithmetic instructions"));

static cl::opt<bool> Sw64IntShift("sw-shift-word", cl::init(false),
                                  cl::desc("Enable sw64 core4 integer"
                                           "shift instructions"));

static cl::opt<bool> Sw64ByteInst("sw-rev", cl::init(false),
                                  cl::desc("Enable sw64 core4 byte"
                                           "manipulation instructions"));

static cl::opt<bool> Sw64FloatArith("sw-recip", cl::init(true),
                                    cl::desc("Enable sw64 core4 float"
                                             "arithmetic instructions"));

static cl::opt<bool> Sw64FloatRound("sw-fprnd", cl::init(false),
                                    cl::desc("Enable sw64 core4 float"
                                             "round instructions"));

static cl::opt<bool> Sw64FloatCmov("sw-cmov", cl::init(true),
                                   cl::desc("Enable sw64 core4 float"
                                            "cmov instructions"));

static cl::opt<bool> Sw64PostInc("sw-auto-inc-dec", cl::init(false),
                                 cl::desc("Enable sw64 core4 post-inc"
                                          "load and store instructions"));

static cl::opt<bool>
    Sw64CasInst("sw-use-cas", cl::init(true),
                cl::desc("Enable sw64 core4 cas instructions"));

static cl::opt<bool>
    Sw64CrcInst("sw-crc32", cl::init(false),
                cl::desc("Enable sw64 core4 crc32 instructions"));

static cl::opt<bool> Sw64SCbtInst("sw-sbt-cbt", cl::init(false),
                                  cl::desc("Enable sw64 core4 integer"
                                           "sbt and cbt instructions"));

static cl::opt<bool>
    Sw64WmembInst("sw-wmemb", cl::init(false),
                  cl::desc("Enable sw64 core4 wmemb instructions"));

static cl::opt<bool> Sw64InstMullShiftAddSub("sw64-inst-mull-shiftaddsub",
                                             cl::init(true),
                                             cl::desc("Inst mull optmize to"
                                                      "shift with add or sub"));

static cl::opt<bool> Sw64InstExt("sw64-ext-opt", cl::init(false),
                                 cl::desc("Optimize zext and sext"));

static cl::opt<bool> Sw64InstMemset("sw64-inst-memset", cl::init(true),
                                    cl::desc("Delete part of call memset"));

cl::opt<bool> HasSIMD("msimd", cl::desc("Support the SIMD"), cl::init(false));

void Sw64Subtarget::anchor() {}

Sw64Subtarget &Sw64Subtarget::initializeSubtargetDependencies(const Triple &TT,
                                                              StringRef CPU,
                                                              StringRef FS) {
  std::string CPUName = std::string(CPU);
  std::string TuneCPUName = std::string(CPU);
  ParseSubtargetFeatures(CPUName, /*TuneCPU*/ TuneCPUName, FS);
  return *this;
}

Sw64Subtarget::Sw64Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : Sw64GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS), InstrInfo(),
      Sw64OptMul(Sw64InstMullShiftAddSub), Sw64OptMemset(Sw64InstMemset),
      Sw64OptExt(Sw64InstExt),
      ReserveRegister(Sw64::GPRCRegClass.getNumRegs() +
                      Sw64::F4RCRegClass.getNumRegs() + 1),
      Sw64EnableIntAri(Sw64IntArith), Sw64EnableIntShift(Sw64IntShift),
      Sw64EnableByteInst(Sw64ByteInst), Sw64EnableFloatAri(Sw64FloatArith),
      Sw64EnableFloatRound(Sw64FloatRound), Sw64EnableFloatCmov(Sw64FloatCmov),
      Sw64EnablePostInc(Sw64PostInc), Sw64EnableCasInst(Sw64CasInst),
      Sw64EnableCrcInst(Sw64CrcInst), Sw64EnableSCbtInst(Sw64SCbtInst),
      Sw64EnableWmembInst(Sw64WmembInst),
      FrameLowering(initializeSubtargetDependencies(TT, CPU, FS)),
      TLInfo(TM, *this), TSInfo(), curgpdist(0) {}

void Sw64Subtarget::overrideSchedPolicy(MachineSchedPolicy &Policy,
                                        unsigned NumRegionInstrs) const {
  Policy.OnlyBottomUp = false;
  // Spilling is generally expensive on Sw64, so always enable
  // register-pressure tracking.
  Policy.ShouldTrackPressure = true;
}
