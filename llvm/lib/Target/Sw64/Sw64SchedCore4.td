//===- Sw64SchedCore4.td - Sw64 Scheduling Definitions -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

let SchedModel = GenericSw64Model in {
  // 3 pipeline ALU

def :  WriteRes<WriteCrc, [C3PortALU]> { let Latency = 1; }

// FIXME: the latency of div and rem
def :  WriteRes<WriteIDiv, [C3PortALU0]> { let Latency = 4; }
def :  WriteRes<WriteIRem, [C3PortALU0]> { let Latency = 4; }

def : WriteRes<WriteFCvtF64ToF64, [C3PortFPU]>{  let Latency = 2; }
def : WriteRes<WriteFCvtF32ToF32, [C3PortFPU]>{  let Latency = 2; }

def : WriteRes<WriteFREC, [C3PortFPU]>{  let Latency = 3; }

def : WriteRes<WriteFCvtF64ToI32, [C3PortFPU]>{  let Latency = 2; }
def : WriteRes<WriteFCvtI64ToF32, [C3PortFPU]>{  let Latency = 2; }
def : WriteRes<WriteFCvtI32ToF32, [C3PortFPU]>{  let Latency = 2; }
def : WriteRes<WriteFCvtI64ToF64, [C3PortFPU]>{  let Latency = 2; }
def : WriteRes<WriteFCvtI32ToF64, [C3PortFPU]>{  let Latency = 2; }

def : InstRW<[WriteLD], (instrs LDL_A, LDW_A, LDHU_A, LDBU_A)>;
def : InstRW<[WriteFLD], (instrs LDS_A, LDD_A)>;

def : InstRW<[WriteST], (instrs STL_A, STW_A, STH_A, STB_A)>;
def : InstRW<[WriteFST], (instrs STS_A, STD_A)>;

def : InstRW<[WriteIDiv], (instregex "^(DIV)(L|Q)$")>;
def : InstRW<[WriteIDiv], (instregex "^(UDIV)(L|Q)$")>;
def : InstRW<[WriteIRem], (instregex "^(REM)(L|Q)$")>;
def : InstRW<[WriteIRem], (instregex "^(UREM)(L|Q)$")>;
def : InstRW<[WriteJmp], (instrs ADDPI, ADDPIS)>;
def : InstRW<[WriteImm], (instregex "^(C|S)(BT)(r|i)$")>;

def : InstRW<[WriteIALU], (instrs REVBH, REVBW, REVBL)>;

def : InstRW<[WriteIALU], (instregex "^(SLLW|SRAW|SRLW|ROLW|ROLL)(r|i)*$")>;

def : InstRW<[WriteCrc], (instregex "^(CRC32C)(B|H|W|L)*$")>;
def : InstRW<[WriteCrc], (instregex "^(CRC32)(B|H|W|L)*$")>;

def : InstRW<[WriteFCvtF64ToI64], (instrs CMOVDL, CMOVDL_G, CMOVDL_P, CMOVDL_Z, CMOVDL_N)>;
def : InstRW<[WriteFCvtF64ToI64], (instrs CMOVDLU, CMOVDLU_G, CMOVDLU_P, CMOVDLU_Z, CMOVDLU_N)>;
def : InstRW<[WriteFCvtF64ToI32], (instrs CMOVDWU, CMOVDWU_G, CMOVDWU_P, CMOVDWU_Z, CMOVDWU_N)>;
def : InstRW<[WriteFCvtF64ToI32], (instrs CMOVDW, CMOVDW_G, CMOVDW_P, CMOVDW_Z, CMOVDW_N)>;
def : InstRW<[WriteFCvtF64ToI32], (instrs FCVTHS, FCVTSH)>;

def : InstRW<[WriteFCvtI64ToF32], (instrs CMOVLS, CMOVULS)>;
def : InstRW<[WriteFCvtI32ToF32], (instrs CMOVWS, CMOVUWS)>;
def : InstRW<[WriteFCvtI64ToF64], (instrs CMOVLD, CMOVULD)>;
def : InstRW<[WriteFCvtI32ToF64], (instrs CMOVWD, CMOVUWD)>;

def : InstRW<[WriteFCvtF64ToF64], (instrs FRID, FRID_G, FRID_P, FRID_Z, FRID_N)>;
def : InstRW<[WriteFCvtF32ToF32], (instrs FRIS, FRIS_G, FRIS_P, FRIS_Z, FRIS_N)>;

def : InstRW<[WriteFREC], (instrs FRECS)>;
def : InstRW<[WriteFREC], (instrs FRECD)>;

def : InstRW<[WriteST], (instrs CASW, CASL)>;

def : InstRW<[WriteLD], (instrs DPFHR, DPFHW)>;

def : InstRW<[WriteCSR], (instrs CSRR, CSRW)>;
def : InstRW<[WriteCSR], (instrs CSRWS, CSRWC)>;

def : InstRW<[WriteJmp], (instrs LBR)>;
}
