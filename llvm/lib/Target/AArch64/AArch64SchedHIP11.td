//=- AArch64SchedHIP11.td - Huawei HIP11 Scheduling Defs -*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the machine model for Huawei HIP11 to support instruction
// scheduling and other instruction cost heuristics.
//
//===----------------------------------------------------------------------===//

def HIP11Model : SchedMachineModel {
  let IssueWidth            =   4; // HIP11 can dispatch 4 micro-ops per cycle.
  let MicroOpBufferSize     =  88; // Based on the reorder buffer.
  let LoadLatency           =   4; // Basic latency for most load instructions.
  let MispredictPenalty     =  14; // Based on ALU pipeline depth.
  let LoopMicroOpBufferSize =  40; // Based on the instruction queue size.
  let CompleteModel         =   1;

  list<Predicate> UnsupportedFeatures = !listconcat(PAUnsupported.F,
                                                    SME2Unsupported.F,
                                                    SVE2p1Unsupported.F,
                                                    [HasMTE, HasSMEI16I64]);
}

let SchedModel = HIP11Model in {

// HIP11 has 9 pipelines. There are three ALUs in total, of which two can
// also handle branches. The two Advanced SIMD&FP units handle different
// sets of operations. FSTD unit is used when processing vector load/store
// instructions.

def HIP11UnitALU   : ProcResource<1>;
def HIP11UnitBRU1  : ProcResource<1>;  // Alias ALU2
def HIP11UnitBRU2  : ProcResource<1>;  // Alias ALU3
def HIP11UnitMDU   : ProcResource<1>;  // Alias ALU4
def HIP11UnitFSU1  : ProcResource<1>;
def HIP11UnitFSU2  : ProcResource<1>;
def HIP11UnitFSTD  : ProcResource<1>;  // Fp Store Data
def HIP11UnitLd0St : ProcResource<1>;
def HIP11UnitLd1   : ProcResource<1>;

def HIP11UnitAnyALU : ProcResGroup<[HIP11UnitALU, HIP11UnitBRU1, HIP11UnitBRU2]>;
def HIP11UnitAnyBRU : ProcResGroup<[HIP11UnitBRU1, HIP11UnitBRU2]>;
def HIP11UnitAnyFSU : ProcResGroup<[HIP11UnitFSU1, HIP11UnitFSU2]>;
def HIP11UnitAnyLdSt : ProcResGroup<[HIP11UnitLd0St, HIP11UnitLd1]>;  // Any load/store

//===----------------------------------------------------------------------===//
// Define customized scheduler read/write types specific to the HIP11

//===----------------------------------------------------------------------===//
// Define Generic 1 micro-op types

def HIP11Write_1cyc_1AnyALU : SchedWriteRes<[HIP11UnitAnyALU]> { let Latency = 1; }
def HIP11Write_1cyc_1ALU : SchedWriteRes<[HIP11UnitALU]> { let Latency = 1; }

def HIP11Write_1cyc_1BRU    : SchedWriteRes<[HIP11UnitAnyBRU]> { let Latency = 1; }

def HIP11Write_2cyc_1MDU : SchedWriteRes<[HIP11UnitMDU]> { let Latency = 2; }
def HIP11Write_3cyc_1MDU : SchedWriteRes<[HIP11UnitMDU]> { let Latency = 3; }
def HIP11Write_4cyc_1MDU : SchedWriteRes<[HIP11UnitMDU]> { let Latency = 4; }
def HIP11Write_5cyc_1MDU : SchedWriteRes<[HIP11UnitMDU]> { let Latency = 5; }
def HIP11Write_12cyc_1MDU_RC : SchedWriteRes<[HIP11UnitMDU]> { let Latency = 12;
                                                                       let ResourceCycles = [12]; }
def HIP11Write_20cyc_1MDU_RC : SchedWriteRes<[HIP11UnitMDU]> { let Latency = 20;
                                                                       let ResourceCycles = [20]; }

def HIP11Write_1cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 1; }
def HIP11Write_1cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 1; }
def HIP11Write_2cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 2; }
def HIP11Write_2cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 2; }
def HIP11Write_3cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 3; }
def HIP11Write_3cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 3; }
def HIP11Write_4cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 4; }
def HIP11Write_4cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 4; }
def HIP11Write_4cyc_1FSU2 : SchedWriteRes<[HIP11UnitFSU2]> { let Latency = 4; }
def HIP11Write_6cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 6; }
def HIP11Write_6cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 6; }
def HIP11Write_6cyc_1FSU2 : SchedWriteRes<[HIP11UnitFSU2]> { let Latency = 6; }
def HIP11Write_7cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 7; }
def HIP11Write_7cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 7; }
def HIP11Write_7cyc_1FSU2 : SchedWriteRes<[HIP11UnitFSU2]> { let Latency = 7; }
def HIP11Write_9cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 9; }
def HIP11Write_9cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 9; }
def HIP11Write_10cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 10; }
def HIP11Write_11cyc_1FSU2 : SchedWriteRes<[HIP11UnitFSU2]> { let Latency = 11; }
def HIP11Write_13cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 13; }
def HIP11Write_15cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 15; }
def HIP11Write_16cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 16; }
def HIP11Write_16cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 16; }
def HIP11Write_18cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 18; }
def HIP11Write_21cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 21; }
def HIP11Write_24cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 24; }
def HIP11Write_27cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 27; }
def HIP11Write_46cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 46; }
def HIP11Write_48cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 48; }
def HIP11Write_76cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 76; }
def HIP11Write_94cyc_1FSU1 : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 94; }
def HIP11Write_96cyc_1FSU : SchedWriteRes<[HIP11UnitAnyFSU]> { let Latency = 96; }


def HIP11Write_6cyc_1FSU1_RC : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 6;
                                                                        let ResourceCycles = [6]; }
def HIP11Write_7cyc_1FSU1_RC : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 7;
                                                                        let ResourceCycles = [7]; }
def HIP11Write_9cyc_1FSU1_RC : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 9;
                                                                        let ResourceCycles = [9]; }
def HIP11Write_10cyc_1FSU1_RC : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 10;
                                                                        let ResourceCycles = [10]; }
def HIP11Write_11cyc_1FSU1_RC : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 11;
                                                                        let ResourceCycles = [11]; }
def HIP11Write_13cyc_1FSU1_RC : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 13;
                                                                        let ResourceCycles = [13]; }
def HIP11Write_15cyc_1FSU1_RC : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 15;
                                                                        let ResourceCycles = [15]; }
def HIP11Write_31cyc_1FSU1_RC : SchedWriteRes<[HIP11UnitFSU1]> { let Latency = 31;
                                                                        let ResourceCycles = [31]; }

def HIP11Write_4cyc_1LdSt : SchedWriteRes<[HIP11UnitAnyLdSt]> { let Latency = 4; }
def HIP11Write_5cyc_1LdSt : SchedWriteRes<[HIP11UnitAnyLdSt]> { let Latency = 5; }

def HIP11Write_1cyc_1LdSt1 : SchedWriteRes<[HIP11UnitLd0St]> { let Latency = 1; }
def HIP11Write_2cyc_1LdSt1 : SchedWriteRes<[HIP11UnitLd0St]> { let Latency = 2; }

//===----------------------------------------------------------------------===//
// Define Generic 2 micro-op types

def HIP11Write_1cyc_1LdSt1_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitFSTD]> {
  let Latency     = 1;
  let NumMicroOps = 2;
}

def HIP11Write_1cyc_1LdSt1_1AnyALU : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitAnyALU]> {
  let Latency     = 1;
  let NumMicroOps = 2;
}

def HIP11Write_2cyc_1ALU_1FSU1 : SchedWriteRes<[HIP11UnitALU, HIP11UnitFSU1]> {
  let Latency     = 2;
  let NumMicroOps = 2;
}

def HIP11Write_2cyc_2BRU : SchedWriteRes<[HIP11UnitAnyBRU, HIP11UnitAnyBRU]> {
  let Latency     = 2;
  let NumMicroOps = 2;
}

def HIP11Write_2cyc_1LdSt1_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitFSTD]> {
  let Latency     = 2;
  let NumMicroOps = 2;
}

def HIP11Write_2cyc_1LdSt1_1AnyALU : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitAnyALU]> {
  let Latency     = 2;
  let NumMicroOps = 2;
}

def HIP11Write_2cyc_1AnyALU_1FSU : SchedWriteRes<[HIP11UnitAnyALU, HIP11UnitAnyFSU]> {
  let Latency     = 2;
  let NumMicroOps = 2;
}

def HIP11Write_2cyc_1AnyALU_1FSU1 : SchedWriteRes<[HIP11UnitAnyALU, HIP11UnitFSU1]> {
  let Latency     = 2;
  let NumMicroOps = 2;
}

def HIP11Write_2cyc_1AnyALU_1ALU : SchedWriteRes<[HIP11UnitAnyALU, HIP11UnitALU]> {
  let Latency     = 2;
  let NumMicroOps = 2;
}

def HIP11Write_3cyc_1ALU_1FSU1 : SchedWriteRes<[HIP11UnitALU, HIP11UnitFSU1]> {
  let Latency     = 3;
  let NumMicroOps = 2;
}

def HIP11Write_3cyc_2FSU1 : SchedWriteRes<[HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 3;
  let NumMicroOps = 2;
}

def HIP11Write_4cyc_2FSU1 : SchedWriteRes<[HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 4;
  let NumMicroOps = 2;
}

def HIP11Write_4cyc_2LdSt1 : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St]> {
  let Latency     = 4;
  let NumMicroOps = 2;
}

def HIP11Write_4cyc_1LdSt_1AnyALU : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyALU]> {
  let Latency     = 4;
  let NumMicroOps = 2;
}

def HIP11Write_4cyc_1ALU_1FSU1 : SchedWriteRes<[HIP11UnitALU, HIP11UnitFSU1]> {
  let Latency     = 4;
  let NumMicroOps = 2;
}

def HIP11Write_5cyc_1FSU2_1LdSt : SchedWriteRes<[HIP11UnitFSU2, HIP11UnitAnyLdSt]> {
  let Latency     = 5;
  let NumMicroOps = 2;
}

def HIP11Write_5cyc_1LdSt_1AnyALU : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyALU]> {
  let Latency     = 5;
  let NumMicroOps = 2;
}

def HIP11Write_5cyc_1AnyALU_1FSU1 : SchedWriteRes<[HIP11UnitAnyALU, HIP11UnitFSU1]> {
  let Latency     = 5;
  let NumMicroOps = 2;
}

def HIP11Write_6cyc_1MDU_1BRU : SchedWriteRes<[HIP11UnitMDU, HIP11UnitAnyBRU]> {
  let Latency     = 6;
  let NumMicroOps = 2;
}

def HIP11Write_6cyc_2LdSt : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt]> {
  let Latency     = 6;
  let NumMicroOps = 2;
}

def HIP11Write_6cyc_1LdSt_1AnyALU : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyALU]> {
  let Latency     = 6;
  let NumMicroOps = 2;
}

def HIP11Write_6cyc_1AnyALU_1FSU1 : SchedWriteRes<[HIP11UnitAnyALU, HIP11UnitFSU1]> {
  let Latency     = 6;
  let NumMicroOps = 2;
}

def HIP11Write_7cyc_1LdSt_1FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitFSU1]> {
  let Latency     = 7;
  let NumMicroOps = 2;
}

def HIP11Write_7cyc_1LdSt_1FSU : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyFSU]> {
  let Latency     = 7;
  let NumMicroOps = 2;
}

def HIP11Write_7cyc_1AnyALU_1FSU1 : SchedWriteRes<[HIP11UnitAnyALU, HIP11UnitFSU1]> {
  let Latency     = 7;
  let NumMicroOps = 2;
}

def HIP11Write_8cyc_1AnyALU_1FSU1 : SchedWriteRes<[HIP11UnitAnyALU, HIP11UnitFSU1]> {
  let Latency     = 8;
  let NumMicroOps = 2;
}

def HIP11Write_8cyc_1ALU_1FSU1 : SchedWriteRes<[HIP11UnitALU, HIP11UnitFSU1]> {
  let Latency     = 8;
  let NumMicroOps = 2;
}

def HIP11Write_9cyc_1MDU_1LdSt : SchedWriteRes<[HIP11UnitMDU, HIP11UnitAnyLdSt]> {
  let Latency     = 9;
  let NumMicroOps = 2;
}

def HIP11Write_9cyc_1FSU2_1LdSt : SchedWriteRes<[HIP11UnitFSU2, HIP11UnitAnyLdSt]> {
  let Latency     = 9;
  let NumMicroOps = 2;
}

def HIP11Write_10cyc_1FSU2_1LdSt : SchedWriteRes<[HIP11UnitFSU2, HIP11UnitAnyLdSt]> {
  let Latency     = 10;
  let NumMicroOps = 2;
}

def HIP11Write_12cyc_1ALU_1FSU1 : SchedWriteRes<[HIP11UnitALU, HIP11UnitFSU1]> {
  let Latency     = 12;
  let NumMicroOps = 2;
}

//===----------------------------------------------------------------------===//
// Define Generic 3 micro-op types

def HIP11Write_1cyc_1LdSt1_1AnyALU_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitAnyALU,
                                                              HIP11UnitFSTD]> {
  let Latency     = 1;
  let NumMicroOps = 3;
}

def HIP11Write_1cyc_1LdSt1_1FSU1_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitFSU1,
                                                            HIP11UnitFSTD]> {
  let Latency     = 1;
  let NumMicroOps = 3;
}

def HIP11Write_2cyc_1LdSt1_1AnyALU_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitAnyALU,
                                                              HIP11UnitFSTD]> {
  let Latency     = 2;
  let NumMicroOps = 3;
}

def HIP11Write_2cyc_2LdSt1_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                      HIP11UnitFSTD]> {
  let Latency     = 2;
  let NumMicroOps = 3;
}

def HIP11Write_2cyc_1LdSt1_1FSU1_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitFSU1,
                                                            HIP11UnitFSTD]> {
  let Latency     = 2;
  let NumMicroOps = 3;
}

def HIP11Write_6cyc_3FSU1 : SchedWriteRes<[HIP11UnitFSU1, HIP11UnitFSU1,
                                               HIP11UnitFSU1]> {
  let Latency     = 6;
  let NumMicroOps = 3;
}

def HIP11Write_7cyc_3LdSt : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                               HIP11UnitAnyLdSt]> {
  let Latency     = 7;
  let NumMicroOps = 3;
}

//===----------------------------------------------------------------------===//
// Define Generic 4 micro-op types

def HIP11Write_2cyc_2LdSt1_2FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                      HIP11UnitFSTD, HIP11UnitFSTD]> {
  let Latency     = 2;
  let NumMicroOps = 4;
}

def HIP11Write_7cyc_2LdSt_2FSU : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                    HIP11UnitAnyFSU, HIP11UnitAnyFSU]> {
  let Latency     = 7;
  let NumMicroOps = 4;
}

def HIP11Write_7cyc_2LdSt_2FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                     HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 7;
  let NumMicroOps = 4;
}

def HIP11Write_8cyc_2LdSt_2FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                     HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 8;
  let NumMicroOps = 4;
}

def HIP11Write_8cyc_4LdSt : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                               HIP11UnitAnyLdSt, HIP11UnitAnyLdSt]> {
  let Latency     = 8;
  let NumMicroOps = 4;
}

def HIP11Write_9cyc_4FSU1 : SchedWriteRes<[HIP11UnitFSU1, HIP11UnitFSU1,
                                               HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 9;
  let NumMicroOps = 4;
}

def HIP11Write_12cyc_4FSU : SchedWriteRes<[HIP11UnitAnyFSU, HIP11UnitAnyFSU,
                                               HIP11UnitAnyFSU, HIP11UnitAnyFSU]> {
  let Latency     = 12;
  let NumMicroOps = 4;
}

def HIP11Write_14cyc_2LdSt_2FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 14;
  let NumMicroOps = 4;
}

//===----------------------------------------------------------------------===//
// Define Generic 5 micro-op types

def HIP11Write_9cyc_5FSU1 : SchedWriteRes<[HIP11UnitFSU1, HIP11UnitFSU1,
                                               HIP11UnitFSU1, HIP11UnitFSU1,
                                               HIP11UnitFSU1]> {
  let Latency     = 9;
  let NumMicroOps = 5;
}

def HIP11Write_10cyc_5FSU1 : SchedWriteRes<[HIP11UnitFSU1, HIP11UnitFSU1,
                                                HIP11UnitFSU1, HIP11UnitFSU1,
                                                HIP11UnitFSU1]> {
  let Latency     = 10;
  let NumMicroOps = 5;
}

def HIP11Write_4cyc_2LdSt1_2FSU1_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                            HIP11UnitFSU1, HIP11UnitFSU1,
                                                            HIP11UnitFSTD]> {
  let Latency     = 4;
  let NumMicroOps = 5;
}

def HIP11Write_9cyc_2LdSt1_1FSU1_2FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                            HIP11UnitFSU1, HIP11UnitFSTD,
                                                            HIP11UnitFSTD]> {
  let Latency     = 9;
  let NumMicroOps = 5;
}

def HIP11Write_14cyc_2LdSt_2FSU1_1MDU : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                           HIP11UnitFSU1, HIP11UnitFSU1,
                                                           HIP11UnitMDU]> {
  let Latency     = 14;
  let NumMicroOps = 5;
}

//===----------------------------------------------------------------------===//
// Define Generic 6 micro-op types

def HIP11Write_11cyc_6FSU1 : SchedWriteRes<[HIP11UnitFSU1, HIP11UnitFSU1,
                                                HIP11UnitFSU1, HIP11UnitFSU1,
                                                HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 11;
  let NumMicroOps = 6;
}

def HIP11Write_3cyc_3LdSt1_3FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                      HIP11UnitLd0St, HIP11UnitFSTD,
                                                      HIP11UnitFSTD, HIP11UnitFSTD]> {
  let Latency     = 3;
  let NumMicroOps = 6;
}

def HIP11Write_9cyc_2LdSt1_1FSU1_2FSTD_1MDU : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                                 HIP11UnitFSU1, HIP11UnitFSTD,
                                                                 HIP11UnitFSTD, HIP11UnitMDU]> {
  let Latency     = 9;
  let NumMicroOps = 6;
}

def HIP11Write_9cyc_3LdSt_3FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                     HIP11UnitAnyLdSt, HIP11UnitFSU1,
                                                     HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 9;
  let NumMicroOps = 6;
}

//===----------------------------------------------------------------------===//
// Define Generic 7 micro-op types

def HIP11Write_3cyc_3LdSt1_3FSU1_1FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                            HIP11UnitLd0St, HIP11UnitFSU1,
                                                            HIP11UnitFSU1, HIP11UnitFSU1,
                                                            HIP11UnitFSTD]> {
  let Latency     = 3;
  let NumMicroOps = 7;
}

//===----------------------------------------------------------------------===//
// Define Generic 8 micro-op types

def HIP11Write_4cyc_4LdSt1_4FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                      HIP11UnitLd0St, HIP11UnitLd0St,
                                                      HIP11UnitFSTD, HIP11UnitFSTD,
                                                      HIP11UnitFSTD, HIP11UnitFSTD]> {
  let Latency     = 4;
  let NumMicroOps = 8;
}

def HIP11Write_10cyc_4LdSt_4FSTD : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitFSTD, HIP11UnitFSTD,
                                                      HIP11UnitFSTD, HIP11UnitFSTD]> {
  let Latency     = 10;
  let NumMicroOps = 8;
}

def HIP11Write_10cyc_4LdSt_4FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 10;
  let NumMicroOps = 8;
}

def HIP11Write_12cyc_4LdSt_4FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 12;
  let NumMicroOps = 8;
}

//===----------------------------------------------------------------------===//
// Define Generic 12 micro-op types

def HIP11Write_15cyc_6LdSt_6FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 15;
  let NumMicroOps = 12;
}

def HIP11Write_27cyc_6LdSt_6FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 27;
  let NumMicroOps = 12;
}

//===----------------------------------------------------------------------===//
// Define Generic 13 micro-op types

def HIP11Write_23cyc_6LdSt1_1FSU1_6FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                             HIP11UnitLd0St, HIP11UnitLd0St,
                                                             HIP11UnitLd0St, HIP11UnitLd0St,
                                                             HIP11UnitFSTD, HIP11UnitFSTD,
                                                             HIP11UnitFSTD, HIP11UnitFSTD,
                                                             HIP11UnitFSTD, HIP11UnitFSTD,
                                                             HIP11UnitFSU1]> {
  let Latency     = 23;
  let NumMicroOps = 13;
}

def HIP11Write_27cyc_6LdSt_6FSU1_1MDU : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                           HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                           HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                           HIP11UnitFSU1, HIP11UnitFSU1,
                                                           HIP11UnitFSU1, HIP11UnitFSU1,
                                                           HIP11UnitFSU1, HIP11UnitFSU1,
                                                           HIP11UnitMDU]> {
  let Latency     = 27;
  let NumMicroOps = 13;
}

//===----------------------------------------------------------------------===//
// Define Generic 14 micro-op types

def HIP11Write_23cyc_6LdSt1_1FSU1_6FSTD_1MDU : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                                  HIP11UnitLd0St, HIP11UnitLd0St,
                                                                  HIP11UnitLd0St, HIP11UnitLd0St,
                                                                  HIP11UnitFSU1, HIP11UnitFSTD,
                                                                  HIP11UnitFSTD, HIP11UnitFSTD,
                                                                  HIP11UnitFSTD, HIP11UnitFSTD,
                                                                  HIP11UnitFSTD, HIP11UnitMDU]> {
  let Latency     = 23;
  let NumMicroOps = 14;
}

//===----------------------------------------------------------------------===//
// Define Generic 16 micro-op types

def HIP11Write_10cyc_8LdSt1_8FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                       HIP11UnitLd0St, HIP11UnitLd0St,
                                                       HIP11UnitLd0St, HIP11UnitLd0St,
                                                       HIP11UnitLd0St, HIP11UnitLd0St,
                                                       HIP11UnitFSTD, HIP11UnitFSTD,
                                                       HIP11UnitFSTD, HIP11UnitFSTD,
                                                       HIP11UnitFSTD, HIP11UnitFSTD,
                                                       HIP11UnitFSTD, HIP11UnitFSTD]> {
  let Latency     = 10;
  let NumMicroOps = 16;
}

def HIP11Write_14cyc_8LdSt_8FSTD : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitFSTD, HIP11UnitFSTD,
                                                      HIP11UnitFSTD, HIP11UnitFSTD,
                                                      HIP11UnitFSTD, HIP11UnitFSTD,
                                                      HIP11UnitFSTD, HIP11UnitFSTD]> {
  let Latency     = 14;
  let NumMicroOps = 16;
}

def HIP11Write_33cyc_8LdSt_8FSU1 : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1,
                                                      HIP11UnitFSU1, HIP11UnitFSU1]> {
  let Latency     = 33;
  let NumMicroOps = 16;
}

//===----------------------------------------------------------------------===//
// Define Generic 17 micro-op types

def HIP11Write_30cyc_8LdSt1_1FSU1_8FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                             HIP11UnitLd0St, HIP11UnitLd0St,
                                                             HIP11UnitLd0St, HIP11UnitLd0St,
                                                             HIP11UnitLd0St, HIP11UnitLd0St,
                                                             HIP11UnitFSU1, HIP11UnitFSTD,
                                                             HIP11UnitFSTD, HIP11UnitFSTD,
                                                             HIP11UnitFSTD, HIP11UnitFSTD,
                                                             HIP11UnitFSTD, HIP11UnitFSTD,
                                                             HIP11UnitFSTD]> {
  let Latency     = 30;
  let NumMicroOps = 17;
}

def HIP11Write_33cyc_8LdSt_8FSU1_1MDU : SchedWriteRes<[HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                           HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                           HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                           HIP11UnitAnyLdSt, HIP11UnitAnyLdSt,
                                                           HIP11UnitFSU1, HIP11UnitFSU1,
                                                           HIP11UnitFSU1, HIP11UnitFSU1,
                                                           HIP11UnitFSU1, HIP11UnitFSU1,
                                                           HIP11UnitFSU1, HIP11UnitFSU1,
                                                           HIP11UnitMDU]> {
  let Latency     = 33;
  let NumMicroOps = 17;
}

//===----------------------------------------------------------------------===//
// Define Generic 18 micro-op types

def HIP11Write_30cyc_8LdSt1_1FSU1_8FSTD_1MDU : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                                  HIP11UnitLd0St, HIP11UnitLd0St,
                                                                  HIP11UnitLd0St, HIP11UnitLd0St,
                                                                  HIP11UnitLd0St, HIP11UnitLd0St,
                                                                  HIP11UnitFSU1, HIP11UnitFSTD,
                                                                  HIP11UnitFSTD, HIP11UnitFSTD,
                                                                  HIP11UnitFSTD, HIP11UnitFSTD,
                                                                  HIP11UnitFSTD, HIP11UnitFSTD,
                                                                  HIP11UnitFSTD, HIP11UnitMDU]> {
  let Latency     = 30;
  let NumMicroOps = 18;
}

//===----------------------------------------------------------------------===//
// Define Generic 32 micro-op types

def HIP11Write_18cyc_16LdSt1_16FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                         HIP11UnitLd0St, HIP11UnitLd0St,
                                                         HIP11UnitLd0St, HIP11UnitLd0St,
                                                         HIP11UnitLd0St, HIP11UnitLd0St,
                                                         HIP11UnitLd0St, HIP11UnitLd0St,
                                                         HIP11UnitLd0St, HIP11UnitLd0St,
                                                         HIP11UnitLd0St, HIP11UnitLd0St,
                                                         HIP11UnitLd0St, HIP11UnitLd0St,
                                                         HIP11UnitFSTD, HIP11UnitFSTD,
                                                         HIP11UnitFSTD, HIP11UnitFSTD,
                                                         HIP11UnitFSTD, HIP11UnitFSTD,
                                                         HIP11UnitFSTD, HIP11UnitFSTD,
                                                         HIP11UnitFSTD, HIP11UnitFSTD,
                                                         HIP11UnitFSTD, HIP11UnitFSTD,
                                                         HIP11UnitFSTD, HIP11UnitFSTD,
                                                         HIP11UnitFSTD, HIP11UnitFSTD]> {
  let Latency     = 18;
  let NumMicroOps = 32;
}

//===----------------------------------------------------------------------===//
// Define Generic 33 micro-op types

def HIP11Write_16cyc_16LdSt1_1FSU1_16FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                               HIP11UnitLd0St, HIP11UnitLd0St,
                                                               HIP11UnitLd0St, HIP11UnitLd0St,
                                                               HIP11UnitLd0St, HIP11UnitLd0St,
                                                               HIP11UnitLd0St, HIP11UnitLd0St,
                                                               HIP11UnitLd0St, HIP11UnitLd0St,
                                                               HIP11UnitLd0St, HIP11UnitLd0St,
                                                               HIP11UnitLd0St, HIP11UnitLd0St,
                                                               HIP11UnitFSTD, HIP11UnitFSTD,
                                                               HIP11UnitFSTD, HIP11UnitFSTD,
                                                               HIP11UnitFSTD, HIP11UnitFSTD,
                                                               HIP11UnitFSTD, HIP11UnitFSTD,
                                                               HIP11UnitFSTD, HIP11UnitFSTD,
                                                               HIP11UnitFSTD, HIP11UnitFSTD,
                                                               HIP11UnitFSTD, HIP11UnitFSTD,
                                                               HIP11UnitFSTD, HIP11UnitFSTD,
                                                               HIP11UnitFSU1]> {
  let Latency     = 16;
  let NumMicroOps = 33;
}

//===----------------------------------------------------------------------===//
// Define Generic 45 micro-op types

def HIP11Write_22cyc_22LdSt1_FSU1_22FSTD : SchedWriteRes<[HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitLd0St, HIP11UnitLd0St,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSTD, HIP11UnitFSTD,
                                                              HIP11UnitFSU1]> {
  let Latency     = 22;
  let NumMicroOps = 45;
}

//===----------------------------------------------------------------------===//
// Map the target-defined scheduler read/write resources and latency for HIP11

// Integer ALU
def : WriteRes<WriteImm,   [HIP11UnitAnyALU]> { let Latency = 1; }
def : WriteRes<WriteI,     [HIP11UnitAnyALU]> { let Latency = 1; }
def : WriteRes<WriteISReg, [HIP11UnitMDU]>    { let Latency = 2; }
def : WriteRes<WriteIEReg, [HIP11UnitMDU]>    { let Latency = 2; }
def : WriteRes<WriteExtr,  [HIP11UnitAnyALU]> { let Latency = 1; }
def : WriteRes<WriteIS,    [HIP11UnitAnyALU]> { let Latency = 1; }

// Integer Mul/MAC/Div
def : WriteRes<WriteID32,  [HIP11UnitMDU]> { let Latency = 12;
                                                 let ResourceCycles = [12]; }
def : WriteRes<WriteID64,  [HIP11UnitMDU]> { let Latency = 20;
                                                 let ResourceCycles = [20]; }
def : WriteRes<WriteIM32,  [HIP11UnitMDU]> { let Latency = 3; }
def : WriteRes<WriteIM64,  [HIP11UnitMDU]> { let Latency = 4; }

// Load
def : WriteRes<WriteLD,    [HIP11UnitAnyLdSt]> { let Latency = 4; }
def : WriteRes<WriteLDHi,  []> { let Latency = 4; }

// Pre/Post Indexing
def : WriteRes<WriteLDIdx, [HIP11UnitAnyALU]> { let Latency = 4; }
def : WriteRes<WriteSTIdx, [HIP11UnitAnyALU]> { let Latency = 1; }
def : WriteRes<WriteAdr,   [HIP11UnitAnyALU]> { let Latency = 1; }

// Store
def : WriteRes<WriteST,    [HIP11UnitLd0St]> { let Latency = 1; }
def : WriteRes<WriteSTP,   [HIP11UnitLd0St]> { let Latency = 1; }

// FP
def : WriteRes<WriteF,     [HIP11UnitAnyFSU]> { let Latency = 2; }
def : WriteRes<WriteFCmp,  [HIP11UnitAnyFSU]> { let Latency = 1; }
def : WriteRes<WriteFCvt,  [HIP11UnitFSU1]>   { let Latency = 2; }
def : WriteRes<WriteFCopy, [HIP11UnitFSU1]>   { let Latency = 1; }
def : WriteRes<WriteFImm,  [HIP11UnitAnyFSU]> { let Latency = 2; }
def : WriteRes<WriteFMul,  [HIP11UnitAnyFSU]> { let Latency = 3; }

// FP Div, Sqrt
def : WriteRes<WriteFDiv,  [HIP11UnitFSU1]>   { let Latency = 10;
                                                    let ResourceCycles = [10]; }
def : WriteRes<WriteVd,    [HIP11UnitAnyFSU]> { let Latency = 4; }
def : WriteRes<WriteVq,    [HIP11UnitAnyFSU]> { let Latency = 4; }
def : WriteRes<WriteVST,   [HIP11UnitAnyFSU, HIP11UnitFSTD]> { let Latency = 1; }

// Branch
def : WriteRes<WriteBr,    [HIP11UnitAnyBRU]> { let Latency = 1; }
def : WriteRes<WriteBrReg, [HIP11UnitAnyBRU]> { let Latency = 1; }

def : WriteRes<WriteSys,     []> { let Latency = 1; }
def : WriteRes<WriteBarrier, []> { let Latency = 1; }
def : WriteRes<WriteHint,    []> { let Latency = 1; }

// Forwarding logic is modeled only for multiply and accumulate.
def : ReadAdvance<ReadI,       0>;
def : ReadAdvance<ReadISReg,   0>;
def : ReadAdvance<ReadIEReg,   0>;
def : ReadAdvance<ReadIM,      0>;
def : ReadAdvance<ReadIMA,     2, [WriteIM32, WriteIM64]>;
def : ReadAdvance<ReadID,      0>;
def : ReadAdvance<ReadExtrHi,  0>;
def : ReadAdvance<ReadAdrBase, 0>;
def : ReadAdvance<ReadVLD,     0>;
def : ReadAdvance<ReadST,      0>;

def : WriteRes<WriteAtomic,  []> { let Unsupported = 1; }

def : InstRW<[WriteI], (instrs COPY)>;

// Detailed Refinements
//===----------------------------------------------------------------------===//

// Contains all of the HIP11-specific SchedWriteRes types. The approach below
// is to define a generic SchedWriteRes for every combination of latency and
// micro-ops. The naming conventions is to use a prefix, one field for latency,
// and one or more microOp count/type designators.
//
//   Prefix: HIP11Write
//   Latency: #cyc
//   Micro-op Count/Types: #(ALU|BRU|MDU|LdSt|FSU1|FSU2|FSU|FSTD)
//
// e.g. HIP11Write_6cyc_1ALU_6LdSt_4FSU means the total latency is 6 cycles,
// and 1 micro-ops are issued down one ALU pipe, six LdSt pipes, and four FSU
// pipes.

/*Add specific SchedWriteRes types.*/

// Branch Instructions
// -----------------------------------------------------------------------------

// Branch, immed
// Branch and link, immed
// Branch and link, register
def : InstRW<[HIP11Write_1cyc_1BRU], (instrs B, BL, BLR)>;

// Branch, register
// Compare and branch
def : InstRW<[HIP11Write_1cyc_1BRU], (instregex "^(BR|RET|(CBZ|CBNZ|TBZ|TBNZ))$")>;

// Branch with link to register
// Branch to register
// Return from subroutine
def : InstRW<[HIP11Write_6cyc_1MDU_1BRU], (instrs BLRAA, BLRAAZ, BLRAB, BLRABZ, BRAA,
                                               BRAAZ, BRAB, BRABZ, RETAA, RETAB)>;

// Exception return
def : InstRW<[HIP11Write_2cyc_1MDU], (instrs ERETAA, ERETAB)>;

// Arithmetic and Logical Instructions
// -----------------------------------------------------------------------------

// Instruction classification references "Arm Cortex-A77 Core Software Opimization Guide"

// Arithmetic, basic
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(ADD|SUB)[WX]r(r|i)$")>;
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(ADC|SBC)[WX]r$")>;

// Arithmetic, basic, flag set
def : InstRW<[HIP11Write_1cyc_1BRU],    (instregex "^(ADD|SUB)S[WX]r(r|i)$")>;
def : InstRW<[HIP11Write_1cyc_1BRU],    (instregex "^(ADC|SBC)S[WX]r$")>;

// Arithmetic, extend and shift
// extend
def HIP11WriteIEReg : SchedWriteVariant<[
                            SchedVar<RegExtendedPred, [WriteIEReg]>,
                            SchedVar<NoSchedPred, [WriteI]>]>;
def : InstRW<[HIP11WriteIEReg], (instregex "^(ADD|SUB)[WX]r(x|x64)$")>;

def HIP11WriteIERegBr : SchedWriteVariant<[
                              SchedVar<RegExtendedPred, [HIP11Write_2cyc_2BRU]>,
                              SchedVar<NoSchedPred, [WriteBr]>]>;
def : InstRW<[HIP11WriteIERegBr], (instregex "^(ADD|SUB)S[WX]r(x|x64)$")>;

// shift
def HIP11WriteISReg : SchedWriteVariant<[
                            SchedVar<RegShiftedPred, [WriteISReg]>,
                            SchedVar<NoSchedPred, [WriteI]>]>;
def : InstRW<[HIP11WriteISReg], (instregex "^(ADD|SUB)[WX]rs$")>;

def HIP11WriteISRegBr : SchedWriteVariant<[
                              SchedVar<RegShiftedPred, [HIP11Write_2cyc_2BRU]>,
                              SchedVar<NoSchedPred, [WriteBr]>]>;
def : InstRW<[HIP11WriteISRegBr], (instregex "^(ADD|SUB)S[WX]rs$")>;

// Conditional compare
def : InstRW<[HIP11Write_1cyc_1BRU], (instregex "^(CCMN|CCMP)(W|X)(r|i)$")>;

// Conditional select
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(CSEL|CSINC|CSINV|CSNEG)(W|X)r$")>;

// Logical, basic
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(AND|EOR|ORR)[WX]r(r|i)$")>;
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(EON|ORN)[WX]rr$")>;
def : InstRW<[HIP11Write_2cyc_1MDU],    (instregex "^BIC[WX]rr$")>;

def : InstRW<[HIP11Write_1cyc_1BRU],   (instregex "^ANDS[WX]r(r|i)$")>;
def : InstRW<[HIP11Write_2cyc_1MDU],   (instregex "^BICS[WX]rr$")>;

// Logical, shift, no flagset
def : InstRW<[HIP11WriteISReg], (instregex "^(AND|BIC|EON|EOR|ORN|ORR)[WX]rs$")>;

// Logical, shift, flagset
def : InstRW<[HIP11WriteISRegBr], (instregex "^ANDS[WX]rs$")>;
def : InstRW<[HIP11WriteISReg], (instregex "^BICS[WX]rs$")>;

// Move and shift instructions
// -----------------------------------------------------------------------------
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instrs ADR, ADRP)>;
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^MOV[NZK][WX]i")>;
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(LSLV|LSRV|ASRV|RORV)(W|X)r$")>;

// Divide and Multiply Instructions
// -----------------------------------------------------------------------------

// Divide, W-form
def : InstRW<[HIP11Write_12cyc_1MDU_RC], (instregex "^(S|U)DIVWr$")>;

// Divide, X-form
def : InstRW<[HIP11Write_20cyc_1MDU_RC], (instregex "^(S|U)DIVXr$")>;

// Multiply accumulate, W-form
def HIP11ReadMAW : SchedReadAdvance<2, [HIP11Write_3cyc_1MDU]>;
def : InstRW<[HIP11Write_3cyc_1MDU, ReadIM, ReadIM, HIP11ReadMAW], (instrs MADDWrrr, MSUBWrrr)>;

// Multiply accumulate, X-form
def HIP11ReadMAQ : SchedReadAdvance<3, [HIP11Write_4cyc_1MDU]>;
def : InstRW<[HIP11Write_3cyc_1MDU, ReadIM, ReadIM, HIP11ReadMAQ], (instrs MADDXrrr, MSUBXrrr)>;

// Multiply accumulate long
def : InstRW<[HIP11Write_3cyc_1MDU, HIP11ReadMAW], (instregex "(S|U)(MADDL|MSUBL)rrr")>;

// Multiply high
def : InstRW<[HIP11Write_4cyc_1MDU], (instregex "^(S|U)MULHrr$")>;

// Miscellaneous Data-Processing Instructions
// -----------------------------------------------------------------------------

// Bitfield extract
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^EXTR(W|X)rri$")>;

// Bitfield move
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(S|U)?BFM(W|X)ri$")>;

// Count leading
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(CLS|CLZ)(W|X)r$")>;

// Reverse bits/bytes
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(RBIT|REV(16|32)?)(W|X)r$")>;

// Pointer Authentication Instructions
// -----------------------------------------------------------------------------

// Authenticate data address
// Authenticate instruction address
// Pointer authentication code for data address
// Pointer Authentication Code for Instruction address
def : InstRW<[HIP11Write_5cyc_1MDU], (instregex "^AUT", "^PAC")>;

// Strip pointer authentication code
def : InstRW<[HIP11Write_2cyc_1MDU], (instrs XPACD, XPACI, XPACLRI)>;

// Load instructions
// -----------------------------------------------------------------------------

// Load register, literal
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^LDR(W|X)l$")>;
def : InstRW<[HIP11Write_4cyc_1LdSt], (instrs LDRSWl)>;
def : InstRW<[HIP11Write_4cyc_1LdSt], (instrs PRFMl)>;

// Load register, unscaled immed
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^LDUR(BB|HH|W|X)i$")>;
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^LDURS(BW|BX|HW|HX|W)i$")>;
def : InstRW<[HIP11Write_4cyc_1LdSt], (instrs PRFUMi)>;

// Load register, immed post/pre-index
def : InstRW<[WriteAdr, HIP11Write_4cyc_1LdSt], (instregex "^LDR(BB|HH|W|X)(post|pre)$")>;
def : InstRW<[WriteAdr, HIP11Write_4cyc_1LdSt], (instregex "^LDRS(BW|BX|HW|HX|W)(post|pre)$")>;

// Load register, immed unprivileged
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^LDTR(B|H|W|X)i$")>;
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^LDTRS(BW|BX|HW|HX|W)i$")>;

// Load register, unsigned immed
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^LDR(BB|HH|W|X)ui$")>;
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^LDRS(BW|BX|HW|HX|W)ui$")>;
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^PRFMui$")>;

// Load register, register offset
def : InstRW<[HIP11Write_5cyc_1LdSt_1AnyALU], (instregex "^LDR(BB|HH|W|X)ro(W|X)$")>;
def : InstRW<[HIP11Write_5cyc_1LdSt_1AnyALU], (instregex "^LDRS(BW|BX|HW|HX|W)ro(W|X)$")>;
def : InstRW<[HIP11Write_5cyc_1LdSt_1AnyALU], (instregex "^PRFMro(W|X)$")>;

// Load pair, immed offset
def : InstRW<[HIP11Write_4cyc_1LdSt], (instregex "^LDN?P(W|X)i$")>;
def : InstRW<[HIP11Write_4cyc_1LdSt], (instrs LDPSWi)>;

// Load pair, immed post/pre-index
def : InstRW<[WriteAdr, HIP11Write_4cyc_1LdSt_1AnyALU, WriteLDHi], (instregex "^LDP(W|X)(post|pre)$")>;
def : InstRW<[WriteAdr, HIP11Write_4cyc_1LdSt_1AnyALU, WriteLDHi], (instrs LDPSWpost, LDPSWpre)>;

// Load Register, with pointer authentication
def : InstRW<[HIP11Write_9cyc_1MDU_1LdSt], (instregex "^LDRA[AB](indexed|writeback)")>;

// Store instructions
// -----------------------------------------------------------------------------

// Store register, unscaled immed
def : InstRW<[HIP11Write_1cyc_1LdSt1], (instregex "^STUR(BB|HH|W|X)i$")>;

// Store register, immed post/pre-index
def : InstRW<[WriteAdr, HIP11Write_1cyc_1LdSt1_1AnyALU],  (instregex "^STR(BB|HH|W|X)(post|pre)$")>;

// Store register, immed unprivileged
def : InstRW<[HIP11Write_1cyc_1LdSt1], (instregex "^STTR(B|H|W|X)i$")>;

// Store register, unsigned immed
def : InstRW<[HIP11Write_1cyc_1LdSt1], (instregex "^STR(BB|HH|W|X)ui$")>;

// Store register, register offset
def : InstRW<[HIP11Write_2cyc_1LdSt1_1AnyALU, ReadST, ReadAdrBase], (instregex "^STR(BB|HH|W|X)ro(W|X)$")>;

// Store pair, immed offset
def : InstRW<[HIP11Write_1cyc_1LdSt1], (instregex "^STN?P(W|X)i$")>;

// Store pair, immed post/pre-index
def : InstRW<[WriteAdr, HIP11Write_2cyc_1LdSt1_1AnyALU], (instregex "^STP(W|X)(post|pre)$")>;

// FP data processing instructions
// -----------------------------------------------------------------------------

// FP sign manipulation
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "F(ABS|NEG)(H|D|S)r")>;

// FP conditional compare
def : InstRW<[HIP11Write_2cyc_1AnyALU_1FSU], (instregex "^FCCMP(E)?(S|D)rr$")>;

// FP conditional select
def : InstRW<[HIP11Write_2cyc_1AnyALU_1FSU1], (instregex "^FCSEL(S|D)rrr$")>;

// FP compare
def : InstRW<[HIP11Write_1cyc_1FSU], (instregex "^FCMP(E)?(S|D)r(r|i)$")>;

// FP divide, H-form
def : InstRW<[HIP11Write_6cyc_1FSU1_RC], (instrs FDIVHrr)>;
// FP divide, S-form
def : InstRW<[HIP11Write_7cyc_1FSU1_RC], (instrs FDIVSrr)>;
// FP divide, D-form
def : InstRW<[HIP11Write_10cyc_1FSU1_RC], (instrs FDIVDrr)>;

// FP square root, H-form
def : InstRW<[HIP11Write_7cyc_1FSU1_RC], (instrs FSQRTHr)>;
// FP square root, S-form
def : InstRW<[HIP11Write_9cyc_1FSU1_RC], (instrs FSQRTSr)>;
// FP square root, D-form
def : InstRW<[HIP11Write_15cyc_1FSU1_RC], (instrs FSQRTDr)>;

// FP fused multiply-add
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^FN?M(ADD|SUB)[HSD]rrr$")>;

// FP max/min
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^F(MAX|MIN).+rr")>;

// FP add
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^F(ADD|SUB).+rr")>;

// FP multiply
def : InstRW<[HIP11Write_3cyc_1FSU], (instregex "^FN?M(UL).+rr")>;

// Floating-point Round to Integral
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRINT[AIMNPXZ][HSD]r$",
                                           "^FRINT(32|64)[XZ][SD]r$")>;

// FP miscellaneous instructions
// -----------------------------------------------------------------------------

// Instruction classification references "Arm Cortex-A77 Core Software Opimization Guide"

// FP convert, from vec to vec reg
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVT[HSD][HSD]r")>;

// FP convert, from gen to vec reg
def : InstRW<[HIP11Write_3cyc_1ALU_1FSU1], (instregex "^[SU]CVTF[SU][WX][SD]ri$")>;

// FP convert, from vec to gen reg
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVT(A|M|N|P|Z)(S|U)U(W|X)(S|D|H)r$")>;

// FP move, immed
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs FMOVHi, FMOVSi, FMOVDi)>;

// FP move, register, w/o conversion
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs FMOVHr, FMOVSr, FMOVDr)>;

// FP move, general
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs FMOVHWr, FMOVHXr, FMOVSWr, FMOVDXr)>;
def : InstRW<[HIP11Write_1cyc_1ALU], (instrs FMOVWSr, FMOVXDr)>;
def : InstRW<[HIP11Write_2cyc_1AnyALU_1ALU], (instrs FMOVWHr, FMOVXHr)>;

// FP transfer, from gen to vec reg
def : InstRW<[HIP11Write_3cyc_1ALU_1FSU1], (instrs FMOVXDHighr)>;

// FP transfer, from vec to gen reg
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs FMOVDXHighr)>;

// FP load instructions
// -----------------------------------------------------------------------------

// Load vector reg, literal
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LDR[DSQ]l")>;

// Load vector reg, unscaled immed
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LDUR[BDHSQ]i")>;

// Load vector reg, immed post/pre-index
def : InstRW<[WriteAdr, HIP11Write_5cyc_1LdSt], (instregex "^LDR[BDHSQ](post|pre)")>;

// Load vector reg, unsigned immed
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LDR[BDHSQ]ui")>;

// Load vector reg, register offset
def : InstRW<[HIP11Write_6cyc_1LdSt_1AnyALU, ReadAdrBase], (instregex "^LDR(Q|D|H|S|B)ro(W|X)$")>;

// Load vector pair, immed offset
def : InstRW<[HIP11Write_5cyc_1LdSt, WriteLDHi], (instregex "^LDN?P[DQS]i")>;

// Load vector pair, immed post/pre-index
def : InstRW<[WriteAdr, HIP11Write_5cyc_1LdSt, WriteLDHi], (instregex "^LDP[DQS](post|pre)")>;

// FP store instructions
// -----------------------------------------------------------------------------

// Store vector reg, unscaled immed
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "^STUR[BHSDQ]i")>;

// Store vector reg, immed post/pre-index
def : InstRW<[HIP11Write_1cyc_1LdSt1_1AnyALU_1FSTD, ReadAdrBase], (instregex "^STR[BHSDQ](post|pre)")>;

// Store vector reg, unsigned immed
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "^STR[BHSDQ]ui")>;

// Store vector reg, reg offset
def : InstRW<[HIP11Write_2cyc_1LdSt1_1AnyALU_1FSTD, ReadAdrBase], (instregex "^STR[BHSDQ]ro[WX]")>;

// Store vector pair, immed offset
def : InstRW<[HIP11Write_2cyc_2LdSt1_1FSTD], (instregex "^STN?P[SDQ]i")>;

// Store vector pair, immed post/pre-index
def : InstRW<[WriteAdr, HIP11Write_2cyc_2LdSt1_1FSTD],(instregex "^STP[SDQ](post|pre)")>;

// ASIMD Integer Instructions
// -----------------------------------------------------------------------------

// Reference for forms in this group
//   D form - v8i8, v4i16, v2i32
//   Q form - v16i8, v8i16, v4i32
//   D form - v1i8, v1i16, v1i32, v1i64
//   Q form - v16i8, v8i16, v4i32, v2i64
//   D form - v8i8_v8i16, v4i16_v4i32, v2i32_v2i64
//   Q form - v16i8_v8i16, v8i16_v4i32, v4i32_v2i64

// ASIMD bit reverse
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^RBITv")>;

def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(ABS|NEG)v")>;

// ASIMD absolute diff accum
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]ABAv")>;

// ASIMD absolute diff accum long
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]ABAL2?v")>;

// ASIMD arith, reduce, 4H/4S
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^(ADDV|[SU]ADDLV)v4(i16|i32)v$")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^[SU]ADD[L|W]v")>;

// ASIMD arith, reduce, 8B/8H
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^(ADDV|[SU]ADDLV)v8(i8|i16)v$")>;

// ASIMD arith, reduce, 16B
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs ADDVv16i8v, SADDLVv16i8v, UADDLVv16i8v)>;

// ASIMD MMLA
def : InstRW<[HIP11Write_7cyc_1FSU1], (instrs UMMLA, SMMLA, USMMLA)>;

// ASIMD dot product
// ASIMD dot product using signed and unsigned integers
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^([SU]|SU|US)DOT(lane)?(v8|v16)i8$")>;

// ASIMD max/min, reduce, 4H/4S
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^[SU](MIN|MAX)V(v4i16|v4i32)v$")>;
// ASIMD max/min, reduce, 8B/8H
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^[SU](MIN|MAX)V(v8i8|v8i16)v$")>;
// ASIMD max/min, reduce, 16B
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^[SU](MIN|MAX)Vv16i8v$")>;

// ASIMD multiply, D-form
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^(MUL|SQR?DMULH)(v8i8|v4i16|v2i32|v1i8|v1i16|v1i32|v1i64)(_indexed)?$")>;
// ASIMD multiply, Q-form
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^(MUL|SQR?DMULH)(v16i8|v8i16|v4i32)(_indexed)?$")>;
// ASIMD multiply accumulate, D-form
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^ML[AS](v8i8|v4i16|v2i32)(_indexed)?$")>;
// ASIMD multiply accumulate, Q-form
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^ML[AS](v16i8|v8i16|v4i32)(_indexed)?$")>;

// ASIMD multiply long
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^(S|U|SQD)MULLv", "^SQDMULL(i16|i32)")>;

// ASIMD multiply accumulate long
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^(S|U|SQD)ML[AS]Lv", "^SQDML[AS]L(i16|i32)")>;

// ASIMD multiply accumulate high
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SQRDMLAHv", "^SQRDMLSHv")>;

// ASIMD multiply/multiply long (8x8) polynomial
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^PMULL(v8i8|v16i8)", "^PMUL(v8i8|v16i8)")>;

// ASIMD pairwise add and accumulate long
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]ADALPv")>;

// ASIMD shift accumulate
def : InstRW<[HIP11Write_4cyc_1FSU1], (instregex "^(S|SR|U|UR)SRAv", "^(S|SR|U|UR)SRAd")>;

// ASIMD shift by immed, basic
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SHL(v|d)", "^SHLLv", "^SHRNv", "^SSHLLv",
                                          "^SSHR(v|d)", "^USHLLv", "^USHR(v|d)")>;

// ASIMD shift by immed and insert, basic
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SLI(v|d)", "^SRI(v|d)")>;

// ASIMD shift by immed, complex
def : InstRW<[HIP11Write_4cyc_1FSU1], (instregex "^RSHRNv", "^SQRSHRN(v|b|h|s)",
                                          "^SRSHR(v|d)", "^URSHR(v|d)")>;
def : InstRW<[HIP11Write_4cyc_1FSU1], (instregex "^SQRSHRUN(v|b|h|s)")>;

def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SQSHLU(b|h|s|d|v)", "^SQSHR(N|UN)(b|h|s|v)",
                                          "^UQR?SHRN(b|h|s|v)")>;

// ASIMD shift by register, basic
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]SHLv")>;

def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^SSUB[L|W]v")>;

def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "TRN[1|2]v")>;

// ASIMD shift by register, complex
def : InstRW<[HIP11Write_4cyc_1FSU1], (instregex "^[SU]RSHLv", "^[SU]QRSHLv")>;
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]QSHL(b|h|s|d|v8i8_shift|v16i8_shift|v4i16_shift|v8i16_shift|v2i32_shift|v4i32_shift|v2i64_shift)",
                                          "^[SU]QSHL(v1i8|v1i16|v1i32|v1i64|v8i8|v16i8|v4i16|v8i16|v2i32|v4i32|v2i64)$")>;

// ASIMD Arithmetic
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "(ADD|SUB)(v8i8|v4i16|v2i32|v1i64)")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "(ADD|SUB)(v16i8|v8i16|v4i32|v2i64)")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "(ADD|SUB)HNv.*")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "(RADD|RSUB)HNv.*", "^SUQADDv", "^USQADDv")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^SQADDv", "^SQNEGv", "^SQSUBv", "^SRHADDv", "^SHADDv", "^SHSUBv",
                                          "^UQADDv", "^UQSUBv", "^URHADDv", "^SQABSv", "^UHADDv",
                                          "^UHSUBv", "^UADDLP", "^SADDLP")>;
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^USUBLv", "^USUBWv")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "ADDP(v8i8|v16i8|v4i16|v8i16|v2i32|v4i32|v2i64|v2i64p)")>;

// ASIMD logical (MVN (alias for NOT), ORN, ORR)
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^ANDv", "^BICv", "^EORv", "^ORRv", "^ORNv", "^NOTv")>;

def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "(CLS|CLZ)(v8i8|v2i32|v4i16|v4i32|v8i16|v16i8)")>;

def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "CNT(v8i8|v16i8)")>;

def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^CMEQv", "^CMGEv", "^CMGTv", "^CMTSTv",
                                                    "^CMLEv", "^CMLTv", "^CMHIv", "^CMHSv")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^SMAXv", "^SMINv", "^UMAXv", "^UMINv")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^SMAXPv", "^SMINPv", "^UMAXPv", "^UMINPv")>;
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^SABDv", "^UABDv")>;
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^SABDLv", "^UABDLv")>;

// ASIMD Floating-Point instructions
// -----------------------------------------------------------------------------

// Reference for forms in this group
//   D form - v2f32
//   Q form - v4f32, v2f64
//   D form - 32, 64
//   D form - v1i32, v1i64
//   D form - v2i32
//   Q form - v4i32, v2i64

// ASIMD FP complex multiply add
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^FCMLAv")>;

// ASIMD FP arith, normal, D-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^(FABD|FADD|FSUB)(v2f32|32|64|v2i32p)")>;

// ASIMD FP arith, normal, Q-form
// same as D-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^(FABD|FADD|FSUB)(v4f32|v2f64|v2i64p)")>;

// ASIMD FP arith, pairwise, D-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FADDP(v2f32|32|64|v2i32)")>;

// ASIMD FP arith, pairwise, Q-form
// same as D-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FADDP(v4f32|v2f64|v2i64)")>;

// ASIMD FP compare, D-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^(FACGE|FACGT|FCMEQ|FCMGE|FCMGT|FCMLE|FCMLT)(v2f32|32|64|v1i32|v2i32|v1i64)")>;

// ASIMD FP compare, Q-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^(FACGE|FACGT|FCMEQ|FCMGE|FCMGT|FCMLE|FCMLT)(v4f32|v2f64|v4i32|v2i64)")>;

// ASIMD FP convert, long (F16 to F32)
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVTL(v4|v8)i16")>;

// ASIMD FP convert, long (F32 to F64)
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVTL(v2|v4)i32")>;

// ASIMD FP convert, narrow (F32 to F16)
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVTN(v4|v8)i16")>;

// ASIMD FP convert, narrow (F64 to F32)
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVTN(v2|v4)i32", "^FCVTXN(v2|v4)f32", "^FCVTXNv")>;

def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]CVTF(d|s|h)$", "^[SU]CVTFv1(i16|i32|i64|f64)$", "^FCVTZ(S|U)(d|s|h)$",
                                          "^[SU]CVTF(v4i16_shift|v8i16_shift|v2i32_shift|v4i32_shift|v2i64_shift)$",
                                          "^FCVTZ(S|U)(v4i16_shift|v8i16_shift|v2i32_shift|v4i32_shift|v2i64_shift)$")>;

// ASIMD FP convert, other, D-form F32 and Q-form F64
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[FSU]CVT[AMNPZ][SU]v2f(32|64)$", "^[SU]CVTFv2f(32|64)$")>;

// ASIMD FP convert, other, D-form F16 and Q-form F32
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[FSU]CVT[AMNPZ][SU]v4f(16|32)$", "^[SU]CVTFv4f(16|32)$")>;

// ASIMD FP convert, other, Q-form F16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[FSU]CVT[AMNPZ][SU]v8f16$", "^[SU]CVTFv8f16$")>;

def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[FSU]CVT[AMNPZ][SU]v1(i32|i64|f16)$")>;

// ASIMD FP divide, D-form, F16
def : InstRW<[HIP11Write_7cyc_1FSU1_RC], (instregex "FDIVv4f16")>;

// ASIMD FP divide, D-form, F32
def : InstRW<[HIP11Write_7cyc_1FSU1_RC], (instregex "FDIVv2f32")>;

// ASIMD FP divide, Q-form, F16
def : InstRW<[HIP11Write_9cyc_1FSU1_RC], (instregex "FDIVv8f16")>;

// ASIMD FP divide, Q-form, F32
def : InstRW<[HIP11Write_9cyc_1FSU1_RC], (instregex "FDIVv4f32")>;

// ASIMD FP divide, Q-form, F64
def : InstRW<[HIP11Write_10cyc_1FSU1_RC], (instregex "FDIVv2f64")>;

// ASIMD FP square root, D-form, F16
def : InstRW<[HIP11Write_9cyc_1FSU1_RC], (instrs FSQRTv4f16)>;

// ASIMD FP square root, D-form, F32
def : InstRW<[HIP11Write_9cyc_1FSU1_RC], (instrs FSQRTv2f32)>;

// ASIMD FP square root, Q-form, F16
def : InstRW<[HIP11Write_13cyc_1FSU1_RC], (instrs FSQRTv8f16)>;

// ASIMD FP square root, Q-form, F32
def : InstRW<[HIP11Write_13cyc_1FSU1_RC], (instrs FSQRTv4f32)>;

// ASIMD FP square root, Q-form, F64
def : InstRW<[HIP11Write_15cyc_1FSU1_RC], (instrs FSQRTv2f64)>;

// ASIMD FP max/min, normal, D-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^(FMAX|FMIN)(NM)?(v2f32)")>;

// ASIMD FP max/min, normal, Q-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^(FMAX|FMIN)(NM)?(v4f32|v2f64)")>;

// ASIMD FP max/min, pairwise, D-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^(FMAX|FMIN)(NM)?P(v2f32|v2i32)")>;

// ASIMD FP max/min, pairwise, Q-form
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^(FMAX|FMIN)(NM)?P(v4f32|v2f64|v2i64)")>;

// ASIMD FP max/min, reduce, Half-precision
def : InstRW<[HIP11Write_16cyc_1FSU1], (instregex "^(FMAX|FMIN)(NM)?Vv4i16v", "^(FMAX|FMIN)(NM)?Vv8i16v")>;

// ASIMD FP max/min, reduce, Single-precision
def : InstRW<[HIP11Write_10cyc_1FSU1], (instregex "^(FMAX|FMIN)(NM)?Vv4i32v")>;

// ASIMD FP multiply
def : InstRW<[HIP11Write_3cyc_1FSU], (instregex "^FMULv", "^FMULXv", "^FMULX(16|32|64)")>;

// ASIMD FP multiply accumulate
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^FMLAv", "^FMLSv")>;

// ASIMD FP multiply accumulate long
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^FM(LAL|LSL)(2)?(lane)?(v4f16|v8f16)")>;

// ASIMD FP round, Q-form F16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRINT[AIMNPXZ]v8f16$")>;

// ASIMD FP round, D-form F32 and Q-form F64
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRINT[AIMNPXZ]v2f(32|64)$", "^FRINT(32|64)[XZ]v2f(32|64)$")>;

// ASIMD FP round, D-form F16 and Q-form F32
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRINT[AIMNPXZ]v4f(16|32)$", "^FRINT(32|64)[XZ]v4f32$")>;

// ASIMD FP absolute value
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FABSv")>;

// ASIMD move, integer immed
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^MOVIv", "^MOVID")>;

// ASIMD move, FP immed
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^FMOVv")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^MVNIv")>;

// ASIMD FP negate
def : InstRW<[HIP11Write_1cyc_1FSU], (instregex "^FNEGv")>;

// ASIMD BFloat16 (BF16) instructions
// -----------------------------------------------------------------------------

// ASIMD convert, F32 to BF16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs BFCVTN, BFCVTN2)>;

// ASIMD dot product
def : InstRW<[HIP11Write_7cyc_1FSU], (instregex "^BFDOT(v4bf16|v8bf16)", "^BF16DOTlane")>;

// ASIMD matrix multiply accumulate
def : InstRW<[HIP11Write_12cyc_4FSU], (instrs BFMMLA)>;

// ASIMD multiply accumulate long
def : InstRW<[HIP11Write_4cyc_1FSU], (instrs BFMLALB, BFMLALT, BFMLALBIdx, BFMLALTIdx)>;

// Scalar convert, F32 to BF16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs BFCVT)>;

// ASIMD Miscellaneous
// -----------------------------------------------------------------------------

// Reference for forms in this group
//   D form - v8i8, v4i16, v2i32
//   Q form - v16i8, v8i16, v4i32
//   D form - v1i8, v1i16, v1i32, v1i64
//   Q form - v16i8, v8i16, v4i32, v2i64

// ASIMD reciprocal estimate, D-form
// ASIMD reciprocal estimate, Q-form
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRECPEv", "^URECPEv", "^FRSQRTEv", "^URSQRTEv")>;
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRECPXv")>;

// ASIMD reverse
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^REV16v", "^REV32v", "^REV64v")>;

// ASIMD bitwise insert, Q-form
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^(BIF|BIT|BSEL|BSL)(v16i8|v8i8)")>;

// ASIMD duplicate, element
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^DUP(i8|i16|i32|i64)$")>;

// ASIMD duplicate, gen reg
def : InstRW<[HIP11Write_2cyc_1ALU_1FSU1], (instregex "^DUPv.+gpr")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^DUP(v2i64lane|v2i32lane|v4i32lane|v4i16lane|v8i16lane|v8i8lane|v16i8lane)$")>;

// ASIMD move, saturating
// Integer SIMD shift instruction
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]QXTNv", "^SQXTUNv")>;

// ASIMD reciprocal step, D-form, FZ
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^F(RECP|RSQRT)S(v2f32|v1i32|v2i32|v1i64|32|64)")>;

// ASIMD reciprocal step, Q-form, FZ
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^F(RECP|RSQRT)S(v2f64|v4f32|v4i32)")>;

// ASIMD table lookup, D-form
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TB[LX]v8i8One")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TB[LX]v8i8Two")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TB[LX]v8i8Three")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TB[LX]v8i8Four")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TB[LX]v16i8One")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TB[LX]v16i8Two")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TB[LX]v16i8Three")>;
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TB[LX]v16i8Four")>;

// ASIMD transfer, element to gen reg
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^[SU]MOVv")>;

// ASIMD transfer, gen reg to element
def : InstRW<[HIP11Write_2cyc_1ALU_1FSU1], (instregex "^INSv")>;

// ASIMD unzip/zip, Q-form
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^UZP1v", "^UZP2v", "^ZIP1v", "^ZIP2v")>;

// ASIMD extract
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^EXTv")>;

// ASIMD extract narrow
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^XTNv")>;

// CRC
// -----------------------------------------------------------------------------
def HIP11ReadCRC : SchedReadAdvance<1, [HIP11Write_2cyc_1MDU]>;
def : InstRW<[HIP11Write_2cyc_1MDU, HIP11ReadCRC], (instregex "^CRC32")>;

// ASIMD load instructions
// -----------------------------------------------------------------------------

// ASIMD load, 1 element, one lane, B/H/S
// ASIMD load, 1 element, one lane, D
def : InstRW<[HIP11Write_7cyc_1LdSt_1FSU1],           (instregex "LD1i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP11Write_7cyc_1LdSt_1FSU1], (instregex "LD1i(8|16|32|64)_POST$")>;

// ASIMD load, 1 element, all lanes, D-form, B/H/S
// ASIMD load, 1 element, all lanes, D-form, D
def : InstRW<[HIP11Write_7cyc_1LdSt_1FSU1],           (instregex "LD1Rv(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_7cyc_1LdSt_1FSU1], (instregex "LD1Rv(8b|4h|2s|1d)_POST$")>;

// ASIMD load, 1 element, all lanes, Q-form
def : InstRW<[HIP11Write_7cyc_1LdSt_1FSU1],           (instregex "LD1Rv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_7cyc_1LdSt_1FSU1], (instregex "LD1Rv(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 1 element, multiple, 1 reg, D-form
def : InstRW<[HIP11Write_5cyc_1LdSt],           (instregex "LD1Onev(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_5cyc_1LdSt], (instregex "LD1Onev(8b|4h|2s|1d)_POST$")>;

// ASIMD load, 1 element, multiple, 1 reg, Q-form
def : InstRW<[HIP11Write_5cyc_1LdSt],           (instregex "LD1Onev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_5cyc_1LdSt], (instregex "LD1Onev(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 1 element, multiple, 2 reg, D-form
def : InstRW<[HIP11Write_6cyc_2LdSt],           (instregex "LD1Twov(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_6cyc_2LdSt], (instregex "LD1Twov(8b|4h|2s|1d)_POST$")>;

// ASIMD load, 1 element, multiple, 2 reg, Q-form
def : InstRW<[HIP11Write_6cyc_2LdSt],           (instregex "LD1Twov(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_6cyc_2LdSt], (instregex "LD1Twov(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 1 element, multiple, 3 reg, D-form
def : InstRW<[HIP11Write_7cyc_3LdSt],           (instregex "LD1Threev(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_7cyc_3LdSt], (instregex "LD1Threev(8b|4h|2s|1d)_POST$")>;

// ASIMD load, 1 element, multiple, 3 reg, D-form
def : InstRW<[HIP11Write_7cyc_3LdSt],           (instregex "LD1Threev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_7cyc_3LdSt], (instregex "LD1Threev(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 1 element, multiple, 4 reg, D-form
def : InstRW<[HIP11Write_8cyc_4LdSt],           (instregex "LD1Fourv(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_8cyc_4LdSt], (instregex "LD1Fourv(8b|4h|2s|1d)_POST$")>;

// ASIMD load, 1 element, multiple, 4 reg, Q-form
def : InstRW<[HIP11Write_8cyc_4LdSt],           (instregex "LD1Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_8cyc_4LdSt], (instregex "LD1Fourv(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 2 element, one lane, B/H
// ASIMD load, 2 element, one lane, S
// ASIMD load, 2 element, one lane, D
def : InstRW<[HIP11Write_7cyc_2LdSt_2FSU1],           (instregex "LD2i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP11Write_7cyc_2LdSt_2FSU1], (instregex "LD2i(8|16|32|64)_POST$")>;

// ASIMD load, 2 element, all lanes, D-form, B/H/S
// ASIMD load, 2 element, all lanes, D-form, D
def : InstRW<[HIP11Write_8cyc_2LdSt_2FSU1],           (instregex "LD2Rv(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_8cyc_2LdSt_2FSU1], (instregex "LD2Rv(8b|4h|2s|1d)_POST$")>;

// ASIMD load, 2 element, all lanes, Q-form
def : InstRW<[HIP11Write_8cyc_2LdSt_2FSU1],           (instregex "LD2Rv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_8cyc_2LdSt_2FSU1], (instregex "LD2Rv(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 2 element, multiple, D-form, B/H/S
def : InstRW<[HIP11Write_8cyc_2LdSt_2FSU1],           (instregex "LD2Twov(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP11Write_8cyc_2LdSt_2FSU1], (instregex "LD2Twov(8b|4h|2s)_POST$")>;

// ASIMD load, 2 element, multiple, Q-form, B/H/S
// ASIMD load, 2 element, multiple, Q-form, D
def : InstRW<[HIP11Write_8cyc_2LdSt_2FSU1],           (instregex "LD2Twov(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_8cyc_2LdSt_2FSU1], (instregex "LD2Twov(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 3 element, one lane, B/H
// ASIMD load, 3 element, one lane, S
// ASIMD load, 3 element, one lane, D
def : InstRW<[HIP11Write_9cyc_3LdSt_3FSU1],           (instregex "LD3i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP11Write_9cyc_3LdSt_3FSU1], (instregex "LD3i(8|16|32|64)_POST$")>;

// ASIMD load, 3 element, all lanes, D-form, B/H/S
// ASIMD load, 3 element, all lanes, D-form, D
def : InstRW<[HIP11Write_9cyc_3LdSt_3FSU1],           (instregex "LD3Rv(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_9cyc_3LdSt_3FSU1], (instregex "LD3Rv(8b|4h|2s|1d)_POST$")>;

// ASIMD load, 3 element, all lanes, Q-form, B/H/S
// ASIMD load, 3 element, all lanes, Q-form, D
def : InstRW<[HIP11Write_9cyc_3LdSt_3FSU1],           (instregex "LD3Rv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_9cyc_3LdSt_3FSU1], (instregex "LD3Rv(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 3 element, multiple, D-form, B/H/S
def : InstRW<[HIP11Write_15cyc_6LdSt_6FSU1],           (instregex "LD3Threev(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP11Write_15cyc_6LdSt_6FSU1], (instregex "LD3Threev(8b|4h|2s)_POST$")>;

// ASIMD load, 3 element, multiple, Q-form, B/H/S/D
def : InstRW<[HIP11Write_15cyc_6LdSt_6FSU1],           (instregex "LD3Threev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_15cyc_6LdSt_6FSU1], (instregex "LD3Threev(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 4 element, one lane, B/H
// ASIMD load, 4 element, one lane, S
// ASIMD load, 4 element, one lane, D
def : InstRW<[HIP11Write_10cyc_4LdSt_4FSU1],           (instregex "LD4i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP11Write_10cyc_4LdSt_4FSU1], (instregex "LD4i(8|16|32|64)_POST$")>;

// ASIMD load, 4 element, all lanes, D-form, B/H/S
// ASIMD load, 4 element, all lanes, D-form, D
def : InstRW<[HIP11Write_10cyc_4LdSt_4FSU1],           (instregex "LD4Rv(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_10cyc_4LdSt_4FSU1], (instregex "LD4Rv(8b|4h|2s|1d)_POST$")>;

// ASIMD load, 4 element, all lanes, Q-form, B/H/S
// ASIMD load, 4 element, all lanes, Q-form, D
def : InstRW<[HIP11Write_10cyc_4LdSt_4FSU1],           (instregex "LD4Rv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_10cyc_4LdSt_4FSU1], (instregex "LD4Rv(16b|8h|4s|2d)_POST$")>;

// ASIMD load, 4 element, multiple, D-form, B/H/S
def : InstRW<[HIP11Write_12cyc_4LdSt_4FSU1],           (instregex "LD4Fourv(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP11Write_12cyc_4LdSt_4FSU1], (instregex "LD4Fourv(8b|4h|2s)_POST$")>;

// ASIMD load, 4 element, multiple, Q-form, B/H/S
// ASIMD load, 4 element, multiple, Q-form, D
def : InstRW<[HIP11Write_12cyc_4LdSt_4FSU1],           (instregex "LD4Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_12cyc_4LdSt_4FSU1], (instregex "LD4Fourv(16b|8h|4s|2d)_POST$")>;

// ASIMD Store instructions
// -----------------------------------------------------------------------------

// ASIMD store, 1 element, one lane, B/H/S
// ASIMD store, 1 element, one lane, D
def : InstRW<[HIP11Write_2cyc_1LdSt1_1FSU1_1FSTD],           (instregex "ST1i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP11Write_2cyc_1LdSt1_1FSU1_1FSTD], (instregex "ST1i(8|16|32|64)_POST$")>;

// ASIMD store, 1 element, multiple, 1 reg, D-form
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD],           (instregex "ST1Onev(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "ST1Onev(8b|4h|2s|1d)_POST$")>;

// ASIMD store, 1 element, multiple, 1 reg, Q-form
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD],           (instregex "ST1Onev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "ST1Onev(16b|8h|4s|2d)_POST$")>;

// ASIMD store, 1 element, multiple, 2 reg, D-form
def : InstRW<[HIP11Write_2cyc_2LdSt1_2FSTD],           (instregex "ST1Twov(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_2cyc_2LdSt1_2FSTD], (instregex "ST1Twov(8b|4h|2s|1d)_POST$")>;

// ASIMD store, 1 element, multiple, 2 reg, Q-form
def : InstRW<[HIP11Write_2cyc_2LdSt1_2FSTD],           (instregex "ST1Twov(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_2cyc_2LdSt1_2FSTD], (instregex "ST1Twov(16b|8h|4s|2d)_POST$")>;

// ASIMD store, 1 element, multiple, 3 reg, D-form
def : InstRW<[HIP11Write_3cyc_3LdSt1_3FSTD],           (instregex "ST1Threev(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_3cyc_3LdSt1_3FSTD], (instregex "ST1Threev(8b|4h|2s|1d)_POST$")>;

// ASIMD store, 1 element, multiple, 3 reg, Q-form
def : InstRW<[HIP11Write_3cyc_3LdSt1_3FSTD],           (instregex "ST1Threev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_3cyc_3LdSt1_3FSTD], (instregex "ST1Threev(16b|8h|4s|2d)_POST$")>;

// ASIMD store, 1 element, multiple, 4 reg, D-form
def : InstRW<[HIP11Write_4cyc_4LdSt1_4FSTD],           (instregex "ST1Fourv(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP11Write_4cyc_4LdSt1_4FSTD], (instregex "ST1Fourv(8b|4h|2s|1d)_POST$")>;

// ASIMD store, 1 element, multiple, 4 reg, Q-form
def : InstRW<[HIP11Write_4cyc_4LdSt1_4FSTD],           (instregex "ST1Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_4cyc_4LdSt1_4FSTD], (instregex "ST1Fourv(16b|8h|4s|2d)_POST$")>;

// ASIMD store, 2 element, one lane, B/H/S
// ASIMD store, 2 element, one lane, D
def : InstRW<[HIP11Write_2cyc_1LdSt1_1FSU1_1FSTD],           (instregex "ST2i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP11Write_2cyc_1LdSt1_1FSU1_1FSTD], (instregex "ST2i(8|16|32|64)_POST$")>;

// ASIMD store, 2 element, multiple, D-form, B/H/S
def : InstRW<[HIP11Write_3cyc_3LdSt1_3FSU1_1FSTD],           (instregex "ST2Twov(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP11Write_3cyc_3LdSt1_3FSU1_1FSTD], (instregex "ST2Twov(8b|4h|2s)_POST$")>;

// ASIMD store, 2 element, multiple, Q-form, B/H/S
// ASIMD store, 2 element, multiple, Q-form, D
def : InstRW<[HIP11Write_3cyc_3LdSt1_3FSU1_1FSTD],           (instregex "ST2Twov(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_3cyc_3LdSt1_3FSU1_1FSTD], (instregex "ST2Twov(16b|8h|4s|2d)_POST$")>;

// ASIMD store, 3 element, one lane, B/H
// ASIMD store, 3 element, one lane, S
// ASIMD store, 3 element, one lane, D
def : InstRW<[HIP11Write_4cyc_2LdSt1_2FSU1_1FSTD],           (instregex "ST3i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP11Write_4cyc_2LdSt1_2FSU1_1FSTD], (instregex "ST3i(8|16|32|64)_POST$")>;

// ASIMD store, 3 element, multiple, D-form, B/H/S
def : InstRW<[HIP11Write_16cyc_16LdSt1_1FSU1_16FSTD],           (instregex "ST3Threev(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP11Write_16cyc_16LdSt1_1FSU1_16FSTD], (instregex "ST3Threev(8b|4h|2s)_POST$")>;

// ASIMD store, 3 element, multiple, Q-form, B/H/S
// ASIMD store, 3 element, multiple, Q-form, D
def : InstRW<[HIP11Write_16cyc_16LdSt1_1FSU1_16FSTD],           (instregex "ST3Threev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_16cyc_16LdSt1_1FSU1_16FSTD], (instregex "ST3Threev(16b|8h|4s|2d)_POST$")>;

// ASIMD store, 4 element, one lane, B/H/S
def : InstRW<[HIP11Write_4cyc_2LdSt1_2FSU1_1FSTD],           (instregex "ST4i(8|16|32)$")>;
def : InstRW<[WriteAdr, HIP11Write_4cyc_2LdSt1_2FSU1_1FSTD], (instregex "ST4i(8|16|32)_POST$")>;
// ASIMD store, 4 element, one lane, D
def : InstRW<[HIP11Write_4cyc_2LdSt1_2FSU1_1FSTD],           (instregex "ST4i(64)$")>;
def : InstRW<[WriteAdr, HIP11Write_4cyc_2LdSt1_2FSU1_1FSTD], (instregex "ST4i(64)_POST$")>;

// ASIMD store, 4 element, multiple, D-form, B/H/S
def : InstRW<[HIP11Write_22cyc_22LdSt1_FSU1_22FSTD],           (instregex "ST4Fourv(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP11Write_22cyc_22LdSt1_FSU1_22FSTD], (instregex "ST4Fourv(8b|4h|2s)_POST$")>;
// ASIMD store, 4 element, multiple, Q-form, B/H/S
def : InstRW<[HIP11Write_22cyc_22LdSt1_FSU1_22FSTD],           (instregex "ST4Fourv(16b|8h|4s)$")>;
def : InstRW<[WriteAdr, HIP11Write_22cyc_22LdSt1_FSU1_22FSTD], (instregex "ST4Fourv(16b|8h|4s)_POST$")>;
// ASIMD store, 4 element, multiple, Q-form, D
def : InstRW<[HIP11Write_22cyc_22LdSt1_FSU1_22FSTD],           (instregex "ST4Fourv(2d)$")>;
def : InstRW<[WriteAdr, HIP11Write_22cyc_22LdSt1_FSU1_22FSTD], (instregex "ST4Fourv(2d)_POST$")>;

// SVE Load instructions
// -----------------------------------------------------------------------------

// Load vector
def : InstRW<[HIP11Write_5cyc_1LdSt], (instrs LDR_ZXI)>;

// Load predicate
def : InstRW<[HIP11Write_7cyc_1LdSt_1FSU1], (instrs LDR_PXI)>;

// Contiguous load, scalar + imm
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LD1[BHWD]_IMM_REAL$", "^LD1S?B_[HSD]_IMM_REAL$",
                                                "^LD1S?H_[SD]_IMM_REAL$", "^LD1S?W_D_IMM_REAL$" )>;
// Contiguous load, scalar + scalar
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LD1[BHWD]$", "^LD1S?B_[HSD]$", "^LD1S?H_[SD]$", "^LD1S?W_D$" )>;

// Contiguous load broadcast, scalar + imm
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LD1R[BHWD]_IMM$", "^LD1RSW_IMM$", "^LD1RS?B_[HSD]_IMM$", "^LD1RS?H_[SD]_IMM$",
                                          "^LD1RS?W_D_IMM$", "^LD1RQ_[BHWD]_IMM$")>;

// Contiguous load broadcast, scalar + scalar
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LD1RQ_[BHWD]$")>;

// Non temporal load, scalar + imm
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LDNT1[BHWD]_ZRI$")>;

// Non temporal load, scalar + scalar
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LDNT1[BHWD]_ZRR$")>;
// Non temporal gather load, vector + scalar 32-bit element size
def : InstRW<[HIP11Write_14cyc_8LdSt_8FSTD], (instregex "^LDNT1[BHW]_ZZR_S_REAL$", "^LDNT1S[BH]_ZZR_S_REAL$")>;
// Non temporal gather load, vector + scalar 64-bit element size
def : InstRW<[HIP11Write_10cyc_4LdSt_4FSTD], (instregex "^LDNT1S?[BHW]_ZZR_D_REAL$")>;
def : InstRW<[HIP11Write_10cyc_4LdSt_4FSTD], (instrs LDNT1D_ZZR_D_REAL)>;

// Contiguous first faulting load, scalar + scalar
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LDFF1[BHWD]_REAL$", "^LDFF1S?B_[HSD]_REAL$",
                                                "^LDFF1S?H_[SD]_REAL$", "^LDFF1S?W_D_REAL$")>;

// Contiguous non faulting load, scalar + imm
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^LDNF1[BHWD]_IMM_REAL$", "^LDNF1S?B_[HSD]_IMM_REAL$",
                                          "^LDNF1S?H_[SD]_IMM_REAL$", "^LDNF1S?W_D_IMM_REAL$")>;

// Gather load, vector + imm, 32-bit element size
def : InstRW<[HIP11Write_14cyc_8LdSt_8FSTD], (instregex "^GLD(FF)?1S?[BH]_S_IMM_REAL$",
                                                  "^GLD(FF)?1W_IMM_REAL$")>;

// Gather load, vector + imm, 64-bit element size
def : InstRW<[HIP11Write_10cyc_4LdSt_4FSTD], (instregex "^GLD(FF)?1S?[BHW]_D_IMM_REAL$",
                                                 "^GLD(FF)?1D_IMM_REAL$")>;

// Gather load, 64-bit element size
def : InstRW<[HIP11Write_10cyc_4LdSt_4FSTD], (instregex "^GLD(FF)?1S?[BHW]_D_[SU]XTW(_SCALED)?_REAL$", "^GLD(FF)?1S?[BHW]_D(_SCALED)?_REAL$",
                                                 "^GLD(FF)?1D_[SU]XTW(_SCALED)?_REAL$", "^GLD(FF)?1D(_SCALED)?_REAL$")>;

// Gather load, 32-bit scaled offset
def : InstRW<[HIP11Write_14cyc_8LdSt_8FSTD], (instregex "^GLD(FF)?1S?[HW]_S_[SU]XTW_SCALED_REAL$", "^GLD(FF)?1W_[SU]XTW_SCALED_REAL")>;

// Gather load, 32-bit unpacked unscaled offset
def : InstRW<[HIP11Write_14cyc_8LdSt_8FSTD], (instregex "^GLD(FF)?1S?[BH]_S_[SU]XTW_REAL$", "^GLD(FF)?1W_[SU]XTW_REAL$")>;

// Contiguous Load two structures to two vectors, scalar + imm
def : InstRW<[HIP11Write_14cyc_2LdSt_2FSU1], (instregex "^LD2[BHWD]_IMM$")>;

// Contiguous Load two structures to two vectors, scalar + scalar
def : InstRW<[HIP11Write_14cyc_2LdSt_2FSU1_1MDU], (instregex "^LD2[BHWD]$")>;

// Contiguous Load three structures to three vectors, scalar + imm
def : InstRW<[HIP11Write_27cyc_6LdSt_6FSU1], (instregex "^LD3[BHWD]_IMM$")>;

// Contiguous Load three structures to three vectors, scalar + scalar
def : InstRW<[HIP11Write_27cyc_6LdSt_6FSU1_1MDU], (instregex "^LD3[BHWD]$")>;

// Contiguous Load four structures to four vectors, scalar + imm
def : InstRW<[HIP11Write_33cyc_8LdSt_8FSU1], (instregex "^LD4[BHWD]_IMM$")>;

// Contiguous Load four structures to four vectors, scalar + scalar
def : InstRW<[HIP11Write_33cyc_8LdSt_8FSU1_1MDU], (instregex "^LD4[BHWD]$")>;

// SVE Store instructions
// -----------------------------------------------------------------------------

// Store from predicate reg
def : InstRW<[HIP11Write_2cyc_1LdSt1_1FSTD], (instrs STR_PXI)>;

// Store from vector reg
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSU1_1FSTD], (instrs STR_ZXI)>;

// Contiguous store, scalar + imm
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "^ST1[BHWD]_IMM$", "^ST1B_[HSD]_IMM$", "^ST1H_[SD]_IMM$", "^ST1W_D_IMM$")>;

// Contiguous store, scalar + scalar
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "^ST1H(_[SD])?$")>;
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "^ST1[BWD]$", "^ST1B_[HSD]$", "^ST1W_D$")>;

// Contiguous store two structures from two vectors, scalar + imm
def : InstRW<[HIP11Write_9cyc_2LdSt1_1FSU1_2FSTD], (instregex "^ST2[BHWD]_IMM$")>;

// Contiguous store two structures from two vectors, scalar + scalar
def : InstRW<[HIP11Write_9cyc_2LdSt1_1FSU1_2FSTD_1MDU], (instrs ST2H)>;

// Contiguous store two structures from two vectors, scalar + scalar
def : InstRW<[HIP11Write_9cyc_2LdSt1_1FSU1_2FSTD_1MDU], (instregex "^ST2[BWD]$")>;

// Contiguous store three structures from three vectors, scalar + imm
def : InstRW<[HIP11Write_23cyc_6LdSt1_1FSU1_6FSTD], (instregex "^ST3[BHWD]_IMM$")>;

// Contiguous store three structures from three vectors, scalar + scalar
def : InstRW<[HIP11Write_23cyc_6LdSt1_1FSU1_6FSTD_1MDU], (instrs ST3H)>;

// Contiguous store three structures from three vectors, scalar + scalar
def : InstRW<[HIP11Write_23cyc_6LdSt1_1FSU1_6FSTD_1MDU], (instregex "^ST3[BWD]$")>;

// Contiguous store four structures from four vectors, scalar + imm
def : InstRW<[HIP11Write_30cyc_8LdSt1_1FSU1_8FSTD], (instregex "^ST4[BHWD]_IMM$")>;

// Contiguous store four structures from four vectors, scalar + scalar
def : InstRW<[HIP11Write_30cyc_8LdSt1_1FSU1_8FSTD_1MDU], (instrs ST4H)>;

// Contiguous store four structures from four vectors, scalar + scalar
def : InstRW<[HIP11Write_30cyc_8LdSt1_1FSU1_8FSTD_1MDU], (instregex "^ST4[BWD]$")>;

// Non temporal store, scalar + imm
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "^STNT1[BHWD]_ZRI$")>;

// Non temporal store, scalar + scalar
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD], (instrs STNT1H_ZRR)>;
def : InstRW<[HIP11Write_1cyc_1LdSt1_1FSTD], (instregex "^STNT1[BWD]_ZRR$")>;

// Scatter non temporal store, vector + scalar 32-bit element size
def : InstRW<[HIP11Write_18cyc_16LdSt1_16FSTD], (instregex "^STNT1[BHW]_ZZR_S")>;

// Scatter non temporal store, vector + scalar 64-bit element size
def : InstRW<[HIP11Write_10cyc_8LdSt1_8FSTD], (instregex "^STNT1[BHWD]_ZZR_D")>;

// Scatter store vector + imm 32-bit element size
def : InstRW<[HIP11Write_18cyc_16LdSt1_16FSTD], (instregex "^SST1[BH]_S_IMM$", "^SST1W_IMM$")>;

// Scatter store vector + imm 64-bit element size
def : InstRW<[HIP11Write_10cyc_8LdSt1_8FSTD], (instregex "^SST1[BHW]_D_IMM$", "^SST1D_IMM$")>;

// Scatter store, 32-bit scaled offset
def : InstRW<[HIP11Write_18cyc_16LdSt1_16FSTD], (instregex "^SST1(H_S|W)_[SU]XTW_SCALED$")>;

// Scatter store, 32-bit unpacked unscaled offset
def : InstRW<[HIP11Write_18cyc_16LdSt1_16FSTD], (instregex "^SST1[BHW]_D_[SU]XTW$")>;
def : InstRW<[HIP11Write_10cyc_8LdSt1_8FSTD], (instregex "^SST1D_[SU]XTW$")>;

// Scatter store, 32-bit unpacked scaled offset
def : InstRW<[HIP11Write_18cyc_16LdSt1_16FSTD], (instregex "^SST1[HW]_D_[SU]XTW_SCALED$")>;
def : InstRW<[HIP11Write_10cyc_8LdSt1_8FSTD], (instregex "^SST1D_[SU]XTW_SCALED$")>;

// Scatter store, 32-bit unscaled offset
def : InstRW<[HIP11Write_18cyc_16LdSt1_16FSTD], (instregex "^SST1[BH]_S_[SU]XTW$", "^SST1W_[SU]XTW$")>;

// Scatter store, 64-bit scaled offset
def : InstRW<[HIP11Write_18cyc_16LdSt1_16FSTD], (instregex "^SST1[HW]_D_SCALED")>;
def : InstRW<[HIP11Write_10cyc_8LdSt1_8FSTD], (instregex "^SST1D_SCALED", "^SST1[BHW]_D$", "^SST1D$")>;

// SVE Predicate instructions
// -----------------------------------------------------------------------------

// Loop control, based on predicate
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs BRKA_PPmP, BRKA_PPzP, BRKB_PPmP, BRKB_PPzP)>;

// Loop control, based on predicate and flag setting
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs BRKAS_PPzP, BRKBS_PPzP)>;

// Loop control, propagating
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs BRKN_PPzP, BRKPA_PPzPP, BRKPB_PPzPP)>;

// Loop control, propagating and flag setting
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs BRKNS_PPzP, BRKPAS_PPzPP, BRKPBS_PPzPP)>;

// Loop control, based on GPR
def : InstRW<[HIP11Write_2cyc_1MDU], (instregex "^WHILE(GE|GT|HI|HS|LE|LO|LS|LT)_P(WW|XX)_[BHSD]$")>;
def : InstRW<[HIP11Write_2cyc_1MDU], (instregex "^WHILE(RW|WR)_PXX_[BHSD]$")>;

// Loop terminate
def : InstRW<[HIP11Write_1cyc_1BRU], (instregex "^CTERM(EQ|NE)_(WW|XX)$")>;

// Predicate counting scalar
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instrs ADDPL_XXI, ADDVL_XXI)>;
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instrs RDVLI_XI)>;
def : InstRW<[HIP11Write_1cyc_1AnyALU], (instregex "^(CNT|DEC|INC|SQDEC|SQINC|UQDEC|UQINC)[BHWD]_XPiI$",
                                          "^SQ(DEC|INC)[BHWD]_XPiWdI$", "^(UQDEC|UQINC)[BHWD]_WPiI$")>;

// Predicate counting scalar, active predicate
def : InstRW<[HIP11Write_3cyc_2FSU1], (instregex "^CNTP_XCI_[BHSD]$", "^CNTP_XPP_[BHSD]$")>;
def : InstRW<[HIP11Write_5cyc_1AnyALU_1FSU1], (instregex "^(DEC|INC)P_XP_[BHSD]$")>;
def : InstRW<[HIP11Write_8cyc_1AnyALU_1FSU1], (instregex "^(SQDEC|SQINC)P_XPWd_[BHSD]$")>;
def : InstRW<[HIP11Write_7cyc_1AnyALU_1FSU1], (instregex "^(SQDEC|SQINC)P_XP_[BHSD]$")>;
def : InstRW<[HIP11Write_7cyc_1AnyALU_1FSU1], (instregex "^UQDECP_WP_[BHSD]$")>;
def : InstRW<[HIP11Write_6cyc_1AnyALU_1FSU1], (instregex "^UQDECP_XP_[BHSD]$")>;
def : InstRW<[HIP11Write_8cyc_1AnyALU_1FSU1], (instregex "^UQINCP_WP_[BHSD]$")>;
def : InstRW<[HIP11Write_6cyc_1AnyALU_1FSU1], (instregex "^UQINCP_XP_[BHSD]$")>;

// Predicate counting vector, active predicate
def : InstRW<[HIP11Write_6cyc_3FSU1], (instregex "^(DEC|INC|SQDEC|SQINC|UQDEC|UQINC)P_ZP_[HSD]$")>;

// Predicate logical
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(AND|BIC|EOR|NAND|NOR|ORN|ORR)_PPzPP$")>;

// Predicate logical, flag setting
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(ANDS|BICS|EORS|NANDS|NORS|ORNS|ORRS)_PPzPP$")>;

// Predicate reverse
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^REV_PP_[BHSD]$")>;

// Predicate select
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs SEL_PPPP)>;

// Predicate set
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^PFALSE$", "^PTRUE_[BHSD]$")>;

// Predicate set/initialize, set flags
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^PTRUES_[BHSD]$")>;

// Predicate find first/next
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^PFIRST_B$", "^PNEXT_[BHSD]$")>;

// Predicate test
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs PTEST_PP)>;

// Predicate transpose
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^TRN[12]_PPP_[BHSDQ]$")>;

// Predicate unpack and widen
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs PUNPKHI_PP, PUNPKLO_PP)>;

// Predicate zip/unzip
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(ZIP|UZP)[12]_PPP_[BHSDQ]$")>;

// SVE integer instructions
// -----------------------------------------------------------------------------

// Arithmetic, absolute diff
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]ABD_ZPmZ_[BHSD]$")>;

// Arithmetic, absolute diff accum
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]ABA_ZZZ_[BHSD]$")>;

// Arithmetic, absolute diff accum long
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]ABAL[TB]_ZZZ_[HSD]$")>;

// Arithmetic, absolute diff long
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]ABDL[TB]_ZZZ_[HSD]$")>;

// Arithmetic, basic
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(ABS|ADD|CNOT|NEG)_ZPmZ_[BHSD]$", "^ADD_ZZZ_[BHSD]$",
                                          "^ADD_ZI_[BHSD]$", "^ADR_[SU]XTW_ZZZ_D_[0123]$", "^ADR_LSL_ZZZ_[SD]_[0123]$",
                                          "^SSUB[LW][BT]_ZZZ_[HSD]$", "^SSUBL(BT|TB)_ZZZ_[HSD]$")>;

def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(SUB|SUBR)_ZPmZ_[BHSD]$", "^(SUB|SUBR)_ZI_[BHSD]$", "^SUB_ZZZ_[BHSD]$")>;

def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^[SU]ADD[LW][BT]_ZZZ_[HSD]$", "^SADDLBT_ZZZ_[HSD]$", "^[SU]H(ADD|SUB|SUBR)_ZPmZ_[BHSD]$",
                                          "^(UQSUB|UQSUBR)_ZPmZ_[BHSD]$", "^(USQ|URH)ADD_ZPmZ_[BHSD]$", "^USUB[LW][BT]_ZZZ_[HSD]$",
                                          "^R?SUBHN[BT]_ZZZ_[BHS]$")>;

// Arithmetic, complex
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^RADDHNB_ZZZ_[BHS]$")>;
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^ADDHNB_ZZZ_[BHS]$", "^SQ(ABS|ADD|NEG|SUB|SUBR)_ZPmZ_[BHSD]$", "^[SU]QADD_(ZZZ|ZI)_[BHSD]$",
                                          "^SQSUB_(ZZZ|ZI)_[BHSD]$", "^(SRH|SUQ|UQ)ADD_ZPmZ_[BHSD]$")>;
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^RADDHNT_ZZZ_[BHS]$", "^UQSUB_(ZZZ|ZI)_[BHSD]$")>;
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^ADDHNT_ZZZ_[BHS]$")>;

def : InstRW<[HIP11Write_11cyc_6FSU1], (instregex "^SADDV_VPZ_[BHS]$", "^UADDV_VPZ_[BHS]$")>;

def : InstRW<[HIP11Write_10cyc_5FSU1], (instregex "^UADDV_VPZ_D$")>;

// Arithmetic, large integer
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(AD|SB)CL[BT]_ZZZ_[SD]$")>;

// Arithmetic, pairwise add
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^ADDP_ZPmZ_[BHSD]$")>;

// Arithmetic, pairwise add and accum long
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]ADALP_ZPmZ_[HSD]$")>;

// Arithmetic, shift
def : InstRW<[HIP11Write_3cyc_2FSU1], (instregex "^(ASR|LSL|LSR)_WIDE_ZPmZ_[BHS]$", "^(ASR|LSL|LSR)_WIDE_ZZZ_[BHS]$", "^(ASR|LSL|LSR)_ZPmI_[BHSD]$",
                                          "^(ASR|LSL|LSR)_ZPmZ_[BHSD]$", "^(ASR|LSL|LSR)_ZZI_[BHSD]$", "^(ASRR|LSLR|LSRR)_ZPmZ_[BHSD]$")>;

// Arithmetic, shift and accumulate
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^(SRSRA|SSRA|URSRA|USRA)_ZZI_[BHSD]$")>;

// Arithmetic, shift by immediate
// Arithmetic, shift by immediate and insert
def : InstRW<[HIP11Write_3cyc_2FSU1], (instregex "^(SHRNB|SHRNT|SSHLLB|SSHLLT|USHLLB|USHLLT|SLI|SRI)_ZZI_[BHSD]$")>;

// Arithmetic, shift complex
def : InstRW<[HIP11Write_4cyc_2FSU1], (instregex "^(SQ)?RSHRU?N[BT]_ZZI_[BHS]$", "^(SQRSHL|SQRSHLR|UQRSHL|UQRSHLR)_ZPmZ_[BHSD]$",
                                           "^UQRSHRN[BT]_ZZI_[BHS]$")>;
def : InstRW<[HIP11Write_3cyc_2FSU1], (instregex "^(SQSHL|SQSHLR|UQSHL|UQSHLR)_ZPmZ_[BHSD]$", "^(UQSHL|SQSHL|SQSHLU)_ZPmI_[BHSD]",
                                           "^SQSHRU?N[BT]_ZZI_[BHS]$", "UQSHRN[BT]_ZZI_[BHS]$")>;

// Arithmetic, shift right for divide
def : InstRW<[HIP11Write_4cyc_2FSU1], (instregex "^ASRD_ZPmI_[BHSD]$")>;

// Arithmetic, shift rounding
def : InstRW<[HIP11Write_4cyc_2FSU1], (instregex "^(SRSHL|SRSHLR|URSHL|URSHLR)_ZPmZ_[BHSD]$", "^[SU]RSHR_ZPmI_[BHSD]$")>;

// Bit manipulation
def : InstRW<[HIP11Write_4cyc_2FSU1], (instregex "^(BDEP|BEXT)_ZZZ_[BHSD]$")>;

def : InstRW<[HIP11Write_9cyc_1FSU1], (instregex "^BGRP_ZZZ_[BHSD]$")>;

// Bitwise select
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(BSL|BSL1N|BSL2N|NBSL)_ZZZZ$")>;

// Count/reverse bits
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(CLS|CLZ|CNT|RBIT)_ZPmZ_[BHSD]$")>;

// Broadcast logical bitmask immediate to vector
def : InstRW<[HIP11Write_1cyc_1FSU1], (instrs DUPM_ZI)>;

// Compare and set flags
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^CMP(EQ|GE|GT|HI|HS|LE|LO|LS|LT|NE)_PPzZ[IZ]_[BHSD]$",
                                          "^CMP(EQ|GE|GT|HI|HS|LE|LO|LS|LT|NE)_WIDE_PPzZZ_[BHS]$")>;

// Complex add
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(SQ)?CADD_ZZI_[BHSD]$")>;

// Complex dot product 8-bit element
def : InstRW<[HIP11Write_3cyc_1FSU], (instrs CDOT_ZZZ_S, CDOT_ZZZI_S)>;

// Complex dot product 16-bit element
def : InstRW<[HIP11Write_3cyc_1FSU], (instrs CDOT_ZZZ_D, CDOT_ZZZI_D)>;

// Complex multiply-add B, H, S element size
def : InstRW<[HIP11Write_3cyc_1FSU], (instregex "^CMLA_ZZZ_[BHS]$", "^CMLA_ZZZI_[HS]$")>;

// Complex multiply-add D element size
def : InstRW<[HIP11Write_3cyc_1FSU], (instrs CMLA_ZZZ_D)>;

// Conditional extract operations, scalar form
def : InstRW<[HIP11Write_4cyc_1FSU1], (instregex "^CLAST[AB]_RPZ_[BHSD]$")>;

// Conditional extract operations, SIMD&FP scalar and vector forms
def : InstRW<[HIP11Write_7cyc_1FSU1], (instregex "^SPLICE_ZPZZ?_[BHSD]$")>;
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^COMPACT_ZPZ_[SD]$")>;
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^CLAST[AB]_[VZ]PZ_[BHSD]$")>;

// Convert to floating point, 64b to float or convert to double
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]CVTF_ZPmZ_Dto[SD]$")>;

// Convert to floating point, 64b to half
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]CVTF_ZPmZ_DtoH$")>;

// Convert to floating point, 32b to single or half
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]CVTF_ZPmZ_Sto[HS]$")>;

// Convert to floating point, 32b to double
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]CVTF_ZPmZ_StoD$")>;

// Convert to floating point, 16b to half
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^[SU]CVTF_ZPmZ_HtoH$")>;

// Copy, scalar
def : InstRW<[HIP11Write_4cyc_1ALU_1FSU1], (instregex "^CPY_ZPmR_[BHSD]$")>;

// Copy, scalar SIMD&FP or imm
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^CPY_ZPm[IV]_[BHSD]$", "^CPY_ZPzI_[BHSD]$")>;

// Divides, 32 bit
def : InstRW<[HIP11Write_11cyc_1FSU1_RC], (instregex "^[SU]DIVR?_ZPmZ_S$")>;

// Divides, 64 bit
def : InstRW<[HIP11Write_11cyc_1FSU1_RC], (instregex "^[SU]DIVR?_ZPmZ_D$")>;

// Dot product, 8 bit
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]DOT_ZZZI?_S$")>;

// Dot product, 8 bit, using signed and unsigned integers
def : InstRW<[HIP11Write_3cyc_1FSU1], (instrs SUDOT_ZZZI, USDOT_ZZZI, USDOT_ZZZ)>;

// Dot product, 16 bit
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]DOT_ZZZI?_D$")>;

// Duplicate, immediate and indexed form
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^DUP_ZI_[BHSD]$", "^DUP_ZZI_[BHSDQ]$")>;

// Duplicate, scalar form
def : InstRW<[HIP11Write_4cyc_1ALU_1FSU1], (instregex "^DUP_ZR_[BHSD]$")>;

// Extend, sign or zero
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^[SU]XTB_ZPmZ_[HSD]$", "^[SU]XTH_ZPmZ_[SD]$", "^[SU]XTW_ZPmZ_[D]$")>;

// Extract
def : InstRW<[HIP11Write_3cyc_1FSU1], (instrs EXT_ZZI, EXT_ZZI_B)>;

// Extract narrow saturating
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]QXTN[BT]_ZZ_[BHS]$", "^SQXTUN[BT]_ZZ_[BHS]$")>;

// Extract/insert operation, SIMD and FP scalar form
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex  "^INSR_ZV_[BHSD]$")>;
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^LAST[AB]_VPZ_[BHSD]$")>;

// Extract/insert operation, scalar
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^INSR_ZR_[BHSD]$")>;
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^LAST[AB]_RPZ_[BHSD]$")>;

// Histogram operations
def : InstRW<[HIP11Write_46cyc_1FSU1], (instrs HISTCNT_ZPzZZ_D)>;
def : InstRW<[HIP11Write_94cyc_1FSU1], (instrs HISTCNT_ZPzZZ_S)>;
def : InstRW<[HIP11Write_76cyc_1FSU1], (instrs HISTSEG_ZZZ)>;

// Horizontal operations, B, H, S, D form, immediate operands only
def : InstRW<[HIP11Write_4cyc_1FSU1], (instregex "^INDEX_II_[BHSD]$")>;

// Horizontal operations, B, H, S, D form, scalar, immediate operands /
// immediate, scalar operands
def : InstRW<[HIP11Write_8cyc_1ALU_1FSU1], (instregex "^INDEX_(IR|RI)_[BHSD]$")>;

// Horizontal operations, B, H, S, D form, scalar operands only
def : InstRW<[HIP11Write_12cyc_1ALU_1FSU1], (instregex "^INDEX_RR_[BHSD]$")>;

// Logical
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(AND|EOR|ORR)_ZI$", "^(AND|BIC|EOR|EOR(BT|TB)?|ORR)_ZZZ$",
                                          "^EOR(BT|TB)_ZZZ_[BHSD]$", "^(AND|BIC|EOR|NOT|ORR)_ZPmZ_[BHSD]$")>;

// Max/min, basic and pairwise
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^[SU](MAX|MIN)_ZI_[BHSD]$", "^[SU](MAX|MIN)P?_ZPmZ_[BHSD]$")>;

// Matching operations
def : InstRW<[HIP11Write_31cyc_1FSU1_RC], (instregex "^N?MATCH_PPzZZ_B$")>;
def : InstRW<[HIP11Write_15cyc_1FSU1_RC], (instregex "^N?MATCH_PPzZZ_H$")>;

// Matrix multiply-accumulate
def : InstRW<[HIP11Write_7cyc_1FSU1], (instrs SMMLA_ZZZ, UMMLA_ZZZ, USMMLA_ZZZ)>;

// Move prefix
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^MOVPRFX_ZP[mz]Z_[BHSD]$", "^MOVPRFX_ZZ$")>;

// Multiply, B, H, S element size
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^MUL_(ZI|ZPmZ|ZZZI|ZZZ)_[BHS]$", "^[SU]MULH_(ZPmZ|ZZZ)_[BHS]$")>;

// Multiply, D element size
def : InstRW<[HIP11Write_4cyc_2FSU1], (instregex "^MUL_(ZI|ZPmZ|ZZZI|ZZZ)_D$", "^[SU]MULH_(ZPmZ|ZZZ)_D$")>;

// Multiply long
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]MULL[BT]_ZZZI_[SD]$", "^[SU]MULL[BT]_ZZZ_[HSD]$")>;

// Multiply accumulate, B, H, S element size
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^ML[AS]_ZZZI_[BHS]$", "^(ML[AS]|MAD|MSB)_ZPmZZ_[BHS]$")>;

// Multiply accumulate, D element size
def : InstRW<[HIP11Write_4cyc_2FSU1], (instregex "^ML[AS]_ZZZI_D$", "^(ML[AS]|MAD|MSB)_ZPmZZ_D$")>;

// Multiply accumulate long
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]ML[AS]L[BT]_ZZZ_[HSD]$", "^[SU]ML[AS]L[BT]_ZZZI_[SD]$")>;

// Multiply accumulate saturating doubling long regular
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SQDML[AS](LB|LT|LBT)_ZZZ_[HSD]$", "^SQDML[AS](LB|LT)_ZZZI_[SD]$")>;

// Multiply saturating doubling high, B, H, S element size
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SQDMULH_ZZZ_[BHS]$", "^SQDMULH_ZZZI_[HS]$")>;

// Multiply saturating doubling high, D element size
def : InstRW<[HIP11Write_3cyc_1FSU1], (instrs SQDMULH_ZZZ_D, SQDMULH_ZZZI_D)>;

// Multiply saturating doubling long
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SQDMULL[BT]_ZZZ_[HSD]$", "^SQDMULL[BT]_ZZZI_[SD]$")>;

// Multiply saturating rounding doubling regular/complex accumulate, B, H, S
// element size
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SQRDML[AS]H_ZZZ_[BHS]$", "^SQRDCMLAH_ZZZ_[BHS]$",
                                         "^SQRDML[AS]H_ZZZI_[HS]$", "^SQRDCMLAH_ZZZI_[HS]$")>;

// Multiply saturating rounding doubling regular/complex accumulate, D element
// size
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SQRDML[AS]H_ZZZI?_D$", "^SQRDCMLAH_ZZZ_D$")>;

// Multiply saturating rounding doubling regular/complex, B, H, S element size
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SQRDMULH_ZZZ_[BHS]$", "^SQRDMULH_ZZZI_[HS]$")>;

// Multiply saturating rounding doubling regular/complex, D element size
def : InstRW<[HIP11Write_4cyc_2FSU1], (instregex "^SQRDMULH_ZZZI?_D$")>;

// Multiply/multiply long, (8x8) polynomial
def : InstRW<[HIP11Write_3cyc_2FSU1], (instregex "^PMUL_ZZZ_B$")>;
def : InstRW<[HIP11Write_3cyc_2FSU1], (instregex "^PMULL[BT]_ZZZ_[HDQ]$")>;

// Predicate counting vector
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(DEC|INC|SQDEC|SQINC|UQDEC|UQINC)[HWD]_ZPiI$")>;

// Reciprocal estimate
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs URECPE_ZPmZ_S, URSQRTE_ZPmZ_S)>;

// Reduction, arithmetic, B form
def : InstRW<[HIP11Write_10cyc_5FSU1], (instregex "^[SU](MAX|MIN)V_VPZ_B")>;

// Reduction, arithmetic, H form
def : InstRW<[HIP11Write_10cyc_5FSU1], (instregex "^[SU](MAX|MIN)V_VPZ_H")>;

// Reduction, arithmetic, S form
def : InstRW<[HIP11Write_10cyc_5FSU1], (instregex "^[SU](MAX|MIN)V_VPZ_S")>;

// Reduction, arithmetic, D form
def : InstRW<[HIP11Write_10cyc_5FSU1], (instregex "^[SU](MAX|MIN)V_VPZ_D")>;

// Reduction, logical
def : InstRW<[HIP11Write_9cyc_5FSU1], (instregex "^(ANDV|EORV|ORV)_VPZ_[BHSD]$")>;

// Reverse, vector
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^REV_ZZ_[BHSD]$", "^REVB_ZPmZ_[HSD]$", "^REVH_ZPmZ_[SD]$", "^REVW_ZPmZ_D$")>;

// Select, vector form
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^SEL_ZPZZ_[BHSD]$")>;

// Table lookup
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^TBL_ZZZZ?_[BHSD]$")>;

// Table lookup extension
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^TBX_ZZZ_[BHSD]$")>;

// Transpose, vector form
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^TRN[12]_ZZZ_[BHSDQ]$")>;

// Unpack and extend
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^[SU]UNPK(HI|LO)_ZZ_[HSD]$")>;

// Zip/unzip
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^(UZP|ZIP)[12]_ZZZ_[BHSDQ]$")>;

def : InstRW<[HIP11Write_4cyc_2FSU1], (instregex "SCLAMP_ZZZ_[BHSD]")>;

// SVE floating-point instructions
// -----------------------------------------------------------------------------

// Floating point absolute value/difference
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FABD_ZPmZ_[HSD]$")>;
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FABS_ZPmZ_[HSD]$")>;

// Floating point arithmetic
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FNEG_ZPmZ_[HSD]$")>;
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^F(ADD|SUB)_(ZPm[IZ]|ZZZ)_[HSD]$", "^FADDP_ZPmZZ_[HSD]$", "^FSUBR_ZPm[IZ]_[HSD]$")>;

// Floating point associative add, F16
def : InstRW<[HIP11Write_96cyc_1FSU], (instrs FADDA_VPZ_H)>;

// Floating point associative add, F32
def : InstRW<[HIP11Write_48cyc_1FSU], (instrs FADDA_VPZ_S)>;

// Floating point associative add, F64
def : InstRW<[HIP11Write_24cyc_1FSU], (instrs FADDA_VPZ_D)>;

// Floating point compare
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FACG[ET]_PPzZZ_[HSD]$")>;
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FCM(EQ|GE|GT|NE)_PPzZ[0Z]_[HSD]$", "^FCM(LE|LT)_PPzZ0_[HSD]$", "^FCMUO_PPzZZ_[HSD]$")>;

// Floating point complex add
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FCADD_ZPmZ_[HSD]$")>;

// Floating point complex multiply add
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^FCMLA_ZPmZZ_[HSD]$", "^FCMLA_ZZZI_[HS]$")>;

// Floating point convert, long or narrow (F16 to F32 or F32 to F16)
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVT_ZPmZ_(HtoS|StoH)$", "^FCVTLT_ZPmZ_HtoS$", "^FCVTNT_ZPmZ_StoH$")>;

// Floating point convert, long or narrow (F16 to F64, F32 to F64, F64 to F32
// or F64 to F16)
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVT_ZPmZ_(HtoD|StoD|DtoS|DtoH)$", "^FCVTLT_ZPmZ_StoD$", "^FCVTNT_ZPmZ_DtoS$")>;

// Floating point convert, round to odd
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs FCVTX_ZPmZ_DtoS, FCVTXNT_ZPmZ_DtoS)>;

// Floating point base2 log, F16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs FLOGB_ZPmZ_H)>;

// Floating point base2 log, F32
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs FLOGB_ZPmZ_S)>;

// Floating point base2 log, F64
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs FLOGB_ZPmZ_D)>;

// Floating point convert to integer, F16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVTZ[SU]_ZPmZ_HtoH$")>;

// Floating point convert to integer, F32
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVTZ[SU]_ZPmZ_(HtoS|StoS)$")>;

// Floating point convert to integer, F64
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FCVTZ[SU]_ZPmZ_(HtoD|StoD|DtoS|DtoD)$")>;

// Floating point copy
def : InstRW<[HIP11Write_3cyc_1FSU1], (instregex "^FCPY_ZPmI_[HSD]$", "^FDUP_ZI_[HSD]$")>;

// Floating point divide, F16
def : InstRW<[HIP11Write_9cyc_1FSU1_RC], (instregex "^FDIVR?_ZPmZ_H$")>;

// Floating point divide, F32
def : InstRW<[HIP11Write_9cyc_1FSU1_RC], (instregex "^FDIVR?_ZPmZ_S$")>;

// Floating point divide, F64
def : InstRW<[HIP11Write_10cyc_1FSU1_RC], (instregex "^FDIVR?_ZPmZ_D$")>;

// Floating point min/max pairwise
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^F(MAX|MIN)(NM)?P_ZPmZZ_[HSD]$")>;

// Floating point min/max
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^F(MAX|MIN)(NM)?_ZPm[IZ]_[HSD]$")>;

// Floating point multiply
def : InstRW<[HIP11Write_3cyc_1FSU], (instregex "^(FSCALE|FMULX)_ZPmZ_[HSD]$", "^FMUL_(ZPm[IZ]|ZZZI?)_[HSD]$")>;

// Floating point multiply accumulate
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^FML[AS]_(ZPmZZ|ZZZI)_[HSD]$",
                                         "^(FMAD|FNMAD|FNML[AS]|FN?MSB)_ZPmZZ_[HSD]$")>;

// Floating point multiply add/sub accumulate long
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^FML[AS]L[BT]_ZZZI?_SHH$")>;

// Floating point reciprocal estimate, F16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs FRECPE_ZZ_H, FRECPX_ZPmZ_H)>;

// Floating point reciprocal estimate, F32
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs FRECPE_ZZ_S, FRECPX_ZPmZ_S)>;

// Floating point reciprocal estimate, F64
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs FRECPE_ZZ_D, FRECPX_ZPmZ_D)>;

def : InstRW<[HIP11Write_3cyc_1FSU1], (instrs FRSQRTE_ZZ_H, FRSQRTE_ZZ_S, FRSQRTE_ZZ_D)>;

// Floating point reciprocal step
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^F(RECPS|RSQRTS)_ZZZ_[HSD]$")>;

// Floating point reduction, F16
def : InstRW<[HIP11Write_16cyc_1FSU], (instregex "^FADDV_VPZ_H$")>;
def : InstRW<[HIP11Write_27cyc_1FSU], (instregex "^(FMAXNMV|FMAXV|FMINNMV|FMINV)_VPZ_H$")>;

// Floating point reduction, F32
def : InstRW<[HIP11Write_15cyc_1FSU], (instregex "^FADDV_VPZ_S$")>;
def : InstRW<[HIP11Write_21cyc_1FSU], (instregex "^(FMAXNMV|FMAXV|FMINNMV|FMINV)_VPZ_S$")>;

// Floating point reduction, F64
def : InstRW<[HIP11Write_18cyc_1FSU], (instregex "^FADDV_VPZ_D$")>;
def : InstRW<[HIP11Write_15cyc_1FSU], (instregex "^(FMAXNMV|FMAXV|FMINNMV|FMINV)_VPZ_D$")>;

// Floating point round to integral, F16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRINT[AIMNPXZ]_ZPmZ_H$")>;

// Floating point round to integral, F32
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRINT[AIMNPXZ]_ZPmZ_S$")>;

// Floating point round to integral, F64
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FRINT[AIMNPXZ]_ZPmZ_D$")>;

// Floating point square root, F16
def : InstRW<[HIP11Write_13cyc_1FSU1_RC], (instrs FSQRT_ZPmZ_H)>;

// Floating point square root, F32
def : InstRW<[HIP11Write_13cyc_1FSU1_RC], (instrs FSQRT_ZPmZ_S)>;

// Floating point square root, F64
def : InstRW<[HIP11Write_15cyc_1FSU1_RC], (instrs FSQRT_ZPmZ_D)>;

// Floating point trigonometric exponentiation
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^FEXPA_ZZ_[HSD]$")>;

// Floating point trigonometric multiply add
def : InstRW<[HIP11Write_4cyc_1FSU], (instregex "^FTMAD_ZZI_[HSD]$")>;

// Floating point trigonometric, miscellaneous
def : InstRW<[HIP11Write_3cyc_1FSU], (instregex "^FTSMUL_ZZZ_[HSD]$")>;
def : InstRW<[HIP11Write_2cyc_1FSU], (instregex "^FTSSEL_ZZZ_[HSD]$")>;

// SVE BFloat16 (BF16) instructions
// -----------------------------------------------------------------------------

// Convert, F32 to BF16
def : InstRW<[HIP11Write_2cyc_1FSU1], (instrs BFCVT_ZPmZ, BFCVTNT_ZPmZ)>;

// Dot product
def : InstRW<[HIP11Write_6cyc_1FSU], (instrs BFDOT_ZZI, BFDOT_ZZZ)>;

// Matrix multiply accumulate
def : InstRW<[HIP11Write_4cyc_1FSU], (instrs BFMLALB_ZZZ, BFMLALT_ZZZ, BFMLALB_ZZZI, BFMLALT_ZZZI)>;
def : InstRW<[HIP11Write_9cyc_1FSU], (instrs BFMMLA_ZZZ)>;

// SVE Miscellaneous instructions
// -----------------------------------------------------------------------------

// Read first fault register, unpredicated
def : InstRW<[HIP11Write_2cyc_1MDU], (instrs RDFFR_P_REAL)>;

// Read first fault register, predicated
def : InstRW<[HIP11Write_2cyc_1MDU], (instrs RDFFR_PPz_REAL)>;

// Read first fault register and set flags
def : InstRW<[HIP11Write_2cyc_1MDU], (instrs RDFFRS_PPz)>;

// Set first fault register
// Write to first fault register
def : InstRW<[HIP11Write_2cyc_1MDU], (instrs SETFFR, WRFFR)>;

// Prefetch
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^PRF[BHWD]_PRI")>;
def : InstRW<[HIP11Write_5cyc_1LdSt], (instregex "^PRF[BHWD]_PRR")>;

def : InstRW<[HIP11Write_14cyc_8LdSt_8FSTD], (instregex "^PRF[BHW]_[SD]", "^PRF[BHW]_[SD]_PZI$", "^PRF[BHW]_D_SCALED$")>;
def : InstRW<[HIP11Write_10cyc_4LdSt_4FSTD], (instregex "^PRFD_[SD]", "^PRFD_[SD]_PZI$", "^PRFD_D_SCALED$")>;

// SVE Cryptographic instructions
// -----------------------------------------------------------------------------

// Crypto AES ops
def : InstRW<[HIP11Write_2cyc_1FSU1], (instregex "^AES[DE]_ZZZ_B$", "^AESI?MC_ZZ_B$")>;

// Crypto SHA3 ops
def : InstRW<[HIP11Write_1cyc_1FSU1], (instregex "^(BCAX|EOR3)_ZZZZ$", "^RAX1_ZZZ_D$")>;
def : InstRW<[HIP11Write_9cyc_4FSU1], (instregex "^XAR_ZZZI_[BHSD]$")>;

// Crypto SM4 ops
def : InstRW<[HIP11Write_4cyc_1FSU1], (instregex "^SM4E(KEY)?_ZZZ_S$")>;

// SME data processing Instructions
// -----------------------------------------------------------------------------

def : InstRW<[HIP11Write_6cyc_1FSU2], (instregex "^ADD[HV]A_MPPZ_[SD]$", "^BFMOP[AS]_MPPZZ")>;

def : InstRW<[HIP11Write_11cyc_1FSU2], (instregex "^FMOP[AS]_MPPZZ_[SD]$", "^FMOP[AS]L_MPPZZ$")>;

def : InstRW<[HIP11Write_6cyc_1FSU2], (instregex "^SMOP[AS]_MPPZZ_[SD]$", "^USMOP[AS]_MPPZZ_[SD]$",
                                          "^UMOP[AS]_MPPZZ_[SD]$", "^SUMOP[AS]_MPPZZ_[SD]$")>;

def : InstRW<[HIP11Write_6cyc_1FSU2], (instregex "^INSERT_MXIPZ")>;

def : InstRW<[HIP11Write_7cyc_1FSU2], (instregex "^EXTRACT_ZPMXI")>;

def : InstRW<[HIP11Write_4cyc_1FSU2], (instregex "^ZERO_M")>;

// SME Load instructions
// -----------------------------------------------------------------------------

def : InstRW<[HIP11Write_9cyc_1FSU2_1LdSt], (instregex "^LD1_MXIPXX")>;

def : InstRW<[HIP11Write_5cyc_1FSU2_1LdSt], (instregex "^LDR_ZA")>;

// SME Load instructions
// -----------------------------------------------------------------------------

def : InstRW<[HIP11Write_10cyc_1FSU2_1LdSt], (instregex "^ST1_MXIPXX")>;

def : InstRW<[HIP11Write_5cyc_1FSU2_1LdSt], (instregex "^STR_ZA")>;

// ----------------------------------------------------------------------------
} // SchedModel = HIP11Model
