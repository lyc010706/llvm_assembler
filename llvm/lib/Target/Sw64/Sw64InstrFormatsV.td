//===- Sw64InstrFormats.td - Sw64 Instruction Formats ----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// SIMD Instruction format superclass
//===----------------------------------------------------------------------===//


class SIMDPseudo<dag outs, dag ins, list<dag> pattern>:
  PseudoInstSw64<outs, ins, "", pattern> {
}


class InstSw64V<bits<6> op> : Instruction {
  field bits<32> Inst;
  let Namespace = "Sw64";
  let Inst{31-26} = op;
  let Size = 4;
  field bits<32> SoftFail = 0;
}

class InstSw64VLog<bits<4> op> : Instruction {
  field bits<32> Inst;
  let Namespace = "Sw64";
  let Inst{31-28} = op;
  let Size = 4;
  field bits<32> SoftFail = 0;
}

// VLDD/VLDW       Chapter2.6.3
// Memory  |31     26|25      21|20      16|15               0|
//         |  Opcode |   RA/RA  |    RB    |        disp      |
class MFormV<bits<6> opcode> : InstSw64V<opcode> {

  bits<5> RA;
  bits<21> addr;

  let Inst{25-21} = RA;
  let Inst{20-16} = addr{20-16};
  let Inst{15-0} = addr{15-0};
}

// Memory + Func  |31     26|25      21|20      16|15    12|11           0|
//                |  Opcode |   RA/RA  |    RB    |  Func  |    disp      |
class MFuncFormV<bits<6> opcode, bits<4> func> : InstSw64V<opcode> {

  bits<5> RA;
  bits<21> addr;
  bits<4> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = addr{20-16};
  let Inst{15-12} = Function;
  let Inst{11-0} = addr{11-0};
}

// fadds/faddd... Chapter2.6.4
// simple_operation_form |31     26|25      21|20     16|15   13|12            5|4     0|
//              r + r :  |  Opcode |      RA  |     RB  |  SBZ  |      Func     |   RC  |
class FPFormV<bits<6> opcode, bits<8> fun> : InstSw64V<opcode> {

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

class FPFormV_2RV<bits<6> opcode, bits<8> fun> : InstSw64V<opcode> {

  bits<5> RC;
  bits<5> RA;
  bits<5> RB;
  bits<8> Function = fun;

  let Inst{25-21} = RA;
  let Inst{15-13} = 0;
  let Inst{12-5} = Function;
  let Inst{4-0} = RC;
}

class FPFormV_2RV1<bits<6> opcode, bits<8> fun> : InstSw64V<opcode> {

  bits<5> RC;
  bits<5> RA;
  bits<5> RB;
  bits<8> Function = fun;

  let Inst{20-16} = RB;
  let Inst{15-13} = 0;
  let Inst{12-5} = Function;
  let Inst{4-0} = RC;
}

class FPFormV_CT<bits<6> opcode, bits<8> fun> : InstSw64V<opcode> {

  bits<5> RC;
  bits<5> RA;
  bits<5> RB;
  bits<8> Function = fun;

  let Inst{25-21} = RA;
  let Inst{15-13} = 0;
  let Inst{12-5} = Function;
  let Inst{4-0} = RC;
}


//      Chapter2.6.4
// simple_operation_form |31     26|25      21|20             13|12            5|4     0|
//              r + i :  |  Opcode |   RA     |        imm      |      Func     |   RC  |
class FPFormIV<bits<6> opcode, bits<8> fun> : InstSw64V<opcode> {

  bits<5> RC;
  bits<5> RA;
  bits<8> Imm;
  bits<8> Function = fun;

  let Inst{25-21} = RA;
  let Inst{20-13} = Imm;
  let Inst{12-11} = Function{7-6};
  let Inst{10} = 1;
  let Inst{9-5} = Function{4-0};
  let Inst{4-0} = RC;
}


// New add      fselXX  Chapter2.6.5(3)
// fp_complex_operation_form  |31     26|25     21|20    16|15     10|9    5|4    0|
//                   r + r :  |  Opcode |    Va   |    Vb  |   Func  |  Vc  |  Vd  |
class FForm4V<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> RC;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = Function;
  let Inst{9-5} = RC;
  let Inst{4-0} = RD;
}

class FForm4VINSECTL<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> RC;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = Function;
  let Inst{9-5} = 31;
}

class FForm4VCPY<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> RC;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = 31;
  let Inst{15-10} = Function;
  let Inst{9-5} = 31;
  let Inst{4-0} = RD;
}

// vcpyw/vcpys
class FForm2V<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> RC;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = 0;
  let Inst{15-10} = Function;
  let Inst{9-5} = 0;
  let Inst{4-0} = RC;
}


// New add      fselXX  Chapter2.6.5(4)
// fp_complex_operation_form  |31     26|25     21|20    16|15     10|9    5|4    0|
//                   r + i :  |  Opcode |    Va   |    Vb  |   Func  |  imm |  Vd  |
class FForm4LV<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {

  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> Imm;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = Function;
  let Inst{9-5} = Imm;
  let Inst{4-0} = RD;
}

class FForm4LV1<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {

  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> Imm;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = 31;
  let Inst{15-10} = Function;
  let Inst{9-5} = Imm;
  let Inst{4-0} = RD;
}

class FForm4LV2<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {

  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> RC;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = Function;
  let Inst{9-5} = RC;
  let Inst{4-0} = RD;
}
// New add  vext Fix the RD to RC
// fp_complex_operation_form  |31     26|25     21|20    16|15     10|9    5|4    0|
//                   r + i :  |  Opcode |    Va   |    Vb  |   Func  |  imm |  Vd  |
class FForm4LVV<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {

  bits<5> RC;
  bits<5> RA;
  bits<5> RB;
  bits<5> Imm;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = 0;
  let Inst{15-10} = Function;
  let Inst{9-5} = Imm;
  let Inst{4-0} = RC;
}

// New add      vlogxx
// vlogxx:  |31     28|27      26|25    21|20    16|15      10|9    5|4    0|
//          |  Opcode | zz[7:6]  |   Va   |   Vb   |  zz[5:0] |  Vc  |  Vd  |
class FForm_VANDW<bits<6> opcode> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = 0;
  let Inst{9-5} = 31;
  let Inst{4-0} = RD;
}

class FForm_VBICW<bits<6> opcode> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = 0x30;
  let Inst{9-5} = 31;
  let Inst{4-0} = RD;
}

class FForm_VBISW<bits<6> opcode> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = 0x3c;
  let Inst{9-5} = 31;
  let Inst{4-0} = RD;
}

class FForm_VXORW<bits<6> opcode> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = 0x3c;
  let Inst{9-5} = 31;
  let Inst{4-0} = RD;
}

class FForm_VEQVW<bits<6> opcode> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = 0x03;
  let Inst{9-5} = 31;
  let Inst{4-0} = RD;
}

class FForm_VORNOTW<bits<6> opcode> : InstSw64V<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = 0x33;
  let Inst{9-5} = 31;
  let Inst{4-0} = RD;
}


class FForm4LVLogZZ<bits<4> opcode> : InstSw64VLog<opcode> {
  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> RC;
  bits<8> Imm;

  let Inst{27-26} = Imm{7-6};
  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = Imm{5-0};
  let Inst{9-5} = RC;
  let Inst{4-0} = RD;
}

class FForm4LVLog<bits<4> opcode, bits<8> zz> : InstSw64VLog<opcode> {
  bits<5> RC;
  bits<5> RA;
  bits<5> RB;

  let Inst{27-26} = zz{7-6};
  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15-10} = zz{5-0};
  let Inst{9-5} = 31;
  let Inst{4-0} = RC;
}

// fp_complex_operation_form  |31     26|25     21|20    16|15     10|9    5|4    0|
//                   r + i :  |  Opcode |    Va   |    Vb  |   Func  |  imm |  Vd  |
class FForm4_VSELi<bits<6> opcode, bits<6> func> : InstSw64V<opcode> {

  bits<5> RD;
  bits<5> RA;
  bits<5> RB;
  bits<5> Imm;
  bits<6> Function = func;

  let Inst{25-21} = RA;
  let Inst{20-16} = RB;
  let Inst{15} = 1;
  let Inst{14-10} = Function{4-0};
  let Inst{9-5} = Imm;
  let Inst{4-0} = RD;
}

class VectorIndex<ValueType ty, code pred> : Operand<ty>, ImmLeaf<ty, pred>;

def VectorIndexB : VectorIndex<i64, [{ return ((uint64_t)Imm) < 32; }]>;
def VectorIndexH : VectorIndex<i64, [{ return ((uint64_t)Imm) < 16; }]>;
def VectorIndexS : VectorIndex<i64, [{ return ((uint64_t)Imm) < 8; }]>;
def VectorIndexD : VectorIndex<i64, [{ return ((uint64_t)Imm) < 4; }]>;
