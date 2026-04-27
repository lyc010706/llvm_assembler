//=- AArch64SchedHIP10C.td - Huawei HIP10C Scheduling Defs ---*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the machine model for Huawei HIP10C to support instruction
// scheduling and other instruction cost heuristics.
//
//===----------------------------------------------------------------------===//

def HIP10CModel : SchedMachineModel {
  let IssueWidth            =  12; // HIP10C can dispatch 12 micro-ops per cycle.
  let MicroOpBufferSize     = 320; // Based on the reorder buffer.
  let LoadLatency           =   4; // Basic latency for most load instructions.
  let MispredictPenalty     =  16; // Based on ALU pipeline depth.
  let LoopMicroOpBufferSize =  16; // Based on the instruction queue size.
  let CompleteModel         =   0;

  list<Predicate> UnsupportedFeatures = !listconcat(SVE2Unsupported.F,
                                                    SMEUnsupported.F,
                                                    []);
}

let SchedModel = HIP10CModel in {

def HIP10CUnitB     : ProcResource<2>;   // Branch 0/1
def HIP10CUnitS0    : ProcResource<1>;   // Integer Single-Cycle 0
def HIP10CUnitS1    : ProcResource<1>;   // Integer Single-Cycle 1
def HIP10CUnitS23   : ProcResource<2>;   // Integer Single-Cycle 2/3
def HIP10CUnitM0    : ProcResource<1>;   // Integer Multi-Cycle 0
def HIP10CUnitM1    : ProcResource<1>;   // Integer Multi-Cycle 1
def HIP10CUnitLD    : ProcResource<2>;   // Load 0/1
def HIP10CUnitST    : ProcResource<2>;   // Store 0/1
def HIP10CUnitF0    : ProcResource<1>;   // FP/ASIMD/SVE 0
def HIP10CUnitF1    : ProcResource<1>;   // FP/ASIMD/SVE 1
def HIP10CUnitSTD   : ProcResource<2>;   // Store data 0/1

def HIP10CUnitS01   : ProcResGroup<[HIP10CUnitS0, HIP10CUnitS1]>;
def HIP10CUnitS     : ProcResGroup<[HIP10CUnitS0, HIP10CUnitS1, HIP10CUnitS23]>;
def HIP10CUnitM     : ProcResGroup<[HIP10CUnitM0, HIP10CUnitM1]>;
def HIP10CUnitF     : ProcResGroup<[HIP10CUnitF0, HIP10CUnitF1]>;

//
// Contains all of the HIP10C-specific SchedWriteRes types. The approach below
// is to define a generic SchedWriteRes for every combination of latency and
// micro-ops. The naming conventions is to use a prefix, one field for latency,
// and one or more microOp count/type designators.
//
//   Prefix: HIP10CWrite
//   Latency: #cyc
//   Micro-op Count/Types: #(B|S01|S1|S23|S|M0|M1|M|LD|ST|F1|F|STD)
//
// e.g. HIP10CWrite_6c_1S_6LD_4F means the total latency is 6 cycles,
// and 11 micro-ops are issued down 1 S pipe, 6 LD pipes, and 4 F pipes
//
def HIP10CWrite_0c                       : SchedWriteRes<[]>                     { let Latency = 0;  }

let Latency = 1, NumMicroOps = 1 in
def HIP10CWrite_1c_1B : SchedWriteRes<[HIP10CUnitB]> {
}

let Latency = 2, NumMicroOps = 2 in
def HIP10CWrite_2c_1B_1S23 : SchedWriteRes<[HIP10CUnitB, HIP10CUnitS23]> {
}

let Latency = 1, NumMicroOps = 1 in
def HIP10CWrite_1c_1S : SchedWriteRes<[HIP10CUnitS]> {
}

let Latency = 1, NumMicroOps = 1 in
def HIP10CWrite_1c_1S23 : SchedWriteRes<[HIP10CUnitS23]> {
}

let Latency = 2, NumMicroOps = 1 in
def HIP10CWrite_2c_1M : SchedWriteRes<[HIP10CUnitM]> {
}

let Latency = 2, NumMicroOps = 1 in
def HIP10CWrite_2c_1M_MA : SchedWriteRes<[HIP10CUnitM]> {
}

let Latency = 12, NumMicroOps = 1 in
def HIP10CWrite_12c_12M0 : SchedWriteRes<[HIP10CUnitM0]> {
  let ResourceCycles = [12];
}

let Latency = 20, NumMicroOps = 1 in
def HIP10CWrite_20c_20M0 : SchedWriteRes<[HIP10CUnitM0]> {
  let ResourceCycles = [20];
}

let Latency = 3, NumMicroOps = 1 in
def HIP10CWrite_3c_1M : SchedWriteRes<[HIP10CUnitM]> {
}

let Latency = 3, NumMicroOps = 1 in
def HIP10CWrite_3c_1M_MA : SchedWriteRes<[HIP10CUnitM]> {
}

let Latency = 5, NumMicroOps = 1 in
def HIP10CWrite_5c_1M1 : SchedWriteRes<[HIP10CUnitM1]> {
}

let Latency = 6, NumMicroOps = 3 in
def HIP10CWrite_6c_2B_1M1 : SchedWriteRes<[HIP10CUnitB, HIP10CUnitM1]> {
  let ResourceCycles = [2, 1];
}

let Latency = 9, NumMicroOps = 3 in
def HIP10CWrite_9c_2LD_1M1 : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitM1]> {
  let ResourceCycles = [2, 1];
}

let Latency = 1, NumMicroOps = 1 in
def HIP10CWrite_1c_1S1 : SchedWriteRes<[HIP10CUnitS1]> {
}

let Latency = 4, NumMicroOps = 1 in
def HIP10CWrite_4c_1LD : SchedWriteRes<[HIP10CUnitLD]> {
}

let Latency = 1, NumMicroOps = 2 in
def HIP10CWrite_1c_2S : SchedWriteRes<[HIP10CUnitS]> {
  let ResourceCycles = [2];
}

let Latency = 4, NumMicroOps = 3 in
def HIP10CWrite_4c_1LD_2S : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitS]> {
  let ResourceCycles = [1, 2];
}

let Latency = 5, NumMicroOps = 3 in
def HIP10CWrite_5c_1LD_2S : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitS]> {
  let ResourceCycles = [1, 2];
}

let Latency = 1, NumMicroOps = 2 in
def HIP10CWrite_1c_1ST_1M : SchedWriteRes<[HIP10CUnitST, HIP10CUnitM]> {
}

let Latency = 1, NumMicroOps = 4 in
def HIP10CWrite_1c_1ST_2S_1M : SchedWriteRes<[HIP10CUnitST, HIP10CUnitS, HIP10CUnitM]> {
  let ResourceCycles = [1, 2, 1];
}

let Latency = 2, NumMicroOps = 4 in
def HIP10CWrite_2c_1ST_2S_1M : SchedWriteRes<[HIP10CUnitST, HIP10CUnitS, HIP10CUnitM]> {
  let ResourceCycles = [1, 2, 1];
}

let Latency = 1, NumMicroOps = 3 in
def HIP10CWrite_1c_1ST_1STD_1M : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD, HIP10CUnitM]> {
}

let Latency = 1, NumMicroOps = 1 in
def HIP10CWrite_1c_1F : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 2, NumMicroOps = 1 in
def HIP10CWrite_2c_1F : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 4, NumMicroOps = 2 in
def HIP10CWrite_4c_1F_1S23 : SchedWriteRes<[HIP10CUnitF, HIP10CUnitS23]> {
}

let Latency = 7, NumMicroOps = 3 in
def HIP10CWrite_7c_1S01_1F_1S23 : SchedWriteRes<[HIP10CUnitS01, HIP10CUnitF, HIP10CUnitS23]> {
}

let Latency = 4, NumMicroOps = 2 in
def HIP10CWrite_4c_1S01_1F : SchedWriteRes<[HIP10CUnitS01, HIP10CUnitF]> {
}

let Latency = 7, NumMicroOps = 4 in
def HIP10CWrite_7c_4F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [4];
}

let Latency = 10, NumMicroOps = 6 in
def HIP10CWrite_10c_6F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [6];
}

let Latency = 9, NumMicroOps = 5 in
def HIP10CWrite_9c_5F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [5];
}

let Latency = 15, NumMicroOps = 11 in
def HIP10CWrite_15c_11F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [11];
}

let Latency = 15, NumMicroOps = 1 in
def HIP10CWrite_15c_11F_DIV : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [11];
}

let Latency = 4, NumMicroOps = 1 in
def HIP10CWrite_4c_1F : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 4, NumMicroOps = 1 in
def HIP10CWrite_4c_1F_FM : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 4, NumMicroOps = 1 in
def HIP10CWrite_4c_1F_FM_NEON : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 3, NumMicroOps = 1 in
def HIP10CWrite_3c_1F : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 3, NumMicroOps = 1 in
def HIP10CWrite_3c_1F_FM : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 3, NumMicroOps = 1 in
def HIP10CWrite_3c_1F_FM_NEON : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 5, NumMicroOps = 2 in
def HIP10CWrite_5c_1S01_1F : SchedWriteRes<[HIP10CUnitS01, HIP10CUnitF]> {
}

let Latency = 5, NumMicroOps = 2 in
def HIP10CWrite_5c_1F_1S23 : SchedWriteRes<[HIP10CUnitF, HIP10CUnitS23]> {
}

let Latency = 3, NumMicroOps = 1 in
def HIP10CWrite_3c_1S01 : SchedWriteRes<[HIP10CUnitS01]> {
}

let Latency = 6, NumMicroOps = 1 in
def HIP10CWrite_6c_1LD : SchedWriteRes<[HIP10CUnitLD]> {
}

let Latency = 6, NumMicroOps = 3 in
def HIP10CWrite_6c_1LD_2S : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitS]> {
  let ResourceCycles = [1, 2];
}

let Latency = 7, NumMicroOps = 3 in
def HIP10CWrite_7c_2S_1LD : SchedWriteRes<[HIP10CUnitS, HIP10CUnitLD]> {
  let ResourceCycles = [2, 1];
}

let Latency = 1, NumMicroOps = 2 in
def HIP10CWrite_1c_1ST_1STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
}

let Latency = 1, NumMicroOps = 4 in
def HIP10CWrite_1c_1ST_1STD_2S : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD, HIP10CUnitS]> {
  let ResourceCycles = [1, 1, 2];
}

let Latency = 2, NumMicroOps = 4 in
def HIP10CWrite_2c_2S_1ST_1STD : SchedWriteRes<[HIP10CUnitS, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [2, 1, 1];
}

let Latency = 2, NumMicroOps = 4 in
def HIP10CWrite_2c_2ST_2STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [2, 2];
}

let Latency = 2, NumMicroOps = 8 in
def HIP10CWrite_2c_2ST_2STD_4S : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD, HIP10CUnitS]> {
  let ResourceCycles = [2, 2, 4];
}

let Latency = 5, NumMicroOps = 2 in
def HIP10CWrite_5c_2F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [2];
}

let Latency = 2, NumMicroOps = 1 in
def HIP10CWrite_2c_1F1 : SchedWriteRes<[HIP10CUnitF1]> {
}

let Latency = 3, NumMicroOps = 2 in
def HIP10CWrite_3c_2F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [2];
}

let Latency = 9, NumMicroOps = 6 in
def HIP10CWrite_9c_6F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [6];
}

let Latency = 9, NumMicroOps = 1 in
def HIP10CWrite_9c_6F_DIV : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [6];
}

let Latency = 13, NumMicroOps = 9 in
def HIP10CWrite_13c_9F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [9];
}

let Latency = 13, NumMicroOps = 1 in
def HIP10CWrite_13c_9F_DIV : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [9];
}

let Latency = 21, NumMicroOps = 17 in
def HIP10CWrite_21c_17F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [17];
}

let Latency = 25, NumMicroOps = 21 in
def HIP10CWrite_25c_21F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [21];
}

let Latency = 6, NumMicroOps = 2 in
def HIP10CWrite_6c_2F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [2];
}

let Latency = 9, NumMicroOps = 4 in
def HIP10CWrite_9c_4F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [4];
}

let Latency = 5, NumMicroOps = 1 in
def HIP10CWrite_5c_1F : SchedWriteRes<[HIP10CUnitF]> {
}

let Latency = 2, NumMicroOps = 2 in
def HIP10CWrite_2c_2F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [2];
}

let Latency = 3, NumMicroOps = 3 in
def HIP10CWrite_3c_3F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [3];
}

let Latency = 4, NumMicroOps = 4 in
def HIP10CWrite_4c_4F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [4];
}

let Latency = 6, NumMicroOps = 2 in
def HIP10CWrite_6c_2LD : SchedWriteRes<[HIP10CUnitLD]> {
  let ResourceCycles = [2];
}

let Latency = 6, NumMicroOps = 3 in
def HIP10CWrite_6c_3LD : SchedWriteRes<[HIP10CUnitLD]> {
  let ResourceCycles = [3];
}

let Latency = 6, NumMicroOps = 4 in
def HIP10CWrite_6c_4LD : SchedWriteRes<[HIP10CUnitLD]> {
  let ResourceCycles = [4];
}

let Latency = 7, NumMicroOps = 2 in
def HIP10CWrite_7c_1LD_1F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
}

let Latency = 8, NumMicroOps = 4 in
def HIP10CWrite_8c_2LD_2F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [2, 2];
}

let Latency = 9, NumMicroOps = 6 in
def HIP10CWrite_9c_3LD_3F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [3, 3];
}

let Latency = 8, NumMicroOps = 6 in
def HIP10CWrite_8c_3LD_3F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [3, 3];
}

let Latency = 14, NumMicroOps = 16 in
def HIP10CWrite_14c_8LD_8F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [8, 8];
}

let Latency = 9, NumMicroOps = 8 in
def HIP10CWrite_9c_4LD_4F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [4, 4];
}

let Latency = 8, NumMicroOps = 8 in
def HIP10CWrite_8c_4LD_4F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [4, 4];
}

let Latency = 2, NumMicroOps = 2 in
def HIP10CWrite_2c_1ST_1STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
}

let Latency = 3, NumMicroOps = 4 in
def HIP10CWrite_3c_2ST_2STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [2, 2];
}

let Latency = 4, NumMicroOps = 6 in
def HIP10CWrite_4c_3ST_3STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [3, 3];
}

let Latency = 5, NumMicroOps = 8 in
def HIP10CWrite_5c_4ST_4STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [4, 4];
}

let Latency = 1, NumMicroOps = 4 in
def HIP10CWrite_1c_2ST_2STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [2, 2];
}

let Latency = 7, NumMicroOps = 9 in
def HIP10CWrite_7c_3F_3ST_3STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [3, 3, 3];
}

let Latency = 6, NumMicroOps = 6 in
def HIP10CWrite_6c_2F_2ST_2STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [2, 2, 2];
}

let Latency = 4, NumMicroOps = 9 in
def HIP10CWrite_4c_3F_3ST_3STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [3, 3, 3];
}

let Latency = 10, NumMicroOps = 24 in
def HIP10CWrite_10c_8F_8ST_8STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [8, 8, 8];
}

let Latency = 10, NumMicroOps = 18 in
def HIP10CWrite_10c_6F_6ST_6STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [6, 6, 6];
}

let Latency = 4, NumMicroOps = 12 in
def HIP10CWrite_4c_4F_4ST_4STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [4, 4, 4];
}

let Latency = 4, NumMicroOps = 1 in
def HIP10CWrite_4c_1F1 : SchedWriteRes<[HIP10CUnitF1]> {
}

let Latency = 1, NumMicroOps = 1 in
def HIP10CWrite_1c_1F1 : SchedWriteRes<[HIP10CUnitF1]> {
}

let Latency = 1, NumMicroOps = 2 in
def HIP10CWrite_1c_2F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [2];
}

let Latency = 2, NumMicroOps = 1 in
def HIP10CWrite_2c_1S01 : SchedWriteRes<[HIP10CUnitS01]> {
}

let Latency = 2, NumMicroOps = 1 in
def HIP10CWrite_2c_1S23 : SchedWriteRes<[HIP10CUnitS23]> {
}

let Latency = 5, NumMicroOps = 3 in
def HIP10CWrite_5c_1F_2S : SchedWriteRes<[HIP10CUnitF, HIP10CUnitS]> {
  let ResourceCycles = [1, 2];
}

let Latency = 5, NumMicroOps = 6 in
def HIP10CWrite_5c_2F_4S : SchedWriteRes<[HIP10CUnitF, HIP10CUnitS]> {
  let ResourceCycles = [2, 4];
}

let Latency = 6, NumMicroOps = 2 in
def HIP10CWrite_6c_1F_1M : SchedWriteRes<[HIP10CUnitF, HIP10CUnitM]> {
}

let Latency = 6, NumMicroOps = 4 in
def HIP10CWrite_6c_2F_2M : SchedWriteRes<[HIP10CUnitF, HIP10CUnitM]> {
  let ResourceCycles = [2, 2];
}

let Latency = 4, NumMicroOps = 2 in
def HIP10CWrite_4c_2F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [2];
}

let Latency = 5, NumMicroOps = 4 in
def HIP10CWrite_5c_2F_2S23 : SchedWriteRes<[HIP10CUnitF, HIP10CUnitS23]> {
  let ResourceCycles = [2, 2];
}

let Latency = 7, NumMicroOps = 3 in
def HIP10CWrite_7c_3F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [3];
}

let Latency = 7, NumMicroOps = 6 in
def HIP10CWrite_7c_6F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [6];
}

let Latency = 5, NumMicroOps = 4 in
def HIP10CWrite_5c_4F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [4];
}

let Latency = 3, NumMicroOps = 4 in
def HIP10CWrite_3c_4F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [4];
}

let Latency = 5, NumMicroOps = 2 in
def HIP10CWrite_5c_1S23_1F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF ]> {
}

let Latency = 5, NumMicroOps = 4 in
def HIP10CWrite_5c_2S23_2F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
  let ResourceCycles = [2, 2];
}

let Latency = 17, NumMicroOps = 13 in
def HIP10CWrite_17c_13F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [13];
}

let Latency = 17, NumMicroOps = 26 in
def HIP10CWrite_17c_26F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [26];
}

let Latency = 10, NumMicroOps = 4 in
def HIP10CWrite_10c_2S23_2F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
  let ResourceCycles = [2, 2];
}

let Latency = 10, NumMicroOps = 8 in
def HIP10CWrite_10c_4S23_4F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
  let ResourceCycles = [4, 4];
}

let Latency = 11, NumMicroOps = 6 in
def HIP10CWrite_11c_3S23_3F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
  let ResourceCycles = [3, 3];
}

let Latency = 11, NumMicroOps = 12 in
def HIP10CWrite_11c_6S23_6F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
  let ResourceCycles = [6, 6];
}

let Latency = 6, NumMicroOps = 2 in
def HIP10CWrite_6c_1S23_1F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
}

let Latency = 6, NumMicroOps = 4 in
def HIP10CWrite_6c_2S23_2F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
  let ResourceCycles = [2, 2];
}

let Latency = 7, NumMicroOps = 4 in
def HIP10CWrite_7c_2S23_2F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
  let ResourceCycles = [2, 2];
}

let Latency = 7, NumMicroOps = 8 in
def HIP10CWrite_7c_4S23_4F : SchedWriteRes<[HIP10CUnitS23, HIP10CUnitF]> {
  let ResourceCycles = [4, 4];
}

let Latency = 13, NumMicroOps = 18 in
def HIP10CWrite_13c_18F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [18];
}

let Latency = 15, NumMicroOps = 22 in
def HIP10CWrite_15c_22F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [22];
}

let Latency = 7, NumMicroOps = 2 in
def HIP10CWrite_7c_2F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [2];
}

let Latency = 9, NumMicroOps = 8 in
def HIP10CWrite_9c_8F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [8];
}

let Latency = 21, NumMicroOps = 34 in
def HIP10CWrite_21c_34F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [34];
}

let Latency = 25, NumMicroOps = 42 in
def HIP10CWrite_25c_42F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [42];
}

let Latency = 6, NumMicroOps = 4 in
def HIP10CWrite_6c_4F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [4];
}

let Latency = 8, NumMicroOps = 2 in
def HIP10CWrite_8c_1LD_1F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
}

let Latency = 9, NumMicroOps = 4 in
def HIP10CWrite_9c_2LD_2F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [2, 2];
}

let Latency = 11, NumMicroOps = 12 in
def HIP10CWrite_11c_6LD_6F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [6, 6];
}

let Latency = 9, NumMicroOps = 16 in
def HIP10CWrite_9c_8LD_8F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [8, 8];
}

let Latency = 16, NumMicroOps = 32 in
def HIP10CWrite_16c_16LD_16F : SchedWriteRes<[HIP10CUnitLD, HIP10CUnitF]> {
  let ResourceCycles = [16, 16];
}

let Latency = 18, NumMicroOps = 8 in
def HIP10CWrite_18c_8LD : SchedWriteRes<[HIP10CUnitLD]> {
  let ResourceCycles = [8];
}

let Latency = 16, NumMicroOps = 4 in
def HIP10CWrite_16c_4LD : SchedWriteRes<[HIP10CUnitLD]> {
  let ResourceCycles = [4];
}

let Latency = 3, NumMicroOps = 3 in
def HIP10CWrite_3c_1F_1ST_1STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
}

let Latency = 3, NumMicroOps = 6 in
def HIP10CWrite_3c_2F_2ST_2STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [2, 2, 2];
}

let Latency = 6, NumMicroOps = 12 in
def HIP10CWrite_6c_4F_4ST_4STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [4, 4, 4];
}

let Latency = 8, NumMicroOps = 18 in
def HIP10CWrite_8c_6F_6ST_6STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [6, 6, 6];
}

let Latency = 13, NumMicroOps = 48 in
def HIP10CWrite_13c_16F_16ST_16STD : SchedWriteRes<[HIP10CUnitF, HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [16, 16, 16];
}

let Latency = 8, NumMicroOps = 16 in
def HIP10CWrite_8c_8ST_8STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [8, 8];
}

let Latency = 4, NumMicroOps = 8 in
def HIP10CWrite_4c_4ST_4STD : SchedWriteRes<[HIP10CUnitST, HIP10CUnitSTD]> {
  let ResourceCycles = [4, 4];
}

let Latency = 9, NumMicroOps = 27 in
def HIP10CWrite_9c_18F_9M1 : SchedWriteRes<[HIP10CUnitF, HIP10CUnitM1]> {
  let ResourceCycles = [18, 9];
}

let Latency = 10, NumMicroOps = 18 in
def HIP10CWrite_10c_18F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [18];
}

let Latency = 6, NumMicroOps = 10 in
def HIP10CWrite_6c_10F : SchedWriteRes<[HIP10CUnitF]> {
  let ResourceCycles = [10];
}

def : ReadAdvance<ReadI,       0>;
def : ReadAdvance<ReadISReg,   0>;
def : ReadAdvance<ReadIEReg,   0>;
def : ReadAdvance<ReadIM,      0>;
def : ReadAdvance<ReadIMA,     0>;
def : ReadAdvance<ReadID,      0>;
def : ReadAdvance<ReadExtrHi,  0>;
def : ReadAdvance<ReadAdrBase, 0>;
def : ReadAdvance<ReadST,      0>;
def : ReadAdvance<ReadVLD,     0>;

def : WriteRes<WriteAtomic,  []> { let Unsupported = 1; }
def : WriteRes<WriteBarrier, []> { let Latency = 1; }
def : WriteRes<WriteHint,    []> { let Latency = 1; }

// COPY
def : InstRW<[HIP10CWrite_1c_1S], (instrs COPY)>;

// MSR
def : WriteRes<WriteSys, []> { let Latency = 1; }

// Branch immed
def : SchedAlias<WriteBr, HIP10CWrite_1c_1B>;
// HIP10CWrite_1c_1B B, B.cond

// Branch register
def : SchedAlias<WriteBrReg, HIP10CWrite_1c_1B>;
// HIP10CWrite_1c_1B BR, RET

// Branch and link immed
def : InstRW<[HIP10CWrite_2c_1B_1S23], (instrs BL)>;

// Branch and link register
def : InstRW<[HIP10CWrite_2c_1B_1S23], (instrs BLR)>;

// Compare and branch
// HIP10CWrite_1c_1B CBZ, CBNZ, TBZ, TBNZ

//ALU basic
def : SchedAlias<WriteI, HIP10CWrite_1c_1S>;
// HIP10CWrite_1c_1S ADD, ADC, AND, BIC, EON, EOR, ORN, ORR, SUB, SBC

// ALU basic flagset
// HIP10CWrite_1c_1S23 ADDS, ADCS, ANDS, BICS, SUBS, SBCS
def : InstRW<[HIP10CWrite_1c_1S23],
             (instregex "^(ADD|SUB)S[WX]r[ir]$",
                        "^(ADC|SBC)S[WX]r$",
                        "^ANDS[WX]ri$",
                        "^(AND|BIC)S[WX]rr$")>;

// ALU extend and shift
def : SchedAlias<WriteIEReg, HIP10CWrite_2c_1M>;
// HIP10CWrite_2c_1M ADD{S}, SUB{S}

// Check for LSL shift <= 2
def IsCheapLSL2        : MCSchedPredicate<
                          CheckAll<
                            [CheckShiftLSL,
                             CheckAny<
                               [CheckShiftBy0,
                                CheckShiftBy1,
                                CheckShiftBy2]>]>>;

// Arithmetic LSL shift shift <= 2
def HIP10CWriteISReg : SchedWriteVariant<
                     [SchedVar<IsCheapLSL2, [HIP10CWrite_1c_1S]>,
                      SchedVar<NoSchedPred, [HIP10CWrite_2c_1M]>]>;
def : SchedAlias<WriteISReg, HIP10CWriteISReg>;
// HIP10CWrite_1c_1S ADD, SUB

// Arithmetic flagset LSL shift shift <= 2
// HIP10CWrite_1c_1S23 ADDS, SUBS

// Arithmetic LSR/ASR/ROR shift or LSL shift > 2
// HIP10CWrite_2c_1M ADD{S}, SUB{S}

// Conditional compare
// HIP10CWrite_1c_1S23 CCMN, CCMP

// Conditional select
// HIP10CWrite_1c_1S CSEL, CSINC, CSINV, CSNEG
def : InstRW<[HIP10CWrite_1c_1S], (instregex "CSEL[WX]r", "CSINC[WX]r", "CSINV[WX]r", "CSNEG[WX]r")>;

// Convert floating-point condition flags
def : InstRW<[HIP10CWrite_1c_1S], (instrs AXFLAG, XAFLAG)>;

// Flag manipulation instructions
def : InstRW<[HIP10CWrite_1c_1S], (instrs SETF8, SETF16, RMIF, CFINV)>;

// Logical shift no flagset
// HIP10CWrite_1c_1S BIC
def : InstRW<[HIP10CWrite_1c_1S], (instregex "^(BIC)[WX]rs$")>;

// Logical shift no flagset
// HIP10CWrite_2c_1M AND, EON, EOR, ORN, ORR
def : InstRW<[HIP10CWrite_2c_1M], (instregex "^(AND|EON|EOR|ORN|ORR)[WX]rs$")>;

// Logical shift flagset
// HIP10CWrite_2c_1M ANDS, BICS
def : InstRW<[HIP10CWrite_2c_1M], (instregex "^(AND|BIC)S[WX]rs$")>;

// Divide W-form
def : SchedAlias<WriteID32, HIP10CWrite_12c_12M0>;
// HIP10CWrite_12c_12M0 SDIV, UDIV

// Divide X-form
def : SchedAlias<WriteID64, HIP10CWrite_20c_20M0>;
// HIP10CWrite_20c_20M0 SDIV, UDIV

// Multiply W-form
def : SchedAlias<WriteIM32, HIP10CWrite_2c_1M>;
// HIP10CWrite_2c_1M MUL, MNEG

// Multiply X-form
def : SchedAlias<WriteIM64, HIP10CWrite_3c_1M>;
// HIP10CWrite_3c_1M MUL, MNEG

def HIP10CReadMA1  : SchedReadAdvance<1, [HIP10CWrite_2c_1M_MA, HIP10CWrite_3c_1M_MA]>;
def HIP10CReadMA2  : SchedReadAdvance<2, [HIP10CWrite_2c_1M_MA, HIP10CWrite_3c_1M_MA]>;

// Multiply accumulate W-form
// HIP10CWrite_2c_1M MADD, MSUB
def : InstRW<[HIP10CWrite_2c_1M_MA, ReadIM, ReadIM, HIP10CReadMA1], (instrs MADDWrrr, MSUBWrrr)>;

// Multiply accumulate X-form
// HIP10CWrite_3c_1M MADD, MSUB
def : InstRW<[HIP10CWrite_3c_1M_MA, ReadIM, ReadIM, HIP10CReadMA2], (instrs MADDXrrr, MSUBXrrr)>;

// Multiply accumulate long
// HIP10CWrite_2c_1M SMADDL, SMSUBL, UMADDL, UMSUBL
def : InstRW<[HIP10CWrite_2c_1M_MA, ReadIM, ReadIM, HIP10CReadMA1], (instrs SMADDLrrr, SMSUBLrrr, UMADDLrrr, UMSUBLrrr)>;

// Multiply high
// HIP10CWrite_3c_1M SMULH, UMULH

// Multiply long
// HIP10CWrite_2c_1M SMNEGL, SMULL, UMNEGL, UMULL

// Authenticate data address
def : InstRW<[HIP10CWrite_5c_1M1], (instrs AUTDA, AUTDB, AUTDZA, AUTDZB)>;

// Authenticate instruction address
def : InstRW<[HIP10CWrite_5c_1M1], (instrs AUTIA, AUTIB, AUTIA1716, AUTIB1716, AUTIASP, AUTIBSP, AUTIAZ, AUTIBZ, AUTIZA, AUTIZB)>;

// Branch and link register with pointer authentication
def : InstRW<[HIP10CWrite_6c_2B_1M1], (instrs BLRAA, BLRAAZ, BLRAB, BLRABZ)>;

// Branch register with pointer authentication
def : InstRW<[HIP10CWrite_6c_2B_1M1], (instrs BRAA, BRAAZ, BRAB, BRABZ)>;

// Branch return with pointer authentication
// HIP10CWrite_6c_2B_1M1 RETA, RETB
def : InstRW<[HIP10CWrite_6c_2B_1M1], (instrs RETAA, RETAB)>;

// Compute pointer authentication code for data address
def : InstRW<[HIP10CWrite_5c_1M1], (instrs PACDA, PACDB, PACDZA, PACDZB)>;

// Compute pointer authentication code using generic key
def : InstRW<[HIP10CWrite_5c_1M1], (instrs PACGA)>;

// Compute pointer authentication code for instruction address
def : InstRW<[HIP10CWrite_5c_1M1], (instrs PACIA, PACIB, PACIA1716, PACIB1716, PACIASP, PACIBSP, PACIAZ, PACIBZ, PACIZA, PACIZB)>;

// Load register with pointer authentication
// HIP10CWrite_9c_2LD_1M1 LDRAA, LDRAB
def : InstRW<[HIP10CWrite_9c_2LD_1M1], (instregex "^LDRA[AB](indexed|writeback)")>;

// Strip pointer authentication code
def : InstRW<[HIP10CWrite_1c_1S1], (instrs XPACD, XPACI, XPACLRI)>;

// Address generation
def : InstRW<[HIP10CWrite_1c_1S23], (instrs ADR, ADRP)>;

// Bitfield extract one reg
def : SchedAlias<WriteExtr, HIP10CWrite_1c_1S>;
// HIP10CWrite_1c_1S EXTR

// Bitfield extract two regs
// HIP10CWrite_1c_1S EXTR

// Bitfield move basic
def : SchedAlias<WriteIS, HIP10CWrite_1c_1S>;
// HIP10CWrite_1c_1S SBFM, UBFM

// Bitfield move insert
// HIP10CWrite_1c_1S BFM

// Move immed
def : SchedAlias<WriteImm, HIP10CWrite_1c_1S>;
// HIP10CWrite_1c_1S MOVN, MOVK, MOVZ

// Count leading
// HIP10CWrite_1c_1S CLS, CLZ

// Reverse bits/bytes
// HIP10CWrite_1c_1S RBIT, REV, REV16, REV32

// Variable shift
// HIP10CWrite_1c_1S ASRV, LSLV, LSRV, RORV

// Load register literal
def : SchedAlias<WriteLD, HIP10CWrite_4c_1LD>;
// HIP10CWrite_4c_1LD LDR, LDRSW, PRFM

// Load register unscaled immed
// HIP10CWrite_4c_1LD LDUR, LDURB, LDURH, LDURSB, LDURSH, LDURSW, PRFUM

// Load register immed post-index
def : SchedAlias<WriteAdr, HIP10CWrite_1c_2S>;
// HIP10CWrite_4c_1LD_2S LDR, LDRB, LDRH, LDRSB, LDRSH, LDRSW

// Load register immed pre-index
// HIP10CWrite_4c_1LD_2S LDR, LDRB, LDRH, LDRSB, LDRSH, LDRSW

// Load register immed unprivileged
// HIP10CWrite_4c_1LD LDTR, LDTRB, LDTRH, LDTRSB, LDTRSH, LDTRSW

// Load register unsigned immed
// HIP10CWrite_4c_1LD LDR, LDRB, LDRH, LDRSB, LDRSH, LDRSW, PRFM

// Load register register offset no-extend basic
def : SchedAlias<WriteLDIdx, HIP10CWrite_4c_1LD>;
// HIP10CWrite_4c_1LD LDR, LDRB, LDRH, LDRSB, LDRSH, LDRSW, PRFM

// Load register register offset no-extend scale by 4/8
// HIP10CWrite_4c_1LD LDR, LDRSW, PRFM

// Load register register offset no-extend scale by 2
// HIP10CWrite_5c_1LD_2S LDRH, LDRSH

// Load register register offset extend
// HIP10CWrite_4c_1LD LDR, LDRB, LDRH, LDRSB, LDRSH, LDRSW, PRFM

// Load register register offset extend scale by 4/8
// HIP10CWrite_4c_1LD LDR, LDRSW, PRFM

// Load register register offset extend scale by 2
// HIP10CWrite_5c_1LD_2S LDRH, LDRSH

// Load pair immed offset normal
def : SchedAlias<WriteLDHi, HIP10CWrite_0c>;
// HIP10CWrite_4c_1LD LDP, LDNP

// Load pair immed offset signed words
// HIP10CWrite_4c_1LD LDPSW

// Load pair immed post-index normal
// HIP10CWrite_4c_1LD_2S LDP

// Load pair immed post-index signed words
// HIP10CWrite_4c_1LD_2S LDPSW

// Load pair immed pre-index normal
// HIP10CWrite_4c_1LD_2S LDP

// Load pair immed pre-index signed words
// HIP10CWrite_4c_1LD_2S LDPSW

// Store register unscaled immed
def : SchedAlias<WriteST, HIP10CWrite_1c_1ST_1M>;
// HIP10CWrite_1c_1ST_1M STUR, STURB, STURH

// Store register immed post-index
def : SchedAlias<WriteSTIdx, HIP10CWrite_1c_1ST_1M>;
// HIP10CWrite_1c_1ST_1M STR, STRB, STRH

// Store register immed pre-index
// HIP10CWrite_1c_1ST_2S_1M STR, STRB, STRH

// Store register immed unprivileged
// HIP10CWrite_1c_1ST_1M STTR, STTRB, STTRH

// Store register unsigned immed
// HIP10CWrite_1c_1ST_1M STR, STRB, STRH

// Store register register offset no-extend basic
// HIP10CWrite_1c_1ST_1M STR, STRB, STRH

// Store register register offset scaled by 4/8
// HIP10CWrite_1c_1ST_1M STR

// Store register offset no-extend scaled by 2
// HIP10CWrite_2c_1ST_2S_1M STRH

// Store register register offset extend
// HIP10CWrite_1c_1ST_1M STR, STRB, STRH

// Store register register offset extend scale by 4/8
// HIP10CWrite_1c_1ST_1M STR

// Store register register offset extend scale by 2
// HIP10CWrite_2c_1ST_2S_1M STRH

// Store pair immed offset
def : SchedAlias<WriteSTP, HIP10CWrite_1c_1ST_1STD_1M>;
// HIP10CWrite_1c_1ST_1STD_1M STP, STNP

// Store pair immed post-index
// HIP10CWrite_1c_1ST_2S_1M STP

// Store pair immed pre-index
// HIP10CWrite_1c_1ST_2S_1M STP

// FP absolute value
// HIP10CWrite_1c_1F FABS
def : InstRW<[HIP10CWrite_1c_1F], (instregex "FABS[DHS]r")>;

// FP absolute value
def : SchedAlias<WriteVd, HIP10CWrite_2c_1F>;
// HIP10CWrite_2c_1F FABD

// FP negate
// HIP10CWrite_1c_1F FNEG
def : InstRW<[HIP10CWrite_1c_1F], (instregex "FNEG[DHS]r")>;

// FP compare
def : SchedAlias<WriteFCmp, HIP10CWrite_4c_1F_1S23>;
// HIP10CWrite_4c_1F_1S23 FCMP{E}

// FP conditional compare
// HIP10CWrite_7c_1S01_1F_1S23 FCCMP{E}
def : InstRW<[HIP10CWrite_7c_1S01_1F_1S23], (instregex "FCCMP(D|ED|EH|ES|H|S)rr")>;

// FP conditional select
// HIP10CWrite_4c_1S01_1F FCSEL
def : InstRW<[HIP10CWrite_4c_1S01_1F], (instregex "FCSEL[DHS]rrr")>;

def : SchedAlias<WriteFDiv, HIP10CWrite_7c_4F>;

// FP divide H-form
// HIP10CWrite_7c_4F FDIV

// FP divide S-form
// HIP10CWrite_7c_4F FDIV

// FP divide D-form
// HIP10CWrite_10c_6F FDIV
def : InstRW<[HIP10CWrite_10c_6F], (instrs FDIVDrr)>;

// FP square root H-form
// HIP10CWrite_7c_4F FSQRT

// FP square root S-form
// HIP10CWrite_9c_5F FSQRT
def : InstRW<[HIP10CWrite_9c_5F], (instrs FSQRTSr)>;

// FP square root D-form
// HIP10CWrite_15c_11F FSQRT
def : InstRW<[HIP10CWrite_15c_11F], (instrs FSQRTDr)>;

def ReadFM : SchedReadAdvance<0>;
def ReadFMA: SchedReadAdvance<2, [WriteFMul, HIP10CWrite_4c_1F_FM]>;

// FP fused multiply-add
// HIP10CWrite_4c_1F FMADD, FMSUB, FNMADD, FNMSUB
def : InstRW<[HIP10CWrite_4c_1F_FM, ReadFM, ReadFM, ReadFMA], (instregex "^(FMADD|FMSUB|FNMADD|FNMSUB)[DHS]rrr")>;

// FP max/min
def : SchedAlias<WriteF, HIP10CWrite_2c_1F>;
// HIP10CWrite_2c_1F FMAX, FMAXNM, FMIN, FMINNM

// FP add
// HIP10CWrite_2c_1F FADD, FSUB

// FP multiply
def : SchedAlias<WriteFMul, HIP10CWrite_3c_1F_FM>;
// HIP10CWrite_3c_1F FMUL, FNMUL

// FP round to FP integral
// HIP10CWrite_2c_1F FRINTA, FRINTI, FRINTM, FRINTN, FRINTP, FRINTX, FRINTZ, FRINT32Z, FRINT32X, FRINT64Z, FRINT64X

// FP convert to FP
def : SchedAlias<WriteFCvt, HIP10CWrite_2c_1F>;
// HIP10CWrite_2c_1F FCVT

// FP convert from gen to vec reg
// HIP10CWrite_5c_1S01_1F SCVTF, UCVTF
def : InstRW<[HIP10CWrite_5c_1S01_1F], (instregex "^[SU]CVTF[SU][WX][HSD]ri$")>;

// FP convert from vec to gen reg
// HIP10CWrite_5c_1F_1S23 FCVTAS, FCVTAU, FCVTMS, FCVTMU, FCVTNS, FCVTNU, FCVTPS, FCVTPU, FCVTZS, FCVTZU
def : InstRW<[HIP10CWrite_5c_1F_1S23], (instregex "^FCVT[AMNPZ][SU][SU][WX][HSD]r$")>;

// FP convert Javascript from to gen reg
def : InstRW<[HIP10CWrite_5c_1F_1S23], (instrs FJCVTZS)>;

// FP move immed
def : SchedAlias<WriteFImm, HIP10CWrite_1c_1F>;
// HIP10CWrite_1c_1F FMOV

// FP move register
// HIP10CWrite_1c_1F FMOV
def : InstRW<[HIP10CWrite_1c_1F], (instrs FMOVHr, FMOVSr, FMOVDr)>;

// FP transfer from gen to low half of vec reg
// HIP10CWrite_3c_1S01 FMOV
def : InstRW<[HIP10CWrite_3c_1S01], (instrs FMOVWHr, FMOVXHr, FMOVWSr, FMOVXDr)>;

// FP transfer from gen to high half of vec reg
// HIP10CWrite_4c_1S01_1F FMOV
def : InstRW<[HIP10CWrite_4c_1S01_1F], (instrs FMOVXDHighr)>;

// FP transfer from vec to gen reg
def : SchedAlias<WriteFCopy, HIP10CWrite_1c_1F>;
// HIP10CWrite_1c_1F FMOV

// Load vector reg literal
// HIP10CWrite_6c_1LD LDR
def : InstRW<[HIP10CWrite_6c_1LD, ReadAdrBase], (instregex "^LDR[SDQ]l$",
                                                      "^LDUR[BHSDQ]i$",
                                                      "^LDR[BHSDQ]ui$")>;

// Load vector reg unscaled immed
// HIP10CWrite_6c_1LD LDUR

// Load vector reg immed post-index
// HIP10CWrite_6c_1LD_2S LDR
def : InstRW<[HIP10CWrite_6c_1LD, WriteAdr],
             (instregex "^LDR[BHSDQ](post|pre)$")>;

// Load vector reg immed pre-index
// HIP10CWrite_6c_1LD_2S LDR

// Load vector reg immed unprivileged
// HIP10CWrite_6c_1LD LDR

// Load vector reg unsigned immed
// HIP10CWrite_6c_1LD LDR

// Load vector reg register offset no-extend basic
// HIP10CWrite_6c_1LD LDR
def : InstRW<[HIP10CWrite_6c_1LD, ReadAdrBase], (instregex "^LDR[BSD]ro[WX]$")>;

// Load vector reg register offset no-extend scale by 4/8
// HIP10CWrite_6c_1LD LDR

// Load vector reg register offset no-extend scale by 2
// HIP10CWrite_6c_1LD LDR

// Load vector reg register offset extend
// HIP10CWrite_7c_2S_1LD LDR
def : InstRW<[HIP10CWrite_7c_2S_1LD, ReadAdrBase], (instregex "^LDR[HQ]ro[WX]$")>;

// Load vector reg register offset extend scale by 4/8
// HIP10CWrite_7c_2S_1LD LDR

// Load vector reg register offset extend scale by 2
// HIP10CWrite_7c_2S_1LD LDR

// Load vector pair immed offset normal
// HIP10CWrite_6c_1LD LDP, LDNP
def : InstRW<[HIP10CWrite_6c_1LD, HIP10CWrite_0c], (instregex "^LDN?P[SDQ]i$")>;

// Load vector pair immed offset signed words
// HIP10CWrite_6c_1LD LDP, LDNP

// Load vector pair immed post-index normal
// HIP10CWrite_6c_1LD_2S LDP
def : InstRW<[HIP10CWrite_6c_1LD, HIP10CWrite_0c, WriteAdr],
             (instregex "^LDP[SDQ](pre|post)$")>;

// Load vector pair immed post-index signed words
// HIP10CWrite_6c_1LD_2S LDP

// Load vector pair immed pre-index normal
// HIP10CWrite_6c_1LD_2S LDP

// Load vector pair immed pre-index signed words
// HIP10CWrite_6c_1LD_2S LDP

// Store vector reg unscaled immed
// HIP10CWrite_1c_1ST_1STD STUR
def : InstRW<[HIP10CWrite_1c_1ST_1STD], (instregex "^STUR[BHSDQ]i$")>;

// Store vector reg immed post-index
// HIP10CWrite_1c_1ST_1STD_2S STR
def : InstRW<[HIP10CWrite_1c_1ST_1STD, WriteAdr],
             (instregex "^STR[BHSDQ](pre|post)$")>;

// Store vector reg immed pre-index
// HIP10CWrite_1c_1ST_1STD_2S STR

// Store vector reg immed unprivileged
// HIP10CWrite_1c_1ST_1STD STR

// Store vector reg unsigned immed
// HIP10CWrite_1c_1ST_1STD STR
def : InstRW<[HIP10CWrite_1c_1ST_1STD], (instregex "^STR[BHSDQ]ui$")>;

// Store vector reg reg offset no-extend basic
// HIP10CWrite_1c_1ST_1STD STR
def : InstRW<[HIP10CWrite_1c_1ST_1STD, ReadAdrBase],
             (instregex "^STR[BHSDQ]ro[WX]$")>;

// Store vector reg offset no-extend scaled by 2
// HIP10CWrite_1c_1ST_1STD STR

// Store vector reg reg offset extend
// HIP10CWrite_2c_2S_1ST_1STD STR

// Store vector reg reg offset extend scale by 4/8
// HIP10CWrite_2c_2S_1ST_1STD STR

// Store vector reg reg offset extend scale by 1
// HIP10CWrite_2c_2S_1ST_1STD STR

// Store vector pair immed offset S/D-form
// HIP10CWrite_1c_1ST_1STD STP, STNP
def : InstRW<[HIP10CWrite_1c_1ST_1STD], (instregex "^STN?P[SD]i$")>;

// Store vector pair immed offset Q-form
// HIP10CWrite_2c_2ST_2STD STP, STNP
def : InstRW<[HIP10CWrite_2c_2ST_2STD], (instregex "^STN?P[Q]i$")>;

// Store vector pair immed post-index S/D-form
// HIP10CWrite_1c_1ST_1STD_2S STP
def : InstRW<[HIP10CWrite_1c_1ST_1STD, WriteAdr],
             (instregex "^STP[SD](pre|post)$")>;

// Store vector pair immed post-index Q-form
// HIP10CWrite_2c_2ST_2STD_4S STP
def : InstRW<[HIP10CWrite_2c_2ST_2STD_4S, WriteAdr], (instrs STPQpre, STPQpost)>;

// Store vector pair immed pre-index S/D-form
// HIP10CWrite_1c_1ST_1STD_2S STP

// Store vector pair immed pre-index Q-form
// HIP10CWrite_2c_2ST_2STD_4S STP

// ASIMD absolute diff
def : SchedAlias<WriteVq, HIP10CWrite_2c_1F>;
// HIP10CWrite_2c_1F SABD, UABD

// ASIMD absolute diff accum
// HIP10CWrite_2c_1F SABA, UABA

// ASIMD absolute diff accum long
// HIP10CWrite_2c_1F SABAL{2}, UABAL{2}

// ASIMD arith basic
// HIP10CWrite_1c_1F ABS, ADD, NEG, SADDL{2}, SHADD, SHSUB, SSUBL{2}, SUB, UADDL{2}, UHADD, UHSUB, USUBL{2}
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(ABS|ADD|NEG|SADDL|SHADD|SHSUB|SSUBL|SUB|UADDL|UHADD|UHSUB|USUBL)v(1|2|4|8|16)i")>;

// ASIMD arith wide
// HIP10CWrite_2c_1F SADDW{2}, SSUBW{2}, UADDW{2}, USUBW{2}

// ASIMD arith complex
// HIP10CWrite_1c_1F ADDHN{2}, SQABS, SQADD, SQNEG, SQSUB, SUBHN{2}, SUQADD, UQADD, UQSUB, USQADD
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(ADDHN|SQABS|SQADD|SQNEG|SQSUB|SUBHN|SUQADD|UQADD|UQSUB|USQADD)v(1|2|4|8|16)i")>;

// ASIMD arith complex
// HIP10CWrite_2c_1F RADDHN{2}, RSUBHN{2}, SRHADD, URHADD

// ASIMD arith pair-wise
// HIP10CWrite_2c_1F ADDP, SADDLP, UADDLP

// ASIMD arith reduce
// HIP10CWrite_2c_1F ADDV, SADDLV, UADDLV

// ASIMD compare
// HIP10CWrite_1c_1F CMGT, CMEQ, CMGE, CMLT, CMLE, CMTST, CMHI, CMHS
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(CMGT|CMEQ|CMGE|CMLT|CMLE|CMTST|CMHI|CMHS)v(1|2|4|8|16)i")>;

// ASIMD dot product
// HIP10CWrite_3c_1F SDOT, UDOT
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^([SU]|SU|US)DOT(lane)?v(8|16)i8$")>;

// ASIMD dot product using signed and unsigned integers
// HIP10CWrite_3c_1F SUDOT, USDOT

// ASIMD logical
// HIP10CWrite_1c_1F AND, NOT, ORN, ORR, MOV, BIC, EOR, MVN
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(AND|NOT|ORN|ORR|MOV|BIC|EOR|MVN)v(1|2|4|8|16)i")>;

// ASIMD matrix multiply-accumulate
// HIP10CWrite_5c_2F SMMLA, UMMLA, USMMLA
def : InstRW<[HIP10CWrite_5c_2F], (instrs SMMLA, UMMLA, USMMLA)>;

// ASIMD max/min
// HIP10CWrite_1c_1F SMAX, SMIN, UMAX, UMIN
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(SMAX|SMIN|UMAX|UMIN)v(1|2|4|8|16)i")>;

// ASIMD max/min pair-wise
// HIP10CWrite_2c_1F SMAXP, SMINP, UMAXP, UMINP

// ASIMD max/min reduce S form
// HIP10CWrite_2c_1F SMAXV, SMINV, UMAXV, UMINV

// ASIMD max/min reduce B/H form
// HIP10CWrite_3c_1F SMAXV, SMINV, UMAXV, UMINV
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^(SMAXV|SMINV|UMAXV|UMINV)v.*i(8|16)v$")>;

// ASIMD multiply B/H/S form
// HIP10CWrite_3c_1F MUL, SQDMULH, SQRDMULH
def : InstRW<[HIP10CWrite_3c_1F],
             (instregex "^MUL(v[148]i16|v[124]i32)$",
                        "^SQR?DMULH(v[48]i16|v[24]i32)$",
                        "^ML[AS](v[148]i16|v[124]i32)$",
                        "^[SU]ML[AS]Lv",
                        "^SQRDML[AS]H(v[148]i16|v[124]i32)$",
                        "^SQDML[AS]Lv")>;

// ASIMD multiply accumulate B/H/S form
// HIP10CWrite_3c_1F MLA, MLS

// ASIMD multiply accumulate high
// HIP10CWrite_3c_1F SQRDMLAH, SQRDMLSH

// ASIMD multiply long
// HIP10CWrite_3c_1F SMULL{2}, SQDMULL{2}, UMULL{2}

// ASIMD multiply accumulate long
// HIP10CWrite_3c_1F SMLAL{2}, SMLSL{2}, UMLAL{2}, UMLSL{2}

// ASIMD multiply accumulate saturating long
// HIP10CWrite_3c_1F SQDMLAL{2}, SQDMLSL{2}

// ASIMD multiply/multiply long (8x8) polynomial
// HIP10CWrite_2c_1F1 PMUL, PMULL{2}

// ASIMD pairwise add and accumulate long
// HIP10CWrite_3c_1F SADALP, UADALP
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^(SADALP|UADALP)v(1|2|4|8|16)i")>;

// ASIMD shift accumulate
// HIP10CWrite_3c_1F SSRA, SRSRA, USRA, URSRA
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^(SSRA|SRSRA|USRA|URSRA)v(1|2|4|8|16)i")>;

// ASIMD shift by immed basic
// HIP10CWrite_2c_1F SHL, SHLL{2}, SHRN{2}, SSHLL{2}, SSHR, SXTL{2}, USHLL{2}, USHR, UXTL{2}

// ASIMD shift by immed and insert basic
// HIP10CWrite_2c_1F SLI, SRI

// ASIMD shift by immed complex
// HIP10CWrite_3c_1F RSHRN{2}, SQRSHRN{2}, SQRSHRUN{2}, SRSHR, UQRSHRN{2}, URSHR
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^(RSHRN|SQRSHRN|SQRSHRUN|SRSHR|UQRSHRN|URSHR)v(1|2|4|8|16)i")>;

// ASIMD shift by immed complex
// HIP10CWrite_2c_1F SQSHRN{2}, SQSHRUN{2}, SQSHLU, UQSHRN{2}

// ASIMD shift by immed complex
// HIP10CWrite_2c_1F SQSHL, UQSHL

// ASIMD shift by register basic
// HIP10CWrite_3c_1F SSHL, USHL
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^(SSHL|USHL|SQSHL|UQSHL)v(1|2|4|8|16)i")>;

// ASIMD shift by register complex
// HIP10CWrite_4c_1F SRSHL, SQRSHL, URSHL, UQRSHL
def : InstRW<[HIP10CWrite_4c_1F], (instregex "^(SRSHL|SQRSHL|URSHL|UQRSHL)v(1|2|4|8|16)i")>;

// ASIMD shift by register complex
// HIP10CWrite_3c_1F SQSHL, UQSHL

// ASIMD FP absolute value
// HIP10CWrite_1c_1F FABS
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(FABS|FNEG)v(1|2|4|8)f")>;

// ASIMD FP negative value
// HIP10CWrite_1c_1F FNEG

// ASIMD FP absolute difference
// HIP10CWrite_2c_1F FABD

// ASIMD FP arith
// HIP10CWrite_2c_1F FADD, FSUB

// ASIMD FP add pairwise
// HIP10CWrite_3c_1F FADDP
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^(FADDP)v(1|2|4|8)f")>;

// ASIMD FP compare
// HIP10CWrite_2c_1F FACGE, FACGT, FCMEQ, FCMGE, FCMGT, FCMLE, FCMLT

// ASIMD FP convert long
// HIP10CWrite_3c_2F FCVTL{2}
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^(FCVTL|FCVTXN|FCVTN)v(1|2|4|8)f")>;

// ASIMD FP convert narrow
// HIP10CWrite_3c_2F FCVTXN{2}, FCVTN{2}

// ASIMD FP convert to Integer/Fixed point D-form
// HIP10CWrite_2c_1F FCVTNS, FCVTNU, FCVTMS, FCVTMU, FCVTAS, FCVTAU, FCVTPS, FCVTPU, FCVTZS, FCVTZU

// ASIMD FP convert to Integer/Fixed point Q-form
// HIP10CWrite_3c_2F FCVTNS, FCVTNU, FCVTMS, FCVTMU, FCVTAS, FCVTAU, FCVTPS, FCVTPU, FCVTZS, FCVTZU
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^FCVT(NS|NU|MS|MU|AS|AU|PS|PU|ZS|ZU)v(2f64|4f32|8f16)")>;

// ASIMD FP convert from Integer/Fixed-point to FP D-form
// HIP10CWrite_2c_1F SCVTF, UCVTF

// ASIMD FP convert from Integer/Fixed-point to FP Q-form
// HIP10CWrite_3c_2F SCVTF, UCVTF
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^[SU]CVTFv(2f64|4f32|8f16)")>;

// ASIMD FP divide D-form F16
// HIP10CWrite_9c_6F FDIV
def : InstRW<[HIP10CWrite_9c_6F_DIV], (instregex "^FDIVv(4f16|2f32)")>;

// ASIMD FP divide D-form F32
// HIP10CWrite_9c_6F FDIV

// ASIMD FP divide Q-form F16
// HIP10CWrite_13c_9F FDIV
def : InstRW<[HIP10CWrite_13c_9F_DIV], (instregex "^FDIVv(8f16|4f32)")>;

// ASIMD FP divide Q-form F32
// HIP10CWrite_13c_9F FDIV

// ASIMD FP divide Q-form F64
// HIP10CWrite_15c_11F FDIV
def : InstRW<[HIP10CWrite_15c_11F_DIV], (instregex "^FDIVv(2f64)")>;

// ASIMD FP square root D-form F16
// HIP10CWrite_13c_9F FSQRT
def : InstRW<[HIP10CWrite_13c_9F], (instregex "^FSQRTv(4f16|2f32)")>;

// ASIMD FP square root D-form F32
// HIP10CWrite_13c_9F FSQRT

// ASIMD FP square root Q-form F16
// HIP10CWrite_21c_17F FSQRT
def : InstRW<[HIP10CWrite_21c_17F], (instregex "^FSQRTv(8f16|4f32)")>;

// ASIMD FP square root Q-form F32
// HIP10CWrite_21c_17F FSQRT

// ASIMD FP square root Q-form F64
// HIP10CWrite_25c_21F FSQRT
def : InstRW<[HIP10CWrite_25c_21F], (instrs FSQRTv2f64)>;

// ASIMD FP max/min
// HIP10CWrite_2c_1F FMAX, FMAXNM, FMIN, FMINNM

// ASIMD FP max/min pairwise
// HIP10CWrite_3c_1F FMAXP, FMAXNMP, FMINP, FMINNMP
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^(FMAXP|FMAXNMP|FMINP|FMINNMP)v")>;

// ASIMD FP max/min reduce F16
// HIP10CWrite_3c_1F FMAXV, FMAXNMV, FMINV, FMINNMV
def : InstRW<[HIP10CWrite_3c_1F], (instregex "^(FMAXV|FMAXNMV|FMINV|FMINNMV)v4i16v")>;

// ASIMD FP max/min reduce F32 F64
// HIP10CWrite_2c_1F FMAXV, FMAXNMV, FMINV, FMINNMV

def ReadFM_NEON : SchedReadAdvance<0>;
def ReadFMA_NEON: SchedReadAdvance<2, [HIP10CWrite_3c_1F_FM_NEON, HIP10CWrite_4c_1F_FM_NEON]>;

// ASIMD FP multiply
// HIP10CWrite_3c_1F FMUL, FMULX
def : InstRW<[HIP10CWrite_3c_1F_FM_NEON], (instregex "^FMULX?v")>;

// ASIMD FP fused multiply-add
// HIP10CWrite_4c_1F FMLA, FMLS
def : InstRW<[HIP10CWrite_4c_1F_FM_NEON, ReadFM_NEON, ReadFM_NEON, ReadFMA_NEON], (instregex "^FML[AS]v", "^FML[AS]L2?v")>;

// ASIMD FP fused multiply-add long
// HIP10CWrite_4c_1F FMLAL{2}, FMLSL{2}

// ASIMID FP round to FP integral D-form
// HIP10CWrite_2c_1F FRINTN, FRINTM, FRINTP, FRINTZ, FRINTA, FRINTX, FRINTI, FRINT32X, FRINT64X, FRINT32Z, FRINT64Z

// ASIMDI FP round to FP integral Q-form
// HIP10CWrite_3c_2F FRINTN, FRINTM, FRINTP, FRINTZ, FRINTA, FRINTX, FRINTI, FRINT32X, FRINT64X, FRINT32Z, FRINT64Z
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^FRINT(N|M|P|Z|A|X|I|32X|64X|32Z|64Z)v(2f64|4f32|8f16)")>;

// ASIMD convert F32 to BF16
// HIP10CWrite_3c_2F BFCVTN{2}
def : InstRW<[HIP10CWrite_3c_2F], (instrs BFCVTN, BFCVTN2)>;

// ASIMD dot product
// HIP10CWrite_6c_2F BFDOT
def : InstRW<[HIP10CWrite_6c_2F], (instregex "^BF(DOT|16DOTlane)v[48]bf16$")>;

// ASIMD matrix multiply accumulate
// HIP10CWrite_9c_4F BFMMLA
def : InstRW<[HIP10CWrite_9c_4F], (instrs BFMMLA)>;

// ASIMD multiply accumulate long
// HIP10CWrite_5c_1F BFMLALB, BFMLALT
def : InstRW<[HIP10CWrite_5c_1F], (instregex "^BFMLAL[BT](Idx)?$")>;

// Scalar convert F32 to BF16
// HIP10CWrite_2c_1F BFCVT

// ASIMD bit reverse
// HIP10CWrite_1c_1F RBIT
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(RBIT|BIF|BIT|BSL|CLZ|CLS)v")>;

// ASIMD bitwise insert
// HIP10CWrite_1c_1F BIF, BIT, BSL

// ASIMD count
// HIP10CWrite_1c_1F CLZ, CLS

// ASIMD count D
// HIP10CWrite_2c_1F CNT

// ASIMD count B/H/S
// HIP10CWrite_1c_1F CNT
def : InstRW<[HIP10CWrite_1c_1F], (instrs CNTv8i8)>;

// ASIMD duplicate gen reg
// HIP10CWrite_4c_1S01_1F DUP
def : InstRW<[HIP10CWrite_4c_1S01_1F], (instregex "^DUPv.*gpr$")>;

// ASIMD duplicate element
// HIP10CWrite_1c_1F DUP
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^DUPv.*lane$")>;

// ASIMD extract
// HIP10CWrite_1c_1F EXT
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(EXT|XTN)v")>;

// ASIMD extract narrow
// HIP10CWrite_1c_1F XTN{2}

// ASIMD extract narrow saturating
// HIP10CWrite_2c_1F SQXTN{2}, SQXTUN{2}, UQXTN{2}

// ASIMD insert element to element
// HIP10CWrite_4c_1S01_1F INS
def : InstRW<[HIP10CWrite_4c_1S01_1F], (instregex "^INSv")>;

// ASIMD FP move immed
// HIP10CWrite_1c_1F FMOV
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^FMOVv(2|4|8)f")>;

// ASIMD move integer immediate
// HIP10CWrite_1c_1F MOVI, MVNI
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(MOVI|MVNI)v")>;

// ASIMD reciprocal and square root estimate D-form
// HIP10CWrite_2c_1F URECPE, URSQRTE

// ASIMD reciprocal and square root estimate Q-form
// HIP10CWrite_3c_2F URECPE, URSQRTE
def : InstRW<[HIP10CWrite_3c_2F], (instrs URECPEv4i32, URSQRTEv4i32)>;

// ASIMD FP reciprocal and square root estimate D-form
// HIP10CWrite_2c_1F FRECPE, FRSQRTE

// ASIMD FP reciprocal and square root estimate Q-form
// HIP10CWrite_3c_2F FRECPE, FRSQRTE
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^(FRECPE|FRSQRTE)v(2f64|4f32|8f16)")>;

// ASIMD FP reciprocal exponent D-form
// HIP10CWrite_2c_1F FRECPX

// ASIMD FP reciprocal exponent Q-form
// HIP10CWrite_3c_2F FRECPX

// ASIMD FP reciprocal step
// HIP10CWrite_4c_1F FRECPS, FRSQRTS
def : InstRW<[HIP10CWrite_4c_1F], (instregex "^FRECPS(16|32|64)$", "^FRECPSv",
                                         "^FRSQRTS(16|32|64)$", "^FRSQRTSv")>;

// ASIMD reverse
// HIP10CWrite_1c_1F REV16, REV32, REV64
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^REV(16|32|64)v")>;

// ASIMD table lookup 1 or 2 table regs
// HIP10CWrite_1c_1F TBL
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^TBLv(8|16)i8(One|Two)$",
                                           "^TBXv(8|16)i8One$")>;

// ASIMD table lookup 3 table regs
// HIP10CWrite_2c_2F TBL
def : InstRW<[HIP10CWrite_2c_2F], (instrs TBLv8i8Three, TBLv16i8Three,
                                        TBXv8i8Two, TBXv16i8Two)>;

// ASIMD table lookup 4 table regs
// HIP10CWrite_3c_3F TBL
def : InstRW<[HIP10CWrite_3c_3F], (instrs TBLv8i8Four, TBLv16i8Four)>;

// ASIMD table lookup extension 1 table reg
// HIP10CWrite_1c_1F TBX

// ASIMD table lookup extension 2 table reg
// HIP10CWrite_2c_2F TBX

// ASIMD table lookup extension 3 table reg
// HIP10CWrite_3c_3F TBX
def : InstRW<[HIP10CWrite_3c_3F], (instrs TBXv8i8Three, TBXv16i8Three)>;

// ASIMD table lookup extension 4 table reg
// HIP10CWrite_4c_4F TBX
def : InstRW<[HIP10CWrite_4c_4F], (instrs TBXv8i8Four, TBXv16i8Four)>;

// ASIMD move FP to general register
// HIP10CWrite_2c_1F SMOV, UMOV
def : InstRW<[HIP10CWrite_2c_1F], (instregex "^SMOVvi(((8|16)to(32|64))|32to64)$",
                                         "^UMOVvi(8|16|32|64)$")>;

// ASIMD transpose
// HIP10CWrite_1c_1F TRN1, TRN2
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^TRN[12]v")>;

// ASIMD uzip/zip
// HIP10CWrite_1c_1F UZP1, UZP2, ZIP1, ZIP2
def : InstRW<[HIP10CWrite_1c_1F], (instregex "^(UZP1|UZP2|ZIP1|ZIP2)v")>;

// SIMD load 1-element multiple 1-reg
// HIP10CWrite_6c_1LD LD1
def : InstRW<[HIP10CWrite_6c_1LD],
             (instregex "^LD1Onev(8b|16b|4h|8h|2s|4s|1d|2d)$")>;
def : InstRW<[HIP10CWrite_6c_1LD, WriteAdr],
             (instregex "^LD1Onev(8b|16b|4h|8h|2s|4s|1d|2d)_POST$")>;

// SIMD load 1-element multiple 2-reg
// HIP10CWrite_6c_2LD LD1
def : InstRW<[HIP10CWrite_6c_2LD],
             (instregex "^LD1Twov(8b|16b|4h|8h|2s|4s|1d|2d)$")>;
def : InstRW<[HIP10CWrite_6c_2LD, WriteAdr],
             (instregex "^LD1Twov(8b|16b|4h|8h|2s|4s|1d|2d)_POST$")>;

// SIMD load 1-element multiple 3-reg
// HIP10CWrite_6c_3LD LD1
def : InstRW<[HIP10CWrite_6c_3LD],
             (instregex "^LD1Threev(8b|16b|4h|8h|2s|4s|1d|2d)$")>;
def : InstRW<[HIP10CWrite_6c_3LD, WriteAdr],
             (instregex "^LD1Threev(8b|16b|4h|8h|2s|4s|1d|2d)_POST$")>;

// SIMD load 1-element multiple 4-reg
// HIP10CWrite_6c_4LD LD1
def : InstRW<[HIP10CWrite_6c_4LD],
             (instregex "^LD1Fourv(8b|4h|2s|1d|16b|8h|4s|2d)$")>;
def : InstRW<[HIP10CWrite_6c_4LD, WriteAdr],
             (instregex "^LD1Fourv(8b|4h|2s|1d|16b|8h|4s|2d)_POST$")>;

// SIMD load 1-element single 1 lane
// HIP10CWrite_7c_1LD_1F LD1
def : InstRW<[HIP10CWrite_7c_1LD_1F],
             (instregex "^LD1(i|Rv)(8|16|32|64)$",
                        "^LD1Rv(8b|16b|4h|8h|2s|4s|1d|2d)$")>;
def : InstRW<[HIP10CWrite_7c_1LD_1F, WriteAdr],
             (instregex "^LD1i(8|16|32|64)_POST$",
                        "^LD1Rv(8b|16b|4h|8h|2s|4s|1d|2d)_POST$")>;

// SIMD load 1-element single replicate to all lanes
// HIP10CWrite_7c_1LD_1F LD1R

// SIMD load 2-elements multiple
// HIP10CWrite_8c_2LD_2F LD2
def : InstRW<[HIP10CWrite_8c_2LD_2F],
             (instregex "^LD2Twov(16b|8h|4s|2d|8b|4h|2s)$")>;
def : InstRW<[HIP10CWrite_8c_2LD_2F, WriteAdr],
             (instregex "^LD2Twov(16b|8h|4s|2d|8b|4h|2s)_POST$")>;

// SIMD load 2-element single 1 lane
// HIP10CWrite_8c_2LD_2F LD2
def : InstRW<[HIP10CWrite_8c_2LD_2F],
             (instregex "^LD2i(8|16|32|64)$",
                        "^LD2Rv(8b|16b|4h|8h|2s|4s|1d|2d)$")>;
def : InstRW<[HIP10CWrite_8c_2LD_2F, WriteAdr],
             (instregex "^LD2i(8|16|32|64)_POST$",
                        "^LD2Rv(8b|16b|4h|8h|2s|4s|1d|2d)_POST$")>;

// SIMD load 2-element single replicate to all lanes
// HIP10CWrite_8c_2LD_2F LD2R

// SIMD load LD3 (multiple structures)
// HIP10CWrite_9c_3LD_3F LD3
def : InstRW<[HIP10CWrite_9c_3LD_3F],
             (instregex "^LD3Threev(8b|4h|2s|16b|8h|4s|2d)$")>;
def : InstRW<[HIP10CWrite_9c_3LD_3F, WriteAdr],
             (instregex "^LD3Threev(8b|4h|2s|16b|8h|4s|2d)_POST$")>;

// SIMD load 3-element single 1 lane
// HIP10CWrite_8c_3LD_3F LD3
def : InstRW<[HIP10CWrite_8c_3LD_3F],
             (instregex "^LD3i(8|16|32|64)$",
                        "^LD3Rv(8b|16b|4h|8h|2s|4s|1d|2d)$")>;
def : InstRW<[HIP10CWrite_8c_3LD_3F, WriteAdr],
             (instregex "^LD3i(8|16|32|64)_POST$",
                        "^LD3Rv(8b|16b|4h|8h|2s|4s|1d|2d)_POST$")>;

// SIMD load 3-element single replicate to all lanes
// HIP10CWrite_8c_3LD_3F LD3R

// SIMD load 4-element multiple Q-form B/H/S
// HIP10CWrite_14c_8LD_8F LD4
def : InstRW<[HIP10CWrite_14c_8LD_8F],
             (instregex "^LD4Fourv(16b|8h|4s)$")>;
def : InstRW<[HIP10CWrite_14c_8LD_8F, WriteAdr],
             (instregex "^LD4Fourv(16b|8h|4s)_POST$")>;

// SIMD load 4 element multiple Q-form D
// HIP10CWrite_9c_4LD_4F LD4
def : InstRW<[HIP10CWrite_9c_4LD_4F],
             (instregex "^LD4Fourv(2d|8b|4h|2s)$")>;
def : InstRW<[HIP10CWrite_9c_4LD_4F, WriteAdr],
             (instregex "^LD4Fourv(2d|8b|4h|2s)_POST$")>;

// SIMD load 4-element multiple D-form
// HIP10CWrite_9c_4LD_4F LD4

// SIMD load 4-element single 1 lane
// HIP10CWrite_8c_4LD_4F LD4
def : InstRW<[HIP10CWrite_8c_4LD_4F],
             (instregex "^LD4i(8|16|32|64)$",
                        "^LD4Rv(8b|16b|4h|8h|2s|4s|1d|2d)$")>;
def : InstRW<[HIP10CWrite_8c_4LD_4F, WriteAdr],
             (instregex "^LD4i(8|16|32|64)_POST$",
                        "^LD4Rv(8b|16b|4h|8h|2s|4s|1d|2d)_POST$")>;

// SIMD load 4-element single replicate to all lanes
//HIP10CWrite_8c_4LD_4F LD4R

// SIMD store 1-element multiple 1 reg Q-form
//HIP10CWrite_2c_1ST_1STD ST1
def : InstRW<[HIP10CWrite_2c_1ST_1STD],
             (instregex "^ST1Onev(8b|16b|4h|8h|2s|4s|1d|2d)$",
                        "^ST1Twov(8b|4h|2s|1d)$")>;
def : InstRW<[HIP10CWrite_2c_1ST_1STD, WriteAdr],
             (instregex "^ST1Onev(8b|16b|4h|8h|2s|4s|1d|2d)_POST$",
                        "^ST1Twov(8b|4h|2s|1d)_POST$")>;

// SIMD store 1-element multiple 2 reg Q-form
// HIP10CWrite_3c_2ST_2STD ST1
def : InstRW<[HIP10CWrite_3c_2ST_2STD],
             (instregex "^ST1Twov(16b|8h|4s|2d)$")>;
def : InstRW<[HIP10CWrite_3c_2ST_2STD, WriteAdr],
             (instregex "^ST1Twov(16b|8h|4s|2d)_POST$")>;

// SIMD store 1-element multiple 3 reg Q-form
// HIP10CWrite_4c_3ST_3STD ST1
def : InstRW<[HIP10CWrite_4c_3ST_3STD],
             (instregex "^ST1Threev(16b|8h|4s|2d)$")>;
def : InstRW<[HIP10CWrite_4c_3ST_3STD, WriteAdr],
             (instregex "^ST1Threev(16b|8h|4s|2d)_POST$")>;

// SIMD store 1-element multiple 4 reg Q-form
// HIP10CWrite_5c_4ST_4STD ST1
def : InstRW<[HIP10CWrite_5c_4ST_4STD],
             (instregex "^ST1Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[HIP10CWrite_5c_4ST_4STD, WriteAdr],
             (instregex "^ST1Fourv(16b|8h|4s|2d)_POST$")>;

// SIMD store 1-element multiple 1 reg D-form
// HIP10CWrite_2c_1ST_1STD ST1

// SIMD store 1-element multiple 2 reg D-form
// HIP10CWrite_2c_1ST_1STD ST1

// SIMD store 1-element multiple 3 reg D-form
// HIP10CWrite_3c_2ST_2STD ST1
def : InstRW<[HIP10CWrite_3c_2ST_2STD],
             (instregex "^ST1Threev(8b|4h|2s|1d)$",
                        "^ST1Fourv(8b|4h|2s|1d)$")>;
def : InstRW<[HIP10CWrite_3c_2ST_2STD, WriteAdr],
             (instregex "^ST1Threev(8b|4h|2s|1d)_POST$",
                        "^ST1Fourv(8b|4h|2s|1d)_POST$")>;

// SIMD store 1-element multiple 4 reg D-form
// HIP10CWrite_3c_2ST_2STD ST1

// SIMD store 1-element single 1 lane
// HIP10CWrite_1c_1ST_1STD ST1
def : InstRW<[HIP10CWrite_1c_1ST_1STD],
             (instregex "^ST1i(8|16|32|64)$",
                        "^ST2Twov(8b|4h|2s)$",
                        "^ST2i(8|16|32|64)$")>;
def : InstRW<[HIP10CWrite_1c_1ST_1STD, WriteAdr],
             (instregex "^ST1i(8|16|32|64)_POST$",
                        "^ST2Twov(8b|4h|2s)_POST$",
                        "^ST2i(8|16|32|64)_POST$")>;

// SIMD store 2-element multiple Q-form
// HIP10CWrite_1c_2ST_2STD ST2
def : InstRW<[HIP10CWrite_1c_2ST_2STD],
             (instregex "^ST2Twov(16b|8h|4s|2d)$")>;
def : InstRW<[HIP10CWrite_1c_2ST_2STD, WriteAdr],
             (instregex "^ST2Twov(16b|8h|4s|2d)_POST$")>;

// SIMD store 2-element multiple D-form
// HIP10CWrite_1c_1ST_1STD ST2

// SIMD store 2-element single 1 lane
// HIP10CWrite_1c_1ST_1STD ST2

// SIMD store 3-element multiple Q-form
// HIP10CWrite_7c_3F_3ST_3STD ST3
def : InstRW<[HIP10CWrite_7c_3F_3ST_3STD],
             (instregex "^ST3Threev(16b|8h|4s|2d)$")>;
def : InstRW<[HIP10CWrite_7c_3F_3ST_3STD, WriteAdr],
             (instregex "^ST3Threev(16b|8h|4s|2d)_POST$")>;

// SIMD store 3-element multiple D-form
// HIP10CWrite_6c_2F_2ST_2STD ST3
def : InstRW<[HIP10CWrite_6c_2F_2ST_2STD],
             (instregex "^ST3Threev(8b|4h|2s)$")>;
def : InstRW<[HIP10CWrite_6c_2F_2ST_2STD, WriteAdr],
             (instregex "^ST3Threev(8b|4h|2s)_POST$")>;

// SIMD store 3-element single 1 lane
// HIP10CWrite_4c_3F_3ST_3STD ST3
def : InstRW<[HIP10CWrite_4c_3F_3ST_3STD],
             (instregex "^ST3i(8|16|32|64)$")>;
def : InstRW<[HIP10CWrite_4c_3F_3ST_3STD, WriteAdr],
             (instregex "^ST3i(8|16|32|64)_POST$")>;

// SIMD store 4-element multiple Q-form
// HIP10CWrite_10c_8F_8ST_8STD ST4
def : InstRW<[HIP10CWrite_10c_8F_8ST_8STD],
             (instregex "^ST4Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[HIP10CWrite_10c_8F_8ST_8STD, WriteAdr],
             (instregex "^ST4Fourv(16b|8h|4s|2d)_POST$")>;

// SIMD store 4-element multiple D-form
// HIP10CWrite_10c_6F_6ST_6STD ST4
def : InstRW<[HIP10CWrite_10c_6F_6ST_6STD],
             (instregex "^ST4Fourv(8b|4h|2s)$")>;
def : InstRW<[HIP10CWrite_10c_6F_6ST_6STD, WriteAdr],
             (instregex "^ST4Fourv(8b|4h|2s)_POST$")>;

// SIMD store 4-element single 1 lane
// HIP10CWrite_4c_4F_4ST_4STD ST4
def : InstRW<[HIP10CWrite_4c_4F_4ST_4STD],
             (instregex "^ST4i(8|16|32|64)$")>;
def : InstRW<[HIP10CWrite_4c_4F_4ST_4STD, WriteAdr],
             (instregex "^ST4i(8|16|32|64)_POST$")>;

// crypto AES ops
// HIP10CWrite_2c_1F AESD, AESE, AESMC, AESIMC

// Crypto polynomial (64x64) multiply long
// HIP10CWrite_2c_1F1 PMULL{2}
def : InstRW<[HIP10CWrite_2c_1F1], (instregex "^PMULLv")>;

// Crypto SHA1 hash acceleration ops
// HIP10CWrite_2c_1F1 SHA1H
def : InstRW<[HIP10CWrite_2c_1F1], (instregex "^SHA1(H|SU[01])rr$",
                                          "^SHA256SU[01]rr$",
                                          "^SHA512(H2?|SU[01])$",
                                          "^SM3(PARTW(1|2SM3SS1)|TT[12][AB])$")>;

// Crypto SHA1 hash acceleration ops
// HIP10CWrite_4c_1F1 SHA1C, SHA1M, SHA1P
def : InstRW<[HIP10CWrite_4c_1F1], (instregex "^SHA1[CMP]rrr$",
                                          "^SHA256H2?rrr$",
                                          "^SM4E(KEY)?$")>;

// Crypto SHA1 schedule acceleration ops
// HIP10CWrite_2c_1F1 SHA1SU0, SHA1SU1

// Crypto SHA256 hash acceleration ops
// HIP10CWrite_4c_1F1 SHA256H, SHA256H2

// Crypto SHA256 schedule acceleration ops
// HIP10CWrite_2c_1F1 SHA256SU0, SHA256SU1

// Crypto SHA512 hash acceleration ops
// HIP10CWrite_2c_1F1 SHA512H, SHA512H2, SHA512SU0, SHA512SU1

// Crypto SHA3 ops
// HIP10CWrite_1c_1F1 BCAX, EOR3, RAX1, XAR
def : InstRW<[HIP10CWrite_1c_1F1], (instrs BCAX, EOR3, RAX1, XAR)>;

// Crypto SM3 ops
// HIP10CWrite_2c_1F1 SM3PARTW1, SM3PARTW2, SM3SS1, SM3TT1A, SM3TT1B, SM3TT2A, SM3TT2B

// Crypto SM4 ops
// HIP10CWrite_4c_1F1 SM4E, SM4EKEY

// CRC checksum ops
// HIP10CWrite_2c_1M CRC32, CRC32C
def : InstRW<[HIP10CWrite_2c_1M], (instregex "^CRC32C?[BHWX]rr$")>;

// Loop control based on predicate SVE256
// HIP10CWrite_1c_2F BRKA, BRKB
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^BRK[AB]_PP[mz]P$")>;
def : InstRW<[HIP10CWrite_1c_2F], (instrs BRKN_PPzP, BRKPA_PPzPP, BRKPB_PPzPP)>;

// Loop control based on predicate and flag setting SVE256
// HIP10CWrite_1c_2F BRKAS, BRKBS
def : InstRW<[HIP10CWrite_1c_2F], (instrs BRKAS_PPzP, BRKBS_PPzP, BRKNS_PPzP,
                                       BRKPAS_PPzPP, BRKPBS_PPzPP)>;

// Loop control propagating SVE256
// HIP10CWrite_1c_2F BRKN, BRKPA, BRKPB

// Loop control propagating and flag setting SVE256
// HIP10CWrite_1c_2F BRKNS, BRKPAS, BRKPBS

// Loop control based on GPR
// HIP10CWrite_2c_1S01 WHILELE, WHILELO, WHILELS, WHILELT
def : InstRW<[HIP10CWrite_2c_1S01], (instregex "^WHILE(LE|LO|LS|LT)_P(WW|XX)_[BHSD]$")>;

// Loop terminate
// HIP10CWrite_1c_1S23 CTERMEQ, CTERMNE
def : InstRW<[HIP10CWrite_1c_1S23], (instregex "^CTERM(EQ|NE)_(WW|XX)$")>;

// Predicate counting scalar
// HIP10CWrite_1c_1S ADDPL, ADDVL, CNTB, CNTH, CNTW, CNTD, DECB, DECH, DECW, DECD, INCB, INCH, INCW, INCD, RDVL
def : InstRW<[HIP10CWrite_1c_1S], (instrs ADDPL_XXI, ADDVL_XXI, RDVLI_XI)>;
def : InstRW<[HIP10CWrite_1c_1S], (instregex "^(CNT)[BHWD]_XPiI$",
                                                "^(DEC|INC)[BHWD]_XPiI$")>;

// Predicate counting scalar
// HIP10CWrite_2c_1S23 SQDECB, SQDECH, SQDECW, SQDECD, SQINCB, SQINCH, SQINCW, SQINCD, UQDECB, UQDECH, UQDECW, UQDECD,  UQINCB, UQINCH, UQINCW, UQINCD
def : InstRW<[HIP10CWrite_2c_1S23], (instregex "^([SU]Q)(DEC|INC)[BHWD]_XPiI$",
                                          "^SQ(DEC|INC)[BHWD]_XPiWdI$",
                                          "^UQ(DEC|INC)[BHWD]_WPiI$")>;

// Predicate counting scalar active predicate SVE256
// HIP10CWrite_5c_2F_4S DECP, INCP
def : InstRW<[HIP10CWrite_5c_2F_4S], (instregex
                                          "^(DEC|INC)P_XP_[BHSD]$")>;

// Predicate counting scalar active predicate SVE256
// HIP10CWrite_1c_2F CNTP
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^CNTP_XPP_[BHSD]$")>;

// Predicate counting scalar active predicate SVE256
// HIP10CWrite_6c_2F_2M SQDECP, SQINCP, UQDECP, UQINCP
def : InstRW<[HIP10CWrite_6c_2F_2M], (instregex
                                          "^([SU]Q)(DEC|INC)P_XP_[BHSD]$",
                                          "^UQ(DEC|INC)P_WP_[BHSD]$",
                                          "^[SU]Q(DEC|INC)P_XPWd_[BHSD]$")>;

// Predicate counting vector active predicate SVE256
// HIP10CWrite_4c_4F DECP, INCP, SQDECP, SQINCP, UQDECP, UQINCP
def : InstRW<[HIP10CWrite_4c_4F], (instregex "^([SU]Q)?(DEC|INC)P_ZP_[HSD]$")>;

// Predicate logical SVE256
// HIP10CWrite_1c_2F AND, BIC, EOR, MOV, NAND, NOR, NOT, ORN, ORR
def : InstRW<[HIP10CWrite_1c_2F],
             (instregex "^(AND|BIC|EOR|NAND|NOR|ORN|ORR)_PPzPP$")>;

// Predicate logical flag setting SVE256
// HIP10CWrite_1c_2F ANDS, BICS, EORS, NANDS, NORS, NOTS, ORNS, ORRS
def : InstRW<[HIP10CWrite_1c_2F],
             (instregex "^(AND|BIC|EOR|NAND|NOR|ORN|ORR)S_PPzPP$")>;

// Predicate reverse SVE256
// HIP10CWrite_1c_2F REV
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^REV_PP_[BHSD]$",
                                          "^PFALSE$", "^PFIRST_B$",
                                          "^PNEXT_[BHSD]$", "^PTRUE_[BHSD]$",
                                          "^TRN[12]_PPP_[BHSDQ]$",
                                          "^(ZIP|UZP)[12]_PPP_[BHSDQ]$")>;
def : InstRW<[HIP10CWrite_1c_2F], (instrs PTEST_PP,
                                       PUNPKHI_PP, PUNPKLO_PP)>;
def : InstRW<[HIP10CWrite_1c_2F], (instrs SEL_PPPP)>;
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^PTRUES_[BHSD]$")>;

// Predicate select SVE256
// HIP10CWrite_1c_2F SEL

// Predicate set SVE256
// HIP10CWrite_1c_2F PFALSE, PTRUE

// Predicate set/initialize set flags SVE256
// HIP10CWrite_1c_2F PTRUES

// Predicate find first/next SVE256
// HIP10CWrite_1c_2F PFIRST, PNEXT

// Predicate test SVE256
// HIP10CWrite_1c_2F PTEST

// Predicate transpose SVE256
// HIP10CWrite_1c_2F TRN1, TRN2

// Predicate unpack and widen SVE256
// HIP10CWrite_1c_2F PUNPKHI, PUNPKLO

// Predicate zip/unzip SVE256
// HIP10CWrite_1c_2F ZIP1, ZIP2, UZP1, UZP2

// Arithmetic absolute diff SABD UABD SVE256
// HIP10CWrite_2c_2F SABD, UABD

// Arithmetic address generation SVE256
// HIP10CWrite_2c_2F ADR

// Arithmetic basic SVE256
// HIP10CWrite_1c_2F ABS, ADD, SUB, SUBR, NEG, CNOT
def : InstRW<[HIP10CWrite_1c_2F],
             (instregex "^(ABS|CNOT|NEG)_ZPmZ_[BHSD]$",
                        "^(ADD|SUB)_Z(I|P[mZ]Z|ZZ)_[BHSD]$",
                        "^ADR_[SU]XTW_ZZZ_D_[0123]$",
                        "^ADR_LSL_ZZZ_[SD]_[0123]$",
                        "^[SU]ABD_ZP[mZ]Z_[BHSD]$",
                        "^[SU](MAX|MIN)_Z(I|P[mZ]Z)_[BHSD]$",
                        "^[SU]Q(ADD|SUB)_Z(I|ZZ)_[BHSD]$",
                        "^SUBR_Z(I|P[mZ]Z)_[BHSD]$",
                        "^(AND|EOR|ORR)_ZI$",
                        "^(AND|BIC|EOR|EOR(BT|TB)?|ORR)_ZZZ$",
                        "^EOR(BT|TB)_ZZZ_[BHSD]$",
                        "^(AND|BIC|EOR|NOT|ORR)_ZPmZ_[BHSD]$")>;

// Arithmetic complex SVE256
// HIP10CWrite_1c_2F SQADD, SQSUB, UQADD, UQSUB

// Arithmetic shift SVE256
// HIP10CWrite_3c_2F ASR, LSR, LSL, ASRR, LSLR, LSRR
def : InstRW<[HIP10CWrite_3c_2F],
             (instregex "^(ASR|LSL|LSR)_WIDE_Z(Pm|Z)Z_[BHS]",
                        "^(ASR|LSL|LSR)_ZPm[IZ]_[BHSD]",
                        "^(ASR|LSL|LSR)_ZZI_[BHSD]",
                        "^(ASR|LSL|LSR)_ZPZ[IZ]_[BHSD]",
                        "^(ASRR|LSLR|LSRR)_ZPmZ_[BHSD]")>;

// Arithmetic shift right for divide SVE256
// HIP10CWrite_3c_2F ASRD
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^ASRD_ZP[mZ]I_[BHSD]$")>;

// Count/reverse bits SVE256
// HIP10CWrite_1c_2F CLS, CLZ, RBIT
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^(CLS|CLZ|RBIT)_ZPmZ_[BHSD]$")>;

// Count/reverse bits B H S form SVE256
// HIP10CWrite_1c_2F CNT
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^(CNT)_ZPmZ_[BHS]$")>;

// Count/reverse bits D-form SVE256
// HIP10CWrite_2c_2F CNT
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^(CNT)_ZPmZ_[D]$")>;

// Broadcast logical bitmask immediate to vector SVE256
// HIP10CWrite_1c_2F DUPM, MOV
def : InstRW<[HIP10CWrite_1c_2F], (instrs DUPM_ZI)>;

// Compare and set flags SVE256
// HIP10CWrite_1c_2F CMPEQ, CMPGE, CMPGT, CMPHI, CMPHS, CMPLE, CMPLO, CMPLS, CMPLT, CMPNE
def : InstRW<[HIP10CWrite_1c_2F],
             (instregex "^CMP(EQ|GE|GT|HI|HS|LE|LO|LS|LT|NE)_PPzZ[IZ]_[BHSD]$",
                        "^CMP(EQ|GE|GT|HI|HS|LE|LO|LS|LT|NE)_WIDE_PPzZZ_[BHS]$")>;

// Conditional extract operations scalar form SVE256
// HIP10CWrite_5c_2F_2S23 CLASTA, CLASTB
def : InstRW<[HIP10CWrite_5c_2F_2S23], (instregex "^CLAST[AB]_RPZ_[BHSD]$")>;

// Conditional extract operations SIMD&FP scalar and vector forms SVE256
// HIP10CWrite_2c_2F CLASTA, CLASTB
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^CLAST[AB]_[VZ]PZ_[BHSD]$")>;

// Conditional extract operations SIMD&FP scalar and vector forms SVE256
// HIP10CWrite_7c_6F SPLICE
def : InstRW<[HIP10CWrite_7c_6F], (instregex "^SPLICE_ZPZZ?_[BHSD]$")>;

// Conditional extract operations SIMD&FP scalar and vector forms SVE256
// HIP10CWrite_5c_4F COMPACT
def : InstRW<[HIP10CWrite_5c_4F], (instregex "^COMPACT_ZPZ_[SD]$")>;

// Convert to floating point SVE256
// HIP10CWrite_3c_4F SCVTF, UCVTF
def : InstRW<[HIP10CWrite_3c_4F], (instregex "^[SU]CVTF_ZPmZ_Dto[HSD]",
                                          "^[SU]CVTF_ZPmZ_StoD",
                                          "^[SU]CVTF_ZPmZ_Sto[HS]$",
                                          "^[SU]CVTF_ZPmZ_HtoH$")>;

// Copy scalar SVE256
// HIP10CWrite_5c_2S23_2F CPY
def : InstRW<[HIP10CWrite_5c_2S23_2F], (instregex "^CPY_ZPmR_[BHSD]$")>;

// Copy imm SVE256
// HIP10CWrite_1c_2F CPY
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^CPY_ZP([mz]I)_[BHSD]$")>;

// Copy scalar SIMD&FP SVE256
// HIP10CWrite_2c_2F CPY
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^CPY_ZP(mV)_[BHSD]$")>;

// Divides 32-bit SVE256
// HIP10CWrite_17c_26F SDIV, SDIVR, UDIV, UDIVR
def : InstRW<[HIP10CWrite_17c_26F], (instregex "^[SU]DIVR?_ZPmZ_S$")>;

// Divides 64-bit SVE256
// HIP10CWrite_17c_26F SDIV, SDIVR, UDIV, UDIVR
def : InstRW<[HIP10CWrite_17c_26F], (instregex "^[SU]DIVR?_ZPmZ_D$")>;

// Dot product SVE256
// HIP10CWrite_3c_2F SDOT, UDOT
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^[SU]DOT_ZZZI?_[DS]$")>;
def : InstRW<[HIP10CWrite_3c_2F], (instrs SUDOT_ZZZI, USDOT_ZZZ, USDOT_ZZZI)>;

// Dot product 8-bit using signed and unsigned integers SVE256
// HIP10CWrite_3c_2F SUDOT, USDOT

// Duplicate indexed SVE256
// HIP10CWrite_2c_2F DUP, MOV
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^DUP_ZZI_[BHSDQ]$")>;

// Duplicate immediate SVE256
// HIP10CWrite_1c_2F DUP, MOV
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^DUP_ZI_[BHSD]$")>;

// Duplicate scalar SVE256
// HIP10CWrite_5c_2S23_2F DUP, MOV
def : InstRW<[HIP10CWrite_5c_2S23_2F], (instregex "^DUP_ZR_[BHSD]$")>;

// Extend sign or zero SVE256
// HIP10CWrite_1c_2F SXTB, SXTH, SXTW, UXTB, UXTH, UXTW
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^[SU]XTB_ZPmZ_[HSD]$",
                                          "^[SU]XTH_ZPmZ_[SD]$",
                                          "^[SU]XTW_ZPmZ_[D]$")>;

// Extract SVE256
// HIP10CWrite_2c_2F EXT
def : InstRW<[HIP10CWrite_2c_2F], (instrs EXT_ZZI)>;

// Insert operation scalar SVE256
// HIP10CWrite_5c_2S23_2F INSR
def : InstRW<[HIP10CWrite_5c_2S23_2F], (instregex "^INSR_ZR_[BHSD]$")>;

// Insert operation SIMD and FP scalar SVE256
// HIP10CWrite_2c_2F INSR
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^LAST[AB]_VPZ_[BHSD]$",
                                          "^INSR_ZV_[BHSD]$")>;

// Extract operation SIMD and FP scalar SVE256
// HIP10CWrite_2c_2F LASTA, LASTB

// Extract operation scalar SVE256
// HIP10CWrite_2c_2F LASTA, LASTB
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^LAST[AB]_RPZ_[BHSD]$")>;

// Horizontal operations B H S form immediate operands only SVE256
// HIP10CWrite_3c_2F INDEX
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^INDEX_II_[BHS]$")>;

// Horizontal operations D form immediate operands only SVE256
// HIP10CWrite_4c_4F INDEX
def : InstRW<[HIP10CWrite_4c_4F], (instrs INDEX_II_D)>;

// Horizontal operations B H S form scalar start immediate/scalar increment SVE256
// HIP10CWrite_10c_4S23_4F INDEX
def : InstRW<[HIP10CWrite_10c_4S23_4F], (instregex "^INDEX_(RI|RR)_[BHS]$")>;

// Horizontal operations D form scalar start immediate/scalar increment SVE256
// HIP10CWrite_11c_6S23_6F INDEX
def : InstRW<[HIP10CWrite_11c_6S23_6F], (instregex "^INDEX_(RI|RR)_[D]$")>;

// Horizontal operations B H S form immediate start scalar increment SVE256
// HIP10CWrite_6c_2S23_2F INDEX
def : InstRW<[HIP10CWrite_6c_2S23_2F], (instregex "^INDEX_(IR)_[BHS]$")>;

// Horizontal operations D form scalar immediate start scalar increment SVE256
// HIP10CWrite_7c_4S23_4F INDEX
def : InstRW<[HIP10CWrite_7c_4S23_4F], (instregex "^INDEX_(IR)_[D]$")>;

// Logical SVE256
// HIP10CWrite_1c_2F AND, ORR, EOR, BIC, NOT, EON, MOV, ORN

// Max/min basic and pairwise SVE256
// HIP10CWrite_1c_2F SMAX, SMIN, UMAX, UMIN

// Matrix multiply-accumulate SVE256
// HIP10CWrite_5c_4F SMMLA, UMMLA, USMMLA
def : InstRW<[HIP10CWrite_5c_4F], (instrs SMMLA_ZZZ, UMMLA_ZZZ, USMMLA_ZZZ)>;

// Move prefix SVE256
// HIP10CWrite_1c_2F MOVPRFX
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^MOVPRFX_ZP[mz]Z_[BHSD]$",
                                           "^MOVPRFX_ZZ$")>;

// Multiply B H S element size SVE256
// HIP10CWrite_3c_2F MUL, SMULH, UMULH
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^MUL_(ZI|ZPmZ)_[BHS]$",
                                          "^[SU]MULH_(ZPmZ|ZZZ)_[BHS]$")>;

// Multiply D element size SVE256
// HIP10CWrite_4c_4F MUL, SMULH, UMULH
def : InstRW<[HIP10CWrite_4c_4F], (instregex "^MUL_(ZI|ZPmZ)_D$",
                                          "^[SU]MULH_ZPmZ_D$")>;

// Multiply accumulate B H S element size SVE256
// HIP10CWrite_3c_2F MLA, MLS, MAD, MSB
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^(ML[AS]|MAD|MSB)_ZPmZZ_[BHS]")>;

// Multiply accumulate D element size SVE256
// HIP10CWrite_4c_4F MLA, MLS, MAD, MSB
def : InstRW<[HIP10CWrite_4c_4F], (instregex
                                          "^(MLA|MLS|MAD|MSB)_ZPmZZ_D$")>;

// Predicate counting vector SVE256
// HIP10CWrite_1c_2F SQINCH, SQINCW, SQINCD, SQDECH, SQDECW, SQDECD, INCH, INCW, INCD, DECH, DECW, DECD, UQINCH, UQINCW, UQINCD, UQDECH, UQDECW, UQDECD
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^([SU]Q)?(DEC|INC)[HWD]_ZPiI$")>;

// Predicate counting vector B H S SVE256
// HIP10CWrite_1c_2F CNT

// Predicate counting vector D SVE256
// HIP10CWrite_2c_2F CNT

// Reduction arithmetic SVE256
// HIP10CWrite_2c_2F SADDV, UADDV
def : InstRW<[HIP10CWrite_2c_2F],
             (instregex "^[SU](ADD)V_VPZ_[BHSD]")>;

// Reduction arithmetic B H S form SVE256
// HIP10CWrite_3c_2F SMAXV, SMINV, UMAXV, UMINV
def : InstRW<[HIP10CWrite_3c_2F],
             (instregex "^[SU](MAX|MIN)V_VPZ_[BHS]")>;

// Reduction arithmetic D form SVE256
// HIP10CWrite_2c_2F SMAXV, SMINV, UMAXV, UMINV
def : InstRW<[HIP10CWrite_2c_2F],
             (instregex "^[SU](MAX|MIN)V_VPZ_D")>;

// Reduction logical SVE256
// HIP10CWrite_1c_2F ANDV, EORV, ORV
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^(AND|EOR|OR)V_VPZ_[BHSD]$")>;

// Reverse vector SVE256
// HIP10CWrite_1c_2F REV
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^REV_ZZ_[BHSD]$",
                                           "^REVB_ZPmZ_[HSD]$",
                                           "^REVH_ZPmZ_[SD]$",
                                           "^REVW_ZPmZ_D$")>;

// Reverse vector SVE256
// HIP10CWrite_1c_2F REVB, REVH, REVW

// Select vector form SVE256
// HIP10CWrite_1c_2F MOV, SEL
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^SEL_ZPZZ_[BHSD]$",
                                           "^TRN[12]_ZZZ_[BHSDQ]$"
                                           )>;


// Table lookup SVE256
// HIP10CWrite_2c_2F TBL
def : InstRW<[HIP10CWrite_2c_2F], (instregex
                                           "^TB[LX]_ZZZ_[BHSD]$",
                                           "^[SU]UNPK(HI|LO)_ZZ_[HSD]$",
                                           "^(UZP|ZIP)[12]_ZZZ_[BHSDQ]$")>;

// Transpose vector form SVE256
// HIP10CWrite_1c_2F TRN1, TRN2

// Unpack and extend SVE256
// HIP10CWrite_2c_2F SUNPKHI, SUNPKLO, UUNPKHI, UUNPKLO

// Zip/unzip SVE256
// HIP10CWrite_2c_2F UZP1, UZP2, ZIP1, ZIP2

// Floating point absolute value SVE256
// HIP10CWrite_1c_2F FABS
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^FAB[S]_ZPmZ_[HSD]$",
                                           "^FNEG_ZPmZ_[HSD]$")>;

// Floating point negative value SVE256
// HIP10CWrite_1c_2F FNEG

// Floating point absolute difference SVE256
// HIP10CWrite_2c_2F FABD
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^FAB[D]_ZPmZ_[HSD]$",
                                           "^F(ADD|SUB)_(ZPm[IZ]|ZZZ)_[HSD]$",
                                           "^FADDP_ZPmZZ_[HSD]$",
                                           "^FSUBR_ZPm[IZ]_[HSD]$")>;

// Floating point arithmetic SVE256
// HIP10CWrite_2c_2F FADD, FSUB, FSUBR

// Floating point associative add, F16
def : InstRW<[HIP10CWrite_10c_18F], (instrs FADDA_VPZ_H)>;

// Floating point associative add, F32
def : InstRW<[HIP10CWrite_6c_10F], (instrs FADDA_VPZ_S)>;

// Floating point associative add, F64
def : InstRW<[HIP10CWrite_4c_1F], (instrs FADDA_VPZ_D)>;

// Floating point compare SVE256
// HIP10CWrite_2c_2F FACGE, FACGT, FACLE, FACLT, FCMEQ, FCMGE, FCMGT, FCMLE, FCMLT, FCMNE, FCMUO
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^FAC(GE|GT)_PPzZZ_[HSD]$",
                                          "^FCM(EQ|GE|GT|NE|UO)_PPzZZ_[HSD]$",
                                          "^FCM(EQ|GE|GT|LE|LT|NE)_PPzZ0_[HSD]$")>;

// Floating point complex add SVE256
// HIP10CWrite_3c_2F FCADD
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^FCADD_ZPmZ_[HSD]$")>;

// Floating point complex multiply add SVE256
// HIP10CWrite_5c_2F FCMLA
def : InstRW<[HIP10CWrite_5c_2F], (instregex "^FCMLA_ZPmZZ_[HSD]$",
                                           "^FCMLA_ZZZI_[HS]$")>;

// Floating point complex multiply add index SVE256
// HIP10CWrite_5c_2F FCMLA

// Floating point convert long or narrow SVE256
// HIP10CWrite_3c_4F FCVT
def : InstRW<[HIP10CWrite_3c_4F], (instregex "^FCVT_ZPmZ_",
                                          "^FCVTZ[SU]_ZPmZ_")>;

// Floating point convert to integer SVE256
// HIP10CWrite_3c_4F FCVTZS, FCVTZU

// Floating point copy SVE256
// HIP10CWrite_1c_2F FCPY, FDUP, FMOV
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^FCPY_ZPmI_[HSD]$",
                                           "^FDUP_ZI_[HSD]$")>;

// Floating point divide F16 SVE256
// HIP10CWrite_13c_18F FDIV, FDIVR
def : InstRW<[HIP10CWrite_13c_18F], (instregex "^FDIVR?_ZPmZ_H$")>;

// Floating point divide F32 SVE256
// HIP10CWrite_13c_18F FDIV, FDIVR
def : InstRW<[HIP10CWrite_13c_18F], (instregex "^FDIVR?_ZPmZ_S$")>;

// Floating point divide F64 SVE256
// HIP10CWrite_15c_22F FDIV, FDIVR
def : InstRW<[HIP10CWrite_15c_22F], (instregex "^FDIVR?_ZPmZ_D$")>;

// Floating point min/max SVE256
// HIP10CWrite_2c_2F FMAX, FMIN, FMAXNM, FMINNM
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^F(MAX|MIN)(NM)?_ZPm[IZ]_[HSD]$")>;

// Floating point multiply SVE256
// HIP10CWrite_3c_2F FMUL
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^F(SCALE|MULX)_ZPmZ_[HSD]$",
                                           "^FMUL_(ZPm[IZ]|ZZZI?)_[HSD]$")>;

// Floating point multiply SVE256
// HIP10CWrite_3c_2F FSCALE, FMULX

// Floating point multiply accumulate SVE256
// HIP10CWrite_4c_2F FMLA, FMLS, FMAD, FMSB, FNMAD, FNMLA, FNMLS, FNMSB
def : InstRW<[HIP10CWrite_4c_2F], (instregex "^F(N?M(AD|SB)|N?ML[AS])_ZPmZZ_[HSD]$",
                                           "^FML[AS]_ZZZI_[HSD]$",
                                           "^F(RECPS|RSQRTS)_ZZZ_[HSD]$")>;

// Floating point reciprocal estimate SVE256
// HIP10CWrite_3c_4F FRECPE, FRECPX, FRSQRTE
def : InstRW<[HIP10CWrite_3c_4F], (instrs FRECPE_ZZ_H, FRSQRTE_ZZ_H,
                                       FRECPE_ZZ_S, FRSQRTE_ZZ_S,
                                       FRECPE_ZZ_D, FRSQRTE_ZZ_D)>;
def : InstRW<[HIP10CWrite_3c_4F], (instregex "^FRECPX_ZPmZ_[HSD]$")>;

// Floating point reciprocal step SVE256
// HIP10CWrite_4c_2F FRECPS, FRSQRTS

// Floating point reduction F16 SVE256
// HIP10CWrite_9c_8F FADDV
def : InstRW<[HIP10CWrite_9c_8F], (instregex "^F(ADD|((MAX|MIN)(NM)?))V_VPZ_H$")>;

// Floating point reduction F32 SVE256
// HIP10CWrite_7c_6F FADDV

// Floating point reduction F64 SVE256
// HIP10CWrite_5c_4F FADDV

// Floating point reduction F16 F32
// HIP10CWrite_3c_2F FMAXNMV, FMAXV, FMINNMV, FMINV
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^F(ADD|((MAX|MIN)(NM)?))V_VPZ_S$")>;

// Floating point reduction F64
// HIP10CWrite_2c_2F FMAXNMV, FMAXV, FMINNMV, FMINV
def : InstRW<[HIP10CWrite_2c_2F], (instregex "^F(ADD|((MAX|MIN)(NM)?))V_VPZ_D$")>;

// Floating point round to integral SVE256
// HIP10CWrite_3c_4F FRINTA, FRINTM, FRINTN, FRINTP, FRINTX, FRINTZ
def : InstRW<[HIP10CWrite_3c_4F], (instregex "^FRINT[AIMNPXZ]_ZPmZ_[HSD]$")>;

// Floating point square root F16 SVE256
// HIP10CWrite_21c_34F FSQRT
def : InstRW<[HIP10CWrite_21c_34F], (instrs FSQRT_ZPmZ_H)>;

// Floating point square root F32 SVE256
// HIP10CWrite_21c_34F FSQRT
def : InstRW<[HIP10CWrite_21c_34F], (instrs FSQRT_ZPmZ_S)>;

// Floating point square root F64 SVE256
// HIP10CWrite_25c_42F FSQRT
def : InstRW<[HIP10CWrite_25c_42F], (instrs FSQRT_ZPmZ_D)>;

// Floating point trigonometric exponentiation SVE256
// HIP10CWrite_3c_2F FEXPA
def : InstRW<[HIP10CWrite_3c_2F], (instregex "^FEXPA_ZZ_[HSD]$")>;

// Floating point trigonometric multiply add SVE256
// HIP10CWrite_4c_2F FTMAD
def : InstRW<[HIP10CWrite_4c_2F], (instregex "^FTMAD_ZZI_[HSD]$",
                                           "^FTS(MUL)_ZZZ_[HSD]$")>;

// Floating point trigonometric miscellaneous SVE256
// HIP10CWrite_4c_2F FTSMUL

// Floating point trigonometric miscellaneous SVE256
// HIP10CWrite_1c_2F FTSSEL
def : InstRW<[HIP10CWrite_1c_2F], (instregex "^FTS(SEL)_ZZZ_[HSD]$")>;

// Convert F32 to BF16 SVE256
// HIP10CWrite_3c_4F BFCVT, BFCVTNT
def : InstRW<[HIP10CWrite_3c_4F], (instrs BFCVT_ZPmZ, BFCVTNT_ZPmZ)>;

// Dot product SVE256
// HIP10CWrite_6c_4F BFDOT
def : InstRW<[HIP10CWrite_6c_4F], (instrs BFDOT_ZZI, BFDOT_ZZZ)>;

// Matrix multiply accumulate SVE256
// HIP10CWrite_9c_8F BFMMLA
def : InstRW<[HIP10CWrite_9c_8F], (instrs BFMMLA_ZZZ)>;

// Multiply accumulate long SVE256
// HIP10CWrite_5c_2F BFMLALB, BFMLALT, BFMLALB, BFMLALT
def : InstRW<[HIP10CWrite_5c_2F], (instregex "^BFMLAL[BT]_ZZZ(I)?$")>;

// Load vector
// HIP10CWrite_6c_1LD LDR
def : InstRW<[HIP10CWrite_6c_1LD], (instrs LDR_ZXI)>;

// Load predicate SVE256
// HIP10CWrite_8c_2LD_2F LDR
def : InstRW<[HIP10CWrite_8c_2LD_2F], (instrs LDR_PXI)>;

// Contiguous load scalar + imm
// HIP10CWrite_6c_1LD LD1B, LD1H, LD1W, LD1D, LD1SB, LD1SH, LD1SW
def : InstRW<[HIP10CWrite_6c_1LD], (instregex "^LD1[BHWD]_IMM_REAL$",
                                           "^LD1S?B_[HSD]_IMM_REAL$",
                                           "^LD1S?H_[SD]_IMM_REAL$",
                                           "^LD1S?W_D_IMM_REAL$",
                                           "^LD1[BWD]$",
                                           "^LD1S?B_[HSD]$",
                                           "^LD1S?W_D$")>;
def : InstRW<[HIP10CWrite_6c_1LD], (instregex "^LD1H$",
                                              "^LD1S?H_[SD]$")>;

// Contiguous load scalar + scalar
// HIP10CWrite_6c_1LD LD1B, LD1H, LD1W, LD1D, LD1SB, LD1SH, LD1SW

// Contiguous load broadcast scalar + imm SVE256
// HIP10CWrite_8c_2LD_2F LD1RB, LD1RH, LD1RW, LD1RD, LD1RSB, LD1RSH, LD1RSW, LD1RQB, LD1RQH, LD1RQW, LD1RQD
def : InstRW<[HIP10CWrite_8c_2LD_2F], (instregex "^LD1R[BHWD]_IMM$",
                                           "^LD1RSW_IMM$",
                                           "^LD1RS?B_[HSD]_IMM$",
                                           "^LD1RS?H_[SD]_IMM$",
                                           "^LD1RS?W_D_IMM$",
                                           "^LD1RQ_[BHWD]_IMM$",
                                           "^LD1RQ_[BWD]$")>;
def : InstRW<[HIP10CWrite_8c_2LD_2F], (instregex "^LD1RQ_H$")>;

// Contiguous load broadcast scalar + scalar SVE256
// HIP10CWrite_8c_2LD_2F LD1RQB, LD1RQH, LD1RQW, LD1RQD

// Non-temporal load scalar + imm
// HIP10CWrite_6c_1LD LDNT1B, LDNT1H, LDNT1W, LDNT1D
def : InstRW<[HIP10CWrite_6c_1LD], (instregex "^LDNT1[BHWD]_ZRI$")>;

// Non-temporal load scalar + scalar
// HIP10CWrite_6c_1LD LDNT1B, LDNT1H, LDNT1W, LDNT1D
def : InstRW<[HIP10CWrite_6c_1LD], (instrs LDNT1H_ZRR)>;
def : InstRW<[HIP10CWrite_6c_1LD], (instregex "^LDNT1[BWD]_ZRR$")>;

// Contiguous first faulting load scalar + scalar
// HIP10CWrite_6c_1LD LDFF1B, LDFF1H, LDFF1W, LDFF1D, LDFF1SB, LDFF1SH, LDFF1SW, LDFF1SD
def : InstRW<[HIP10CWrite_6c_1LD], (instregex "^LDFF1H_REAL$",
                                              "^LDFF1S?H_[SD]_REAL$")>;
def : InstRW<[HIP10CWrite_6c_1LD], (instregex "^LDFF1[BWD]_REAL$",
                                              "^LDFF1S?B_[HSD]_REAL$",
                                              "^LDFF1S?W_D_REAL$")>;

// Contiguous non-faulting load scalar + imm
// HIP10CWrite_6c_1LD LDNF1B, LDNF1D, LDNF1H, LDNF1W, LDNF1SB, LDNF1SH, LDNF1SW
// Contiguous non faulting load, scalar + imm
def : InstRW<[HIP10CWrite_6c_1LD], (instregex "^LDNF1[BHWD]_IMM_REAL$",
                                           "^LDNF1S?B_[HSD]_IMM_REAL$",
                                           "^LDNF1S?H_[SD]_IMM_REAL$",
                                           "^LDNF1S?W_D_IMM_REAL$")>;

// Contiguous Load two structures to two vectors scalar + imm SVE256
// HIP10CWrite_9c_4LD_4F LD2B, LD2D, LD2H, LD2W
def : InstRW<[HIP10CWrite_9c_4LD_4F], (instregex "^LD2[BHWD]_IMM$")>;

// Contiguous Load two structures to two vectors scalar + scalar SVE256
// HIP10CWrite_9c_4LD_4F LD2B, LD2D, LD2H, LD2W
def : InstRW<[HIP10CWrite_9c_4LD_4F], (instrs LD2H)>;
def : InstRW<[HIP10CWrite_9c_4LD_4F],  (instregex "^LD2[BWD]$")>;

// Contiguous Load three structures to three vectors scalar + imm SVE256
// HIP10CWrite_11c_6LD_6F LD3B, LD3D, LD3H, LD3W
def : InstRW<[HIP10CWrite_11c_6LD_6F], (instregex "^LD3[BHWD]_IMM$")>;

// Contiguous Load three structures to three vectors scalar + scalar SVE256
// HIP10CWrite_11c_6LD_6F LD3B, LD3D, LD3H, LD3W
def : InstRW<[HIP10CWrite_11c_6LD_6F], (instregex "^LD3[BHWD]$")>;

// Contiguous Load four structures to four vectors scalar + imm SVE256
// HIP10CWrite_16c_16LD_16F LD4B, LD4D, LD4H, LD4W
def : InstRW<[HIP10CWrite_16c_16LD_16F], (instregex "^LD4[BHWD]_IMM$")>;

// Contiguous Load four structures to four vectors scalar + scalar SVE256
// HIP10CWrite_16c_16LD_16F LD4B, LD4D, LD4H, LD4W
def : InstRW<[HIP10CWrite_16c_16LD_16F], (instregex "^LD4[BHWD]$")>;

// Gather load vector + imm 32-bit element size
// HIP10CWrite_18c_8LD LD1B, LD1H, LD1W, LD1SB, LD1SH, LD1SW, LDFF1B, LDFF1H, LDFF1W, LDFF1SB, LDFF1SH, LDFF1SW
def : InstRW<[HIP10CWrite_18c_8LD], (instregex "^GLD(FF)?1S?[BH]_S_IMM_REAL$",
                                             "^GLD(FF)?1W_IMM_REAL$")>;

// Gather load vector + imm 64-bit element size
// HIP10CWrite_16c_4LD LD1B, LD1H, LD1W, LD1D, LD1SB, LD1SH, LD1SW, LDFF1B, LDFF1D, LDFF1H, LDFF1W, LDFF1SB, LDFF1SD, LDFF1SH, LDFF1SW
def : InstRW<[HIP10CWrite_16c_4LD],
             (instregex "^GLD(FF)?1S?[BHW]_D_IMM_REAL$",
                        "^GLD(FF)?1S?[BHW]_D_([SU]XTW_)?(SCALED_)?REAL$",
                        "^GLD(FF)?1D_IMM_REAL$",
                        "^GLD(FF)?1D_([SU]XTW_)?(SCALED_)?REAL$")>;

// Gather load 32-bit scaled offset
// HIP10CWrite_18c_8LD LD1H, LD1SH, LDFF1H, LDFF1SH, LD1W, LDFF1W, LDFF1SW
def : InstRW<[HIP10CWrite_18c_8LD],
             (instregex "^GLD(FF)?1S?[HW]_S_[SU]XTW_SCALED_REAL$",
                        "^GLD(FF)?1W_[SU]XTW_SCALED_REAL")>;

// Gather load 32-bit unpacked unscaled offset
// HIP10CWrite_16c_4LD LD1B, LD1SB, LDFF1B, LDFF1SB, LD1D, LDFF1D, LD1H, LD1SH, LDFF1H, LDFF1SH, LD1W, LD1SW, LDFF1W, LDFF1SW
def : InstRW<[HIP10CWrite_16c_4LD],
             (instregex "^GLD(FF)?1S?[BH]_S_[SU]XTW_REAL$",
                        "^GLD(FF)?1W_[SU]XTW_REAL$")>;

def : InstRW<[HIP10CWrite_4c_1LD], (instregex "^PRF[BHWD]")>;

// Store from predicate reg SVE256
// HIP10CWrite_3c_2F_2ST_2STD STR
def : InstRW<[HIP10CWrite_3c_2F_2ST_2STD], (instrs STR_PXI)>;

// Store from vector reg
// HIP10CWrite_1c_2ST_2STD STR
def : InstRW<[HIP10CWrite_1c_2ST_2STD], (instrs STR_ZXI)>;

// Contiguous store scalar + imm
// HIP10CWrite_1c_2ST_2STD ST1B, ST1H, ST1W, ST1D
def : InstRW<[HIP10CWrite_1c_2ST_2STD], (instregex "^ST1[BHWD]_IMM$",
                                              "^ST1B_[HSD]_IMM$",
                                              "^ST1H_[SD]_IMM$",
                                              "^ST1W_D_IMM$",
                                              "^ST1[BWD]$",
                                              "^ST1B_[HSD]$",
                                              "^ST1W_D$")>;
def : InstRW<[HIP10CWrite_1c_2ST_2STD], (instregex "^ST1H(_[SD])?$")>;

// Contiguous store scalar + scalar
// HIP10CWrite_1c_2ST_2STD ST1B, ST1H, ST1W, ST1D

// Contiguous store two structures from two vectors scalar + imm SVE256
// HIP10CWrite_6c_4F_4ST_4STD ST2B, ST2H, ST2W, ST2D
def : InstRW<[HIP10CWrite_6c_4F_4ST_4STD], (instregex "^ST2[BHWD]_IMM$",
                                              "^ST2[BWD]$")>;
def : InstRW<[HIP10CWrite_6c_4F_4ST_4STD], (instrs ST2H)>;

// Contiguous store two structures from two vectors scalar + scalar SVE256
// HIP10CWrite_6c_4F_4ST_4STD ST2B, ST2H, ST2W, ST2D

// Contiguous store three structures from three vectors scalar + imm SVE256
// HIP10CWrite_8c_6F_6ST_6STD ST3B, ST3H, ST3W, ST3D
def : InstRW<[HIP10CWrite_8c_6F_6ST_6STD], (instregex "^ST3[BHWD]_IMM$")>;

// Contiguous store three structures from three vectors scalar + scalar SVE256
// HIP10CWrite_8c_6F_6ST_6STD ST3B, ST3H, ST3W, ST3D
def : InstRW<[HIP10CWrite_8c_6F_6ST_6STD], (instregex "^ST3[BHWD]$")>;

// Contiguous store four structures from four vectors scalar + imm SVE256
// HIP10CWrite_13c_16F_16ST_16STD ST4B, ST4H, ST4W, ST4D
def : InstRW<[HIP10CWrite_13c_16F_16ST_16STD], (instregex "^ST4[BHWD]_IMM$")>;

// Contiguous store four structures from four vectors scalar + scalar SVE256
// HIP10CWrite_13c_16F_16ST_16STD ST4B, ST4H, ST4W, ST4D
def : InstRW<[HIP10CWrite_13c_16F_16ST_16STD], (instregex "^ST4[BHWD]$")>;

// Non-temporal store scalar + imm
// HIP10CWrite_1c_2ST_2STD STNT1B, STNT1H, STNT1W, STNT1D
def : InstRW<[HIP10CWrite_1c_2ST_2STD], (instregex "^STNT1[BHWD]_ZRI$",
                                              "^STNT1[BWD]_ZRR$")>;
def : InstRW<[HIP10CWrite_1c_2ST_2STD], (instrs STNT1H_ZRR)>;

// Non-temporal store scalar + scalar
// HIP10CWrite_1c_2ST_2STD STNT1B, STNT1H, STNT1W, STNT1D

// Scatter store vector + imm 32-bit element size
// HIP10CWrite_8c_8ST_8STD ST1B, ST1H, ST1W
def : InstRW<[HIP10CWrite_8c_8ST_8STD], (instregex "^SST1[BH]_S_IMM$",
                                               "^SST1W_IMM$",
                                               "^SST1(H_S|W)_[SU]XTW_SCALED$",
                                               "^SST1[BH]_S_[SU]XTW$",
                                               "^SST1W_[SU]XTW$")>;

// Scatter store vector + imm 64-bit element size
// HIP10CWrite_4c_4ST_4STD ST1B, ST1H, ST1W, ST1D
def : InstRW<[HIP10CWrite_4c_4ST_4STD], (instregex "^SST1[BHW]_D_IMM$",
                                              "^SST1D_IMM$",
                                              "^SST1[HW]_D_SCALED$",
                                              "^SST1D_SCALED$",
                                              "^SST1[BHW]_D$",
                                              "^SST1D$")>;

// Scatter store 32-bit scaled offset
// HIP10CWrite_8c_8ST_8STD ST1H, ST1W

// Scatter store 32-bit unpacked unscaled offset
// HIP10CWrite_8c_8ST_8STD ST1B, ST1H, ST1W, ST1D
def : InstRW<[HIP10CWrite_8c_8ST_8STD], (instregex "^SST1[BHW]_D_[SU]XTW$",
                                              "^SST1D_[SU]XTW$",
                                              "^SST1[HW]_D_[SU]XTW_SCALED$",
                                              "^SST1D_[SU]XTW_SCALED$")>;

// Scatter store 32-bit unpacked scaled offset
// HIP10CWrite_8c_8ST_8STD ST1H, ST1W, ST1D

// Scatter store 32-bit unscaled offset
// HIP10CWrite_8c_8ST_8STD ST1B, ST1H, ST1W

// Scatter store 64-bit scaled offset
// HIP10CWrite_4c_4ST_4STD ST1H, ST1W, ST1D

// Scatter store 64-bit unscaled offset
// HIP10CWrite_4c_4ST_4STD ST1B, ST1H, ST1W, ST1D

// Read first fault register unpredicated SVE256
// HIP10CWrite_1c_2F RDFFR
def : InstRW<[HIP10CWrite_1c_2F], (instrs RDFFR_P_REAL)>;

// Read first fault register predicated SVE256
// HIP10CWrite_1c_2F RDFFR
def : InstRW<[HIP10CWrite_1c_2F], (instrs RDFFR_PPz_REAL)>;

// Read first fault register and set flags SVE256
// HIP10CWrite_1c_2F RDFFRS
def : InstRW<[HIP10CWrite_1c_2F], (instrs RDFFRS_PPz)>;

// Write to first fault register
def : InstRW<[HIP10CWrite_9c_18F_9M1], (instrs WRFFR)>;

// Set first fault register
def : InstRW<[HIP10CWrite_0c], (instrs SETFFR)>;

}
