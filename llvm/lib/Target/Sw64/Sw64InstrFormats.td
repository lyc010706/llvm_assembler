//===- Sw64InstrFormats.td - Sw64 Instruction Formats ----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

def u5imm   : Operand<i64>{
  let DecoderMethod = "decodeUImmOperand<5>";
}
def u6imm   : Operand<i64>{
  let DecoderMethod = "decodeUImmOperand<6>";
}
def u8imm   : Operand<i64>{
  let DecoderMethod = "decodeUImmOperand<8>";
}
def u8immHex   : Operand<i64>{
  let DecoderMethod = "decodeUImmOperand<8>";
  let PrintMethod = "printHexImm";
}
def s8imm   : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<8>";
}
def s13imm  : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<13>";
}
def s12imm  : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<12>";
}
def s14imm  : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<14>";
}
def s16imm  : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<16>";
  let OperandType = "OPERAND_PCREL";
}
def s21imm  : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<21>";
  let OperandType = "OPERAND_PCREL";
}
def u26imm  : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<26>";
}
def s64imm  : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<64>";
  let PrintMethod = "printMemoryArg";
}
def u64imm  : Operand<i64>{
  let DecoderMethod = "decodeSImmOperand<64>";
}

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//
// Sw64 instruction baseline
class InstSw64<bits<6> op, string opstr, string operands> : Instruction {
  field bits<32> Inst;
  let Namespace = "Sw64";
  let Inst{31-26} = op;

  let AsmString = opstr # " " # operands;
  // Add Size: Number of bytes in encoding
  let Size = 4;
  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<32> SoftFail = 0;
}

//Chapter2.6.1
// sys_call  |31     26|25                 0|
//           |  Opcode |        Func        |
class PALForm<bits<6> opcode, dag iops, dag oops,
              string opstr, string operands>
    : InstSw64<opcode, opstr, operands> {
  let OutOperandList = oops;
  let InOperandList = iops;
  bits<26> disp;

  let Inst{25-0} = disp;
}

// Branching	beq/bge/bgt	Chapter2.6.2
// COND_BRANCH  |31     26|25      21|20                           0|
//              |  Opcode |   RA/Fa  |          disp                |

def JmpTargetAsmOperand : AsmOperandClass {
  let Name = "JmpImm";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isImm";
  let ParserMethod = "parseJmpImm";
}

def target : Operand<OtherVT> {
  let ParserMatchClass = JmpTargetAsmOperand;
	let EncoderMethod = "getBranchTargetOpValue";
  let DecoderMethod = "decodeSImmOperand<21>";
  let OperandType = "OPERAND_PCREL";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<22, 2>(Imm);
    return MCOp.isBareSymbolRef();
  }];
}

class BForm<bits<6> opcode, dag iops, dag oops,
            string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;
  bits<64> Opc; //dummy
  bits<5> RA;
  bits<21> disp;

  let Inst{25-21} = RA;
  let Inst{20-0} = disp;
}

// LDL/LDW	 Chapter2.6.3
// Memory  |31     26|25      21|20      16|15               0|
//         |  Opcode |   RA/Fa  |    RB    |        disp      |
class MForm<bits<6> opcode, dag iops, dag oops,
            string opstr, string operands="", list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RA;
  bits<16> DISP;
  bits<5> RB;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-0} = DISP;
}

class MfcForm<bits<6> opcode, bits<16> Func, dag iops, dag oops,
              string opstr, string operands="", list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  bits<16> Function=Func;
  bits<5> RA;
  bits<5> RB;

  let OutOperandList = oops;
  let InOperandList = iops;
  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-0} = Function;
}


// New Add, for atomic-op
// Memory + Func  |31     26|25      21|20      16|15    12|11           0|
//                |  Opcode |   RA/Fa  |    RB    |  Func  |    disp      |
class MFuncForm<bits<6> opcode, bits<4> func, dag iops, dag oops,
                string opstr, string operands="", list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let InOperandList = iops;
  let OutOperandList = oops;

  bits<5> RA;
  bits<12> disp;
  bits<5> RB;
  bits<4> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-12} = Function;
  let Inst{11-0} = disp;
}

// New Add, for privilege inst
// Memory + Func  |31     26|25      21|20      16|15    12|11           0|
//                |  Opcode |    TH    |    RB    |  Func  |    disp      |
class MPrvlForm<bits<6> opcode, bits<4> func, dag iops, dag oops,
                string opstr, string operands="", list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let InOperandList = iops;
  let OutOperandList = oops;

  bits<5> TH;
  bits<12> disp;
  bits<5> RB;
  bits<4> Function = func;

  let Inst{25-21} = TH;
  let Inst{20-16} = RB;
  let Inst{15-12} = Function;
  let Inst{11-0} = disp;
}

//	Chapter2.6.4
// simple_operation_form |31     26|25      21|20     16|15   13|12            5|4     0|
//              r + r :  |  Opcode |   RA     |  RB     |  SBZ  |      Func     |  RC   |
class OForm<bits<6> opcode, bits<8> fun, dag iops, dag oops,
            string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RC;
  bits<5> RA;
  bits<5> RB;
  bits<8> Function = fun;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-13} = 0;
  let Inst{12-5} = Function;
  let Inst{4-0} = RC;
}


// 	Chapter2.6.4
// simple_operation_form |31     26|25      21|20             13|12            5|4     0|
//              r + i :  |  Opcode |   RA     |        imm      |      Func     |   RC  |
class OFormL<bits<6> opcode, bits<8> fun, dag iops, dag oops,
            string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RC;
  bits<5> RA;
  bits<8> L;
  bits<8> Function = fun;

  let Inst{25-21} = RA;
  let Inst{20-13} = L;
  let Inst{12-5} = Function;
  let Inst{4-0} = RC;
}

//      Chapter2.6.4
// simple_operation_form |31     26|25                        13|12            5|4     0|
//              r + i :  |  Opcode |           imm              |      Func     |   RC  |
class OFormI<bits<6> opcode, bits<8> fun, dag iops, dag oops,
            string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RC;
  bits<13> L;
  bits<8> Function = fun;

  let Inst{25-13} = L;
  let Inst{12-5} = Function;
  let Inst{4-0} = RC;
}



// seleq/selne...		Chapter2.6.5(1)
// int_complex_operation_form |31     26|25      21|20     16|15   13|12  10|9    5|4    0|
//                   r + r :  |  Opcode |    RA    |    RB   |  SBZ  | Func |  RC  |  RD  |
class OForm4<bits<6> opcode, bits<3> fun, dag iops, dag oops,
             string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> RC;
  bits<3> Function = fun;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-13} = 0;
  let Inst{12-10} = Function;
  let Inst{9-5} = RC;
  let Inst{4-0} = RD;
}

// seleq/selne...		Chapter2.6.5(2)
// int_complex_operation_form |31     26|25      21|20             13|12  10|9    5|4    0|
//                   r + i :  |  Opcode |    RA    |        imm      | Func |  RC  |  RD  |
class OForm4L<bits<6> opcode, bits<3> fun, dag iops, dag oops,
              string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RD;
  bits<5> RA;
  bits<8> L;
  bits<3> Function = fun;
  bits<5> RC;

  let Inst{25-21} = RA;
  let Inst{20-13} = L;
  let Inst{12-10} = Function;
  let Inst{9-5} = RC;
  let Inst{4-0} = RD;
}

// fadds/faddd... Chapter2.6.4
// simple_operation_form |31     26|25      21|20     16|15   13|12            5|4     0|
//              r + r :  |  Opcode |      Fa  |     Fb  |  SBZ  |      Func     |   Fc  |
class FPForm<bits<6> opcode, bits<8> fun, dag iops, dag oops,
             string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let InOperandList = iops;
  let OutOperandList = oops;

  bits<5> RC;
  bits<5> RA;
  bits<5> RB;
  bits<8> Function = fun;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-13} = 0;
  let Inst{12-5} = Function;
  let Inst{4-0} = RC;
}

class FPForm1<bits<6> opcode, bits<8> fun, dag iops, dag oops,
             string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let InOperandList = iops;
  let OutOperandList = oops;

  bits<5> RC;
  bits<5> RA;
  bits<5> RB;
  bits<8> Function = fun;

  let Inst{25-21} = RA;
  let Inst{15-13} = 0;
  let Inst{12-5} = Function;
  let Inst{4-0} = RC;
}

// New add  	fselXX	Chapter2.6.5(3)
// fp_complex_operation_form  |31     26|25     21|20    16|15     10|9    5|4    0|
//                   r + r :  |  Opcode |    Fa   |    Fb  |   Func  |  Fc  |  Fd  |
class FForm4<bits<6> opcode, bits<6> func, dag iops, dag oops,
             string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RD;
  bits<5> RC;
  bits<5> RB;
  bits<5> RA;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = Function;
  let Inst{9-5} = RC;
  let Inst{4-0} = RD;
}

// New add  	fselXX	Chapter2.6.5(4)
// fp_complex_operation_form  |31     26|25     21|20    16|15     10|9    5|4    0|
//                   r + i :  |  Opcode |    Fa   |    Fb  |   Func  |  imm |  Fd  |
class FForm4L<bits<6> opcode, bits<6> func, dag iops, dag oops,
              string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;
  let Constraints = "$RFALSE = $RDEST";
  let DisableEncoding = "$RFALSE";

  bits<5> RA;
  bits<5> RB;
  bits<5> LIT;
  bits<5> RD;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = Function;
  let Inst{9-5} = LIT;
  let Inst{4-0} = RD;
}

// New add  	CSRXX	Chapter4.9.2
// fp_complex_operation_form  |31     26|25     21|20    16|15       8|7       0|
//                         :  |  Opcode |    Ra   |    Rb  |   Func   |  Index  |
class CSRForm<bits<6> opcode, bits<8> func, dag iops, dag oops,
             string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RB;
  bits<5> RA;
  bits<8> Function = func;
  bits<8> L;

  let Inst{25-21} = RA;
  let Inst{20-16} = 0x1f;
  let Inst{15-8} = Function;
  let Inst{7-0} = L;
}

// New add  FCVTSH	Chapter 4.6.3.3
// fp_complex_operation_form  |31     26|25     21|20    16|15     10|9    5|4    0|
//                   r + i :  |  Opcode |    Fa   |    Fb  |   Func  |  imm |  Fd  |
class FCForm4L<bits<6> opcode, bits<6> func, dag iops, dag oops,
              string opstr, string operands, list<dag> pattern=[]>
    : InstSw64<opcode, opstr, operands> {
  let Pattern = pattern;
  let OutOperandList = oops;
  let InOperandList = iops;

  bits<5> RA;
  bits<5> RB;
  bits<5> LIT;
  bits<5> RD;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = Function;
  let Inst{9-5} = LIT;
  let Inst{4-0} = RD;
}


// Pseudo instructions.
class PseudoInstSw64<dag oops, dag iops, string opstr="", list<dag> pattern>
    : InstSw64<0, opstr, "">  {
  let OutOperandList = oops;
  let InOperandList = iops;
  let Pattern = pattern;
  let isCodeGenOnly = 1;
}
