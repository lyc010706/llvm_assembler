//===- Sw64CallingConv.td - Calling Conventions for Sw64 -*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Sw64 architecture.
//===----------------------------------------------------------------------===//

/// CCIfSubtarget - Match if the current subtarget has a feature F.
class CCIfSubtarget<string F, CCAction A>
    : CCIf<!strconcat("static_cast<const Sw64Subtarget&>"
                       "(State.getMachineFunction().getSubtarget()).",
                     F), A>;

//===----------------------------------------------------------------------===//
// Sw64 Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_Sw64 : CallingConv<[
  // i64 is returned in register R0
  // R1 is an llvm extension, I don't know what gcc does
  CCIfType<[i64], CCAssignToReg<[R0, R1, R2, R3]>>,

  // f32 / f64 are returned in F0/F1
  CCIfType<[f32, f64], CCAssignToReg<[F0, F1]>>,

  CCIfType<[v32i8, v16i16, v8i32, v4i64, v4f32, v4f64],
       CCAssignToReg<[V0, V1]>>,

  CCIfSubtarget<"hasSIMD()",
       CCIfType<[v32i8, v16i16, v8i32, v4i64, v4f32, v4f64],
       CCAssignToReg<[F0, F1]>>>
]>;

// In soft-mode, register R16+R17, instead of R0+R1, is used to return a long
// double value.
def RetCC_F128Soft_Sw64 : CallingConv<[
  CCIfType<[i64], CCAssignToReg<[R16, R17]>>
]>;


//===----------------------------------------------------------------------===//
// Sw64 Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_Sw64 : CallingConv<[
  // The first 6 arguments are passed in registers, whether integer or
  // floating-point

  CCIfType<[i64], CCAssignToRegWithShadow<[R16, R17, R18, R19, R20, R21],
                                          [F16, F17, F18, F19, F20, F21]>>,

  CCIfType<[f32, f64], CCAssignToRegWithShadow<[F16, F17, F18, F19, F20, F21],
                                               [R16, R17, R18, R19, R20, R21]>>,

  CCIfType<[v32i8, v16i16, v8i32, v4i64, v4f64, v4f32],
            CCAssignToRegWithShadow<[V16, V17, V18, V19, V20, V21],
                                    [R16, R17, R18, R19, R20, R21]>>,

  // Stack slots are 8 bytes in size and 8-byte aligned.
  CCIfType<[i64, f32, f64], CCAssignToStack<8, 8>>,

  CCIfSubtarget<"hasSIMD()",
       CCIfType<[v32i8, v16i16, v8i32, v4i64, v4f32, v4f64],
       CCAssignToReg<[F16, F17, F18, F19, F20, F21]>>>
]>;

// CalleeSavedRegs
def CSR_I64 : CalleeSavedRegs<(add (sequence "R%u", 9, 14), R15, R26)>;

def CSR_F64 : CalleeSavedRegs<(add CSR_I64, (sequence "F%u", 2, 9))>;
