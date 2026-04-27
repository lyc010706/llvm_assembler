//===- Sw64Schedule.td - Sw64 Scheduling Definitions -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Define scheduler resources associated with def operands.
def WriteIALU       : SchedWrite;    // 32 or 64-bit integer ALU operations
def WriteSHT        : SchedWrite;    // 32 or 64-bit integer ALU operations
def WriteIMul       : SchedWrite;    // 32-bit or 64-bit multiply
def WriteIDiv       : SchedWrite;    // 32-bit or 64-bit divided
def WriteIRem       : SchedWrite;    // 32-bit or 64-bit remainder
def WriteImm        : SchedWrite;    // 32-bit multiply on RV64I
def WriteCrc        : SchedWrite;
def WriteJmp        : SchedWrite;    // Jump
def WriteBR         : SchedWrite;    // Jump
def WriteFBR        : SchedWrite;    // float Jump
def WriteNop        : SchedWrite;
def WriteLD         : SchedWrite;    // Load double-word
def WriteFLD        : SchedWrite;    // Load double-word
def WriteFREC       : SchedWrite;
def WriteCSR        : SchedWrite;    // CSR instructions
def WriteST         : SchedWrite;    // Store byte
def WriteFST        : SchedWrite;    // Store byte
def WriteCNT        : SchedWrite;    //Atomic memory operation word size
def WriteSEL        : SchedWrite;    // bytes operate and selection operate
def WriteFPS        : SchedWrite;    //Atomic memory operation double word size
def WriteBOP        : SchedWrite;    //Atomic memory operation double word size
def WriteAtomicSTW  : SchedWrite;    // Atomic store word
def WriteAtomicSTD  : SchedWrite;    // Atomic store double word
def WriteFPU32      : SchedWrite;    // FP 32-bit computation
def WriteFPU64      : SchedWrite;    // FP 64-bit computation
def WriteFMul32     : SchedWrite;    // 32-bit floating point multiply
def WriteFMulAdd32  : SchedWrite;    // 32-bit floating point multiply add
def WriteFMulSub32  : SchedWrite;    // 32-bit floating point multiply sub
def WriteFMul64     : SchedWrite;    // 64-bit floating point multiply
def WriteFMulAdd64  : SchedWrite;      // 64-bit floating point multiply add
def WriteFMulSub64  : SchedWrite;    // 64-bit floating point multiply sub
def WriteFDiv32     : SchedWrite;    // 32-bit floating point divide
def WriteFDiv64     : SchedWrite;    // 64-bit floating point divide
def WriteFSqrt32    : SchedWrite;    // 32-bit floating point sqrt
def WriteFSqrt64    : SchedWrite;    // 64-bit floating point sqrt
def WriteFSEL       : SchedWrite;    // float selection operate
def WriteNOP       : SchedWrite;    // float selection operate
def WriteFCvtF32ToI32     : SchedWrite;
def WriteFCvtF32ToI64     : SchedWrite;
def WriteFCvtF64ToI32     : SchedWrite;
def WriteFCvtF64ToI64     : SchedWrite;
def WriteFCvtI32ToF32     : SchedWrite;
def WriteFCvtI32ToF64     : SchedWrite;
def WriteFCvtI64ToF32     : SchedWrite;
def WriteFCvtI64ToF64     : SchedWrite;
def WriteFMovF32ToI32     : SchedWrite;
def WriteFMovI32ToF32     : SchedWrite;
def WriteFMovF64ToI64     : SchedWrite;
def WriteFMovI64ToF64     : SchedWrite;
def WriteFCvtF32ToF64     : SchedWrite;
def WriteFCvtF64ToF32     : SchedWrite;
def WriteFCvtF64ToF64     : SchedWrite;
def WriteFCvtF32ToF32     : SchedWrite;

def WriteAdrLD : WriteSequence<[WriteImm, WriteLD]>;
def WriteAdrAdr : WriteSequence<[WriteImm, WriteImm]>;

def WriteFLDS : SchedWrite;
def WriteFSTDS : SchedWrite;
def WriteVEADD : SchedWrite;
def WriteVESHT2 : SchedWrite;
def WriteVESHT3 : SchedWrite;
def WriteVECNT2 : SchedWrite;
def WriteVECNT3 : SchedWrite;
def WriteVESEL : SchedWrite;
def WriteFMA2 : SchedWrite;
def WriteFMA6 : SchedWrite;
def WriteFMA17 : SchedWrite;
def WriteVPM1 : SchedWrite;
def WriteVPM2 : SchedWrite;
def WriteVCON1 : SchedWrite;
def WriteVCON2 : SchedWrite;
def WriteVSUM : SchedWrite;
def WriteVFREC : SchedWrite;
def WriteVFCT : SchedWrite;
def WriteVFRIS : SchedWrite;
