//===- Sw64InstrInfo.td - The Sw64 Instruction Set -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

include "Sw64InstrFormats.td"

//===----------------------------------------------------------------------===//
// Sw64 Instruction Predicate Definitions.
//
def EnableIntShift         : Predicate<"Subtarget->enableIntShift()">,
                             AssemblerPredicate<(all_of Featureintshift), "swIntShift">;

def EnableFloatCmov        : Predicate<"Subtarget->enableFloatCmov()">;

def EnableCrcInst          : Predicate<"Subtarget->enableCrcInst()">;

def EnableWmembInst        : Predicate<"Subtarget->enableWmembInst()">;

def EnableCasInst          : Predicate<"Subtarget->enableCasInst()">;

def HasSIMD                : Predicate<"Subtarget->hasSIMD()">,
                             AssemblerPredicate<(all_of FeatureSIMD)>;
//********************
//Custom DAG Nodes
//********************

def SDTFPUnaryOpUnC  : SDTypeProfile<1, 1, [
  SDTCisFP<1>, SDTCisFP<0>
]>;

def SDTIntTriOp : SDTypeProfile<1, 3, [
  SDTCisInt<1>, SDTCisInt<0>, SDTCisInt<2>, SDTCisInt<3>
]>;

def Sw64_cvtqt   : SDNode<"Sw64ISD::CVTQT_",    SDTFPUnaryOpUnC, []>;
def Sw64_cvtqs   : SDNode<"Sw64ISD::CVTQS_",    SDTFPUnaryOpUnC, []>;
def Sw64_cvttq   : SDNode<"Sw64ISD::CVTTQ_"  ,  SDTFPUnaryOp, []>;
def Sw64_cvtts   : SDNode<"Sw64ISD::CVTTS_",    SDTFPUnaryOpUnC,
                          [SDNPHasChain]>;
def Sw64_cvtst   : SDNode<"Sw64ISD::CVTST_",    SDTFPUnaryOpUnC,
                          [SDNPHasChain]>;
def Sw64_tprello : SDNode<"Sw64ISD::TPRelLo",   SDTIntBinOp, []>;
def Sw64_tprelhi : SDNode<"Sw64ISD::TPRelHi",   SDTIntBinOp, []>;

def Sw64_tlsgd   : SDNode<"Sw64ISD::TLSGD",    SDTIntTriOp, []>;
def Sw64_tlsldm   : SDNode<"Sw64ISD::TLSLDM",    SDTIntBinOp, []>;
def Sw64_dtprello : SDNode<"Sw64ISD::DTPRelLo",   SDTIntBinOp, []>;
def Sw64_dtprelhi : SDNode<"Sw64ISD::DTPRelHi",   SDTIntBinOp, []>;

def Sw64_syscall   : SDNode<"Sw64ISD::SysCall", SDTIntUnaryOp, []>;
def Sw64_LDAWithChain : SDNode<"Sw64ISD::LDAWC", SDTIntBinOp, [SDNPHasChain]>;
def Sw64_gprello : SDNode<"Sw64ISD::GPRelLo",   SDTIntUnaryOp>;
def Sw64_gprelhi : SDNode<"Sw64ISD::GPRelHi",   SDTIntUnaryOp>;
def Sw64_rellit  : SDNode<"Sw64ISD::RelLit",    SDTIntUnaryOp>;

def Sw64_gprel : SDNode<"Sw64ISD::GPRel",   SDTIntUnaryOp>;
def Sw64_tprel : SDNode<"Sw64ISD::TPRel",   SDTIntUnaryOp>;
def Sw64_dtprel : SDNode<"Sw64ISD::DTPRel",   SDTIntUnaryOp>;

def Sw64ldih : SDNode<"Sw64ISD::LDIH", SDTIntUnaryOp, []>;
def Sw64ldi : SDNode<"Sw64ISD::LDI", SDTIntBinOp, [SDNPOutGlue]>;

def Sw64_relgottp  : SDNode<"Sw64ISD::RelGottp",    SDTIntBinOp, [SDNPMayLoad]>;
def retflag       : SDNode<"Sw64ISD::RET_FLAG", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue]>;

// These are target-independent nodes, but have target-specific formats.
def SDT_Sw64CallSeqStart : SDCallSeqStart<[ SDTCisVT<0, i64> ]>;
def SDT_Sw64CallSeqEnd   : SDCallSeqEnd<[ SDTCisVT<0, i64>,
                                          SDTCisVT<1, i64> ]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_Sw64CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_Sw64CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def Sw64_frecs : SDNode<"Sw64ISD::FRECS",     SDTFPUnaryOp, []>;
def Sw64_frecd : SDNode<"Sw64ISD::FRECD",     SDTFPUnaryOp, []>;
def Sw64_sbt   : SDNode<"Sw64ISD::SBT",     SDTIntBinOp, []>;
def Sw64_cbt   : SDNode<"Sw64ISD::CBT",     SDTIntBinOp, []>;
def Sw64_addpi   : SDNode<"Sw64ISD::ADDPI",   SDTIntUnaryOp, []>;
def Sw64_addpis  : SDNode<"Sw64ISD::ADDPIS",  SDTIntUnaryOp, []>;

def Sw64_revbh  : SDNode<"Sw64ISD::REVBH",  SDTIntUnaryOp, []>;
def Sw64_revbw  : SDNode<"Sw64ISD::REVBW",  SDTIntUnaryOp, []>;

def Sw64_rolw : SDNode<"Sw64ISD::ROLW",   SDTIntBinOp, []>;

def Sw64_crc32b : SDNode<"Sw64ISD::CRC32B",   SDTIntBinOp, []>;
def Sw64_crc32h : SDNode<"Sw64ISD::CRC32H",   SDTIntBinOp, []>;
def Sw64_crc32w : SDNode<"Sw64ISD::CRC32W",   SDTIntBinOp, []>;
def Sw64_crc32l : SDNode<"Sw64ISD::CRC32L",   SDTIntBinOp, []>;
def Sw64_crc32cb : SDNode<"Sw64ISD::CRC32CB",   SDTIntBinOp, []>;
def Sw64_crc32ch : SDNode<"Sw64ISD::CRC32CH",   SDTIntBinOp, []>;
def Sw64_crc32cw : SDNode<"Sw64ISD::CRC32CW",   SDTIntBinOp, []>;
def Sw64_crc32cl : SDNode<"Sw64ISD::CRC32CL",   SDTIntBinOp, []>;

def Sw64_casl : SDNode<"Sw64ISD::CASL",   SDTIntBinOp, []>;
def Sw64_casw : SDNode<"Sw64ISD::CASW",   SDTIntBinOp, []>;

let Constraints = "$RD = $RC" in
class inst_cas<bits<6> opc, bits<8> fun, string opstr>
	: OForm <opc, fun, (ins GPRC:$RA, GPRC:$RB, GPRC:$RC), (outs GPRC:$RD),
                 opstr, "$RA,$RB,$RC">;

def CASW : inst_cas<0x10, 0x5e, "casw">;
def CASL : inst_cas<0x10, 0x5f, "casl">;

def : Pat<(atomic_cmp_swap_64 GPRC:$ptr, GPRC:$cmp, GPRC:$swp),
          (CASL GPRC:$cmp, GPRC:$ptr, GPRC:$swp)>,
          Requires<[EnableCasInst, HasCore4]>;

def : Pat<(atomic_cmp_swap_32 GPRC:$ptr, GPRC:$cmp, GPRC:$swp),
          (CASW GPRC:$cmp, GPRC:$ptr, GPRC:$swp)>,
          Requires<[EnableCasInst, HasCore4]>;

def call_symbol : Operand<i64>;
//********************
//Paterns for matching
//********************
def invX : SDNodeXForm<imm, [{ //invert
  return getI64Imm(~N->getZExtValue(), SDLoc(N));
}]>;
def negX : SDNodeXForm<imm, [{ //negate
  return getI64Imm(~N->getZExtValue() + 1, SDLoc(N));
}]>;
def SExt32 : SDNodeXForm<imm, [{ //signed extend int to long
  return getI64Imm(((int64_t)N->getZExtValue() << 32) >> 32, SDLoc(N));
}]>;
def SExt16 : SDNodeXForm<imm, [{ //signed extend int to long
  return getI64Imm(((int64_t)N->getZExtValue() << 48) >> 48, SDLoc(N));
}]>;
def LL16 : SDNodeXForm<imm, [{ //lda part of constant
  return getI64Imm(get_lda16(N->getZExtValue()), SDLoc(N));
}]>;
def LH16 : SDNodeXForm<imm, [{ //ldah part of constant (or more if too big)
  return getI64Imm(get_ldah16(N->getZExtValue()), SDLoc(N));
}]>;
def iZAPX : SDNodeXForm<and, [{ // get imm to ZAPi
  ConstantSDNode *RHS = cast<ConstantSDNode>(N->getOperand(1));
  return getI64Imm(get_zapImm(SDValue(), RHS->getZExtValue()), SDLoc(N));
}]>;
def nearP2X : SDNodeXForm<imm, [{
  return getI64Imm(Log2_64(getNearPower2((uint64_t)N->getZExtValue())), SDLoc(N));
}]>;
def nearP2RemX : SDNodeXForm<imm, [{
  uint64_t x =
    abs64(N->getZExtValue() - getNearPower2((uint64_t)N->getZExtValue()));
  return getI64Imm(Log2_64(x), SDLoc(N));
}]>;

def immUExt8  : PatLeaf<(imm), [{ //imm fits in 8 bit zero extended field
  return (uint64_t)N->getZExtValue() == (uint8_t)N->getZExtValue();
}]>;
def immUExt8inv  : PatLeaf<(imm), [{ //inverted imm fits in 8 bit zero extended field
  return (uint64_t)~N->getZExtValue() == (uint8_t)~N->getZExtValue();
}], invX>;
def immUExt8neg  : PatLeaf<(imm), [{ //negated imm fits in 8 bit zero extended field
  return ((uint64_t)~N->getZExtValue() + 1) ==
         (uint8_t)((uint64_t)~N->getZExtValue() + 1);
}], negX>;

def immUExt13 : PatLeaf<(imm), [{
  return (uint32_t)N->getZExtValue() < (1 << 13);
}]>;

def immSExt12  : PatLeaf<(imm), [{ //imm fits in 12 bit sign extended field
  return ((int64_t)N->getZExtValue() << 52) >> 52 ==
         (int64_t)N->getZExtValue();
}]>;

def immSExt16  : PatLeaf<(imm), [{ //imm fits in 16 bit sign extended field
  return ((int64_t)N->getZExtValue() << 48) >> 48 ==
         (int64_t)N->getZExtValue();
}]>;

def zappat : PatFrag<(ops node:$LHS), (and node:$LHS, imm), [{
  ConstantSDNode *RHS = dyn_cast<ConstantSDNode>(N->getOperand(1));
  if (!RHS) return 0;
  uint64_t build = get_zapImm(N->getOperand(0), (uint64_t)RHS->getZExtValue());
  return build != 0;
}]>;

def immFPZ  : PatLeaf<(fpimm), [{ //the only fpconstant nodes are +/- 0.0
  (void)N; // silence warning.
  return true;
}]>;

def immRem1 :PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),1,0);}]>;
def immRem2 :PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),2,0);}]>;
def immRem3 :PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),3,0);}]>;
def immRem4 :PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),4,0);}]>;
def immRem5 :PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),5,0);}]>;
def immRem1n:PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),1,1);}]>;
def immRem2n:PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),2,1);}]>;
def immRem3n:PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),3,1);}]>;
def immRem4n:PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),4,1);}]>;
def immRem5n:PatLeaf<(imm),[{return chkRemNearPower2(N->getZExtValue(),5,1);}]>;

def immRemP2n : PatLeaf<(imm), [{
  return isPowerOf2_64(getNearPower2((uint64_t)N->getZExtValue()) -
                       N->getZExtValue());
}]>;
def immRemP2 : PatLeaf<(imm), [{
  return isPowerOf2_64(N->getZExtValue() -
                       getNearPower2((uint64_t)N->getZExtValue()));
}]>;
def immUExt8ME : PatLeaf<(imm), [{ //use this imm for mulqi
  int64_t d =  abs64((int64_t)N->getZExtValue() -
               (int64_t)getNearPower2((uint64_t)N->getZExtValue()));
  if (isPowerOf2_64(d)) return false;
  switch (d) {
    case 1: case 3: case 5: return false;
    default: return (uint64_t)N->getZExtValue() == (uint8_t)N->getZExtValue();
  };
}]>;

def intop : PatFrag<(ops node:$op), (sext_inreg node:$op, i32)>;
def add4  : PatFrag<(ops node:$op1, node:$op2),
                    (add (shl node:$op1, (i64 2)), node:$op2)>;
def sub4  : PatFrag<(ops node:$op1, node:$op2),
                    (sub (shl node:$op1, (i64 2)), node:$op2)>;
def add8  : PatFrag<(ops node:$op1, node:$op2),
                    (add (shl node:$op1, (i64 3)), node:$op2)>;
def sub8  : PatFrag<(ops node:$op1, node:$op2),
                    (sub (shl node:$op1, (i64 3)), node:$op2)>;

class ThridOpFrag<dag res> : PatFrag<(ops node:$LHS, node:$MHS, node:$RHS), res>;
class BinOpFrag<dag res> : PatFrag<(ops node:$LHS, node:$RHS), res>;
class CmpOpFrag<dag res> : PatFrag<(ops node:$R), res>;

def IsOrAdd: PatFrag<(ops node:$A, node:$B), (or node:$A, node:$B), [{
  return isOrEquivalentToAdd(N);
}]>;
def AddrFI : ComplexPattern<iPTR, 1, "SelectAddrFI", [frameindex], []>;

//Pseudo ops for selection

def WTF : PseudoInstSw64<(outs), (ins variable_ops), "#wtf", []>, Sched<[]>;

let hasCtrlDep = 1, Defs = [R30], Uses = [R30] in {
def ADJUSTSTACKUP : PseudoInstSw64<(outs), (ins s64imm:$amt1, s64imm:$amt2),
                "; ADJUP $amt1",
                [(callseq_end timm:$amt1, timm:$amt2)]>, Sched<[]>;
def ADJUSTSTACKDOWN : PseudoInstSw64<(outs), (ins s64imm:$amt1, s64imm:$amt2),
                "; ADJDOWN $amt1",
                [(callseq_start (i64 timm:$amt1), (i64 timm:$amt2))]>, Sched<[]>;
}

let isCodeGenOnly = 1 in {
def ALTENT : PseudoInstSw64<(outs), (ins s64imm:$TARGET), "$$${TARGET}..ng:\n",
                            []>, Sched<[]>;
def PCLABEL : PseudoInstSw64<(outs), (ins s64imm:$num), "PCMARKER_$num:\n",[]>,
                            Sched<[]>;
def MEMLABEL : PseudoInstSw64<(outs), (ins s64imm:$i, s64imm:$j, s64imm:$k,
                              s64imm:$m),
                              "LSMARKER$$$i$$$j$$$k$$$m:", []>, Sched<[]>;
}
let hasNoSchedulingInfo = 1 in {
let usesCustomInserter = 1 in {   // Expanded after instruction selection.
def ATOMIC_CMP_SWAP_I32 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 32-bit atomic cmpare and swap",
    [(set GPRC:$dst, (atomic_cmp_swap_32 GPRC:$ptr, GPRC:$cmp, GPRC:$swp))]>;

def ATOMIC_CMP_SWAP_I64 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 64-bit atomic compare and swap",
    [(set GPRC:$dst, (atomic_cmp_swap_64 GPRC:$ptr, GPRC:$cmp, GPRC:$swp))]>;


def ATOMIC_LOAD_ADD_I32 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic load add",
    [(set GPRC:$dst, (atomic_load_add_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_ADD_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic load add",
    [(set GPRC:$dst, (atomic_load_add_64 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_UMAX_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 64-bit atomic load umax",
    [(set GPRC:$dst, (atomic_load_umax_64 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_MAX_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 64-bit atomic load max",
    [(set GPRC:$dst, (atomic_load_max_64 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_UMIN_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 64-bit atomic load umin",
    [(set GPRC:$dst, (atomic_load_umin_64 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_MIN_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 64-bit atomic load min",
    [(set GPRC:$dst, (atomic_load_min_64 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_NAND_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 64-bit atomic load nand",
    [(set GPRC:$dst, (atomic_load_nand_64 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_UMAX_I32 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 32-bit atomic load umax",
    [(set GPRC:$dst, (atomic_load_umax_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_MAX_I32 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 32-bit atomic load max",
    [(set GPRC:$dst, (atomic_load_max_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_UMIN_I32 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 32-bit atomic load umin",
    [(set GPRC:$dst, (atomic_load_umin_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_MIN_I32 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 32-bit atomic load min",
    [(set GPRC:$dst, (atomic_load_min_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_NAND_I32 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 32-bit atomic load nand",
    [(set GPRC:$dst, (atomic_load_nand_32 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_UMAX_I16 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 16-bit atomic load umax",
    [(set GPRC:$dst, (atomic_load_umax_16 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_MAX_I16 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 16-bit atomic load max",
    [(set GPRC:$dst, (atomic_load_max_16 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_UMIN_I16 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 16-bit atomic load umin",
    [(set GPRC:$dst, (atomic_load_umin_16 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_MIN_I16 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 16-bit atomic load min",
    [(set GPRC:$dst, (atomic_load_min_16 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_NAND_I16 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 16-bit atomic load nand",
    [(set GPRC:$dst, (atomic_load_nand_16 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_UMAX_I8 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 8-bit atomic load umax",
    [(set GPRC:$dst, (atomic_load_umax_8 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_MAX_I8 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 8-bit atomic load max",
    [(set GPRC:$dst, (atomic_load_max_8 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_UMIN_I8 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 8-bit atomic load umin",
    [(set GPRC:$dst, (atomic_load_umin_8 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_MIN_I8 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 8-bit atomic load min",
    [(set GPRC:$dst, (atomic_load_min_8 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_NAND_I8 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp),"# 8-bit atomic load nand",
    [(set GPRC:$dst, (atomic_load_nand_8 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_SWAP_I32 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic swap",
    [(set GPRC:$dst, (atomic_swap_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_SWAP_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic swap",
    [(set GPRC:$dst, (atomic_swap_64 GPRC:$ptr, GPRC:$swp))]>;


def ATOMIC_LOAD_AND_I32 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic load and",
    [(set GPRC:$dst, (atomic_load_and_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_AND_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic load and",
    [(set GPRC:$dst, (atomic_load_and_64 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_OR_I32 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic load or",
    [(set GPRC:$dst, (atomic_load_or_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_OR_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic load and",
    [(set GPRC:$dst, (atomic_load_or_64 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_SUB_I32 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic load sub",
    [(set GPRC:$dst, (atomic_load_sub_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_SUB_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic load sub",
    [(set GPRC:$dst, (atomic_load_sub_64 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_XOR_I32 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic load xor",
    [(set GPRC:$dst, (atomic_load_xor_32 GPRC:$ptr, GPRC:$swp))]>;
def ATOMIC_LOAD_XOR_I64 :PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic load xor",
    [(set GPRC:$dst, (atomic_load_xor_64 GPRC:$ptr, GPRC:$swp))]>;


//I8
def ATOMIC_LOAD_ADD_I8: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic load add",
    [(set GPRC:$dst, (atomic_load_add_8 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_SWAP_I8: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic swap",
    [(set GPRC:$dst, (atomic_swap_8 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_AND_I8: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic load and",
    [(set GPRC:$dst, (atomic_load_and_8 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_OR_I8: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic load or",
    [(set GPRC:$dst, (atomic_load_or_8 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_SUB_I8: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic load sub",
    [(set GPRC:$dst, (atomic_load_sub_8 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_XOR_I8: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic load xor",
    [(set GPRC:$dst, (atomic_load_xor_8 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_CMP_SWAP_I8 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 8-bit atomic compare and swap",
    [(set GPRC:$dst, (atomic_cmp_swap_8 GPRC:$ptr, GPRC:$cmp, GPRC:$swp))]>;

//I16
def ATOMIC_LOAD_ADD_I16: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic load add",
    [(set GPRC:$dst, (atomic_load_add_16 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_SWAP_I16: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic swap",
    [(set GPRC:$dst, (atomic_swap_16 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_AND_I16: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic and",
    [(set GPRC:$dst, (atomic_load_and_16 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_OR_I16: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic load or",
    [(set GPRC:$dst, (atomic_load_or_16 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_SUB_I16: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic load and sub",
    [(set GPRC:$dst, (atomic_load_sub_16 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_LOAD_XOR_I16: PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic xor",
    [(set GPRC:$dst, (atomic_load_xor_16 GPRC:$ptr, GPRC:$swp))]>;

def ATOMIC_CMP_SWAP_I16 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 16-bit atomic compare and swap",
    [(set GPRC:$dst, (atomic_cmp_swap_16 GPRC:$ptr, GPRC:$cmp, GPRC:$swp))]>;


def CAS32 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 32-bit atomic compare and swap",
    [(set GPRC:$dst, (atomic_cmp_swap_32 GPRC:$ptr, GPRC:$cmp, GPRC:$swp))]>;
def CAS64 : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 64-bit atomic compare and swap",
    [(set GPRC:$dst, (atomic_cmp_swap_64 GPRC:$ptr, GPRC:$cmp, GPRC:$swp))]>;

def LAS32 : PseudoInstSw64<(outs GPRC:$dst), (ins GPRC:$ptr, GPRC:$swp),
    "# 32-bit atomic load and sub",
    [(set GPRC:$dst, (atomic_load_add_32 GPRC:$ptr, GPRC:$swp))]>;
def LAS64 :PseudoInstSw64<(outs GPRC:$dst), (ins GPRC:$ptr, GPRC:$swp),
    "# 64-bit atomic load and sub",
    [(set GPRC:$dst, (atomic_load_add_64 GPRC:$ptr, GPRC:$swp))]>;

def SWAP32 : PseudoInstSw64<(outs GPRC:$dst), (ins GPRC:$ptr, GPRC:$swp),
    "# 32-bit atomic swap",
    [(set GPRC:$dst, (atomic_swap_32 GPRC:$ptr, GPRC:$swp))]>;
def SWAP64 :PseudoInstSw64<(outs GPRC:$dst), (ins GPRC:$ptr, GPRC:$swp),
    "# 64-bit atomic swap",
    [(set GPRC:$dst, (atomic_swap_64 GPRC:$ptr, GPRC:$swp))]>;
}

let mayLoad = 1, mayStore = 1 in {
  def ATOMIC_LOAD_ADD_I32_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic", []>;
  def ATOMIC_LOAD_ADD_I64_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic", []>;

  def ATOMIC_SWAP_I32_POSTRA      : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic", []>;
  def ATOMIC_SWAP_I64_POSTRA      : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic", []>;

  def ATOMIC_LOAD_AND_I32_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic", []>;
  def ATOMIC_LOAD_AND_I64_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic", []>;

  def ATOMIC_LOAD_OR_I32_POSTRA   : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic", []>;
  def ATOMIC_LOAD_OR_I64_POSTRA   : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic", []>;

  def ATOMIC_LOAD_SUB_I32_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic", []>;
  def ATOMIC_LOAD_SUB_I64_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic", []>;

  def ATOMIC_LOAD_XOR_I32_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 32-bit atomic", []>;
  def ATOMIC_LOAD_XOR_I64_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$swp), "# 64-bit atomic", []>;

  def ATOMIC_CMP_SWAP_I32_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 32-bit atomic", []>;
  def ATOMIC_CMP_SWAP_I64_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
      (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 64-bit atomic", []>;
}
def ATOMIC_LOAD_ADD_I8_POSTRA     : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_ADD_I16_POSTRA    : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic", []>;

def ATOMIC_SWAP_I8_POSTRA         : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_SWAP_I16_POSTRA        : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic", []>;

def ATOMIC_LOAD_AND_I8_POSTRA     : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_AND_I16_POSTRA    : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic", []>;

def ATOMIC_LOAD_OR_I8_POSTRA      : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_OR_I16_POSTRA     : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic", []>;

def ATOMIC_LOAD_SUB_I8_POSTRA     : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_SUB_I16_POSTRA    : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic", []>;

def ATOMIC_LOAD_XOR_I8_POSTRA     : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_XOR_I16_POSTRA    : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$swp), "# 16-bit atomic", []>;

def ATOMIC_CMP_SWAP_I8_POSTRA     : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_CMP_SWAP_I16_POSTRA    : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 16-bit atomic", []>;

def ATOMIC_LOAD_UMAX_I8_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_MAX_I8_POSTRA   : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_UMIN_I8_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_MIN_I8_POSTRA   : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 8-bit atomic", []>;
def ATOMIC_LOAD_NAND_I8_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 8-bit atomic", []>;

def ATOMIC_LOAD_UMAX_I16_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 16-bit atomic", []>;
def ATOMIC_LOAD_MAX_I16_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 16-bit atomic", []>;
def ATOMIC_LOAD_UMIN_I16_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 16-bit atomic", []>;
def ATOMIC_LOAD_MIN_I16_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 16-bit atomic", []>;
def ATOMIC_LOAD_NAND_I16_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 16-bit atomic", []>;

def ATOMIC_LOAD_UMAX_I32_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 32-bit atomic", []>;
def ATOMIC_LOAD_MAX_I32_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 32-bit atomic", []>;
def ATOMIC_LOAD_UMIN_I32_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 32-bit atomic", []>;
def ATOMIC_LOAD_MIN_I32_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 32-bit atomic", []>;
def ATOMIC_LOAD_NAND_I32_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 32-bit atomic", []>;

def ATOMIC_LOAD_UMAX_I64_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 64-bit atomic", []>;
def ATOMIC_LOAD_MAX_I64_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 64-bit atomic", []>;
def ATOMIC_LOAD_UMIN_I64_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 64-bit atomic", []>;
def ATOMIC_LOAD_MIN_I64_POSTRA  : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 64-bit atomic", []>;
def ATOMIC_LOAD_NAND_I64_POSTRA : PseudoInstSw64<(outs GPRC:$dst),
    (ins GPRC:$ptr, GPRC:$cmp, GPRC:$swp), "# 64-bit atomic", []>;



}// for atomic load/store. set hasNoSchedulingInfo

//***********************
//Real instructions
//***********************

//4 The base instruction system
//4.1 system call instruction
let hasSideEffects = 1 in
class inst_syscall<bits<6> opcode, string opstr, string operands="">
    : PALForm<opcode, (ins u26imm:$disp), (outs), opstr, operands>;

let Defs=[R0] in
def SYS_CALL : inst_syscall<0x00, "sys_call", "$disp">;

def LBR : inst_syscall<0x1d, "lbr", "$disp">;

def : Pat<(Sw64_syscall (i64 immUExt8:$N)),(SYS_CALL immUExt8:$N )>;

//4.2 control instruction

//4.2.1 jump
// Call

def SDT_Sw64JmpLink      : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
def Sw64JmpLink : SDNode<"Sw64ISD::JmpLink", SDT_Sw64JmpLink,
                          [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                           SDNPVariadic]>;

class arg_jmp<string opstr, bits<6> opcode, list<dag> pattern>
    : MForm<opcode, (ins GPRC:$RA, GPRC:$RB , s16imm:$DISP), (outs) , opstr,
            "$RA,(${RB}),$DISP", pattern>;

class branch_i<string opstr, bits<6> opcode, list<dag> pattern>
    : BForm<opcode, (ins GPRC:$RA, target:$disp) , (outs) ,
            opstr, "$RA,${disp}", pattern>;

let isCall = 1, Defs = [R26], Uses = [R27] in
def JSR : arg_jmp<"call", 0x01,  []>;

let isReturn = 1, isTerminator = 1, isBarrier = 1 in
def RET : arg_jmp<"ret", 0x02,  []>;

def Sw64Ret : SDNode<"Sw64ISD::Ret", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

let RA = 31, DISP=0 in
def JMP : arg_jmp<"jmp", 0x03,  []>;

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasCtrlDep = 1 in
def BR : branch_i<"br", 0x04, []>;
let isBranch = 1, isTerminator = 1, hasCtrlDep = 1 in
def BSR : branch_i<"bsr", 0x05, []>;

// for expand Call target, we create two Insns like:
// load R27,symbol(GP) defs R27
// call R26,R27,symbol use R27
// so we dont need to use R27, we actually def R27
let isBarrier = 1, isCall = 1, Defs = [R26, R27], Uses = [R29] in {
def PseudoCall : PseudoInstSw64<(outs), (ins call_symbol:$func), "",
                                []>,Sched<[WriteJmp]>;
}

let isBarrier = 1, isCall = 1, Defs = [R26], Uses = [R27, R29] in {
  def PseudoCallIndirect : PseudoInstSw64<(outs), (ins GPRC:$RB), "",
		            [(Sw64JmpLink  GPRC:$RB)]>,
			  PseudoInstExpansion<(JSR R26, GPRC:$RB, 0)>,
			    Sched<[WriteJmp]>;
}

let isBarrier = 1, isBranch = 1, isTerminator = 1 in
def PseudoBrind : PseudoInstSw64<(outs), (ins GPRC:$RB), "",
                                 [(brind GPRC:$RB)]>,
                  PseudoInstExpansion<(JMP R31, GPRC:$RB, 0)>,
                  Sched<[WriteJmp]>;

//to match libgcc _div _rem
let isBarrier = 1, isCall = 1, Defs = [R23, R24, R25, R27, R28]
                    , Uses = [R24, R25, R27] in
def PseudoCallDiv : PseudoInstSw64<(outs), (ins), "", []>,
                    PseudoInstExpansion<(JSR R23, R27, 0)>,
                    Sched<[WriteJmp]>;

let  isBranch = 1, isTerminator = 1, isBarrier = 1, hasCtrlDep = 1  in
def PseudoBR : PseudoInstSw64<(outs), (ins target:$disp), "", [(br bb:$disp)]>,
               PseudoInstExpansion<(BR R31, target:$disp)>, Sched<[WriteJmp]>;

let isBarrier = 1, isReturn = 1, isTerminator = 1 in
def PseudoRet : PseudoInstSw64<(outs), (ins), "", [(Sw64Ret)]>,
                PseudoInstExpansion<(RET R31, R26, 1)>, Sched<[WriteJmp]>;

////4.2.2 uncondition shift
/////////////////////////////////////////////////////////
//Branching
/////////////////////////////////////////////////////////
let isBranch = 1, isTerminator = 1, hasCtrlDep = 1 in{
class br_icc<bits<6> opc, string asmstr>
    : BForm<opc, (ins GPRC:$RA, target:$disp), (outs),
            asmstr, "$RA,${disp}">;
class br_fcc<bits<6> opc, string asmstr>
    : BForm<opc, (ins F8RC:$RA, target:$disp), (outs),
            asmstr, "$RA,${disp}">;
}
def BEQ  : br_icc<0x30, "beq">;
def BGE  : br_icc<0x35, "bge">;
def BGT  : br_icc<0x34, "bgt">;
def BLBC : br_icc<0x36, "blbc">;
def BLBS : br_icc<0x37, "blbs">;
def BLE  : br_icc<0x33, "ble">;
def BLT  : br_icc<0x32, "blt">;
def BNE  : br_icc<0x31, "bne">;

//Branches, float
def FBEQ : br_fcc<0x38, "fbeq">;
def FBGE : br_fcc<0x3D, "fbge">;
def FBGT : br_fcc<0x3C, "fbgt">;
def FBLE : br_fcc<0x3B, "fble">;
def FBLT : br_fcc<0x3A, "fblt">;
def FBNE : br_fcc<0x39, "fbne">;
//4.3 load and store instruction
//4.3.1 load integer

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class load_ri<string opstr, bits<6> opcode, RegisterClass regtype,
              SDPatternOperator loadop>
    : MForm<opcode, (ins s64imm:$DISP, GPRC:$RB), (outs regtype:$RA),
            opstr, "$RA,${DISP}(${RB})",
            [(set regtype:$RA,
                (loadop (add GPRC:$RB, immSExt16:$DISP)))]>;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 ,Constraints = "$RB = $wback,@earlyclobber $wback" in
class load_ri1<string opstr, bits<6> opcode, bits<4> func,
               RegisterClass regtype, SDPatternOperator loadop>
    : MFuncForm<opcode, func, (ins GPRC:$RB, s12imm:$disp), (outs regtype:$RA, GPRC:$wback),
           opstr,"$RA,${disp}(${RB})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class store_ri<string opstr, bits<6> opcode, RegisterClass regtype,
               SDPatternOperator storeop>
    : MForm<opcode, (ins regtype:$RA, s64imm:$DISP, GPRC:$RB), (outs),
            opstr, "$RA,${DISP}(${RB})",
            [(storeop regtype:$RA,
               (add GPRC:$RB, immSExt16:$DISP))]>;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 ,Constraints = "$RB = $wback,@earlyclobber $wback" in
class store_ri1<string opstr, bits<6> opcode, bits<4> func,
                RegisterClass regtype, SDPatternOperator storeop>
    : MFuncForm<opcode, func, (ins regtype:$RA, GPRC:$RB, s12imm:$disp), (outs GPRC:$wback),
            opstr, "$RA,${disp}(${RB})">;

// integer load
def LDL  : load_ri<"ldl",  0x23, GPRC, load>;
def LDW  : load_ri<"ldw",  0x22, GPRC, sextloadi32>;
def LDHU : load_ri<"ldhu", 0x21, GPRC, zextloadi16>;
def LDBU : load_ri<"ldbu", 0x20, GPRC, zextloadi8>;
def LDL_A  : load_ri1<"ldl_a",  0x1E, 0x3, GPRC, load>;
def LDW_A  : load_ri1<"ldw_a",  0x1E, 0x2, GPRC, sextloadi32>;
def LDHU_A : load_ri1<"ldhu_a", 0x1E, 0x1, GPRC, zextloadi16>;
def LDBU_A : load_ri1<"ldbu_a", 0x1E, 0x0, GPRC, zextloadi8>;

// float load
def LDS : load_ri<"flds", 0x26, F4RC, load>;
def LDD : load_ri<"fldd", 0x27, F8RC, load>;
def LDS_A : load_ri1<"flds_a", 0x1E, 0x4, F4RC, load>;
def LDD_A : load_ri1<"fldd_a", 0x1E, 0x5, F8RC, load>;

// integer store
def STL : store_ri<"stl", 0x2B, GPRC, store>;
def STW : store_ri<"stw", 0x2A, GPRC, truncstorei32>;
def STH : store_ri<"sth", 0x29, GPRC, truncstorei16>;
def STB : store_ri<"stb", 0x28, GPRC, truncstorei8>;
def STL_A : store_ri1<"stl_a", 0x1E, 0x9, GPRC, store>;
def STW_A : store_ri1<"stw_a", 0x1E, 0x8, GPRC, truncstorei32>;
def STH_A : store_ri1<"sth_a", 0x1E, 0x7, GPRC, truncstorei16>;
def STB_A : store_ri1<"stb_a", 0x1E, 0x6, GPRC, truncstorei8>;

// float store
def STS : store_ri<"fsts", 0x2E, F4RC, store>;
def STD : store_ri<"fstd", 0x2F, F8RC, store>;
def STS_A : store_ri1<"fsts_a", 0x1E, 0xA, F4RC, store>;
def STD_A : store_ri1<"fstd_a", 0x1E, 0xB, F8RC, store>;

// imm inst
def LDA :  MForm<0x3E,(ins s64imm:$DISP, GPRC:$RB) , (outs GPRC:$RA),
                 "ldi", "$RA,${DISP}(${RB})",
                 [(set GPRC:$RA, (add GPRC:$RB, immSExt16:$DISP))]>;
def LDAH : MForm<0x3F,(ins s64imm:$DISP, GPRC:$RB) , (outs GPRC:$RA),
                 "ldih", "$RA,${DISP}(${RB})", []>;

let Uses = [R29] in {
def LOADgprel : PseudoInstSw64<(outs GPRC:$dst), (ins s64imm:$addr), "",
    [(set GPRC:$dst, (Sw64_gprel tglobaladdr:$addr))]>, Sched<[WriteLD]>;

def LOADconstant : PseudoInstSw64<(outs GPRC:$dst), (ins s64imm:$addr), "", []>,
                   Sched<[WriteAdrLD]>;

def LOADlit : PseudoInstSw64<(outs GPRC:$dst), (ins s64imm:$addr), "",
    [(set GPRC:$dst, (Sw64_rellit tglobaladdr:$addr))]>, Sched<[WriteLD]>;

def LOADlitSym : PseudoInstSw64<(outs GPRC:$dst), (ins s64imm:$addr), "",
    [(set GPRC:$dst, (Sw64_rellit texternalsym:$addr))]>, Sched<[WriteLD]>;


// The MOVaddr instruction should match only when the add is not folded
// into a load or store address.
def MOVaddrGP
: PseudoInstSw64<(outs GPRC:$dst), (ins i64imm:$hi, i64imm:$low), "",
  [(set GPRC:$dst, (Sw64ldi (Sw64ldih tglobaladdr:$hi),
                     tglobaladdr:$low))]>, Sched<[WriteAdrAdr]>;

def MOVaddrCP
: PseudoInstSw64<(outs GPRC:$dst), (ins i64imm:$hi, i64imm:$low), "",
  [(set GPRC:$dst, (Sw64ldi (Sw64ldih tconstpool:$hi),
                     tconstpool:$low))]>, Sched<[WriteAdrAdr]>;

def MOVaddrBA
: PseudoInstSw64<(outs GPRC:$dst), (ins i64imm:$hi, i64imm:$low), "",
  [(set GPRC:$dst, (Sw64ldi (Sw64ldih tblockaddress:$hi),
                     tblockaddress:$low))]>, Sched<[WriteAdrAdr]>;

def MOVaddrEXT
: PseudoInstSw64<(outs GPRC:$dst), (ins i64imm:$hi, i64imm:$low), "",
  [(set GPRC:$dst, (Sw64ldi (Sw64ldih texternalsym:$hi),
                     texternalsym:$low))]>, Sched<[WriteAdrAdr]>;

def MOVaddrJT
: PseudoInstSw64<(outs GPRC:$dst), (ins i64imm:$hi, i64imm:$low), "",
  [(set GPRC:$dst, (Sw64ldi (Sw64ldih tjumptable:$hi),
                     tjumptable:$low))]>, Sched<[WriteAdrAdr]>;
}

//TODO: for core3 target, sw64 need gpdisp to get global address table
// we cannot change Prologue disp, or it will cause error
let isBarrier = 1, hasNoSchedulingInfo = 1, Defs = [R29] in
def MOVProgPCGp : PseudoInstSw64<(outs),
                  (ins s16imm:$DISP, s16imm:$NUM , GPRC:$dst_reg), "", []>,
									 Sched<[WriteAdrAdr]>;

let Defs = [R29] in
def MOVaddrPCGp : PseudoInstSw64<(outs),
                  (ins s16imm:$DISP, s16imm:$NUM , GPRC:$dst_reg), "", []>,
									 Sched<[WriteAdrAdr]>;

// def patterns

def : Pat<(Sw64JmpLink tglobaladdr:$func),
          (PseudoCall tglobaladdr:$func)>;
def : Pat<(Sw64JmpLink texternalsym:$func),
          (PseudoCall texternalsym:$func)>;

def : Pat<(Sw64_LDAWithChain GPRC:$RB, immSExt16:$DISP),
          (LDA immSExt16:$DISP, GPRC:$RB)>;


def : Pat<(i64 (extloadi8 (add GPRC:$RB, immSExt16:$DISP))),
          (LDBU   immSExt16:$DISP, GPRC:$RB)>;
def : Pat<(i64 (extloadi16 (add GPRC:$RB, immSExt16:$DISP))),
          (LDHU  immSExt16:$DISP, GPRC:$RB)>;
def : Pat<(i64 (extloadi32 (add GPRC:$RB, immSExt16:$DISP))),
          (LDW   immSExt16:$DISP, GPRC:$RB)>;
def : Pat<(i64 (zextloadi8 (add GPRC:$RB, immSExt16:$DISP))),
          (LDBU   immSExt16:$DISP, GPRC:$RB)>;
def : Pat<(i64 (zextloadi16 (add GPRC:$RB, immSExt16:$DISP))),
          (LDHU   immSExt16:$DISP, GPRC:$RB)>;
def : Pat<(i64 (sextloadi32 (add GPRC:$RB, immSExt16:$DISP))),
          (LDW   immSExt16:$DISP, GPRC:$RB)>;

def : Pat<(i64 (load GPRC:$addr)),
          (LDL  0, GPRC:$addr)>;
def : Pat<(i64 (sextloadi32 GPRC:$addr)),
          (LDW  0, GPRC:$addr)>;
def : Pat<(i64 (extloadi32 GPRC:$addr)),
          (LDW  0, GPRC:$addr)>;
def : Pat<(i64 (zextloadi16 GPRC:$addr)),
          (LDHU 0, GPRC:$addr)>;
def : Pat<(i64 (extloadi16 GPRC:$addr)),
          (LDHU 0, GPRC:$addr)>;
def : Pat<(i64 (zextloadi8 GPRC:$addr)),
          (LDBU 0, GPRC:$addr)>;
def : Pat<(i64 (extloadi8 GPRC:$addr)),
          (LDBU 0, GPRC:$addr)>;

//4.3.5 s float load
def : Pat<(f32 (load GPRC:$addr)),
          (LDS  0, GPRC:$addr)>;
def : Pat<(f64 (load GPRC:$addr)),
          (LDD  0, GPRC:$addr)>;
//4.3.3 store integer
def : Pat<(store GPRC:$DATA, GPRC:$addr),
          (STL  GPRC:$DATA, 0, GPRC:$addr)>;
def : Pat<(truncstorei32 GPRC:$DATA, GPRC:$addr),
          (STW  GPRC:$DATA, 0, GPRC:$addr)>;
def : Pat<(truncstorei16 GPRC:$DATA, GPRC:$addr),
          (STH GPRC:$DATA, 0, GPRC:$addr)>;
def : Pat<(truncstorei8 GPRC:$DATA, GPRC:$addr),
          (STB GPRC:$DATA, 0, GPRC:$addr)>;
def : Pat<(store F4RC:$DATA, GPRC:$addr),
          (STS  F4RC:$DATA, 0, GPRC:$addr)>;
def : Pat<(store F8RC:$DATA, GPRC:$addr),
          (STD  F8RC:$DATA, 0, GPRC:$addr)>;
multiclass LdPat<PatFrag LoadOp, InstSw64 Inst, ValueType VT> {
  def : Pat<(VT (LoadOp GPRC:$addr)), (Inst 0, GPRC:$addr)>;
  def : Pat<(VT (LoadOp AddrFI:$addr)), (Inst 0, AddrFI:$addr)>;
  def : Pat<(VT (LoadOp (add GPRC:$addr, immSExt16:$DISP))),
            (Inst immSExt16:$DISP, GPRC:$addr)>;
   def : Pat<(VT (LoadOp (add AddrFI:$addr, immSExt16:$DISP))),
            (Inst immSExt16:$DISP, AddrFI:$addr)>;
  def : Pat<(VT (LoadOp (IsOrAdd AddrFI:$addr, immSExt16:$DISP))),
            (Inst immSExt16:$DISP, AddrFI:$addr)>;
}

multiclass StPat<PatFrag StoreOp, InstSw64 Inst, RegisterClass StTy> {
  def : Pat<(StoreOp StTy:$rs2, GPRC:$rs1), (Inst StTy:$rs2, 0, GPRC:$rs1)>;
  def : Pat<(StoreOp StTy:$rs2, AddrFI:$rs1), (Inst StTy:$rs2, 0, AddrFI:$rs1)>;
  def : Pat<(StoreOp StTy:$rs2, (add GPRC:$rs1, immSExt16:$DISP)),
            (Inst StTy:$rs2, immSExt16:$DISP, GPRC:$rs1)>;
  def : Pat<(StoreOp StTy:$rs2, (add AddrFI:$rs1, immSExt16:$DISP)),
            (Inst StTy:$rs2, immSExt16:$DISP, AddrFI:$rs1)>;
  def : Pat<(StoreOp StTy:$rs2, (IsOrAdd AddrFI:$rs1, immSExt16:$DISP)),
            (Inst StTy:$rs2, immSExt16:$DISP, AddrFI:$rs1)>;
}

defm : LdPat<load, LDL, i64>;
defm : LdPat<extloadi32, LDW, i64>;
defm : LdPat<sextloadi32, LDW, i64>;
defm : LdPat<extloadi16, LDHU, i64>;
defm : LdPat<zextloadi16, LDHU, i64>;
defm : LdPat<extloadi8, LDBU, i64>;
defm : LdPat<zextloadi8, LDBU, i64>;
defm : LdPat<load, LDS, f32>;
defm : LdPat<load, LDD, f64>;

defm : StPat<store, STL, GPRC>;
defm : StPat<truncstorei32, STW, GPRC>;
defm : StPat<truncstorei16, STH, GPRC>;
defm : StPat<truncstorei8, STB, GPRC>;
defm : StPat<store, STS, F4RC>;
defm : StPat<store, STD, F8RC>;
//4.5 int caculate instruction
multiclass inst_rr_ri<bits<8> fun, string opstr,
                      SDPatternOperator OpNode = null_frag> {
  def r : OForm <0x10, fun, (ins GPRC:$RA, GPRC:$RB), (outs GPRC:$RC),
                 opstr, "$RA,$RB,$RC",
                 [(set GPRC:$RC, (OpNode GPRC:$RA, GPRC:$RB))]>;
  def i : OFormL<0x12, fun,(ins GPRC:$RA, u8imm:$L), (outs GPRC:$RC),
                 opstr, "$RA,$L,$RC",
                 [(set GPRC:$RC, (OpNode GPRC:$RA, (i64 immUExt8:$L)))]>;
}

let Predicates = [EnableCrcInst] in
class inst_rr_r<bits<8> fun, string opstr,
                SDPatternOperator OpNode>
    :  OForm <0x10, fun, (ins GPRC:$RA, GPRC:$RB), (outs GPRC:$RC),
                 opstr, "$RA,$RB,$RC",
                 [(set GPRC:$RC, (OpNode GPRC:$RA, GPRC:$RB))]>;

multiclass inst_rr_rin<bits<8> funL, bits<8> funQ, string opstr,
                      SDPatternOperator OpNode > {
  def Q : OForm <0x10, funQ, (ins GPRC:$RA, GPRC:$RB), (outs GPRC:$RC),
                 opstr # "l", "$RA,$RB,$RC",
                 [(set GPRC:$RC, (OpNode GPRC:$RA, GPRC:$RB))]>;
  def L : OForm <0x10, funL, (ins GPRC:$RA, GPRC:$RB), (outs GPRC:$RC),
                 opstr # "w", "$RA,$RB,$RC",
                 [(set GPRC:$RC, (OpNode GPRC:$RA, GPRC:$RB))]>;
}

class inst_rr_13<bits<8> fun, string opstr,
                 SDPatternOperator OpNode=null_frag>
    : OFormI <0x10, fun, (ins s13imm:$L), (outs GPRC:$RC),
             opstr, "$L, $RC",
             [(set GPRC:$RC, (OpNode immUExt13:$L))]>;

let RA = 31 in
class inst_rr_2<bits<8> fun, string opstr, ValueType vt>
    : OForm <0x10, fun, (ins GPRC:$RB), (outs GPRC:$RC),
             opstr, "$RB, $RC",
             [(set GPRC:$RC, (sext_inreg GPRC:$RB, vt))]>;

let RA = 31 in
class inst_rr_3<bits<8> fun, string opstr,
                SDPatternOperator OpNode=null_frag>
    : OForm <0x10, fun, (ins GPRC:$RB), (outs GPRC:$RC),
             opstr, "$RB, $RC",
             [(set GPRC:$RC, (OpNode GPRC:$RB))]>;

multiclass inst_lw<bits<8> funL, bits<8> funQ, string opstr,
                   SDPatternOperator OpNode> {
  defm L : inst_rr_ri<funL, opstr # "w">;
  defm Q : inst_rr_ri<funQ, opstr # "l">;
}

defm ADD  : inst_lw<0x00, 0x08,"add",  add>;
defm SUB  : inst_lw<0x01, 0x09,"sub",  sub>;
defm S4ADD: inst_lw<0x02, 0x0a,"s4add", add4>;
defm S4SUB: inst_lw<0x03, 0x0b,"s4sub", sub4>;
defm S8ADD: inst_lw<0x04, 0x0c,"s8add", add8>;
defm S8SUB: inst_lw<0x05, 0x0d,"s8sub", sub8>;
defm MUL  : inst_lw<0x10, 0x18,"mul",  mul>;

let mayRaiseFPException = 1, hasSideEffects = 1 in {
  defm DIV  : inst_rr_rin<0x11, 0x1a, "div",  sdiv>;
  defm UDIV : inst_rr_rin<0x12, 0x1b, "udiv",  udiv>;
  defm REM  : inst_rr_rin<0x13, 0x1c, "rem",  srem>;
  defm UREM : inst_rr_rin<0x14, 0x1d, "urem",  urem>;
}

def ADDPI : inst_rr_13<0x1e, "addpi", Sw64_addpi>;
def ADDPIS : inst_rr_13<0x1f, "addpis", Sw64_addpis>;

defm SBT  : inst_rr_ri<0x2d, "sbt",  Sw64_sbt>;
defm CBT  : inst_rr_ri<0x2e, "cbt",  Sw64_cbt>;

defm UMULH : inst_rr_ri<0x19, "umulh", mulhu>;

let RA=31 in
class inst_ct<bits<8> func, string opstr, SDNode OpNode>
    : OForm<0x10, func, (ins GPRC:$RB), (outs GPRC:$RC),
            opstr, "$RB,$RC", [(set GPRC:$RC, (OpNode GPRC:$RB))]>;

def CTTZ : inst_ct<0x5A, "cttz", cttz>;
def CTLZ : inst_ct<0x59, "ctlz", ctlz>;
def CTPOP : inst_ct<0x58, "ctpop", ctpop>;

defm ZAP : inst_rr_ri<0x68, "zap">;
defm ZAPNOT : inst_rr_ri<0x69, "zapnot">;

def SEXTB : inst_rr_2<0x6A, "sextb", i8>;
def SEXTH : inst_rr_2<0x6B, "sexth", i16>;

//4.5.2 integer cmp
defm CMPEQ : inst_rr_ri<0x28, "cmpeq", seteq>;
defm CMPLT : inst_rr_ri<0x29, "cmplt", setlt>;
defm CMPLE : inst_rr_ri<0x2A, "cmple", setle>;
defm CMPULT : inst_rr_ri<0x2B, "cmpult", setult>;
defm CMPULE : inst_rr_ri<0x2C, "cmpule", setule>;

//4.5.3 integer order
defm AND   : inst_rr_ri<0x38, "and",   and>;
defm BIC   : inst_rr_ri<0x39, "bic",
     BinOpFrag<(and node:$LHS, (not node:$RHS))>>;
defm BIS   : inst_rr_ri<0x3A, "bis",   or>;
defm ORNOT : inst_rr_ri<0x3B, "ornot",
     BinOpFrag<(or node:$LHS, (not node:$RHS))>>;
defm XOR   : inst_rr_ri<0x3C, "xor",   xor>;
defm EQV   : inst_rr_ri<0x3D, "eqv",
     BinOpFrag<(not (xor node:$LHS, node:$RHS))>>;

//4.5.4 integer move position
defm SL    : inst_rr_ri<0x48, "sll",   shl>;
defm SRA   : inst_rr_ri<0x4A, "sra",   sra>;
defm SRL   : inst_rr_ri<0x49, "srl",   srl>;
defm ROLL  : inst_rr_ri<0x4B, "roll",   rotl>;
defm SLLW  : inst_rr_ri<0x4C, "sllw">;
defm SRLW  : inst_rr_ri<0x4D, "srlw">;
defm SRAW  : inst_rr_ri<0x4E, "sraw">;
defm ROLW  : inst_rr_ri<0x4F, "rolw",   Sw64_rolw>;

def sexti32 : ComplexPattern<i64, 1, "selectSExti32">;

def zexti32 : ComplexPattern<i64, 1, "selectZExti32">;

def : Pat<(srem (sexti32 (i64 GPRC:$RA)), (sexti32 (i64 GPRC:$RB))),
          (REML GPRC:$RA, GPRC:$RB)>;

def : Pat<(sext_inreg (sdiv GPRC:$RA, GPRC:$RB), i32),
          (DIVL GPRC:$RA, GPRC:$RB)>;

def : Pat<(sext_inreg (udiv (and GPRC:$RA,0xffffffff), (and GPRC:$RB,0xffffffff)), i32),
          (UDIVL GPRC:$RA, GPRC:$RB)>;

def : Pat<(sext_inreg (shl GPRC:$RA, GPRC:$RB), i32),
          (SLLWr GPRC:$RA, GPRC:$RB)>,
          Requires<[EnableIntShift, HasCore4]>;

def : Pat<(sext_inreg (shl GPRC:$RA, (i64 immUExt8:$L)), i32),
          (SLLWi GPRC:$RA, (i64 immUExt8:$L))>,
          Requires<[EnableIntShift, HasCore4]>;

def : Pat<(sext_inreg (srl GPRC:$RA, GPRC:$RB), i32),
          (SRLWr GPRC:$RA, GPRC:$RB)>,
          Requires<[EnableIntShift, HasCore4]>;

def : Pat<(srl (i64 (zexti32 GPRC:$RA)), (i64 immUExt8:$L)),
          (SRLWi GPRC:$RA, (i64 immUExt8:$L))>,
          Requires<[EnableIntShift, HasCore4]>;

def : Pat<(sra (i64 (sexti32 GPRC:$RA)), (i64 GPRC:$RB)),
          (SRAWr GPRC:$RA, GPRC:$RB)>,
          Requires<[EnableIntShift, HasCore4]>;

def : Pat<(sra (i64 (sexti32 GPRC:$RA)), (i64 immUExt8:$L)),
          (SRAWi GPRC:$RA, (i64 immUExt8:$L))>,
          Requires<[EnableIntShift, HasCore4]>;

def : Pat<(sext_inreg (rotl GPRC:$RA, GPRC:$RB), i32),
          (ROLWr GPRC:$RA, GPRC:$RB)>,
          Requires<[EnableIntShift, HasCore4]>;

def : Pat<(sext_inreg (rotl GPRC:$RA, (i64 immUExt8:$L)), i32),
          (ROLWi GPRC:$RA, (i64 immUExt8:$L))>,
          Requires<[EnableIntShift, HasCore4]>;

multiclass PatGprInst<SDPatternOperator OpNode, string Inst>{
def : Pat<(OpNode GPRC:$rs1, GPRC:$rs2),
            (!cast<Instruction>(Inst # "Qr")
                  GPRC:$rs1, GPRC:$rs2)>;

def : Pat<(intop (OpNode GPRC:$rs1, GPRC:$rs2)),
            (!cast<Instruction>(Inst # "Lr")
                  GPRC:$rs1, GPRC:$rs2)>;

def : Pat<(OpNode GPRC:$rs1, immUExt8:$imm8),
            (!cast<Instruction>(Inst # "Qi")
                  GPRC:$rs1, immUExt8:$imm8)>;

def : Pat<(intop (OpNode GPRC:$rs1, immUExt8:$imm8)),
            (!cast<Instruction>(Inst # "Li")
                  GPRC:$rs1, immUExt8:$imm8)>;
}

multiclass PatGprInstn<SDPatternOperator OpNode, string Inst>{
def : Pat<(OpNode GPRC:$rs1, GPRC:$rs2),
            (!cast<Instruction>(Inst # "Q")
                  GPRC:$rs1, GPRC:$rs2)>;

def : Pat<(intop (OpNode GPRC:$rs1, GPRC:$rs2)),
            (!cast<Instruction>(Inst # "L")
                  GPRC:$rs1, GPRC:$rs2)>;
}

defm : PatGprInst<add, "ADD">;
defm : PatGprInst<sub, "SUB">;
defm : PatGprInst<mul, "MUL">;
defm : PatGprInst<add4, "S4ADD">;
defm : PatGprInst<add8, "S8ADD">;
defm : PatGprInst<sub4, "S4SUB">;
defm : PatGprInst<sub8, "S8SUB">;

//Const cases since legalize does sub x, int -> add x, inv(int) + 1
def : Pat<(intop (add GPRC:$RA, immUExt8neg:$L)),
          (SUBLi GPRC:$RA, immUExt8neg:$L)>;
def : Pat<(add GPRC:$RA, immUExt8neg:$L), (SUBQi GPRC:$RA, immUExt8neg:$L)>;
def : Pat<(intop (add4 GPRC:$RA, immUExt8neg:$L)),
          (S4SUBLi GPRC:$RA, immUExt8neg:$L)>;
def : Pat<(add4 GPRC:$RA, immUExt8neg:$L), (S4SUBQi GPRC:$RA, immUExt8neg:$L)>;
def : Pat<(intop (add8 GPRC:$RA, immUExt8neg:$L)),
          (S8SUBLi GPRC:$RA, immUExt8neg:$L)>;
def : Pat<(add8 GPRC:$RA, immUExt8neg:$L), (S8SUBQi GPRC:$RA, immUExt8neg:$L)>;
// Define the pattern that produces ZAPNOTi.

def : Pat<(cttz_zero_undef i64:$Rn), (CTTZ $Rn)>;

def : Pat<(zappat:$imm GPRC:$RA),
          (ZAPNOTi GPRC:$RA, (iZAPX GPRC:$imm))>;

def : Pat<(sext_inreg GPRC:$RB, i32),
          (ADDLi GPRC:$RB, 0)>;

def : Pat<(sext_inreg (add GPRC:$RA, GPRC:$RB), i32),
          (ADDLr GPRC:$RA, GPRC:$RB)>;

def : Pat<(setueq GPRC:$X, GPRC:$Y), (CMPEQr GPRC:$X, GPRC:$Y)>;
def : Pat<(setueq GPRC:$X, immUExt8:$Y), (CMPEQi GPRC:$X, immUExt8:$Y)>;
def : Pat<(setueq immUExt8:$Y, GPRC:$X), (CMPEQi GPRC:$X, immUExt8:$Y)>;

def : Pat<(seteq GPRC:$X, GPRC:$Y), (CMPEQr GPRC:$X, GPRC:$Y)>;
def : Pat<(seteq GPRC:$X, immUExt8:$Y), (CMPEQi GPRC:$X, immUExt8:$Y)>;
def : Pat<(seteq immUExt8:$Y, GPRC:$X), (CMPEQi GPRC:$X, immUExt8:$Y)>;

def : Pat<(setugt GPRC:$X, GPRC:$Y), (CMPULTr GPRC:$Y, GPRC:$X)>;
def : Pat<(setugt immUExt8:$X, GPRC:$Y), (CMPULTi GPRC:$Y, immUExt8:$X)>;
def : Pat<(brcond (i64 (setugt GPRC:$Y, immUExt8:$X)), bb:$DISP), (BEQ (CMPULEi GPRC:$Y, immUExt8:$X), bb:$DISP)>;

def : Pat<(setuge GPRC:$X, GPRC:$Y), (CMPULEr GPRC:$Y, GPRC:$X)>;
def : Pat<(setuge immUExt8:$X, GPRC:$Y), (CMPULEi GPRC:$Y, immUExt8:$X)>;
def : Pat<(brcond (i64 (setuge GPRC:$Y, immUExt8:$X)), bb:$DISP), (BEQ (CMPULTi GPRC:$Y, immUExt8:$X), bb:$DISP)>;

def : Pat<(setgt GPRC:$X, GPRC:$Y), (CMPLTr GPRC:$Y, GPRC:$X)>;
def : Pat<(setgt immUExt8:$X, GPRC:$Y), (CMPLTi GPRC:$Y, immUExt8:$X)>;
def : Pat<(brcond (i64 (setgt GPRC:$Y, immUExt8:$X)), bb:$DISP), (BEQ (CMPLEi GPRC:$Y, immUExt8:$X), bb:$DISP)>;

def : Pat<(setge GPRC:$X, GPRC:$Y), (CMPLEr GPRC:$Y, GPRC:$X)>;
def : Pat<(setge immUExt8:$X, GPRC:$Y), (CMPLEi GPRC:$Y, immUExt8:$X)>;
def : Pat<(brcond (i64 (setgt GPRC:$Y, immUExt8:$X)), bb:$DISP), (BEQ (CMPLTi GPRC:$Y, immUExt8:$X), bb:$DISP)>;

def : Pat<(setne GPRC:$X, GPRC:$Y), (CMPEQi (CMPEQr GPRC:$X, GPRC:$Y), 0)>;
def : Pat<(setne GPRC:$X, immUExt8:$Y),
          (CMPEQi (CMPEQi GPRC:$X, immUExt8:$Y), 0)>;
def : Pat<(brcond (i64 (setne GPRC:$X, immUExt8:$Y)), bb:$DISP), (BEQ (CMPEQi GPRC:$X, immUExt8:$Y), bb:$DISP)>;
def : Pat<(brcond (i64 (setne immUExt8:$Y, GPRC:$X)), bb:$DISP), (BEQ (CMPEQi GPRC:$X, immUExt8:$Y), bb:$DISP)>;

def : Pat<(setune GPRC:$X, GPRC:$Y), (CMPEQi (CMPEQr GPRC:$X, GPRC:$Y), 0)>;
def : Pat<(setune GPRC:$X, immUExt8:$Y),
          (CMPEQi (CMPEQr GPRC:$X, immUExt8:$Y), 0)>;

//after put here because SLi
//Stupid crazy arithmetic stuff:
let AddedComplexity = 1 in {
def : Pat<(mul GPRC:$RA, 5), (S4ADDQr GPRC:$RA, GPRC:$RA)>;
def : Pat<(sext_inreg (mul GPRC:$RA, 5), i32), (S4ADDLr GPRC:$RA, GPRC:$RA)>;
def : Pat<(mul GPRC:$RA, 9), (S8ADDQr GPRC:$RA, GPRC:$RA)>;
def : Pat<(sext_inreg (mul GPRC:$RA, 9), i32), (S8ADDLr GPRC:$RA, GPRC:$RA)>;
def : Pat<(mul GPRC:$RA, 3), (S4SUBQr GPRC:$RA, GPRC:$RA)>;
def : Pat<(sext_inreg (mul GPRC:$RA, 3), i32), (S4SUBLr GPRC:$RA, GPRC:$RA)>;
def : Pat<(mul GPRC:$RA, 7), (S8SUBQr GPRC:$RA, GPRC:$RA)>;
def : Pat<(sext_inreg (mul GPRC:$RA, 7), i32), (S8SUBLr GPRC:$RA, GPRC:$RA)>;
//slight tree expansion if we are multiplying near to a power of 2

def : Pat<(mul GPRC:$RA, immRem1:$imm),
          (ADDQr (SLi GPRC:$RA, (nearP2X immRem1:$imm)), GPRC:$RA)>;
def : Pat<(mul GPRC:$RA, immRem2:$imm),
          (ADDQr (SLi GPRC:$RA, (nearP2X immRem2:$imm)),
                 (ADDQr GPRC:$RA, GPRC:$RA))>;
def : Pat<(mul GPRC:$RA, immRem3:$imm),
          (ADDQr (SLi GPRC:$RA, (nearP2X immRem3:$imm)),
                 (S4SUBQr GPRC:$RA, GPRC:$RA))>;
def : Pat<(mul GPRC:$RA, immRem4:$imm),
          (S4ADDQr GPRC:$RA, (SLi GPRC:$RA, (nearP2X immRem4:$imm)))>;
def : Pat<(mul GPRC:$RA, immRem5:$imm),
          (ADDQr (SLi GPRC:$RA, (nearP2X immRem5:$imm)),
                 (S4ADDQr GPRC:$RA, GPRC:$RA))>;
def : Pat<(mul GPRC:$RA, immRemP2:$imm),
          (ADDQr (SLi GPRC:$RA, (nearP2X immRemP2:$imm)),
                 (SLi GPRC:$RA, (nearP2RemX immRemP2:$imm)))>;

} //Added complexity
//4.5.5 integer cond select
// RA:COND RB:TRUE RC:FALSE RD:DEST
multiclass select_inst< bits<3> fun, string opstr, SDPatternOperator OpNode> {
def r : OForm4 <0x11, fun, (ins GPRC:$RA, GPRC:$RB, GPRC:$RC),
                (outs GPRC:$RD), opstr, "$RA,$RB,$RC,$RD",
                [(set GPRC:$RD,
                    (select (i64 (OpNode GPRC:$RA)), GPRC:$RB, GPRC:$RC))]>;
def i : OForm4L<0x13, fun, (ins GPRC:$RA, u8imm:$L, GPRC:$RC),
                (outs GPRC:$RD), opstr, "$RA,$L,$RC,$RD",
                [(set GPRC:$RD,
                    (select (i64 (OpNode GPRC:$RA)), immUExt8:$L, GPRC:$RC))]>;
}

defm SELEQ  : select_inst<0x0, "seleq",  CmpOpFrag<(seteq node:$R, 0)>>;
defm SELNE  : select_inst<0x5, "selne",  CmpOpFrag<(setne node:$R, 0)>>;
defm SELLT  : select_inst<0x4, "sellt",  CmpOpFrag<(setlt node:$R, 0)>>;
defm SELLE  : select_inst<0x3, "selle",  CmpOpFrag<(setle node:$R, 0)>>;
defm SELGT  : select_inst<0x2, "selgt",  CmpOpFrag<(setgt node:$R, 0)>>;
defm SELGE  : select_inst<0x1, "selge",  CmpOpFrag<(setge node:$R, 0)>>;
defm SELLBC : select_inst<0x6, "sellbc", CmpOpFrag<(xor   node:$R, 1)>>;
defm SELLBS : select_inst<0x7, "sellbs", CmpOpFrag<(and   node:$R, 1)>>;

//General pattern for select
def : Pat<(select GPRC:$which, GPRC:$src1, GPRC:$src2),
          (SELNEr GPRC:$which, GPRC:$src1, GPRC:$src2)>;
def : Pat<(select GPRC:$which, GPRC:$src1, immUExt8:$src2),
          (SELEQi GPRC:$which, immUExt8:$src2, GPRC:$src1)>;
def : Pat<(select (i64 (setne GPRC:$RCOND, 0)), GPRC:$RTRUE, immUExt8:$RFALSE),
          (SELEQi GPRC:$RCOND, immUExt8:$RFALSE, GPRC:$RTRUE)>;
def : Pat<(select (i64 (setgt GPRC:$RCOND, 0)), GPRC:$RTRUE, immUExt8:$RFALSE),
          (SELLEi GPRC:$RCOND, immUExt8:$RFALSE, GPRC:$RTRUE)>;
def : Pat<(select (i64 (setge GPRC:$RCOND, 0)), GPRC:$RTRUE, immUExt8:$RFALSE),
          (SELLTi GPRC:$RCOND, immUExt8:$RFALSE, GPRC:$RTRUE)>;
def : Pat<(select (i64 (setlt GPRC:$RCOND, 0)), GPRC:$RTRUE, immUExt8:$RFALSE),
          (SELGEi GPRC:$RCOND, immUExt8:$RFALSE, GPRC:$RTRUE)>;
def : Pat<(select (i64 (setle GPRC:$RCOND, 0)), GPRC:$RTRUE, immUExt8:$RFALSE),
          (SELGTi GPRC:$RCOND, immUExt8:$RFALSE, GPRC:$RTRUE)>;

def : Pat<(mulhs GPRC:$RA, GPRC:$RB),
          (SUBQr (UMULHr GPRC:$RA, GPRC:$RB),
                 (ADDQr (SELGEr GPRC:$RB, R31, GPRC:$RA),
                        (SELGEr GPRC:$RA, R31, GPRC:$RB)))>;

defm CMPBGE : inst_rr_ri<0x6C, "cmpgeb">;

defm EXTLB : inst_rr_ri<0x50, "extlb">;
defm EXTLH : inst_rr_ri<0x51, "extlh">;
defm EXTLW : inst_rr_ri<0x52, "extlw">;
defm EXTLL : inst_rr_ri<0x53, "extll">;

defm EXTHB : inst_rr_ri<0x54, "exthb">;
defm EXTHH : inst_rr_ri<0x55, "exthh">;
defm EXTHW : inst_rr_ri<0x56, "exthw">;
defm EXTHL : inst_rr_ri<0x57, "exthl">;

defm INSLB: inst_rr_ri<0x40, "inslb">;
defm INSLH: inst_rr_ri<0x41, "inslh">;
defm INSLW: inst_rr_ri<0x42, "inslw">;
defm INSLL: inst_rr_ri<0x43, "insll">;
defm INSHB: inst_rr_ri<0x44, "inshb">;
defm INSHH: inst_rr_ri<0x45, "inshh">;
defm INSHW: inst_rr_ri<0x46, "inshw">;
defm INSHL: inst_rr_ri<0x47, "inshl">;

def REVBH: inst_rr_3<0x5B, "revbh", Sw64_revbh>;
def REVBW: inst_rr_3<0x5C, "revbw", Sw64_revbw>;
def REVBL: inst_rr_3<0x5D, "revbl", bswap>;

def : Pat<(sra (bswap GPRC:$RB), (i64 32)),
          (REVBW GPRC:$RB)>;
def : Pat<(sra (bswap GPRC:$RB), (i64 48)),
          (REVBH GPRC:$RB)>;
def : Pat<(srl (bswap GPRC:$RB), (i64 32)),
          (REVBW GPRC:$RB)>;
def : Pat<(srl (bswap GPRC:$RB), (i64 48)),
          (REVBH GPRC:$RB)>;

defm MASKLB: inst_rr_ri<0x60, "masklb">;
defm MASKLH: inst_rr_ri<0x61, "masklh">;
defm MASKLW: inst_rr_ri<0x62, "masklw">;
defm MASKLL: inst_rr_ri<0x63, "maskll">;
defm MASKHB: inst_rr_ri<0x64, "maskhb">;
defm MASKHH: inst_rr_ri<0x65, "maskhh">;
defm MASKHW: inst_rr_ri<0x66, "maskhw">;
defm MASKHL: inst_rr_ri<0x67, "maskhl">;

// 4.5.7 crc32 instruction
def CRC32B: inst_rr_r<0x20, "crc32b", Sw64_crc32b>;
def CRC32H: inst_rr_r<0x21, "crc32h", Sw64_crc32h>;
def CRC32W: inst_rr_r<0x22, "crc32w", Sw64_crc32w>;
def CRC32L: inst_rr_r<0x23, "crc32l", Sw64_crc32l>;
def CRC32CB: inst_rr_r<0x24, "crc32cb", Sw64_crc32cb>;
def CRC32CH: inst_rr_r<0x25, "crc32ch", Sw64_crc32ch>;
def CRC32CW: inst_rr_r<0x26, "crc32cw", Sw64_crc32cw>;
def CRC32CL: inst_rr_r<0x27, "crc32cl", Sw64_crc32cl>;

def : Pat<(Sw64_crc32b GPRC:$rs1, GPRC:$rs2),
          (CRC32B GPRC:$rs1, GPRC:$rs2)>,
	  Requires<[EnableCrcInst, HasCore4]>;
def : Pat<(Sw64_crc32h GPRC:$rs1, GPRC:$rs2),
          (CRC32H GPRC:$rs1, GPRC:$rs2)>,
	  Requires<[EnableCrcInst, HasCore4]>;
def : Pat<(Sw64_crc32w GPRC:$rs1, GPRC:$rs2),
          (CRC32W GPRC:$rs1, GPRC:$rs2)>,
	  Requires<[EnableCrcInst, HasCore4]>;
def : Pat<(Sw64_crc32l GPRC:$rs1, GPRC:$rs2),
          (CRC32L GPRC:$rs1, GPRC:$rs2)>,
	  Requires<[EnableCrcInst, HasCore4]>;
def : Pat<(Sw64_crc32cb GPRC:$rs1, GPRC:$rs2),
          (CRC32CB GPRC:$rs1, GPRC:$rs2)>,
	  Requires<[EnableCrcInst, HasCore4]>;
def : Pat<(Sw64_crc32ch GPRC:$rs1, GPRC:$rs2),
          (CRC32CH GPRC:$rs1, GPRC:$rs2)>,
	  Requires<[EnableCrcInst, HasCore4]>;
def : Pat<(Sw64_crc32cw GPRC:$rs1, GPRC:$rs2),
          (CRC32CW GPRC:$rs1, GPRC:$rs2)>,
	  Requires<[EnableCrcInst, HasCore4]>;
def : Pat<(Sw64_crc32cl GPRC:$rs1, GPRC:$rs2),
          (CRC32CL GPRC:$rs1, GPRC:$rs2)>,
	  Requires<[EnableCrcInst, HasCore4]>;

//4.6 float caculate instruction
//4.6.1 float caculate
class inst_fpu<bits<8>func , string opstr, RegisterClass regtype>
    : FPForm<0x18, func, (ins regtype:$RA, regtype:$RB),
             (outs regtype:$RC), opstr, "$RA,$RB,$RC">;

class inst_fpu_rr<bits<8>func , string opstr, RegisterClass regtype ,
                  SDPatternOperator OpNode>
    : FPForm<0x18, func, (ins regtype:$RA, regtype:$RB),
             (outs regtype:$RC), opstr, "$RA,$RB,$RC",
             [(set regtype:$RC, (OpNode regtype:$RA, regtype:$RB))]>;

class inst_fpu_rr_rev64<bits<8>func , string opstr,
                  SDPatternOperator OpNode>
    : FPForm<0x18, func, (ins F8RC:$RA, F8RC:$RB),
             (outs F8RC:$RC), opstr, "$RA,$RB,$RC",
             [(set F8RC:$RC, (OpNode F8RC:$RB, F8RC:$RA))]>;

let DecoderNamespace = "FP32" in {
class inst_fpu_rr_rev32<bits<8>func , string opstr,
                  SDPatternOperator OpNode>
    : FPForm<0x18, func, (ins F4RC:$RA, F4RC:$RB),
             (outs F4RC:$RC), opstr, "$RA,$RB,$RC",
             [(set F4RC:$RC, (OpNode F4RC:$RB, F4RC:$RA))]>;
}

multiclass inst_fpu_sd<bits<8>funcS, bits<8>funcD ,
                       string opstr, SDPatternOperator OpNode>{
  def S : inst_fpu_rr<funcS, opstr # "s", F4RC, OpNode>;
  def D : inst_fpu_rr<funcD, opstr # "d", F8RC, OpNode>;
}

defm ADD  : inst_fpu_sd<0x00, 0x01, "fadd", fadd>;
defm SUB  : inst_fpu_sd<0x02, 0x03, "fsub", fsub>;
defm MUL  : inst_fpu_sd<0x04, 0x05, "fmul", fmul>;
defm DIV  : inst_fpu_sd<0x06, 0x07, "fdiv", fdiv>;

def CPYSD   : inst_fpu_rr_rev64<0x30, "fcpys",  fcopysign>;
def CPYSED  : inst_fpu_rr_rev64<0x31, "fcpyse", fcopysign>;
def CPYSND  : inst_fpu_rr_rev64<0x32, "fcpysn", null_frag>;
def CPYSS   : inst_fpu_rr_rev32<0x30, "fcpys",  fcopysign>;
def CPYSES  : inst_fpu_rr_rev32<0x31, "fcpyse", fcopysign>;
def CPYSNS  : inst_fpu_rr_rev32<0x32, "fcpysn", null_frag>;

def CMPTEQ  : inst_fpu<0x10, "fcmpeq", F8RC>;
def CMPTLE  : inst_fpu<0x11, "fcmple", F8RC>;
def CMPTLT  : inst_fpu<0x12, "fcmplt", F8RC>;
def CMPTUN  : inst_fpu<0x13, "fcmpun", F8RC>;

//4.6.2 float cmp

def : Pat<(fabs F8RC:$RB),
          (CPYSD F31, F8RC:$RB)>;
def : Pat<(fabs F4RC:$RB),
          (CPYSS F31, F4RC:$RB)>;
def : Pat<(fneg F8RC:$RB),
          (CPYSND F8RC:$RB, F8RC:$RB)>;
def : Pat<(fneg F4RC:$RB),
          (CPYSNS F4RC:$RB, F4RC:$RB)>;

def : Pat<(fcopysign F4RC:$A, (fneg F4RC:$B)),
          (CPYSNS F4RC:$B, F4RC:$A)>;
def : Pat<(fcopysign F8RC:$A, (fneg F8RC:$B)),
          (CPYSND F8RC:$B, F8RC:$A)>;

def : Pat<(fneg (fcopysign F4RC:$A, F4RC:$B)),
          (CPYSNS F4RC:$B, F4RC:$A)>;
def : Pat<(fneg (fcopysign F8RC:$A, F8RC:$B)),
          (CPYSND F8RC:$B, F8RC:$A)>;

def : Pat<(fneg (fcopysign F4RC:$A, F8RC:$B)),
          (CPYSNS (COPY_TO_REGCLASS F8RC:$B, F4RC), F4RC:$A)>;
def : Pat<(fneg (fcopysign F8RC:$A, F4RC:$B)),
          (CPYSND (COPY_TO_REGCLASS F4RC:$B, F8RC), F8RC:$A)>;

def : Pat<(fcopysign F4RC:$A, F8RC:$B),
          (CPYSS (COPY_TO_REGCLASS F8RC:$B, F4RC), F4RC:$A)>;
def : Pat<(fcopysign F8RC:$A, F4RC:$B),
          (CPYSD (COPY_TO_REGCLASS F4RC:$B, F8RC), F8RC:$A)>;

//4.6.3 float convert

let RA = 31 in
class inst_fpu_cvt<bits<8> func, string opstr,
                   RegisterClass regorg, RegisterClass regdst,
                   SDPatternOperator OpNode=null_frag>
    : FPForm<0x18, func, (ins regorg:$RB), (outs regdst:$RC),
             opstr, "$RB,$RC",
             [(set regdst:$RC, (OpNode regorg:$RB))]>;

let RA = 31 in
class inst_fpu_cmov<bits<8> func, string opstr,
                   RegisterClass regorg, RegisterClass regdst,
                   SDPatternOperator OpNode=null_frag>
    : FPForm<0x10, func, (ins regorg:$RB), (outs regdst:$RC),
             opstr, "$RB,$RC",
             [(set (i64 regdst:$RC), (OpNode regorg:$RB))]>;

let RA = 31 in
class inst_flw<bits<8>func , string opstr, RegisterClass regtype>
    : FPForm<0x18, func, (ins regtype:$RB),
             (outs regtype:$RC), opstr, "$RB,$RC">;

let RB = 31 in
class inst_fpu_cvt1<bits<8> func, string opstr,
                   RegisterClass regorg, RegisterClass regdst,
                   SDPatternOperator OpNode=null_frag>
    : FPForm<0x18, func, (ins regorg:$RA), (outs regdst:$RC),
             opstr, "$RA,$RC",
             [(set regdst:$RC, (OpNode regorg:$RA))]>;

def CVTQS : inst_fpu_cvt<0x2D, "fcvtls", F8RC, F4RC, Sw64_cvtqs>;
def CVTQT : inst_fpu_cvt<0x2F, "fcvtld", F8RC, F8RC, Sw64_cvtqt>;
def CVTTQ : inst_fpu_cvt<0x24, "fcvtdl_z", F8RC, F8RC, Sw64_cvttq>;
def CVTST : inst_fpu_cvt<0x20, "fcvtsd", F4RC, F8RC, fpextend>;
def CVTTS : inst_fpu_cvt<0x21, "fcvtds", F8RC, F4RC, fpround>;
def : Pat<(Sw64_cvtts F8RC:$RB), (CVTTS F8RC:$RB)>;
def : Pat<(Sw64_cvtst F4RC:$RB), (CVTST F4RC:$RB)>;

def FCVTWL : inst_flw<0x28, "fcvtwl", F8RC>;
def FCVTLW : inst_flw<0x29, "fcvtlw", F8RC>;

def FCTTDL_G : inst_fpu_cvt<0x22, "fcvtdl_g", F8RC, F8RC>;
def FCTTDL_P : inst_fpu_cvt<0x23, "fcvtdl_p", F8RC, F8RC>;
def FCTTDL_N : inst_fpu_cvt<0x25, "fcvtdl_n", F8RC, F8RC>;
def FCTTDL : inst_fpu_cvt<0x27, "fcvtdl", F8RC, F8RC>;

def FCVTHS : inst_fpu_cvt<0x2E, "fcvths", F8RC, F8RC>;
def FCVTSH : FCForm4L<0x1B, 0x37, (ins F8RC:$RA, F8RC:$RB, u6imm:$LIT),
                 (outs F8RC:$RD),  "fcvtsh", "$RA,$RB,$LIT,$RD">;

def CMOVDL : inst_fpu_cmov<0x72, "cmovdl", F8RC, GPRC>;
def CMOVDL_G : inst_fpu_cmov<0x74, "cmovdl_g", F8RC, GPRC>;
def CMOVDL_P : inst_fpu_cmov<0x7A, "cmovdl_p", F8RC, GPRC>;
def CMOVDL_Z : inst_fpu_cmov<0x7C, "cmovdl_z", F8RC, GPRC>;
def CMOVDL_N : inst_fpu_cmov<0x80, "cmovdl_n", F8RC, GPRC>;
def CMOVDLU : inst_fpu_cmov<0x81, "cmovdlu", F8RC, GPRC>;
def CMOVDLU_G : inst_fpu_cmov<0x82, "cmovdlu_g", F8RC, GPRC>;
def CMOVDLU_P : inst_fpu_cmov<0x83, "cmovdlu_p", F8RC, GPRC>;
def CMOVDLU_Z : inst_fpu_cmov<0x84, "cmovdlu_z", F8RC, GPRC>;
def CMOVDLU_N : inst_fpu_cmov<0x85, "cmovdlu_n", F8RC, GPRC>;
def CMOVDWU : inst_fpu_cmov<0x86, "cmovdwu", F8RC, GPRC>;
def CMOVDWU_G : inst_fpu_cmov<0x87, "cmovdwu_g", F8RC, GPRC>;
def CMOVDWU_P : inst_fpu_cmov<0x88, "cmovdwu_p", F8RC, GPRC>;
def CMOVDWU_Z : inst_fpu_cmov<0x89, "cmovdwu_z", F8RC, GPRC>;
def CMOVDWU_N : inst_fpu_cmov<0x8A, "cmovdwu_n", F8RC, GPRC>;
def CMOVDW : inst_fpu_cmov<0x8B, "cmovdw", F8RC, GPRC>;
def CMOVDW_G : inst_fpu_cmov<0x8C, "cmovdw_g", F8RC, GPRC>;
def CMOVDW_P : inst_fpu_cmov<0x8D, "cmovdw_p", F8RC, GPRC>;
def CMOVDW_Z : inst_fpu_cmov<0x8E, "cmovdw_z", F8RC, GPRC>;
def CMOVDW_N : inst_fpu_cmov<0x8F, "cmovdw_n", F8RC, GPRC>;

// f64 to i64
def : Pat<(i64 (fp_to_sint F8RC:$Rn)), (CMOVDL_Z F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_sint (fround F8RC:$Rn))), (CMOVDL_G F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_sint (fceil F8RC:$Rn))), (CMOVDL_P F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_sint (ffloor F8RC:$Rn))), (CMOVDL_N F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_sint (fnearbyint F8RC:$Rn))), (CMOVDL F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;

// f64 to u64
def : Pat<(i64 (fp_to_uint F8RC:$Rn)), (CMOVDLU_Z F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_uint (fround F8RC:$Rn))), (CMOVDLU_G F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_uint (fceil F8RC:$Rn))), (CMOVDLU_P F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_uint (ffloor F8RC:$Rn))), (CMOVDLU_N F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_uint (fnearbyint F8RC:$Rn))), (CMOVDLU F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;

// f32 to i64
def : Pat<(i64 (fp_to_sint F4RC:$Rn)), (CMOVDL_Z (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_sint (fround F4RC:$Rn))), (CMOVDL_G (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_sint (fceil F4RC:$Rn))), (CMOVDL_P (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_sint (ffloor F4RC:$Rn))), (CMOVDL_N (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_sint (fnearbyint F4RC:$Rn))), (CMOVDL (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;

// f32 to u64
def : Pat<(i64 (fp_to_uint F4RC:$Rn)), (CMOVDLU_Z (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_uint (fround F4RC:$Rn))), (CMOVDLU_G (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_uint (fceil F4RC:$Rn))), (CMOVDLU_P (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_uint (ffloor F4RC:$Rn))), (CMOVDLU_N (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(i64 (fp_to_uint (fnearbyint F4RC:$Rn))), (CMOVDLU (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;

// f64 to u32
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint F8RC:$Rn))), i32),
          (CMOVDWU_Z F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint (fround F8RC:$Rn)))), i32),
          (CMOVDWU_G F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint (fceil F8RC:$Rn)))), i32),
          (CMOVDWU_P F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint (ffloor F8RC:$Rn)))), i32),
          (CMOVDWU_N F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint (fnearbyint F8RC:$Rn)))), i32),
          (CMOVDWU F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;

// f64 to i32
def : Pat<(sext_inreg (fp_to_sint F8RC:$Rn), i32),
          (CMOVDW_Z F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (fp_to_sint (fround F8RC:$Rn)), i32),
          (CMOVDW_G F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (fp_to_sint (fceil F8RC:$Rn)), i32),
          (CMOVDW_P F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (fp_to_sint (ffloor F8RC:$Rn)), i32),
          (CMOVDW_N F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (fp_to_sint (fnearbyint F8RC:$Rn)), i32),
          (CMOVDW F8RC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;

// f32 to u32
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint F4RC:$Rn))), i32),
          (CMOVDWU_Z (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint (fround F4RC:$Rn)))), i32),
          (CMOVDWU_G (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint (fceil F4RC:$Rn)))), i32),
          (CMOVDWU_P (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint (ffloor F4RC:$Rn)))), i32),
          (CMOVDWU_N (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (assertzext (i64 (fp_to_uint (fnearbyint F4RC:$Rn)))), i32),
          (CMOVDWU (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;

// f32 to i32
def : Pat<(sext_inreg (fp_to_sint F4RC:$Rn), i32),
          (CMOVDW_Z (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (fp_to_sint (fround F4RC:$Rn)), i32),
          (CMOVDW_G (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (fp_to_sint (fceil F4RC:$Rn)), i32),
          (CMOVDW_P (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (fp_to_sint (ffloor F4RC:$Rn)), i32),
          (CMOVDW_N (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(sext_inreg (fp_to_sint (fnearbyint F4RC:$Rn)), i32),
          (CMOVDW (CVTST F4RC:$Rn))>, Requires<[EnableFloatCmov, HasCore4]>;

def CMOVLS : inst_fpu_cvt<0x48, "cmovls", GPRC, F4RC>;
def CMOVWS : inst_fpu_cvt<0x49, "cmovws", GPRC, F4RC>;
def CMOVLD : inst_fpu_cvt<0x4a, "cmovld", GPRC, F8RC>;
def CMOVWD : inst_fpu_cvt<0x4b, "cmovwd", GPRC, F8RC>;
def CMOVULS : inst_fpu_cvt<0x4c, "cmovuls", GPRC, F4RC>;
def CMOVULD : inst_fpu_cvt<0x4e, "cmovuld", GPRC, F8RC>;
def CMOVUWS : inst_fpu_cvt<0x4d, "cmovuws", GPRC, F4RC>;
def CMOVUWD : inst_fpu_cvt<0x4f, "cmovuwd", GPRC, F8RC>;

def : Pat<(f32 (sint_to_fp GPRC:$Rn)), (CMOVLS GPRC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(f32 (sint_to_fp (assertsext GPRC:$Rn))), (CMOVWS GPRC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(f64 (sint_to_fp GPRC:$Rn)), (CMOVLD GPRC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(f64 (sint_to_fp (assertsext GPRC:$Rn))), (CMOVWD GPRC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(f32 (uint_to_fp GPRC:$Rn)), (CMOVULS GPRC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(f64 (uint_to_fp GPRC:$Rn)), (CMOVULD GPRC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(f32 (uint_to_fp (and (assertsext GPRC:$Rn), 0xffffffff))), (CMOVUWS GPRC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;
def : Pat<(f64 (uint_to_fp (and (assertsext GPRC:$Rn), 0xffffffff))), (CMOVUWD GPRC:$Rn)>, Requires<[EnableFloatCmov, HasCore4]>;

def FRIS   : inst_fpu_cvt<0x5A, "fris", F4RC, F4RC, fnearbyint>;
def FRIS_G : inst_fpu_cvt<0x5B, "fris_g", F4RC, F4RC, fround>;
def FRIS_P : inst_fpu_cvt<0x5C, "fris_p", F4RC, F4RC, fceil>;
def FRIS_Z : inst_fpu_cvt<0x5D, "fris_z", F4RC, F4RC, ftrunc>;
def FRIS_N : inst_fpu_cvt<0x5F, "fris_n", F4RC, F4RC, ffloor>;

def FRID   : inst_fpu_cvt<0x60, "frid", F8RC, F8RC, fnearbyint>;
def FRID_G : inst_fpu_cvt<0x61, "frid_g", F8RC, F8RC, fround>;
def FRID_P : inst_fpu_cvt<0x62, "frid_p", F8RC, F8RC, fceil>;
def FRID_Z : inst_fpu_cvt<0x63, "frid_z", F8RC, F8RC, ftrunc>;
def FRID_N : inst_fpu_cvt<0x64, "frid_n", F8RC, F8RC, ffloor>;

def SQRTSS : inst_flw<0x08, "fsqrts", F4RC>;
def SQRTSD : inst_flw<0x09, "fsqrtd", F8RC>;

def FRECS : inst_fpu_cvt1<0x58, "frecs", F4RC, F4RC, Sw64_frecs>;
def FRECD : inst_fpu_cvt1<0x59, "frecd", F8RC, F8RC, Sw64_frecd>;

def : Pat<(fsqrt F4RC:$RB), (SQRTSS F4RC:$RB)>;
def : Pat<(fsqrt F8RC:$RB), (SQRTSD F8RC:$RB)>;

//4.6.6 int 2 float or float 2 int

let RB = 31 in
class inst_fpu_fi<bits<6> opc, bits<8> func, string opstr,
                  RegisterClass regorg, RegisterClass regdst,
                  SDPatternOperator OpNode>
    : FPForm<opc, func, (ins regorg:$RA), (outs regdst:$RC),
             opstr, "$RA,$RC",
             [(set regdst:$RC, (OpNode regorg:$RA))]>;

def ITOFS : inst_fpu_fi<0x18, 0x40, "ifmovs", GPRC, F4RC, bitconvert>;
def ITOFT : inst_fpu_fi<0x18, 0x41, "ifmovd", GPRC, F8RC, bitconvert>;
def FTOIS : inst_fpu_fi<0x10, 0x70, "fimovs", F4RC, GPRC, bitconvert>;
def FTOIT : inst_fpu_fi<0x10, 0x78, "fimovd", F8RC, GPRC, bitconvert>;

let DecoderNamespace = "SIMD" in {
class inst_fpu_fi_simd<bits<6> opc, bits<8> func, string opstr,
                  RegisterClass regorg, RegisterClass regdst,
                  SDPatternOperator OpNode>
    : FPForm1<opc, func, (ins regorg:$RA), (outs regdst:$RC),
             opstr, "$RA,$RC",
             [(set regdst:$RC, (OpNode regorg:$RA))]>;
}

def ITOFStmp : inst_fpu_fi_simd<0x18, 0x40, "ifmovs", GPRC, FPRC, null_frag>;
def ITOFTtmp : inst_fpu_fi_simd<0x18, 0x41, "ifmovd", GPRC, FPRC, null_frag>;
def FTOIStmp : inst_fpu_fi_simd<0x10, 0x70, "fimovs", FPRC, GPRC, null_frag>;
def FTOITtmp : inst_fpu_fi_simd<0x10, 0x78, "fimovd", FPRC, GPRC, null_frag>;

multiclass inst_fpu_fma<bits<6> funcS, bits<6> funcD, string opstr> {
  def S : FForm4<0x19, funcS, (ins F4RC:$RA, F4RC:$RB, F4RC:$RC),
                 (outs F4RC:$RD),  opstr # "s", "$RA,$RB,$RC,$RD">;

  def D : FForm4<0x19, funcD, (ins F8RC:$RA, F8RC:$RB, F8RC:$RC),
                 (outs F8RC:$RD),  opstr # "d", "$RA,$RB,$RC,$RD">;
}

defm FMA  : inst_fpu_fma<0x00, 0x01, "fma">;
defm FMS  : inst_fpu_fma<0x02, 0x03, "fms">;
defm FNMA : inst_fpu_fma<0x04, 0x05, "fnma">;
defm FNMS : inst_fpu_fma<0x06, 0x07, "fnms">;

multiclass fma_pat<RegisterClass regtype, string type> {
def : Pat<(fma regtype:$RA, regtype:$RB, regtype:$RC),
    (!cast<Instruction>("FMA" # type)
        regtype:$RA, regtype:$RB, regtype:$RC)>;

def : Pat<(fma regtype:$RA, regtype:$RB,
                            (fneg regtype:$RC)),
    (!cast<Instruction>("FMS" # type)
        regtype:$RA, regtype:$RB, regtype:$RC)>;

def : Pat<(fneg
          (fma regtype:$RA, regtype:$RB,
                         (fneg regtype:$RC))),
    (!cast<Instruction>("FNMA" # type)
        regtype:$RA, regtype:$RB, regtype:$RC)>;

def : Pat<(fneg
          (fma regtype:$RA, regtype:$RB, regtype:$RC)),
    (!cast<Instruction>("FNMS" # type)
        regtype:$RA, regtype:$RB, regtype:$RC)>;

}

defm : fma_pat<F4RC, "S">;
defm : fma_pat<F8RC, "D">;

class inst_fpu_select64<bits<6> fun, string opstr>
    : FForm4 <0x19, fun, (ins F8RC:$RC, F8RC:$RB, F8RC:$RA),
                (outs F8RC:$RD), opstr, "$RA,$RB,$RC,$RD">;

let DecoderNamespace = "FP32" in {
class inst_fpu_select32<bits<6> fun, string opstr>
    : FForm4 <0x19, fun, (ins F4RC:$RC, F4RC:$RB, F8RC:$RA),
                (outs F4RC:$RD), opstr, "$RA,$RB,$RC,$RD">;
}
def FSELEQD : inst_fpu_select64<0x10, "fseleq">;
def FSELNED : inst_fpu_select64<0x11, "fselne">;
def FSELLTD : inst_fpu_select64<0x12, "fsellt">;
def FSELLED : inst_fpu_select64<0x13, "fselle">;
def FSELGTD : inst_fpu_select64<0x14, "fselgt">;
def FSELGED : inst_fpu_select64<0x15, "fselge">;
def FSELEQS : inst_fpu_select32<0x10, "fseleq">;
def FSELNES : inst_fpu_select32<0x11, "fselne">;
def FSELLTS : inst_fpu_select32<0x12, "fsellt">;
def FSELLES : inst_fpu_select32<0x13, "fselle">;
def FSELGTS : inst_fpu_select32<0x14, "fselgt">;
def FSELGES : inst_fpu_select32<0x15, "fselge">;

multiclass f_select_pat<SDPatternOperator OpNode, string Inst,
                        InstSw64 InstCmp> {
def : Pat<(select (i64 (OpNode F8RC:$RA, F8RC:$RB)), F4RC:$st, F4RC:$sf),
        (!cast<Instruction>(Inst # "S")
               F4RC:$sf, F4RC:$st, (InstCmp F8RC:$RA, F8RC:$RB))>;
def : Pat<(select (i64 (OpNode F8RC:$RA, F8RC:$RB)), F8RC:$st, F8RC:$sf),
        (!cast<Instruction>(Inst # "D")
               F8RC:$sf, F8RC:$st, (InstCmp F8RC:$RA, F8RC:$RB))>;
}

multiclass f_select_pat_c<SDPatternOperator OpNode, string Inst,
                          InstSw64 InstCmp> {
def : Pat<(select (i64 (OpNode F8RC:$RA, F8RC:$RB)), F4RC:$st, F4RC:$sf),
        (!cast<Instruction>(Inst # "S")
               F4RC:$sf, F4RC:$st, (InstCmp F8RC:$RB, F8RC:$RA))>;
def : Pat<(select (i64 (OpNode F8RC:$RA, F8RC:$RB)), F8RC:$st, F8RC:$sf),
        (!cast<Instruction>(Inst # "D")
               F8RC:$sf, F8RC:$st, (InstCmp F8RC:$RB, F8RC:$RA))>;
}
defm : f_select_pat<seteq, "FSELNE", CMPTEQ>;
defm : f_select_pat<setne, "FSELEQ", CMPTEQ>;
defm : f_select_pat<setlt, "FSELNE", CMPTLT>;
defm : f_select_pat<setle, "FSELNE", CMPTLE>;

defm : f_select_pat_c<setgt, "FSELNE", CMPTLT>;
defm : f_select_pat_c<setge, "FSELNE", CMPTLE>;

def :  Pat<(select GPRC:$RC, F8RC:$st, F8RC:$sf),
           (f64 (FSELEQD  F8RC:$st, F8RC:$sf, (ITOFT GPRC:$RC)))>;
def :  Pat<(select GPRC:$RC, F4RC:$st, F4RC:$sf),
           (f32 (FSELEQS  F4RC:$st, F4RC:$sf, (ITOFT GPRC:$RC)))>;

//4.6.9 read and write float register
let RB=31, RC=31 in {
def RFPCR : FPForm<0x18,0x50,(ins), (outs F8RC:$RA), "rfpcr", "$RA">;
def WFPCR : FPForm<0x18,0x51,(ins F8RC:$RA), (outs), "wfpcr", "$RA">;
}

let RA=31, RB = 31, RC=31 in {
def SETFPEC0 : FPForm<0x18,0x54,(ins), (outs),"setfpec0","">;
def SETFPEC1 : FPForm<0x18,0x55,(ins), (outs),"setfpec1","">;
def SETFPEC2 : FPForm<0x18,0x56,(ins), (outs),"setfpec2","">;
def SETFPEC3 : FPForm<0x18,0x57,(ins), (outs),"setfpec3","">;

def NOP      : FPForm<0x10,0x3a,(ins), (outs),"nop", "">;
}
//4.7 sundry instruction
let RA = 0, RB= 0, hasNoSchedulingInfo=1 in{
def WMEMB : MfcForm<0x06, 0x0002,(ins), (outs), "wmemb">;
def IMEMB : MfcForm<0x06, 0x0001,(ins), (outs), "imemb">;
def MB  : MfcForm<0x06, 0x0000,(ins), (outs), "memb">; //memory barrier
def HALT : MfcForm<0x06, 0x0080,(ins), (outs), "halt">;
}
def : Pat<(trap), (SYS_CALL 0x80)>;
def : Pat<(atomic_fence (i64 5), (timm)), (WMEMB)>,Requires<[EnableWmembInst, HasCore4]>;
def : Pat<(atomic_fence (timm), (timm)), (MB)>;
let RB = 31 in {
//4.7.3 read time counter
def RPCC : MfcForm<0x06, 0x0020,(ins), (outs GPRC:$RA), "rtc", "$RA">; //Read process cycle counter
//4.7.4 read cpu core id
def RCID : MfcForm<0x06, 0x0040,(ins), (outs GPRC:$RA), "rcid", "$RA">;
}
//4.7.6 atom operate instruction

let mayLoad = 1 in {
def LDQ_L : MFuncForm<0x08,0x1,(ins s64imm:$disp, GPRC:$RB),(outs GPRC:$RA),
                      "lldl", "$RA,${disp}(${RB})">;
def LDL_L : MFuncForm<0x08,0x0,(ins s64imm:$disp, GPRC:$RB),(outs GPRC:$RA),
                      "lldw", "$RA,${disp}(${RB})">;
}


let mayStore = 1 in {
def STQ_C : MFuncForm<0x08,0x9, (ins GPRC:$RA, s64imm:$disp, GPRC:$RB), (outs),
                      "lstl","$RA,${disp}(${RB})">;
def STL_C : MFuncForm<0x08,0x8, (ins GPRC:$RA, s64imm:$disp, GPRC:$RB), (outs),
                      "lstw","$RA,${disp}(${RB})">;
}

let RB = 31,  hasNoSchedulingInfo = 1 in {
def WR_F : MfcForm<0x06, 0x1020, (ins GPRC:$RA) , (outs), "wr_f", "$RA">;

def RD_F : MfcForm<0x06, 0x1000, (ins GPRC:$RA) , (outs), "rd_f", "$RA">;
}

//4.8 cache control instruction
// Prefetch
def SDT_ZPrefetch : SDTypeProfile<0, 2, [
  SDTCisPtrTy<0>, SDTCisInt<1>,
]>;

def z_s_fillcs : SDNode<"Sw64ISD::Z_S_FILLCS", SDT_ZPrefetch, [SDNPHasChain]>;
def z_s_fillde : SDNode<"Sw64ISD::Z_S_FILLDE", SDT_ZPrefetch, [SDNPHasChain]>;
def z_fillde : SDNode<"Sw64ISD::Z_FILLDE", SDT_ZPrefetch, [SDNPHasChain]>;
def z_fillde_e : SDNode<"Sw64ISD::Z_FILLDE_E", SDT_ZPrefetch, [SDNPHasChain]>;
def z_fillcs : SDNode<"Sw64ISD::Z_FILLCS", SDT_ZPrefetch, [SDNPHasChain]>;
def z_fillcs_e : SDNode<"Sw64ISD::Z_FILLCS_E", SDT_ZPrefetch, [SDNPHasChain]>;
def z_e_fillcs : SDNode<"Sw64ISD::Z_E_FILLCS", SDT_ZPrefetch, [SDNPHasChain]>;
def z_e_fillde : SDNode<"Sw64ISD::Z_E_FILLDE", SDT_ZPrefetch, [SDNPHasChain]>;

def z_flushd : SDNode<"Sw64ISD::Z_FLUSHD", SDT_ZPrefetch, [SDNPHasChain]>;

let usesCustomInserter = 1, mayLoad = 1, mayStore = 0, RA = 31 in
class Prefetch<string opstr, bits<6> opcode, SDPatternOperator loadop>
      : MForm<opcode, (ins s64imm:$DISP, GPRC:$RB), (outs),
            opstr, "${DISP}(${RB})",
           [(loadop immSExt16:$DISP, GPRC:$RB)]>;

def FILLCS   : Prefetch<"fillcs",   0x09, z_fillcs>;
def S_FILLDE : Prefetch<"s_fillde", 0x23, z_s_fillde>;
def S_FILLCS : Prefetch<"s_fillcs", 0x22, z_s_fillcs>;
def FILLDE   : Prefetch<"fillde",   0x26, z_fillde>;
def FILLDE_E : Prefetch<"fillde_e", 0x27, z_fillde_e>;
def FILLCS_E : Prefetch<"fillcs_e", 0x0B, z_fillcs_e>;
def E_FILLCS : Prefetch<"e_fillcs", 0x0A, z_e_fillcs>;
def E_FILLDE : Prefetch<"e_fillde", 0x0C, z_e_fillde>;

// END Prefetch


//4.9 privilege instruction
def DPFHR : MPrvlForm<0x1e,0xe,(ins u5imm:$TH, s64imm:$disp, GPRC:$RB), (outs),
                      "dpfhr", "$TH,${disp}(${RB})">;
def DPFHW : MPrvlForm<0x1e,0xf,(ins u5imm:$TH, s64imm:$disp, GPRC:$RB), (outs),
                      "dpfhw", "$TH,${disp}(${RB})">;

//4.9.1 csrr and csrw
// ----------------------------------------------------------
def CSRR : CSRForm<0x06, 0xfe, (ins GPRC:$RA, u5imm:$L), (outs), "csrr", "$RA,$L">;
def CSRW : CSRForm<0x06, 0xff, (ins GPRC:$RA, u5imm:$L), (outs), "csrw", "$RA,$L">;
// ----------------------------------------------------------

//4.9.2 csrws and csrwc
// ----------------------------------------------------------
def CSRWS : CSRForm<0x06, 0xfc, (ins GPRC:$RA, u5imm:$L), (outs), "csrws", "$RA,$L">;
def CSRWC : CSRForm<0x06, 0xfd, (ins GPRC:$RA, u5imm:$L), (outs), "csrwc", "$RA,$L">;
// ----------------------------------------------------------

class BrPat<InstSw64 Inst>
    : Pat<(brcond GPRC:$RA, bb:$DISP),
           (Inst GPRC:$RA, bb:$DISP)>;

class BrPat_const<PatFrag CondOp, InstSw64 Inst>
    : Pat<(brcond (i64 (CondOp GPRC:$RA, 0)), bb:$DISP),
          (Inst GPRC:$RA, bb:$DISP)>;

class BrPat_cond<PatFrag CondOp, InstSw64 InstBr,
                 InstSw64 InstCmp, RegisterClass regtype>
    : Pat<(brcond (i64 (CondOp regtype:$RB, regtype:$RA)), bb:$DISP),
          (InstBr (InstCmp regtype:$RA, regtype:$RB), bb:$DISP)>;
class BrPat_cond_i<PatFrag CondOp, InstSw64 InstBr,
                 InstSw64 InstCmp, RegisterClass regtype>
    : Pat<(brcond (i64 (CondOp regtype:$RA, regtype:$RB)), bb:$DISP),
          (InstBr (InstCmp regtype:$RA, regtype:$RB), bb:$DISP)>;

class BrSwapPat_cond<PatFrag CondOp, InstSw64 InstBr,
                     InstSw64 InstCmp, RegisterClass regtype>
    : Pat<(brcond (i64 (CondOp regtype:$RB, regtype:$RA)), bb:$DISP),
          (InstBr (InstCmp regtype:$RB, regtype:$RA), bb:$DISP)>;

class BrPat_f<PatFrag CondOp, InstSw64 InstBr>
    : Pat<(brcond (i64 (CondOp F8RC:$RA, immFPZ)), bb:$DISP),
          (InstBr F8RC:$RA, bb:$DISP)>;


def : BrPat_const<seteq, BEQ>;
def : BrPat_const<setge, BGE>;
def : BrPat_const<setgt, BGT>;
def : BrPat_const<setle, BLE>;
def : BrPat_const<setlt, BLT>;
def : BrPat_const<setne, BNE>;

def : BrPat<BNE>;
def : BrPat_cond_i<seteq, BNE, CMPEQr, GPRC>;
def : BrPat_cond_i<setle, BNE, CMPLEr, GPRC>;
def : BrPat_cond_i<setlt, BNE, CMPLTr, GPRC>;
def : BrPat_cond_i<setule, BNE, CMPULEr, GPRC>;
def : BrPat_cond_i<setult, BNE, CMPULTr, GPRC>;

def : BrPat_cond_i<setne, BEQ, CMPEQr, GPRC>;
def : BrPat_cond_i<setune, BEQ, CMPEQr, GPRC>;

def : BrPat_f<seteq, FBEQ>;
def : BrPat_f<setne, FBNE>;
def : BrPat_f<setle, FBLE>;
def : BrPat_f<setlt, FBLT>;

def : BrPat_cond<setlt,  FBEQ, CMPTLE, F8RC>;
def : BrPat_cond<setult,  FBEQ, CMPTLE, F8RC>;
def : BrPat_cond<setle,  FBEQ, CMPTLT, F8RC>;
def : BrPat_cond<setule,  FBEQ, CMPTLT, F8RC>;

def : BrSwapPat_cond<setgt,  FBEQ, CMPTLE, F8RC>;
def : BrSwapPat_cond<setugt,  FBEQ, CMPTLE, F8RC>;
def : BrSwapPat_cond<setge,  FBEQ, CMPTLT, F8RC>;
def : BrSwapPat_cond<setuge,  FBEQ, CMPTLT, F8RC>;

def : BrPat_cond_i<setne,  FBEQ, CMPTEQ, F8RC>;
def : BrPat_cond_i<seteq,  FBNE, CMPTEQ, F8RC>;

def : BrPat_cond_i<setge,   FBEQ, CMPTLT, F8RC>;
def : BrPat_cond_i<setgt,   FBEQ, CMPTLE, F8RC>;


 class IBrPat_cond_i<PatFrag CondOp, InstSw64 InstBr,
                 InstSw64 InstCmp, RegisterClass regtype>
    : Pat<(brcond (i64(CondOp regtype:$RA, immUExt8:$RB)), bb:$DISP),
          (InstBr (InstCmp regtype:$RA, immUExt8:$RB), bb:$DISP)>;
 def : IBrPat_cond_i<seteq, BNE, CMPEQi, GPRC>;
 def : IBrPat_cond_i<setne, BEQ, CMPEQi, GPRC>;
 def : IBrPat_cond_i<setlt, BNE, CMPLTi, GPRC>;
 def : IBrPat_cond_i<setle, BNE, CMPLEi, GPRC>;
 def : IBrPat_cond_i<setgt, BEQ, CMPLEi, GPRC>;
 def : IBrPat_cond_i<setge, BEQ, CMPLTi, GPRC>;

 def : IBrPat_cond_i<setult, BNE, CMPULTi, GPRC>;
 def : IBrPat_cond_i<setule, BNE, CMPULEi, GPRC>;
 def : IBrPat_cond_i<setugt, BEQ, CMPULEi, GPRC>;
 def : IBrPat_cond_i<setuge, BEQ, CMPULTi, GPRC>;


//Int cond patterns
def : Pat<(brcond (i64 (and GPRC:$RA, 1)), bb:$DISP),
          (BLBS  GPRC:$RA, bb:$DISP)>;

def : Pat<(brcond (i64 (seteq (and GPRC:$RA, 1), 0)), bb:$DISP),
          (BLBC  GPRC:$RA, bb:$DISP)>;

//Constant handling
def immSExt16int  : PatLeaf<(imm), [{ //(int)imm fits in a 16 bit sign extended field
  return ((int64_t)N->getZExtValue() << 48) >> 48 ==
         ((int64_t)N->getZExtValue() << 32) >> 32;
}], SExt16>;

def immConst2PartInt  : PatLeaf<(imm), [{
  //true if imm fits in a LDAH LDA pair with zeroext
  uint64_t uval = N->getZExtValue();
  int32_t val32 = (int32_t)uval;
  return ((uval >> 32) == 0 && //empty upper bits
          val32 <= IMM_FULLHIGH);
}]>;

def immConst2Part  : PatLeaf<(imm), [{
  //true if imm fits in a LDAH LDA pair
  int64_t val = (int64_t)N->getZExtValue();
  return (val <= IMM_FULLHIGH  && val >= IMM_FULLLOW);
}]>;


def : Pat<(i64 immConst2Part:$imm),
          (LDA (LL16 immConst2Part:$imm),
               (LDAH (LH16 immConst2Part:$imm), R31))>;

def : Pat<(i64 immSExt16:$imm),
          (LDA immSExt16:$imm, R31)>;

def : Pat<(i64 immSExt16int:$imm),
          (ZAPNOTi (LDA immSExt16int:$imm, R31), 15)>;

def : Pat<(i64 immConst2PartInt:$imm),
          (ZAPNOTi (LDA (LL16 (i64 (SExt32 immConst2PartInt:$imm))),
                        (LDAH (LH16 (i64 (SExt32 immConst2PartInt:$imm))),
                              R31)), 15)>;

//===----------------------------------------------------------------------===//
// Instruction aliases
//===----------------------------------------------------------------------===//
// 4.5.1 integer caculate
def : InstAlias<"addw $RA,$L,$RC",
                (ADDLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"subw $RA,$L,$RC",
                (SUBLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"s4addw $RA,$L,$RC",
                (S4ADDLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"s4subw $RA,$L,$RC",
                (S4SUBLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"s8addw $RA,$L,$RC",
                (S8ADDLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"s8subw $RA,$L,$RC",
                (S8SUBLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"addl $RA,$L,$RC",
                (ADDQi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"subl $RA,$L,$RC",
                (SUBQi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"s4addl $RA,$L,$RC",
                (S4ADDQi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"s4subl $RA,$L,$RC",
                (S4SUBQi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"s8addl $RA,$L,$RC",
                (S8ADDQi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"s8subl $RA,$L,$RC",
                (S8SUBQi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"mulw $RA,$L,$RC",
                (MULLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"mull $RA,$L,$RC",
                (MULQi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"umulh $RA,$L,$RC",
                (UMULHi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"zap $RA,$L,$RC",
                (ZAPi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"zapnot $RA,$L,$RC",
                (ZAPNOTi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"addpi $L,$RC",
                (ADDPI GPRC:$RC, s13imm:$L), 0>;
def : InstAlias<"addpis $L,$RC",
                (ADDPIS GPRC:$RC, s13imm:$L), 0>;
def : InstAlias<"sbt $RA,$L,$RC",
                (SBTi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"cbt $RA,$L,$RC",
                (CBTi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;

// 4.5.2 integer cmp
def : InstAlias<"cmpeq $RA,$L,$RC",
                (CMPEQi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"cmple $RA,$L,$RC",
                (CMPLEi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"cmplt $RA,$L,$RC",
                (CMPLTi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"cmpule $RA,$L,$RC",
                (CMPULEi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"cmpult $RA,$L,$RC",
                (CMPULTi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;

// 4.5.3 integer order
def : InstAlias<"and $RA,$L,$RC",
                (ANDi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"bic $RA,$L,$RC",
                (BICi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"bis $RA,$L,$RC",
                (BISi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"ornot $RA,$L,$RC",
                (ORNOTi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"xor $RA,$L,$RC",
                (XORi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"eqv $RA,$L,$RC",
                (EQVi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;

// 4.5.4 integer move position
def : InstAlias<"sll $RA,$L,$RC",
                (SLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"srl $RA,$L,$RC",
                (SRLi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;
def : InstAlias<"sra $RA,$L,$RC",
                (SRAi GPRC:$RA, GPRC:$RC, u8imm:$L), 0>;

// 4.5.5 integer cond select
def : InstAlias<"seleq $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELEQi GPRC:$RCOND, GPRC:$RFALSE, u8imm:$RTRUE, GPRC:$RDEST),
                 0>;
def : InstAlias<"seleq $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELEQr GPRC:$RCOND, GPRC:$RTRUE, GPRC:$RFALSE, GPRC:$RDEST),
                 0>;
def : InstAlias<"selge $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELGEi GPRC:$RCOND, GPRC:$RFALSE, u8imm:$RTRUE, GPRC:$RDEST),
                 0>;
def : InstAlias<"selgt $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELGTi GPRC:$RCOND, GPRC:$RFALSE, u8imm:$RTRUE, GPRC:$RDEST),
                 0>;
def : InstAlias<"selle $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELLEi GPRC:$RCOND, GPRC:$RFALSE, u8imm:$RTRUE, GPRC:$RDEST),
                 0>;
def : InstAlias<"sellt $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELLTi GPRC:$RCOND, GPRC:$RFALSE, u8imm:$RTRUE, GPRC:$RDEST),
                 0>;
def : InstAlias<"selne $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELNEi GPRC:$RCOND, GPRC:$RFALSE, u8imm:$RTRUE, GPRC:$RDEST),
                 0>;
def : InstAlias<"sellbc $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELLBCi GPRC:$RCOND, GPRC:$RFALSE, u8imm:$RTRUE, GPRC:$RDEST),
                 0>;
def : InstAlias<"sellbs $RCOND,$RTRUE,$RFALSE,$RDEST",
                (SELLBSi GPRC:$RCOND, GPRC:$RFALSE, u8imm:$RTRUE, GPRC:$RDEST),
                 0>;

// 4.5.6 byte operate
def : InstAlias<"nop", (BISr R31, R31, R31), 0>;

def : InstAlias<"ldi $RA, $imm",
                (LDA GPRC:$RA, s64imm:$imm, R31), 0>;
def : InstAlias<"br $disp",
                (BR R31, target:$disp), 0>;
def : InstAlias<"mov $imm,$RA",
                (BISi GPRC:$RA, R31,  u8imm:$imm), 0>;
def : InstAlias<"mov $RB,$RA",
                (BISr GPRC:$RA, R31, GPRC:$RB), 0>;
def : InstAlias<"ret $31,($26), $imm",
                (RET R31, R26, s16imm:$imm), 0>;
def : InstAlias<"ret",
                (RET R31, R26, 1), 0>;
def : InstAlias<"ldgp $29,0(${RA})",
                (MOVaddrPCGp 0, 0, GPRC:$RA), 0>;
def : InstAlias<"clr $RA",
                (BISr R31, R31, GPRC:$RA), 0>;

//===----------------------------------------------------------------------===//
// Sw64 vector Definitions.
//===----------------------------------------------------------------------===//

include "Sw64VectorVarDefine.td"
include "Sw64InstrFormatsV.td"
include "Sw64InstrVector.td"
