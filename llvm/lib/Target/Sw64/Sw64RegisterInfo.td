//===- Sw64RegisterInfo.td - The Sw64 Register File ------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the Sw64 register set.
//
//===----------------------------------------------------------------------===//
let Namespace = "Sw64" in {
def sub_32: SubRegIndex<32>;
} //Namespace Sw64

// For register encoding
class Sw64Reg<bits<16> Enc, string n, list<string> alt= []> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Sw64";
  let AltNames = alt;
}

class Sw64RegWithSubRegs<bits<16> Enc, string n, list<Register> subregs>
  : RegisterWithSubRegs<n, subregs> {
  let HWEncoding = Enc;
  let Namespace = "Sw64";
}

// GPR - One of the 32 32-bit general-purpose registers
class Sw64GPR<bits<16> Enc, string n, list<string> alt= []> : Sw64Reg<Enc, n, alt>;
// FPR - One of the 32 64-bit floating-point registers
class Sw64FPR<bits<16> Enc, string n, list<Register> subregs = []>
 : Sw64RegWithSubRegs<Enc, n, subregs>;

// VEC - One of the 32 256-bit vector registers
class Sw64VEC<bits<16> Enc, string n, list<Register> subregs>
  : Sw64RegWithSubRegs<Enc, n, subregs> {
  let SubRegIndices = [sub_32];
}

class Unallocatable {
  bit isAllocatable = 0;
}

let Namespace = "Sw64" in {

// General-purpose registers
def R0 : Sw64GPR< 0, "$0">, DwarfRegNum<[0]>;
def R1 : Sw64GPR< 1, "$1">, DwarfRegNum<[1]>;
def R2 : Sw64GPR< 2, "$2">, DwarfRegNum<[2]>;
def R3 : Sw64GPR< 3, "$3">, DwarfRegNum<[3]>;
def R4 : Sw64GPR< 4, "$4">, DwarfRegNum<[4]>;
def R5 : Sw64GPR< 5, "$5">, DwarfRegNum<[5]>;
def R6 : Sw64GPR< 6, "$6">, DwarfRegNum<[6]>;
def R7 : Sw64GPR< 7, "$7">, DwarfRegNum<[7]>;
def R8 : Sw64GPR< 8, "$8">, DwarfRegNum<[8]>;
def R9 : Sw64GPR< 9, "$9">, DwarfRegNum<[9]>;
def R10 : Sw64GPR< 10, "$10">, DwarfRegNum<[10]>;
def R11 : Sw64GPR< 11, "$11">, DwarfRegNum<[11]>;
def R12 : Sw64GPR< 12, "$12">, DwarfRegNum<[12]>;
def R13 : Sw64GPR< 13, "$13">, DwarfRegNum<[13]>;
def R14 : Sw64GPR< 14, "$14">, DwarfRegNum<[14]>;
def R15 : Sw64GPR< 15, "$15", ["$fp"]>, DwarfRegNum<[15]>;
def R16 : Sw64GPR< 16, "$16">, DwarfRegNum<[16]>;
def R17 : Sw64GPR< 17, "$17">, DwarfRegNum<[17]>;
def R18 : Sw64GPR< 18, "$18">, DwarfRegNum<[18]>;
def R19 : Sw64GPR< 19, "$19">, DwarfRegNum<[19]>;
def R20 : Sw64GPR< 20, "$20">, DwarfRegNum<[20]>;
def R21 : Sw64GPR< 21, "$21">, DwarfRegNum<[21]>;
def R22 : Sw64GPR< 22, "$22">, DwarfRegNum<[22]>;
def R23 : Sw64GPR< 23, "$23">, DwarfRegNum<[23]>;
def R24 : Sw64GPR< 24, "$24">, DwarfRegNum<[24]>;
def R25 : Sw64GPR< 25, "$25">, DwarfRegNum<[25]>;
def R26 : Sw64GPR< 26, "$26", ["$ra"]>, DwarfRegNum<[26]>;
def R27 : Sw64GPR< 27, "$27", ["$pv"]>, DwarfRegNum<[27]>;
def R28 : Sw64GPR< 28, "$28", ["$at"]>, DwarfRegNum<[28]>;
def R29 : Sw64GPR< 29, "$29", ["$gp"]>, DwarfRegNum<[29]>;
def R30 : Sw64GPR< 30, "$30", ["$sp"]>, DwarfRegNum<[30]>;
def R31 : Sw64GPR< 31, "$31", ["$zero"]>, DwarfRegNum<[31]>;

// Floating-point registers
def F0 : Sw64FPR< 0, "$f0">, DwarfRegNum<[32]>;
def F1 : Sw64FPR< 1, "$f1">, DwarfRegNum<[33]>;
def F2 : Sw64FPR< 2, "$f2">, DwarfRegNum<[34]>;
def F3 : Sw64FPR< 3, "$f3">, DwarfRegNum<[35]>;
def F4 : Sw64FPR< 4, "$f4">, DwarfRegNum<[36]>;
def F5 : Sw64FPR< 5, "$f5">, DwarfRegNum<[37]>;
def F6 : Sw64FPR< 6, "$f6">, DwarfRegNum<[38]>;
def F7 : Sw64FPR< 7, "$f7">, DwarfRegNum<[39]>;
def F8 : Sw64FPR< 8, "$f8">, DwarfRegNum<[40]>;
def F9 : Sw64FPR< 9, "$f9">, DwarfRegNum<[41]>;
def F10 : Sw64FPR< 10, "$f10">, DwarfRegNum<[42]>;
def F11 : Sw64FPR< 11, "$f11">, DwarfRegNum<[43]>;
def F12 : Sw64FPR< 12, "$f12">, DwarfRegNum<[44]>;
def F13 : Sw64FPR< 13, "$f13">, DwarfRegNum<[45]>;
def F14 : Sw64FPR< 14, "$f14">, DwarfRegNum<[46]>;
def F15 : Sw64FPR< 15, "$f15">, DwarfRegNum<[47]>;
def F16 : Sw64FPR< 16, "$f16">, DwarfRegNum<[48]>;
def F17 : Sw64FPR< 17, "$f17">, DwarfRegNum<[49]>;
def F18 : Sw64FPR< 18, "$f18">, DwarfRegNum<[50]>;
def F19 : Sw64FPR< 19, "$f19">, DwarfRegNum<[51]>;
def F20 : Sw64FPR< 20, "$f20">, DwarfRegNum<[52]>;
def F21 : Sw64FPR< 21, "$f21">, DwarfRegNum<[53]>;
def F22 : Sw64FPR< 22, "$f22">, DwarfRegNum<[54]>;
def F23 : Sw64FPR< 23, "$f23">, DwarfRegNum<[55]>;
def F24 : Sw64FPR< 24, "$f24">, DwarfRegNum<[56]>;
def F25 : Sw64FPR< 25, "$f25">, DwarfRegNum<[57]>;
def F26 : Sw64FPR< 26, "$f26">, DwarfRegNum<[58]>;
def F27 : Sw64FPR< 27, "$f27">, DwarfRegNum<[59]>;
def F28 : Sw64FPR< 28, "$f28">, DwarfRegNum<[60]>;
def F29 : Sw64FPR< 29, "$f29">, DwarfRegNum<[61]>;
def F30 : Sw64FPR< 30, "$f30">, DwarfRegNum<[62]>;
def F31 : Sw64FPR< 31, "$f31">, DwarfRegNum<[63]>;

// Floating-point registers
let SubRegIndices = [sub_32] in {
def Q0 : Sw64FPR< 0, "$f0", [F0]>, DwarfRegNum<[32]>;
def Q1 : Sw64FPR< 1, "$f1", [F1]>, DwarfRegNum<[33]>;
def Q2 : Sw64FPR< 2, "$f2", [F2]>, DwarfRegNum<[34]>;
def Q3 : Sw64FPR< 3, "$f3", [F3]>, DwarfRegNum<[35]>;
def Q4 : Sw64FPR< 4, "$f4", [F4]>, DwarfRegNum<[36]>;
def Q5 : Sw64FPR< 5, "$f5", [F5]>, DwarfRegNum<[37]>;
def Q6 : Sw64FPR< 6, "$f6", [F6]>, DwarfRegNum<[38]>;
def Q7 : Sw64FPR< 7, "$f7", [F7]>, DwarfRegNum<[39]>;
def Q8 : Sw64FPR< 8, "$f8", [F8]>, DwarfRegNum<[40]>;
def Q9 : Sw64FPR< 9, "$f9", [F9]>, DwarfRegNum<[41]>;
def Q10 : Sw64FPR< 10, "$f10", [F10]>, DwarfRegNum<[42]>;
def Q11 : Sw64FPR< 11, "$f11", [F11]>, DwarfRegNum<[43]>;
def Q12 : Sw64FPR< 12, "$f12", [F12]>, DwarfRegNum<[44]>;
def Q13 : Sw64FPR< 13, "$f13", [F13]>, DwarfRegNum<[45]>;
def Q14 : Sw64FPR< 14, "$f14", [F14]>, DwarfRegNum<[46]>;
def Q15 : Sw64FPR< 15, "$f15", [F15]>, DwarfRegNum<[47]>;
def Q16 : Sw64FPR< 16, "$f16", [F16]>, DwarfRegNum<[48]>;
def Q17 : Sw64FPR< 17, "$f17", [F17]>, DwarfRegNum<[49]>;
def Q18 : Sw64FPR< 18, "$f18", [F18]>, DwarfRegNum<[50]>;
def Q19 : Sw64FPR< 19, "$f19", [F19]>, DwarfRegNum<[51]>;
def Q20 : Sw64FPR< 20, "$f20", [F20]>, DwarfRegNum<[52]>;
def Q21 : Sw64FPR< 21, "$f21", [F21]>, DwarfRegNum<[53]>;
def Q22 : Sw64FPR< 22, "$f22", [F22]>, DwarfRegNum<[54]>;
def Q23 : Sw64FPR< 23, "$f23", [F23]>, DwarfRegNum<[55]>;
def Q24 : Sw64FPR< 24, "$f24", [F24]>, DwarfRegNum<[56]>;
def Q25 : Sw64FPR< 25, "$f25", [F25]>, DwarfRegNum<[57]>;
def Q26 : Sw64FPR< 26, "$f26", [F26]>, DwarfRegNum<[58]>;
def Q27 : Sw64FPR< 27, "$f27", [F27]>, DwarfRegNum<[59]>;
def Q28 : Sw64FPR< 28, "$f28", [F28]>, DwarfRegNum<[60]>;
def Q29 : Sw64FPR< 29, "$f29", [F29]>, DwarfRegNum<[61]>;
def Q30 : Sw64FPR< 30, "$f30", [F30]>, DwarfRegNum<[62]>;
def Q31 : Sw64FPR< 31, "$f31", [F31]>, DwarfRegNum<[63]>;
}

// Vector registers
def V0 : Sw64VEC< 0, "$f0", [Q0]>, DwarfRegNum<[32]>;
def V1 : Sw64VEC< 1, "$f1", [Q1]>, DwarfRegNum<[33]>;
def V2 : Sw64VEC< 2, "$f2", [Q2]>, DwarfRegNum<[34]>;
def V3 : Sw64VEC< 3, "$f3", [Q3]>, DwarfRegNum<[35]>;
def V4 : Sw64VEC< 4, "$f4", [Q4]>, DwarfRegNum<[36]>;
def V5 : Sw64VEC< 5, "$f5", [Q5]>, DwarfRegNum<[37]>;
def V6 : Sw64VEC< 6, "$f6", [Q6]>, DwarfRegNum<[38]>;
def V7 : Sw64VEC< 7, "$f7", [Q7]>, DwarfRegNum<[39]>;
def V8 : Sw64VEC< 8, "$f8", [Q8]>, DwarfRegNum<[40]>;
def V9 : Sw64VEC< 9, "$f9", [Q9]>, DwarfRegNum<[41]>;
def V10 : Sw64VEC< 10, "$f10", [Q10]>, DwarfRegNum<[42]>;
def V11 : Sw64VEC< 11, "$f11", [Q11]>, DwarfRegNum<[43]>;
def V12 : Sw64VEC< 12, "$f12", [Q12]>, DwarfRegNum<[44]>;
def V13 : Sw64VEC< 13, "$f13", [Q13]>, DwarfRegNum<[45]>;
def V14 : Sw64VEC< 14, "$f14", [Q14]>, DwarfRegNum<[46]>;
def V15 : Sw64VEC< 15, "$f15", [Q15]>, DwarfRegNum<[47]>;
def V16 : Sw64VEC< 16, "$f16", [Q16]>, DwarfRegNum<[48]>;
def V17 : Sw64VEC< 17, "$f17", [Q17]>, DwarfRegNum<[49]>;
def V18 : Sw64VEC< 18, "$f18", [Q18]>, DwarfRegNum<[50]>;
def V19 : Sw64VEC< 19, "$f19", [Q19]>, DwarfRegNum<[51]>;
def V20 : Sw64VEC< 20, "$f20", [Q20]>, DwarfRegNum<[52]>;
def V21 : Sw64VEC< 21, "$f21", [Q21]>, DwarfRegNum<[53]>;
def V22 : Sw64VEC< 22, "$f22", [Q22]>, DwarfRegNum<[54]>;
def V23 : Sw64VEC< 23, "$f23", [Q23]>, DwarfRegNum<[55]>;
def V24 : Sw64VEC< 24, "$f24", [Q24]>, DwarfRegNum<[56]>;
def V25 : Sw64VEC< 25, "$f25", [Q25]>, DwarfRegNum<[57]>;
def V26 : Sw64VEC< 26, "$f26", [Q26]>, DwarfRegNum<[58]>;
def V27 : Sw64VEC< 27, "$f27", [Q27]>, DwarfRegNum<[59]>;
def V28 : Sw64VEC< 28, "$f28", [Q28]>, DwarfRegNum<[60]>;
def V29 : Sw64VEC< 29, "$f29", [Q29]>, DwarfRegNum<[61]>;
def V30 : Sw64VEC< 30, "$f30", [Q30]>, DwarfRegNum<[62]>;
def V31 : Sw64VEC< 31, "$f31", [Q31]>, DwarfRegNum<[63]>;

} // Namespace Sw64

/// Register classes
def GPRC : RegisterClass<"Sw64", [i64], 64, (add
     // Volatile
     R0, R1, R2, R3, R4, R5, R6, R7, R8, R16, R17, R18, R19, R20, R21, R22,
     R23, R24, R25, R28,
     //Special meaning, but volatile
     R27, //procedure address
     R26, //return address
     R29, //global offset table address
     // Non-volatile
     R9, R10, R11, R12, R13, R14,
     // Don't allocate 15, 30, 31
     R15, R30, R31)>;

def F4RC : RegisterClass<"Sw64", [f32], 64, (add F0, F1,
        F10, F11, F12, F13, F14, F15, F16, F17, F18, F19,
        F20, F21, F22, F23, F24, F25, F26, F27, F28, F29, F30,
        // Saved:
        F2, F3, F4, F5, F6, F7, F8, F9,
        // zero:
        F31)>;

def F8RC : RegisterClass<"Sw64", [f64], 64, (add F4RC)>;

// lowest 64bits part for simd vector
def FPRC : RegisterClass<"Sw64", [i64, f64], 64, (sequence "Q%u", 0, 31)>;

def FPRC_lo : RegisterClass<"Sw64", [i32, f32], 64, (trunc F4RC, 32)>;

// Stack pointer and global pointer classes for instructions that are limited
// to a single register.
def SP64 : RegisterClass<"Sw64", [i64], 64, (add R30)>, Unallocatable;
def GP64 : RegisterClass<"Sw64", [i64], 64, (add R29)>, Unallocatable;

def FP30 : RegisterClass<"Sw64", [f32], 64, (add F30)>, Unallocatable;
def FD30 : RegisterClass<"Sw64", [f64], 64, (add F30)>, Unallocatable;
// Register Operands.

class Sw64AsmRegOperand : AsmOperandClass {
  let ParserMethod = "parseAnyRegister";
}

def GPRCAsmOperand : Sw64AsmRegOperand {
  let Name = "Reg";
}

def F4RCAsmOperand : Sw64AsmRegOperand {
  let Name = "F4RCAsmReg";
  let PredicateMethod = "isFGRAsmReg";
}

def F8RCAsmOperand : Sw64AsmRegOperand {
  let Name = "F8RCAsmReg";
  let PredicateMethod = "isFGRAsmReg";
}

def V256AsmOperand : Sw64AsmRegOperand {
  let Name = "V256AsmReg";
}

def V256B : RegisterClass<"Sw64", [v32i8], 256, (add (sequence "V%u", 0, 31))>;
def V256H : RegisterClass<"Sw64", [v16i16], 256, (add (sequence "V%u", 0, 31))>;
def V256W : RegisterClass<"Sw64", [v4f32], 256, (add (sequence "V%u", 0, 31))>;

def V256L : RegisterClass<"Sw64", [v32i8, v16i16, v8i32, v4i64, v4f64, v4f32], 256,
                                  (add (sequence "V%u", 0, 31))>;

def V256all : RegisterClass<"Sw64", [v32i8, v16i16, v8i32, v4i64, v4f32 ,v4f64],
                                  256, (add (sequence "V%u", 0, 31))>;

// adding a special class for floating selection
def V256Floating : RegisterClass<"Sw64", [v4f32, v4f64],
                            256, (add (sequence "V%u", 0, 31))>;
def V256E64 : RegisterClass<"Sw64", [v4i64, v4f32, v4f64],
                            256, (add (sequence "V%u", 0, 31))>;

def GPRCOpnd : RegisterOperand<GPRC> {
  let ParserMatchClass = GPRCAsmOperand;
}

def F4RCOpnd : RegisterOperand<F4RC> {
  let ParserMatchClass = F4RCAsmOperand;
}

def F8RCOpnd : RegisterOperand<F8RC> {
  let ParserMatchClass = F8RCAsmOperand;
}

def FPRCOpnd : RegisterOperand<FPRC> {
  let ParserMatchClass = F8RCAsmOperand;
}

def FPRCloOpnd : RegisterOperand<FPRC_lo> {
  let ParserMatchClass = F8RCAsmOperand;
}

def V256BOpnd : RegisterOperand<V256B> {
  let ParserMatchClass = V256AsmOperand;
}

def V256HOpnd : RegisterOperand<V256H> {
  let ParserMatchClass = V256AsmOperand;
}

def V256WOpnd : RegisterOperand<V256W> {
  let ParserMatchClass = V256AsmOperand;
}

def V256LOpnd : RegisterOperand<V256L> {
  let ParserMatchClass = V256AsmOperand;
}

def V256ALOpnd : RegisterOperand<V256all> {
  let ParserMatchClass = V256AsmOperand;
}

def V256FOpnd : RegisterOperand<V256Floating> {
  let ParserMatchClass = V256AsmOperand;
}
