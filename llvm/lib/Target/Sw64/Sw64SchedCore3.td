//===- Sw64SchedCore3.td - Sw64 Scheduling Definitions -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Core3 processor architecture mannual
def GenericSw64Model : SchedMachineModel {
// Core 3 has 4-way decode and 7-way dispatch,
// in a cycle, can maxinum dispatch 3-way to ALU, 2-way to AGU, 2-way to FPU,
// so set the dispatch width to 4 is optitional.
  let IssueWidth        =   4;
  let LoadLatency       =   4; // Optimistic load latency
  let LoopMicroOpBufferSize = 16; //InsnQueue has 16 entry
  let MispredictPenalty =  13; // Fetch + Decode/Rename/Dispatch + Branch
  let CompleteModel = false;
  let MicroOpBufferSize =  72; // ROB size

  let PostRAScheduler = 1;
}

let SchedModel = GenericSw64Model in {
  // chapter 2.2.1
  // 3 pipeline ALU
  def C3PortALU0 : ProcResource<1>;
  def C3PortALU1 : ProcResource<1>;
  def C3PortALU2 : ProcResource<1>;

  // 2.2.1
  // alu0 has ADD MUL, alu1 has BR/CSR BOP/SHT
  // alu2 has CNT BOP/SHT ADD/SEL
  def C3PortALU : ProcResGroup<[C3PortALU0, C3PortALU1, C3PortALU2]> {
		let BufferSize = 32;
	}
  def C3PortALU01 : ProcResGroup<[C3PortALU0, C3PortALU1]>;
  def C3PortALU12 : ProcResGroup<[C3PortALU1, C3PortALU2]>;

  // 2 pipeline Alu Mem
  // 2.2.3
  // Core3a interger has two AGU Unit
  // 2 LSU Unit deel with all load/store
  def C3LSU  : ProcResource<2>;

  def C3PortAGU0 : ProcResource<1>;
  def C3PortAGU1 : ProcResource<1>;

  def C3PortAGU01 : ProcResGroup<[C3PortAGU0, C3PortAGU1]>;

  let Super = C3LSU in
  def C3Load :  ProcResource<2> {
    let BufferSize = 32;
  }

  def C3LoadQueue : LoadQueue<C3Load>;

  let Super = C3LSU in
  def C3Store : ProcResource<1> {
    let BufferSize = 16;
  }

  def C3StoreQueue : StoreQueue<C3Store>;

  // 2 pipeline FPU-SIMD
  def C3PortFPU0 : ProcResource<1>;
  def C3PortFPU1 : ProcResource<1>;


  def C3PortFPU : ProcResGroup<[C3PortFPU0, C3PortFPU1]>;

  def C3GprRF: RegisterFile<105, [GPRC], [1]>;

  def C3FpuRF: RegisterFile<95, [F4RC, F8RC], [1]>;

  def C3RCU : RetireControlUnit<72, 4>;

  class C3WriteRes<SchedWrite SchedRW, list<ProcResourceKind> ExePorts,
       int Lat, list<int> Res = [], int UOps = 1> :
          WriteRes<SchedRW, ExePorts> {
                  let Latency = Lat;
                  let ResourceCycles = Res;
                  let NumMicroOps = UOps;
          }

  class C3LSWriteRes<SchedWrite SchedRW, list<ProcResourceKind> ExePorts,
       int Lat, list<int> Res = [], int UOps = 1> :
          WriteRes<SchedRW, ExePorts> {
                  let Latency = !add(Lat, 1);
                  let ResourceCycles = !if(!empty(Res), [1, 1], !listconcat([1], Res));
                  let NumMicroOps = UOps;
          }

  def :  C3WriteRes<WriteJmp, [C3PortALU1], 1> ;
  def :  C3WriteRes<WriteBR, [C3PortALU1], 1> ;
  def :  C3WriteRes<WriteFBR, [C3PortFPU0], 6> ;
  def :  C3WriteRes<WriteImm, [C3PortALU], 1> ;
  def :  C3WriteRes<WriteSHT, [C3PortALU12], 1> ;
  def :  C3WriteRes<WriteIALU, [C3PortALU], 1> ;
  def :  C3WriteRes<WriteNOP, [C3PortALU], 0> ; //nop do not execute in backend
  def :  C3WriteRes<WriteIMul, [C3PortALU0], 4> ;
  def :  C3WriteRes<WriteCSR, [C3PortALU1], 3> ;
  def :  C3WriteRes<WriteCNT, [C3PortALU2], 1> ;
  def :  C3WriteRes<WriteSEL, [C3PortALU2], 1> ;
  def :  C3WriteRes<WriteBOP, [C3PortALU12], 1> ;

  def : C3WriteRes<WriteFPU32, [C3PortFPU], 6> ;
  def : C3WriteRes<WriteFPU64, [C3PortFPU], 6> ;

  def : C3WriteRes<WriteFSEL, [C3PortFPU1], 2, []> ;

  def : C3WriteRes<WriteFPS, [C3Load], 4> ;

  def : C3LSWriteRes<WriteAtomicSTW, [C3PortAGU01, C3Store], 4> ;

  def : C3WriteRes<WriteFDiv32, [C3PortFPU1], 19, [19]> ;

  def : C3WriteRes<WriteFDiv64, [C3PortFPU1], 34, [34]> ;

  def : C3WriteRes<WriteFSqrt32, [C3PortFPU1], 19, [19]> ;

  def : C3WriteRes<WriteFSqrt64, [C3PortFPU1], 33, [33]> ;

  def : C3LSWriteRes<WriteST, [C3PortAGU01, C3Store], 4>;
  def : C3LSWriteRes<WriteLD, [C3PortAGU01, C3Load], 4>;

  def : C3LSWriteRes<WriteFST, [C3PortAGU01, C3Store], 4>;
  def : C3LSWriteRes<WriteFLD, [C3PortAGU01, C3Load], 4>;

  def : C3WriteRes<WriteFCvtF32ToI64, [C3PortFPU0], 4>;
  def : C3WriteRes<WriteFCvtF64ToI64, [C3PortFPU0], 4>;

  def : C3WriteRes<WriteFMovF64ToI64, [C3PortFPU], 4>;
  def : C3WriteRes<WriteFMovI64ToF64, [C3PortFPU], 4>;

  def : C3WriteRes<WriteFCvtF64ToF32, [C3PortFPU0], 4>;
  def : C3WriteRes<WriteFCvtF32ToF64, [C3PortFPU0], 4>;

  def : InstRW<[WriteIALU], (instrs COPY)>;

  def : InstRW<[WriteBR], (instrs BR, BEQ, BGE,
                           BGT, BLBC, BLBS, BLE, BLT, BNE, BSR)>;

  def : InstRW<[WriteBR], (instrs SYS_CALL)>;
  def : InstRW<[WriteBR], (instrs JMP, JSR, RET)>;
  def : InstRW<[WriteFBR], (instregex "^FB(EQ|GE|GT|LE|LT|NE)$")>;

  def : InstRW<[WriteLD], (instregex "^(S_FILL|E_FILL)(CS|DE)$")>;
  def : InstRW<[WriteLD], (instregex "^FILL(CS|DE|CS_E|DE_E)$")>;

  def : InstRW<[WriteLD], (instregex "^LD(L|W|HU|BU)$")>;
  def : InstRW<[WriteFLD], (instregex "^LD(S|D)$")>;

  def : InstRW<[WriteST], (instregex "^ST(L|W|H|B)$")>;
  def : InstRW<[WriteFST], (instregex "^ST(S|D)$")>;

  def : InstRW<[WriteImm], (instregex "^LDAH*$")>;

  def : InstRW<[WriteIALU], (instregex "^(ADD|SUB|S(4|8)(ADD|SUB))(L|Q)(r|i)$")>;
  def : InstRW<[WriteIMul], (instregex "^(MUL)(L|Q)(r|i)$")>;

  def : InstRW<[WriteCNT], (instrs CTLZ, CTPOP, CTTZ)>;

  def : InstRW<[WriteBOP], (instrs ZAPr, ZAPi, ZAPNOTr, ZAPNOTi, SEXTB, SEXTH)>;

  def : InstRW<[WriteIALU], (instregex "^CMP(EQ|LE|LT|ULE|ULT|BGE)(r|i)*$")>;
  def : InstRW<[WriteFPU64], (instregex "^CMP(TEQ|TLE|TLT|TUN)$")>;

  def : InstRW<[WriteIALU], (instregex "^(AND|BIC|BIS|ORNOT|XOR|EQV)(r|i)*$")>;

  def : InstRW<[WriteSHT], (instregex "^(SL|SRA|SRL)(r|i)*$")>;
  def : InstRW<[WriteIMul], (instrs UMULHi, UMULHr)>;

  def : InstRW<[WriteSEL], (instregex "^SEL(EQ|NE|LE|LT|GT|GE|LBC|LBS)(r|i)*$")>;

  def : InstRW<[WriteBOP], (instregex "^EXT(BL|WL|LL|LW|HB|HH|HW|HL)(r|i)*$")>;

  def : InstRW<[WriteBOP], (instregex "^MASKL[BHLW](r|i)*$")>;
  def : InstRW<[WriteBOP], (instregex "^MASKH[BHLW](r|i)*$")>;
  def : InstRW<[WriteBOP], (instregex "^INSL[BHLW](r|i)*$")>;
  def : InstRW<[WriteBOP], (instregex "^INSH[BHLW](r|i)*$")>;

  def : InstRW<[WriteFPU32], (instregex "^(ADD|SUB|MUL)(S|D)*$")>;
  def : InstRW<[WriteFPU32], (instregex "^CPY(S|SE|SN)(S|D)*$")>;
  def : InstRW<[WriteFPU64], (instregex "^SETFPEC(0|1|2|3)*$")>;
  def : InstRW<[WriteImm], (instrs NOP)>;

  def : InstRW<[WriteFCvtF64ToF32], (instrs FCVTLW, FCVTWL)>;
  def : InstRW<[WriteFCvtF64ToI64], (instrs CVTQS, CVTQT)>;
  def : InstRW<[WriteFCvtF64ToI64], (instrs CVTTQ, FCTTDL, FCTTDL_G, FCTTDL_P, FCTTDL_N)>;
  def : InstRW<[WriteFCvtF64ToF32], (instrs CVTST, CVTTS)>;

  def : InstRW<[WriteFPU32], (instregex "^(F|FN)M(A|S)S$")>;
  def : InstRW<[WriteFPU64], (instregex "^(F|FN)M(A|S)D$")>;

  def : InstRW<[WriteFSEL], (instregex "^FSEL(EQ|GE|GT|LE|LT|NE)S$")>;
  def : InstRW<[WriteFSEL], (instregex "^FSEL(EQ|GE|GT|LE|LT|NE)D$")>;

  def : InstRW<[WriteFSqrt32], (instrs SQRTSS)>;
  def : InstRW<[WriteFSqrt64], (instrs SQRTSD)>;

  def : InstRW<[WriteFDiv32], (instrs DIVS)>;
  def : InstRW<[WriteFDiv64], (instrs DIVD)>;

  def : InstRW<[WriteFPS], (instrs FTOIS, FTOIT, ITOFS, ITOFT)>;

  def : InstRW<[WriteLD], (instrs LDL_L, LDQ_L)>;
  def : InstRW<[WriteST], (instrs STL_C, STQ_C)>;

  def : InstRW<[WriteIALU], (instrs RCID, RPCC)>;
  def : InstRW<[WriteFPS], (instrs WFPCR, RFPCR)>;
}
