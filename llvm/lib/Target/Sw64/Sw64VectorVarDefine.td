//===- Sw64InstrInfo.td - The Sw64 Instruction Set -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Sw64 Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

class ConstantSImmAsmOperandClass<int Bits, list<AsmOperandClass> Supers = [],
                                  int Offset = 0> : AsmOperandClass {
  let Name = "ConstantSImm" # Bits # "_" # Offset;
  let RenderMethod = "addConstantSImmOperands<" # Bits # ", " # Offset # ">";
  let PredicateMethod = "isConstantSImm<" # Bits # ", " # Offset # ">";
  let SuperClasses = Supers;
  let DiagnosticType = "SImm" # Bits # "_" # Offset;
}

class ConstantUImmAsmOperandClass<int Bits, list<AsmOperandClass> Supers = [],
                                  int Offset = 0> : AsmOperandClass {
  let Name = "ConstantUImm" # Bits # "_" # Offset;
  let RenderMethod = "addConstantUImmOperands<" # Bits # ", " # Offset # ">";
  let PredicateMethod = "isConstantUImm<" # Bits # ", " # Offset # ">";
  let SuperClasses = Supers;
  let DiagnosticType = "UImm" # Bits # "_" # Offset;
}

def ConstantUImm7Lsl2AsmOperandClass : AsmOperandClass {
  let Name = "UImm7Lsl2";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledSImm<7, 2>";
}

def ConstantSImm8AsmOperandClass
    : ConstantSImmAsmOperandClass<8, [ConstantUImm7Lsl2AsmOperandClass]>;

def ConstantUImm8AsmOperandClass
    : ConstantUImmAsmOperandClass<8, [ConstantUImm7Lsl2AsmOperandClass]>;

foreach I = {8} in
  def vsplat_simm # I : Operand<vAny> {
    let ParserMatchClass =
        !cast<AsmOperandClass>("ConstantSImm" # I # "AsmOperandClass");
  }

foreach I = {8} in
  def vsplat_uimm # I : Operand<vAny> {
    let ParserMatchClass =
        !cast<AsmOperandClass>("ConstantUImm" # I # "AsmOperandClass");
  }

// Generic case - only to support certain assembly pseudo instructions.
class UImmAnyAsmOperandClass<int Bits, list<AsmOperandClass> Supers = []>
    : AsmOperandClass {
  let Name = "ImmAny";
  let RenderMethod = "addConstantUImmOperands<32>";
  let PredicateMethod = "isSImm<" # Bits # ">";
  let SuperClasses = Supers;
  let DiagnosticType = "ImmAny";
}

class SImmAsmOperandClass<int Bits, list<AsmOperandClass> Supers = []>
    : AsmOperandClass {
  let Name = "SImm" # Bits;
  let RenderMethod = "addSImmOperands<" # Bits # ">";
  let PredicateMethod = "isSImm<" # Bits # ">";
  let SuperClasses = Supers;
  let DiagnosticType = "SImm" # Bits;
}

class UImmAsmOperandClass<int Bits, list<AsmOperandClass> Supers = []>
    : AsmOperandClass {
  let Name = "UImm" # Bits;
  let RenderMethod = "addUImmOperands<" # Bits # ">";
  let PredicateMethod = "isUImm<" # Bits # ">";
  let SuperClasses = Supers;
  let DiagnosticType = "UImm" # Bits;
}

def UImm32CoercedAsmOperandClass : UImmAnyAsmOperandClass<33, []> {
  let Name = "UImm32_Coerced";
  let DiagnosticType = "UImm32_Coerced";
}

def SImm32RelaxedAsmOperandClass
    : SImmAsmOperandClass<32, [UImm32CoercedAsmOperandClass]> {
  let Name = "SImm32_Relaxed";
  let PredicateMethod = "isAnyImm<33>";
  let DiagnosticType = "SImm32_Relaxed";
}

def SImm32AsmOperandClass
    : SImmAsmOperandClass<32, [SImm32RelaxedAsmOperandClass]>;
def ConstantUImm26AsmOperandClass
    : ConstantUImmAsmOperandClass<26, [SImm32AsmOperandClass]>;
def ConstantUImm20AsmOperandClass
    : ConstantUImmAsmOperandClass<20, [ConstantUImm26AsmOperandClass]>;

def UImm16RelaxedAsmOperandClass
    : UImmAsmOperandClass<16, [ConstantUImm20AsmOperandClass]> {
  let Name = "UImm16_Relaxed";
  let PredicateMethod = "isAnyImm<16>";
  let DiagnosticType = "UImm16_Relaxed";
}

// FIXME: One of these should probably have UImm16AsmOperandClass as the
//        superclass instead of UImm16RelaxedasmOPerandClass.
def UImm16AsmOperandClass
    : UImmAsmOperandClass<16, [UImm16RelaxedAsmOperandClass]>;
def SImm16RelaxedAsmOperandClass
    : SImmAsmOperandClass<16, [UImm16RelaxedAsmOperandClass]> {
  let Name = "SImm16_Relaxed";
  let PredicateMethod = "isAnyImm<16>";
  let DiagnosticType = "SImm16_Relaxed";
}

def SImm16AsmOperandClass
    : SImmAsmOperandClass<16, [SImm16RelaxedAsmOperandClass]>;

def ConstantSImm10Lsl3AsmOperandClass : AsmOperandClass {
  let Name = "SImm10Lsl3";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledSImm<10, 3>";
  let SuperClasses = [SImm16AsmOperandClass];
  let DiagnosticType = "SImm10_Lsl3";
}

def Sw64MemAsmOperand : AsmOperandClass {
  let Name = "Mem";
  let ParserMethod = "parseMemOperand";
}

foreach I = {16, 32} in
  def simm # I : Operand<i32> {
    let DecoderMethod = "DecodeSImmWithOffsetAndScale<" # I # ">";
    let ParserMatchClass = !cast<AsmOperandClass>("SImm" # I # "AsmOperandClass");
  }

foreach I = {1, 2, 3} in
  def Sw64MemSimm16Lsl # I # AsmOperand : AsmOperandClass {
    let Name = "MemOffsetSimm16_" # I;
    let SuperClasses = [Sw64MemAsmOperand];
    let RenderMethod = "addMemOperands";
    let ParserMethod = "parseMemOperand";
    let PredicateMethod = "isMemWithSimmOffset<10, " # I # ">";
    let DiagnosticType = "MemSImm10Lsl" # I;
  }

class mem_generic : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops ptr_rc, simm16);
  let EncoderMethod = "getMemEncoding";
  let ParserMatchClass = Sw64MemAsmOperand;
  let OperandType = "OPERAND_MEMORY";
}

def ConstantSImm10Lsl2AsmOperandClass : AsmOperandClass {
  let Name = "SImm10Lsl2";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledSImm<10, 2>";
  let SuperClasses = [ConstantSImm10Lsl3AsmOperandClass];
  let DiagnosticType = "SImm10_Lsl2";
}

foreach I = {2, 3} in
  def simm16_ # I : Operand<i32> {
    let DecoderMethod = "DecodeSImmWithOffsetAndScale<10, " # I # ">";
    let ParserMatchClass =
        !cast<AsmOperandClass>("ConstantSImm10Lsl" # I # "AsmOperandClass");
  }

  def mem_simm16 : mem_generic {
    let MIOperandInfo = (ops ptr_rc, !cast<Operand>("simm16_2"));
    let EncoderMethod = "getMemEncoding<2>";
    let ParserMatchClass =
            !cast<AsmOperandClass>("Sw64MemSimm16Lsl2AsmOperand");
  }

  def mem_simm12 : mem_generic {
    let MIOperandInfo = (ops ptr_rc, !cast<Operand>("simm16_3"));
    let EncoderMethod = "getMemEncoding<3>";
    let ParserMatchClass =
            !cast<AsmOperandClass>("Sw64MemSimm16Lsl3AsmOperand");
  }

class ConstantUImmRangeAsmOperandClass<int Bottom, int Top,
                                       list<AsmOperandClass> Supers = []>
    : AsmOperandClass {
  let Name = "ConstantUImmRange" # Bottom # "_" # Top;
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isConstantUImmRange<" # Bottom # ", " # Top # ">";
  let SuperClasses = Supers;
  let DiagnosticType = "UImmRange" # Bottom # "_" # Top;
}

def ConstantSImm19Lsl2AsmOperandClass : AsmOperandClass {
  let Name = "SImm19Lsl2";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledSImm<19, 2>";
  let SuperClasses = [ConstantUImm20AsmOperandClass];
  let DiagnosticType = "SImm19_Lsl2";
}

def ConstantSImm11AsmOperandClass
    : ConstantSImmAsmOperandClass<11, [ConstantSImm10Lsl2AsmOperandClass]>;
def ConstantSImm10Lsl1AsmOperandClass : AsmOperandClass {
  let Name = "SImm10Lsl1";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledSImm<10, 1>";
  let SuperClasses = [ConstantSImm11AsmOperandClass];
  let DiagnosticType = "SImm10_Lsl1";
}

def ConstantUImm10AsmOperandClass
    : ConstantUImmAsmOperandClass<10, [ConstantSImm10Lsl1AsmOperandClass]>;
def ConstantSImm10AsmOperandClass
    : ConstantSImmAsmOperandClass<10, [ConstantUImm10AsmOperandClass]>;
def ConstantSImm9AsmOperandClass
    : ConstantSImmAsmOperandClass<9, [ConstantSImm10AsmOperandClass]>;
def ConstantSImm7Lsl2AsmOperandClass : AsmOperandClass {
  let Name = "SImm7Lsl2";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledSImm<7, 2>";
  let SuperClasses = [ConstantSImm9AsmOperandClass];
  let DiagnosticType = "SImm7_Lsl2";
}

def ConstantUImm7Sub1AsmOperandClass
    : ConstantUImmAsmOperandClass<7, [ConstantUImm8AsmOperandClass], -1> {
  // Specify the names since the -1 offset causes invalid identifiers otherwise.
  let Name = "UImm7_N1";
  let DiagnosticType = "UImm7_N1";
}
def ConstantUImm7AsmOperandClass
    : ConstantUImmAsmOperandClass<7, [ConstantUImm7Sub1AsmOperandClass]>;
def ConstantUImm6Lsl2AsmOperandClass : AsmOperandClass {
  let Name = "UImm6Lsl2";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledUImm<6, 2>";
  let SuperClasses = [ConstantUImm7AsmOperandClass];
  let DiagnosticType = "UImm6_Lsl2";
}

def ConstantUImm6AsmOperandClass
    : ConstantUImmAsmOperandClass<6, [ConstantUImm6Lsl2AsmOperandClass]>;
def ConstantSImm6AsmOperandClass
    : ConstantSImmAsmOperandClass<6, [ConstantUImm6AsmOperandClass]>;

def ConstantUImm5Lsl2AsmOperandClass : AsmOperandClass {
  let Name = "UImm5Lsl2";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledUImm<5, 2>";
  let SuperClasses = [ConstantSImm6AsmOperandClass];
  let DiagnosticType = "UImm5_Lsl2";
}
def ConstantUImm5_Range2_64AsmOperandClass
    : ConstantUImmRangeAsmOperandClass<2, 64, [ConstantUImm5Lsl2AsmOperandClass]>;
def ConstantUImm5Plus33AsmOperandClass
    : ConstantUImmAsmOperandClass<5, [ConstantUImm5_Range2_64AsmOperandClass],
                                  33>;
def ConstantUImm5ReportUImm6AsmOperandClass
    : ConstantUImmAsmOperandClass<5, [ConstantUImm5Plus33AsmOperandClass]> {
  let Name = "ConstantUImm5_0_Report_UImm6";
  let DiagnosticType = "UImm5_0_Report_UImm6";
}
def ConstantUImm5Plus32AsmOperandClass
    : ConstantUImmAsmOperandClass<
          5, [ConstantUImm5ReportUImm6AsmOperandClass], 32>;
def ConstantUImm5Plus32NormalizeAsmOperandClass
    : ConstantUImmAsmOperandClass<5, [ConstantUImm5Plus32AsmOperandClass], 32> {
  let Name = "ConstantUImm5_32_Norm";
  // We must also subtract 32 when we render the operand.
  let RenderMethod = "addConstantUImmOperands<5, 32, -32>";
}

def ConstantUImm5Plus1ReportUImm6AsmOperandClass
    : ConstantUImmAsmOperandClass<
          5, [ConstantUImm5Plus32NormalizeAsmOperandClass], 1>{
  let Name = "ConstantUImm5_Plus1_Report_UImm6";
}

def ConstantUImm5Plus1AsmOperandClass
    : ConstantUImmAsmOperandClass<
          5, [ConstantUImm5Plus1ReportUImm6AsmOperandClass], 1>;
def ConstantUImm5AsmOperandClass
    : ConstantUImmAsmOperandClass<5, [ConstantUImm5Plus1AsmOperandClass]>;
def ConstantSImm5AsmOperandClass
    : ConstantSImmAsmOperandClass<5, [ConstantUImm5AsmOperandClass]>;
def ConstantUImm4AsmOperandClass
    : ConstantUImmAsmOperandClass<4, [ConstantSImm5AsmOperandClass]>;
def ConstantSImm4AsmOperandClass
    : ConstantSImmAsmOperandClass<4, [ConstantUImm4AsmOperandClass]>;
def ConstantUImm3AsmOperandClass
    : ConstantUImmAsmOperandClass<3, [ConstantSImm4AsmOperandClass]>;
def ConstantUImm2Plus1AsmOperandClass
    : ConstantUImmAsmOperandClass<2, [ConstantUImm3AsmOperandClass], 1>;
def ConstantUImm2AsmOperandClass
    : ConstantUImmAsmOperandClass<2, [ConstantUImm3AsmOperandClass]>;
def ConstantUImm1AsmOperandClass
    : ConstantUImmAsmOperandClass<1, [ConstantUImm2AsmOperandClass]>;

// Unsigned Operands
foreach I = {1, 2, 3, 4, 5, 6, 7, 8, 10, 20, 26} in
  def uimm # I : Operand<i32> {
    let PrintMethod = "printUImm<" # I # ">";
    let ParserMatchClass =
        !cast<AsmOperandClass>("ConstantUImm" # I # "AsmOperandClass");
  }

foreach I = {1, 2, 3, 4} in
  def uimm # I # _ptr : Operand<iPTR> {
    let PrintMethod = "printUImm<" # I # ">";
    let ParserMatchClass =
        !cast<AsmOperandClass>("ConstantUImm" # I # "AsmOperandClass");
  }
