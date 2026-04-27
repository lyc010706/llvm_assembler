//===- Sw64InstrVector.td - SIMD instructions -*- tablegen ----------------*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes Sw64 SIMD instructions.
//
//===----------------------------------------------------------------------===//

class IsCommutable {
  bit isCommutable = 1;
}

def SDT_VSetCC : SDTypeProfile<1, 3, [SDTCisInt<0>,
                                      SDTCisInt<1>,
                                      SDTCisSameAs<1, 2>,
                                      SDTCisVT<3, OtherVT>]>;
def SDT_VFSetCC : SDTypeProfile<1, 3, [SDTCisInt<0>,
                                       SDTCisFP<1>,
                                       SDTCisSameAs<1, 2>,
                                       SDTCisVT<3, OtherVT>]>;

def SDT_VSHF : SDTypeProfile<1, 3, [SDTCisVec<0>, SDTCisVec<1>, SDTCisInt<3>,
                                    SDTCisSameAs<0, 1>]>;

def SDT_SHF : SDTypeProfile<1, 2, [SDTCisInt<0>, SDTCisVec<0>,
                                   SDTCisVT<1, i32>, SDTCisSameAs<0, 2>]>;
def SDT_ILV : SDTypeProfile<1, 2, [SDTCisInt<0>, SDTCisVec<0>,
                                   SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>]>;
def SDT_INSVE : SDTypeProfile<1, 4, [SDTCisVec<0>, SDTCisSameAs<0, 1>,
                                     SDTCisVT<2, i32>, SDTCisSameAs<0, 3>,
                                     SDTCisVT<4, i32>]>;
def SDT_VINSECTL : SDTypeProfile<1, 2, [SDTCisInt<0>, SDTCisVec<0>,
                                   SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>]>;

def SDT_VecReduce : SDTypeProfile<1, 1, [    // vector reduction
  SDTCisFP<0>, SDTCisVec<1>
]>;

def SDTVBroadcast  : SDTypeProfile<1, 1, [SDTCisVec<0>]>;

def SDT_ZVecBinaryFp : SDTypeProfile<1, 2, [SDTCisVec<0>,
                                             SDTCisSameAs<0, 1>,
                                             SDTCisVT<2, f32>]>;

def SDT_ZVecBinaryInt : SDTypeProfile<1, 2, [SDTCisVec<0>,
                                             SDTCisSameAs<0, 1>,
                                             SDTCisVT<2, i64>]>;

def SDT_ZVecCT : SDTypeProfile<1, 1, [    // vector number of head 0/1.
  SDTCisInt<0>, SDTCisVec<1>
]>;
def SDT_ZVecFREC : SDTypeProfile<1, 1, [    // vector number of head 0/1.
  SDTCisVec<0>, SDTCisFP<0>, SDTCisSameAs<0, 1>
]>;

def SDT_Vlog : SDTypeProfile<1, 4, [       // vlogzz
  SDTCisVec<0>, SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>, SDTCisSameAs<2, 3>, SDTCisInt<4>
]>;

def SDT_ZVecFCMP : SDTypeProfile<1, 2, [
  SDTCisVec<0>, SDTCisFP<0>, SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>
]>;
def SDT_ZVecFCVT : SDTypeProfile<1, 1, [
  SDTCisVec<0>, SDTCisVec<1>
]>;
def SDT_ZVecFCVTDL : SDTypeProfile<1, 1, [
  SDTCisVec<0>, SDTCisVec<1>, SDTCisInt<0>, SDTCisFP<1>
]>;
def SDT_ZVecFCVTSH : SDTypeProfile<1, 3, [
  SDTCisVec<0>, SDTCisVec<1>
]>;
def SDT_ZVecFCVTHS : SDTypeProfile<1, 2, [
  SDTCisVec<0>, SDTCisVec<1>
]>;

def SDT_ZVecFRI : SDTypeProfile<1, 1, [
  SDTCisVec<0>, SDTCisFP<0>, SDTCisSameAs<0, 1>
]>;
def SDT_ZVecCPY : SDTypeProfile<1, 1, [
  SDTCisVec<1>
]>;

def SDT_VSELECT : SDTypeProfile<1, 3, [
  SDTCisInt<0>, SDTCisVec<0>, SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>
]>;
def SDT_VSQRT : SDTypeProfile<1, 1, [
  SDTCisVec<0>, SDTCisFP<0>, SDTCisSameAs<0, 1>
]>;
def SDT_VSUMF : SDTypeProfile<1, 1, [
  SDTCisFP<0>, SDTCisVec<1>, SDTCisFP<1>
]>;
def SDT_Sw64VTruncStore : SDTypeProfile<0, 2, [SDTCisPtrTy<1>]>;

def Sw64VBroadCastLd : SDNode<"Sw64ISD::VBROADCAST_LD", SDTLoad>;
def Sw64VBroadCast   : SDNode<"Sw64ISD::VBROADCAST", SDTVBroadcast>;

def Sw64VBroadCasti32: PatFrag<(ops node:$src),
                           (Sw64VBroadCastLd node:$src), [{
    return cast<MemIntrinsicSDNode>(N)->getMemoryVT().getStoreSize() == 4;
}]>;

def Sw64VBroadCastf32: PatFrag<(ops node:$src),
                           (Sw64VBroadCastLd node:$src), [{
    return cast<MemIntrinsicSDNode>(N)->getMemoryVT().getStoreSize() == 4
           && cast<MemIntrinsicSDNode>(N)->getMemoryVT() == MVT::f32;
}]>;

def Sw64VBroadCastf64: PatFrag<(ops node:$src),
                           (Sw64VBroadCastLd node:$src), [{
    return cast<MemIntrinsicSDNode>(N)->getMemoryVT().getStoreSize() == 8;
}]>;

def Sw64VTruncStore : SDNode<"Sw64ISD::VTRUNCST", SDT_Sw64VTruncStore, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

def z_ldwe : SDNode<"Sw64ISD::VLDWE", SDTLoad>;
def z_ldse : SDNode<"Sw64ISD::VLDSE", SDTLoad>;
def z_ldde : SDNode<"Sw64ISD::VLDDE", SDTLoad>;

def z_vlog : SDNode<"Sw64ISD::VLOG", SDT_Vlog>;

def z_ctpop : SDNode<"Sw64ISD::VCTPOP", SDT_ZVecCT>;
def z_ctlz : SDNode<"Sw64ISD::VCTLZ", SDT_ZVecCT>;

def Sw64VNOR : SDNode<"Sw64ISD::VNOR", SDTIntBinOp,
                      [SDNPCommutative, SDNPAssociative]>;
def Sw64VEQV : SDNode<"Sw64ISD::VEQV", SDTIntBinOp>;
def Sw64VORNOT : SDNode<"Sw64ISD::VORNOT", SDTIntBinOp>;


def Sw64VSHF : SDNode<"Sw64ISD::VSHF", SDT_VSHF>;

def Sw64SHF  : SDNode<"Sw64ISD::SHF", SDT_SHF>;
def Sw64ILVEV : SDNode<"Sw64ISD::ILVEV", SDT_ILV>;
def Sw64ILVOD : SDNode<"Sw64ISD::ILVOD", SDT_ILV>;
def Sw64ILVL  : SDNode<"Sw64ISD::ILVL",  SDT_ILV>;
def Sw64ILVR  : SDNode<"Sw64ISD::ILVR",  SDT_ILV>;
def Sw64PCKEV : SDNode<"Sw64ISD::PCKEV", SDT_ILV>;
def Sw64PCKOD : SDNode<"Sw64ISD::PCKOD", SDT_ILV>;
def Sw64INSVE : SDNode<"Sw64ISD::INSVE", SDT_INSVE>;

def Sw64VFCMPEQ : SDNode<"Sw64ISD::VFCMPEQ", SDT_ZVecFCMP>;
def Sw64VFCMPLE : SDNode<"Sw64ISD::VFCMPLE", SDT_ZVecFCMP>;
def Sw64VFCMPLT : SDNode<"Sw64ISD::VFCMPLT", SDT_ZVecFCMP>;
def Sw64VFCMPUN : SDNode<"Sw64ISD::VFCMPUN", SDT_ZVecFCMP>;

def Sw64VFCVTSD : SDNode<"Sw64ISD::VFCVTSD", SDT_ZVecFCVT>;
def Sw64VFCVTDS : SDNode<"Sw64ISD::VFCVTDS", SDT_ZVecFCVT>;
def Sw64VFCVTLS : SDNode<"Sw64ISD::VFCVTLS", SDT_ZVecFCVT>;
def Sw64VFCVTLD : SDNode<"Sw64ISD::VFCVTLD", SDT_ZVecFCVT>;
def Sw64VFCVTSH : SDNode<"Sw64ISD::VFCVTSH", SDT_ZVecFCVTSH>;
def Sw64VFCVTHS : SDNode<"Sw64ISD::VFCVTHS", SDT_ZVecFCVTHS>;

def Sw64VFCVTDL : SDNode<"Sw64ISD::VFCVTDL", SDT_ZVecFCVTDL>;
def Sw64VFCVTDLG : SDNode<"Sw64ISD::VFCVTDLG", SDT_ZVecFCVTDL>;
def Sw64VFCVTDLP : SDNode<"Sw64ISD::VFCVTDLP", SDT_ZVecFCVTDL>;
def Sw64VFCVTDLZ : SDNode<"Sw64ISD::VFCVTDLZ", SDT_ZVecFCVTDL>;
def Sw64VFCVTDLN : SDNode<"Sw64ISD::VFCVTDLN", SDT_ZVecFCVTDL>;

def Sw64VFRIS : SDNode<"Sw64ISD::VFRIS", SDT_ZVecFRI>;
def Sw64VFRISG : SDNode<"Sw64ISD::VFRISG", SDT_ZVecFRI>;
def Sw64VFRISP : SDNode<"Sw64ISD::VFRISP", SDT_ZVecFRI>;
def Sw64VFRISZ : SDNode<"Sw64ISD::VFRISZ", SDT_ZVecFRI>;
def Sw64VFRISN : SDNode<"Sw64ISD::VFRISN", SDT_ZVecFRI>;
def Sw64VFRID : SDNode<"Sw64ISD::VFRID", SDT_ZVecFRI>;
def Sw64VFRIDG : SDNode<"Sw64ISD::VFRIDG", SDT_ZVecFRI>;
def Sw64VFRIDP : SDNode<"Sw64ISD::VFRIDP", SDT_ZVecFRI>;
def Sw64VFRIDZ : SDNode<"Sw64ISD::VFRIDZ", SDT_ZVecFRI>;
def Sw64VFRIDN : SDNode<"Sw64ISD::VFRIDN", SDT_ZVecFRI>;

def vseleqw :  SDNode<"Sw64ISD::VSELEQW", SDT_VSELECT>;
def vselltw :  SDNode<"Sw64ISD::VSELLTW", SDT_VSELECT>;
def vsellew :  SDNode<"Sw64ISD::VSELLEW", SDT_VSELECT>;
def vsellbcw : SDNode<"Sw64ISD::VSELLBCW", SDT_VSELECT>;

def vfcmovlt : SDNode<"Sw64ISD::VFCMOVLT", SDTFPTernaryOp>;
def vfcmovle : SDNode<"Sw64ISD::VFCMOVLE", SDTFPTernaryOp>;
def vfcmoveq : SDNode<"Sw64ISD::VFCMOVEQ", SDTFPTernaryOp>;

def vect_vucaddw : SDNode<"Sw64ISD::VECT_VUCADDW", SDTIntBinOp>;
def vect_vucaddh : SDNode<"Sw64ISD::VECT_VUCADDH", SDTIntBinOp>;
def vect_vucaddb : SDNode<"Sw64ISD::VECT_VUCADDB", SDTIntBinOp>;
def vect_vucsubw : SDNode<"Sw64ISD::VECT_VUCSUBW", SDTIntBinOp>;
def vect_vucsubh : SDNode<"Sw64ISD::VECT_VUCSUBH", SDTIntBinOp>;
def vect_vucsubb : SDNode<"Sw64ISD::VECT_VUCSUBB", SDTIntBinOp>;

def z_vshl_by_scalar    : SDNode<"Sw64ISD::VSHL_BY_SCALAR",
                                 SDT_ZVecBinaryFp>;
def z_vsrl_by_scalar    : SDNode<"Sw64ISD::VSRL_BY_SCALAR",
                                 SDT_ZVecBinaryFp>;
def z_vsra_by_scalar    : SDNode<"Sw64ISD::VSRA_BY_SCALAR",
                                 SDT_ZVecBinaryFp>;

def z_vcopyf : SDNode<"Sw64ISD::VCOPYF",
      SDTypeProfile<1, 1, [SDTCisPtrTy<1>]>, []>;

def z_v8sll    : SDNode<"Sw64ISD::V8SLL",
                                 SDT_ZVecBinaryInt>;

def z_v8srl    : SDNode<"Sw64ISD::V8SRL",
                                 SDT_ZVecBinaryInt>;

def z_v8sra    : SDNode<"Sw64ISD::V8SRA",
                                 SDT_ZVecBinaryInt>;

def z_vrotr    : SDNode<"Sw64ISD::VROTR",
                                 SDT_ZVecBinaryInt>;

def Sw64VINSECTL  : SDNode<"Sw64ISD::VINSECTL", SDT_VINSECTL>;

// ---- For immediate format.

def SDT_ZV8X : SDTypeProfile<1, 2, [SDTCisSameAs<0, 1>, SDTCisPtrTy<2>]>;

def Sw64V8SLL : SDNode<"Sw64ISD::V8SLLi", SDT_ZV8X>;
def Sw64V8SRL : SDNode<"Sw64ISD::V8SRLi", SDT_ZV8X>;
def Sw64V8SRA : SDNode<"Sw64ISD::V8SRAi", SDT_ZV8X>;
def Sw64VROTR : SDNode<"Sw64ISD::VROTRi", SDT_ZV8X>;
def Sw64VROLB : SDNode<"Sw64ISD::VROLBi", SDT_ZV8X>;
def Sw64VROLH : SDNode<"Sw64ISD::VROLHi", SDT_ZV8X>;
def Sw64VROLL : SDNode<"Sw64ISD::VROLLi", SDT_ZV8X>;

def z_v8slli : PatFrag<(ops node:$vec, node:$val),
    (v8i32 (Sw64V8SLL node:$vec, node:$val))>;

def z_v8srli : PatFrag<(ops node:$vec, node:$val),
    (v8i32 (Sw64V8SRL node:$vec, node:$val))>;

def z_v8srai : PatFrag<(ops node:$vec, node:$val),
    (v8i32 (Sw64V8SRA node:$vec, node:$val))>;

def z_vrotri : PatFrag<(ops node:$vec, node:$val),
    (v8i32 (Sw64VROTR node:$vec, node:$val))>;

def z_vrolbi : PatFrag<(ops node:$vec, node:$val),
    (v32i8 (Sw64VROLB node:$vec, node:$val))>;
def z_vrolhi : PatFrag<(ops node:$vec, node:$val),
    (v16i16 (Sw64VROLH node:$vec, node:$val))>;
def z_vrolli : PatFrag<(ops node:$vec, node:$val),
    (v4i64 (Sw64VROLL node:$vec, node:$val))>;

def z_vslls : PatFrag<(ops node:$vec, node:$val),
    (v4f32 (Sw64V8SLL node:$vec, node:$val))>;

def z_vslld : PatFrag<(ops node:$vec, node:$val),
    (v4f64 (Sw64V8SLL node:$vec, node:$val))>;

def z_vsrls : PatFrag<(ops node:$vec, node:$val),
    (v4f32 (Sw64V8SRL node:$vec, node:$val))>;

def z_vsrld : PatFrag<(ops node:$vec, node:$val),
    (v4f64 (Sw64V8SRL node:$vec, node:$val))>;

// ----

def Sw64VExtractSExt : SDNode<"Sw64ISD::VEXTRACT_SEXT_ELT",
    SDTypeProfile<1, 3, [SDTCisPtrTy<2>]>, []>;
def Sw64VExtractZExt : SDNode<"Sw64ISD::VEXTRACT_ZEXT_ELT",
    SDTypeProfile<1, 3, [SDTCisPtrTy<2>]>, []>;

// Pattern fragments
def vextract_sext_i8  : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractSExt node:$vec, node:$idx, i8)>;
def vextract_sext_i16 : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractSExt node:$vec, node:$idx, i16)>;
def vextract_sext_i32 : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractSExt node:$vec, node:$idx, i32)>;
def vextract_sext_i64 : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractSExt node:$vec, node:$idx, i64)>;

def vextract_sext_f32 : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractSExt node:$vec, node:$idx, f32)>;

def vextract_sext_f64 : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractSExt node:$vec, node:$idx, f64)>;

def vextract_zext_i8  : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractZExt node:$vec, node:$idx, i8)>;
def vextract_zext_i16 : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractZExt node:$vec, node:$idx, i16)>;
def vextract_zext_i32 : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractZExt node:$vec, node:$idx, i32)>;
def vextract_zext_i64 : PatFrag<(ops node:$vec, node:$idx),
                                (Sw64VExtractZExt node:$vec, node:$idx, i64)>;

def vsetcc : SDNode<"ISD::SETCC", SDT_VSetCC>;
def vfsetcc : SDNode<"ISD::SETCC", SDT_VFSetCC>;

class vsetcc_type<ValueType ResTy, CondCode CC> :
  PatFrag<(ops node:$lhs, node:$rhs),
          (vsetcc (ResTy node:$lhs), (ResTy node:$rhs), CC)>;

def SDT_VSetGE : SDTypeProfile<1, 2, [SDTCisInt<0>,
                                      SDTCisVec<1>,
                                      SDTCisSameAs<1, 2>]>;
def vsetge_v8i32  : SDNode<"Sw64ISD::VSETGE", SDT_VSetGE>;

def z_vsetge : PatFrag<(ops node:$vec, node:$val),
              (vsetge_v8i32 node:$vec, node:$val)>;

class Vector_2Op_Pat<SDPatternOperator OpNode, ValueType Ty> :
  PatFrag<(ops node:$vec, node:$val),
          (OpNode (Ty node:$vec), (Ty node:$val))>;

class Vector_1Op_Pat<SDPatternOperator OpNode, ValueType Ty> :
  PatFrag<(ops node:$src),
          (OpNode (Ty node:$src))>;

multiclass MultiVec2OpPat<SDPatternOperator OpNode> {
  def v8i32  : Vector_2Op_Pat<OpNode, v8i32>;
  def v4i64  : Vector_2Op_Pat<OpNode, v4i64>;
  def v4f32  : Vector_2Op_Pat<OpNode, v4f32>;
  def v4f64  : Vector_2Op_Pat<OpNode, v4f64>;
  def v16i16 : Vector_2Op_Pat<OpNode, v16i16>;
  def v32i8  : Vector_2Op_Pat<OpNode, v32i8>;
}

defm add   : MultiVec2OpPat<add>;
defm sub   : MultiVec2OpPat<sub>;
defm and   : MultiVec2OpPat<and>;
defm xor   : MultiVec2OpPat<xor>;
defm or    : MultiVec2OpPat<or>;

def vbic   : BinOpFrag<(and node:$LHS, (vnot node:$RHS))>;
def vornot : BinOpFrag<(or node:$LHS, (vnot node:$RHS))>;
def veqv   : BinOpFrag<(vnot (xor node:$LHS, node:$RHS))>;

def vseteq_v8i32  : vsetcc_type<v8i32, SETEQ>;
def vsetle_v8i32  : vsetcc_type<v8i32, SETLE>;
def vsetlt_v8i32  : vsetcc_type<v8i32, SETLT>;
def vsetule_v8i32 : vsetcc_type<v8i32, SETULE>;
def vsetult_v8i32 : vsetcc_type<v8i32, SETULT>;
def vsetueq_v32i8  : vsetcc_type<v32i8, SETUEQ>;
def vsetugt_v32i8  : vsetcc_type<v32i8, SETUGT>;

def SDT_VMAX : SDTypeProfile<1, 2, [SDTCisInt<0>,SDTCisVec<0>,
                                    SDTCisSameAs<0,1>,
                                    SDTCisSameAs<0,2>]>;
def SDT_VFMAX : SDTypeProfile<1, 2, [SDTCisFP<0>,SDTCisVec<0>,
                                     SDTCisSameAs<0,1>,
                                     SDTCisSameAs<0,2>]>;
def vmax : SDNode<"Sw64ISD::VMAX", SDT_VMAX>;
def vmin : SDNode<"Sw64ISD::VMIN", SDT_VMAX>;
def vumax : SDNode<"Sw64ISD::VUMAX", SDT_VMAX>;
def vumin : SDNode<"Sw64ISD::VUMIN", SDT_VMAX>;

def vmaxf : SDNode<"Sw64ISD::VMAXF", SDT_VFMAX>;
def vminf : SDNode<"Sw64ISD::VMINF", SDT_VFMAX>;

class vfsetcc_type<ValueType ResTy, ValueType OpTy, CondCode CC> :
  PatFrag<(ops node:$lhs, node:$rhs),
          (ResTy (vfsetcc (OpTy node:$lhs), (OpTy node:$rhs), CC))>;

// ISD::SETFALSE cannot occur
def vfseteq_v4f32 : vfsetcc_type<v4i64, v4f32, SETEQ>;
def vfseteq_v4f64 : vfsetcc_type<v4i64, v4f64, SETEQ>;
def vfsetge_v4f32 : vfsetcc_type<v4i64, v4f32, SETGE>;
def vfsetge_v4f64 : vfsetcc_type<v4i64, v4f64, SETGE>;
def vfsetgt_v4f32 : vfsetcc_type<v4i64, v4f32, SETGT>;
def vfsetgt_v4f64 : vfsetcc_type<v4i64, v4f64, SETGT>;
def vfsetle_v4f32 : vfsetcc_type<v4i64, v4f32, SETLE>;
def vfsetle_v4f64 : vfsetcc_type<v4i64, v4f64, SETLE>;
def vfsetlt_v4f32 : vfsetcc_type<v4i64, v4f32, SETLT>;
def vfsetlt_v4f64 : vfsetcc_type<v4i64, v4f64, SETLT>;
def vfsetne_v4f32 : vfsetcc_type<v4i64, v4f32, SETNE>;
def vfsetne_v4f64 : vfsetcc_type<v4i64, v4f64, SETNE>;

def vfsetoeq_v4f32 : vfsetcc_type<v4i64, v4f32, SETOEQ>;
def vfsetoeq_v4f64 : vfsetcc_type<v4i64, v4f64, SETOEQ>;
def vfsetoge_v4f32 : vfsetcc_type<v4i64, v4f32, SETOGE>;
def vfsetoge_v4f64 : vfsetcc_type<v4i64, v4f64, SETOGE>;
def vfsetogt_v4f32 : vfsetcc_type<v4i64, v4f32, SETOGT>;
def vfsetogt_v4f64 : vfsetcc_type<v4i64, v4f64, SETOGT>;
def vfsetole_v4f32 : vfsetcc_type<v4i64, v4f32, SETOLE>;
def vfsetole_v4f64 : vfsetcc_type<v4i64, v4f64, SETOLE>;
def vfsetolt_v4f32 : vfsetcc_type<v4i64, v4f32, SETOLT>;
def vfsetolt_v4f64 : vfsetcc_type<v4i64, v4f64, SETOLT>;
def vfsetone_v4f32 : vfsetcc_type<v4i64, v4f32, SETONE>;
def vfsetone_v4f64 : vfsetcc_type<v4i64, v4f64, SETONE>;
def vfsetord_v4f32 : vfsetcc_type<v4i64, v4f32, SETO>;
def vfsetord_v4f64 : vfsetcc_type<v4i64, v4f64, SETO>;
def vfsetun_v4f32  : vfsetcc_type<v4i64, v4f32, SETUO>;
def vfsetun_v4f64  : vfsetcc_type<v4i64, v4f64, SETUO>;
def vfsetueq_v4f32 : vfsetcc_type<v4i64, v4f32, SETUEQ>;
def vfsetueq_v4f64 : vfsetcc_type<v4i64, v4f64, SETUEQ>;
def vfsetuge_v4f32 : vfsetcc_type<v4i64, v4f32, SETUGE>;
def vfsetuge_v4f64 : vfsetcc_type<v4i64, v4f64, SETUGE>;
def vfsetugt_v4f32 : vfsetcc_type<v4i64, v4f32, SETUGT>;
def vfsetugt_v4f64 : vfsetcc_type<v4i64, v4f64, SETUGT>;
def vfsetule_v4f32 : vfsetcc_type<v4i64, v4f32, SETULE>;
def vfsetule_v4f64 : vfsetcc_type<v4i64, v4f64, SETULE>;
def vfsetult_v4f32 : vfsetcc_type<v4i64, v4f32, SETULT>;
def vfsetult_v4f64 : vfsetcc_type<v4i64, v4f64, SETULT>;
def vfsetune_v4f32 : vfsetcc_type<v4i64, v4f32, SETUNE>;
def vfsetune_v4f64 : vfsetcc_type<v4i64, v4f64, SETUNE>;
// ISD::SETTRUE cannot occur
// ISD::SETFALSE2 cannot occur
// ISD::SETTRUE2 cannot occur

class SplatComplexPattern<Operand opclass, ValueType ty, int numops, string fn,
                          list<SDNode> roots = [],
                          list<SDNodeProperty> props = []> :
  ComplexPattern<ty, numops, fn, roots, props> {
  Operand OpClass = opclass;
}

multiclass MultiVec1OpPat<SDPatternOperator OpNode> {
  def v8i32  : Vector_1Op_Pat<OpNode, v8i32>;
  def v4i64  : Vector_1Op_Pat<OpNode, v4i64>;
  def v4f32  : Vector_1Op_Pat<OpNode, v4f32>;
  def v4f64  : Vector_1Op_Pat<OpNode, v4f64>;
  def v16i16 : Vector_1Op_Pat<OpNode, v16i16>;
  def v32i8  : Vector_1Op_Pat<OpNode, v32i8>;
}

defm vsplat : MultiVec1OpPat<Sw64VBroadCast>;

def vsplati64_simm8 : SplatComplexPattern<s8imm, v4i64, 1,
                                          "selectVSplatSimm8",
                                          [build_vector, bitconvert]>;

def vsplati64_uimm8 : SplatComplexPattern<vsplat_uimm8, v4i64, 1,
                                          "selectVSplatUimm8",
                                          [build_vector, bitconvert]>;

def vsplati32_simm8 : SplatComplexPattern<vsplat_simm8, v8i32, 1,
                                          "selectVSplatSimm8",
                                          [build_vector, bitconvert]>;

def vsplati32_uimm8 : SplatComplexPattern<vsplat_uimm8, v8i32, 1,
                                          "selectVSplatUimm8",
                                          [build_vector, bitconvert]>;

def vsplati16_uimm8 : SplatComplexPattern<vsplat_uimm8, v16i16, 1,
                                          "selectVSplatUimm8",
                                          [build_vector, bitconvert]>;

def vsplati8_uimm8 : SplatComplexPattern<vsplat_uimm8, v32i8, 1,
                                          "selectVSplatUimm8",
                                          [build_vector, bitconvert]>;

def AddSubImm8Pat : ComplexPattern<i64, 1, "SelectAddSubImm<MVT::i8>", []>;
def ComplexImmPat : ComplexPattern<i64, 1, "SelectComplexImm", []>;

def addrimm10 : ComplexPattern<iPTR, 2, "selectIntAddrSImm10", [frameindex]>;

def addrimm10lsl1 : ComplexPattern<iPTR, 2, "selectIntAddrSImm10Lsl1",
                                   [frameindex]>;

def addrimm16 : ComplexPattern<iPTR, 2, "selectIntAddrSImm16",
                                   [frameindex]>;
def addrimm12 : ComplexPattern<iPTR, 2, "selectIntAddrSImm12",
                                   [frameindex]>;

def immZExt1Ptr : ImmLeaf<iPTR, [{return isUInt<1>(Imm);}]>;
def immZExt2Ptr : ImmLeaf<iPTR, [{return isUInt<2>(Imm);}]>;
def immZExt3Ptr : ImmLeaf<iPTR, [{return isUInt<3>(Imm);}]>;
def immZExt4Ptr : ImmLeaf<iPTR, [{return isUInt<4>(Imm);}]>;
def immZExt5Ptr : ImmLeaf<iPTR, [{return isUInt<5>(Imm);}]>;
def immZExt8Ptr : ImmLeaf<iPTR, [{return isUInt<8>(Imm);}]>;

def vinsert_v8i32 : PatFrag<(ops node:$vec, node:$val, node:$idx),
    (v8i32 (vector_insert node:$vec, node:$val, node:$idx))>;

def vinsert_v4f32 : PatFrag<(ops node:$vec, node:$val, node:$idx),
    (v4f32 (vector_insert node:$vec, node:$val, node:$idx))>;

def vinsert_v4f64 : PatFrag<(ops node:$vec, node:$val, node:$idx),
    (v4f64 (vector_insert node:$vec, node:$val, node:$idx))>;

def vinsert_v32i8 : PatFrag<(ops node:$vec, node:$val, node:$idx),
    (v32i8 (vector_insert node:$vec, node:$val, node:$idx))>;

def vinsert_v16i16 : PatFrag<(ops node:$vec, node:$val, node:$idx),
    (v16i16 (vector_insert node:$vec, node:$val, node:$idx))>;
// Instruction desc.
// 存储器指令格式
class VectorStoreBASE<string instr_asm, ValueType vt,
                   SDPatternOperator OpNode,
                   RegisterOperand ROWD, Operand MemOpnd,
                   ComplexPattern Addr = addrimm10> {
  dag OutOperandList = (outs);
  dag InOperandList = (ins ROWD:$RA, MemOpnd:$addr);
  string AsmString = !strconcat(instr_asm, "\t$RA, $addr");
  list<dag> Pattern = [(OpNode (vt ROWD:$RA), Addr:$addr)];
}

class VectorLoadBASE<string instr_asm, ValueType vt,
                   SDPatternOperator OpNode,
                   RegisterOperand ROWD, Operand MemOpnd,
                   ComplexPattern Addr = addrimm10> {
  dag OutOperandList = (outs ROWD:$RA);
  dag InOperandList = (ins MemOpnd:$addr);
  string AsmString = !strconcat(instr_asm, "\t$RA, $addr");
  list<dag> Pattern = [(set ROWD:$RA, (vt (OpNode Addr:$addr)))];
}

let mayStore = 1 in
class VectorStore<bits<6> opcode, string instr_asm, RegisterOperand ROWD,
                  ValueType vt, SDPatternOperator OpNode=null_frag>
  : MFormV<opcode>,
  VectorStoreBASE<instr_asm, vt, OpNode, ROWD, mem_simm16, addrimm16>;

let mayLoad = 1 in
class VectorLoad<bits<6> opcode, string instr_asm, RegisterOperand ROWD,
                 ValueType vt, SDPatternOperator OpNode=null_frag>
  : MFormV<opcode>,
  VectorLoadBASE<instr_asm, vt, OpNode, ROWD, mem_simm16, addrimm16>;

let DecoderMethod = "DecodeFIXMEInstruction" in{
def VSTS : VectorStore<0x0E, "vsts", V256LOpnd, v4f32, store>;
def VSTD : VectorStore<0x0F, "vstd", V256LOpnd, v4f64, store>;
def VLDS : VectorLoad <0x0C, "vlds", V256LOpnd, v4f32, load>;
def VLDD : VectorLoad <0x0D, "vldd", V256LOpnd, v4f64, load>;
def VLDWE : VectorLoad <0x09, "ldwe", V256LOpnd, v8i32, Sw64VBroadCasti32>;
def VLDSE : VectorLoad <0x0A, "ldse", V256LOpnd, v4f32, Sw64VBroadCastf32>;
def VLDDE : VectorLoad <0x0B, "ldde", V256LOpnd, v4f64, Sw64VBroadCastf64>;
}
multiclass V256Pat<SDPatternOperator OpNode> {
  def v32i8  : PatFrag<(ops node:$src), (v32i8 (OpNode node:$src))>;
  def v16i16 : PatFrag<(ops node:$src), (v16i16 (OpNode node:$src))>;
  def v8i32  : PatFrag<(ops node:$src), (v8i32 (OpNode node:$src))>;
  def v4i64  : PatFrag<(ops node:$src), (v4i64 (OpNode node:$src))>;
  def v4f64  : PatFrag<(ops node:$src), (v4f64 (OpNode node:$src))>;
}

////////////////////////////////////////////
// Extern Vector Memory Operation
// /////////////////////////////////////////
// 带功能域的存储器指令格式
let mayStore = 1 in
class VectorStoreExt<bits<4> func, string instr_asm, ValueType vt,
                     SDPatternOperator OpNode=null_frag>
  : MFuncFormV<0x1C, func>,
  VectorStoreBASE<instr_asm, vt, OpNode, V256LOpnd, mem_simm12, addrimm12>;

let mayLoad = 1 in
class VectorLoadExt<bits<4> func, string instr_asm, ValueType vt,
                    SDPatternOperator OpNode=null_frag>
  : MFuncFormV<0x1C, func>,
  VectorLoadBASE<instr_asm, vt, OpNode, V256LOpnd, mem_simm12, addrimm12>;

let DecoderMethod = "DecodeFIXMEInstruction" in{
def VLDWU  : VectorLoadExt <0x00, "vldw_u" , v8i32>;
def VLDSU  : VectorLoadExt <0x02, "vlds_u" , v4f32>;
def VLDDU  : VectorLoadExt <0x04, "vldd_u" , v4f64>;
def VLDDNC : VectorLoadExt <0x0e, "vldd_nc", v4f64>;
def VSTWU  : VectorStoreExt<0x01, "vstw_u" , v8i32>;
def VSTSU  : VectorStoreExt<0x03, "vsts_u" , v4f32>;
def VSTDU  : VectorStoreExt<0x05, "vstd_u" , v4f64>;
def VSTWUL : VectorStoreExt<0x08, "vstw_ul", v8i32>;
def VSTSUL : VectorStoreExt<0x0a, "vsts_ul", v4f32>;
def VSTDUL : VectorStoreExt<0x0c, "vstd_ul", v4f64>;
def VSTWUH : VectorStoreExt<0x09, "vstw_uh", v8i32>;
def VSTSUH : VectorStoreExt<0x0b, "vsts_uh", v4f32>;
def VSTDUH : VectorStoreExt<0x0d, "vstd_uh", v4f64>;
def VSTDNC : VectorStoreExt<0x0f, "vstd_nc", v4f64>;
}
class vload_pat<ValueType Vt, SDPatternOperator OpNode, Instruction Inst>
  : Pat<(Vt (OpNode addrimm16:$src)), (Inst addrimm16:$src)>;

class vstore_pat<ValueType Vt, SDPatternOperator OpNode, Instruction Inst>
  : Pat<(OpNode (Vt V256L:$DST), addrimm16:$src), (Inst $DST, addrimm16:$src)>;


// commom pattern for load/store intrinsic
multiclass vector_mem_multipat<ValueType Vt> {
def : vload_pat<Vt, load, VLDD>;
def : vstore_pat<Vt, store, VSTD>;
def : vload_pat<Vt, int_sw64_vload, VLDD>;
def : vstore_pat<Vt, int_sw64_vstore, VSTD>;
}

multiclass vector_mem_intrpat<ValueType Vt, Instruction InstL, Instruction InstS> {
def : vload_pat<Vt, int_sw64_vload, InstL>;
def : vstore_pat<Vt, int_sw64_vstore, InstS>;
}

// extension pattern for load_u/loade/store_u/storeuh/..
multiclass vector_mem_extension<ValueType Vt, string LoadI, string StoreI> {
def : vload_pat <Vt, int_sw64_vload_u, !cast<Instruction>(LoadI#U)>;
def : vload_pat <Vt, int_sw64_vloade,  !cast<Instruction>(LoadI#E)>;
def : vstore_pat<Vt, int_sw64_vstore_u, !cast<Instruction>(StoreI#U)>;
def : vstore_pat<Vt, int_sw64_vstoreuh, !cast<Instruction>(StoreI#UH)>;
def : vstore_pat<Vt, int_sw64_vstoreul, !cast<Instruction>(StoreI#UL)>;
}

defm : vector_mem_multipat<v32i8>;
defm : vector_mem_multipat<v16i16>;
defm : vector_mem_multipat<v8i32>;
defm : vector_mem_multipat<v4i64>;

defm : vector_mem_intrpat<v4f32, VLDS, VSTS>;
defm : vector_mem_intrpat<v4f64, VLDD, VSTD>;

defm : vector_mem_extension<v8i32, "VLDW", "VSTW">;
defm : vector_mem_extension<v4f32, "VLDS", "VSTS">;
defm : vector_mem_extension<v4f64, "VLDD", "VSTD">;
defm : vector_mem_extension<v4i64, "VLDD", "VSTD">;

multiclass vector_mem_nc<ValueType Vt> {
def : vload_pat <Vt, int_sw64_vloadnc, VLDDNC>;
def : vstore_pat<Vt, int_sw64_vstorenc, VSTDNC>;
}

defm : vector_mem_nc<v32i8>;
defm : vector_mem_nc<v16i16>;
defm : vector_mem_nc<v8i32>;
defm : vector_mem_nc<v4i64>;
defm : vector_mem_nc<v4f64>;

def : Pat<(v8i32 (Sw64VBroadCast (i64 (extloadi32 addrimm16:$src)))),
          (VLDWE addrimm16:$src)>;
def : Pat<(v4f32 (Sw64VBroadCast (f32 (load addrimm16:$src)))),
          (VLDSE addrimm16:$src)>;
def : Pat<(v4i64 (Sw64VBroadCast (i64 (load addrimm16:$src)))),
          (VLDDE addrimm16:$src)>;
def : Pat<(v4f64 (Sw64VBroadCast (f64 (load addrimm16:$src)))),
          (VLDDE addrimm16:$src)>;

def : vstore_pat<v4i64, Sw64VTruncStore, VSTS>;

class SIMD_3RR_SAME<string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                   RegisterOperand ROC, RegisterOperand ROA = ROC,
                   RegisterOperand ROB = ROC> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
  list<dag> Pattern = [(set (Vt ROC:$RC), (OpNode (Vt ROA:$RA), (Vt ROB:$RB)))];
}

class SIMD_3RI_SAME<string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                   Operand immtype, RegisterOperand ROC,
                   RegisterOperand ROA = ROC> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, immtype:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $Imm, $RC");
  list<dag> Pattern = [(set (Vt ROC:$RC), (OpNode (Vt ROA:$RA), immtype:$Imm))];
}

class SIMD_4RR_SAME<string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                   RegisterOperand ROC> {
  dag OutOperandList = (outs ROC:$RD);
  dag InOperandList = (ins ROC:$RA, ROC:$RB, ROC:$RC);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC, $RD");
  list<dag> Pattern = [(set (Vt ROC:$RD),
    (OpNode (Vt ROC:$RA), (Vt ROC:$RB), (Vt ROC:$RC)))];
}

class Vector_2OP_Reg_Pat<SDPatternOperator OpNode, ValueType Vt, ValueType Yt,
                        RegisterOperand OR, Instruction Inst>
  : Pat<(OpNode (Vt OR:$RA), (Yt OR:$RB)),
        (Inst (Vt OR:$RA), (Yt OR:$RB))>;

class Vector_1OP_Imm_Pat<SDPatternOperator OpNode, ValueType Vt,
                        ComplexPattern cpx,
                        RegisterOperand OR, Instruction Inst>
  : Pat<(Vt (OpNode (Vt OR:$RA), (i64 cpx:$Imm))),
        (Inst (Vt OR:$RA), $Imm)>;

class Vector_2OP_Imm_VB_Pat<SDPatternOperator OpNode, SDPatternOperator immop,
                        ValueType Vt, ValueType it, ComplexPattern cpx,
                        RegisterOperand OR, Instruction Inst>
  : Pat<(Vt (OpNode (Vt OR:$RA), (it (immop (i64 cpx:$Imm))))),
        (Inst (Vt OR:$RA), $Imm)>;

class Vector_2OP_Reg_Scalar<SDPatternOperator OpNode,
                        ValueType Vt, ValueType it, RegisterOperand ROA,
                        RegisterOperand ROB, Instruction Inst>
  : Pat<(Vt (OpNode (Vt ROA:$RA), (i64 GPRCOpnd:$RB))),
        (Inst (Vt ROA:$RA), (i32 (COPY_TO_REGCLASS GPRCOpnd:$RB, ROB)))>;

class Vector_2OP_Reg_S32<SDPatternOperator OpNode,
                        ValueType Vt, RegisterOperand ROA, Instruction Inst>
  : Pat<(Vt (OpNode (Vt ROA:$RA), (Vt (Sw64VBroadCast (i64 GPRCOpnd:$RB))))),
        (Inst (Vt ROA:$RA), (i32 (COPY_TO_REGCLASS GPRCOpnd:$RB, FPRC_lo)))>;

class Vector_3OP_SameReg_Pat<SDPatternOperator OpNode,
                        ValueType Vt, RegisterOperand OR, Instruction Inst>
  : Pat<(OpNode (Vt OR:$RA), (Vt OR:$RB), (Vt OR:$RC)),
        (Inst OR:$RA, OR:$RB, OR:$RC)>;

multiclass SIMD_ARITH<bits<6> Opcode, bits<8>func,
                      string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                      Operand immtype, RegisterOperand RO,
                      SDPatternOperator IOp = null_frag,
                      ComplexPattern cpx = AddSubImm8Pat> {
  def rr : FPFormV<Opcode, func>, SIMD_3RR_SAME<instr_asm, OpNode, Vt, RO>;

  def ri : FPFormIV<Opcode, func>,
           SIMD_3RI_SAME<instr_asm, null_frag, Vt, immtype, RO>;

  def : Vector_2OP_Reg_Pat<IOp, Vt, Vt, RO, !cast<Instruction>(NAME # rr)>;

  def : Vector_2OP_Imm_VB_Pat<OpNode, Sw64VBroadCast, Vt, Vt,
        cpx, RO, !cast<Instruction>(NAME # ri)>;

  def : Vector_2OP_Imm_VB_Pat<IOp, Sw64VBroadCast, Vt, Vt,
        cpx, RO, !cast<Instruction>(NAME # ri)>;
}

defm VUCADDv16i16 : SIMD_ARITH<0x1A, 0x42, "vucaddh", add, v16i16,
                            s8imm, V256LOpnd, int_sw64_vucaddh_v16hi>;
defm VUCSUBv16i16 : SIMD_ARITH<0x1A, 0x43, "vucsubh", sub, v16i16,
                            s8imm, V256LOpnd, int_sw64_vucsubh_v16hi>;
defm VUCADDv32i8  : SIMD_ARITH<0x1A, 0x44, "vucaddb", add, v32i8,
                            s8imm, V256LOpnd, int_sw64_vucaddb_v32qi>;
defm VUCSUBv32i8  : SIMD_ARITH<0x1A, 0x45, "vucsubb", sub, v32i8,
                            s8imm, V256LOpnd, int_sw64_vucsubb_v32qi>;
defm VADDv8i32    : SIMD_ARITH<0x1A, 0x00, "vaddw", add, v8i32,
                            s8imm, V256LOpnd>;
defm VSUBv8i32    : SIMD_ARITH<0x1A, 0x01, "vsubw", sub, v8i32,
                            s8imm, V256LOpnd>;
defm VUCADDv8i32  : SIMD_ARITH<0x1A, 0x40, "vucaddw", add, v8i32,
                            s8imm, V256LOpnd, int_sw64_vucaddw>;
defm VUCSUBv8i32  : SIMD_ARITH<0x1A, 0x41, "vucsubw", sub, v8i32,
                            s8imm, V256LOpnd, int_sw64_vucsubw>;
defm VADDv4i64    : SIMD_ARITH<0x1A, 0x0E, "vaddl", add, v4i64,
                            s8imm, V256LOpnd>;
defm VSUBv4i64    : SIMD_ARITH<0x1A, 0x0F, "vsubl", sub, v4i64,
                            s8imm, V256LOpnd>;

def : Vector_2OP_Reg_Pat<int_sw64_vucaddb, v8i32, v8i32, V256LOpnd, VUCADDv32i8rr>;
def : Vector_2OP_Reg_Pat<int_sw64_vucsubb, v8i32, v8i32, V256LOpnd, VUCSUBv32i8rr>;
def : Vector_2OP_Reg_Pat<int_sw64_vucaddh, v8i32, v8i32, V256LOpnd, VUCADDv16i16rr>;
def : Vector_2OP_Reg_Pat<int_sw64_vucsubh, v8i32, v8i32, V256LOpnd, VUCSUBv16i16rr>;

def : Vector_1OP_Imm_Pat<int_sw64_vucaddbi, v8i32, AddSubImm8Pat, V256LOpnd, VUCADDv32i8ri>;
def : Vector_1OP_Imm_Pat<int_sw64_vucsubbi, v8i32, AddSubImm8Pat, V256LOpnd, VUCSUBv32i8ri>;
def : Vector_1OP_Imm_Pat<int_sw64_vucaddhi, v8i32, AddSubImm8Pat, V256LOpnd, VUCADDv16i16ri>;
def : Vector_1OP_Imm_Pat<int_sw64_vucsubhi, v8i32, AddSubImm8Pat, V256LOpnd, VUCSUBv16i16ri>;

class SIMD_3RR_VCMPGEW<string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                   RegisterOperand ROC = FPRCOpnd, RegisterOperand ROA = V256LOpnd,
                   RegisterOperand ROB = V256LOpnd> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
}

class SIMD_3RI_VCMPGEW<string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                   Operand immtype, RegisterOperand ROC = FPRCOpnd,
                   RegisterOperand ROA = V256LOpnd> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, immtype:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $Imm, $RC");
}

def VCMPGEWrr : FPFormV<0x1A, 0x02>,  SIMD_3RR_VCMPGEW<"vcmpgew", null_frag, v8i32>;
def VCMPGEWri : FPFormIV<0x1A, 0x02>, SIMD_3RI_VCMPGEW<"vcmpgew", null_frag, v8i32, u8imm>;

def : Pat<(int_sw64_vcmpgew (v8i32 V256LOpnd:$RA), (v8i32 V256LOpnd:$RB)),
          (i64 (FTOIStmp (VCMPGEWrr (v8i32 V256LOpnd:$RA), (v8i32 V256LOpnd:$RB))))>;
def : Pat<(int_sw64_vcmpgew (v8i32 V256LOpnd:$RA), (v8i32 (Sw64VBroadCast (i64 AddSubImm8Pat:$Imm)))),
          (i64 (FTOIStmp (VCMPGEWri (v8i32 V256LOpnd:$RA), $Imm)))>;

defm VCMPEQW  : SIMD_ARITH<0x1A, 0x03, "vcmpeqw",  seteq, v8i32,
                           u8imm, V256LOpnd, int_sw64_vcmpeqw,  AddSubImm8Pat>;
defm VCMPLEW  : SIMD_ARITH<0x1A, 0x04, "vcmplew",  setle, v8i32,
                           u8imm, V256LOpnd, int_sw64_vcmplew,  AddSubImm8Pat>;
defm VCMPLTW  : SIMD_ARITH<0x1A, 0x05, "vcmpltw",  setlt, v8i32,
                           u8imm, V256LOpnd, int_sw64_vcmpltw,  AddSubImm8Pat>;
defm VCMPULEW : SIMD_ARITH<0x1A, 0x06, "vcmpulew", setule, v8i32,
                           u8imm, V256LOpnd, int_sw64_vcmpulew, AddSubImm8Pat>;
defm VCMPULTW : SIMD_ARITH<0x1A, 0x07, "vcmpultw", setult, v8i32,
                           u8imm, V256LOpnd, int_sw64_vcmpultw, AddSubImm8Pat>;

defm VCMPUEQB : SIMD_ARITH<0x1A, 0x4B, "vcmpueqb", null_frag, v32i8,
                           u8imm, V256LOpnd, int_sw64_vcmpueqb, AddSubImm8Pat>;
defm VCMPUGTB : SIMD_ARITH<0x1A, 0x4C, "vcmpugtb", null_frag, v32i8,
                           u8imm, V256LOpnd, int_sw64_vcmpugtb, AddSubImm8Pat>;

class SIMD_2RR_BASE<string instr_asm,
                   RegisterOperand ROC, RegisterOperand ROA = ROC> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RC");
}

def CTPOPOW : FPFormV_CT<0x1A, 0x18>,
              SIMD_2RR_BASE<"ctpopow", FPRCOpnd, V256LOpnd>;
def CTLZOW  : FPFormV_CT<0x1A, 0x19>,
              SIMD_2RR_BASE<"ctlzow", FPRCOpnd, V256LOpnd>;

def VSUMv8i32 : FPFormV_CT<0x1A, 0x47>,
                SIMD_2RR_BASE<"vsumw", FPRCOpnd, V256LOpnd>;
def VSUMv4i64 : FPFormV_CT<0x1A, 0x48>,
                SIMD_2RR_BASE<"vsuml", FPRCOpnd, V256LOpnd>;

def : Pat<(int_sw64_vsumw (v8i32 V256LOpnd:$RA)),
          (i64 (FTOIStmp (i64 (VSUMv8i32 (v8i32 V256LOpnd:$RA)))))>;

def : Pat<(int_sw64_vsuml (v4i64 V256LOpnd:$RA)),
          (i64 (FTOITtmp (i64 (VSUMv4i64 (v4i64 V256LOpnd:$RA)))))>;

def : Pat<(int_sw64_ctpopow (v4i64 V256LOpnd:$RA)),
          (i64 (FTOIStmp (i64 (CTPOPOW (v4i64 V256LOpnd:$RA)))))>;

def : Pat<(int_sw64_ctlzow (v4i64 V256LOpnd:$RA)),
          (i64 (FTOIStmp (i64 (CTLZOW (v4i64 V256LOpnd:$RA)))))>;

class SIMD_3RR_SCALER<string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                   RegisterOperand ROC, RegisterOperand ROA = ROC,
                   RegisterOperand ROB = ROC> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
  list<dag> Pattern = [(set (Vt ROC:$RC), (OpNode (Vt ROA:$RA), ROB:$RB))];
}

multiclass SIMD_SHIFT<bits<6> Opcode, bits<8>func,
                      string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                      Operand immtype, RegisterOperand ROA, RegisterOperand ROB,
                      SDPatternOperator IOp = null_frag> {
  def rr : FPFormV<Opcode, func>,
           SIMD_3RR_SCALER<instr_asm, null_frag, Vt, ROA, ROA, ROB>;

  def ri : FPFormIV<Opcode, func>,
           SIMD_3RI_SAME<instr_asm, null_frag, Vt, immtype, ROA>;

  def : Vector_2OP_Reg_Scalar<OpNode, Vt, Vt, ROA, ROB,
                      !cast<Instruction>(NAME # rr)>;

  def : Vector_1OP_Imm_Pat<OpNode, Vt, AddSubImm8Pat, ROA, !cast<Instruction>(NAME # ri)>;
}

multiclass SIMD_Shift_Multi<bits<8> funcW, bits<8> funcB,bits<8> funcH,
                            bits<8> funcL, string instr_asm, RegisterOperand RO,
                            SDPatternOperator OpNode> {
defm v8i32  : SIMD_SHIFT<0x1A, funcW, instr_asm#w, OpNode, v8i32,
                        s8imm, V256LOpnd, RO>;
defm v16i16 : SIMD_SHIFT<0x1A, funcH, instr_asm#h, OpNode, v16i16,
                        s8imm, V256LOpnd, RO>;
defm v32i8  : SIMD_SHIFT<0x1A, funcB, instr_asm#b, OpNode, v32i8,
                        s8imm, V256LOpnd, RO>;
defm v4i64  : SIMD_SHIFT<0x1A, funcL, instr_asm#l, OpNode, v4i64,
                        s8imm, V256LOpnd, RO>;

def : Vector_2OP_Imm_VB_Pat<OpNode, Sw64VBroadCast, v8i32, v8i32,
      AddSubImm8Pat, V256LOpnd, !cast<Instruction>(NAME # v8i32 #ri)>;

def : Vector_2OP_Imm_VB_Pat<OpNode, Sw64VBroadCast, v16i16, v16i16,
      AddSubImm8Pat, V256LOpnd, !cast<Instruction>(NAME # v16i16 #ri)>;

def : Vector_2OP_Imm_VB_Pat<OpNode, Sw64VBroadCast, v32i8, v32i8,
      AddSubImm8Pat, V256LOpnd, !cast<Instruction>(NAME # v32i8 #ri)>;

def : Vector_2OP_Imm_VB_Pat<OpNode, Sw64VBroadCast, v4i64, v4i64,
      AddSubImm8Pat, V256LOpnd, !cast<Instruction>(NAME # v4i64 #ri)>;
}

defm VSLL : SIMD_Shift_Multi<0x08, 0x10, 0x14, 0x1A, "vsll",
                             FPRCloOpnd, int_sw64_vsll>;
defm VSRL : SIMD_Shift_Multi<0x09, 0x11, 0x15, 0x1B, "vsrl",
                             FPRCloOpnd, int_sw64_vsrl>;
defm VSRA : SIMD_Shift_Multi<0x0A, 0x12, 0x16, 0x1C, "vsra",
                             FPRCloOpnd, int_sw64_vsra>;
defm VROL : SIMD_Shift_Multi<0x0B, 0x13, 0x17, 0x1D, "vrol",
                             FPRCloOpnd, int_sw64_vrol>;

multiclass Vector_Shift_VB<SDPatternOperator OpNode, ValueType Vt,
                           string InstName> {
def : Vector_1OP_Imm_Pat<OpNode, Vt, AddSubImm8Pat, V256LOpnd,
      !cast<Instruction>(InstName # ri)>;
def : Vector_2OP_Reg_S32<OpNode, Vt, V256LOpnd,
      !cast<Instruction>(InstName # rr)>;
}

multiclass Vector_Shift<SDPatternOperator OpNode, string InstName> {
defm : Vector_Shift_VB<OpNode, v32i8 , InstName # v32i8 >;
defm : Vector_Shift_VB<OpNode, v16i16, InstName # v16i16>;
defm : Vector_Shift_VB<OpNode, v8i32 , InstName # v8i32 >;
defm : Vector_Shift_VB<OpNode, v4i64 , InstName # v4i64 >;
}

defm : Vector_Shift<int_sw64_vsll, "VSLL">;
defm : Vector_Shift<int_sw64_vsrl, "VSRL">;
defm : Vector_Shift<int_sw64_vsra, "VSRA">;
defm : Vector_Shift<int_sw64_vrol, "VROL">;

defm VSLLOW : SIMD_SHIFT<0x1A, 0x0C, "sllow", int_sw64_sllow, v4i64,
                         s8imm, V256LOpnd, FPRCloOpnd>;
defm VSRLOW : SIMD_SHIFT<0x1A, 0x0D, "srlow", int_sw64_srlow, v4i64,
                         s8imm, V256LOpnd, FPRCloOpnd>;
defm VSRAOW : SIMD_SHIFT<0x1A, 0x46, "sraow", int_sw64_sraow, v4i64,
                         s8imm, V256LOpnd, FPRCloOpnd>;

def  : Pat<(int_sw64_vslls (v4f32 V256LOpnd:$RA), (i64 AddSubImm8Pat:$Imm)),
        (VSLLOWri V256LOpnd:$RA, $Imm)>;
def  : Pat<(int_sw64_vslld (v4f64 V256LOpnd:$RA), (i64 AddSubImm8Pat:$Imm)),
        (VSLLOWri V256LOpnd:$RA, $Imm)>;

def  : Pat<(int_sw64_vsrls (v4f32 V256LOpnd:$RA), (i64 AddSubImm8Pat:$Imm)),
        (VSRLOWri V256LOpnd:$RA, $Imm)>;
def  : Pat<(int_sw64_vsrld (v4f64 V256LOpnd:$RA), (i64 AddSubImm8Pat:$Imm)),
        (VSRLOWri V256LOpnd:$RA, $Imm)>;

multiclass SIMD_LOGIC<bits<8> OpFunc,string instr_asm, RegisterOperand RO,
                      SDPatternOperator OpNode> {
def "" : FForm4LVLog<0x5, OpFunc>,
            SIMD_3RR_SAME<instr_asm, OpNode, v8i32, RO>;

def : Vector_2OP_Reg_Pat<OpNode, v16i16, v16i16, RO,
                        !cast<Instruction>(NAME)>;
def : Vector_2OP_Reg_Pat<OpNode, v32i8, v32i8, RO,
                        !cast<Instruction>(NAME)>;
def : Vector_2OP_Reg_Pat<OpNode, v4i64, v4i64, RO,
                        !cast<Instruction>(NAME)>;
}

defm VOR  : SIMD_LOGIC<0x54, "vbisw", V256LOpnd, or>;
defm VAND : SIMD_LOGIC<0x40, "vandw", V256LOpnd, and>;
defm VXOR : SIMD_LOGIC<0x1c, "vxorw", V256LOpnd, xor>;

defm VORNOT  : SIMD_LOGIC<0x51, "vornotw", V256LOpnd, vornot>;
defm VBIC    : SIMD_LOGIC<0x10, "vbicw",   V256LOpnd, vbic>;
defm VEQV    : SIMD_LOGIC<0x41, "veqvw",   V256LOpnd, veqv>;

def : Pat<(v8i32  immAllZerosV), (VOR  (v8i32 V31) , (v8i32 V31))>;
def : Pat<(v32i8  immAllZerosV), (VOR  (v32i8 V31) , (v32i8 V31))>;
def : Pat<(v16i16 immAllZerosV), (VOR  (v16i16 V31), (v16i16 V31))>;
def : Pat<(v4i64  immAllZerosV), (VOR  (v4i64 V31) , (v4i64 V31))>;

def : Pat<(v8i32  immAllOnesV), (VEQV  (v8i32 V31) , (v8i32 V31))>;
def : Pat<(v32i8  immAllOnesV), (VEQV  (v32i8 V31) , (v32i8 V31))>;
def : Pat<(v16i16 immAllOnesV), (VEQV  (v16i16 V31), (v16i16 V31))>;
def : Pat<(v4i64  immAllOnesV), (VEQV  (v4i64 V31) , (v4i64 V31))>;

class SIMD_INSERT_BASE<string instr_asm,
                  Operand ImmOp, ValueType vectype, ValueType eltVt,
                  RegisterOperand FPO = FPRCOpnd> {
  dag OutOperandList = (outs V256LOpnd:$RD);
  dag InOperandList = (ins FPO:$RA, V256LOpnd:$RB, ImmOp:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $Imm, $RD");

  list<dag> Pattern = [(set V256LOpnd:$RD,
              (vector_insert (vectype V256LOpnd:$RB),
                    (eltVt FPO:$RA), ImmOp:$Imm))];

}

multiclass SIMD_INSERT_Multi<bits<6> funcB, bits<6> funcH,bits<6> funcW,
                            bits<6> funcL, string instr_asm> {
def E8  : FForm4LV<0x1B, funcB>,
         SIMD_INSERT_BASE<instr_asm # b, VectorIndexB, v32i8,  i64>;

def E16 : FForm4LV<0x1B, funcH>,
         SIMD_INSERT_BASE<instr_asm # h, VectorIndexH, v16i16, i64>;

def E32 : FForm4LV<0x1B, funcW>,
         SIMD_INSERT_BASE<instr_asm # w, VectorIndexS, v8i32,  i32, FPRCloOpnd>;

def E64 : FForm4LV<0x1B, funcL>,
         SIMD_INSERT_BASE<instr_asm # f, VectorIndexD, v4f64,  f64>;
}

defm VINS : SIMD_INSERT_Multi<0x2A, 0x2B, 0x20, 0x21, "vins">;

def : Pat<(vector_insert (v4f32 V256LOpnd:$RB), (f32 FPRCloOpnd:$RA), VectorIndexD:$idx),
          (VINSE64 (f64 (COPY_TO_REGCLASS FPRCloOpnd:$RA, FPRC)), (v4f32 V256LOpnd:$RB), VectorIndexD:$idx)>;

def : Pat<(vector_insert (v4i64 V256LOpnd:$RB), (i64 FPRCOpnd:$RA), VectorIndexD:$idx),
          (VINSE64 (i64 FPRCOpnd:$RA), (v4i64 V256LOpnd:$RB), VectorIndexD:$idx)>;

class vins_pat<SDPatternOperator OpNode, Instruction Inst, RegisterClass RC,
               ValueType vectype, ValueType eltvt, Operand ImmOp>
    : Pat<(OpNode GPRCOpnd:$RA, (vectype V256LOpnd:$RB), ImmOp:$idx),
          (vectype (Inst (eltvt (COPY_TO_REGCLASS GPRCOpnd:$RA, RC)), (vectype V256LOpnd:$RB), ImmOp:$idx))>;

class vinselt<SDPatternOperator OpNode, Instruction Inst, RegisterClass RC,
               ValueType vectype, ValueType eltvt, Operand ImmOp>
    : Pat<(OpNode (vectype V256LOpnd:$RA), GPRCOpnd:$RB, ImmOp:$idx),
          (vectype (Inst (eltvt (COPY_TO_REGCLASS GPRCOpnd:$RB, RC)), (vectype V256LOpnd:$RA), ImmOp:$idx))>;

def : vins_pat<int_sw64_vinsw,  VINSE32, FPRC_lo, v8i32 , i32, VectorIndexS>;
def : vins_pat<int_sw64_vinsb,  VINSE8 , FPRC, v32i8 , i64, VectorIndexB>;
def : vins_pat<int_sw64_vinsh,  VINSE16, FPRC, v16i16, i64, VectorIndexH>;
def : vins_pat<int_sw64_vinsl,  VINSE64, FPRC, v4i64 , i64, VectorIndexD>;

def : vinselt<vector_insert,  VINSE32, FPRC_lo, v8i32 , i32, VectorIndexS>;
def : vinselt<vector_insert,  VINSE8 , FPRC, v32i8 , i64, VectorIndexB>;
def : vinselt<vector_insert,  VINSE16, FPRC, v16i16, i64, VectorIndexH>;
def : vinselt<vector_insert,  VINSE64, FPRC, v4i64 , i64, VectorIndexD>;

def : Pat<(int_sw64_vinsfs (f32 FPRCloOpnd:$RA),
              (v4f32 V256LOpnd:$RB), VectorIndexD:$idx),
          (v4f32 (VINSE64 (f64 (COPY_TO_REGCLASS FPRCloOpnd:$RA, FPRC)), (v4f32 V256LOpnd:$RB), VectorIndexD:$idx))>;
def : Pat<(int_sw64_vinsfd (f64 FPRCOpnd:$RA),
              (v4f64 V256LOpnd:$RB), VectorIndexD:$idx),
          (v4f64 (VINSE64 (f64 FPRCOpnd:$RA), (v4f64 V256LOpnd:$RB), VectorIndexD:$idx))>;

multiclass SIMD_COPY_Multi<bits<6> funcB, bits<6> funcH,bits<6> funcW,
                           bits<6> funcL, string instr_asm> {
def E8  : FForm2V<0x1B, funcB>,
          SIMD_2RR_BASE<instr_asm # b, V256LOpnd, FPRCOpnd>;

def E16 : FForm2V<0x1B, funcH>,
          SIMD_2RR_BASE<instr_asm # h, V256LOpnd, FPRCOpnd>;

def E32 : FForm2V<0x1B, funcW>,
          SIMD_2RR_BASE<instr_asm # w, V256LOpnd, FPRCloOpnd>;

def E64 : FForm2V<0x1B, funcL>,
          SIMD_2RR_BASE<instr_asm # f, V256LOpnd, FPRCOpnd>;

def : Pat <(v32i8 (Sw64VBroadCast GPRCOpnd:$RA)),
           (v32i8 (!cast<Instruction>(NAME # E8)
                  (i64 (COPY_TO_REGCLASS GPRCOpnd:$RA, FPRC))))>;

def : Pat <(v16i16 (Sw64VBroadCast GPRCOpnd:$RA)),
           (v16i16 (!cast<Instruction>(NAME # E16)
                  (i64 (COPY_TO_REGCLASS GPRCOpnd:$RA, FPRC))))>;

def : Pat <(v8i32 (Sw64VBroadCast GPRCOpnd:$RA)),
           (v8i32 (!cast<Instruction>(NAME # E32)
                  (i32 (COPY_TO_REGCLASS GPRCOpnd:$RA, FPRC_lo))))>;

def : Pat <(v4i64 (Sw64VBroadCast GPRCOpnd:$RA)),
           (v4i64 (!cast<Instruction>(NAME # E64)
                  (i64 (COPY_TO_REGCLASS GPRCOpnd:$RA, FPRC))))>;

def : Pat <(v4f64 (Sw64VBroadCast (f64 FPRCOpnd:$RA))),
           (v4f64 (!cast<Instruction>(NAME # E64) (f64 FPRCOpnd:$RA)))>;

def : Pat <(v4f32 (Sw64VBroadCast (f32 FPRCloOpnd:$RA))),
           (v4f32 (!cast<Instruction>(NAME # E64) (f64 (COPY_TO_REGCLASS FPRCloOpnd:$RA, FPRC))))>;
}

defm VCPY : SIMD_COPY_Multi<0x32, 0x33, 0x24, 0x25, "vcpy">;

multiclass SIMD_VINSECT_Multi<string instr_asm> {
def H : FForm4VINSECTL<0x1B, 0x2C>,
        SIMD_3RR_SAME<instr_asm # h, Sw64VINSECTL, v16i16, V256LOpnd>;

def W : FForm4VINSECTL<0x1B, 0x2D>,
        SIMD_3RR_SAME<instr_asm # w, Sw64VINSECTL, v8i32, V256LOpnd>;

def L : FForm4VINSECTL<0x1B, 0x2E>,
        SIMD_3RR_SAME<instr_asm # l, Sw64VINSECTL, v4i64, V256LOpnd>;

def B : FForm4VINSECTL<0x1B, 0x2F>,
        SIMD_3RR_SAME<instr_asm # b, Sw64VINSECTL, v32i8, V256LOpnd>;
}

defm VINSECTL : SIMD_VINSECT_Multi<"vinsectl">;

def VSHFQB : FForm4VINSECTL<0x1B, 0x31>,
             SIMD_3RR_SAME<"vshfqb", int_sw64_vshfqb, v32i8, V256LOpnd>;

class SIMD_4RI_BASE<string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                    Operand ImmOp, ImmLeaf Imm, RegisterOperand ROD,
                    RegisterOperand ROA = ROD, RegisterOperand ROB = ROD> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ROB:$RB, ImmOp:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $Imm, $RD");

  list<dag> Pattern = [(set ROD:$RD,
                      (OpNode (Vt ROA:$RA), (Vt ROB:$RB), Imm:$Imm))];
}

class SIMD_4RR_BASE<string instr_asm,
                    RegisterOperand ROC, RegisterOperand ROA,
                    RegisterOperand ROB = ROA, RegisterOperand ROD = ROA> {

  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ROB:$RB, ROC:$RC);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC, $RD");

}

def VSHFQ  : FForm4LV<0x1B, 0x30>,
             SIMD_4RI_BASE<"vshfq", int_sw64_vshfq, v8i32, u5imm, immZExt4Ptr, V256LOpnd>;

def VCONW  : FForm4LV2<0x1B, 0x26>,
             SIMD_4RR_BASE<"vconw", FPRCOpnd, V256LOpnd>;
def VCONS  : FForm4LV2<0x1B, 0x28>,
             SIMD_4RR_BASE<"vcons", FPRCOpnd, V256LOpnd>;
def VCOND  : FForm4LV2<0x1B, 0x29>,
             SIMD_4RR_BASE<"vcond", FPRCOpnd, V256LOpnd>;
def VSHFW  : FForm4LV2<0x1B, 0x27>,
             SIMD_4RR_BASE<"vshfw", FPRCOpnd, V256LOpnd>;

def : Pat<(int_sw64_vshfq (v8i32 V256LOpnd:$RA),
                (v8i32 V256LOpnd:$RB), (i64 ComplexImmPat:$imm)),
          (VSHFQ V256LOpnd:$RA, V256LOpnd:$RB, $imm)>;

def : Pat<(int_sw64_vconw (v8i32 V256LOpnd:$RA),
                (v8i32 V256LOpnd:$RB), (i64 GPRCOpnd:$RC)),
          (VCONW (v8i32 V256LOpnd:$RA), (v8i32 V256LOpnd:$RB),
                        (i64 (ITOFTtmp GPRCOpnd:$RC)))>;

def : Pat<(int_sw64_vcons (v4f32 V256LOpnd:$RA),
                (v4f32 V256LOpnd:$RB), (i64 GPRCOpnd:$RC)),
          (VCONS (v4f32 V256LOpnd:$RA), (v4f32 V256LOpnd:$RB),
                        (i64 (ITOFTtmp GPRCOpnd:$RC)))>;

def : Pat<(int_sw64_vcond (v4f64 V256LOpnd:$RA),
                (v4f64 V256LOpnd:$RB), (i64 GPRCOpnd:$RC)),
          (VCOND (v4f64 V256LOpnd:$RA), (v4f64 V256LOpnd:$RB),
                        (i64 (ITOFTtmp GPRCOpnd:$RC)))>;

def : Pat<(int_sw64_vconl (v4i64 V256LOpnd:$RA),
                (v4i64 V256LOpnd:$RB), (i64 GPRCOpnd:$RC)),
          (VCOND (v4i64 V256LOpnd:$RA), (v4i64 V256LOpnd:$RB),
                        (i64 (ITOFTtmp GPRCOpnd:$RC)))>;

def : Pat<(Sw64VSHF (v8i32 V256LOpnd:$RA),
                (v8i32 V256LOpnd:$RB), (i64 GPRCOpnd:$RC)),
          (VSHFW (v8i32 V256LOpnd:$RA), (v8i32 V256LOpnd:$RB),
                        (i64 (ITOFTtmp GPRCOpnd:$RC)))>;
def : Pat<(Sw64VSHF (v4i64 V256LOpnd:$RA),
                (v4i64 V256LOpnd:$RB), (i64 GPRCOpnd:$RC)),
          (VSHFW (v4i64 V256LOpnd:$RA), (v4i64 V256LOpnd:$RB),
                        (i64 (ITOFTtmp GPRCOpnd:$RC)))>;
def : Pat<(Sw64VSHF (v4f32 V256LOpnd:$RA),
                (v4f32 V256LOpnd:$RB), (i64 GPRCOpnd:$RC)),
          (VSHFW (v4f32 V256LOpnd:$RA), (v4f32 V256LOpnd:$RB),
                        (i64 (ITOFTtmp GPRCOpnd:$RC)))>;
def : Pat<(Sw64VSHF (v4f64 V256LOpnd:$RA),
                (v4f64 V256LOpnd:$RB), (i64 GPRCOpnd:$RC)),
          (VSHFW (v4f64 V256LOpnd:$RA), (v4f64 V256LOpnd:$RB),
                        (i64 (ITOFTtmp GPRCOpnd:$RC)))>;

def VEXTW : FForm4LVV<0x1B, 0x22>,
            SIMD_3RI_SAME<"vextw", null_frag, v8i32, u5imm, FPRCOpnd, V256LOpnd>;
def VEXTF : FForm4LVV<0x1B, 0x23>,
            SIMD_3RI_SAME<"vextf", null_frag, v4f32, u5imm, FPRCOpnd, V256LOpnd>;

multiclass Vector_extract_pat<SDPatternOperator Intr, Instruction Inst,
                              Instruction TransI, ValueType vecty,
                              ValueType ext_vt, SDPatternOperator Index> {
def : Pat<(ext_vt (vector_extract (vecty V256LOpnd:$RA), Index:$Idx)),
          (ext_vt (TransI (Inst (vecty V256LOpnd:$RA), Index:$Idx)))>;

def : Pat<(ext_vt (Intr (vecty V256LOpnd:$RA), Index:$Idx)),
          (ext_vt (TransI (Inst (vecty V256LOpnd:$RA), Index:$Idx)))>;
}

defm : Vector_extract_pat<int_sw64_vextw, VEXTW, FTOIStmp, v8i32, i64, VectorIndexS>;
defm : Vector_extract_pat<int_sw64_vextl, VEXTF, FTOITtmp, v4i64, i64, VectorIndexD>;

// TODO: How to Combine it with class pattern?
def : Pat<(f64 (vector_extract (v4f64 V256LOpnd:$RA), VectorIndexD:$Idx)),
          (f64 (VEXTF (v4f64 V256LOpnd:$RA), VectorIndexD:$Idx))>;
def : Pat<(f64 (int_sw64_vextfd (v4f64 V256LOpnd:$RA), VectorIndexD:$Idx)),
          (f64 (VEXTF (v4f64 V256LOpnd:$RA), VectorIndexD:$Idx))>;
def : Pat<(f32 (vector_extract (v4f32 V256LOpnd:$RA), VectorIndexD:$Idx)),
          (f32 (COPY_TO_REGCLASS (VEXTF (v4f32 V256LOpnd:$RA), VectorIndexD:$Idx), FPRC_lo))>;
def : Pat<(f32 (int_sw64_vextfs (v4f32 V256LOpnd:$RA), VectorIndexD:$Idx)),
          (f32 (COPY_TO_REGCLASS (VEXTF (v4f32 V256LOpnd:$RA), VectorIndexD:$Idx), FPRC_lo))>;

class SIMD_VLOGZZ<string instr_asm,
                  Operand ImmOp, ImmLeaf Imm, ValueType TyNode,
                  RegisterOperand ROD> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROD:$RA, ROD:$RB, ROD:$RC, ImmOp:$Imm);
  string AsmString = !strconcat(instr_asm # "$Imm", "\t$RA, $RB, $RC, $RD");

  list<dag> Pattern = [(set ROD:$RD,
                        (z_vlog (TyNode ROD:$RA), (TyNode ROD:$RB),
                         (TyNode ROD:$RC), Imm:$Imm))];
}

def VLOGZZ : FForm4LVLogZZ<0x5>,
             SIMD_VLOGZZ<"vlog", u8immHex, immZExt8Ptr, v4i64, V256LOpnd>;

multiclass SIMD_Floating_3RR<bits<6> Opcode, bits<8>func,
                      string instr_asm, SDPatternOperator OpNode> {
def "" : FPFormV<Opcode, func>,
         SIMD_3RR_SAME<instr_asm, null_frag, v4f64, V256LOpnd>;

def : Pat<(v4i64 (OpNode (v4f32 V256LOpnd:$RA), (v4f32 V256LOpnd:$RB))),
          (v4i64 (!cast<Instruction>(NAME) V256LOpnd:$RA, V256LOpnd:$RB))>;

def : Pat<(v4i64 (OpNode (v4f64 V256LOpnd:$RA), (v4f64 V256LOpnd:$RB))),
          (v4i64 (!cast<Instruction>(NAME) V256LOpnd:$RA, V256LOpnd:$RB))>;
}

defm VFCMPEQ : SIMD_Floating_3RR<0x1A, 0x8C, "vfcmpeq", setoeq>;
defm VFCMPLE : SIMD_Floating_3RR<0x1A, 0x8D, "vfcmple", setole>;
defm VFCMPLT : SIMD_Floating_3RR<0x1A, 0x8E, "vfcmplt", setolt>;
defm VFCMPUN : SIMD_Floating_3RR<0x1A, 0x8F, "vfcmpun", setuo>;


multiclass Vector_compare_pat<SDPatternOperator OpNode, Instruction Inst> {
def : Pat <(OpNode (v4f32 V256LOpnd:$RA), (v4f32 V256LOpnd:$RB)),
           (Inst V256LOpnd:$RA, V256LOpnd:$RB)>;
def : Pat <(OpNode (v4f64 V256LOpnd:$RA), (v4f64 V256LOpnd:$RB)),
           (Inst V256LOpnd:$RA, V256LOpnd:$RB)>;
}

defm : Vector_compare_pat<Sw64VFCMPEQ, VFCMPEQ>;
defm : Vector_compare_pat<Sw64VFCMPLE, VFCMPLE>;
defm : Vector_compare_pat<Sw64VFCMPLT, VFCMPLT>;
defm : Vector_compare_pat<Sw64VFCMPUN, VFCMPUN>;

def VCPYS  : FPFormV<0x1A, 0x90>,
    SIMD_3RR_SAME<"vcpys", int_sw64_vcpysd,  v4f64, V256LOpnd>;
def VCPYSE : FPFormV<0x1A, 0x91>,
    SIMD_3RR_SAME<"vcpyse", int_sw64_vcpysed, v4f64, V256LOpnd>;
def VCPYSN : FPFormV<0x1A, 0x92>,
    SIMD_3RR_SAME<"vcpysn", int_sw64_vcpysnd, v4f64, V256LOpnd>;

def : Pat<(int_sw64_vcpyss V256LOpnd:$RA, V256LOpnd:$RB),
          (VCPYS V256LOpnd:$RA, V256LOpnd:$RB)>;
def : Pat<(int_sw64_vcpyses V256LOpnd:$RA, V256LOpnd:$RB),
          (VCPYSE V256LOpnd:$RA, V256LOpnd:$RB)>;
def : Pat<(int_sw64_vcpysns V256LOpnd:$RA, V256LOpnd:$RB),
          (VCPYSN V256LOpnd:$RA, V256LOpnd:$RB)>;

multiclass SIMD_FMA<bits<6> funcS, bits<6> funcD,
                    string instr_asm, SDPatternOperator OpNode> {
def S : FForm4V<0x1B, funcS>,
        SIMD_4RR_SAME<instr_asm#s, OpNode, v4f32, V256LOpnd>;
def D : FForm4V<0x1B, funcD>,
        SIMD_4RR_SAME<instr_asm#d, OpNode, v4f64, V256LOpnd>;
}

defm VMA  : SIMD_FMA<0x00, 0x01, "vma", fma>;
defm VMS  : SIMD_FMA<0x02, 0x03, "vms",
  ThridOpFrag<(fma node:$LHS, node:$MHS, (fneg node:$RHS))> >;
defm VNMA : SIMD_FMA<0x04, 0x05, "vnma",
  ThridOpFrag<(fma (fneg node:$LHS), node:$MHS, node:$RHS)> >;
defm VNMS : SIMD_FMA<0x06, 0x07, "vnms",
  ThridOpFrag<(fneg (fma node:$LHS, node:$MHS, node:$RHS))> >;

multiclass SIMD_FLOAT_SEL<bits<6> func, string instr_asm,
                          SDPatternOperator OpNode> {
def "" : FForm4V<0x1B, func>,
         SIMD_4RR_SAME<instr_asm, OpNode, v4f64, V256LOpnd>;

def : Vector_3OP_SameReg_Pat<OpNode, v4f32, V256LOpnd,
                            !cast<Instruction>(NAME)>;
}

defm VFSELEQ : SIMD_FLOAT_SEL<0x10, "vfseleq", vfcmoveq>;
defm VFSELLT : SIMD_FLOAT_SEL<0x12, "vfsellt", vfcmovlt>;
defm VFSELLE : SIMD_FLOAT_SEL<0x13, "vfselle", vfcmovle>;

// 简单运算指令格式： 寄存器格式
class SIMD_3RV_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROC, RegisterOperand ROA = ROC,
                       RegisterOperand ROB = ROC,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
  InstrItinClass Itinerary = itin;

  string Constraints = "@earlyclobber $RC";
}

class SIMD_3RVV_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROC, RegisterOperand ROA = ROC,
                       RegisterOperand ROB = ROC,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
  InstrItinClass Itinerary = itin;
}

class SIMD_3RV_TY_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       ValueType TyNode,
                       RegisterOperand ROC, RegisterOperand ROA = ROC,
                       RegisterOperand ROB = ROC,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
  InstrItinClass Itinerary = itin;

  string Constraints = "@earlyclobber $RC";
}

class SIMD_VFCMPS_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROC, RegisterOperand ROA,
                       RegisterOperand ROB = ROA,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
  InstrItinClass Itinerary = itin;

  string Constraints = "@earlyclobber $RC";
}

class SIMD_2RV_R_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROC, RegisterOperand ROA,
                       RegisterOperand ROB,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
  InstrItinClass Itinerary = itin;
}

class SIMD_VSETGE_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROC, RegisterOperand ROA,
                       RegisterOperand ROB = ROA,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");

  bit usesCustomInserter = 1;
  InstrItinClass Itinerary = itin;
}

class SIMD_VSQRT_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROC, RegisterOperand ROB,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RB, $RC");
  InstrItinClass Itinerary = itin;

  string Constraints = "@earlyclobber $RC";
}

class SIMD_POPCNT_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       ValueType TyNode,
                       RegisterOperand ROC, RegisterOperand ROB,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RB, $RC");
  InstrItinClass Itinerary = itin;

  bit usesCustomInserter = 1;
}

class SIMD_REDUCE_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       ValueType TyNode,
                       RegisterOperand ROC, RegisterOperand ROA,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RC");
  InstrItinClass Itinerary = itin;

  bit usesCustomInserter = 1; // 6A should be extend.
}

// 简单运算指令格式： 立即数格式
class SIMD_I8_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       SplatComplexPattern SplatImm, RegisterOperand ROC,
                       RegisterOperand ROA = ROC,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, SplatImm.OpClass:$imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $imm, $RC");
  InstrItinClass Itinerary = itin;
}


// 浮点复核运算指令格式  寄存器格式
class SIMD_4RV_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROD, RegisterOperand ROA = ROD,
                       RegisterOperand ROB = ROD, RegisterOperand ROC = ROD,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ROB:$RB, ROC:$RC);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC, $RD");

  InstrItinClass Itinerary = itin;

  string Constraints = "@earlyclobber $RD";
}

class SIMD_4RV_DESC_SEL<string instr_asm, SDPatternOperator OpNode,
                       ValueType Vt, RegisterOperand ROD, RegisterOperand ROA = ROD,
                       RegisterOperand ROB = ROD, RegisterOperand ROC = ROD> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ROB:$RB, ROC:$RC);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC, $RD");

  list<dag> Pattern = [(set (Vt ROD:$RD), (OpNode (Vt ROA:$RA), (Vt ROB:$RB), (Vt ROC:$RC)))];
}

class SIMD_4RV_DESC_VNMSS<string instr_asm,
                       RegisterOperand ROD, RegisterOperand ROA = ROD,
                       RegisterOperand ROB = ROD, RegisterOperand ROC = ROD,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ROB:$RB, ROC:$RC);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC, $RD");

  InstrItinClass Itinerary = itin;

  string Constraints = "@earlyclobber $RD";
}


class SIMD_SELECT_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROD, RegisterOperand ROA = ROD,
                       RegisterOperand ROB = ROD, RegisterOperand ROC = ROD,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROC:$RC, ROB:$RB, ROA:$RA);
  string AsmString = !strconcat(instr_asm, "\t$RC, $RB, $RA, $RD");

  InstrItinClass Itinerary = itin;
}



class SIMD_VSETGE_I_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       ValueType VecTy, Operand ImmOp, ImmLeaf Imm,
                       RegisterOperand ROC, RegisterOperand ROA,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ImmOp:$imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $imm, $RC");

  bit usesCustomInserter = 1;
  InstrItinClass Itinerary = itin;
}

// Since we canonicalize buildvectors to v16i8, all vnots "-1" operands will be
// of that type.
def vnot_sw64 : PatFrag<(ops node:$in),
                       (xor node:$in, (bitconvert (v8i32 immAllOnesV)))>;

class SIMD_VBIC_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
			SDPatternOperator OpNode1, RegisterOperand ROC,
			RegisterOperand ROA = ROC,
                        RegisterOperand ROB = ROC,
                        InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");

  InstrItinClass Itinerary = itin;
}

class SIMD_VORNOT_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                        RegisterOperand ROC,
                        RegisterOperand ROA = ROC,
                        RegisterOperand ROB = ROC,
                        InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");

  InstrItinClass Itinerary = itin;
}

class SIMD_COPY_DESC_BASE<string instr_asm, ValueType TyNode,
                            SDPatternOperator OpNode, RegisterOperand ROB,
                            RegisterOperand ROA = ROB, bit Num,
                            InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROB:$RB);
  dag InOperandList = (ins ROA:$RA);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB");

  bit usesCustomInserter = Num; // 6A should be extend.
  InstrItinClass Itinerary = itin;
}

class SIMD_COPYF_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                            RegisterOperand ROB, RegisterOperand ROA = ROB,
                            InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROB:$RB);
  dag InOperandList = (ins ROA:$RA);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB");
  list<dag> Pattern = [];
  InstrItinClass Itinerary = itin;
}

class SIMD_COPYF_PSEUDO_BASE<ValueType VT, SDPatternOperator OpNode,
                              RegisterClass RCWD, RegisterClass RCWS = RCWD> :
      SIMDPseudo<(outs RCWD:$wd), (ins RCWS:$fs),
                [(set RCWD:$wd, (VT (OpNode RCWS:$fs)))]> {
  let usesCustomInserter = 1;
}

class SIMD_VSHIFT_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       ValueType TyNode, RegisterOperand ROC,
                       RegisterOperand ROA = ROC,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, F4RCOpnd:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC");
  InstrItinClass Itinerary = itin;

}

class SIMD_VINSECTL_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       RegisterOperand ROD,
                       RegisterOperand ROA = ROD, RegisterOperand ROB = ROD,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RD");
  InstrItinClass Itinerary = itin;

}

class SIMD_INSERT_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                           Operand ImmOp, ImmLeaf Imm, RegisterOperand ROD,
                           RegisterOperand ROA, bit Num,
                           InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ROD:$RB, ImmOp:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $Imm, $RD");
  InstrItinClass Itinerary = itin;

  bit usesCustomInserter = Num;
}

class SIMD_EXTRACT_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                            ValueType VecTy, Operand ImmOp, ImmLeaf Imm,
                            RegisterOperand ROD, RegisterOperand ROA,
                            bit Num, InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ImmOp:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $Imm, $RD");

  bit usesCustomInserter = Num;
  InstrItinClass Itinerary = itin;
}

class SIMD_MIX_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                            bit Num, RegisterOperand ROC, RegisterOperand ROA,
                            RegisterOperand ROB = ROA, RegisterOperand ROD = ROA,
                            InstrItinClass itin = NoItinerary> {

  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROA:$RA, ROB:$RB, ROC:$RC);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $RC, $RD");

  bit usesCustomInserter = Num;
  InstrItinClass Itinerary = itin;
}

class VADDWC_DESC : SIMD_3RVV_DESC_BASE<"vaddw", addv8i32, V256LOpnd>, IsCommutable;

class SIMD_2RV_SRi_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                       Operand ImmOp, ImmLeaf Imm, RegisterOperand ROC,
                       RegisterOperand ROA = ROC,
                       InstrItinClass itin = NoItinerary> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA, ImmOp:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $Imm, $RC");
  InstrItinClass Itinerary = itin;
}
class VSUMW_DESC : SIMD_REDUCE_DESC_BASE<"vsumw", vecreduce_add, v8i32, GPRCOpnd, V256LOpnd>;
class VSUML_DESC : SIMD_REDUCE_DESC_BASE<"vsuml", vecreduce_add, v4i64, GPRCOpnd, V256LOpnd>;

class VADDWC_ENC : FPFormV<0x1A, 0x00>;

class VSUMW_ENC : FPFormV_2RV<0x1A, 0x47>;
class VSUML_ENC : FPFormV_2RV<0x1A, 0x48>;

//--------------------------- Instruction defs ----------------------------------------//

class SIMD_VMAX_VMIN<bits<8>func, string instr_asm, SDPatternOperator OpNode,
                     ValueType vt, RegisterOperand RO>
  : FPFormV<0x1A, func>, SIMD_3RR_SAME<instr_asm, OpNode, vt, RO>, IsCommutable;

def VMAXB : SIMD_VMAX_VMIN<0x1E, "vmaxb", vmax, v32i8,  V256LOpnd>;
def VMINB : SIMD_VMAX_VMIN<0x1F, "vminb", vmin, v32i8,  V256LOpnd>;
def VMAXH : SIMD_VMAX_VMIN<0x50, "vmaxh", vmax, v16i16, V256LOpnd>;
def VMINH : SIMD_VMAX_VMIN<0x51, "vminh", vmin, v16i16, V256LOpnd>;
def VMAXW : SIMD_VMAX_VMIN<0x52, "vmaxw", vmax, v8i32,  V256LOpnd>;
def VMINW : SIMD_VMAX_VMIN<0x53, "vminw", vmin, v8i32,  V256LOpnd>;
def VMAXL : SIMD_VMAX_VMIN<0x54, "vmaxl", vmax, v4i64,  V256LOpnd>;
def VMINL : SIMD_VMAX_VMIN<0x55, "vminl", vmin, v4i64,  V256LOpnd>;

def VUMAXB : SIMD_VMAX_VMIN<0x56, "vumaxb", vumax, v32i8,  V256LOpnd>;
def VUMINB : SIMD_VMAX_VMIN<0x57, "vuminb", vumin, v32i8,  V256LOpnd>;
def VUMAXH : SIMD_VMAX_VMIN<0x58, "vumaxh", vumax, v16i16, V256LOpnd>;
def VUMINH : SIMD_VMAX_VMIN<0x59, "vuminh", vumin, v16i16, V256LOpnd>;
def VUMAXW : SIMD_VMAX_VMIN<0x5A, "vumaxw", vumax, v8i32,  V256LOpnd>;
def VUMINW : SIMD_VMAX_VMIN<0x5B, "vuminw", vumin, v8i32,  V256LOpnd>;
def VUMAXL : SIMD_VMAX_VMIN<0x5C, "vumaxl", vumax, v4i64,  V256LOpnd>;
def VUMINL : SIMD_VMAX_VMIN<0x5D, "vuminl", vumin, v4i64,  V256LOpnd>;

def VMAXS : SIMD_VMAX_VMIN<0xAC, "vmaxs", vmaxf, v4f32, V256LOpnd>;
def VMINS : SIMD_VMAX_VMIN<0xAD, "vmins", vminf, v4f32, V256LOpnd>;
def VMAXD : SIMD_VMAX_VMIN<0xAE, "vmaxd", vmaxf, v4f64, V256LOpnd>;
def VMIND : SIMD_VMAX_VMIN<0xAF, "vmind", vminf, v4f64, V256LOpnd>;


// For VSELXX pattern match with imm operand
multiclass SIMD_VSELXX<bits<6> Opcode, bits<6>func,
                      string instr_asm, SDPatternOperator OpNode, ValueType Vt,
                      Operand immtype, RegisterOperand RO,
                      SDPatternOperator IOp = null_frag,
                      ComplexPattern cpx = ComplexImmPat> {

  def rr : FForm4V<Opcode, func>, SIMD_4RV_DESC_SEL<instr_asm, OpNode, Vt, RO>;

  def ri : FForm4_VSELi<Opcode, func>, SIMD_4RI_BASE<instr_asm, OpNode, Vt, immtype, immZExt5Ptr, RO>;

  def : Pat<(Vt (OpNode (Vt RO:$RA), (Vt RO:$RB), (Vt (Sw64VBroadCast (i64 cpx:$Imm))))),
            (!cast<Instruction>(NAME # ri) (Vt RO:$RA), (Vt RO:$RB), $Imm)>;
}

defm VSELEQW  : SIMD_VSELXX<0x1B, 0x18, "vseleqw",  vseleqw,  v8i32, u5imm, V256LOpnd>;
defm VSELLBCW : SIMD_VSELXX<0x1B, 0x19, "vsellbcw", vsellbcw, v8i32, u5imm, V256LOpnd>;
defm VSELLTW  : SIMD_VSELXX<0x1B, 0x1A, "vselltw",  vselltw,  v8i32, u5imm, V256LOpnd>;
defm VSELLEW  : SIMD_VSELXX<0x1B, 0x1B, "vsellew",  vsellew,  v8i32, u5imm, V256LOpnd>;

class SIMD_ARITH_FLOAT<bits<6> Opcode, bits<8>func,
                       string instr_asm, SDPatternOperator OpNode,
                       ValueType Vt, RegisterOperand RO> :
      FPFormV<Opcode, func>, SIMD_3RR_SAME<instr_asm, OpNode, Vt, RO>;

def VADDS : SIMD_ARITH_FLOAT<0x1A, 0x80, "vadds", fadd, v4f32, V256LOpnd>;
def VADDD : SIMD_ARITH_FLOAT<0x1A, 0x81, "vaddd", fadd, v4f64, V256LOpnd>;
def VSUBS : SIMD_ARITH_FLOAT<0x1A, 0x82, "vsubs", fsub, v4f32, V256LOpnd>;
def VSUBD : SIMD_ARITH_FLOAT<0x1A, 0x83, "vsubd", fsub, v4f64, V256LOpnd>;
def VMULS : SIMD_ARITH_FLOAT<0x1A, 0x84, "vmuls", fmul, v4f32, V256LOpnd>;
def VMULD : SIMD_ARITH_FLOAT<0x1A, 0x85, "vmuld", fmul, v4f64, V256LOpnd>;
def VDIVS : SIMD_ARITH_FLOAT<0x1A, 0x86, "vdivs", fdiv, v4f32, V256LOpnd>;
def VDIVD : SIMD_ARITH_FLOAT<0x1A, 0x87, "vdivd", fdiv, v4f64, V256LOpnd>;


def vsqrt_sw :  SDNode<"Sw64ISD::VSQRT", SDT_VSQRT>;

class SIMD_VSQRT<string instr_asm, SDPatternOperator OpNode,
                           ValueType Vt, RegisterOperand ROC,
                           RegisterOperand ROB=ROC> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROB:$RB);
  string AsmString = !strconcat(instr_asm, "\t$RB, $RC");
  list<dag> Pattern = [(set (Vt ROC:$RC), (OpNode (Vt ROB:$RB)))];
}

def VSQRTS : FPFormV_2RV1<0x1A, 0x88>, SIMD_VSQRT<"vsqrts", vsqrt_sw, v4f32, V256LOpnd>;
def VSQRTD : FPFormV_2RV1<0x1A, 0x89>, SIMD_VSQRT<"vsqrtd", vsqrt_sw, v4f64, V256LOpnd>;

def Sw64VFREC : SDNode<"Sw64ISD::VFREC", SDT_ZVecFREC>;

def VFRECS : FPFormV_2RV1<0x1A, 0xAA>, SIMD_VSQRT<"vfrecs", Sw64VFREC, v4f32, V256LOpnd>;
def VFRECD : FPFormV_2RV1<0x1A, 0xAB>, SIMD_VSQRT<"vfrecd", Sw64VFREC, v4f64, V256LOpnd>;

class SIMD_VSUMF<string instr_asm, SDPatternOperator OpNode,
                       ValueType TyC, ValueType TyA,
                       RegisterOperand ROC, RegisterOperand ROA> {
  dag OutOperandList = (outs ROC:$RC);
  dag InOperandList = (ins ROA:$RA);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RC");
  list<dag> Pattern = [(set (TyC ROC:$RC), (OpNode (TyA ROA:$RA)))];
}

def VFCVTSD : FPFormV_2RV<0x1A, 0x95>, SIMD_VSUMF<"vfcvtsd", Sw64VFCVTSD, v4f64, v4f32, V256LOpnd, V256LOpnd>;
def VFCVTDS : FPFormV_2RV<0x1A, 0x96>, SIMD_VSUMF<"vfcvtds", Sw64VFCVTDS, v4f32, v4f64, V256LOpnd, V256LOpnd>;
def VFCVTLS : FPFormV_2RV<0x1A, 0x99>, SIMD_VSUMF<"vfcvtls", Sw64VFCVTLS, v4f32, v4i64, V256LOpnd, V256LOpnd>;
def VFCVTLD : FPFormV_2RV<0x1A, 0x9A>, SIMD_VSUMF<"vfcvtld", Sw64VFCVTLD, v4f64, v4i64, V256LOpnd, V256LOpnd>;

class SIMD_FCVTSH_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                           Operand ImmOp, ImmLeaf Imm, RegisterOperand ROD> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROD:$RA, ROD:$RB, ImmOp:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $RB, $Imm, $RD");
  list<dag> Pattern = [(set (v4f64 ROD:$RD), (OpNode (v4f32 ROD:$RA), (v4f32 ROD:$RB), Imm:$Imm))];
}

class SIMD_FCVTHS_DESC_BASE<string instr_asm, SDPatternOperator OpNode,
                           Operand ImmOp, ImmLeaf Imm, RegisterOperand ROD> {
  dag OutOperandList = (outs ROD:$RD);
  dag InOperandList = (ins ROD:$RA, ImmOp:$Imm);
  string AsmString = !strconcat(instr_asm, "\t$RA, $Imm, $RD");
  list<dag> Pattern = [(set (v4f32 ROD:$RD), (OpNode (v4f64 ROD:$RA), Imm:$Imm))];
}

def VFCVTSH : FForm4LV<0x1B, 0x35>,  SIMD_FCVTSH_DESC_BASE<"vfcvtsh", Sw64VFCVTSH, uimm5, immZExt5Ptr, V256LOpnd>;
def VFCVTHS : FForm4LV1<0x1B, 0x36>, SIMD_FCVTHS_DESC_BASE<"vfcvths", Sw64VFCVTHS, uimm5, immZExt5Ptr, V256LOpnd>;

def VFCVTDL  : FPFormV_2RV<0x1A, 0x9B>, SIMD_VSUMF<"vfcvtdl",   Sw64VFCVTDL,  v4i64, v4f32, V256LOpnd, V256LOpnd>;
def VFCVTDLG : FPFormV_2RV<0x1A, 0x9C>, SIMD_VSUMF<"vfcvtdl_g", Sw64VFCVTDLG, v4i64, v4f32, V256LOpnd, V256LOpnd>;
def VFCVTDLP : FPFormV_2RV<0x1A, 0x9D>, SIMD_VSUMF<"vfcvtdl_p", Sw64VFCVTDLP, v4i64, v4f32, V256LOpnd, V256LOpnd>;
def VFCVTDLZ : FPFormV_2RV<0x1A, 0x9E>, SIMD_VSUMF<"vfcvtdl_z", Sw64VFCVTDLZ, v4i64, v4f32, V256LOpnd, V256LOpnd>;
def VFCVTDLN : FPFormV_2RV<0x1A, 0x9F>, SIMD_VSUMF<"vfcvtdl_n", Sw64VFCVTDLN, v4i64, v4f32, V256LOpnd, V256LOpnd>;

def VFRIS  : FPFormV_2RV1<0x1A, 0xA0>, SIMD_VSQRT<"vfris",   Sw64VFRIS,  v4f32, V256LOpnd>;
def VFRISG : FPFormV_2RV1<0x1A, 0xA1>, SIMD_VSQRT<"vfris_g", Sw64VFRISG, v4f32, V256LOpnd>;
def VFRISP : FPFormV_2RV1<0x1A, 0xA2>, SIMD_VSQRT<"vfris_p", Sw64VFRISP, v4f32, V256LOpnd>;
def VFRISZ : FPFormV_2RV1<0x1A, 0xA3>, SIMD_VSQRT<"vfris_z", Sw64VFRISZ, v4f32, V256LOpnd>;
def VFRISN : FPFormV_2RV1<0x1A, 0xA4>, SIMD_VSQRT<"vfris_n", Sw64VFRISN, v4f32, V256LOpnd>;
def VFRID  : FPFormV_2RV1<0x1A, 0xA5>, SIMD_VSQRT<"vfrid",   Sw64VFRID,  v4f64, V256LOpnd>;
def VFRIDG : FPFormV_2RV1<0x1A, 0xA6>, SIMD_VSQRT<"vfrid_g", Sw64VFRIDG, v4f64, V256LOpnd>;
def VFRIDP : FPFormV_2RV1<0x1A, 0xA7>, SIMD_VSQRT<"vfrid_p", Sw64VFRIDP, v4f64, V256LOpnd>;
def VFRIDZ : FPFormV_2RV1<0x1A, 0xA8>, SIMD_VSQRT<"vfrid_z", Sw64VFRIDZ, v4f64, V256LOpnd>;
def VFRIDN : FPFormV_2RV1<0x1A, 0xA9>, SIMD_VSQRT<"vfrid_n", Sw64VFRIDN, v4f64, V256LOpnd>;

def vsumf :  SDNode<"Sw64ISD::VSUMF", SDT_VSUMF>;

def VSUMS : FPFormV_2RV<0x1A, 0x93>, SIMD_VSUMF<"vsums", vsumf, f32, v4f32, F4RCOpnd, V256LOpnd>;
def VSUMD : FPFormV_2RV<0x1A, 0x94>, SIMD_VSUMF<"vsumd", vsumf, f64, v4f64, F8RCOpnd, V256LOpnd>;

// Patterns.
class SIMDPat<dag pattern, dag result, list<Predicate> pred = [HasSIMD]> :
  Pat<pattern, result>, Requires<pred>;

// ------------------------
class Sw64Pat<dag pattern, dag result> : Pat<pattern, result>;

// TODO: Add support for FPOpFusion::Standard
def AllowFPOpFusion : Predicate<"TM.Options.AllowFPOpFusion =="
                                " FPOpFusion::Fast">;

class ASE_SIMD {
  list<Predicate> ASEPredicate = [HasSIMD];
}


class FPOP_FUSION_FAST {
  list <Predicate> AdditionalPredicates = [AllowFPOpFusion];
}


// Additional VNMSX patterns: -a*b + c == -(a*b - c)
multiclass Vecotr_fma_pat<SDPatternOperator OpNode, string Inst> {
def : Vector_3OP_SameReg_Pat<OpNode, v4f32, V256LOpnd,
          !cast<Instruction>(Inst#S)>;

def : Vector_3OP_SameReg_Pat<OpNode, v4f64, V256LOpnd,
          !cast<Instruction>(Inst#D)>;
}

defm : Vecotr_fma_pat<ThridOpFrag<(fma (fneg node:$LHS), node:$MHS, node:$RHS)>, "VNMA">;
defm : Vecotr_fma_pat<ThridOpFrag<(fma node:$LHS, (fneg node:$MHS), node:$RHS)>, "VNMA">;

def : Pat<(int_sw64_vnmsd V256LOpnd:$RA, V256LOpnd:$RB, V256LOpnd:$RC),
          (VNMSD $RA, $RB, $RC)>;

def : Pat<(fneg v4f64:$RA), (VCPYSN $RA, $RA)>;
def : Pat<(fneg v4f32:$RA), (VCPYSN $RA, $RA)>;

def :Pat<(v4f32 (fadd (v4f32 V256LOpnd:$RA), (v4f32 V256LOpnd:$RB))),
         (VADDD V256LOpnd:$RA, V256LOpnd:$RB)>;

class bitconvert_pat<ValueType dstTy, ValueType srcTy>
 : Pat<(dstTy  (bitconvert (srcTy V256LOpnd:$RA))), (dstTy V256LOpnd:$RA)>;

def : bitconvert_pat<v32i8, v16i16>;
def : bitconvert_pat<v32i8, v8i32>;
def : bitconvert_pat<v32i8, v4i64>;
def : bitconvert_pat<v32i8, v4f64>;

def : bitconvert_pat<v16i16, v8i32>;
def : bitconvert_pat<v16i16, v32i8>;
def : bitconvert_pat<v16i16, v4i64>;
def : bitconvert_pat<v16i16, v4f64>;

def : bitconvert_pat<v8i32, v16i16>;
def : bitconvert_pat<v8i32, v32i8>;
def : bitconvert_pat<v8i32, v4i64>;
def : bitconvert_pat<v8i32, v4f64>;

def : bitconvert_pat<v4i64, v16i16>;
def : bitconvert_pat<v4i64, v32i8>;
def : bitconvert_pat<v4i64, v8i32>;
def : bitconvert_pat<v4i64, v4f64>;

def : bitconvert_pat<v4f64, v16i16>;
def : bitconvert_pat<v4f64, v32i8>;
def : bitconvert_pat<v4f64, v8i32>;
def : bitconvert_pat<v4f64, v4i64>;

