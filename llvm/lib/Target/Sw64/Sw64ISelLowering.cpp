//===-- Sw64ISelLowering.cpp - Sw64 DAG Lowering Implementation ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Sw64TargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "Sw64ISelLowering.h"
#include "MCTargetDesc/Sw64BaseInfo.h"
#include "Sw64.h"
#include "Sw64MachineFunctionInfo.h"
#include "Sw64Subtarget.h"
#include "Sw64TargetMachine.h"
#include "Sw64TargetObjectFile.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/FastISel.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsSw64.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

using namespace llvm;

#define DEBUG_TYPE "sw_64-lower"

/// AddLiveIn - This helper function adds the specified physical register to the
/// MachineFunction as a live in value.  It also creates a corresponding virtual
/// register for it.
static unsigned AddLiveIn(MachineFunction &MF, unsigned PReg,
                          const TargetRegisterClass *RC) {
  assert(RC->contains(PReg) && "Not the correct regclass!");
  Register VReg = MF.getRegInfo().createVirtualRegister(RC);
  MF.getRegInfo().addLiveIn(PReg, VReg);
  return VReg;
}

const char *Sw64TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((Sw64ISD::NodeType)Opcode) {
  default:
    return 0;
  case Sw64ISD::CVTQT_:
    return "Sw64::CVTQT_";
  case Sw64ISD::CVTQS_:
    return "Sw64::CVTQS_";
  case Sw64ISD::CVTTQ_:
    return "Sw64::CVTTQ_";
  case Sw64ISD::CVTST_:
    return "Sw64::CVTST_";
  case Sw64ISD::CVTTS_:
    return "Sw64::CVTTS_";
  case Sw64ISD::JmpLink:
    return "Sw64::JmpLink";
  case Sw64ISD::Ret:
    return "Sw64::Ret";
  case Sw64ISD::TPRelLo:
    return "Sw64::TPRelLo";
  case Sw64ISD::TPRelHi:
    return "Sw64::TPRelHi";
  case Sw64ISD::SysCall:
    return "Sw64::SysCall";
  case Sw64ISD::LDAWC:
    return "Sw64::Sw64_LDAWC";

  case Sw64ISD::TLSGD:
    return "Sw64::TLSGD";
  case Sw64ISD::DTPRelLo:
    return "Sw64::DTPRelLo";
  case Sw64ISD::DTPRelHi:
    return "Sw64::DTPRelHi";
  case Sw64ISD::TLSLDM:
    return "Sw64::TLSLDM";
  case Sw64ISD::RelGottp:
    return "Sw64::RelGottp";
  case Sw64ISD::GPRelHi:
    return "Sw64::GPRelHi";
  case Sw64ISD::GPRelLo:
    return "Sw64::GPRelLo";
  case Sw64ISD::RelLit:
    return "Sw64::RelLit";
  case Sw64ISD::GlobalRetAddr:
    return "Sw64::GlobalRetAddr";
  case Sw64ISD::CALL:
    return "Sw64::CALL";
  case Sw64ISD::DivCall:
    return "Sw64::DivCall";
  case Sw64ISD::RET_FLAG:
    return "Sw64::RET_FLAG";
  case Sw64ISD::COND_BRANCH_I:
    return "Sw64::COND_BRANCH_I";
  case Sw64ISD::COND_BRANCH_F:
    return "Sw64::COND_BRANCH_F";
  case Sw64ISD::MEMBARRIER:
    return "Sw64ISD::MEMBARRIER";

  case Sw64ISD::GPRel:
    return "Sw64ISD::GPRel";
  case Sw64ISD::TPRel:
    return "Sw64ISD::TPRel";
  case Sw64ISD::DTPRel:
    return "Sw64ISD::DTPRel";
  case Sw64ISD::LDIH:
    return "Sw64ISD::LDIH";
  case Sw64ISD::LDI:
    return "Sw64ISD::LDI";

  case Sw64ISD::Z_S_FILLCS:
    return "Sw64ISD::Z_S_FILLCS";
  case Sw64ISD::Z_S_FILLDE:
    return "Sw64ISD::Z_S_FILLDE";
  case Sw64ISD::Z_FILLDE:
    return "Sw64ISD::Z_FILLDE";
  case Sw64ISD::Z_FILLDE_E:
    return "Sw64ISD::Z_FILLDE_E";
  case Sw64ISD::Z_FILLCS:
    return "Sw64ISD::Z_FILLCS";
  case Sw64ISD::Z_FILLCS_E:
    return "Sw64ISD::Z_FILLCS_E";
  case Sw64ISD::Z_E_FILLCS:
    return "Sw64ISD::Z_E_FILLCS";
  case Sw64ISD::Z_E_FILLDE:
    return "Sw64ISD::Z_E_FILLDE";
  case Sw64ISD::Z_FLUSHD:
    return "Sw64ISD::Z_FLUSHD";

  case Sw64ISD::FRECS:
    return "Sw64ISD::FRECS";
  case Sw64ISD::FRECD:
    return "Sw64ISD::FRECD";
  case Sw64ISD::SBT:
    return "Sw64ISD::SBT";
  case Sw64ISD::REVBH:
    return "Sw64ISD::REVBH";
  case Sw64ISD::REVBW:
    return "Sw64ISD::REVBW";

  case Sw64ISD::ROLW:
    return "Sw64ISD::ROLW";
  case Sw64ISD::CRC32B:
    return "Sw64ISD::CRC32B";
  case Sw64ISD::CRC32H:
    return "Sw64ISD::CRC32H";
  case Sw64ISD::CRC32W:
    return "Sw64ISD::CRC32W";
  case Sw64ISD::CRC32L:
    return "Sw64ISD::CRC32L";
  case Sw64ISD::CRC32CB:
    return "Sw64ISD::CRC32CB";
  case Sw64ISD::CRC32CH:
    return "Sw64ISD::CRC32CH";
  case Sw64ISD::CRC32CW:
    return "Sw64ISD::CRC32CW";
  case Sw64ISD::CRC32CL:
    return "Sw64ISD::CRC32CL";

  case Sw64ISD::VLDWE:
    return "Sw64ISD::VLDWE";
  case Sw64ISD::VLDSE:
    return "Sw64ISD::VLDSE";
  case Sw64ISD::VLDDE:
    return "Sw64ISD::VLDDE";

  case Sw64ISD::VNOR:
    return "Sw64ISD::VNOR";
  case Sw64ISD::VEQV:
    return "Sw64ISD::VEQV";
  case Sw64ISD::VORNOT:
    return "Sw64ISD::VORNOT";
  case Sw64ISD::VSHF:
    return "Sw64ISD::VSHF";
  case Sw64ISD::SHF:
    return "Sw64ISD::SHF";
  case Sw64ISD::ILVEV:
    return "Sw64ISD::ILVEV";
  case Sw64ISD::ILVOD:
    return "Sw64ISD::ILVOD";
  case Sw64ISD::ILVL:
    return "Sw64ISD::ILVL";
  case Sw64ISD::ILVR:
    return "Sw64ISD::ILVR";
  case Sw64ISD::PCKEV:
    return "Sw64ISD::PCKEV";
  case Sw64ISD::PCKOD:
    return "Sw64ISD::PCKOD";
  case Sw64ISD::VMAX:
    return "Sw64ISD::VMAX";
  case Sw64ISD::VMIN:
    return "Sw64ISD::VMIN";
  case Sw64ISD::VUMAX:
    return "Sw64ISD::VUMAX";
  case Sw64ISD::VUMIN:
    return "Sw64ISD::VUMIN";
  case Sw64ISD::VFREC:
    return "Sw64ISD::VFREC";
  case Sw64ISD::VFCMPEQ:
    return "Sw64ISD::VFCMPEQ";
  case Sw64ISD::VFCMPLE:
    return "Sw64ISD::VFCMPLE";
  case Sw64ISD::VFCMPLT:
    return "Sw64ISD::VFCMPLT";
  case Sw64ISD::VFCMPUN:
    return "Sw64ISD::VFCMPUN";
  case Sw64ISD::VFCVTSD:
    return "Sw64ISD::VFCVTSD";
  case Sw64ISD::VFCVTDS:
    return "Sw64ISD::VFCVTDS";
  case Sw64ISD::VFCVTLS:
    return "Sw64ISD::VFCVTLS";
  case Sw64ISD::VFCVTLD:
    return "Sw64ISD::VFCVTLD";
  case Sw64ISD::VFCVTSH:
    return "Sw64ISD::VFCVTSH";
  case Sw64ISD::VFCVTHS:
    return "Sw64ISD::VFCVTHS";
  case Sw64ISD::VFCVTDL:
    return "Sw64ISD::VFCVTDL";
  case Sw64ISD::VFCVTDLG:
    return "Sw64ISD::VFCVTDLG";
  case Sw64ISD::VFCVTDLP:
    return "Sw64ISD::VFCVTDLP";
  case Sw64ISD::VFCVTDLZ:
    return "Sw64ISD::VFCVTDLZ";
  case Sw64ISD::VFCVTDLN:
    return "Sw64ISD::VFCVTDLN";
  case Sw64ISD::VFRIS:
    return "Sw64ISD::VFRIS";
  case Sw64ISD::VFRISG:
    return "Sw64ISD::VFRISG";
  case Sw64ISD::VFRISP:
    return "Sw64ISD::VFRISP";
  case Sw64ISD::VFRISZ:
    return "Sw64ISD::VFRISZ";
  case Sw64ISD::VFRISN:
    return "Sw64ISD::VFRISN";
  case Sw64ISD::VFRID:
    return "Sw64ISD::VFRID";
  case Sw64ISD::VFRIDG:
    return "Sw64ISD::VFRIDG";
  case Sw64ISD::VFRIDP:
    return "Sw64ISD::VFRIDP";
  case Sw64ISD::VFRIDZ:
    return "Sw64ISD::VFRIDZ";
  case Sw64ISD::VFRIDN:
    return "Sw64ISD::VFRIDN";
  case Sw64ISD::VMAXF:
    return "Sw64ISD::VMAXF";
  case Sw64ISD::VMINF:
    return "Sw64ISD::VMINF";
  case Sw64ISD::VCPYB:
    return "Sw64ISD::VCPYB";
  case Sw64ISD::VCPYH:
    return "Sw64ISD::VCPYH";

  case Sw64ISD::VCON_W:
    return "Sw64ISD::VCON_W";
  case Sw64ISD::VCON_S:
    return "Sw64ISD::VCON_S";
  case Sw64ISD::VCON_D:
    return "Sw64ISD::VCON_D";

  case Sw64ISD::INSVE:
    return "Sw64ISD::INSVE";
  case Sw64ISD::VCOPYF:
    return "Sw64ISD::VCOPYF";
  case Sw64ISD::V8SLL:
    return "Sw64ISD::V8SLL";
  case Sw64ISD::V8SLLi:
    return "Sw64ISD::V8SLLi";
  case Sw64ISD::V8SRL:
    return "Sw64ISD::V8SRL";
  case Sw64ISD::V8SRLi:
    return "Sw64ISD::V8SRLi";
  case Sw64ISD::VROTR:
    return "Sw64ISD::VROTR";
  case Sw64ISD::VROTRi:
    return "Sw64ISD::VROTRi";
  case Sw64ISD::V8SRA:
    return "Sw64ISD::V8SRA";
  case Sw64ISD::V8SRAi:
    return "Sw64ISD::V8SRAi";
  case Sw64ISD::VROLB:
    return "Sw64ISD::VROLB";
  case Sw64ISD::VROLBi:
    return "Sw64ISD::VROLBi";
  case Sw64ISD::VROLH:
    return "Sw64ISD::VROLH";
  case Sw64ISD::VROLHi:
    return "Sw64ISD::VROLHi";
  case Sw64ISD::VROLL:
    return "Sw64ISD::VROLL";
  case Sw64ISD::VROLLi:
    return "Sw64ISD::VROLLi";
  case Sw64ISD::VCTPOP:
    return "Sw64ISD::VCTPOP";
  case Sw64ISD::VCTLZ:
    return "Sw64ISD::VCTLZ";

  case Sw64ISD::VLOG:
    return "Sw64ISD::VLOG";
  case Sw64ISD::VSETGE:
    return "Sw64ISD::VSETGE";

  case Sw64ISD::VSELEQW:
    return "Sw64ISD::VSELEQW";
  case Sw64ISD::VSELLTW:
    return "Sw64ISD::VSELLTW";
  case Sw64ISD::VSELLEW:
    return "Sw64ISD::VSELLEW";
  case Sw64ISD::VSELLBCW:
    return "Sw64ISD::VSELLBCW";

  case Sw64ISD::VFCMOVEQ:
    return "Sw64ISD::VFCMOVEQ";
  case Sw64ISD::VFCMOVLE:
    return "Sw64ISD::VFCMOVLE";
  case Sw64ISD::VFCMOVLT:
    return "Sw64ISD::VFCMOVLT";

  case Sw64ISD::VECT_VUCADDW:
    return "Sw64ISD::VECT_VUCADDW";
  case Sw64ISD::VECT_VUCADDH:
    return "Sw64ISD::VECT_VUCADDH";
  case Sw64ISD::VECT_VUCADDB:
    return "Sw64ISD::VECT_VUCADDB";
  case Sw64ISD::VECT_VUCSUBW:
    return "Sw64ISD::VECT_VUCSUBW";
  case Sw64ISD::VECT_VUCSUBH:
    return "Sw64ISD::VECT_VUCSUBH";
  case Sw64ISD::VECT_VUCSUBB:
    return "Sw64ISD::VECT_VUCSUBB";

  case Sw64ISD::VECREDUCE_FADD:
    return "Sw64ISD::VECREDUCE_FADD";
  case Sw64ISD::VSHL_BY_SCALAR:
    return "Sw64ISD::VSHL_BY_SCALAR";
  case Sw64ISD::VSRL_BY_SCALAR:
    return "Sw64ISD::VSRL_BY_SCALAR";
  case Sw64ISD::VSRA_BY_SCALAR:
    return "Sw64ISD::VSRA_BY_SCALAR";
  case Sw64ISD::VEXTRACT_SEXT_ELT:
    return "Sw64ISD::VEXTRACT_SEXT_ELT";
  case Sw64ISD::VBROADCAST:
    return "Sw64ISD::VBROADCAST";
  case Sw64ISD::VBROADCAST_LD:
    return "Sw64ISD::VBROADCAST_LD";
  case Sw64ISD::VTRUNCST:
    return "Sw64ISD::VTRUNCST";
  }

  return nullptr;
}

Sw64TargetLowering::Sw64TargetLowering(const TargetMachine &TM,
                                       const Sw64Subtarget &Subtarget)
    : TargetLowering(TM), TM(TM), Subtarget(Subtarget) {
  if (Subtarget.hasSIMD()) {
    // Expand all truncating stores and extending loads.
    for (MVT VT0 : MVT::vector_valuetypes()) {
      for (MVT VT1 : MVT::vector_valuetypes()) {
        setTruncStoreAction(VT0, VT1, Expand);
        setLoadExtAction(ISD::SEXTLOAD, VT0, VT1, Expand);
        setLoadExtAction(ISD::ZEXTLOAD, VT0, VT1, Expand);
        setLoadExtAction(ISD::EXTLOAD, VT0, VT1, Expand);
      }
    }
  }

  // Set up the TargetLowering object.
  // I am having problems with shr n i8 1
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrNegativeOneBooleanContent);

  addRegisterClass(MVT::i64, &Sw64::GPRCRegClass);
  addRegisterClass(MVT::f64, &Sw64::F8RCRegClass);
  addRegisterClass(MVT::f32, &Sw64::F4RCRegClass);
  // We want to custom lower some of our intrinsics.
  setOperationAction(ISD::INTRINSIC_WO_CHAIN, MVT::Other, Custom);

  // Loads
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);
  }

  setLoadExtAction(ISD::SEXTLOAD, MVT::i64, MVT::i8, Expand);  // ldbu
  setLoadExtAction(ISD::SEXTLOAD, MVT::i64, MVT::i16, Expand); // ldhu
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i64, MVT::i32, Expand); // ldwu

  if (Subtarget.hasCore4() && Subtarget.enablePostInc()) {
    for (MVT VT : {MVT::i8, MVT::i16, MVT::i32, MVT::i64, MVT::f32, MVT::f64}) {
      setIndexedLoadAction(ISD::POST_INC, VT, Legal);
      setIndexedStoreAction(ISD::POST_INC, VT, Legal);
    }
  }

  setTruncStoreAction(MVT::f32, MVT::f16, Expand);
  setTruncStoreAction(MVT::f64, MVT::f16, Expand);
  setTruncStoreAction(MVT::f64, MVT::f32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, MVT::f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f32, MVT::f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, MVT::f32, Expand);
  setOperationAction(ISD::FP16_TO_FP, MVT::f32, Expand);
  setOperationAction(ISD::FP_TO_FP16, MVT::f32, Expand);
  setOperationAction(ISD::FP16_TO_FP, MVT::f64, Expand);
  setOperationAction(ISD::FP_TO_FP16, MVT::f64, Expand);

  for (MVT VT : MVT::fp_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::f32, Expand);
  }
  setTruncStoreAction(MVT::f64, MVT::f32, Expand);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BR_CC, MVT::i32, Expand);
  setOperationAction(ISD::BR_CC, MVT::i64, Expand);
  setOperationAction(ISD::BR_CC, MVT::f32, Expand);
  setOperationAction(ISD::BR_CC, MVT::f64, Expand);

  // Sw64 wants to turn select_cc of INT/FP into sel/fsel when possible.
  setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i64, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::f32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::f64, Expand);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);

  setOperationAction(ISD::FREM, MVT::f32, Expand);
  setOperationAction(ISD::FREM, MVT::f64, Expand);

  if (Subtarget.hasCore4() && Subtarget.enableFloatCmov()) {
    setOperationAction(ISD::FP_TO_SINT, MVT::i32, Custom);
  } else {
    setOperationAction(ISD::UINT_TO_FP, MVT::i64, Expand);
    setOperationAction(ISD::FP_TO_UINT, MVT::i64, Expand);
    setOperationAction(ISD::FP_TO_SINT, MVT::i32, Custom);
    setOperationAction(ISD::FP_TO_SINT, MVT::i64, Custom);
    setOperationAction(ISD::SINT_TO_FP, MVT::i64, Custom);
  }

  setOperationAction(ISD::FP_TO_SINT_SAT, MVT::i32, Custom);
  setOperationAction(ISD::FP_TO_UINT_SAT, MVT::i32, Custom);

  setOperationAction(ISD::CTPOP, MVT::i64, Expand);
  setOperationAction(ISD::CTTZ, MVT::i64, Expand);
  setOperationAction(ISD::CTLZ, MVT::i64, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i64, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i64, Expand);

  setOperationAction(ISD::UDIV, MVT::i128, Custom);
  setOperationAction(ISD::SDIV, MVT::i128, Custom);
  setOperationAction(ISD::UREM, MVT::i128, Custom);
  setOperationAction(ISD::SREM, MVT::i128, Custom);

  if (!Subtarget.hasCore4() || !Subtarget.enableIntAri()) {
    setOperationAction(ISD::SREM, MVT::i64, Custom);
    setOperationAction(ISD::UREM, MVT::i64, Custom);
    setOperationAction(ISD::SDIV, MVT::i64, Custom);
    setOperationAction(ISD::UDIV, MVT::i64, Custom);
  }

  if (Subtarget.hasCore4() && Subtarget.enableByteInst()) {
    setOperationAction(ISD::BSWAP, MVT::i64, Legal);
    setOperationAction(ISD::BSWAP, MVT::i32, Legal);
    setOperationAction(ISD::BSWAP, MVT::i16, Legal);
  } else {
    setOperationAction(ISD::BSWAP, MVT::i64, Expand);
  }

  if (Subtarget.hasCore4() && Subtarget.enableFloatRound()) {
    for (MVT Ty : {MVT::f32, MVT::f64}) {
      setOperationAction(ISD::FFLOOR, Ty, Legal);
      setOperationAction(ISD::FNEARBYINT, Ty, Legal);
      setOperationAction(ISD::FCEIL, Ty, Legal);
      setOperationAction(ISD::FTRUNC, Ty, Legal);
      setOperationAction(ISD::FROUND, Ty, Legal);
    }
  }

  setOperationAction(ISD::ADDC, MVT::i64, Expand);
  setOperationAction(ISD::ADDE, MVT::i64, Expand);
  setOperationAction(ISD::SUBC, MVT::i64, Expand);
  setOperationAction(ISD::SUBE, MVT::i64, Expand);

  setOperationAction(ISD::UMUL_LOHI, MVT::i64, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i64, Expand);

  setOperationAction(ISD::SRL_PARTS, MVT::i64, Custom);
  setOperationAction(ISD::SRA_PARTS, MVT::i64, Custom);
  setOperationAction(ISD::SHL_PARTS, MVT::i64, Custom);

  setOperationAction(ISD::TRAP, MVT::Other, Legal);

  // We don't support sin/cos/sqrt/pow
  setOperationAction(ISD::FSIN, MVT::f64, Expand);
  setOperationAction(ISD::FCOS, MVT::f64, Expand);
  setOperationAction(ISD::FSIN, MVT::f32, Expand);
  setOperationAction(ISD::FCOS, MVT::f32, Expand);

  setOperationAction(ISD::FSQRT, MVT::f64, Legal);
  setOperationAction(ISD::FSQRT, MVT::f32, Legal);
  setOperationAction(ISD::STRICT_FSQRT, MVT::f64, Legal);
  setOperationAction(ISD::STRICT_FSQRT, MVT::f32, Legal);

  setOperationAction(ISD::FPOW, MVT::f32, Expand);
  setOperationAction(ISD::FPOW, MVT::f64, Expand);

  // We have fused multiply-addition for f32 and f64 but not f128.
  setOperationAction(ISD::FMA, MVT::f64, Legal);
  setOperationAction(ISD::FMA, MVT::f32, Legal);
  setOperationAction(ISD::FMA, MVT::f128, Expand);

  setOperationAction(ISD::SETCC, MVT::f32, Promote);

  setOperationAction(ISD::BITCAST, MVT::f32, Promote);
  // Not implemented yet.
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64, Expand);
  // We want to legalize GlobalAddress and ConstantPool and
  // ExternalSymbols nodes into the appropriate instructions to
  // materialize the address.
  setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);
  setOperationAction(ISD::ConstantPool, MVT::i64, Custom);
  setOperationAction(ISD::ExternalSymbol, MVT::i64, Custom);
  setOperationAction(ISD::GlobalTLSAddress, MVT::i64, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i64, Custom);
  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);
  setOperationAction(ISD::VACOPY, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::i32, Custom);

  setOperationAction(ISD::JumpTable, MVT::i64, Custom);
  setOperationAction(ISD::JumpTable, MVT::i32, Custom);

  setOperationAction(ISD::PREFETCH, MVT::Other, Custom);

  setOperationAction(ISD::ATOMIC_LOAD, MVT::i8, Custom);
  setOperationAction(ISD::ATOMIC_STORE, MVT::i8, Custom);

  setOperationAction(ISD::ATOMIC_LOAD, MVT::i16, Custom);
  setOperationAction(ISD::ATOMIC_STORE, MVT::i16, Custom);

  setOperationAction(ISD::ATOMIC_LOAD, MVT::i32, Custom);
  setOperationAction(ISD::ATOMIC_STORE, MVT::i32, Custom);

  setOperationAction(ISD::ATOMIC_LOAD, MVT::i64, Custom);
  setOperationAction(ISD::ATOMIC_STORE, MVT::i64, Custom);

  setOperationAction(ISD::FSIN, MVT::f64, Expand);
  setOperationAction(ISD::FSINCOS, MVT::f64, Expand);
  setOperationAction(ISD::FSIN, MVT::f32, Expand);
  setOperationAction(ISD::FSINCOS, MVT::f32, Expand);

  setOperationAction(ISD::FADD, MVT::f128, Custom);
  setOperationAction(ISD::FADD, MVT::i128, Custom);
  setStackPointerRegisterToSaveRestore(Sw64::R30);

  if (Subtarget.hasSIMD() || Subtarget.hasCore4()) {
    // We want to custom lower some of our intrinsics.
    setOperationAction(ISD::INTRINSIC_WO_CHAIN, MVT::Other, Custom);
    setOperationAction(ISD::INTRINSIC_W_CHAIN, MVT::Other,
                       Custom); // for builtin_sw64_load
    setOperationAction(ISD::INTRINSIC_VOID, MVT::Other, Custom);
  }

  if (Subtarget.hasSIMD()) {
    addSIMDIntType(MVT::v32i8, &Sw64::V256LRegClass);
    addSIMDIntType(MVT::v16i16, &Sw64::V256LRegClass);
    addSIMDIntType(MVT::v8i32, &Sw64::V256LRegClass);
    addSIMDIntType(MVT::v4i64, &Sw64::V256LRegClass);
    addSIMDFloatType(MVT::v4f32, &Sw64::V256LRegClass);
    addSIMDFloatType(MVT::v4f64, &Sw64::V256LRegClass);

    setTargetDAGCombine(ISD::AND);
    setTargetDAGCombine(ISD::OR);
    setTargetDAGCombine(ISD::SRA);
    setTargetDAGCombine(ISD::VSELECT);
    setTargetDAGCombine(ISD::XOR);

    setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v8i32, Legal);
    setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v4i64, Legal);
    setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v4f32, Legal);
    setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v4f64, Legal);

    setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v16i16, Expand);
    setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v32i8, Expand);

    setOperationAction(ISD::SETCC, MVT::v8i32, Legal);
    setOperationAction(ISD::SETCC, MVT::v4i64, Expand);
    setOperationAction(ISD::SETCC, MVT::v4f32, Legal);
    setOperationAction(ISD::SETCC, MVT::v4f64, Expand);

    if (Subtarget.hasCore4())
      for (auto VT : {MVT::v32i8, MVT::v16i16, MVT::v8i32, MVT::v4i64}) {
        addRegisterClass(VT, &Sw64::V256LRegClass);
        setOperationAction(ISD::SRL, VT, Custom);
        setOperationAction(ISD::SHL, VT, Custom);
        setOperationAction(ISD::SRA, VT, Custom);
        setOperationAction(ISD::BUILD_VECTOR, VT, Expand);
      }
    else {
      addRegisterClass(MVT::v8i32, &Sw64::V256LRegClass);
      setOperationAction(ISD::SRL, MVT::v8i32, Custom);
      setOperationAction(ISD::SHL, MVT::v8i32, Custom);
      setOperationAction(ISD::SRA, MVT::v8i32, Custom);
      setOperationAction(ISD::BUILD_VECTOR, MVT::v8i32, Custom);
    }
  }

  setOperationAction(ISD::FNEG, MVT::v4f32, Legal);
  setOperationAction(ISD::FNEG, MVT::v4f64, Legal);

  setOperationAction(ISD::FCOPYSIGN, MVT::v4f32, Legal);
  setOperationAction(ISD::FCOPYSIGN, MVT::v4f64, Legal);

  if (Subtarget.hasCore4() && Subtarget.enableIntShift()) {
    setOperationAction(ISD::ROTR, MVT::i64, Expand);
    setOperationAction(ISD::ROTL, MVT::i32, Custom);
  } else {
    setOperationAction(ISD::ROTL, MVT::i64, Expand);
    setOperationAction(ISD::ROTR, MVT::i64, Expand);
  }
  if (Subtarget.hasCore4() && Subtarget.enableFloatAri()) {
    setOperationAction(ISD::FDIV, MVT::f32, Legal);
    setOperationAction(ISD::FDIV, MVT::f64, Legal);
  }

  // return R
  setLibcallName(RTLIB::OEQ_F128, "_OtsEqlX");
  setLibcallName(RTLIB::UNE_F128, "_OtsNeqX");
  setLibcallName(RTLIB::UO_F128, "_OtsNeqX");
  setLibcallName(RTLIB::OLE_F128, "_OtsLeqX");
  setLibcallName(RTLIB::OLT_F128, "_OtsLssX");
  setLibcallName(RTLIB::OGE_F128, "_OtsGeqX");
  setLibcallName(RTLIB::OGT_F128, "_OtsGtrX");
  // return R16+R17
  setLibcallName(RTLIB::FPEXT_F64_F128, "_OtsConvertFloatTX");
  setLibcallName(RTLIB::FPEXT_F32_F128, "_OtsConvertFloatTX");
  setLibcallName(RTLIB::UINTTOFP_I64_F128, "_OtsCvtQUX");
  setLibcallName(RTLIB::UINTTOFP_I32_F128, "_OtsCvtQUX");
  setLibcallName(RTLIB::SINTTOFP_I32_F128, "_OtsCvtQX");
  setLibcallName(RTLIB::SINTTOFP_I64_F128, "_OtsCvtQX");
  // add round return R
  setLibcallName(RTLIB::FPTOSINT_F128_I64, "_OtsCvtXQ");
  setLibcallName(RTLIB::FPTOUINT_F128_I64, "_OtsCvtXQ");
  setLibcallName(RTLIB::FPROUND_F128_F64, "_OtsConvertFloatXT");
  setLibcallName(RTLIB::FPROUND_F128_F32, "_OtsConvertFloatXT");
  // add round return R16+R17
  setLibcallName(RTLIB::ADD_F128, "_OtsAddX");
  setLibcallName(RTLIB::SUB_F128, "_OtsSubX");
  setLibcallName(RTLIB::MUL_F128, "_OtsMulX");
  setLibcallName(RTLIB::DIV_F128, "_OtsDivX");
  setOperationAction(ISD::CTPOP, MVT::i32, Promote);
  setOperationAction(ISD::CTPOP, MVT::i64, Legal);

  setMinStackArgumentAlignment(Align(32));
  setMinFunctionAlignment(Align(8));
  setTargetDAGCombine(ISD::MUL);

  computeRegisterProperties(Subtarget.getRegisterInfo());
  MaxStoresPerMemsetOptSize = 16;
  MaxStoresPerMemset = 16;
  MaxStoresPerMemcpy = 4;
  MaxStoresPerMemcpyOptSize = 4;
}

bool Sw64TargetLowering::generateFMAsInMachineCombiner(
    EVT VT, CodeGenOpt::Level OptLevel) const {
  return (OptLevel >= CodeGenOpt::Aggressive) && !VT.isScalableVector();
}

EVT Sw64TargetLowering::getSetCCResultType(const DataLayout &, LLVMContext &,
                                           EVT VT) const {
  // Refer to other.
  if (!VT.isVector())
    return MVT::i64;

  return VT.changeVectorElementTypeToInteger();
}

#include "Sw64GenCallingConv.inc"

static SDValue getTargetNode(GlobalAddressSDNode *N, SDLoc DL, EVT Ty,
                             SelectionDAG &DAG, unsigned Flags) {

  return DAG.getTargetGlobalAddress(N->getGlobal(), DL, Ty, 0, Flags);
}

static SDValue getTargetNode(BlockAddressSDNode *N, SDLoc DL, EVT Ty,
                             SelectionDAG &DAG, unsigned Flags) {

  return DAG.getTargetBlockAddress(N->getBlockAddress(), Ty, N->getOffset(),
                                   Flags);
}

static SDValue getTargetNode(JumpTableSDNode *N, SDLoc DL, EVT Ty,
                             SelectionDAG &DAG, unsigned Flag) {

  return DAG.getTargetJumpTable(N->getIndex(), Ty, Flag);
}

static SDValue getTargetNode(ConstantPoolSDNode *N, SDLoc DL, EVT Ty,
                             SelectionDAG &DAG, unsigned Flags) {

  return DAG.getTargetConstantPool(N->getConstVal(), Ty, N->getAlign(),
                                   N->getOffset(), Flags);
}

// This function returns true if CallSym is a long double emulation routine.
static bool isF128SoftLibCall_void(const char *CallSym) {
  const char *const LibCalls[] = {
      "_OtsAddX", "_OtsConvertFloatTX", "_OtsCvtQUX", "_OtsCvtQX",
      "_OtsDivX", "_OtsMulX",           "_OtsSubX"};

  // Check that LibCalls is sorted betically.
  auto Comp = [](const char *S1, const char *S2) { return strcmp(S1, S2) < 0; };
  assert(std::is_sorted(std::begin(LibCalls), std::end(LibCalls), Comp));

  return std::binary_search(std::begin(LibCalls), std::end(LibCalls), CallSym,
                            Comp);
}

// This function returns true if CallSym is a long double emulation routine.
static bool isF128SoftLibCall_round(const char *CallSym) {
  const char *const LibCalls[] = {
      "_OtsAddX",  "_OtsConvertFloatTX", "_OtsConvertFloatXT",
      "_OtsCvtXQ", "_OtsDivX",           "_OtsMulX",
      "_OtsSubX"};

  // Check that LibCalls is sorted betically.
  auto Comp = [](const char *S1, const char *S2) { return strcmp(S1, S2) < 0; };
  assert(std::is_sorted(std::begin(LibCalls), std::end(LibCalls), Comp));

  return std::binary_search(std::begin(LibCalls), std::end(LibCalls), CallSym,
                            Comp);
}

// Enable SIMD support for the given integer type and Register class.
void Sw64TargetLowering::addSIMDIntType(MVT::SimpleValueType Ty,
                                        const TargetRegisterClass *RC) {
  addRegisterClass(Ty, RC);

  // Expand all builtin opcodes.
  for (unsigned Opc = 0; Opc < ISD::BUILTIN_OP_END; ++Opc)
    setOperationAction(Opc, Ty, Expand);

  // for vfcmpxxs
  setTruncStoreAction(MVT::v4i64, MVT::v4i32, Custom);

  setOperationAction(ISD::BITCAST, Ty, Legal);
  setOperationAction(ISD::LOAD, Ty, Legal);
  setOperationAction(ISD::STORE, Ty, Legal);

  setOperationAction(ISD::EXTRACT_VECTOR_ELT, Ty, Custom);
  setOperationAction(ISD::INSERT_VECTOR_ELT, Ty, Custom);

  setOperationAction(ISD::ROTL, Ty, Custom);
  setOperationAction(ISD::ROTR, Ty, Expand);
  setOperationAction(ISD::ADD, Ty, Legal);
  setOperationAction(ISD::AND, Ty, Legal);
  setOperationAction(ISD::MUL, Ty, Legal);
  setOperationAction(ISD::OR, Ty, Legal);
  setOperationAction(ISD::SDIV, Ty, Legal);
  setOperationAction(ISD::SREM, Ty, Legal);
  setOperationAction(ISD::SUB, Ty, Legal);
  setOperationAction(ISD::UDIV, Ty, Legal);
  setOperationAction(ISD::UREM, Ty, Legal);
  setOperationAction(ISD::UMAX, Ty, Legal);
  setOperationAction(ISD::UMIN, Ty, Legal);
  setOperationAction(ISD::VECTOR_SHUFFLE, Ty, Custom);
  setOperationAction(ISD::XOR, Ty, Legal);

  setOperationAction(ISD::VECREDUCE_ADD, Ty, Legal);

  if (Ty == MVT::v8i32 || Ty == MVT::v4i64) {
    setOperationAction(ISD::FP_TO_SINT, Ty, Legal);
    setOperationAction(ISD::FP_TO_UINT, Ty, Legal);
    setOperationAction(ISD::SINT_TO_FP, Ty, Legal);
    setOperationAction(ISD::UINT_TO_FP, Ty, Legal);
  }
  setCondCodeAction(ISD::SETNE, Ty, Expand);
  setCondCodeAction(ISD::SETGE, Ty, Expand);
  setCondCodeAction(ISD::SETGT, Ty, Expand);
  setCondCodeAction(ISD::SETUGE, Ty, Expand);
  setCondCodeAction(ISD::SETUGT, Ty, Expand);
}

// Enable SIMD support for the given floating-point type and Register class.
void Sw64TargetLowering::addSIMDFloatType(MVT::SimpleValueType Ty,
                                          const TargetRegisterClass *RC) {
  addRegisterClass(Ty, RC);

  // Expand all builtin opcodes.
  for (unsigned Opc = 0; Opc < ISD::BUILTIN_OP_END; ++Opc)
    setOperationAction(Opc, Ty, Expand);

  setOperationAction(ISD::LOAD, Ty, Legal);
  setOperationAction(ISD::STORE, Ty, Legal);
  setOperationAction(ISD::BITCAST, Ty, Legal);
  setOperationAction(ISD::INSERT_VECTOR_ELT, Ty, Custom);
  setOperationAction(ISD::BUILD_VECTOR, Ty, Custom);

  setOperationAction(ISD::FCOPYSIGN, Ty, Legal);

  if (Ty != MVT::v16f16) {
    setOperationAction(ISD::FABS, Ty, Expand);
    setOperationAction(ISD::FADD, Ty, Legal);
    setOperationAction(ISD::FDIV, Ty, Legal);
    setOperationAction(ISD::FEXP2, Ty, Legal);
    setOperationAction(ISD::FLOG2, Ty, Legal);
    setOperationAction(ISD::FMA, Ty, Legal);
    setOperationAction(ISD::FMUL, Ty, Legal);
    setOperationAction(ISD::FRINT, Ty, Legal);
    setOperationAction(ISD::FSQRT, Ty, Legal);
    setOperationAction(ISD::FSUB, Ty, Legal);
    setOperationAction(ISD::VSELECT, Ty, Legal);

    setOperationAction(ISD::SETCC, Ty, Legal);
    setCondCodeAction(ISD::SETO, Ty, Custom);
    setCondCodeAction(ISD::SETOGE, Ty, Expand);
    setCondCodeAction(ISD::SETOGT, Ty, Expand);
    setCondCodeAction(ISD::SETUGE, Ty, Expand);
    setCondCodeAction(ISD::SETUGT, Ty, Expand);
    setCondCodeAction(ISD::SETGE, Ty, Expand);
    setCondCodeAction(ISD::SETGT, Ty, Expand);
    setOperationAction(ISD::VECTOR_SHUFFLE, Ty, Custom);
  }
}

// Fold zero extensions into Sw64ISD::VEXTRACT_[SZ]EXT_ELT
//
// Performs the following transformations:
// - Changes Sw64ISD::VEXTRACT_[SZ]EXT_ELT to zero extension if its
//   sign/zero-extension is completely overwritten by the new one performed by
//   the ISD::AND.
// - Removes redundant zero extensions performed by an ISD::AND.
static SDValue performANDCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const Sw64Subtarget &Subtarget) {
  return SDValue();
}

// Perform combines where ISD::OR is the root node.
//
// Performs the following transformations:
// - (or (and $a, $mask), (and $b, $inv_mask)) => (vselect $mask, $a, $b)
//   where $inv_mask is the bitwise inverse of $mask and the 'or' has a 128-bit
//   vector type.
static SDValue performORCombine(SDNode *N, SelectionDAG &DAG,
                                TargetLowering::DAGCombinerInfo &DCI,
                                const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static bool shouldTransformMulToShiftsAddsSubs(APInt C, EVT VT,
                                               SelectionDAG &DAG,
                                               const Sw64Subtarget &Subtarget) {
  unsigned MaxSteps = 4;
  SmallVector<APInt, 16> WorkStack(1, C);
  unsigned Steps = 0;
  unsigned BitWidth = C.getBitWidth();

  while (!WorkStack.empty()) {
    APInt Val = WorkStack.pop_back_val();

    if (Val == 0 || Val == 1)
      continue;

    if (Steps >= MaxSteps)
      return false;

    if (Val.isPowerOf2()) {
      ++Steps;
      continue;
    }

    APInt Floor = APInt(BitWidth, 1) << Val.logBase2();
    APInt Ceil = Val.isNegative() ? APInt(BitWidth, 0)
                                  : APInt(BitWidth, 1) << C.ceilLogBase2();
    if ((Val - Floor).ule(Ceil - Val)) {
      WorkStack.push_back(Floor);
      WorkStack.push_back(Val - Floor);
    } else {
      WorkStack.push_back(Ceil);
      WorkStack.push_back(Ceil - Val);
    }

    ++Steps;
  }
  // If the value being multiplied is not supported natively, we have to pay
  // an additional legalization cost, conservatively assume an increase in the
  // cost of 3 instructions per step. This values for this heuristic were
  // determined experimentally.
  unsigned RegisterSize = DAG.getTargetLoweringInfo()
                              .getRegisterType(*DAG.getContext(), VT)
                              .getSizeInBits();
  Steps *= (VT.getSizeInBits() != RegisterSize) * 3;
  if (Steps > 27)
    return false;

  return true;
}

static SDValue genConstMult(SDValue X, APInt C, const SDLoc &DL, EVT VT,
                            EVT ShiftTy, SelectionDAG &DAG) {
  // Return 0.
  if (C == 0)
    return DAG.getConstant(0, DL, VT);

  // Return x.
  if (C == 1)
    return X;

  // If c is power of 2, return (shl x, log2(c)).
  if (C.isPowerOf2())
    return DAG.getNode(ISD::SHL, DL, VT, X,
                       DAG.getConstant(C.logBase2(), DL, ShiftTy));

  unsigned BitWidth = C.getBitWidth();
  APInt Floor = APInt(BitWidth, 1) << C.logBase2();
  APInt Ceil = C.isNegative() ? APInt(BitWidth, 0)
                              : APInt(BitWidth, 1) << C.ceilLogBase2();

  // If |c - floor_c| <= |c - ceil_c|,
  // where floor_c = pow(2, floor(log2(c))) and ceil_c = pow(2, ceil(log2(c))),
  // return (add constMult(x, floor_c), constMult(x, c - floor_c)).
  if ((C - Floor).ule(Ceil - C)) {
    SDValue Op0 = genConstMult(X, Floor, DL, VT, ShiftTy, DAG);
    SDValue Op1 = genConstMult(X, C - Floor, DL, VT, ShiftTy, DAG);
    return DAG.getNode(ISD::ADD, DL, VT, Op0, Op1);
  }

  // If |c - floor_c| > |c - ceil_c|,
  // return (sub constMult(x, ceil_c), constMult(x, ceil_c - c)).
  SDValue Op0 = genConstMult(X, Ceil, DL, VT, ShiftTy, DAG);
  SDValue Op1 = genConstMult(X, Ceil - C, DL, VT, ShiftTy, DAG);
  return DAG.getNode(ISD::SUB, DL, VT, Op0, Op1);
}

static SDValue performMULCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const Sw64Subtarget &Subtarget) {
  EVT VT = N->getValueType(0);

  if (Subtarget.enOptMul())
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(N->getOperand(1)))
      if (!VT.isVector() && shouldTransformMulToShiftsAddsSubs(
                                C->getAPIntValue(), VT, DAG, Subtarget))
        return genConstMult(N->getOperand(0), C->getAPIntValue(), SDLoc(N), VT,
                            MVT::i64, DAG);

  return SDValue(N, 0);
}

static SDValue performSHLCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static SDValue performSRACombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static SDValue performSRLCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static SDValue performSETCCCombine(SDNode *N, SelectionDAG &DAG) {
  return SDValue();
}

static SDValue performVSELECTCombine(SDNode *N, SelectionDAG &DAG) {
  return SDValue();
}

static SDValue performXORCombine(SDNode *N, SelectionDAG &DAG,
                                 const Sw64Subtarget &Subtarget) {
  return SDValue();
}

SDValue Sw64TargetLowering::PerformDAGCombine(SDNode *N,
                                              DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  SDValue Val;

  switch (N->getOpcode()) {
  case ISD::AND:
    Val = performANDCombine(N, DAG, DCI, Subtarget);
    break;
  case ISD::OR:
    Val = performORCombine(N, DAG, DCI, Subtarget);
    break;
  case ISD::MUL:
    return performMULCombine(N, DAG, DCI, Subtarget);
  case ISD::SHL:
    Val = performSHLCombine(N, DAG, DCI, Subtarget);
    break;
  case ISD::SRA:
    return performSRACombine(N, DAG, DCI, Subtarget);
  case ISD::SRL:
    return performSRLCombine(N, DAG, DCI, Subtarget);
  case ISD::VSELECT:
    return performVSELECTCombine(N, DAG);
  case ISD::XOR:
    Val = performXORCombine(N, DAG, Subtarget);
    break;
  case ISD::SETCC:
    Val = performSETCCCombine(N, DAG);
    break;
  }

  if (Val.getNode()) {
    LLVM_DEBUG(dbgs() << "\nSw64 DAG Combine:\n";
               N->printrWithDepth(dbgs(), &DAG); dbgs() << "\n=> \n";
               Val.getNode()->printrWithDepth(dbgs(), &DAG); dbgs() << "\n");
    return Val;
  }

  return Sw64TargetLowering::PerformDAGCombineV(N, DCI);
}

/// ------------------------- scaler ------------------------------ ///

static SDValue performDivRemCombineV(SDNode *N, SelectionDAG &DAG,
                                     TargetLowering::DAGCombinerInfo &DCI,
                                     const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static SDValue performSELECTCombineV(SDNode *N, SelectionDAG &DAG,
                                     TargetLowering::DAGCombinerInfo &DCI,
                                     const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static SDValue performANDCombineV(SDNode *N, SelectionDAG &DAG,
                                  TargetLowering::DAGCombinerInfo &DCI,
                                  const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static SDValue performORCombineV(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static SDValue performADDCombineV(SDNode *N, SelectionDAG &DAG,
                                  TargetLowering::DAGCombinerInfo &DCI,
                                  const Sw64Subtarget &Subtarget) {
  return SDValue();
}

static SDValue performSHLCombineV(SDNode *N, SelectionDAG &DAG,
                                  TargetLowering::DAGCombinerInfo &DCI,
                                  const Sw64Subtarget &Subtarget) {
  return SDValue();
}

SDValue Sw64TargetLowering::PerformDAGCombineV(SDNode *N,
                                               DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  unsigned Opc = N->getOpcode();

  switch (Opc) {
  default:
    break;
  case ISD::SDIVREM:
  case ISD::UDIVREM:
    return performDivRemCombineV(N, DAG, DCI, Subtarget);
  case ISD::SELECT:
    return performSELECTCombineV(N, DAG, DCI, Subtarget);
  case ISD::AND:
    return performANDCombineV(N, DAG, DCI, Subtarget);
  case ISD::OR:
    return performORCombineV(N, DAG, DCI, Subtarget);
  case ISD::ADD:
    return performADDCombineV(N, DAG, DCI, Subtarget);
  case ISD::SHL:
    return performSHLCombineV(N, DAG, DCI, Subtarget);
  }

  return SDValue();
}

SDValue Sw64TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                      SmallVectorImpl<SDValue> &InVals) const {

  SelectionDAG &DAG = CLI.DAG;
  SDLoc &dl = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &isTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;
  EVT PtrVT = getPointerTy(DAG.getDataLayout());

  MachineFunction &MF = DAG.getMachineFunction();
  // Sw64 target does not yet support tail call optimization.
  isTailCall = false;

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_Sw64);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getStackSize();
  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, dl);
  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;
  SDValue StackPtr;
  RegsToPass.push_back(std::make_pair((unsigned)Sw64::R27, Callee));

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
    default:
      assert(0 && "Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    }
    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

      if (StackPtr.getNode() == 0)
        StackPtr = DAG.getCopyFromReg(Chain, dl, Sw64::R30, MVT::i64);

      SDValue PtrOff =
          DAG.getNode(ISD::ADD, dl, getPointerTy(DAG.getDataLayout()), StackPtr,
                      DAG.getIntPtrConstant(VA.getLocMemOffset(), dl));

      MemOpChains.push_back(
          DAG.getStore(Chain, dl, Arg, PtrOff, MachinePointerInfo()));
    }
  }
  const ExternalSymbolSDNode *ES =
      dyn_cast_or_null<const ExternalSymbolSDNode>(Callee.getNode());
  if (ES && isF128SoftLibCall_round(ES->getSymbol())) {
    RegsToPass.push_back(std::make_pair(((unsigned)Sw64::R16) + ArgLocs.size(),
                                        DAG.getConstant(2, dl, MVT::i64)));
  }

  // FIXME: Fix the error for clang-repl.

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  // Fix the error for clang-repl.
  // Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (!isTailCall) {
    const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
    const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
    assert(Mask && "Missing call preserved mask for calling convention");
    Ops.push_back(DAG.getRegisterMask(Mask));
  }

  if (InFlag.getNode())
    Ops.push_back(InFlag);
  Chain = DAG.getNode(Sw64ISD::JmpLink, dl, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(
      Chain,
      DAG.getConstant(NumBytes, dl, getPointerTy(DAG.getDataLayout()), true),
      DAG.getConstant(0, dl, getPointerTy(DAG.getDataLayout()), true), InFlag,
      dl);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, dl, DAG,
                         InVals, CLI.Callee.getNode(), CLI.RetTy);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
SDValue Sw64TargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc &dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals, const SDNode *CallNode,
    const Type *RetTy) const {
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  const ExternalSymbolSDNode *ES =
      dyn_cast_or_null<const ExternalSymbolSDNode>(CallNode);

  if (ES && isF128SoftLibCall_void(ES->getSymbol()))
    CCInfo.AnalyzeCallResult(Ins, RetCC_F128Soft_Sw64);
  else

    CCInfo.AnalyzeCallResult(Ins, RetCC_Sw64);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];

    Chain = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getLocVT(), InFlag)
                .getValue(1);

    SDValue RetValue = Chain.getValue(0);
    InFlag = Chain.getValue(2);

    // If this is an 8/16/32-bit value, it is really passed promoted to 64
    // bits. Insert an assert[sz]ext to capture this, then truncate to the
    // right size.

    if (VA.getLocInfo() == CCValAssign::SExt)
      RetValue = DAG.getNode(ISD::AssertSext, dl, VA.getLocVT(), RetValue,
                             DAG.getValueType(VA.getValVT()));
    else if (VA.getLocInfo() == CCValAssign::ZExt)
      RetValue = DAG.getNode(ISD::AssertZext, dl, VA.getLocVT(), RetValue,
                             DAG.getValueType(VA.getValVT()));

    if (VA.getLocInfo() != CCValAssign::Full)
      RetValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), RetValue);

    InVals.push_back(RetValue);
  }

  return Chain;
}

SDValue Sw64TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  Sw64MachineFunctionInfo *FuncInfo = MF.getInfo<Sw64MachineFunctionInfo>();

  unsigned args_int[] = {Sw64::R16, Sw64::R17, Sw64::R18,
                         Sw64::R19, Sw64::R20, Sw64::R21};
  unsigned args_float[] = {Sw64::F16, Sw64::F17, Sw64::F18,
                           Sw64::F19, Sw64::F20, Sw64::F21};
  unsigned args_vector[] = {Sw64::V16, Sw64::V17, Sw64::V18,
                            Sw64::V19, Sw64::V20, Sw64::V21};

  for (unsigned ArgNo = 0, e = Ins.size(); ArgNo != e; ++ArgNo) {
    SDValue argt;
    EVT ObjectVT = Ins[ArgNo].VT;
    SDValue ArgVal;
    if (ArgNo < 6) {
      switch (ObjectVT.getSimpleVT().SimpleTy) {
      default:
        assert(false && "Invalid value type!");
      case MVT::f64:
        args_float[ArgNo] =
            AddLiveIn(MF, args_float[ArgNo], &Sw64::F8RCRegClass);
        ArgVal = DAG.getCopyFromReg(Chain, dl, args_float[ArgNo], ObjectVT);
        break;
      case MVT::f32:
        args_float[ArgNo] =
            AddLiveIn(MF, args_float[ArgNo], &Sw64::F4RCRegClass);
        ArgVal = DAG.getCopyFromReg(Chain, dl, args_float[ArgNo], ObjectVT);
        break;
      case MVT::i64:
        args_int[ArgNo] = AddLiveIn(MF, args_int[ArgNo], &Sw64::GPRCRegClass);
        ArgVal = DAG.getCopyFromReg(Chain, dl, args_int[ArgNo], MVT::i64);
        break;
      case MVT::v32i8:
      case MVT::v16i16:
      case MVT::v8i32:
      case MVT::v4i64:
      case MVT::v4f32:
      case MVT::v4f64:
        args_vector[ArgNo] =
            AddLiveIn(MF, args_vector[ArgNo], &Sw64::V256LRegClass);
        ArgVal = DAG.getCopyFromReg(Chain, dl, args_vector[ArgNo], ObjectVT);
        break;
      }
    } else { // more args
      // Create the frame index object for this incoming parameter...
      int FI = MFI.CreateFixedObject(8, 8 * (ArgNo - 6), true);

      // Create the SelectionDAG nodes corresponding to a load
      // from this parameter
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i64);
      ArgVal = DAG.getLoad(ObjectVT, dl, Chain, FIN, MachinePointerInfo());
    }
    InVals.push_back(ArgVal);
  }

  // If the functions takes variable number of arguments, copy all regs to stack
  if (isVarArg) {
    FuncInfo->setVarArgsOffset(Ins.size() * 8);
    std::vector<SDValue> LS;
    for (int i = 0; i < 6; ++i) {
      if (Register::isPhysicalRegister(args_int[i]))
        args_int[i] = AddLiveIn(MF, args_int[i], &Sw64::GPRCRegClass);
      SDValue argt = DAG.getCopyFromReg(Chain, dl, args_int[i], MVT::i64);
      int FI = MFI.CreateFixedObject(8, -8 * (6 - i), true);
      if (i == 0)
        FuncInfo->setVarArgsBase(FI);
      SDValue SDFI = DAG.getFrameIndex(FI, MVT::i64);
      LS.push_back(DAG.getStore(Chain, dl, argt, SDFI, MachinePointerInfo()));
      if (Register::isPhysicalRegister(args_float[i]))
        args_float[i] = AddLiveIn(MF, args_float[i], &Sw64::F8RCRegClass);
      argt = DAG.getCopyFromReg(Chain, dl, args_float[i], MVT::f64);
      FI = MFI.CreateFixedObject(8, -8 * (12 - i), true);
      SDFI = DAG.getFrameIndex(FI, MVT::i64);
      LS.push_back(DAG.getStore(Chain, dl, argt, SDFI, MachinePointerInfo()));
    }
    // Set up a token factor with all the stack traffic
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, LS);
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

bool Sw64TargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, RetCC_Sw64);
}

SDValue
Sw64TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &dl, SelectionDAG &DAG) const {

  SDValue Copy = DAG.getCopyToReg(
      Chain, dl, Sw64::R26, DAG.getNode(Sw64ISD::GlobalRetAddr, dl, MVT::i64),
      SDValue());
  SmallVector<SDValue, 4> RetOps(1, Chain);

  SDValue Flag;
  unsigned outSize = Outs.size();
  unsigned *ArgReg = new unsigned[outSize];
  for (unsigned j = 0, r = 0, f = 0, v = 0; j != outSize; j++) {
    EVT ArgVT = Outs[j].VT;
    switch (ArgVT.getSimpleVT().SimpleTy) {
    default:
      if (ArgVT.isInteger())
        ArgReg[j] = Sw64::R0 + r++;
      else
        ArgReg[j] = Sw64::F0 + f++;
      Copy =
          DAG.getCopyToReg(Copy, dl, ArgReg[j], OutVals[j], Copy.getValue(1));

      if (ArgVT.isInteger())
        RetOps.push_back(DAG.getRegister(ArgReg[j], MVT::i64));
      else
        RetOps.push_back(DAG.getRegister(ArgReg[j], ArgVT.getSimpleVT()));
      break;

    case MVT::v32i8:
    case MVT::v16i16:
    case MVT::v8i32:
    case MVT::v4i64:
    case MVT::v4f32:
    case MVT::v4f64:
      ArgReg[j] = Sw64::V0 + v++;
      Copy =
          DAG.getCopyToReg(Copy, dl, ArgReg[j], OutVals[j], Copy.getValue(1));
      RetOps.push_back(DAG.getRegister(ArgReg[j], ArgVT.getSimpleVT()));
      break;
    }
  }

  RetOps[0] = Copy;
  RetOps.push_back(Copy.getValue(1));
  return DAG.getNode(Sw64ISD::Ret, dl, MVT::Other, RetOps);
}

void Sw64TargetLowering::LowerVAARG(SDNode *N, SDValue &Chain, SDValue &DataPtr,
                                    SelectionDAG &DAG) const {

  SDLoc dl(N);
  Chain = N->getOperand(0);
  SDValue VAListP = N->getOperand(1);
  const Value *VAListS = cast<SrcValueSDNode>(N->getOperand(2))->getValue();
  unsigned Align = cast<ConstantSDNode>(N->getOperand(3))->getZExtValue();
  Align = std::max(Align,8u);

  SDValue Base =
      DAG.getLoad(MVT::i64, dl, Chain, VAListP, MachinePointerInfo(VAListS));
  SDValue Tmp = DAG.getNode(ISD::ADD, dl, MVT::i64, VAListP,
                            DAG.getConstant(8, dl, MVT::i64));
  SDValue Offset = DAG.getExtLoad(ISD::SEXTLOAD, dl, MVT::i64, Base.getValue(1),
                                  Tmp, MachinePointerInfo(), MVT::i32);
  DataPtr = DAG.getNode(ISD::ADD, dl, MVT::i64, Base, Offset);
  if (N->getValueType(0).isFloatingPoint()) {
    // if fp && Offset < 6*8, then subtract 6*8 from DataPtr
    SDValue FPDataPtr = DAG.getNode(ISD::SUB, dl, MVT::i64, DataPtr,
                                    DAG.getConstant(8 * 6, dl, MVT::i64));
    SDValue CC = DAG.getSetCC(dl, MVT::i64, Offset,
                              DAG.getConstant(8 * 6, dl, MVT::i64), ISD::SETLT);
    DataPtr = DAG.getNode(ISD::SELECT, dl, MVT::i64, CC, FPDataPtr, DataPtr);
  }
  SDValue NewOffset = DAG.getNode(
      ISD::ADD, dl, MVT::i64, Offset,
      DAG.getConstant(Align, dl, MVT::i64));
  Chain = DAG.getTruncStore(Offset.getValue(1), dl, NewOffset, Tmp,
                            MachinePointerInfo(), MVT::i32);
}

/// LowerOperation - Provide custom lowering hooks for some operations.
SDValue Sw64TargetLowering::LowerOperation(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc dl(Op);
  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("Wasn't expecting to be able to lower this!");
  case ISD::JumpTable:
    return LowerJumpTable(Op, DAG);
  case ISD::INTRINSIC_WO_CHAIN:
    return LowerINTRINSIC_WO_CHAIN(Op, DAG);
  case ISD::INTRINSIC_W_CHAIN:
    return LowerINTRINSIC_W_CHAIN(Op, DAG);
  case ISD::INTRINSIC_VOID:
    return LowerINTRINSIC_VOID(Op, DAG);
  case ISD::SRL_PARTS:
    return LowerSRL_PARTS(Op, DAG);
  case ISD::SRA_PARTS:
    return LowerSRA_PARTS(Op, DAG);
  case ISD::SHL_PARTS:
    return LowerSHL_PARTS(Op, DAG);
  case ISD::SINT_TO_FP:
    return LowerSINT_TO_FP(Op, DAG);
  case ISD::FP_TO_SINT:
    return LowerFP_TO_SINT(Op, DAG);
  case ISD::FP_TO_SINT_SAT:
  case ISD::FP_TO_UINT_SAT:
    return LowerFP_TO_INT_SAT(Op, DAG);
  case ISD::ConstantPool:
    return LowerConstantPool(Op, DAG);
  case ISD::BlockAddress:
    return LowerBlockAddress(Op, DAG);
  case ISD::GlobalTLSAddress:
    return LowerGlobalTLSAddress(Op, DAG);
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::ExternalSymbol:
    return LowerExternalSymbol(Op, DAG);
  case ISD::ATOMIC_FENCE:
    return LowerATOMIC_FENCE(Op, DAG);
  case ISD::ATOMIC_LOAD:
    return LowerATOMIC_LOAD(Op, DAG);
  case ISD::ATOMIC_STORE:
    return LowerATOMIC_STORE(Op, DAG);
  case ISD::OR:
    return LowerOR(Op, DAG);
  case ISD::UREM:
  case ISD::SREM:
    return LowerSUREM(Op, DAG);
  // fall through
  case ISD::SDIV:
  case ISD::UDIV:
    return LowerSUDIV(Op, DAG);
  case ISD::VAARG:
    return LowerVAARG(Op, DAG);
  case ISD::VACOPY:
    return LowerVACOPY(Op, DAG);
  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  case ISD::RETURNADDR:
    return DAG.getNode(Sw64ISD::GlobalRetAddr, dl, MVT::i64);
  case ISD::FRAMEADDR:
    return LowerFRAMEADDR(Op, DAG);
  case ISD::PREFETCH:
    return LowerPREFETCH(Op, DAG);
  case ISD::EXTRACT_VECTOR_ELT:
    return LowerEXTRACT_VECTOR_ELT(Op, DAG);
  case ISD::INSERT_VECTOR_ELT:
    return LowerINSERT_VECTOR_ELT(Op, DAG);
  case ISD::BUILD_VECTOR:
    return LowerBUILD_VECTOR(Op, DAG);
  case ISD::SHL:
  case ISD::SRL:
  case ISD::SRA:
  case ISD::ROTL:
    return LowerVectorShift(Op, DAG);
  case ISD::VECTOR_SHUFFLE:
    return LowerVECTOR_SHUFFLE(Op, DAG);
  case ISD::SETCC:
    return LowerSETCC(Op, DAG);
  case ISD::STORE:
    return LowerSTORE(Op, DAG);
  }

  return SDValue();
}

SDValue Sw64TargetLowering::LowerVectorShift(SDValue Op,
                                             SelectionDAG &DAG) const {
  // Look for cases where a vector shift can use the *_BY_SCALAR form.
  // SDValue Op0 = Op.getOperand(0);
  // SDValue Op1 = Op.getOperand(1);
  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  // See whether the shift vector is a splat represented as BUILD_VECTOR.
  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("unexpect vecotr opcode");
  case ISD::ROTL:
    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, VT,
                       DAG.getConstant(Intrinsic::sw64_vrol, DL, MVT::i64),
                       Op.getOperand(0), Op.getOperand(1));
  case ISD::SHL:
    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, VT,
                       DAG.getConstant(Intrinsic::sw64_vsll, DL, MVT::i64),
                       Op.getOperand(0), Op.getOperand(1));
  case ISD::SRL:
  case ISD::SRA:
    unsigned Opc = (Op.getOpcode() == ISD::SRA) ? Intrinsic::sw64_vsra
                                                : Intrinsic::sw64_vsrl;

    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, VT,
                       DAG.getConstant(Opc, DL, MVT::i64), Op.getOperand(0),
                       Op.getOperand(1));
  }

  // Otherwise just treat the current form as legal.
  return Op;
}

// Lower Operand specifics
SDValue Sw64TargetLowering::LowerJumpTable(SDValue Op,
                                           SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "Sw64:: begin lowJumpTable----\n");
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  // FIXME there isn't really any debug info here
  SDLoc dl(Op);
  return getAddr(JT, DAG);
}

SDValue Sw64TargetLowering::LowerConstantPool(SDValue Op,
                                              SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "Sw64:: begin lowConstantPool----\n");
  SDLoc dl(Op);
  SDLoc DL(Op);
  ConstantPoolSDNode *N = cast<ConstantPoolSDNode>(Op);
  // FIXME there isn't really any debug info here
  return getAddr(N, DAG);
}

SDValue Sw64TargetLowering::LowerBlockAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "Sw64:: begin lowBlockAddress----\n");
  SDLoc dl(Op);
  SDLoc DL(Op);

  BlockAddressSDNode *BA = cast<BlockAddressSDNode>(Op);
  return getAddr(BA, DAG);
}

SDValue Sw64TargetLowering::LowerGlobalAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "Sw64:: begin lowGlobalAddress----\n");
  SDLoc dl(Op);
  GlobalAddressSDNode *GSDN = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = GSDN->getGlobal();
  SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i64, GSDN->getOffset());
  // FIXME there isn't really any debug info here
  if (GV->hasLocalLinkage()) {
    return getAddr(GSDN, DAG);
  } else
    return DAG.getNode(Sw64ISD::RelLit, dl, MVT::i64, GA,
                       DAG.getGLOBAL_OFFSET_TABLE(MVT::i64));
}

template <class NodeTy>
SDValue Sw64TargetLowering::getAddr(NodeTy *N, SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "Sw64TargetLowering:: getAddr");
  EVT Ty = getPointerTy(DAG.getDataLayout());
  SDLoc DL(N);

  switch (getTargetMachine().getCodeModel()) {
  default:
    report_fatal_error("Unsupported code model for lowering");
  case CodeModel::Small:
  case CodeModel::Medium: {
    SDValue Hi = getTargetNode(N, DL, Ty, DAG, Sw64II::MO_GPREL_HI);
    SDValue Lo = getTargetNode(N, DL, Ty, DAG, Sw64II::MO_GPREL_LO);
    SDValue MNHi = DAG.getNode(Sw64ISD::LDIH, DL, Ty, Hi);
    return DAG.getNode(Sw64ISD::LDI, DL, Ty, MNHi, Lo);
  }
  }
}

SDValue Sw64TargetLowering::LowerGlobalTLSAddress(SDValue Op,
                                                  SelectionDAG &DAG) const {

  // If the relocation model is PIC, use the General Dynamic TLS Model or
  // Local Dynamic TLS model, otherwise use the Initial Exec or
  // Local Exec TLS Model.

  GlobalAddressSDNode *GSDN = cast<GlobalAddressSDNode>(Op);
  if (DAG.getTarget().useEmulatedTLS())
    return LowerToTLSEmulatedModel(GSDN, DAG);

  SDLoc dl(Op);
  const GlobalValue *GV = GSDN->getGlobal();

  EVT PtrVT = getPointerTy(DAG.getDataLayout());

  TLSModel::Model model = getTargetMachine().getTLSModel(GV);

  if (model == TLSModel::GeneralDynamic || model == TLSModel::LocalDynamic) {
    // General Dynamic == tlsgd
    // LocalDynamic    == tlsldm
    // GA == TGA
    SDValue Argument;
    if (model == TLSModel::GeneralDynamic) {
      SDValue Addr =
          DAG.getTargetGlobalAddress(GV, dl, PtrVT, 0, Sw64II::MO_TLSGD);
      Argument =
          SDValue(DAG.getMachineNode(Sw64::LDA, dl, MVT::i64, Addr,
                                     DAG.getGLOBAL_OFFSET_TABLE(MVT::i64)),
                  0);
    } else {
      SDValue Addr =
          DAG.getTargetGlobalAddress(GV, dl, PtrVT, 0, Sw64II::MO_TLSLDM);
      Argument =
          SDValue(DAG.getMachineNode(Sw64::LDA, dl, MVT::i64, Addr,
                                     DAG.getGLOBAL_OFFSET_TABLE(MVT::i64)),
                  0);
    }
    unsigned PtrSize = PtrVT.getSizeInBits();
    IntegerType *PtrTy = Type::getIntNTy(*DAG.getContext(), PtrSize);
    SDValue TlsGetAddr = DAG.getExternalSymbol("__tls_get_addr", PtrVT);
    ArgListTy Args;
    ArgListEntry Entry;
    Entry.Node = Argument;
    Entry.Ty = PtrTy;
    Args.push_back(Entry);
    TargetLowering::CallLoweringInfo CLI(DAG);
    CLI.setDebugLoc(dl)
        .setChain(DAG.getEntryNode())
        .setLibCallee(CallingConv::C, PtrTy, TlsGetAddr, std::move(Args));
    std::pair<SDValue, SDValue> CallResult = LowerCallTo(CLI);

    SDValue Ret = CallResult.first;
    if (model != TLSModel::LocalDynamic)
      return Ret;

    SDValue DTPHi = DAG.getTargetGlobalAddress(
        GV, dl, MVT::i64, GSDN->getOffset(), Sw64II::MO_DTPREL_HI);
    SDValue DTPLo = DAG.getTargetGlobalAddress(
        GV, dl, MVT::i64, GSDN->getOffset(), Sw64II::MO_DTPREL_LO);

    SDValue Hi =
        SDValue(DAG.getMachineNode(Sw64::LDAH, dl, MVT::i64, DTPHi, Ret), 0);
    return SDValue(DAG.getMachineNode(Sw64::LDA, dl, MVT::i64, DTPLo, Hi), 0);
  }

  if (model == TLSModel::InitialExec) {
    // Initial Exec TLS Model //gottprel
    SDValue Gp = DAG.getGLOBAL_OFFSET_TABLE(MVT::i64);
    SDValue Addr =
        DAG.getTargetGlobalAddress(GV, dl, PtrVT, 0, Sw64II::MO_GOTTPREL);
    SDValue RelDisp =
        SDValue(DAG.getMachineNode(Sw64::LDL, dl, MVT::i64, Addr, Gp), 0);
    SDValue SysCall = DAG.getNode(Sw64ISD::SysCall, dl, MVT::i64,
                                  DAG.getConstant(0x9e, dl, MVT::i64));
    return SDValue(
        DAG.getMachineNode(Sw64::ADDQr, dl, MVT::i64, RelDisp, SysCall), 0);
  } else {
    // Local Exec TLS Model //tprelHi tprelLo
    assert(model == TLSModel::LocalExec);
    SDValue SysCall = DAG.getNode(Sw64ISD::SysCall, dl, MVT::i64,
                                  DAG.getConstant(0x9e, dl, MVT::i64));
    SDValue TPHi = DAG.getTargetGlobalAddress(
        GV, dl, MVT::i64, GSDN->getOffset(), Sw64II::MO_TPREL_HI);
    SDValue TPLo = DAG.getTargetGlobalAddress(
        GV, dl, MVT::i64, GSDN->getOffset(), Sw64II::MO_TPREL_LO);
    SDValue Hi =
        SDValue(DAG.getMachineNode(Sw64::LDAH, dl, MVT::i64, TPHi, SysCall), 0);
    return SDValue(DAG.getMachineNode(Sw64::LDA, dl, MVT::i64, TPLo, Hi), 0);
  }
}

static bool isCrossINSMask(ArrayRef<int> M, EVT VT) {
  unsigned NumElts = VT.getVectorNumElements();
  for (unsigned i = 0; i < NumElts; i++) {
    unsigned idx = i / 2;
    if (M[i] < 0)
      return false;
    if (M[i] != idx && (M[i] - NumElts) != idx)
      return false;
  }
  return true;
}

static SDValue GenerateVectorShuffle(SDValue Op, EVT VT, SelectionDAG &DAG,
                                     SDLoc dl) {
  ShuffleVectorSDNode *SVN = cast<ShuffleVectorSDNode>(Op.getNode());
  ArrayRef<int> ShuffleMask = SVN->getMask();
  if (ShuffleMask.size() > 8)
    return SDValue();

  unsigned NewMask;
  if (VT == MVT::v8i32) {
    for (int i = (ShuffleMask.size() - 1); i >= 0; i--) {
      NewMask = NewMask << 4;
      int idx = ShuffleMask[i];
      int bits = idx > 7 ? 1 : 0;
      idx = idx > 7 ? (idx - 8) : idx;
      NewMask |= (bits << 3) | idx;
    }
  } else if (VT == MVT::v4i64 || VT == MVT::v4f32 || VT == MVT::v4f64) {
    for (int i = ShuffleMask.size() * 2 - 1; i >= 0; i--) {
      NewMask = NewMask << 4;
      int idx = ShuffleMask[i / 2];
      int bits = idx > 3 ? 1 : 0;
      int mod = i % 2;
      idx = idx > 3 ? (idx * 2 + mod - 8) : idx * 2 + mod;
      NewMask |= (bits << 3) | idx;
    }
  }

  SDValue ConstMask = DAG.getConstant(NewMask, dl, MVT::i64);
  return DAG.getNode(Sw64ISD::VSHF, dl, VT, Op.getOperand(0), Op.getOperand(1),
                     ConstMask);
}

SDValue Sw64TargetLowering::LowerVECTOR_SHUFFLE(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc dl(Op);
  EVT VT = Op.getValueType();

  ShuffleVectorSDNode *SVN = cast<ShuffleVectorSDNode>(Op.getNode());
  // Convert shuffles that are directly supported on NEON to target-specific
  // DAG nodes, instead of keeping them as shuffles and matching them again
  // during code selection.  This is more efficient and avoids the possibility
  // of inconsistencies between legalization and selection.
  ArrayRef<int> ShuffleMask = SVN->getMask();

  SDValue V1 = Op.getOperand(0);
  SDValue V2 = Op.getOperand(1);
  assert(V1.getValueType() == VT && "Unexpected VECTOR_SHUFFLE type!");
  assert(ShuffleMask.size() == VT.getVectorNumElements() &&
         "Unexpected VECTOR_SHUFFLE mask size!");

  if (SVN->isSplat()) {
    int Lane = SVN->getSplatIndex();
    // If this is undef splat, generate it via "just" vdup, if possible.
    if (Lane == -1)
      Lane = 0;

    if (Lane == 0 && V1.getOpcode() == ISD::SCALAR_TO_VECTOR)
      return DAG.getNode(Sw64ISD::VBROADCAST, dl, V1.getValueType(),
                         V1.getOperand(0));

    // Test if V1 is a BUILD_VECTOR and the lane being referenced is a non-
    // constant. If so, we can just reference the lane's definition directly.
    if (V1.getOpcode() == ISD::BUILD_VECTOR &&
        !isa<ConstantSDNode>(V1.getOperand(Lane))) {
      SDValue Ext = DAG.getNode(ISD::EXTRACT_SUBVECTOR, dl, V1.getValueType(),
                                V1.getOperand(Lane));
      return DAG.getNode(Sw64ISD::VBROADCAST, dl, VT, Ext);
    }
  }
  if (isCrossINSMask(ShuffleMask, VT))
    return DAG.getNode(Sw64ISD::VINSECTL, dl, VT, V1, V2);

  // SmallVector<int, 32> NewMask;
  SDValue Tmp1 = GenerateVectorShuffle(Op, VT, DAG, dl);

  return Tmp1;
}

SDValue Sw64TargetLowering::LowerINTRINSIC_WO_CHAIN(SDValue Op,
                                                    SelectionDAG &DAG) const {
  SDLoc dl(Op);
  unsigned IntNo = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  unsigned NewIntrinsic;
  EVT VT = Op.getValueType();
  switch (IntNo) {
  default:
    break; // Don't custom lower most intrinsics.
  case Intrinsic::sw64_umulh:
    return DAG.getNode(ISD::MULHU, dl, MVT::i64, Op.getOperand(1),
                       Op.getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_crc32b:
    if (Subtarget.hasCore4() && Subtarget.enableCrcInst())
      return DAG.getNode(Sw64ISD::CRC32B, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_crc32h:
    if (Subtarget.hasCore4() && Subtarget.enableCrcInst())
      return DAG.getNode(Sw64ISD::CRC32H, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_crc32w:
    if (Subtarget.hasCore4() && Subtarget.enableCrcInst())
      return DAG.getNode(Sw64ISD::CRC32W, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_crc32l:
    if (Subtarget.hasCore4() && Subtarget.enableCrcInst())
      return DAG.getNode(Sw64ISD::CRC32L, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_crc32cb:
    if (Subtarget.hasCore4() && Subtarget.enableCrcInst())
      return DAG.getNode(Sw64ISD::CRC32CB, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_crc32ch:
    if (Subtarget.hasCore4() && Subtarget.enableCrcInst())
      return DAG.getNode(Sw64ISD::CRC32CH, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_crc32cw:
    if (Subtarget.hasCore4() && Subtarget.enableCrcInst())
      return DAG.getNode(Sw64ISD::CRC32CW, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_crc32cl:
    if (Subtarget.hasCore4() && Subtarget.enableCrcInst())
      return DAG.getNode(Sw64ISD::CRC32CL, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_sbt:
    if (Subtarget.hasCore4() && Subtarget.enableSCbtInst())
      return DAG.getNode(Sw64ISD::SBT, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    LLVM_FALLTHROUGH;
  case Intrinsic::sw64_cbt:
    if (Subtarget.hasCore4() && Subtarget.enableSCbtInst())
      return DAG.getNode(Sw64ISD::CBT, dl, Op->getValueType(0),
                         Op->getOperand(1), Op->getOperand(2));
    return Op;
  case Intrinsic::sw64_vsllb:
  case Intrinsic::sw64_vsllh:
  case Intrinsic::sw64_vsllw:
  case Intrinsic::sw64_vslll:
    NewIntrinsic = Intrinsic::sw64_vsll;
    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
                       DAG.getConstant(NewIntrinsic, dl, MVT::i64),
                       Op.getOperand(1), Op.getOperand(2));
  case Intrinsic::sw64_vsrlb:
  case Intrinsic::sw64_vsrlh:
  case Intrinsic::sw64_vsrlw:
  case Intrinsic::sw64_vsrll:
    NewIntrinsic = Intrinsic::sw64_vsrl;
    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
                       DAG.getConstant(NewIntrinsic, dl, MVT::i64),
                       Op.getOperand(1), Op.getOperand(2));
    // Fallthough
  case Intrinsic::sw64_vsrab:
  case Intrinsic::sw64_vsrah:
  case Intrinsic::sw64_vsraw:
  case Intrinsic::sw64_vsral:
    NewIntrinsic = Intrinsic::sw64_vsra;
    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
                       DAG.getConstant(NewIntrinsic, dl, MVT::i64),
                       Op.getOperand(1), Op.getOperand(2));
  case Intrinsic::sw64_vrolb:
  case Intrinsic::sw64_vrolh:
  case Intrinsic::sw64_vrolw:
  case Intrinsic::sw64_vroll:
    NewIntrinsic = Intrinsic::sw64_vrol;
    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
                       DAG.getConstant(NewIntrinsic, dl, MVT::i64),
                       Op.getOperand(1), Op.getOperand(2));
  case Intrinsic::sw64_vlogzz:
    return DAG.getNode(Sw64ISD::VLOG, dl, VT, Op.getOperand(1),
                       Op.getOperand(2), Op.getOperand(3), Op.getOperand(4));
  case Intrinsic::sw64_vmaxb:
  case Intrinsic::sw64_vmaxh:
  case Intrinsic::sw64_vmaxw:
  case Intrinsic::sw64_vmaxl:
    return DAG.getNode(Sw64ISD::VMAX, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vumaxb:
  case Intrinsic::sw64_vumaxh:
  case Intrinsic::sw64_vumaxw:
  case Intrinsic::sw64_vumaxl:
    return DAG.getNode(Sw64ISD::VUMAX, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vminb:
  case Intrinsic::sw64_vminh:
  case Intrinsic::sw64_vminw:
  case Intrinsic::sw64_vminl:
    return DAG.getNode(Sw64ISD::VMIN, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vuminb:
  case Intrinsic::sw64_vuminh:
  case Intrinsic::sw64_vuminw:
  case Intrinsic::sw64_vuminl:
    return DAG.getNode(Sw64ISD::VUMIN, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vmaxs:
  case Intrinsic::sw64_vmaxd:
    return DAG.getNode(Sw64ISD::VMAXF, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vmins:
  case Intrinsic::sw64_vmind:
    return DAG.getNode(Sw64ISD::VMINF, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));

  case Intrinsic::sw64_vseleqw:
  case Intrinsic::sw64_vseleqwi:
    return DAG.getNode(Sw64ISD::VSELEQW, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  case Intrinsic::sw64_vselltw:
  case Intrinsic::sw64_vselltwi:
    return DAG.getNode(Sw64ISD::VSELLTW, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  case Intrinsic::sw64_vsellew:
  case Intrinsic::sw64_vsellewi:
    return DAG.getNode(Sw64ISD::VSELLEW, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  case Intrinsic::sw64_vsellbcw:
  case Intrinsic::sw64_vsellbcwi:
    return DAG.getNode(Sw64ISD::VSELLBCW, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  case Intrinsic::sw64_vsqrts:
  case Intrinsic::sw64_vsqrtd:
    return DAG.getNode(Sw64ISD::VSQRT, dl, Op->getValueType(0),
                       Op->getOperand(1));

  case Intrinsic::sw64_vsums:
  case Intrinsic::sw64_vsumd:
    return DAG.getNode(Sw64ISD::VSUMF, dl, Op->getValueType(0),
                       Op->getOperand(1));

  case Intrinsic::sw64_vfrecs:
  case Intrinsic::sw64_vfrecd:
    return DAG.getNode(Sw64ISD::VFREC, dl, Op->getValueType(0),
                       Op->getOperand(1));

  case Intrinsic::sw64_vfcmpeqs:
  case Intrinsic::sw64_vfcmpeqd:
    return DAG.getNode(Sw64ISD::VFCMPEQ, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vfcmples:
  case Intrinsic::sw64_vfcmpled:
    return DAG.getNode(Sw64ISD::VFCMPLE, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vfcmplts:
  case Intrinsic::sw64_vfcmpltd:
    return DAG.getNode(Sw64ISD::VFCMPLT, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vfcmpuns:
  case Intrinsic::sw64_vfcmpund:
    return DAG.getNode(Sw64ISD::VFCMPUN, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));

  case Intrinsic::sw64_vfcvtsd:
    return DAG.getNode(Sw64ISD::VFCVTSD, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfcvtds:
    return DAG.getNode(Sw64ISD::VFCVTDS, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfcvtls:
    return DAG.getNode(Sw64ISD::VFCVTLS, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfcvtld:
    return DAG.getNode(Sw64ISD::VFCVTLD, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfcvtsh:
    return DAG.getNode(Sw64ISD::VFCVTSH, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  case Intrinsic::sw64_vfcvths:
    return DAG.getNode(Sw64ISD::VFCVTHS, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));

  case Intrinsic::sw64_vfcvtdl:
    return DAG.getNode(Sw64ISD::VFCVTDL, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfcvtdl_g:
    return DAG.getNode(Sw64ISD::VFCVTDLG, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfcvtdl_p:
    return DAG.getNode(Sw64ISD::VFCVTDLP, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfcvtdl_z:
    return DAG.getNode(Sw64ISD::VFCVTDLZ, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfcvtdl_n:
    return DAG.getNode(Sw64ISD::VFCVTDLN, dl, Op->getValueType(0),
                       Op->getOperand(1));

  case Intrinsic::sw64_vfris:
    return DAG.getNode(Sw64ISD::VFRIS, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfris_g:
    return DAG.getNode(Sw64ISD::VFRISG, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfris_p:
    return DAG.getNode(Sw64ISD::VFRISP, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfris_z:
    return DAG.getNode(Sw64ISD::VFRISZ, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfris_n:
    return DAG.getNode(Sw64ISD::VFRISN, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfrid:
    return DAG.getNode(Sw64ISD::VFRID, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfrid_g:
    return DAG.getNode(Sw64ISD::VFRIDG, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfrid_p:
    return DAG.getNode(Sw64ISD::VFRIDP, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfrid_z:
    return DAG.getNode(Sw64ISD::VFRIDZ, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vfrid_n:
    return DAG.getNode(Sw64ISD::VFRIDN, dl, Op->getValueType(0),
                       Op->getOperand(1));
  case Intrinsic::sw64_vextw:
  case Intrinsic::sw64_vextl:
  case Intrinsic::sw64_vextfs:
  case Intrinsic::sw64_vextfd:
    return DAG.getNode(ISD::EXTRACT_VECTOR_ELT, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2));
  case Intrinsic::sw64_vfseleqs:
  case Intrinsic::sw64_vfseleqd:
    return DAG.getNode(Sw64ISD::VFCMOVEQ, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  case Intrinsic::sw64_vfselles:
  case Intrinsic::sw64_vfselled:
    return DAG.getNode(Sw64ISD::VFCMOVLE, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  case Intrinsic::sw64_vfsellts:
  case Intrinsic::sw64_vfselltd:
    return DAG.getNode(Sw64ISD::VFCMOVLT, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  case Intrinsic::sw64_vshfw:
    return DAG.getNode(Sw64ISD::VSHF, dl, Op->getValueType(0),
                       Op->getOperand(1), Op->getOperand(2), Op->getOperand(3));
  }
  return Op;
}

SDValue Sw64TargetLowering::LowerVectorMemIntr(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDLoc dl(Op);
  unsigned IntNo = cast<ConstantSDNode>(Op.getOperand(1))->getZExtValue();
  EVT VT = Op.getValueType();
  LLVM_DEBUG(dbgs() << "Custom Lower Vector Memory Intrinsics\n"; Op.dump(););
  SDValue Args = Op.getOperand(2);
  switch (IntNo) {
  default:
    break;
  case Intrinsic::sw64_vload:
    return DAG.getNode(ISD::LOAD, dl, VT, Args);
  }
  return Op;
}

SDValue Sw64TargetLowering::LowerINTRINSIC_W_CHAIN(SDValue Op,
                                                   SelectionDAG &DAG) const {
  SDLoc dl(Op);
  unsigned IntNo = Op.getConstantOperandVal(1);
  unsigned NewIntrinsic;
  EVT VT = Op.getValueType();
  switch (IntNo) {
  default:
    break; // Don't custom lower most intrinsics.
  case Intrinsic::sw64_vloadu: {
    SDValue Chain = Op->getOperand(0);
    SDVTList VTs = DAG.getVTList(VT.getSimpleVT().SimpleTy, MVT::Other);
    NewIntrinsic = Intrinsic::sw64_vload_u;
    SDValue VLOAD_U1 = DAG.getNode(ISD::INTRINSIC_W_CHAIN, dl, VTs, Chain,
                                   DAG.getConstant(NewIntrinsic, dl, MVT::i64),
                                   Op.getOperand(2));
    SDValue Hiaddr =
        DAG.getNode(ISD::ADD, dl, MVT::i64,
                    DAG.getConstant((VT == MVT::v4f32 ? 16 : 32), dl, MVT::i64),
                    Op->getOperand(2));
    SDValue VLOAD_U2 =
        DAG.getNode(ISD::INTRINSIC_W_CHAIN, dl, VTs, Chain,
                    DAG.getConstant(NewIntrinsic, dl, MVT::i64), Hiaddr);

    switch (VT.getSimpleVT().SimpleTy) {
    default:
      break;
    case MVT::v8i32:
      NewIntrinsic = Intrinsic::sw64_vconw;
      break;
    case MVT::v4f32:
      NewIntrinsic = Intrinsic::sw64_vcons;
      break;
    case MVT::v4f64:
    case MVT::v4i64:
      NewIntrinsic = Intrinsic::sw64_vcond;
      break;
    }
    return DAG.getNode(ISD::INTRINSIC_W_CHAIN, dl, VTs, Chain,
                       DAG.getConstant(NewIntrinsic, dl, MVT::i64), VLOAD_U1,
                       VLOAD_U2, Op->getOperand(2));
  }
  }
  return SDValue();
}

SDValue Sw64TargetLowering::LowerINTRINSIC_VOID(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc dl(Op);
  unsigned IntNo = cast<ConstantSDNode>(Op.getOperand(1))->getZExtValue();
  unsigned NewIntrinsic;
  EVT VT = Op.getValueType();
  EVT VTOperand2 = Op.getOperand(2).getValueType();
  switch (IntNo) {
  case Intrinsic::sw64_vstoreu: {
    NewIntrinsic = Intrinsic::sw64_vstoreul;
    SDValue VSTOREUL =
        DAG.getNode(ISD::INTRINSIC_VOID, dl, VT, Op.getOperand(0),
                    DAG.getConstant(NewIntrinsic, dl, MVT::i64),
                    Op.getOperand(2), Op.getOperand(3));

    SDValue Hiaddr = DAG.getNode(
        ISD::ADD, dl, MVT::i64,
        DAG.getConstant((VTOperand2 == MVT::v4f32 ? 16 : 32), dl, MVT::i64),
        Op->getOperand(3));
    NewIntrinsic = Intrinsic::sw64_vstoreuh;
    return DAG.getNode(ISD::INTRINSIC_VOID, dl, VT, VSTOREUL,
                       DAG.getConstant(NewIntrinsic, dl, MVT::i64),
                       Op.getOperand(2), Hiaddr);
  }
  default:
    break;
  }
  return Op;
}

SDValue Sw64TargetLowering::LowerEXTRACT_VECTOR_ELT(SDValue Op,
                                                    SelectionDAG &DAG) const {
  SDLoc dl(Op);
  SDValue Vec = Op.getOperand(0);
  MVT VecVT = Vec.getSimpleValueType();
  SDValue Idx = Op.getOperand(1);
  MVT EltVT = VecVT.getVectorElementType();
  if (EltVT != MVT::i32 && EltVT != MVT::f32 && EltVT != MVT::f64)
    return SDValue();

  if (!dyn_cast<ConstantSDNode>(Idx))
    return SDValue();

  SDValue tmp = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, dl, MVT::i64, Vec, Idx);
  return tmp;
  //  return DAG.getAnyExtOrTrunc(tmp, dl, MVT::i32);
}

SDValue Sw64TargetLowering::LowerINSERT_VECTOR_ELT(SDValue Op,
                                                   SelectionDAG &DAG) const {
  SDLoc dl(Op);
  SDValue Idx = Op.getOperand(2);

  if (!dyn_cast<ConstantSDNode>(Idx))
    return SDValue();

  return Op;
}

static bool isConstantOrUndef(const SDValue Op) {
  if (Op->isUndef())
    return true;
  if (isa<ConstantSDNode>(Op))
    return true;
  if (isa<ConstantFPSDNode>(Op))
    return true;
  return false;
}

static bool isConstantOrUndefBUILD_VECTOR(const BuildVectorSDNode *Op) {
  for (unsigned i = 0; i < Op->getNumOperands(); ++i)
    if (isConstantOrUndef(Op->getOperand(i)))
      return true;
  return false;
}

SDValue Sw64TargetLowering::LowerBUILD_VECTOR(SDValue Op,
                                              SelectionDAG &DAG) const {
  BuildVectorSDNode *Node = cast<BuildVectorSDNode>(Op);
  SDLoc dl(Op);
  MVT VecVT = Op.getSimpleValueType();
  EVT ResTy = Op->getValueType(0);
  SDLoc DL(Op);
  APInt SplatValue, SplatUndef;
  unsigned SplatBitSize;
  bool HasAnyUndefs;

  if (!Subtarget.hasSIMD() || !ResTy.is256BitVector())
    return SDValue();

  if (VecVT.isInteger()) {
    // Certain vector constants, used to express things like logical NOT and
    // arithmetic NEG, are passed through unmodified.  This allows special
    // patterns for these operations to match, which will lower these constants
    // to whatever is proven necessary.
    BuildVectorSDNode *BVN = cast<BuildVectorSDNode>(Op.getNode());
    if (BVN->isConstant())
      if (ConstantSDNode *Const = BVN->getConstantSplatNode()) {
        unsigned BitSize = VecVT.getVectorElementType().getSizeInBits();
        APInt Val(BitSize,
                  Const->getAPIntValue().zextOrTrunc(BitSize).getZExtValue());
        if (Val.isZero() || Val.isAllOnes())
          return Op;
      }
  }
  MVT ElemTy = Op->getSimpleValueType(0).getScalarType();
  unsigned ElemBits = ElemTy.getSizeInBits();

  if (Node->isConstantSplat(SplatValue, SplatUndef, SplatBitSize, HasAnyUndefs,
                            8, false) &&
      SplatBitSize <= 64 && ElemBits == SplatBitSize) {
    // We can only cope with 8, 16, 32, or 64-bit elements
    if (SplatBitSize != 8 && SplatBitSize != 16 && SplatBitSize != 32 &&
        SplatBitSize != 64)
      return SDValue();

    // If the value isn't an integer type we will have to bitcast
    // from an integer type first. Also, if there are any undefs, we must
    // lower them to defined values first.
    if (ResTy.isInteger() && !HasAnyUndefs) {
      return DAG.getNode(Sw64ISD::VBROADCAST, dl, ResTy, Op.getOperand(1));
    }

    EVT ViaVecTy;

    switch (SplatBitSize) {
    default:
      return SDValue();
    case 8:
      ViaVecTy = MVT::v32i8;
      break;
    case 16:
      ViaVecTy = MVT::v16i16;
      break;
    case 32:
      ViaVecTy = MVT::v8i32;
      break;
    case 64:
      ViaVecTy = MVT::v4i64;
      break;
    }

    // SelectionDAG::getConstant will promote SplatValue appropriately.
    SDValue Result = DAG.getConstant(SplatValue, DL, ViaVecTy);

    // Bitcast to the type we originally wanted
    if (ViaVecTy != ResTy)
      Result = DAG.getNode(ISD::BITCAST, dl, ResTy, Result);

    return Result;
  } else if (DAG.isSplatValue(Op, /* AllowUndefs */ false)) {
    return DAG.getNode(Sw64ISD::VBROADCAST, dl, ResTy, Op.getOperand(1));
  } else if (!isConstantOrUndefBUILD_VECTOR(Node)) {
    // Use INSERT_VECTOR_ELT operations rather than expand to stores.
    // The resulting code is the same length as the expansion, but it doesn't
    // use memory operations
    EVT ResTy = Node->getValueType(0);

    assert(ResTy.isVector());

    unsigned NumElts = ResTy.getVectorNumElements();
    SDValue Vector = DAG.getUNDEF(ResTy);
    for (unsigned i = 0; i < NumElts; ++i) {
      Vector =
          DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, ResTy, Vector,
                      Node->getOperand(i), DAG.getConstant(i, DL, MVT::i64));
    }
    return Vector;
  }

  return SDValue();
}

SDValue Sw64TargetLowering::LowerSTORE(SDValue Op, SelectionDAG &DAG) const {
  StoreSDNode &Nd = *cast<StoreSDNode>(Op);

  if (Nd.getMemoryVT() != MVT::v4i32)
    return Op;

  // Replace a v4i64 with v4i32 stores.
  SDLoc DL(Op);

  SDValue Val = Op->getOperand(1);

  return DAG.getMemIntrinsicNode(Sw64ISD::VTRUNCST, DL,
                                 DAG.getVTList(MVT::Other),
                                 {Nd.getChain(), Val, Nd.getBasePtr()},
                                 Nd.getMemoryVT(), Nd.getMemOperand());
}

SDValue Sw64TargetLowering::LowerSETCC(SDValue Op, SelectionDAG &DAG) const {
  // Sw64 Produce not generic v4i64 setcc result, but v4f64/f32 result 2.0
  // Need to use addition compare to reverse the result.
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();
  SDLoc DL(Op);
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);

  // TODO: Trunc v4i64 Compare to v4f64
  // Sw64 Doesn't have v4i64 compare. Due to LLVM speciality, all comparisons
  // will be process as Ingeter, like Vector-64bits compare reults is v4i64.
  // So we have to do it for now.
  if (LHS.getValueType() == MVT::v4i64 && RHS.getValueType() == MVT::v4i64) {
    return SDValue();
  }

  if (CC != ISD::SETO)
    return Op;

  SDValue Res = DAG.getSetCC(DL, MVT::v4i64, Op.getOperand(0), Op.getOperand(1),
                             ISD::SETUO);
  SDValue Zero = DAG.getRegister(Sw64::V31, MVT::v4f64);
  SDValue Cast = DAG.getNode(ISD::BITCAST, DL, MVT::v4f64, Res);
  return DAG.getSetCC(DL, MVT::v4i64, Cast, Zero, ISD::SETOEQ);
}

SDValue Sw64TargetLowering::LowerSHL_PARTS(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc DL(Op);
  MVT VT = MVT::i64;

  SDValue Lo = Op.getOperand(0), Hi = Op.getOperand(1);
  SDValue Shamt = Op.getOperand(2);
  // if shamt < (VT.bits):
  //  lo = (shl lo, shamt)
  //  hi = (or (shl hi, shamt) (srl (srl lo, 1), (xor shamt, (VT.bits-1)))
  // else:
  //  lo = 0
  //  hi = (shl lo, shamt[4:0])
  SDValue Not =
      DAG.getNode(ISD::XOR, DL, MVT::i64, Shamt,
                  DAG.getConstant(VT.getSizeInBits() - 1, DL, MVT::i64));
  SDValue ShiftRight1Lo =
      DAG.getNode(ISD::SRL, DL, VT, Lo, DAG.getConstant(1, DL, VT));
  SDValue ShiftRightLo = DAG.getNode(ISD::SRL, DL, VT, ShiftRight1Lo, Not);
  SDValue ShiftLeftHi = DAG.getNode(ISD::SHL, DL, VT, Hi, Shamt);
  SDValue Or = DAG.getNode(ISD::OR, DL, VT, ShiftLeftHi, ShiftRightLo);
  SDValue ShiftLeftLo = DAG.getNode(ISD::SHL, DL, VT, Lo, Shamt);
  SDValue Cond = DAG.getNode(ISD::AND, DL, MVT::i64, Shamt,
                             DAG.getConstant(VT.getSizeInBits(), DL, MVT::i64));
  Lo = DAG.getNode(ISD::SELECT, DL, VT, Cond, DAG.getConstant(0, DL, VT),
                   ShiftLeftLo);
  Hi = DAG.getNode(ISD::SELECT, DL, VT, Cond, ShiftLeftLo, Or);

  SDValue Ops[2] = {Lo, Hi};
  return DAG.getMergeValues(Ops, DL);
}

SDValue Sw64TargetLowering::LowerSRL_PARTS(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc dl(Op);
  SDValue ShOpLo = Op.getOperand(0);
  SDValue ShOpHi = Op.getOperand(1);
  SDValue ShAmt = Op.getOperand(2);
  SDValue bm = DAG.getNode(ISD::SUB, dl, MVT::i64,
                           DAG.getConstant(64, dl, MVT::i64), ShAmt);
  SDValue BMCC = DAG.getSetCC(dl, MVT::i64, bm,
                              DAG.getConstant(0, dl, MVT::i64), ISD::SETLE);
  // if 64 - shAmt <= 0
  SDValue Hi_Neg = DAG.getConstant(0, dl, MVT::i64);
  SDValue ShAmt_Neg =
      DAG.getNode(ISD::SUB, dl, MVT::i64, DAG.getConstant(0, dl, MVT::i64), bm);
  SDValue Lo_Neg = DAG.getNode(ISD::SRL, dl, MVT::i64, ShOpHi, ShAmt_Neg);
  // else
  SDValue carries = DAG.getNode(ISD::SHL, dl, MVT::i64, ShOpHi, bm);
  SDValue Hi_Pos = DAG.getNode(ISD::SRL, dl, MVT::i64, ShOpHi, ShAmt);
  SDValue Lo_Pos = DAG.getNode(ISD::SRL, dl, MVT::i64, ShOpLo, ShAmt);
  Lo_Pos = DAG.getNode(ISD::OR, dl, MVT::i64, Lo_Pos, carries);
  // Merge
  SDValue Hit = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC, Hi_Neg, Hi_Pos);
  SDValue Lot = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC, Lo_Neg, Lo_Pos);
  SDValue BMCC1 = DAG.getSetCC(dl, MVT::i64, ShAmt,
                               DAG.getConstant(0, dl, MVT::i64), ISD::SETEQ);
  SDValue BMCC2 = DAG.getSetCC(dl, MVT::i64, ShAmt,
                               DAG.getConstant(64, dl, MVT::i64), ISD::SETEQ);
  SDValue Hit1 = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC1, ShOpHi, Hit);
  SDValue Lot1 = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC1, ShOpLo, Lot);
  SDValue Hi = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC2,
                           DAG.getConstant(0, dl, MVT::i64), Hit1);
  SDValue Lo = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC2, ShOpHi, Lot1);

  SDValue Ops[2] = {Lo, Hi};
  return DAG.getMergeValues(Ops, dl);
}

SDValue Sw64TargetLowering::LowerSRA_PARTS(SDValue Op,
                                           SelectionDAG &DAG) const {
  EVT VT = Op.getValueType();
  unsigned VTBits = VT.getSizeInBits();
  SDLoc dl(Op);
  SDValue ShOpLo = Op.getOperand(0);
  SDValue ShOpHi = Op.getOperand(1);
  SDValue ShAmt = Op.getOperand(2);
  SDValue bm = DAG.getNode(ISD::SUB, dl, MVT::i64,
                           DAG.getConstant(64, dl, MVT::i64), ShAmt);
  SDValue BMCC = DAG.getSetCC(dl, MVT::i64, bm,
                              DAG.getConstant(0, dl, MVT::i64), ISD::SETLE);
  // if 64 - shAmt <= 0
  SDValue Hi_Neg = DAG.getNode(ISD::SRA, dl, VT, ShOpHi,
                               DAG.getConstant(VTBits - 1, dl, MVT::i64));
  SDValue ShAmt_Neg =
      DAG.getNode(ISD::SUB, dl, MVT::i64, DAG.getConstant(0, dl, MVT::i64), bm);
  SDValue Lo_Neg = DAG.getNode(ISD::SRA, dl, MVT::i64, ShOpHi, ShAmt_Neg);
  // else
  SDValue carries = DAG.getNode(ISD::SHL, dl, MVT::i64, ShOpHi, bm);
  SDValue Hi_Pos = DAG.getNode(ISD::SRA, dl, MVT::i64, ShOpHi, ShAmt);
  SDValue Lo_Pos = DAG.getNode(ISD::SRL, dl, MVT::i64, ShOpLo, ShAmt);
  Lo_Pos = DAG.getNode(ISD::OR, dl, MVT::i64, Lo_Pos, carries);
  // Merge
  SDValue Hit = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC, Hi_Neg, Hi_Pos);
  SDValue Lot = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC, Lo_Neg, Lo_Pos);
  SDValue BMCC1 = DAG.getSetCC(dl, MVT::i64, ShAmt,
                               DAG.getConstant(0, dl, MVT::i64), ISD::SETEQ);
  SDValue BMCC2 = DAG.getSetCC(dl, MVT::i64, ShAmt,
                               DAG.getConstant(64, dl, MVT::i64), ISD::SETEQ);
  SDValue Hit1 = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC1, ShOpHi, Hit);
  SDValue Lot1 = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC1, ShOpLo, Lot);
  SDValue Hi = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC2,
                           DAG.getNode(ISD::SRA, dl, MVT::i64, ShOpHi,
                                       DAG.getConstant(63, dl, MVT::i64)),
                           Hit1);
  SDValue Lo = DAG.getNode(ISD::SELECT, dl, MVT::i64, BMCC2, ShOpHi, Lot1);
  SDValue Ops[2] = {Lo, Hi};
  return DAG.getMergeValues(Ops, dl);
}

SDValue Sw64TargetLowering::LowerSINT_TO_FP(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc dl(Op);
  assert(Op.getOperand(0).getValueType() == MVT::i64 &&
         "Unhandled SINT_TO_FP type in custom expander!");
  SDValue LD;
  bool isDouble = Op.getValueType() == MVT::f64;
  LD = DAG.getNode(ISD::BITCAST, dl, MVT::f64, Op.getOperand(0));
  SDValue FP = DAG.getNode(isDouble ? Sw64ISD::CVTQT_ : Sw64ISD::CVTQS_, dl,
                           isDouble ? MVT::f64 : MVT::f32, LD);
  return FP;
}

SDValue Sw64TargetLowering::LowerFP_TO_SINT(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc dl(Op);
  bool isDouble = Op.getOperand(0).getValueType() == MVT::f64;
  SDValue src = Op.getOperand(0);

  if (!isDouble) // Promote
    src = DAG.getNode(ISD::FP_EXTEND, dl, MVT::f64, src);

  src = DAG.getNode(Sw64ISD::CVTTQ_, dl, MVT::f64, src);

  return DAG.getNode(ISD::BITCAST, dl, MVT::i64, src);
}

SDValue Sw64TargetLowering::LowerFP_TO_INT_SAT(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDValue width = Op.getOperand(1);

  if (width.getValueType() != MVT::i64)
    width = DAG.getNode(ISD::ZERO_EXTEND, SDLoc(Op), MVT::i64, width);

  return expandFP_TO_INT_SAT(Op.getNode(), DAG);
}

// ----------------------------------------------------------
// For cnstruct a new chain call to libgcc to replace old chain
// from udiv/sidv i128 , i128 to call %sret, i128 ,i128
//
// ----------------------------------------------------------
SDValue Sw64TargetLowering::LowerSUDIVI128(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc dl(Op);

  if (!Op.getValueType().isInteger())
    return SDValue();
  RTLIB::Libcall LC;
  bool isSigned;
  switch (Op->getOpcode()) {
  default:
    llvm_unreachable("Unexpected request for libcall!");
  case ISD::SDIV:
    isSigned = true;
    LC = RTLIB::SDIV_I128;
    break;
  case ISD::UDIV:
    isSigned = false;
    LC = RTLIB::UDIV_I128;
    break;
  case ISD::SREM:
    isSigned = true;
    LC = RTLIB::SREM_I128;
    break;
  case ISD::UREM:
    isSigned = false;
    LC = RTLIB::UREM_I128;
    break;
  }
  SDValue InChain = DAG.getEntryNode();

  // Create a extra stack objdect to store libcall result
  SDValue DemoteStackSlot;
  TargetLowering::ArgListTy Args;
  auto &DL = DAG.getDataLayout();
  uint64_t TySize = 16;
  MachineFunction &MF = DAG.getMachineFunction();
  int DemoteStackIdx =
      MF.getFrameInfo().CreateStackObject(TySize, Align(8), false);
  EVT ArgVT = Op->getOperand(0).getValueType();
  Type *ArgTy = ArgVT.getTypeForEVT(*DAG.getContext());
  Type *StackSlotPtrType = PointerType::get(ArgTy, DL.getAllocaAddrSpace());
  // save the sret infomation
  DemoteStackSlot = DAG.getFrameIndex(DemoteStackIdx, getFrameIndexTy(DL));
  ArgListEntry Entry;
  Entry.Node = DemoteStackSlot;
  Entry.Ty = StackSlotPtrType;
  Entry.IsSRet = true;
  Entry.Alignment = Align(8);
  Args.push_back(Entry);

  // passing udiv/sdiv operands argument
  for (unsigned i = 0, e = Op->getNumOperands(); i != e; ++i) {
    ArgListEntry Entry;
    ArgVT = Op->getOperand(i).getValueType();
    assert(ArgVT.isInteger() && ArgVT.getSizeInBits() == 128 &&
           "Unexpected argument type for lowering");
    Entry.Node = Op->getOperand(i);
    Entry.Ty = IntegerType::get(*DAG.getContext(), 128);
    Entry.IsInReg = true;
    Entry.IsSExt = isSigned;
    Entry.IsZExt = false;
    Args.push_back(Entry);
  }

  SDValue Callee = DAG.getExternalSymbol(getLibcallName(LC),
                                         getPointerTy(DAG.getDataLayout()));
  // create a new libcall to producess udiv/sdiv
  TargetLowering::CallLoweringInfo CLI(DAG);
  CLI.setDebugLoc(dl)
      .setChain(InChain)
      .setLibCallee(
          getLibcallCallingConv(LC),
          static_cast<EVT>(MVT::isVoid).getTypeForEVT(*DAG.getContext()),
          Callee, std::move(Args))
      .setNoReturn(true)
      .setSExtResult(isSigned)
      .setZExtResult(!isSigned);

  SDValue CallInfo = LowerCallTo(CLI).second;
  return LowerCallExtraResult(CallInfo, DemoteStackSlot, DemoteStackIdx, DAG)
      .first;
}

// --------------------------------------------------------------------
// when a call using sret arugments pass in register, the call result
// must be handled, create a load node and tokenfactor to pass the call
// result
// --------------------------------------------------------------------
std::pair<SDValue, SDValue> Sw64TargetLowering::LowerCallExtraResult(
    SDValue &Chain, SDValue &DemoteStackSlot, unsigned DemoteStackIdx,
    SelectionDAG &DAG) const {
  SmallVector<SDValue, 4> Chains(1), ReturnValues(1);
  SDLoc DL(Chain);
  SDNodeFlags Flags;
  Flags.setNoUnsignedWrap(true);
  SDValue Add = DAG.getNode(ISD::ADD, DL, MVT::i64, DemoteStackSlot,
                            DAG.getConstant(0, DL, MVT::i64), Flags);
  SDValue L = DAG.getLoad(MVT::i128, DL, Chain, Add,
                          MachinePointerInfo::getFixedStack(
                              DAG.getMachineFunction(), DemoteStackIdx, 0),
                          /* Alignment = */ 8);
  Chains[0] = L.getValue(1);
  ReturnValues[0] = L;
  Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, Chains);

  SDValue Res = DAG.getNode(ISD::MERGE_VALUES, DL, DAG.getVTList(MVT::i128),
                            ReturnValues);
  return std::make_pair(Res, Chain);
}

SDValue Sw64TargetLowering::LowerExternalSymbol(SDValue Op,
                                                SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "Sw64:: begin lowExternalSymbol----\n");
  SDLoc dl(Op);
  return DAG.getNode(Sw64ISD::RelLit, dl, MVT::i64,
                     DAG.getTargetExternalSymbol(
                         cast<ExternalSymbolSDNode>(Op)->getSymbol(), MVT::i64),
                     DAG.getGLOBAL_OFFSET_TABLE(MVT::i64));
}

SDValue Sw64TargetLowering::LowerATOMIC_FENCE(SDValue Op,
                                              SelectionDAG &DAG) const {
  // FIXME: Need pseudo-fence for 'singlethread' fences
  // FIXME: Set SType for weaker fences where supported/appropriate.
  SDLoc DL(Op);
  return DAG.getNode(Sw64ISD::MEMBARRIER, DL, MVT::Other, Op.getOperand(0));
}

SDValue Sw64TargetLowering::LowerATOMIC_LOAD(SDValue Op,
                                             SelectionDAG &DAG) const {
  AtomicSDNode *N = cast<AtomicSDNode>(Op);
  assert(N->getOpcode() == ISD::ATOMIC_LOAD && "Bad Atomic OP");
  assert((N->getSuccessOrdering() == AtomicOrdering::Unordered ||
          N->getSuccessOrdering() == AtomicOrdering::Monotonic) &&
         "setInsertFencesForAtomic(true) expects unordered / monotonic");
  EVT VT = N->getMemoryVT();
  SDValue Result;
  if (VT != MVT::i64)
    Result =
        DAG.getExtLoad(ISD::SEXTLOAD, SDLoc(Op), MVT::i64, N->getChain(),
                       N->getBasePtr(), N->getPointerInfo(), VT, N->getAlign(),
                       N->getMemOperand()->getFlags(), N->getAAInfo());
  else
    Result = DAG.getLoad(MVT::i64, SDLoc(Op), N->getChain(), N->getBasePtr(),
                         N->getPointerInfo(), N->getAlign(),
                         N->getMemOperand()->getFlags(), N->getAAInfo(),
                         N->getRanges());
  return Result;
}

SDValue Sw64TargetLowering::LowerATOMIC_STORE(SDValue Op,
                                              SelectionDAG &DAG) const {
  AtomicSDNode *N = cast<AtomicSDNode>(Op);
  assert(N->getOpcode() == ISD::ATOMIC_STORE && "Bad Atomic OP");
  assert((N->getSuccessOrdering() == AtomicOrdering::Unordered ||
          N->getSuccessOrdering() == AtomicOrdering::Monotonic) &&
         "setInsertFencesForAtomic(true) expects unordered / monotonic");

  return DAG.getStore(N->getChain(), SDLoc(Op), N->getVal(), N->getBasePtr(),
                      N->getPointerInfo(), N->getAlign(),
                      N->getMemOperand()->getFlags(), N->getAAInfo());
}
MachineMemOperand::Flags
Sw64TargetLowering::getTargetMMOFlags(const Instruction &I) const {
  // Because of how we convert atomic_load and atomic_store to normal loads and
  // stores in the DAG, we need to ensure that the MMOs are marked volatile
  // since DAGCombine hasn't been updated to account for atomic, but non
  // volatile loads.  (See D57601)
  if (auto *SI = dyn_cast<StoreInst>(&I))
    if (SI->isAtomic())
      return MachineMemOperand::MOVolatile;
  if (auto *LI = dyn_cast<LoadInst>(&I))
    if (LI->isAtomic())
      return MachineMemOperand::MOVolatile;
  if (auto *AI = dyn_cast<AtomicRMWInst>(&I))
    if (AI->isAtomic())
      return MachineMemOperand::MOVolatile;
  if (auto *AI = dyn_cast<AtomicCmpXchgInst>(&I))
    if (AI->isAtomic())
      return MachineMemOperand::MOVolatile;
  return MachineMemOperand::MONone;
}

SDValue Sw64TargetLowering::LowerOR(SDValue Op, SelectionDAG &DAG) const {
  SDValue N0 = Op->getOperand(0);
  SDValue N1 = Op->getOperand(1);
  EVT VT = N1.getValueType();
  SDLoc dl(Op);
  if (auto *C1 = dyn_cast<ConstantSDNode>(N1)) {
    const APInt &C1Val = C1->getAPIntValue();
    if (C1Val.isPowerOf2()) {
      SDValue ShAmtC = DAG.getConstant(C1Val.exactLogBase2(), dl, VT);
      return DAG.getNode(Sw64ISD::SBT, dl, VT, N0, ShAmtC);
    }
  }
  // if ((or (srl shl)) || (or (shl srl)) then rolw
  if ((N0->getOpcode() == ISD::SRL && N1->getOpcode() == ISD::SRL) ||
      (N0->getOpcode() == ISD::SRL && N1->getOpcode() == ISD::SHL))
    if (N0->getOperand(1)->getOperand(0)->getOpcode() == ISD::SUB &&
        N0->getOperand(1)->getOperand(0)->getConstantOperandVal(0) == 32)
      return DAG.getNode(Sw64ISD::ROLW, dl, VT, N1->getOperand(0),
                         N1->getOperand(1)->getOperand(0));
  return SDValue();
}

SDValue Sw64TargetLowering::LowerSUREM(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op);
  // Expand only on constant case
  // modify the operate of div 0
  if (Op.getOperand(1).getOpcode() == ISD::Constant &&
      cast<ConstantSDNode>(Op.getNode()->getOperand(1))->getAPIntValue() != 0) {

    EVT VT = Op.getNode()->getValueType(0);

    SmallVector<SDNode *, 8> Built;
    SDValue Tmp1 = Op.getNode()->getOpcode() == ISD::UREM
                       ? BuildUDIV(Op.getNode(), DAG, false, Built)
                       : BuildSDIV(Op.getNode(), DAG, false, Built);

    Tmp1 = DAG.getNode(ISD::MUL, dl, VT, Tmp1, Op.getOperand(1));
    Tmp1 = DAG.getNode(ISD::SUB, dl, VT, Op.getOperand(0), Tmp1);

    return Tmp1;
  }

  return LowerSUDIV(Op, DAG);
}

SDValue Sw64TargetLowering::LowerSUDIV(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op);

  if (!Op.getValueType().isInteger())
    return SDValue();

  // modify the operate of div 0
  if (Op.getOperand(1).getOpcode() == ISD::Constant &&
      cast<ConstantSDNode>(Op.getNode()->getOperand(1))->getAPIntValue() != 0) {
    SmallVector<SDNode *, 8> Built;
    return Op.getOpcode() == ISD::SDIV
               ? BuildSDIV(Op.getNode(), DAG, true, Built)
               : BuildUDIV(Op.getNode(), DAG, true, Built);
  }

  const char *opstr = 0;
  switch (Op.getOpcode()) {
  case ISD::UREM:
    opstr = "__remlu";
    break;
  case ISD::SREM:
    opstr = "__reml";
    break;
  case ISD::UDIV:
    opstr = "__divlu";
    break;
  case ISD::SDIV:
    opstr = "__divl";
    break;
  }

  SDValue Tmp1 = Op.getOperand(0);
  SDValue Tmp2 = Op.getOperand(1);
  SDValue Addr = DAG.getExternalSymbol(opstr, MVT::i64);
  return DAG.getNode(Sw64ISD::DivCall, dl, MVT::i64, Addr, Tmp1, Tmp2);
}

SDValue Sw64TargetLowering::LowerVAARG(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op);
  SDValue Chain, DataPtr;
  LowerVAARG(Op.getNode(), Chain, DataPtr, DAG);
  SDValue Result;
  if (Op.getValueType() == MVT::i32)
    Result = DAG.getExtLoad(ISD::SEXTLOAD, dl, MVT::i64, Chain, DataPtr,
                            MachinePointerInfo(), MVT::i32);
  else if (Op.getValueType() == MVT::f32) {
    Result = DAG.getLoad(MVT::f64, dl, Chain, DataPtr, MachinePointerInfo());
    SDValue InFlags = Result.getValue(1);
    SmallVector<SDValue, 8> Ops;
    Ops.push_back(InFlags);
    Ops.push_back(Result);
    SDVTList NodeTys = DAG.getVTList(MVT::f32, MVT::Other);
    Result = DAG.getNode(Sw64ISD::CVTTS_, dl, NodeTys, Ops);
  } else {
    Result = DAG.getLoad(Op.getValueType(), dl, Chain, DataPtr,
                         MachinePointerInfo());
  }
  return Result;
}

SDValue Sw64TargetLowering::LowerVACOPY(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op);
  SDValue Chain = Op.getOperand(0);
  SDValue DestP = Op.getOperand(1);
  SDValue SrcP = Op.getOperand(2);
  const Value *DestS = cast<SrcValueSDNode>(Op.getOperand(3))->getValue();
  const Value *SrcS = cast<SrcValueSDNode>(Op.getOperand(4))->getValue();
  SDValue Val = DAG.getLoad(getPointerTy(DAG.getDataLayout()), dl, Chain, SrcP,
                            MachinePointerInfo(SrcS));
  SDValue Result =
      DAG.getStore(Val.getValue(1), dl, Val, DestP, MachinePointerInfo(DestS));
  SDValue NP = DAG.getNode(ISD::ADD, dl, MVT::i64, SrcP,
                           DAG.getConstant(8, dl, MVT::i64));
  Val = DAG.getExtLoad(ISD::SEXTLOAD, dl, MVT::i64, Result, NP,
                       MachinePointerInfo(), MVT::i32);
  SDValue NPD = DAG.getNode(ISD::ADD, dl, MVT::i64, DestP,
                            DAG.getConstant(8, dl, MVT::i64));
  return DAG.getTruncStore(Val.getValue(1), dl, Val, NPD, MachinePointerInfo(),
                           MVT::i32);
}

SDValue Sw64TargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op);
  MachineFunction &MF = DAG.getMachineFunction();
  Sw64MachineFunctionInfo *FuncInfo = MF.getInfo<Sw64MachineFunctionInfo>();

  SDValue Chain = Op.getOperand(0);
  SDValue VAListP = Op.getOperand(1);
  const Value *VAListS = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  // vastart stores the address of the VarArgsBase and VarArgsOffset
  SDValue FR = DAG.getFrameIndex(FuncInfo->getVarArgsBase(), MVT::i64);
  SDValue S1 =
      DAG.getStore(Chain, dl, FR, VAListP, MachinePointerInfo(VAListS));
  SDValue SA2 = DAG.getNode(ISD::ADD, dl, MVT::i64, VAListP,
                            DAG.getConstant(8, dl, MVT::i64));

  return DAG.getTruncStore(
      S1, dl, DAG.getConstant(FuncInfo->getVarArgsOffset(), dl, MVT::i64), SA2,
      MachinePointerInfo(), MVT::i32);
}

// Prefetch operands are:
// 1: Address to prefetch
// 2: bool isWrite
// 3: int locality (0 = no locality ... 3 = extreme locality)
// 4: bool isDataCache
SDValue Sw64TargetLowering::LowerPREFETCH(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  unsigned IsWrite = cast<ConstantSDNode>(Op.getOperand(2))->getZExtValue();
  // unsigned Locality = cast<ConstantSDNode>(Op.getOperand(3))->getZExtValue();
  unsigned IsData = cast<ConstantSDNode>(Op.getOperand(4))->getZExtValue();

  unsigned Code = IsData ? Sw64ISD::Z_S_FILLCS : Sw64ISD::Z_FILLCS;
  if (IsWrite == 1 && IsData == 1)
    Code = Sw64ISD::Z_FILLDE;
  if (IsWrite == 0 && IsData == 1)
    Code = Sw64ISD::Z_FILLCS;
  if (IsWrite == 1 && IsData == 0)
    Code = Sw64ISD::Z_S_FILLDE;
  if (IsWrite == 0 && IsData == 0)
    Code = Sw64ISD::Z_FILLCS;

  unsigned PrfOp = 0;

  return DAG.getNode(Code, DL, MVT::Other, Op.getOperand(0),
                     DAG.getConstant(PrfOp, DL, MVT::i64), Op.getOperand(1));
}

SDValue Sw64TargetLowering::LowerROLW(SDNode *N, SelectionDAG &DAG) const {
  SDLoc DL(N);

  SDValue NewOp0 = DAG.getNode(ISD::ANY_EXTEND, DL, MVT::i64, N->getOperand(0));
  SDValue NewOp1 = DAG.getNode(ISD::ANY_EXTEND, DL, MVT::i64, N->getOperand(1));
  SDValue NewRes = DAG.getNode(Sw64ISD::ROLW, DL, MVT::i64, NewOp0, NewOp1);
  return DAG.getNode(ISD::TRUNCATE, DL, N->getValueType(0), NewRes);
}

SDValue Sw64TargetLowering::LowerFRAMEADDR(SDValue Op,
                                           SelectionDAG &DAG) const {
  // check the depth
  if (cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue() != 0) {
    DAG.getContext()->emitError(
        "return address can be determined only for current frame");
    return SDValue();
  }

  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  MFI.setFrameAddressIsTaken(true);
  EVT VT = Op.getValueType();
  SDLoc DL(Op);
  SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), DL, Sw64::R15, VT);
  return FrameAddr;
}

void Sw64TargetLowering::ReplaceNodeResults(SDNode *N,
                                            SmallVectorImpl<SDValue> &Results,
                                            SelectionDAG &DAG) const {
  SDLoc dl(N);
  switch (N->getOpcode()) {
  default:
    break;
  case ISD::SDIV:
  case ISD::UDIV:
  case ISD::SREM:
  case ISD::UREM: {
    SDValue Res = LowerSUDIVI128(SDValue(N, 0), DAG);
    Results.push_back(Res);
    return;
  }
  case ISD::ATOMIC_LOAD:
  case ISD::ATOMIC_STORE:
  case ISD::FP_TO_SINT_SAT:
  case ISD::FP_TO_UINT_SAT:
    return;
  case ISD::FP_TO_SINT: {
    SDValue NewRes =
        DAG.getNode(ISD::FP_TO_SINT, dl, MVT::i64, N->getOperand(0));
    Results.push_back(
        DAG.getNode(ISD::TRUNCATE, dl, N->getValueType(0), NewRes));
    return;
  }
  case ISD::ROTL:
    SDValue Res = LowerROLW(N, DAG);
    Results.push_back(Res);
    return;
  }
  assert(N->getValueType(0) == MVT::i32 && N->getOpcode() == ISD::VAARG &&
         "Unknown node to custom promote!");

  SDValue Chain, DataPtr;
  LowerVAARG(N, Chain, DataPtr, DAG);

  SDValue Res =
      DAG.getLoad(N->getValueType(0), dl, Chain, DataPtr, MachinePointerInfo());

  Results.push_back(Res);
  Results.push_back(SDValue(Res.getNode(), 1));
}

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
Sw64TargetLowering::ConstraintType
Sw64TargetLowering::getConstraintType(const std::string &Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'f':
    case 'r':
      return C_RegisterClass;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

unsigned Sw64TargetLowering::MatchRegName(StringRef Name) const {
  unsigned Reg = StringSwitch<unsigned>(Name.lower())
                     .Case("$0", Sw64::R0)
                     .Case("$1", Sw64::R1)
                     .Case("$2", Sw64::R2)
                     .Case("$3", Sw64::R3)
                     .Case("$4", Sw64::R4)
                     .Case("$5", Sw64::R5)
                     .Case("$6", Sw64::R6)
                     .Case("$7", Sw64::R7)
                     .Case("$8", Sw64::R8)
                     .Case("$9", Sw64::R9)
                     .Case("$10", Sw64::R10)
                     .Case("$11", Sw64::R11)
                     .Case("$12", Sw64::R12)
                     .Case("$13", Sw64::R13)
                     .Case("$14", Sw64::R14)
                     .Case("$15", Sw64::R15)
                     .Case("$16", Sw64::R16)
                     .Case("$17", Sw64::R17)
                     .Case("$18", Sw64::R18)
                     .Case("$19", Sw64::R19)
                     .Case("$20", Sw64::R20)
                     .Case("$21", Sw64::R21)
                     .Case("$22", Sw64::R22)
                     .Case("$23", Sw64::R23)
                     .Case("$24", Sw64::R24)
                     .Case("$25", Sw64::R25)
                     .Case("$26", Sw64::R26)
                     .Case("$27", Sw64::R27)
                     .Case("$28", Sw64::R28)
                     .Case("$29", Sw64::R29)
                     .Case("$30", Sw64::R30)
                     .Case("$31", Sw64::R31)
                     .Default(0);
  return Reg;
}
Register
Sw64TargetLowering::getRegisterByName(const char *RegName, LLT VT,
                                      const MachineFunction &MF) const {
  Register Reg = MatchRegName(StringRef(RegName));
  if (Reg)
    return Reg;

  report_fatal_error("Sw Invalid register name global variable");
}
/// Examine constraint type and operand type and determine a weight value.
/// This object must already have been set up with the operand type
/// and the current alternative constraint selected.
TargetLowering::ConstraintWeight
Sw64TargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &info, const char *constraint) const {
  ConstraintWeight weight = CW_Invalid;
  Value *CallOperandVal = info.CallOperandVal;
  // If we don't have a value, we can't do a match,
  // but allow it at the lowest weight.
  if (CallOperandVal == NULL)
    return CW_Default;
  // Look at the constraint type.
  switch (*constraint) {
  default:
    weight = TargetLowering::getSingleConstraintMatchWeight(info, constraint);
    break;
  case 'f':
    weight = CW_Register;
    break;
  }
  return weight;
}

Instruction *Sw64TargetLowering::emitLeadingFence(IRBuilderBase &Builder,
                                                  Instruction *Inst,
                                                  AtomicOrdering Ord) const {
  if (isa<LoadInst>(Inst) && Ord == AtomicOrdering::SequentiallyConsistent)
    return Builder.CreateFence(AtomicOrdering::AcquireRelease);
  if (isa<StoreInst>(Inst) && isReleaseOrStronger(Ord))
    return Builder.CreateFence(AtomicOrdering::Release);
  return nullptr;
}

Instruction *Sw64TargetLowering::emitTrailingFence(IRBuilderBase &Builder,
                                                   Instruction *Inst,
                                                   AtomicOrdering Ord) const {
  if (isa<LoadInst>(Inst) && isAcquireOrStronger(Ord))
    return Builder.CreateFence(AtomicOrdering::AcquireRelease);
  if (isa<StoreInst>(Inst) && Ord == AtomicOrdering::SequentiallyConsistent)
    return Builder.CreateFence(AtomicOrdering::Release);
  return nullptr;
}

/// This is a helper function to parse a physical register string and split it
/// into non-numeric and numeric parts (Prefix and Reg). The first boolean flag
/// that is returned indicates whether parsing was successful. The second flag
/// is true if the numeric part exists.
static std::pair<bool, bool> parsePhysicalReg(StringRef C, StringRef &Prefix,
                                              unsigned long long &Reg) {
  if (C.front() != '{' || C.back() != '}')
    return std::make_pair(false, false);

  // Search for the first numeric character.
  StringRef::const_iterator I, B = C.begin() + 1, E = C.end() - 1;
  I = std::find_if(B, E, isdigit);

  Prefix = StringRef(B, I - B);

  // The second flag is set to false if no numeric characters were found.
  if (I == E)
    return std::make_pair(true, false);

  // Parse the numeric characters.
  return std::make_pair(!getAsUnsignedInteger(StringRef(I, E - I), 10, Reg),
                        true);
}

std::pair<unsigned, const TargetRegisterClass *>
Sw64TargetLowering::parseRegForInlineAsmConstraint(StringRef C, MVT VT) const {
  const TargetRegisterClass *RC;
  StringRef Prefix;
  unsigned long long Reg;

  std::pair<bool, bool> R = parsePhysicalReg(C, Prefix, Reg);

  if (!R.first)
    return std::make_pair(0U, nullptr);

  if (!R.second)
    return std::make_pair(0U, nullptr);

  if (Prefix == "$f") { // Parse $f0-$f31.
    // The size of FP registers is 64-bit or Reg is an even number, select
    // the 64-bit register class.
    if (VT == MVT::Other)
      VT = MVT::f64;

    RC = getRegClassFor(VT);

  } else { // Parse $0-$31.
    assert(Prefix == "$");
    // Sw64 has only i64 register.
    RC = getRegClassFor(MVT::i64);
    StringRef name((C.data() + 1), (C.size() - 2));

    return std::make_pair(MatchRegName(name), RC);
  }

  assert(Reg < RC->getNumRegs());
  return std::make_pair(*(RC->begin() + Reg), RC);
}
/// Given a register class constraint, like 'r', if this corresponds directly
/// to an LLVM register class, return a register of 0 and the register class
/// pointer.
std::pair<unsigned, const TargetRegisterClass *>
Sw64TargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                 StringRef Constraint,
                                                 MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      return std::make_pair(0U, &Sw64::GPRCRegClass);
    case 'f':
      return VT == MVT::f64 ? std::make_pair(0U, &Sw64::F8RCRegClass)
                            : std::make_pair(0U, &Sw64::F4RCRegClass);
    }
  }

  std::pair<unsigned, const TargetRegisterClass *> R;
  R = parseRegForInlineAsmConstraint(Constraint, VT);

  if (R.second)
    return R;

  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}

//===----------------------------------------------------------------------===//
//  Other Lowering Code
//===----------------------------------------------------------------------===//

MachineBasicBlock *
Sw64TargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                MachineBasicBlock *BB) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Unexpected instr type to insert");

  case Sw64::FILLCS:
  case Sw64::FILLDE:
  case Sw64::S_FILLCS:
  case Sw64::S_FILLDE:
    return emitPrefetch(MI, BB);

  // I64 && I32
  case Sw64::ATOMIC_LOAD_ADD_I32:
  case Sw64::LAS32:
    return emitAtomicBinary(MI, BB);
  case Sw64::ATOMIC_LOAD_ADD_I64:
  case Sw64::LAS64:
    return emitAtomicBinary(MI, BB);

  case Sw64::ATOMIC_SWAP_I32:
  case Sw64::SWAP32:
    return emitAtomicBinary(MI, BB);
  case Sw64::ATOMIC_SWAP_I64:
  case Sw64::SWAP64:
    return emitAtomicBinary(MI, BB);
  case Sw64::ATOMIC_CMP_SWAP_I32:
  case Sw64::CAS32:
    return emitAtomicCmpSwap(MI, BB, 4);
  case Sw64::ATOMIC_CMP_SWAP_I64:
  case Sw64::CAS64:
    return emitAtomicCmpSwap(MI, BB, 8);

  case Sw64::ATOMIC_LOAD_AND_I32:
    return emitAtomicBinary(MI, BB);
  case Sw64::ATOMIC_LOAD_AND_I64:
    return emitAtomicBinary(MI, BB);

  case Sw64::ATOMIC_LOAD_OR_I32:
    return emitAtomicBinary(MI, BB);
  case Sw64::ATOMIC_LOAD_OR_I64:
    return emitAtomicBinary(MI, BB);

  case Sw64::ATOMIC_LOAD_SUB_I32:
    return emitAtomicBinary(MI, BB);
  case Sw64::ATOMIC_LOAD_SUB_I64:
    return emitAtomicBinary(MI, BB);

  case Sw64::ATOMIC_LOAD_XOR_I32:
    return emitAtomicBinary(MI, BB);
  case Sw64::ATOMIC_LOAD_XOR_I64:
    return emitAtomicBinary(MI, BB);

  case Sw64::ATOMIC_LOAD_UMAX_I64:
  case Sw64::ATOMIC_LOAD_MAX_I64:
  case Sw64::ATOMIC_LOAD_UMIN_I64:
  case Sw64::ATOMIC_LOAD_MIN_I64:
  case Sw64::ATOMIC_LOAD_NAND_I64:
    return emitAtomicBinary(MI, BB);

  case Sw64::ATOMIC_LOAD_UMAX_I32:
  case Sw64::ATOMIC_LOAD_MAX_I32:
  case Sw64::ATOMIC_LOAD_UMIN_I32:
  case Sw64::ATOMIC_LOAD_MIN_I32:
  case Sw64::ATOMIC_LOAD_NAND_I32:
    return emitAtomicBinary(MI, BB);

  case Sw64::ATOMIC_LOAD_UMAX_I16:
  case Sw64::ATOMIC_LOAD_MAX_I16:
  case Sw64::ATOMIC_LOAD_UMIN_I16:
  case Sw64::ATOMIC_LOAD_MIN_I16:
  case Sw64::ATOMIC_LOAD_NAND_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);

  case Sw64::ATOMIC_LOAD_UMAX_I8:
  case Sw64::ATOMIC_LOAD_MAX_I8:
  case Sw64::ATOMIC_LOAD_UMIN_I8:
  case Sw64::ATOMIC_LOAD_MIN_I8:
  case Sw64::ATOMIC_LOAD_NAND_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);

  // I8
  case Sw64::ATOMIC_LOAD_ADD_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Sw64::ATOMIC_SWAP_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Sw64::ATOMIC_LOAD_AND_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Sw64::ATOMIC_LOAD_OR_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Sw64::ATOMIC_LOAD_SUB_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Sw64::ATOMIC_LOAD_XOR_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Sw64::ATOMIC_CMP_SWAP_I8:
    return emitAtomicCmpSwapPartword(MI, BB, 1);

  // I16
  case Sw64::ATOMIC_LOAD_ADD_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Sw64::ATOMIC_SWAP_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Sw64::ATOMIC_LOAD_AND_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Sw64::ATOMIC_LOAD_OR_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Sw64::ATOMIC_LOAD_SUB_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Sw64::ATOMIC_LOAD_XOR_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Sw64::ATOMIC_CMP_SWAP_I16:
    return emitAtomicCmpSwapPartword(MI, BB, 2);
  }
}

MachineBasicBlock *
Sw64TargetLowering::emitPrefetch(MachineInstr &MI,
                                 MachineBasicBlock *BB) const {

  Register RA, RB, RC;
  MachineFunction *MF = BB->getParent();
  // MachineRegisterInfo &RegInfo = MF->getRegInfo();
  MachineRegisterInfo &MRI = MF->getRegInfo();

  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  MachineInstr *DefMI = MRI.getVRegDef(MI.getOperand(1).getReg());

  // %11:gprc = PHI %10:gprc, %bb.1, %15:gprc, %bb.4
  // FILLCS 128, %11:gprc
  // it should be directed return.
  if (!(DefMI->getOpcode() == Sw64::LDA && DefMI->getOperand(1).isImm()))
    return BB;

  int Imm = DefMI->getOperand(1).getImm();
  int Distance = Imm + MI.getOperand(0).getImm();
  Register Address = DefMI->getOperand(2).getReg();

  MachineInstr *MII = MI.getNextNode();
  if (MII)
    MII = MII->getNextNode();
  else
    return BB;

  if (MII) {
    if (MII->getOpcode() == Sw64::LDL || MII->getOpcode() == Sw64::LDW ||
        MII->getOpcode() == Sw64::LDHU || MII->getOpcode() == Sw64::LDBU) {
      int MIImm = MII->getOperand(1).getImm();
      if (MIImm > 1000 || MIImm < -1000) {
        MI.eraseFromParent();
        return BB;
      }
    }
  }

  if (Distance > 1500 || Distance < -1500) {
    MI.eraseFromParent(); // The pseudo instruction is gone now.
    return BB;
  }

  BuildMI(*BB, MI, DL, TII->get(MI.getOpcode()))
      .addImm(Distance)
      .addReg(Address);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return BB;
}

MachineBasicBlock *
Sw64TargetLowering::emitReduceSum(MachineInstr &MI,
                                  MachineBasicBlock *BB) const {

  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  Register RB = MI.getOperand(0).getReg();
  Register RA = MI.getOperand(1).getReg();

  Register RC = RegInfo.createVirtualRegister(&Sw64::F4RCRegClass);
  Register RD = RegInfo.createVirtualRegister(&Sw64::F4RCRegClass);
  Register RE = RegInfo.createVirtualRegister(&Sw64::GPRCRegClass);

  MachineBasicBlock::iterator II(MI);

  BuildMI(*BB, II, DL, TII->get(MI.getOpcode()))
      .addReg(RB, RegState::Define | RegState::EarlyClobber)
      .addReg(RA, RegState::Kill)
      .addReg(RC, RegState::Define | RegState::EarlyClobber |
                      RegState::Implicit | RegState::Dead)
      .addReg(RD, RegState::Define | RegState::EarlyClobber |
                      RegState::Implicit | RegState::Dead)
      .addReg(RE, RegState::Define | RegState::EarlyClobber |
                      RegState::Implicit | RegState::Dead);

  MI.eraseFromParent(); // The instruction is gone now.

  return BB;
}

MachineBasicBlock *
Sw64TargetLowering::emitITOFSInstruct(MachineInstr &MI,
                                      MachineBasicBlock *BB) const {
  return BB;
}

MachineBasicBlock *
Sw64TargetLowering::emitFSTOIInstruct(MachineInstr &MI,
                                      MachineBasicBlock *BB) const {

  Register RA, RC;
  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  unsigned Opc = Sw64::CTPOPOW;
  Register Scratch = RegInfo.createVirtualRegister(&Sw64::F4RCRegClass);

  RC = MI.getOperand(0).getReg();
  RA = MI.getOperand(1).getReg();

  if (MI.getOpcode() != Opc)
    Opc = Sw64::CTLZOW;

  BuildMI(*BB, MI, DL, TII->get(Opc))
      .addReg(Scratch, RegState::Define)
      .addReg(RA);
  BuildMI(*BB, MI, DL, TII->get(Sw64::FTOIS))
      .addReg(RC, RegState::Define)
      .addReg(Scratch);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return BB;
}

MachineBasicBlock *Sw64TargetLowering::emitAtomicBinaryPartword(
    MachineInstr &MI, MachineBasicBlock *BB, unsigned Size) const {
  assert((Size == 1 || Size == 2) &&
         "Unsupported size for EmitAtomicBinaryPartial.");

  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetRegisterClass *RC = getRegClassFor(MVT::i64);
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  unsigned OldVal = MI.getOperand(0).getReg();
  unsigned Ptr = MI.getOperand(1).getReg();
  unsigned Incr = MI.getOperand(2).getReg();

  unsigned StoreVal = RegInfo.createVirtualRegister(RC);
  unsigned LockVal = RegInfo.createVirtualRegister(RC);
  unsigned Reg_bic = RegInfo.createVirtualRegister(RC);
  unsigned Scratch = RegInfo.createVirtualRegister(RC);

  unsigned AtomicOp = 0;
  switch (MI.getOpcode()) {
  case Sw64::ATOMIC_LOAD_ADD_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_ADD_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_SUB_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_SUB_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_AND_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_AND_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_OR_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_OR_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_XOR_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_XOR_I8_POSTRA;
    break;
  case Sw64::ATOMIC_SWAP_I8:
    AtomicOp = Sw64::ATOMIC_SWAP_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_ADD_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_ADD_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_SUB_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_SUB_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_AND_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_AND_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_OR_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_OR_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_XOR_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_XOR_I16_POSTRA;
    break;
  case Sw64::ATOMIC_SWAP_I16:
    AtomicOp = Sw64::ATOMIC_SWAP_I16_POSTRA;
    break;

  case Sw64::ATOMIC_LOAD_UMAX_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_UMAX_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_MAX_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_MAX_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_UMIN_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_UMIN_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_MIN_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_MIN_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_NAND_I16:
    AtomicOp = Sw64::ATOMIC_LOAD_NAND_I16_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_UMAX_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_UMAX_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_MAX_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_MAX_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_UMIN_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_UMIN_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_MIN_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_MIN_I8_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_NAND_I8:
    AtomicOp = Sw64::ATOMIC_LOAD_NAND_I8_POSTRA;
    break;
  default:
    llvm_unreachable("Unknown pseudo atomic for replacement!");
  }

  MachineBasicBlock::iterator II(MI);

  unsigned t_Ptr = MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_Ptr).addReg(Ptr).addReg(Ptr);

  unsigned t_Incr = MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_Incr).addReg(Incr).addReg(Incr);

  BuildMI(*BB, II, DL, TII->get(AtomicOp))
      .addReg(OldVal, RegState::Define | RegState::EarlyClobber)
      .addReg(t_Ptr, RegState::EarlyClobber)
      .addReg(t_Incr, RegState::EarlyClobber)
      .addReg(StoreVal, RegState::Define | RegState::EarlyClobber |
                            RegState::Implicit | RegState::Dead)
      .addReg(LockVal, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead)
      .addReg(Reg_bic, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead)
      .addReg(Scratch, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead);

  MI.eraseFromParent(); // The instruction is gone now.

  return BB;
}

MachineBasicBlock *Sw64TargetLowering::emitAtomicCmpSwapPartword(
    MachineInstr &MI, MachineBasicBlock *BB, unsigned Size) const {
  assert((Size == 1 || Size == 2) &&
         "Unsupported size for EmitAtomicCmpSwapPartial.");

  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetRegisterClass *RC = getRegClassFor(MVT::i64);
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  unsigned Dest = MI.getOperand(0).getReg();
  unsigned Ptr = MI.getOperand(1).getReg();
  unsigned OldVal = MI.getOperand(2).getReg();
  unsigned NewVal = MI.getOperand(3).getReg();

  unsigned Reg_bic = RegInfo.createVirtualRegister(RC);
  unsigned Reg_ins = RegInfo.createVirtualRegister(RC);
  unsigned LockVal = RegInfo.createVirtualRegister(RC);
  unsigned Reg_cmp = RegInfo.createVirtualRegister(RC);
  unsigned Reg_mas = RegInfo.createVirtualRegister(RC);

  unsigned AtomicOp = 0;
  switch (MI.getOpcode()) {
  case Sw64::ATOMIC_CMP_SWAP_I8:
    AtomicOp = Sw64::ATOMIC_CMP_SWAP_I8_POSTRA;
    break;
  case Sw64::ATOMIC_CMP_SWAP_I16:
    AtomicOp = Sw64::ATOMIC_CMP_SWAP_I16_POSTRA;
    break;
  default:
    llvm_unreachable("Unknown pseudo atomic for replacement!");
  }

  MachineBasicBlock::iterator II(MI);

  unsigned t_Ptr = MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_Ptr).addReg(Ptr).addReg(Ptr);
  unsigned t_OldVal =
      MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_OldVal)
      .addReg(OldVal)
      .addReg(OldVal);
  unsigned t_NewVal =
      MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_NewVal)
      .addReg(NewVal)
      .addReg(NewVal);

  BuildMI(*BB, II, DL, TII->get(AtomicOp))
      .addReg(Dest, RegState::Define | RegState::EarlyClobber)
      .addReg(t_Ptr, RegState::EarlyClobber)
      .addReg(t_OldVal, RegState::EarlyClobber)
      .addReg(t_NewVal, RegState::EarlyClobber)
      .addReg(Reg_bic, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead)
      .addReg(Reg_ins, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead)
      .addReg(LockVal, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead)
      .addReg(Reg_cmp, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead)
      .addReg(Reg_mas, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead);

  MI.eraseFromParent(); // The instruction is gone now.

  return BB;
}

// This function also handles Sw64::ATOMIC_SWAP_I32 (when BinOpcode == 0), and
// Sw64::SWAP32
MachineBasicBlock *
Sw64TargetLowering::emitAtomicBinary(MachineInstr &MI,
                                     MachineBasicBlock *BB) const {
  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetRegisterClass *RC = getRegClassFor(MVT::i64);
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  unsigned AtomicOp;
  switch (MI.getOpcode()) {
  case Sw64::ATOMIC_LOAD_ADD_I32:
  case Sw64::LAS32:
    AtomicOp = Sw64::ATOMIC_LOAD_ADD_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_SUB_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_SUB_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_AND_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_AND_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_OR_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_OR_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_XOR_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_XOR_I32_POSTRA;
    break;
  case Sw64::ATOMIC_SWAP_I32:
  case Sw64::SWAP32:
    AtomicOp = Sw64::ATOMIC_SWAP_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_ADD_I64:
  case Sw64::LAS64:
    AtomicOp = Sw64::ATOMIC_LOAD_ADD_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_SUB_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_SUB_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_AND_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_AND_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_OR_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_OR_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_XOR_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_XOR_I64_POSTRA;
    break;
  case Sw64::ATOMIC_SWAP_I64:
  case Sw64::SWAP64:
    AtomicOp = Sw64::ATOMIC_SWAP_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_UMAX_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_UMAX_I64_POSTRA;
    break;

  case Sw64::ATOMIC_LOAD_MAX_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_MAX_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_UMIN_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_UMIN_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_MIN_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_MIN_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_NAND_I64:
    AtomicOp = Sw64::ATOMIC_LOAD_NAND_I64_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_UMAX_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_UMAX_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_MAX_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_MAX_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_UMIN_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_UMIN_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_MIN_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_MIN_I32_POSTRA;
    break;
  case Sw64::ATOMIC_LOAD_NAND_I32:
    AtomicOp = Sw64::ATOMIC_LOAD_NAND_I32_POSTRA;
    break;

  default:
    llvm_unreachable("Unknown pseudo atomic for replacement!");
  }

  unsigned OldVal = MI.getOperand(0).getReg();
  unsigned Ptr = MI.getOperand(1).getReg();
  unsigned Incr = MI.getOperand(2).getReg();

  unsigned StoreVal = RegInfo.createVirtualRegister(RC);
  unsigned Scratch = RegInfo.createVirtualRegister(RC);
  unsigned Scratch1 = RegInfo.createVirtualRegister(RC);

  MachineBasicBlock::iterator II(MI);

  unsigned t_Ptr = MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_Ptr).addReg(Ptr).addReg(Ptr);

  unsigned t_Incr = MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_Incr).addReg(Incr).addReg(Incr);

  BuildMI(*BB, II, DL, TII->get(AtomicOp))
      .addReg(OldVal, RegState::Define | RegState::EarlyClobber)
      .addReg(t_Ptr, RegState::EarlyClobber)
      .addReg(t_Incr, RegState::EarlyClobber)
      .addReg(StoreVal, RegState::Define | RegState::EarlyClobber |
                            RegState::Implicit | RegState::Dead)
      .addReg(Scratch, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead)
      .addReg(Scratch1, RegState::Define | RegState::EarlyClobber |
                            RegState::Implicit | RegState::Dead);

  MI.eraseFromParent(); // The instruction is gone now.

  return BB;
}

MachineBasicBlock *Sw64TargetLowering::emitAtomicCmpSwap(MachineInstr &MI,
                                                         MachineBasicBlock *BB,
                                                         unsigned Size) const {
  assert((Size == 4 || Size == 8) && "Unsupported size for EmitAtomicCmpSwap.");
  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetRegisterClass *RC = getRegClassFor(MVT::i64);
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  unsigned AtomicOp;

  switch (MI.getOpcode()) {
  case Sw64::CAS32:
  case Sw64::ATOMIC_CMP_SWAP_I32:
    AtomicOp = Sw64::ATOMIC_CMP_SWAP_I32_POSTRA;
    break;
  case Sw64::CAS64:
  case Sw64::ATOMIC_CMP_SWAP_I64:
    AtomicOp = Sw64::ATOMIC_CMP_SWAP_I64_POSTRA;
    break;
  default:
    llvm_unreachable("Unknown pseudo atomic for replacement!");
  }

  /*
      $0=Dest $16=Ptr $17=OldVal $18=NewVal

      memb
      $BB0_1:
         ldi $0,0($16)
         lldw $0,0($0)
         cmpeq $17,$0,$1
         wr_f $1
         bis $18,$18,$2
         lstw $2,0($16)
         rd_f $2
         beq $1,$BB0_2
         beq $2,$BB0_1
      $BB0_2:
 */

  unsigned Dest = MI.getOperand(0).getReg();
  unsigned Ptr = MI.getOperand(1).getReg();
  unsigned OldVal = MI.getOperand(2).getReg();
  unsigned NewVal = MI.getOperand(3).getReg();
  unsigned Scratch = RegInfo.createVirtualRegister(RC);
  unsigned Reg_cmp = RegInfo.createVirtualRegister(RC);

  MachineBasicBlock::iterator II(MI);

  unsigned t_Ptr = MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_Ptr).addReg(Ptr).addReg(Ptr);
  unsigned t_OldVal =
      MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_OldVal)
      .addReg(OldVal)
      .addReg(OldVal);
  unsigned t_NewVal =
      MF->getRegInfo().createVirtualRegister(&Sw64::GPRCRegClass);
  BuildMI(*BB, II, DL, TII->get(Sw64::BISr), t_NewVal)
      .addReg(NewVal)
      .addReg(NewVal);

  BuildMI(*BB, II, DL, TII->get(AtomicOp))
      .addReg(Dest, RegState::Define | RegState::EarlyClobber)
      .addReg(t_Ptr, RegState::EarlyClobber)
      .addReg(t_OldVal, RegState::EarlyClobber)
      .addReg(t_NewVal, RegState::EarlyClobber)
      .addReg(Scratch, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead)
      .addReg(Reg_cmp, RegState::Define | RegState::EarlyClobber |
                           RegState::Implicit | RegState::Dead);

  MI.eraseFromParent(); // The instruction is gone now.

  return BB;
}

MVT Sw64TargetLowering::getScalarShiftAmountTy(const DataLayout &DL,
                                               EVT LHSTy) const {
  return MVT::i64;
}

bool Sw64TargetLowering::isOffsetFoldingLegal(
    const GlobalAddressSDNode *GA) const {
  // The Sw64 target isn't yet aware of offsets.
  return false;
}

EVT Sw64TargetLowering::getOptimalMemOpType(
    const MemOp &Op, const AttributeList & /*FuncAttributes*/) const {
  if (Subtarget.enOptMemset())
    return MVT::i64;
  return MVT::Other;
}

bool Sw64TargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT) const {
  if (VT != MVT::f32 && VT != MVT::f64)
    return false;
  // +0.0   F31
  // +0.0f  F31
  // -0.0  -F31
  // -0.0f -F31
  return Imm.isZero() || Imm.isNegZero();
}

SDValue Sw64TargetLowering::getRecipEstimate(SDValue Operand, SelectionDAG &DAG,
                                             int Enabled,
                                             int &RefinementSteps) const {
  EVT VT = Operand.getValueType();
  if ((VT == MVT::f32 || VT == MVT::f64) && Subtarget.hasCore4() &&
      Subtarget.enableFloatAri()) {
    if (RefinementSteps == ReciprocalEstimate::Unspecified) {
      if (VT.getScalarType() == MVT::f32)
        RefinementSteps = 2;
      if (VT.getScalarType() == MVT::f64)
        RefinementSteps = 3;
    }
    if (VT.getScalarType() == MVT::f32)
      return DAG.getNode(Sw64ISD::FRECS, SDLoc(Operand), VT, Operand);
    if (VT.getScalarType() == MVT::f64)
      return DAG.getNode(Sw64ISD::FRECD, SDLoc(Operand), VT, Operand);
  }
  return SDValue();
}

bool Sw64TargetLowering::getPostIndexedAddressParts(SDNode *N, SDNode *Op,
                                                    SDValue &Base,
                                                    SDValue &Offset,
                                                    ISD::MemIndexedMode &AM,
                                                    SelectionDAG &DAG) const {
  EVT VT;
  SDValue Ptr;
  LSBaseSDNode *LSN = dyn_cast<LSBaseSDNode>(N);
  if (!LSN)
    return false;
  VT = LSN->getMemoryVT();
  bool IsLegalType = VT == MVT::i8 || VT == MVT::i16 || VT == MVT::i32 ||
                     VT == MVT::i64 || VT == MVT::f32 || VT == MVT::f64;
  if (!IsLegalType)
    return false;
  if (Op->getOpcode() != ISD::ADD)
    return false;
  if (LoadSDNode *LD = dyn_cast<LoadSDNode>(N)) {
    VT = LD->getMemoryVT();
    Ptr = LD->getBasePtr();
  } else if (StoreSDNode *ST = dyn_cast<StoreSDNode>(N)) {
    VT = ST->getMemoryVT();
    Ptr = ST->getBasePtr();
  } else
    return false;

  if (ConstantSDNode *RHS = dyn_cast<ConstantSDNode>(Op->getOperand(1))) {
    uint64_t RHSC = RHS->getZExtValue();
    Base = Ptr;
    Offset = DAG.getConstant(RHSC, SDLoc(N), MVT::i64);
    AM = ISD::POST_INC;
    return true;
  }

  return false;
}

const TargetRegisterClass *Sw64TargetLowering::getRepRegClassFor(MVT VT) const {
  if (VT == MVT::Other)
    return &Sw64::GPRCRegClass;
  if (VT == MVT::i32)
    return &Sw64::FPRC_loRegClass;
  return TargetLowering::getRepRegClassFor(VT);
}

bool Sw64TargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                               const AddrMode &AM, Type *Ty,
                                               unsigned AS,
                                               Instruction *I) const {
  if (!Subtarget.hasCore4() || !Subtarget.enablePostInc())
    return llvm::TargetLoweringBase::isLegalAddressingMode(DL, AM, Ty, AS, I);

  // No global is ever allowed as a base.
  if (AM.BaseGV)
    return false;

  // Require a 12-bit signed offset.
  if (!isInt<12>(AM.BaseOffs))
    return false;

  switch (AM.Scale) {
  case 0: // "r+i" or just "i", depending on HasBaseReg.
    break;
  case 1:
    if (!AM.HasBaseReg) // allow "r+i".
      break;
    return false; // disallow "r+r" or "r+r+i".
  default:
    return false;
  }

  return true;
}

bool Sw64TargetLowering::isFMAFasterThanFMulAndFAdd(const MachineFunction &MF,
                                                    EVT VT) const {
  VT = VT.getScalarType();

  if (!VT.isSimple())
    return false;

  switch (VT.getSimpleVT().SimpleTy) {
  case MVT::f32:
  case MVT::f64:
    return true;
  default:
    break;
  }

  return false;
}

bool Sw64TargetLowering::isFMAFasterThanFMulAndFAdd(const Function &F,
                                                    Type *Ty) const {
  switch (Ty->getScalarType()->getTypeID()) {
  case Type::FloatTyID:
  case Type::DoubleTyID:
    return true;
  default:
    return false;
  }
}

bool Sw64TargetLowering::isZExtFree(SDValue Val, EVT VT2) const {
  // Zexts are free if they can be combined with a load.
  if (Subtarget.enOptExt()) {
    if (auto *LD = dyn_cast<LoadSDNode>(Val)) {
      EVT MemVT = LD->getMemoryVT();
      if ((MemVT == MVT::i8 || MemVT == MVT::i16 ||
           (Subtarget.is64Bit() && MemVT == MVT::i32)) &&
          (LD->getExtensionType() == ISD::NON_EXTLOAD ||
           LD->getExtensionType() == ISD::ZEXTLOAD))
        return true;
    }
  }

  return TargetLowering::isZExtFree(Val, VT2);
}

bool Sw64TargetLowering::isSExtCheaperThanZExt(EVT SrcVT, EVT DstVT) const {
  if (Subtarget.enOptExt())
    return SrcVT == MVT::i32 && DstVT == MVT::i64;
  return false;
}

bool Sw64TargetLowering::isLegalICmpImmediate(int64_t Imm) const {
  if (Subtarget.enOptExt())
    return Imm >= 0 && Imm <= 255;
  return false;
}

bool Sw64TargetLowering::isLegalAddImmediate(int64_t Imm) const {
  if (Subtarget.enOptExt())
    return Imm >= 0 && Imm <= 255;
  return false;
}
