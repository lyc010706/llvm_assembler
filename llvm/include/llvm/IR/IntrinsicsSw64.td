//==- IntrinsicsSw64.td - Sw64 intrinsics                 -*- tablegen -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the Sw64-specific intrinsics.
//
//===----------------------------------------------------------------------===//

let TargetPrefix = "sw64" in {  // All intrinsics start with "llvm.sw64.".
def int_sw64_umulh : ClangBuiltin<"__builtin_sw_64_umulh">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty], [IntrNoMem]>;

def int_sw64_crc32b : ClangBuiltin<"__builtin_sw64_crc32b">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_crc32h : ClangBuiltin<"__builtin_sw64_crc32h">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_crc32w : ClangBuiltin<"__builtin_sw64_crc32w">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_crc32l : ClangBuiltin<"__builtin_sw64_crc32l">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_crc32cb : ClangBuiltin<"__builtin_sw64_crc32cb">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_crc32ch : ClangBuiltin<"__builtin_sw64_crc32ch">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_crc32cw : ClangBuiltin<"__builtin_sw64_crc32cw">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_crc32cl : ClangBuiltin<"__builtin_sw64_crc32cl">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;

def int_sw64_sbt : ClangBuiltin<"__builtin_sw64_sbt">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_cbt : ClangBuiltin<"__builtin_sw64_cbt">,
  Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
  [IntrNoMem]>;

def int_sw64_ctpopow : ClangBuiltin<"__builtin_sw_ctpopow">,
  Intrinsic<[llvm_i64_ty], [llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_ctlzow : ClangBuiltin<"__builtin_sw_ctlzow">,
  Intrinsic<[llvm_i64_ty], [llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_reduc_plusw : ClangBuiltin<"__builtin_sw_reduc_plusw">,
  Intrinsic<[llvm_i32_ty], [llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_reduc_pluss : ClangBuiltin<"__builtin_sw_reduc_pluss">,
  Intrinsic<[llvm_float_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_reduc_plusd : ClangBuiltin<"__builtin_sw_reduc_plusd">,
  Intrinsic<[llvm_double_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;

// SIMD Intrincs
def int_sw64_vaddw : ClangBuiltin<"__builtin_sw_vaddw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vucaddw  : ClangBuiltin<"__builtin_sw_vucaddw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vucaddwi : ClangBuiltin<"__builtin_sw_vucaddwi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vucsubw : ClangBuiltin<"__builtin_sw_vucsubw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vucsubwi : ClangBuiltin<"__builtin_sw_vucsubwi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vucaddh : ClangBuiltin<"__builtin_sw_vucaddh">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vucaddhi : ClangBuiltin<"__builtin_sw_vucaddhi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vucsubh : ClangBuiltin<"__builtin_sw_vucsubh">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vucsubhi : ClangBuiltin<"__builtin_sw_vucsubhi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vucaddb : ClangBuiltin<"__builtin_sw_vucaddb">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vucaddbi : ClangBuiltin<"__builtin_sw_vucaddbi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vucsubb : ClangBuiltin<"__builtin_sw_vucsubb">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vucsubbi : ClangBuiltin<"__builtin_sw_vucsubbi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vslls : ClangBuiltin<"__builtin_sw_vslls">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vslld : ClangBuiltin<"__builtin_sw_vslld">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsrls : ClangBuiltin<"__builtin_sw_vsrls">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsrld : ClangBuiltin<"__builtin_sw_vsrld">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_i64_ty],
  [IntrNoMem]>;

class sw64VectorIntArg
      : Intrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>, llvm_anyint_ty],
      [IntrNoMem]>;
def int_sw64_vsll : sw64VectorIntArg;
def int_sw64_vsrl : sw64VectorIntArg;
def int_sw64_vsra : sw64VectorIntArg;
def int_sw64_vrol : sw64VectorIntArg;

def int_sw64_vsllw : ClangBuiltin<"__builtin_sw_vsllw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsllh : ClangBuiltin<"__builtin_sw_vsllh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsllb : ClangBuiltin<"__builtin_sw_vslln">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vslll : ClangBuiltin<"__builtin_sw_vslll">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsrlw : ClangBuiltin<"__builtin_sw_vsrlw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsrlh : ClangBuiltin<"__builtin_sw_vsrlh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsrlb : ClangBuiltin<"__builtin_sw_vsrlb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsrll : ClangBuiltin<"__builtin_sw_vsrll">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsraw : ClangBuiltin<"__builtin_sw_vsraw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsrah : ClangBuiltin<"__builtin_sw_vsrah">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsrab : ClangBuiltin<"__builtin_sw_vsrab">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsral : ClangBuiltin<"__builtin_sw_vsral">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;

def int_sw64_vrolw : ClangBuiltin<"__builtin_sw_vrolw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vrolwi : ClangBuiltin<"__builtin_sw_vrolwi">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vrolb : ClangBuiltin<"__builtin_sw_vrolb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vrolbi : ClangBuiltin<"__builtin_sw_vrolbi">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vrolh : ClangBuiltin<"__builtin_sw_vrolh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vrolhi : ClangBuiltin<"__builtin_sw_vrolhi">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vroll : ClangBuiltin<"__builtin_sw_vroll">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vrolli : ClangBuiltin<"__builtin_sw_vrolli">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i32_ty],
  [IntrNoMem]>;

def int_sw64_sllow : ClangBuiltin<"__builtin_sw_sllow">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_srlow : ClangBuiltin<"__builtin_sw_srlow">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_sllowi : ClangBuiltin<"__builtin_sw_sllowi">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_srlowi : ClangBuiltin<"__builtin_sw_srlowi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i32_ty],
  [IntrNoMem]>;

def int_sw64_vsqrts : ClangBuiltin<"__builtin_sw_vsqrts">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vsqrtd : ClangBuiltin<"__builtin_sw_vsqrtd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;

def int_sw64_vornotw : ClangBuiltin<"__builtin_sw_vornotw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_veqvw : ClangBuiltin<"__builtin_sw_veqvw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;

def int_sw64_vfcmpeqs : ClangBuiltin<"__builtin_sw_vfcmpeqs">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfcmples : ClangBuiltin<"__builtin_sw_vfcmples">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfcmplts : ClangBuiltin<"__builtin_sw_vfcmplts">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfcmpuns : ClangBuiltin<"__builtin_sw_vfcmpuns">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfcmpeqd : ClangBuiltin<"__builtin_sw_vfcmpeqd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcmpled : ClangBuiltin<"__builtin_sw_vfcmpled">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcmpltd : ClangBuiltin<"__builtin_sw_vfcmpltd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcmpund : ClangBuiltin<"__builtin_sw_vfcmpund">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vcpyss : ClangBuiltin<"__builtin_sw_vcpyss">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vcpysns : ClangBuiltin<"__builtin_sw_vcpysns">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vcpyses : ClangBuiltin<"__builtin_sw_vcpyses">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vcpysd : ClangBuiltin<"__builtin_sw_vcpysd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vcpysnd : ClangBuiltin<"__builtin_sw_vcpysnd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vcpysed : ClangBuiltin<"__builtin_sw_vcpysed">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vseleqw : ClangBuiltin<"__builtin_sw_vseleqw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vseleqwi : ClangBuiltin<"__builtin_sw_vseleqwi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vselltw : ClangBuiltin<"__builtin_sw_vselltw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vselltwi : ClangBuiltin<"__builtin_sw_vselltwi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsellew : ClangBuiltin<"__builtin_sw_vsellew">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vsellewi : ClangBuiltin<"__builtin_sw_vsellewi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vsellbcw : ClangBuiltin<"__builtin_sw_vsellbcw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vsellbcwi : ClangBuiltin<"__builtin_sw_vsellbcwi">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vfseleqs : ClangBuiltin<"__builtin_sw_vfseleqs">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfselles : ClangBuiltin<"__builtin_sw_vfselles">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfsellts : ClangBuiltin<"__builtin_sw_vfsellts">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfseleqd : ClangBuiltin<"__builtin_sw_vfseleqd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfselled : ClangBuiltin<"__builtin_sw_vfselled">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfselltd : ClangBuiltin<"__builtin_sw_vfselltd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;

// Multiply-add instructions
// FIXME
def int_sw64_vnmsd : ClangBuiltin<"__builtin_sw_vnmsd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;

// Vector Insert Intrinsic
def int_sw64_vinsfs : ClangBuiltin<"__builtin_sw_vinsfs">,
  Intrinsic<[llvm_v4f32_ty], [llvm_float_ty, llvm_v4f32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vinsfd : ClangBuiltin<"__builtin_sw_vinsfd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_double_ty, llvm_v4f64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vinsw : ClangBuiltin<"__builtin_sw_vinsw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_i64_ty, llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vinsb  : ClangBuiltin<"__builtin_sw_vinsb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_i64_ty, llvm_v32i8_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vinsh  : ClangBuiltin<"__builtin_sw_vinsh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_i64_ty, llvm_v16i16_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vinsl : ClangBuiltin<"__builtin_sw_vinsl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_i64_ty, llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;

def int_sw64_vextfs : ClangBuiltin<"__builtin_sw_vextfs">,
  Intrinsic<[llvm_float_ty], [llvm_v4f32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vextfd : ClangBuiltin<"__builtin_sw_vextfd">,
  Intrinsic<[llvm_double_ty], [llvm_v4f64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vextw : ClangBuiltin<"__builtin_sw_vextw">,
  Intrinsic<[llvm_i64_ty], [llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vextl : ClangBuiltin<"__builtin_sw_vextl">,
  Intrinsic<[llvm_i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vconw : ClangBuiltin<"__builtin_sw_vconw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_ptr_ty],
  [IntrNoMem, IntrArgMemOnly]>;
def int_sw64_vconl : ClangBuiltin<"__builtin_sw_vconl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty, llvm_ptr_ty],
  [IntrNoMem, IntrArgMemOnly]>;
def int_sw64_vcons : ClangBuiltin<"__builtin_sw_vcons">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_ptr_ty],
  [IntrNoMem, IntrArgMemOnly]>;
def int_sw64_vcond : ClangBuiltin<"__builtin_sw_vcond">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty, llvm_ptr_ty],
  [IntrNoMem, IntrArgMemOnly]>;

def int_sw64_vlogzz : ClangBuiltin<"__builtin_sw_vlogzz">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty, llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;

def int_sw64_vshfw : ClangBuiltin<"__builtin_sw_vshfw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vucaddh_v16hi : ClangBuiltin<"__builtin_sw_vucaddh_v16hi">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_vucaddhi_v16hi : ClangBuiltin<"__builtin_sw_vucaddhi_v16hi">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_vucsubh_v16hi : ClangBuiltin<"__builtin_sw_vucsubh_v16hi">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_vucsubhi_v16hi : ClangBuiltin<"__builtin_sw_vucsubhi_v16hi">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_vucaddb_v32qi : ClangBuiltin<"__builtin_sw_vucaddb_v32qi">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vucaddbi_v32qi : ClangBuiltin<"__builtin_sw_vucaddbi_v32qi">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vucsubb_v32qi : ClangBuiltin<"__builtin_sw_vucsubb_v32qi">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vucsubbi_v32qi : ClangBuiltin<"__builtin_sw_vucsubbi_v32qi">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vmaxb : ClangBuiltin<"__builtin_sw_vmaxb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vminb : ClangBuiltin<"__builtin_sw_vminb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vmaxh : ClangBuiltin<"__builtin_sw_vmaxh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_vminh : ClangBuiltin<"__builtin_sw_vminh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_vmaxw : ClangBuiltin<"__builtin_sw_vmaxw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vminw : ClangBuiltin<"__builtin_sw_vminw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vmaxl : ClangBuiltin<"__builtin_sw_vmaxl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_vminl : ClangBuiltin<"__builtin_sw_vminl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_vumaxb : ClangBuiltin<"__builtin_sw_vumaxb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vuminb : ClangBuiltin<"__builtin_sw_vuminb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vumaxh : ClangBuiltin<"__builtin_sw_vumaxh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_vuminh : ClangBuiltin<"__builtin_sw_vuminh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_vumaxw : ClangBuiltin<"__builtin_sw_vumaxw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vuminw : ClangBuiltin<"__builtin_sw_vuminw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vumaxl : ClangBuiltin<"__builtin_sw_vumaxl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_vuminl : ClangBuiltin<"__builtin_sw_vuminl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_sraow : ClangBuiltin<"__builtin_sw_sraow">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_sraowi : ClangBuiltin<"__builtin_sw_sraowi">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_i32_ty],
  [IntrNoMem]>;

def int_sw64_vcmpgew : ClangBuiltin<"__builtin_sw_vcmpgew">,
  Intrinsic<[llvm_i64_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vcmpeqw : ClangBuiltin<"__builtin_sw_vcmpeqw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vcmplew : ClangBuiltin<"__builtin_sw_vcmplew">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vcmpltw : ClangBuiltin<"__builtin_sw_vcmpltw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vcmpulew : ClangBuiltin<"__builtin_sw_vcmpulew">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vcmpultw : ClangBuiltin<"__builtin_sw_vcmpultw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;

def int_sw64_vcmpueqb : ClangBuiltin<"__builtin_sw_vcmpueqb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vcmpugtb : ClangBuiltin<"__builtin_sw_vcmpugtb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_vsumw : ClangBuiltin<"__builtin_sw_vsumw">,
  Intrinsic<[llvm_i64_ty], [llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vsuml : ClangBuiltin<"__builtin_sw_vsuml">,
  Intrinsic<[llvm_i64_ty], [llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_vbinvw : ClangBuiltin<"__builtin_sw_vbinvw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vwinv : ClangBuiltin<"__builtin_sw_vwinv">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_vseleql : ClangBuiltin<"__builtin_sw_vseleql">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty, llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_veqvb : ClangBuiltin<"__builtin_sw_veqvb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;
def int_sw64_veqvh : ClangBuiltin<"__builtin_sw_veqvh">,
  Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
  [IntrNoMem]>;
def int_sw64_veqvl : ClangBuiltin<"__builtin_sw_veqvl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_vsums : ClangBuiltin<"__builtin_sw_vsums">,
  Intrinsic<[llvm_float_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vsumd : ClangBuiltin<"__builtin_sw_vsumd">,
  Intrinsic<[llvm_double_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfrecs : ClangBuiltin<"__builtin_sw_vfrecs">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfrecd : ClangBuiltin<"__builtin_sw_vfrecd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfris : ClangBuiltin<"__builtin_sw_vfris">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfris_g : ClangBuiltin<"__builtin_sw_vfris_g">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfris_p : ClangBuiltin<"__builtin_sw_vfris_p">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfris_z : ClangBuiltin<"__builtin_sw_vfris_z">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfris_n : ClangBuiltin<"__builtin_sw_vfris_n">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfrid : ClangBuiltin<"__builtin_sw_vfrid">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfrid_g : ClangBuiltin<"__builtin_sw_vfrid_g">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfrid_p : ClangBuiltin<"__builtin_sw_vfrid_p">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfrid_z : ClangBuiltin<"__builtin_sw_vfrid_z">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfrid_n : ClangBuiltin<"__builtin_sw_vfrid_n">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vmaxs : ClangBuiltin<"__builtin_sw_vmaxs">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vmins : ClangBuiltin<"__builtin_sw_vmins">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vmaxd : ClangBuiltin<"__builtin_sw_vmaxd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vmind : ClangBuiltin<"__builtin_sw_vmind">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtsd : ClangBuiltin<"__builtin_sw_vfcvtsd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtds : ClangBuiltin<"__builtin_sw_vfcvtds">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtsh : ClangBuiltin<"__builtin_sw_vfcvtsh">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvths : ClangBuiltin<"__builtin_sw_vfcvths">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4f64_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtls : ClangBuiltin<"__builtin_sw_vfcvtls">,
  Intrinsic<[llvm_v4f32_ty], [llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtld : ClangBuiltin<"__builtin_sw_vfcvtld">,
  Intrinsic<[llvm_v4f64_ty], [llvm_v4i64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtdl : ClangBuiltin<"__builtin_sw_vfcvtdl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtdl_g : ClangBuiltin<"__builtin_sw_vfcvtdl_g">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtdl_p : ClangBuiltin<"__builtin_sw_vfcvtdl_p">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtdl_z : ClangBuiltin<"__builtin_sw_vfcvtdl_z">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtdl_n : ClangBuiltin<"__builtin_sw_vfcvtdl_n">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4f64_ty],
  [IntrNoMem]>;
def int_sw64_vfcvtsl  : ClangBuiltin<"__builtin_sw_vfcvtsl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_v4f32_ty],
  [IntrNoMem]>;

def int_sw64_vshfq : ClangBuiltin<"__builtin_sw_vshfq">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_i64_ty],
  [IntrNoMem]>;
def int_sw64_vshfqb : ClangBuiltin<"__builtin_sw_vshfqb">,
  Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty],
  [IntrNoMem]>;

def int_sw64_vsm3msw  : ClangBuiltin<"__builtin_sw_vsm3msw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;
def int_sw64_vsm3r  : ClangBuiltin<"__builtin_sw_vsm3r">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vsm4key  : ClangBuiltin<"__builtin_sw_vsm4key">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i32_ty],
  [IntrNoMem]>;
def int_sw64_vsm4r  : ClangBuiltin<"__builtin_sw_vsm4r">,
  Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
  [IntrNoMem]>;

def int_sw64_vldw : ClangBuiltin<"__builtin_sw_vldw">,
  Intrinsic<[llvm_v8i32_ty], [llvm_ptr_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_vldl : ClangBuiltin<"__builtin_sw_vldl">,
  Intrinsic<[llvm_v4i64_ty], [llvm_ptr_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_vlds : ClangBuiltin<"__builtin_sw_vlds">,
  Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_vldd : ClangBuiltin<"__builtin_sw_vldd">,
  Intrinsic<[llvm_v4f64_ty], [llvm_ptr_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;

def int_sw64_vload : Intrinsic<[llvm_anyvector_ty], [llvm_anyptr_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_vloadu : Intrinsic<[llvm_anyvector_ty], [llvm_anyptr_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_vload_u : Intrinsic<[llvm_anyvector_ty], [llvm_anyptr_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_vloade : Intrinsic<[llvm_anyvector_ty], [llvm_anyptr_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_vloadnc : Intrinsic<[llvm_anyvector_ty], [llvm_anyptr_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_vstore   : Intrinsic<[], [llvm_anyvector_ty, llvm_anyptr_ty],
  [IntrWriteMem, IntrArgMemOnly]>;
def int_sw64_vstoreu  : Intrinsic<[], [llvm_anyvector_ty, llvm_anyptr_ty],
  [IntrWriteMem, IntrArgMemOnly]>;
def int_sw64_vstore_u : Intrinsic<[], [llvm_anyvector_ty, llvm_anyptr_ty],
  [IntrWriteMem, IntrArgMemOnly]>;
def int_sw64_vstoreuh : Intrinsic<[], [llvm_anyvector_ty, llvm_anyptr_ty],
  [IntrWriteMem, IntrArgMemOnly]>;
def int_sw64_vstoreul : Intrinsic<[], [llvm_anyvector_ty, llvm_anyptr_ty],
  [IntrWriteMem, IntrArgMemOnly]>;
def int_sw64_vstorenc : Intrinsic<[], [llvm_anyvector_ty, llvm_anyptr_ty],
  [IntrWriteMem, IntrArgMemOnly]>;

def int_sw64_loadu : ClangBuiltin<"__builtin_sw_loadu">,
  Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_loadu_i : ClangBuiltin<"__builtin_sw_loadu_i">,
  Intrinsic<[llvm_v8i32_ty], [llvm_ptr_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_loadu_l : ClangBuiltin<"__builtin_sw_loadu_l">,
  Intrinsic<[llvm_v4i64_ty], [llvm_ptr_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_loadu_f : ClangBuiltin<"__builtin_sw_loadu_f">,
  Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_loadu_d : ClangBuiltin<"__builtin_sw_loadu_d">,
  Intrinsic<[llvm_v4f64_ty], [llvm_ptr_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
def int_sw64_load_u : ClangBuiltin<"__builtin_sw_load_u">,
  Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty, llvm_i32_ty],
  [IntrReadMem, IntrArgMemOnly]>;
}
