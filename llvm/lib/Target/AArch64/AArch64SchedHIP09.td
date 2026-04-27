//=- AArch64SchedHIP09.td - Huawei HIP09 Scheduling Defs ---*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the machine model for Huawei HIP09 to support instruction
// scheduling and other instruction cost heuristics.
//
//===----------------------------------------------------------------------===//

def HIP09Model : SchedMachineModel {
  let IssueWidth            =   6; // HIP09 can dispatch 6 micro-ops per cycle.
  let MicroOpBufferSize     =  88; // Based on the reorder buffer.
  let LoadLatency           =   4; // Basic latency for most load instructions.
  let MispredictPenalty     =  14; // Based on ALU pipeline depth.
  let LoopMicroOpBufferSize =  16; // Based on the instruction queue size.
  let CompleteModel         =   1;

  list<Predicate> UnsupportedFeatures = !listconcat(PAUnsupported.F,
                                                    SMEUnsupported.F,
                                                    SVE2Unsupported.F,
                                                   [HasMTE, HasSVE2p1_or_HasSME]);
}

let SchedModel = HIP09Model in {

// HIP09 has 18 pipelines. The 4 Advanced SIMD&FP units handle different
// sets of operations, of which 2 can also handle SVE.

// These are also defined in the upstream AArch64SchedHIP09.td.
// In our implementation, HIP09UnitAB is called HIP09UnitBRU instead.
def HIP09UnitBRU      : ProcResource<2>;   // Branch 0/1
def HIP09UnitALUS0    : ProcResource<1>;   // Integer ALU single cycle 0
def HIP09UnitALUS1    : ProcResource<1>;   // Integer ALU single cycle 1
def HIP09UnitALUS23   : ProcResource<2>;   // Integer ALU single cycle 2/3
def HIP09UnitALUM0    : ProcResource<1>;   // Integer ALU multi cycle 0
def HIP09UnitALUM1    : ProcResource<1>;   // Integer ALU multi cycle 1
def HIP09UnitLD       : ProcResource<2>;   // Load address generation and special memory 0/1
def HIP09UnitST       : ProcResource<2>;   // Store address generation and special memory 0/1
def HIP09UnitFSU0     : ProcResource<1>;   // SIMD&FP 0, can handle sve
def HIP09UnitFSU2     : ProcResource<1>;   // SIMD&FP 2, can handle sve
def HIP09UnitFSU13    : ProcResource<2>;   // SIMD&FP 1/3
def HIP09UnitSTD      : ProcResource<2>;   // Store data 0/1

def HIP09UnitALUS01   : ProcResGroup<[HIP09UnitALUS0, HIP09UnitALUS1]>;
def HIP09UnitALUS     : ProcResGroup<[HIP09UnitALUS0, HIP09UnitALUS1, HIP09UnitALUS23]>;
def HIP09UnitALUM     : ProcResGroup<[HIP09UnitALUM0, HIP09UnitALUM1]>;
def HIP09UnitFSU02    : ProcResGroup<[HIP09UnitFSU0, HIP09UnitFSU2]>;
def HIP09UnitFSU      : ProcResGroup<[HIP09UnitFSU0, HIP09UnitFSU2, HIP09UnitFSU13]>;

//===----------------------------------------------------------------------===//
//
// Contains all of the HIP09-specific SchedWriteRes types. The approach below
// is to define a generic SchedWriteRes for every combination of latency and
// micro-ops. The naming conventions is to use a prefix, one field for latency,
// and one or more microOp count/type designators.
//
//   Prefix: HIP09Write
//   Latency: #cyc
//   Micro-op Count/Types: #(BRU|ALUS01|ALUS23|ALUS|ALUM1|ALUM2|ALUM|LD|ST|FSU0|FSU2|FSU02|FSU|STD)
//
// e.g. HIP09Write_6cyc_1ALUS_6LD_4FSU means the total latency is 6 cycles,
// and 11 micro-ops are issued down 1 ALUS pipe, 6 LD pipes, and 4 FSU
// pipes

def HIP09Write_0cyc                       : SchedWriteRes<[]>                 { let Latency = 0; }

def HIP09Write_1cyc_1BRU                  : SchedWriteRes<[HIP09UnitBRU]>     { let Latency = 1; }

def HIP09Write_1cyc_1ALUS                 : SchedWriteRes<[HIP09UnitALUS]>    { let Latency = 1; }
def HIP09Write_1cyc_1ALUS1                : SchedWriteRes<[HIP09UnitALUS1]>   { let Latency = 1; }
def HIP09Write_1cyc_1ALUS01               : SchedWriteRes<[HIP09UnitALUS01]>  { let Latency = 1; }
def HIP09Write_2cyc_1ALUS01               : SchedWriteRes<[HIP09UnitALUS01]>  { let Latency = 2; }
def HIP09Write_3cyc_1ALUS01               : SchedWriteRes<[HIP09UnitALUS01]>  { let Latency = 3; }
def HIP09Write_1cyc_1ALUS23               : SchedWriteRes<[HIP09UnitALUS23]>  { let Latency = 1; }
def HIP09Write_2cyc_1ALUS23               : SchedWriteRes<[HIP09UnitALUS23]>  { let Latency = 2; }

def HIP09Write_2cyc_1ALUM                 : SchedWriteRes<[HIP09UnitALUM]>    { let Latency = 2; }
def HIP09Write_3cyc_1ALUM                 : SchedWriteRes<[HIP09UnitALUM]>    { let Latency = 3; }
def HIP09Write_5cyc_1ALUM1                : SchedWriteRes<[HIP09UnitALUM1]>   { let Latency = 5; }
def HIP09Write_12cyc_1ALUM0_12RC          : SchedWriteRes<[HIP09UnitALUM0]>   { let Latency = 12; let ResourceCycles = [12]; }
def HIP09Write_20cyc_1ALUM0_20RC          : SchedWriteRes<[HIP09UnitALUM0]>   { let Latency = 20; let ResourceCycles = [20]; }

def HIP09Write_1cyc_1ST                   : SchedWriteRes<[HIP09UnitST]>      { let Latency = 1; }

def HIP09Write_1cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 1; }
def HIP09Write_2cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 2; }
def HIP09Write_3cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 3; }
def HIP09Write_4cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 4; }
def HIP09Write_5cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 5; }
def HIP09Write_5cyc_1FSU_3RC              : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 5;  let ResourceCycles = [3]; }
def HIP09Write_7cyc_1FSU_3RC              : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 7;  let ResourceCycles = [3]; }
def HIP09Write_9cyc_1FSU_5RC              : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 9;  let ResourceCycles = [5]; }
def HIP09Write_9cyc_1FSU_8RC              : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 9;  let ResourceCycles = [8]; }
def HIP09Write_10cyc_1FSU_6RC             : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 10; let ResourceCycles = [6]; }
def HIP09Write_13cyc_1FSU_9RC             : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 13; let ResourceCycles = [9]; }
def HIP09Write_15cyc_1FSU_11RC            : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 15; let ResourceCycles = [11]; }
def HIP09Write_21cyc_1FSU_17RC            : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 21; let ResourceCycles = [17]; }
def HIP09Write_25cyc_1FSU_21RC            : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 25; let ResourceCycles = [21]; }
def HIP09Write_1cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 1; }
def HIP09Write_2cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 2; }
def HIP09Write_3cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 3; }
def HIP09Write_4cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 4; }
def HIP09Write_4cyc_1FSU02_4RC            : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 4;  let ResourceCycles = [4]; }
def HIP09Write_5cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 5; }
def HIP09Write_7cyc_1FSU02_3RC            : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 7;  let ResourceCycles = [3]; }
def HIP09Write_9cyc_1FSU02_3RC            : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 9;  let ResourceCycles = [3]; }
def HIP09Write_12cyc_1FSU02_4RC           : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 12; let ResourceCycles = [4]; }
def HIP09Write_12cyc_1FSU02_8RC           : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 12; let ResourceCycles = [8]; }
def HIP09Write_13cyc_1FSU02_9RC           : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 13; let ResourceCycles = [9]; }
def HIP09Write_15cyc_1FSU02_11RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 15; let ResourceCycles = [11]; }
def HIP09Write_17cyc_1FSU02_13RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 17; let ResourceCycles = [13]; }
def HIP09Write_20cyc_1FSU02_16RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 20; let ResourceCycles = [16]; }
def HIP09Write_21cyc_1FSU02_17RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 21; let ResourceCycles = [17]; }
def HIP09Write_25cyc_1FSU02_21RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 25; let ResourceCycles = [21]; }
def HIP09Write_36cyc_1FSU02_32RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 36; let ResourceCycles = [32]; }
def HIP09Write_1cyc_1FSU2                 : SchedWriteRes<[HIP09UnitFSU2]>    { let Latency = 1; }
def HIP09Write_2cyc_1FSU2                 : SchedWriteRes<[HIP09UnitFSU2]>    { let Latency = 2; }
def HIP09Write_4cyc_1FSU2                 : SchedWriteRes<[HIP09UnitFSU2]>    { let Latency = 4; }
def HIP09Write_4cyc_1LD                   : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 4; }
def HIP09Write_5cyc_1LD                   : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 5; }
def HIP09Write_6cyc_1LD                   : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 6; }
def HIP09Write_6cyc_1LD_3RC               : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 6;  let ResourceCycles = [3]; }
def HIP09Write_6cyc_1LD_4RC               : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 6;  let ResourceCycles = [4]; }
def HIP09Write_16cyc_1LD_4RC              : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 16; let ResourceCycles = [4]; }
def HIP09Write_18cyc_1LD_4RC              : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 18; let ResourceCycles = [4]; }

def HIP09Write_1cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 1;
  let NumMicroOps = 2;
}
def HIP09Write_2cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}
def HIP09Write_2cyc_2FSU02                : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}

def HIP09Write_3cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 3;
  let NumMicroOps = 2;
}

def HIP09Write_3cyc_2FSU02                : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 3;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_2FSU02                : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_4FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 4;
  let NumMicroOps = 4;
}

def HIP09Write_5cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_2FSU02                : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_4FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 6;
  let NumMicroOps = 4;
}

def HIP09Write_7cyc_4FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 7;
  let NumMicroOps = 4;
}

def HIP09Write_9cyc_4FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 9;
  let NumMicroOps = 4;
}

def HIP09Write_6cyc_1BRU_1ALUM1           : SchedWriteRes<[HIP09UnitBRU, HIP09UnitALUM1]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_1cyc_1ST_1STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitSTD]>
{
  let Latency = 1;
  let NumMicroOps = 2;
}

def HIP09Write_1cyc_2ST_2STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 1;
  let NumMicroOps = 4;
}

def HIP09Write_2cyc_1ST_1STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitSTD]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}

def HIP09Write_2cyc_2ST_2STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 2;
  let NumMicroOps = 4;
}

def HIP09Write_2cyc_4ST_4STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 2;
  let NumMicroOps = 8;
}

def HIP09Write_3cyc_2ST_2STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 3;
  let NumMicroOps = 4;
}

def HIP09Write_4cyc_3ST_3STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 4;
  let NumMicroOps = 6;
}

def HIP09Write_4cyc_8ST_8STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 4;
  let NumMicroOps = 16;
}


def HIP09Write_5cyc_4ST_4STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 5;
  let NumMicroOps = 8;
}

def HIP09Write_1cyc_1ST_1STD_1ALUS        : SchedWriteRes<[HIP09UnitST, HIP09UnitSTD, HIP09UnitALUS]>
{
  let Latency = 1;
  let NumMicroOps = 3;
}

def HIP09Write_2cyc_1ST_1STD_1ALUS        : SchedWriteRes<[HIP09UnitST, HIP09UnitSTD, HIP09UnitALUS]>
{
  let Latency = 2;
  let NumMicroOps = 3;
}

def HIP09Write_2cyc_2ST_2STD_2ALUS        : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitALUS, HIP09UnitALUS]>
{
  let Latency = 2;
  let NumMicroOps = 6;
}

def HIP09Write_2cyc_1BRU_1ALUS23          : SchedWriteRes<[HIP09UnitBRU, HIP09UnitALUS23]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_1LD_1ALUS             : SchedWriteRes<[HIP09UnitLD, HIP09UnitALUS]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_1LD_1ALUS             : SchedWriteRes<[HIP09UnitLD, HIP09UnitALUS]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_9cyc_1LD_1ALUM1            : SchedWriteRes<[HIP09UnitLD, HIP09UnitALUM1]>
{
  let Latency = 9;
  let NumMicroOps = 2;
}

def HIP09Write_1cyc_1ST_1ALUM             : SchedWriteRes<[HIP09UnitST, HIP09UnitALUM]>
{
  let Latency = 1;
  let NumMicroOps = 2;
}

def HIP09Write_2cyc_1ST_1ALUM             : SchedWriteRes<[HIP09UnitST, HIP09UnitALUM]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_1ALUS01_1FSU          : SchedWriteRes<[HIP09UnitALUS01, HIP09UnitFSU]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_1cyc_1ST_1ALUS_1ALUM       : SchedWriteRes<[HIP09UnitST, HIP09UnitALUS, HIP09UnitALUM]>
{
  let Latency = 1;
  let NumMicroOps = 3;
}

def HIP09Write_3cyc_1ST_1ALUS_1ALUM       : SchedWriteRes<[HIP09UnitST, HIP09UnitALUS, HIP09UnitALUM]>
{
  let Latency = 3;
  let NumMicroOps = 3;
}

def HIP09Write_4cyc_1FSU_1ALUS23          : SchedWriteRes<[HIP09UnitFSU, HIP09UnitALUS23]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_7cyc_1ALUS01_1FSU_1ALUS23  : SchedWriteRes<[HIP09UnitALUS01, HIP09UnitFSU, HIP09UnitALUS23]>
{
  let Latency = 7;
  let NumMicroOps = 3;
}

def HIP09Write_5cyc_1FSU02_1ALUS          : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitALUS]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_1FSU02_1ALUS          : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitALUS]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_5cyc_1ALUS01_1FSU          : SchedWriteRes<[HIP09UnitALUS01, HIP09UnitFSU]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_5cyc_1FSU_1ALUS23          : SchedWriteRes<[HIP09UnitFSU, HIP09UnitALUS23]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_5cyc_1FSU02_1ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitALUS23]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_4FSU02_4ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 6;
  let NumMicroOps = 8;
}

def HIP09Write_7cyc_4FSU02_4ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 7;
  let NumMicroOps = 8;
}

def HIP09Write_6cyc_6FSU02_6ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 6;
  let NumMicroOps = 12;
}

def HIP09Write_7cyc_6FSU02_6ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 7;
  let NumMicroOps = 12;
}

def HIP09Write_8cyc_6FSU02_6ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 8;
  let NumMicroOps = 12;
}

def HIP09Write_9cyc_6FSU02_6ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 9;
  let NumMicroOps = 12;
}

def HIP09Write_9cyc_18FSU02_9ALUM1        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitALUM1, HIP09UnitALUM1,
                                                              HIP09UnitALUM1, HIP09UnitALUM1, HIP09UnitALUM1, HIP09UnitALUM1,
                                                              HIP09UnitALUM1, HIP09UnitALUM1, HIP09UnitALUM1]>
{
  let Latency = 9;
  let NumMicroOps = 27;
}

def HIP09Write_6cyc_2LD                   : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_7cyc_1LD_1FSU              : SchedWriteRes<[HIP09UnitLD, HIP09UnitFSU]>
{
  let Latency = 7;
  let NumMicroOps = 2;
}

def HIP09Write_8cyc_1LD_1FSU              : SchedWriteRes<[HIP09UnitLD, HIP09UnitFSU]>
{
  let Latency = 8;
  let NumMicroOps = 2;
}

def HIP09Write_8cyc_2LD_2FSU              : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 8;
  let NumMicroOps = 4;
}

def HIP09Write_8cyc_2LD_2FSU02            : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 8;
  let NumMicroOps = 4;
}

def HIP09Write_9cyc_3LD_3FSU              : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 9;
  let NumMicroOps = 6;
}

def HIP09Write_9cyc_4LD_4FSU02            : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 9;
  let NumMicroOps = 8;
}

def HIP09Write_11cyc_6LD_6FSU02            : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 11;
  let NumMicroOps = 12;
}

def HIP09Write_16cyc_16LD_16FSU02         : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 16;
  let NumMicroOps = 32;
}

def HIP09Write_12cyc_8LD_8FSU             : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 12;
  let NumMicroOps = 16;
}

def HIP09Write_13cyc_8LD_8FSU             : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 13;
  let NumMicroOps = 16;
}

def HIP09Write_3cyc_1FSU02_1ST_1STD       : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitST, HIP09UnitSTD]>
{
  let Latency = 3;
  let NumMicroOps = 3;
}

def HIP09Write_4cyc_1FSU_1ST_1STD         : SchedWriteRes<[HIP09UnitFSU, HIP09UnitST, HIP09UnitSTD]>
{
  let Latency = 4;
  let NumMicroOps = 3;
}

def HIP09Write_6cyc_2FSU_2ST_2STD         : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 6;
  let NumMicroOps = 6;
}

def HIP09Write_6cyc_3FSU02_3ST_3STD       : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 6;
  let NumMicroOps = 9;
}

def HIP09Write_6cyc_4FSU02_4ST_4STD       : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 6;
  let NumMicroOps = 12;
}

def HIP09Write_7cyc_3FSU_3ST_3STD         : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 7;
  let NumMicroOps = 9;
}

def HIP09Write_8cyc_16FSU02_16ST_16STD    : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 8;
  let NumMicroOps = 48;
}

def HIP09Write_10cyc_6FSU_6ST_6STD        : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 10;
  let NumMicroOps = 18;
}

def HIP09Write_10cyc_8FSU_8ST_8STD        : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 10;
  let NumMicroOps = 24;
}

//===----------------------------------------------------------------------===//
// Map the target-defined scheduler read/write resources and latency for HIP09.
// The aliases are sufficient for creating a coarse, working model. As the model
// evolves, InstRWs will be used to override some of these SchedAliases.
//
// WARNING: Using SchedAliases is convenient and works well for latency and
//          resource lookup for instructions. However, this creates an entry in
//          AArch64WriteLatencyTable with a WriteResourceID of 0, breaking
//          any SchedReadAdvance since the lookup will fail.

def : SchedAlias<WriteVd,     HIP09Write_2cyc_1FSU>;
def : SchedAlias<WriteVq,     HIP09Write_2cyc_1FSU>;
def : SchedAlias<WriteVLD,    HIP09Write_5cyc_1LD>;
def : SchedAlias<WriteVST,    HIP09Write_1cyc_1ST>;

def : WriteRes<WriteAtomic,   []>                 { let Unsupported = 1; }
def : WriteRes<WriteBarrier,  []>                 { let Latency = 1; }
def : WriteRes<WriteHint,     []>                 { let Latency = 1; }
def : WriteRes<WriteLDHi,     []>                 { let Latency = 4; }
def : WriteRes<WriteIM32,     [HIP09UnitALUM]>    { let Latency = 3; }
def : WriteRes<WriteIM64,     [HIP09UnitALUM]>    { let Latency = 4; }

// Forwarding logic is only modeled for multiply and accumulate.
def : ReadAdvance<ReadI,        0>;
def : ReadAdvance<ReadISReg,    0>;
def : ReadAdvance<ReadIEReg,    0>;
def : ReadAdvance<ReadIM,       0>;
def : ReadAdvance<ReadIMA,      2, [WriteIM32, WriteIM64]>;
def : ReadAdvance<ReadID,       0>;
def : ReadAdvance<ReadExtrHi,   0>;
def : ReadAdvance<ReadAdrBase,  0>;
def : ReadAdvance<ReadVLD,      0>;
def : ReadAdvance<ReadST,       0>;


//===----------------------------------------------------------------------===//
// Specialize the coarse model by associating instruction groups with the
// subtarget-defined types. As the model is refined, this will override most
// of the above SchedAlias mappings.

//Miscellaneous
// -----------------------------------------------------------------------------

def : InstRW<[WriteI],                                          (instrs COPY)>;

// Branch Instructions
// -----------------------------------------------------------------------------

def : SchedAlias<WriteBr,                                       HIP09Write_1cyc_1BRU>;
def : SchedAlias<WriteBrReg,                                    HIP09Write_1cyc_1BRU>;

// Branch, immed
def : InstRW<[HIP09Write_1cyc_1BRU],                            (instrs B, Bcc)>;

// Branch, register
// Compare and branch
def : InstRW<[HIP09Write_1cyc_1BRU],                            (instregex "^(BR|RET|(CBZ|CBNZ|TBZ|TBNZ))$")>;

// Branch and link, immed
// Branch and link, register
def : InstRW<[HIP09Write_2cyc_1BRU_1ALUS23],                    (instrs BL, BLR)>;

// Arithmetic and Logical Instructions
// -----------------------------------------------------------------------------
def : SchedAlias<WriteI,                                        HIP09Write_1cyc_1ALUS>;
def : SchedAlias<WriteISReg,                                    HIP09Write_2cyc_1ALUM>;
def : SchedAlias<WriteIEReg,                                    HIP09Write_2cyc_1ALUM>;

// Convert floating-point condition flags
// Flag manipulation instructions
def : WriteRes<WriteSys, []>                                    { let Latency = 1; }

// ALU, basic
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(ADD|AND|EOR|ORR|SUB)[WX]r(r|i)$")>;
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(ADC|SBC)[WX]r$")>;
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(BIC|EON|ORN)[WX]rr$")>;

// ALU, basic, flagset
def : InstRW<[HIP09Write_1cyc_1ALUS23],                         (instregex "^(ADD|AND|SUB)S[WX]r(r|i)$")>;
def : InstRW<[HIP09Write_1cyc_1ALUS23],                         (instregex "^(ADC|SBC)S[WX]r$")>;
def : InstRW<[HIP09Write_1cyc_1ALUS23],                         (instregex "^BICS[WX]rr$")>;

// Shifted Register with Shift == 0
def HIP09WriteISReg : SchedWriteVariant<[
      SchedVar<RegShiftedPred,                                  [WriteISReg]>,
      SchedVar<NoSchedPred,                                     [WriteI]>]>;
def : InstRW<[HIP09WriteISReg],                                 (instregex "^(ADD|AND|EON|EOR|ORN|ORR|SUB)[WX]rs$")>;

def HIP09WrISReg23 : SchedWriteVariant<[
      SchedVar<RegShiftedPred,                                  [WriteIEReg]>,
      SchedVar<NoSchedPred,                                     [HIP09Write_1cyc_1ALUS23]>]>;
def : InstRW<[HIP09WrISReg23],                                  (instregex "^(ADD|AND|BIC|SUB)S[WX]rs$")>;

// Extended Register with Extend == 0
def HIP09WrIEReg : SchedWriteVariant<[
      SchedVar<RegExtendedPred,                                 [WriteIEReg]>,
      SchedVar<NoSchedPred,                                     [WriteI]>]>;
def : InstRW<[HIP09WrIEReg],                                    (instregex "^(ADD|SUB)[WX]r(x|x64)$")>;

def HIP09WrIEReg23 : SchedWriteVariant<[
      SchedVar<RegExtendedPred,                                 [WriteISReg]>,
      SchedVar<NoSchedPred,                                     [HIP09Write_1cyc_1ALUS23]>]>;
def : InstRW<[HIP09WrIEReg23],                                  (instregex "^(ADD|SUB)S[WX]r(x|x64)$")>;

// Conditional compare
def : InstRW<[HIP09Write_1cyc_1ALUS23],                         (instregex "^(CCMN|CCMP)[WX](r|i)$")>;

// Conditional select
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(CSEL|CSINC|CSINV|CSNEG)[WX]r$")>;

//Convert floating-point condition flags
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(AX|XA)FLAG$")>;

// Flag manipulation instructions
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instrs SETF8, SETF16, RMIF, CFINV)>;

// Logical, shift no flagset
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^BIC[WX]rs$")>;

// Divide and Multiply Instructions
// -----------------------------------------------------------------------------

def : SchedAlias<WriteID32,                                     HIP09Write_12cyc_1ALUM0_12RC>;
def : SchedAlias<WriteID64,                                     HIP09Write_20cyc_1ALUM0_20RC>;

//Divide, W-form
def : InstRW<[HIP09Write_12cyc_1ALUM0_12RC],                    (instregex "^(S|U)DIVWr$")>;

//Divide, X-form
def : InstRW<[HIP09Write_20cyc_1ALUM0_20RC],                    (instregex "^(S|U)DIVXr$")>;

// Multiply, W-form
// Multiply accumulate, W-form
def HIP09ReadMAW : SchedReadAdvance<2,                          [HIP09Write_2cyc_1ALUM]>;
def : InstRW<[HIP09Write_2cyc_1ALUM, HIP09ReadMAW],             (instrs MADDWrrr, MSUBWrrr)>;

// Multiply, x-form
// Multiply accumulate, X-form
def HIP09ReadMAQ : SchedReadAdvance<3,                          [HIP09Write_3cyc_1ALUM]>;
def : InstRW<[HIP09Write_3cyc_1ALUM, HIP09ReadMAQ],             (instrs MADDXrrr, MSUBXrrr)>;

// Multiply accumulate long
// Multiply long
def : InstRW<[HIP09Write_2cyc_1ALUM, HIP09ReadMAW],             (instregex "(S|U)(MADDL|MSUBL)rrr")>;

// Multiply high
def : InstRW<[HIP09Write_3cyc_1ALUM],                           (instregex "^(S|U)MULHrr$")>;

//Pointer Authentication Instructions
// -----------------------------------------------------------------------------

// Bitfield move, basic
def : SchedAlias<WriteIS,                                       HIP09Write_1cyc_1FSU>;

// Authenticate data address
def : InstRW<[HIP09Write_5cyc_1ALUM1],                          (instregex "^AUTDZ?[AB]$")>;

// Authenticate instruction address
def : InstRW<[HIP09Write_5cyc_1ALUM1],                          (instregex "^AUTI[AB](1716|SP|Z)?$", "^AUTIZ[AB]$")>;

// Branch and link, register, with pointer authentication
def : InstRW<[HIP09Write_6cyc_1BRU_1ALUM1],                     (instregex "^BLRA[AB]Z?$")>;

// Branch, register, with pointer authentication
def : InstRW<[HIP09Write_6cyc_1BRU_1ALUM1],                     (instregex "^BRA[AB]Z?$")>;

// Branch, return, with pointer authentication
def : InstRW<[HIP09Write_6cyc_1BRU_1ALUM1],                     (instregex "^RETA[AB]$")>;

// Compute pointer authentication code for data address
def : InstRW<[HIP09Write_5cyc_1ALUM1],                          (instregex "^PACDZ?[AB]$")>;

// Compute pointer authentication code, using generic key
def : InstRW<[HIP09Write_5cyc_1ALUM1],                          (instrs PACGA)>;

// Compute pointer authentication code for instruction address
def : InstRW<[HIP09Write_5cyc_1ALUM1],                          (instregex "^PACI[AB](1716|SP|Z)?$", "^PACIZ[AB]$")>;

// Load register, with pointer authentication
def : InstRW<[HIP09Write_9cyc_1LD_1ALUM1],                      (instregex "^LDRA[AB](indexed|writeback)$")>;

// Strip pointer authentication code
def : InstRW<[HIP09Write_1cyc_1ALUS1],                          (instrs XPACD, XPACI, XPACLRI)>;

// Exception return, with pointer authentication
def : InstRW<[HIP09Write_5cyc_1ALUM1],                          (instregex "^ERETA[AB]$")>;

// Load Instructions
// -----------------------------------------------------------------------------

def : WriteRes<WriteLD,     [HIP09UnitLD]>                      { let Latency = 4; }
def : WriteRes<WriteLDIdx,  [HIP09UnitLD]>                      { let Latency = 4; }

// Pre/Post Indexing
def : WriteRes<WriteAdr,    [HIP09UnitALUS]>                    { let Latency = 1; }

// Load register, literal
def : InstRW<[HIP09Write_4cyc_1LD],                             (instregex "^LDR(W|X)l$")>;
def : InstRW<[HIP09Write_4cyc_1LD],                             (instrs LDRSWl)>;
def : InstRW<[HIP09Write_4cyc_1LD],                             (instrs PRFMl)>;

// Load register, unscaled immed
def : InstRW<[HIP09Write_4cyc_1LD],                             (instregex "^LDUR(W|X|BB|HH)i$")>;
def : InstRW<[HIP09Write_4cyc_1LD],                             (instregex "^LDURS(BW|BX|HW|HX|W)i$")>;
def : InstRW<[HIP09Write_4cyc_1LD],                             (instrs PRFUMi)>;

// Load register, immed post-index
// Load register, immed pre-index
def : InstRW<[WriteAdr, HIP09Write_4cyc_1LD],                   (instregex "^LDR(BB|HH|W|X)(post|pre)$")>;
def : InstRW<[WriteAdr, HIP09Write_4cyc_1LD],                   (instregex "^LDRS(BW|BX|HW|HX|W)(post|pre)$")>;

// Load register, immed unprivileged
def : InstRW<[HIP09Write_4cyc_1LD],                             (instregex "^LDTR(W|X|B|H)i$")>;
def : InstRW<[HIP09Write_4cyc_1LD],                             (instregex "^LDTRS(BW|BX|HW|HX|W)i$")>;

// Load register, unsigned immed
def : InstRW<[HIP09Write_4cyc_1LD],                             (instregex "^LDR(W|X|BB|HH)ui$")>;
def : InstRW<[HIP09Write_4cyc_1LD],                             (instregex "^LDRS(BW|BX|HW|HX|W)ui$")>;
def : InstRW<[HIP09Write_4cyc_1LD],                             (instrs PRFMui)>;

// Load register, register offset
def : InstRW<[HIP09Write_5cyc_1LD],                             (instregex "^LDR(W|X|BB)ro(W|X)$")>;
def : InstRW<[HIP09Write_5cyc_1LD],                             (instregex "^LDRS(BW|BX|W)ro(W|X)$")>;
def : InstRW<[HIP09Write_5cyc_1LD],                             (instregex "^PRFMro(W|X)$")>;

// Load register, register offset, extend, scale by 2
def : InstRW<[HIP09Write_6cyc_1LD_1ALUS],                       (instregex "^LDR(HH|SHW|SHX)ro(W|X)$")>;

// Load pair, immed offset
def : InstRW<[HIP09Write_4cyc_1LD, WriteLDHi],                  (instregex "^LDP(W|X|SW)i$")>;
def : InstRW<[HIP09Write_4cyc_1LD, WriteLDHi],                  (instregex "^LDNP(W|X)i$")>;

// Load pair, immed post-index
def : InstRW<[WriteAdr, HIP09Write_4cyc_1LD_1ALUS, WriteLDHi],  (instregex "^LDP[WX]post$")>;
def : InstRW<[WriteAdr, HIP09Write_4cyc_1LD_1ALUS, WriteLDHi],  (instrs LDPSWpost)>;

// Load pair, immed pre-index
def : InstRW<[WriteAdr, HIP09Write_4cyc_1LD_1ALUS, WriteLDHi],  (instregex "^LDP[WX]pre$")>;
def : InstRW<[WriteAdr, HIP09Write_4cyc_1LD_1ALUS, WriteLDHi],  (instrs LDPSWpre)>;

// Miscellaneous Data-Processing Instructions
// -----------------------------------------------------------------------------

def : SchedAlias<WriteImm,                                      HIP09Write_1cyc_1ALUS>;
def : SchedAlias<WriteExtr,                                     HIP09Write_1cyc_1ALUS>;

// Address generation
def : InstRW<[HIP09Write_1cyc_1ALUS23],                         (instrs ADR, ADRP)>;

// Bitfield extract, one reg
// Bitfield extract, two reg
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^EXTR(W|X)rri$")>;

// Bitfield move, basic
// Bitfield move, insert
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(S|U)?BFM(W|X)ri$")>;

// Move immed
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^MOV[NZK][WX]i$")>;

// Count leading
// Reverse bit/bytes
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(CLS|CLZ|RBIT|REV(16|32)?)(W|X)r$")>;

// Variable shift
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(ASRV|LSLV|LSRV|RORV)(W|X)r$")>;

// Store instructions
// -----------------------------------------------------------------------------
def : WriteRes<WriteST,     [HIP09UnitST]>                      { let Latency = 1; }
def : WriteRes<WriteSTP,    [HIP09UnitST]>                      { let Latency = 1; }
def : WriteRes<WriteSTIdx,  [HIP09UnitST]>                      { let Latency = 1; }

// Store register, unscaled immed
def : InstRW<[HIP09Write_1cyc_1ST_1ALUM],                       (instregex "^STUR(BB|HH|W|X)i$")>;

// Store register, immed post-index
// Store register, immed pre-index
def : InstRW<[WriteAdr, HIP09Write_1cyc_1ST_1ALUS_1ALUM],       (instregex "^STR(BB|HH|W|X)(post|pre)$")>;

// Store register, immed unprivileged
def : InstRW<[HIP09Write_1cyc_1ST_1ALUM],                       (instregex "^STTR(B|H|W|X)i$")>;

// Store register, unsigned immed
def : InstRW<[HIP09Write_1cyc_1ST_1ALUM],                       (instregex "^STR(BB|HH|W|X)ui$")>;

// Store register, register offset
def : InstRW<[HIP09Write_2cyc_1ST_1ALUM],                       (instregex "^STR(BB|W|X)ro(W|X)$")>;

// Store register offset, no-extend, scaled by 2
// def : InstRW<[HIP09Write_3cyc_1ST_1ALUS_1ALUM],                 (instregex "^STRHHro(W|X)$")>;

// Store pair, immed offset
def : InstRW<[HIP09Write_1cyc_1ST_1ALUM],                       (instregex "^STN?P(W|X)i$")>;

// Store pair, immed post-index
// Store pair, immed pre-index
def : InstRW<[WriteAdr, HIP09Write_1cyc_1ST_1ALUS_1ALUM],       (instregex "^STP(W|X)(post|pre)$")>;

// FP data processing instructions
// -----------------------------------------------------------------------------

def : SchedAlias<WriteF,                                        HIP09Write_1cyc_1FSU>;
def : SchedAlias<WriteFCmp,                                     HIP09Write_4cyc_1FSU>;
def : SchedAlias<WriteFDiv,                                     HIP09Write_10cyc_1FSU_6RC>;

// FP absolute value
// FP negate
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(FABS|FNEG)[HSD]r$")>;

// FP absolute value
// def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^FABD$")>;

// FP compare
def : InstRW<[HIP09Write_4cyc_1FSU_1ALUS23],                    (instregex "^FCMPE?[HSD]r[ri]$")>;

// FP conditional compare
def : InstRW<[HIP09Write_7cyc_1ALUS01_1FSU_1ALUS23],            (instregex "^FCCMPE?[HSD]rr$")>;

// FP conditional select
def : InstRW<[HIP09Write_4cyc_1ALUS01_1FSU],                    (instregex "^FCSEL[HSD]rrr$")>;

// FP divide, H-form
def : InstRW<[HIP09Write_7cyc_1FSU_3RC],                        (instrs FDIVHrr)>;
// FP divide, S-form
def : InstRW<[HIP09Write_7cyc_1FSU_3RC],                        (instrs FDIVSrr)>;
// FP divide, D-form
def : InstRW<[HIP09Write_10cyc_1FSU_6RC],                       (instrs FDIVDrr)>;

// FP square root, H-form
def : InstRW<[HIP09Write_7cyc_1FSU_3RC],                        (instrs FSQRTHr)>;
// FP square root, S-form
def : InstRW<[HIP09Write_9cyc_1FSU_5RC],                        (instrs FSQRTSr)>;
// FP square root, D-form
def : InstRW<[HIP09Write_15cyc_1FSU_11RC],                      (instrs FSQRTDr)>;

// FP fused multiply-add
def : InstRW<[HIP09Write_4cyc_1FSU],                            (instregex "^FN?M(ADD|SUB)[HSD]rrr$")>;

// FP max/min
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^FM(AX|IN)(NM)?[HSD]rr$")>;

// FP add
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^F(ADD|SUB)[HSD]rr")>;

//FP multiply
def : WriteRes<WriteFMul, [HIP09UnitFSU]>                       { let Latency = 3; }
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FN?MUL[HSD]rr")>;

// FP round to FP integral
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FRINT[AIMNPXZ][HSD]r$",
                                                                                "^FRINT(32|64)[XZ][SD]r$")>;

// FP convert to FP
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FCVT(HD|SD|DH|SH|DS|HS)r")>;

// FP miscellaneous instructions
// -----------------------------------------------------------------------------

def : SchedAlias<WriteFImm,                                     HIP09Write_1cyc_1FSU>;


// FP convert, from vec to vec reg
def : SchedAlias<WriteFCvt,                                     HIP09Write_5cyc_1ALUS01_1FSU>;

// Integer/ Fixed point convert to FP
def : InstRW<[HIP09Write_5cyc_1ALUS01_1FSU],                    (instregex "^[SU]CVTF[SU][WX][SHD]ri")>;

// FP convert, from vec to gen reg
def : InstRW<[HIP09Write_5cyc_1FSU_1ALUS23],                    (instregex "^FCVT(A|M|N|P)(S|U)U(W|X)(S|D|H)r$")>;
def : InstRW<[HIP09Write_5cyc_1FSU_1ALUS23],                    (instregex "^FCVTZ[SU][SU][WX](S|D|H)ri?$")>;

// FP convert, Javascript from to gen reg
def : InstRW<[HIP09Write_5cyc_1FSU_1ALUS23],                    (instrs FJCVTZS)>;

// FP move, immed
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^FMOV[HSD]i$")>;

// FP move, register
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^FMOV[HSD]r$")>;

// FP transfer, from gen to low half of vec reg
def : InstRW<[HIP09Write_3cyc_1ALUS01],                         (instrs FMOVWHr, FMOVXHr, FMOVWSr, FMOVXDr,
                                                                            FMOVHWr, FMOVHXr, FMOVSWr, FMOVDXr)>;

// FP transfer, from gen to high half of vec reg
def : InstRW<[HIP09Write_4cyc_1ALUS01_1FSU],                    (instrs FMOVXDHighr)>;

//FP transfer, from vec to gen reg
def : SchedAlias<WriteFCopy,                                    HIP09Write_1cyc_2FSU>;

// FP load instructions
// -----------------------------------------------------------------------------

// Load vector reg, literal
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LDR[SDQ]l$")>;

// Load vector reg, unscaled immed
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LDUR[BHSDQ]i")>;

// Load vector reg, immed post-index
// Load vector reg, immed pre-index
def : InstRW<[WriteAdr, HIP09Write_6cyc_1LD],                   (instregex "^LDR[BHSDQ](post|pre)")>;

// Load vector reg, unsigned immed
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LDR[BHSDQ]ui")>;

// Load vector reg, register offset
def : InstRW<[HIP09Write_6cyc_1LD, ReadAdrBase],                (instregex "^LDR[BHSDQ]ro(W|X)$")>;

// Load vector pair, immed offset
def : InstRW<[HIP09Write_6cyc_1LD, WriteLDHi],                  (instregex "^LDN?P[SDQ]i$")>;

// Load vector pair, immed post-index
// Load vector pair, immed pre-index
def : InstRW<[WriteAdr, HIP09Write_6cyc_1LD, WriteLDHi],        (instregex "^LDP[SDQ](post|pre)$")>;

// FP store instructions
// -----------------------------------------------------------------------------

//Store vector reg, unscaled immed
def : InstRW<[HIP09Write_1cyc_1ST_1STD],                        (instregex "^STUR[BHSDQ]i$")>;

// Store vector reg, immed post-index
// Store vector reg, immed pre-index
def : InstRW<[HIP09Write_1cyc_1ST_1STD_1ALUS, ReadAdrBase],     (instregex "^STR[BHSDQ](post|pre)$")>;

// Store vector reg, immed unprivileged
// Store vector reg, unsigned immed
def : InstRW<[HIP09Write_1cyc_1ST_1STD],                        (instregex "^STR[BHSDQ]ui$")>;

// Store vector reg, reg offset, no-extend
// Store vector reg, reg offset, extend
def : InstRW<[HIP09Write_2cyc_1ST_1STD_1ALUS, ReadAdrBase],     (instregex "^STR[BHSDQ]ro[WX]$")>;

// Store vector pair, immed offset
def : InstRW<[HIP09Write_1cyc_1ST_1STD],                        (instregex "^STN?P[SD]i$")>;

// Store vector pair, immed offset
def : InstRW<[HIP09Write_2cyc_2ST_2STD],                        (instregex "^STN?PQi$")>;

// Store vector pair, immed post-index
// Store vector pair, immed pre-index
def : InstRW<[WriteAdr, HIP09Write_1cyc_1ST_1STD_1ALUS],        (instregex "^STP[SD](post|pre)$")>;
def : InstRW<[WriteAdr, HIP09Write_2cyc_2ST_2STD_2ALUS],        (instregex "^STPQ(post|pre)$")>;

// ASIMD integer Instructions
// -----------------------------------------------------------------------------

// ASIMD absolute diff
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^[SU]ABDv")>;

// ASIMD absolute diff accum
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^[SU]ABAL?v")>;

// ASIMD arith, basicc
// ASIMD arith wide
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(ABS|NEG|ADD|SUB)v")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^[SU]ADD(L|W)v")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^[SU]SUB[LW]v")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(SH|UH)(ADD|SUB)v")>;

// Integer SIMD complex arithmetic
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(SU|US)QADDv")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^SQ(ABS|NEG)v")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(SQ|UQ)(ADD|SUB)v")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(ADD|SUB)HNv")>;

// Integer SIMD complex arithmetic
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^R(ADD|SUB)HNv")>;
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^[SU]RHADDv")>;

// ASIMD arith, pair-wise
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^ADDPv")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^[SU]ADDLPv")>;

// ASIMD arith, reduce
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^(ADDV|[SU]ADDLV)v")>;

// ASIMD compare
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^CM(GT|EQ|GE|LT|LE|TST|HI|HS)v")>;

// ASIMD dot product
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^[SU]DOT(lane)?(v8|v16)i8$")>;

// ASIMD dot product using signed and unsigned integers
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^(SU|US)DOT(lane)?(v8|v16)i8$")>;

// ASIMD logical
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(AND|NOT|ORN|ORR|BIC|EOR)v")>;

// ASIMD matrix multiply-accumulate
def : InstRW<[HIP09Write_4cyc_4FSU],                            (instregex "^(S|U|US)MMLA$")>;

// ASIMD max,min
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^[SU](MAX|MIN)v")>;

// ASIMD max/min pair-wise
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^[SU](MAX|MIN)Pv")>;

// ASIMD max/min, reduce, S form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^[SU](MAX|MIN)V(v4|v2)i32v$")>;

// ASIMD max/min, reduce,  B/H form
def : InstRW<[HIP09Write_4cyc_2FSU],                            (instregex "^[SU](MAX|MIN)V(v4i16|v8i8|v8i16|v16i8)v$")>;

// Integer SIMD multiply(accumulate), B form
def : InstRW<[HIP09Write_2cyc_2FSU],                            (instregex "^M(UL|LA|LS)(v8|v16)i8$")>;

// Integer SIMD multiply(accumulate), H/S form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^M(UL|LA|LS)(v4i16|v8i16|v4i32|v2i32)(_indexed)?$",
                                                                              "^SQR?DMULH(v4|v8|v1)i16(_indexed)?$",
                                                                              "^SQR?DMULH(v4|v2|v1)i32(_indexed)?$")>;
// ASIMD multiply accumulate high, H/S form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^SQRDML[AS]H(v4|v8|v1)?i16(_indexed)?$",
                                                                              "^SQRDML[AS]H(v4|v2|v1)?i32(_indexed)?$")>;

// ASIMD multiply(accumulate) long B form
def : InstRW<[HIP09Write_2cyc_2FSU],                            (instregex "^[SU]M(LA|LS|UL)L(v8|v16)i8_v8i16$")>;

// Integer SIMD multiply(accumulate) long H/S form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^(S|U|SQD)M(LA|LS|UL)L(v4|v8)i16",
                                                                                "^(S|U|SQD)M(LA|LS|UL)L(v2|v4)i32",
                                                                                "^SQDM(LA|LS|UL)L(i16|i32)$",
                                                                                "^SQDM(LA|LS|UL)Lv1(i32|i64)_indexed$")>;

// ASIMD multiply/multiply long (8x8) polynomial
def : InstRW<[HIP09Write_2cyc_1FSU2],                           (instregex "^PMULL?(v8i8|v16i8)$")>;

// ASIMD pairwise add and accumulate long
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^[SU]ADALPv")>;

// ASIMD shift accumulate
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^[SU]R?SRA(d|v)")>;

// ASIMD shift by immed, basic
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^SHL(v|d)", "^SH(LL|RN)v",
                                                                                "^[SU]SHLLv", "^[SU]SHR(d|v)")>;

// ASIMD shift by immed and insert, basic
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "SLI(d|v)", "^SRI(d|v)")>;

// ASIMD shift by immed, complex
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^RSHRNv", "^[SU]QRSHRU?N(b|h|s|v)",
                                                                                "^[SU]RSHR(d|v)")>;
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^SQSHLU(b|h|s|d|v)", "^[SU]QSHRU?N(b|h|s|v)")>;
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^[SU]QSHL(b|h|s|d|v)")>;

// ASIMD shift by register, basic
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^[SU]SHLv")>;

// ASIMD shift by immed, complex
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^[SU]QRSHLv", "^[SU]RSHL(d|v)")>;

// ASIMD floating-point instructions
// -----------------------------------------------------------------------------

// Reference for forms in this group
//   D form - v2f32
//   Q form - v4f32, v2f64
//   D form - 32, 64
//   D form - v1i32, v1i64
//   D form - v2i32
//   Q form - v4i32, v2i64

// FP SIMD sign manipulation
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^F(ABS|NEG)v")>;

// ASIMD FP absolute difference
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^FABDv")>;

// ASIMD FP arith
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^F(ADD|SUB)v")>;

// ASIMD FP add pairwise
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FADDPv")>;

// ASIMD FP compare
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^FACG[ET]v")>;
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "FCM(EQ|GE|GT|LE|LT)v")>;

// ASIMD FP convert long
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^FCVTLv")>;

// ASIMD FP convert narrow
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^FCVTX?Nv")>;

// ASIMD FP convert to Integer/Fixed point, D-form
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FCVT[NMAPZ][SU](v4f16|v2f32|v1f16|v1i64|v1i32)")>;
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FCVTZ[SU](h|s|v4i16_shift|v2i32_shift)")>;

// ASIMD FP convert to Integer/Fixed point, Q-form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^FCVT[NMAPZ][SU](v8f16|v4f32|v2f64)")>;
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FCVTZ[SU](d|v4i32_shift|v2i64_shift)")>;

// ASIMD FP convert from Integer/Fixed-point to FP, Q-form
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^[SU]CVTF(h|s|v4f16|v2f32|v1i64|v1i32|v1i16|v4i16_shift|v2i32_shift)$")>;

// ASIMD FP convert from Integer/Fixed-point to FP, Q-form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^[SU]CVTF(d|v8f16|v4f32|v2f64|v8i16_shift|v4i32_shift|v2i64_shift)$")>;

// ASIMD FP divide, D-form, F16
def : InstRW<[HIP09Write_9cyc_1FSU_5RC],                        (instregex "^FDIVv4f16$")>;

// ASIMD FP divide, D-form, F32
def : InstRW<[HIP09Write_9cyc_1FSU_5RC],                        (instregex "^FDIVv2f32$")>;

// ASIMD FP divide, Q-form, F16
def : InstRW<[HIP09Write_13cyc_1FSU_9RC],                       (instregex "^FDIVv8f16$")>;

// ASIMD FP divide, Q-form, F32
def : InstRW<[HIP09Write_13cyc_1FSU_9RC],                       (instregex "^FDIVv4f32$")>;

// ASIMD FP divide, Q-form, F64
def : InstRW<[HIP09Write_15cyc_1FSU_11RC],                      (instregex "^FDIVv2f64$")>;

// ASIMD FP square root, D-form, F16
def : InstRW<[HIP09Write_13cyc_1FSU_9RC],                       (instregex "^FSQRTv4f16$")>;

// ASIMD FP square root, D-form, F32
def : InstRW<[HIP09Write_13cyc_1FSU_9RC],                       (instregex "^FSQRTv2f32$")>;

// ASIMD FP square root, Q-form, F16
def : InstRW<[HIP09Write_21cyc_1FSU_17RC],                      (instregex "^FSQRTv8f16$")>;

// ASIMD FP square root, Q-form, F32
def : InstRW<[HIP09Write_21cyc_1FSU_17RC],                      (instregex "^FSQRTv4f32$")>;

// ASIMD FP square root, Q-form, F64
def : InstRW<[HIP09Write_25cyc_1FSU_21RC],                      (instregex "^FSQRTv2f64$")>;

// ASIMD FP max/min, pairwise
def : InstRW<[HIP09Write_2cyc_1FSU],                            (instregex "^F(MAX|MIN)(NM)?v")>;

def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^F(MAX|MIN)(NM)?Pv")>;

// FP SIMD max,min reduce HP-form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^F(MAX|MIN)(NM)?V(v4|v8)i16v")>;

// FP SIMD max,min reduce SP/DP-form
def : InstRW<[HIP09Write_2cyc_2FSU],                            (instregex "^F(MAX|MIN)(NM)?Vv4i32v")>;

// ASIMD FP multiply
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FMULX?v")>;

// ASIMD FP fused multiply-add
def : InstRW<[HIP09Write_4cyc_1FSU],                            (instregex "^FML[AS]v")>;

// ASIMD FP fused multiply-add long
def : InstRW<[HIP09Write_5cyc_1FSU],                            (instregex "^FML[AS]L2?v")>;

// ASIMD FP round to FP integral, D-form
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FRINT(N|M|P|Z|A|X|I)(v4f16|v2f32)")>;
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FRINT(32|64)[ZX]v2f32")>;

// ASIMD FP round to FP integral, Q-form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^FRINT(N|M|P|Z|A|X|I)(v8f16|v4f32|v2f64)")>;
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^FRINT(32|64)[ZX](v4f32|v2f64)")>;

// ASIMD Bfloat16 (BF16) Instructions
// -----------------------------------------------------------------------------

// ASIMD convert, F32 to BF16
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^BFCVTN2?$")>;

// ASIMD dot product
def : InstRW<[HIP09Write_6cyc_2FSU],                            (instregex "^(BFDOT|BF16DOTlane)v")>;

// ASIMD matrix multiply accumulate
def : InstRW<[HIP09Write_9cyc_4FSU],                            (instrs BFMMLA)>;

// ASIMD multiply accumulate long
def : InstRW<[HIP09Write_5cyc_1FSU],                            (instregex "^BFMLAL[BT](Idx)?$")>;

// Scalar convert, F32 to BF16
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instrs BFCVT)>;

// ASIMD Miscellaneous Instructions
// -----------------------------------------------------------------------------

// Reference for forms in this group
//   D form - v8i8, v4i16, v2i32
//   Q form - v16i8, v8i16, v4i32
//   D form - v1i8, v1i16, v1i32, v1i64
//   Q form - v16i8, v8i16, v4i32, v2i64

// ASIMD bit reverse
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^RBITv")>;

// ASIMD bitwise insert
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(BIF|BIT|BSL)v")>;

// ASIMD count
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(CLS|CLZ)v")>;

// TODO: CNT only supports B element sizes now.
// ASIMD count, D
// ASIMD count, B/H/S
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^CNT(v8i8|v16i8)")>;

// ASIMD duplicate, gen reg
// Integer SIMD complex move general register to FP
def : InstRW<[HIP09Write_4cyc_1ALUS01_1FSU],                    (instregex "^DUPv.+gpr")>;

// ASIMD duplicate, element
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^DUP(i8|i16|i32|i64)$")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^DUPv.+lane")>;

// ASIMD extract
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^EXTv", "^XTNv")>;

// ASIMD extract narrow, saturating
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^[SU]QXTU?Nv")>;

// ASIMD insert, element to element
def : InstRW<[HIP09Write_4cyc_1ALUS01_1FSU],                    (instregex "^INSv")>;

// ASIMD FP move, immed
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^FMOVv")>;

// ASIMD move, integer immediate
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^MOVIv", "^MOVID$")>;
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^MVNIv")>;

// ASIMD reciprocal and square root estimate, D-form
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^URECPEv2i32", "^URSQRTEv2i32")>;

// ASIMD reciprocal and square root estimate, Q-form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^URECPEv4i32", "^URSQRTEv4i32")>;

// ASIMD FP reciprocal and square root estimate, D-form
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^(FRECPE|FRSQRTE)(v2f32|v4f16|v1)")>;

// ASIMD FP reciprocal and square root estimate, Q-form
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^(FRECPE|FRSQRTE)(v8f16|v4f32|v2f64)")>;

// ASIMD FP reciprocal exponent
def : InstRW<[HIP09Write_3cyc_1FSU],                            (instregex "^FRECPXv")>;

// ASIMD FP reciprocal step
def : InstRW<[HIP09Write_4cyc_1FSU],                            (instregex "^FR(ECP|SQRT)S(v|32|64)")>;

// ASIMD reverse
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^REV(16|32|64)v")>;

// ASIMD table lookup, 1 or 2 table RegS
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^TBL(v8|v16)i8(One|Two)$")>;

// ASIMD table lookup, 3 table RegS
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^TBL(v8|v16)i8Three$")>;

// ASIMD table lookup, 4 table RegS
def : InstRW<[HIP09Write_5cyc_1FSU_3RC],                        (instregex "^TBL(v8|v16)i8Four$")>;

// ASIMD table lookup extension, 1 table reg
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^TBX(v8|v16)i8One$")>;

// ASIMD table lookup extension, 2 table reg
def : InstRW<[HIP09Write_3cyc_2FSU],                            (instregex "^TBX(v8|v16)i8Two$")>;

// ASIMD table lookup extension, 3 table reg
def : InstRW<[HIP09Write_5cyc_1FSU_3RC],                        (instregex "^TBX(v8|v16)i8Three$")>;

// ASIMD table lookup extension, 4 table reg
def : InstRW<[HIP09Write_7cyc_4FSU],                            (instregex "^TBX(v8|v16)i8Four$")>;

// ASIMD move FP to general register
def : InstRW<[HIP09Write_2cyc_2FSU],                            (instregex "^[SU]MOV")>;

// ASIMD transpose
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^TRN[12]v")>;

// ASIMD uzip/zip
def : InstRW<[HIP09Write_1cyc_1FSU],                            (instregex "^(UZP|ZIP)[12]v")>;

// ASIMD load instructions
// -----------------------------------------------------------------------------

// SIMD load, 1-element, multiple, 1-reg
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LD1Onev(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_1LD],                   (instregex "^LD1Onev(8b|4h|2s|1d)_POST$")>;
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LD1Onev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_1LD],                   (instregex "^LD1Onev(16b|8h|4s|2d)_POST$")>;

// SIMD load, 1-element, multiple, 2-reg
def : InstRW<[HIP09Write_6cyc_2LD],                             (instregex "^LD1Twov(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_2LD],                   (instregex "^LD1Twov(8b|4h|2s|1d)_POST$")>;
def : InstRW<[HIP09Write_6cyc_2LD],                             (instregex "^LD1Twov(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_2LD],                   (instregex "^LD1Twov(16b|8h|4s|2d)_POST$")>;

// SIMD load, 1-element, multiple, 3-reg
def : InstRW<[HIP09Write_6cyc_1LD_3RC],                         (instregex "^LD1Threev(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_1LD_3RC],               (instregex "^LD1Threev(8b|4h|2s|1d)_POST$")>;
def : InstRW<[HIP09Write_6cyc_1LD_3RC],                         (instregex "^LD1Threev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_1LD_3RC],               (instregex "^LD1Threev(16b|8h|4s|2d)_POST$")>;

// SIMD load, 1-element, multiple, 4-reg
def : InstRW<[HIP09Write_6cyc_1LD_4RC],                         (instregex "^LD1Fourv(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_1LD_4RC],               (instregex "^LD1Fourv(8b|4h|2s|1d)_POST$")>;
def : InstRW<[HIP09Write_6cyc_1LD_4RC],                         (instregex "^LD1Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_1LD_4RC],               (instregex "^LD1Fourv(16b|8h|4s|2d)_POST$")>;


// SIMD load, 1-element, single, 1 lane
def : InstRW<[HIP09Write_7cyc_1LD_1FSU],                        (instregex "^LD1i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP09Write_7cyc_1LD_1FSU],              (instregex "^LD1i(8|16|32|64)_POST$")>;

// SIMD load, 1-element, single, replicate to all lanes
def : InstRW<[HIP09Write_7cyc_1LD_1FSU],                        (instregex "^LD1Rv(8b|4h|2s|1d)$$")>;
def : InstRW<[WriteAdr, HIP09Write_7cyc_1LD_1FSU],              (instregex "^LD1Rv(8b|4h|2s|1d)_POST$")>;
def : InstRW<[HIP09Write_7cyc_1LD_1FSU],                        (instregex "^LD1Rv(16b|8h|4s|2d)$$")>;
def : InstRW<[WriteAdr, HIP09Write_7cyc_1LD_1FSU],              (instregex "^LD1Rv(16b|8h|4s|2d)_POST$")>;

// SIMD load, 2-elements, multiple, Q-form
def : InstRW<[HIP09Write_8cyc_2LD_2FSU],                        (instregex "^LD2Twov(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_2LD_2FSU],              (instregex "^LD2Twov(16b|8h|4s|2d)_POST$")>;

// SIMD load, 2-elements, multiple, other form
def : InstRW<[HIP09Write_8cyc_1LD_1FSU],                        (instregex "^LD2Twov(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_1LD_1FSU],              (instregex "^LD2Twov(8b|4h|2s)_POST$")>;

// SIMD load, 2-element, single, 1 lane
def : InstRW<[HIP09Write_8cyc_1LD_1FSU],                        (instregex "^LD2i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_1LD_1FSU],              (instregex "^LD2i(8|16|32|64)_POST$")>;

// SIMD load LD3 (multiple structures)
def : InstRW<[HIP09Write_9cyc_3LD_3FSU],                        (instregex "^LD3Threev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_9cyc_3LD_3FSU],              (instregex "^LD3Threev(16b|8h|4s|2d)_POST$")>;
def : InstRW<[HIP09Write_9cyc_3LD_3FSU],                        (instregex "^LD3Threev(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP09Write_9cyc_3LD_3FSU],              (instregex "^LD3Threev(8b|4h|2s)_POST$")>;

// SIMD load, 3-element, single, 1 lane
def : InstRW<[HIP09Write_8cyc_2LD_2FSU],                        (instregex "^LD3i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_2LD_2FSU],              (instregex "^LD3i(8|16|32|64)_POST$")>;

// SIMD load, 4-element, multiple, Q-form
def : InstRW<[HIP09Write_13cyc_8LD_8FSU],                       (instregex "^LD4Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_13cyc_8LD_8FSU],             (instregex "^LD4Fourv(16b|8h|4s|2d)_POST$")>;

// SIMD load, 4-element, multiple, D-form
def : InstRW<[HIP09Write_12cyc_8LD_8FSU],                       (instregex "^LD4Fourv(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP09Write_12cyc_8LD_8FSU],             (instregex "^LD4Fourv(8b|4h|2s)_POST$")>;

// SIMD load LD4 (single structure)
def : InstRW<[HIP09Write_8cyc_2LD_2FSU],                        (instregex "^LD4i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_2LD_2FSU],              (instregex "^LD4i(8|16|32|64)_POST$")>;

// SIMD load, 2-element, single, replicate to all lanes
def : InstRW<[HIP09Write_8cyc_1LD_1FSU],                        (instregex "^LD2Rv(8b|4h|2s|1d)$$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_1LD_1FSU],              (instregex "^LD2Rv(8b|4h|2s|1d)_POST$")>;
def : InstRW<[HIP09Write_8cyc_1LD_1FSU],                        (instregex "^LD2Rv(16b|8h|4s|2d)$$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_1LD_1FSU],              (instregex "^LD2Rv(16b|8h|4s|2d)_POST$")>;

// SIMD load, 3-element, single, replicate to all lanes
def : InstRW<[HIP09Write_8cyc_2LD_2FSU],                        (instregex "^LD3Rv(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_2LD_2FSU],              (instregex "^LD3Rv(8b|4h|2s|1d)_POST$")>;
def : InstRW<[HIP09Write_8cyc_2LD_2FSU],                        (instregex "^LD3Rv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_2LD_2FSU],              (instregex "^LD3Rv(16b|8h|4s|2d)_POST$")>;

// SIMD load, 4-element, single, replicate to all lanes
def : InstRW<[HIP09Write_8cyc_2LD_2FSU],                        (instregex "^LD4Rv(8b|4h|2s|1d)$$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_2LD_2FSU],              (instregex "^LD4Rv(8b|4h|2s|1d)_POST$")>;
def : InstRW<[HIP09Write_8cyc_2LD_2FSU],                        (instregex "^LD4Rv(16b|8h|4s|2d)$$")>;
def : InstRW<[WriteAdr, HIP09Write_8cyc_2LD_2FSU],              (instregex "^LD4Rv(16b|8h|4s|2d)_POST$")>;

// ASIMD Store Instructions
// -----------------------------------------------------------------------------

// SIMD store, 1-element, multiple, 1 reg, Q-form
def : InstRW<[HIP09Write_2cyc_1ST_1STD],                        (instregex "ST1Onev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_2cyc_1ST_1STD],              (instregex "ST1Onev(16b|8h|4s|2d)_POST$")>;

// SIMD store, 1-element, multiple, 2 reg, Q-form
def : InstRW<[HIP09Write_3cyc_2ST_2STD],                        (instregex "ST1Twov(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_3cyc_2ST_2STD],              (instregex "ST1Twov(16b|8h|4s|2d)_POST$")>;

// SIMD store, 1-element, multiple, 3 reg, Q-form
def : InstRW<[HIP09Write_4cyc_3ST_3STD],                        (instregex "ST1Threev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_4cyc_3ST_3STD],              (instregex "ST1Threev(16b|8h|4s|2d)_POST$")>;

// SIMD store, 1-element, multiple, 4 reg, Q-form
def : InstRW<[HIP09Write_5cyc_4ST_4STD],                        (instregex "ST1Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_5cyc_4ST_4STD],              (instregex "ST1Fourv(16b|8h|4s|2d)_POST$")>;

// SIMD store ST1 (multiple structure) Q=0, n=1/2
def : InstRW<[HIP09Write_2cyc_1ST_1STD],                        (instregex "ST1(One|Two)v(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP09Write_2cyc_1ST_1STD],              (instregex "ST1(One|Two)v(8b|4h|2s|1d)_POST$")>;

// SIMD store ST1 (multiple structure) Q=0, n=3/4
def : InstRW<[HIP09Write_3cyc_2ST_2STD],                        (instregex "ST1(Three|Four)v(8b|4h|2s|1d)$")>;
def : InstRW<[WriteAdr, HIP09Write_3cyc_2ST_2STD],              (instregex "ST1(Three|Four)v(8b|4h|2s|1d)_POST$")>;

// SIMD store, 1-element, single, 1 lane
// SIMD store, 2-element, single, 1 lane
def : InstRW<[HIP09Write_1cyc_1ST_1STD],                        (instregex "ST[12]i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP09Write_1cyc_1ST_1STD],              (instregex "ST[12]i(8|16|32|64)_POST$")>;

// SIMD store, 2-element, multiple, Q-form
def : InstRW<[HIP09Write_1cyc_2ST_2STD],                        (instregex "ST2Twov(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_1cyc_2ST_2STD],              (instregex "ST2Twov(16b|8h|4s|2d)_POST$")>;

// SIMD store, 2-element, multiple, D-form
def : InstRW<[HIP09Write_1cyc_1ST_1STD],                        (instregex "ST2Twov(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP09Write_1cyc_1ST_1STD],              (instregex "ST2Twov(8b|4h|2s)_POST$")>;

// SIMD store, 3-element, multiple, Q-form
def : InstRW<[HIP09Write_7cyc_3FSU_3ST_3STD],                   (instregex "ST3Threev(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_7cyc_3FSU_3ST_3STD],         (instregex "ST3Threev(16b|8h|4s|2d)_POST$")>;

// SIMD store, 3-element, multiple, D-form
def : InstRW<[HIP09Write_6cyc_2FSU_2ST_2STD],                   (instregex "ST3Threev(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP09Write_6cyc_2FSU_2ST_2STD],         (instregex "ST3Threev(8b|4h|2s)_POST$")>;

// SIMD store, 4-element, multiple, Q-form
def : InstRW<[HIP09Write_10cyc_8FSU_8ST_8STD],                  (instregex "ST4Fourv(16b|8h|4s|2d)$")>;
def : InstRW<[WriteAdr, HIP09Write_10cyc_8FSU_8ST_8STD],        (instregex "ST4Fourv(16b|8h|4s|2d)_POST$")>;

// SIMD store, 4-element, multiple, D-form
def : InstRW<[HIP09Write_10cyc_6FSU_6ST_6STD],                  (instregex "ST4Fourv(8b|4h|2s)$")>;
def : InstRW<[WriteAdr, HIP09Write_10cyc_6FSU_6ST_6STD],        (instregex "ST4Fourv(8b|4h|2s)_POST$")>;

// SIMD store, 3-element, single, 1 lane
// SIMD store, 4-element, single, 1 lane
def : InstRW<[HIP09Write_4cyc_1FSU_1ST_1STD],                   (instregex "ST[34]i(8|16|32|64)$")>;
def : InstRW<[WriteAdr, HIP09Write_4cyc_1FSU_1ST_1STD],         (instregex "ST[34]i(8|16|32|64)_POST$")>;

// Cryptography Extensions v8.0
// -----------------------------------------------------------------------------

// Crypto AES ops
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^AES[DE]rr$", "^AESI?MCrr$")>;

// Crypto polynomial (64x64) multiply long
def : InstRW<[HIP09Write_2cyc_1FSU2],                           (instregex "^PMULL(v1|v2)i64$")>;

// Crypto SHA1 hash acceleration ops
// Crypto SHA1 schedule acceleration ops
def : InstRW<[HIP09Write_2cyc_1FSU2],                           (instregex "^SHA1(H|SU0|SU1)")>;

// Crypto SHA1 hash acceleration ops
def : InstRW<[HIP09Write_4cyc_1FSU2],                           (instregex "^SHA1[CMP]")>;

// Crypto SHA256 schedule acceleration ops
def : InstRW<[HIP09Write_2cyc_1FSU2],                           (instregex "^SHA256SU[01]")>;

// Crypto SHA256 hash acceleration ops
def : InstRW<[HIP09Write_4cyc_1FSU2],                           (instregex "^SHA256H2?rrr")>;

// Cryptography Extensions v8.2
// -----------------------------------------------------------------------------
// v8.2 SHA512 hash acceleration ops
def : InstRW<[HIP09Write_2cyc_1FSU2],                           (instregex "^SHA512(H|H2|SU0|SU1)")>;

// v8.2 SHA3 ops
def : InstRW<[HIP09Write_1cyc_1FSU2],                           (instrs BCAX, EOR3, RAX1, XAR)>;

// v8.2 SM/SM3 ops
def : InstRW<[HIP09Write_2cyc_1FSU2],                           (instregex "^SM3SS1$", "^SM3TT[12][AB]$" ,
                                                                                "^SM3PARTW[12]$")>;

// v8.2 SM/SM4 ops
def : InstRW<[HIP09Write_4cyc_1FSU2],                           (instregex "^SM4E(NCKEY)?$")>;

// CRC
// -----------------------------------------------------------------------------

// CRC checksum ops
def : InstRW<[HIP09Write_2cyc_1ALUM],                           (instregex "^CRC32C?[BHWX]rr$")>;

// 3.22 SVE Predicate instructions
// -----------------------------------------------------------------------------

// Loop control, based on predicate
// Loop control, based on predicate and flag setting
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^BRK[AB]S?_PPzP", "^BRK[AB]_PPmP")>;

// Loop control, propagating
// Loop control, propagating and flag setting
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^BRKNS?_PPzP$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^BRKP[AB]S?_PPzPP$")>;

// Loop control, based on GPR
def : InstRW<[HIP09Write_2cyc_1ALUS01],                         (instregex "^WHILEL(E|O|S|T)_P(WW|XX)_[BHSD]$")>;

// Loop terminate
def : InstRW<[HIP09Write_1cyc_1ALUS23],                         (instregex "^CTERM(EQ|NE)_(WW|XX)$")>;

// Predicate counting scalar
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^ADD(PL|VL)_XXI$")>;
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instregex "^(CNT|DEC|INC)[BHWD]_XPiI$")>;
def : InstRW<[HIP09Write_1cyc_1ALUS],                           (instrs RDVLI_XI)>;

// Predicate counting scalar
def : InstRW<[HIP09Write_2cyc_1ALUS23],                         (instregex "^SQ(DEC|INC)[BHWD]_(XPiWdI|XPiI)$")>;
def : InstRW<[HIP09Write_2cyc_1ALUS23],                         (instregex "^UQ(DEC|INC)[BHWD]_(WPiI|XPiI)$")>;

// Predicate counting scalar, active predicate
def : InstRW<[HIP09Write_5cyc_1FSU02_1ALUS],                    (instregex "^(INCP|DECP)_XP_[BHSD]$")>;

// Predicate counting scalar, active predicate
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^CNTP_XPP_[BHSD]$")>;

// Predicate counting vector, active predicate
def : InstRW<[HIP09Write_6cyc_1FSU02_1ALUS],                    (instregex "^SQ(INCP|DECP)_XPWd_[BHSD]$",
                                                                                "^(SQ|UQ)(INCP|DECP)_[XW]P_[BHSD]$")>;

// Predicate counting vector, active predicate
def : InstRW<[HIP09Write_4cyc_2FSU02],                          (instregex "^(SQ|UQ)?(INCP|DECP)_ZP_[HSD]$")>;

// Predicate logical
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(ORR|EOR|AND|BIC|NOT)_ZPmZ_[BHSD]$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(AND|ORR|EOR|BIC|NAND|NOR|ORN)_PPzPP$")>;

// Predicate logical, flag setting
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(AND|BIC|EOR|ORR|ORN|NOR|NAND)S_PPzPP$")>;

// Predicate reverse
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^REV_PP_[BHSD]$")>;

// Predicate select
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^SEL_ZPZZ_[BHSD]$")>;

// Predicate set
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instrs PFALSE)>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^PTRUE_[BHSD]$")>;

// Predicate set/initialize, set flags
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^PTRUES_[BHSD]$")>;

// Predicate find first/next
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^PFIRST_B$",
                                                                                "^PNEXT_[BHSD]$")>;

// Predicate test
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^PTEST_PP")>;

// Predicate transpose
// Predicate zip/unzip
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(ZIP|UZP|TRN)[12]_PPP_[BHSD]$")>;

// Predicate unpack and widen
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(PUNPKHI|PUNPKLO)_PP$")>;


// 3.23 SVE Integer Instructions
// -----------------------------------------------------------------------------

// Arithmetic, absolute diff SABD, UABD
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^[SU]ABD_ZPmZ_[BHSD]$")>;

// Arithmetic, address generation
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^ADR_[SU]XTW_ZZZ_D_[0123]$", "^ADR_LSL_ZZZ_[SD]_[0123]$")>;

// Arithmetic, basic
// Arithmetic, complex
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(ABS|ADD|SUBR?|NEG|CNOT)_ZPmZ")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(SQ|UQ)?(ADD|SUBR?)_(ZZZ|ZI)_[BHSD]$")>;

// Arithmetic, shift
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^(ASR|LSR|LSL)_ZPmI_[BHSD]$")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^(ASR|LSR|LSL)_ZZI_[BHSD]$")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^(ASR|LSR|LSL)_ZPZ[IZ]")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^(ASR|LSR|LSL)R?_ZPmZ_[BHSD]")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^(ASR|LSR|LSL)_WIDE_(ZPmZ|ZZZ)_[BHS]")>;

// Arithmetic, shift right for divide
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^ASRD_ZPmI")>;

// Count/reverse bits
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(CLS|CLZ|RBIT)_ZPmZ_[BHSD]$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(CLS|CLZ)_ZPmZ_[BHSD]_UNDEF$")>;

// Count/reverse bits, B H S form
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^CNT_ZPmZ_[BHS]$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^CNT_ZPmZ_[BHS]_UNDEF$")>;

// Count/reverse bits, D form
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^CNT_ZPmZ_D$")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^CNT_ZPmZ_D_UNDEF$")>;

// Broadcast logical bitmask immediate to vector
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^DUPM_ZI$")>;

// Compare and set flags
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^CMP(GE|GT|LT|LE|HS|HI|LO|LS|EQ|NE)_PPzZ[ZI]_[BHSD]$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^CMP(GE|GT|LT|LE|HS|HI|LO|LS|EQ|NE)_WIDE_PPzZZ_[BHS]$")>;

// Conditional extract operations, scalar form
def : InstRW<[HIP09Write_5cyc_1FSU02_1ALUS23],                  (instregex "^CLAST[AB]_RPZ_[BHSD]$")>;

// Conditional extract operations, SIMD&FP scalar and vector forms
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^CLAST[AB]_[VZ]PZ_[BHSD]$")>;

// Conditional extract operations, SIMD&FP scalar and vector forms
def : InstRW<[HIP09Write_7cyc_1FSU02_3RC],                      (instregex "^SPLICE_ZPZZ?_[BHSD]$")>;
def : InstRW<[HIP09Write_5cyc_1FSU02],                          (instregex "^COMPACT_ZPZ_[SD]$")>;

// Convert to floating point
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^[SU]CVTF_ZPmZ_(HtoH|StoS|StoD|StoH|DtoS|DtoH|DtoD)$")>;

// SVE copy general register to vector (predicated)
def : InstRW<[HIP09Write_5cyc_1FSU02_1ALUS23],                  (instregex "^CPY_ZPmV_[BHSD]$")>;

// SVE copy integer immediate (predicated)
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^CPY_(ZPmI|ZPzI)_[BHSD]$")>;

// SVE copy element from SIMD&FP scalar register
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^CPY_ZPmR_[BHSD]$")>;

// SVE integer divide vectors (predicated) 32-bit
def : InstRW<[HIP09Write_17cyc_1FSU02_13RC],                    (instregex "^[SU](DIV)R?_ZPmZ_S$")>;

// SVE integer divide vectors (predicated) 64-bit
def : InstRW<[HIP09Write_17cyc_1FSU02_13RC],                    (instregex "^[SU](DIV)R?_ZPmZ_D$")>;

// Dot product, 8-bit
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^[SU]DOT_ZZZI?_S$")>;

// Dot product, 16-bit
def : InstRW<[HIP09Write_3cyc_2FSU02],                          (instregex "^[SU]DOT_ZZZI?_D$")>;

// Dot product, 16-bit, using signed and unsigned integers
def : InstRW<[HIP09Write_3cyc_2FSU02],                          (instregex "^(SU|US)DOT_ZZZI?$")>;

// Duplicate, indexed
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^DUP_ZZI_[BHSDQ]$")>;

// Duplicate, immediate
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^DUP_ZI_[BHSD]$")>;

// Duplicate, scalar
def : InstRW<[HIP09Write_5cyc_1FSU02_1ALUS23],                  (instregex "^DUP_ZR_[BHSD]$")>;

// Extend, sign or zero
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^[SU]XTB_ZPmZ_[HSD]$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^[SU]XTH_ZPmZ_[SD]$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^[SU]XTW_ZPmZ_D$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^[SU]XTB_ZPmZ_[HSD]_UNDEF$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^[SU]XTH_ZPmZ_[SD]_UNDEF$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^[SU]XTW_ZPmZ_D_UNDEF$")>;

// Extract
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^EXT_ZZI(_B)?$")>;

// Insert operation, scalar
def : InstRW<[HIP09Write_5cyc_1FSU02_1ALUS23],                  (instregex "^INSR_ZR_[BHSD]$")>;

// Insert operation, SIMD and FP scalar
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^INSR_ZV_[BHSD]$")>;

// Extract operation, SIMD and FP scalar
// Extract operation, scalar
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^LAST[AB]_[RV]PZ_[BHSD]$")>;

// Horizontal operations, B-form, immediate operands only
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^INDEX_II_B$")>;

// Horizontal operations, H,S,D-form, immediate operands only
def : InstRW<[HIP09Write_3cyc_2FSU02],                          (instregex "^INDEX_II_[HSD]$")>;

// Horizontal operations, B-form, scalar start, immediate increment
def : InstRW<[HIP09Write_8cyc_6FSU02_6ALUS23],                  (instregex "^INDEX_RI_B$")>;

// Horizontal operations, H,S,D-form, scalar start, immediate increment
def : InstRW<[HIP09Write_9cyc_6FSU02_6ALUS23],                  (instregex "^INDEX_RI_[HSD]$")>;

// Horizontal operations, B-form, immediate start, scalar increment
def : InstRW<[HIP09Write_6cyc_4FSU02_4ALUS23],                  (instregex "^INDEX_IR_B$")>;

// Horizontal operations, H,S,D-form, immediate start, scalar increment
def : InstRW<[HIP09Write_7cyc_4FSU02_4ALUS23],                  (instregex "^INDEX_IR_[HSD]$")>;

// Horizontal operations, B-form, scalar
def : InstRW<[HIP09Write_6cyc_6FSU02_6ALUS23],                  (instregex "^INDEX_RR_B$")>;

// Horizontal operations, H,S,D-form, scalar
def : InstRW<[HIP09Write_7cyc_6FSU02_6ALUS23],                  (instregex "^INDEX_RR_[HSD]$")>;

// Logical
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(AND|ORR|EOR|BIC)_(ZZZ|ZI)$")>;

// Max/min, basic and pairwise
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^[SU](MAX|MIN)_(ZPmZ|ZI)_[BHSD]$")>;

// Matrix multiply-accumulate
def : InstRW<[HIP09Write_4cyc_1FSU02_4RC],                      (instregex "^(S|U|US)MMLA_ZZZ$")>;

// Move prefix
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^MOVPRFX")>;

// Multiply, B element size
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^MUL_ZI_B$",
                                                                                "^(MUL|SMULH|UMULH)_ZPmZ_B$")>;

// Multiply, H, S, D element size
def : InstRW<[HIP09Write_3cyc_2FSU02],                          (instregex "^MUL_ZI_[HSD]$",
                                                                                "^(MUL|SMULH|UMULH)_ZPmZ_[HSD]$")>;

// Multiply accumulate, B element size
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^(MLA|MLS|MAD|MSB)_ZPmZZ_B$")>;

// Multiply accumulate, H, S, D element size
def : InstRW<[HIP09Write_3cyc_2FSU02],                          (instregex "^(MLA|MLS|MAD|MSB)_ZPmZZ_[HSD]$",
                                                                                "^(MLA|MLS)_ZZZI_[HSD]$")>;

// Predicate counting vector
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^(SQ|UQ)?(DEC|INC)[HWD]_ZPiI$")>;

// Reduction, arithmetic
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^[SU]ADDV_VPZ_[BHSD]$")>;

// Reduction, arithmetic, B H element size
def : InstRW<[HIP09Write_4cyc_1FSU02],                          (instregex "^[SU](MAX|MIN)V_VPZ_D$")>;

// Reduction, arithmetic, S D element size
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^[SU](MAX|MIN)V_VPZ_[BHS]$")>;

// Reduction, logical
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^(AND|EOR|OR)V_VPZ_[BHSD]$")>;

// Reverse, vector
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^REV_ZZ_[BHSD]$")>;

// Reverse within elements
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^REV[BHW]_ZPmZ_[HSD]$")>;

// Select, vector form
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^SEL_PPPP$")>;

// Table lookup
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^TBL_ZZZ_[BHSD]$")>;

// Transpose, vector form
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^TRN[12]_ZZZ_[BHSDQ]$")>;

// Unpack and extend
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^[SU]UNPK(HI|LO)_ZZ_[HSD]$")>;

// Zip/unzip
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^(UZP|ZIP)[12]_ZZZ_[BHSDQ]$")>;

// 3.24 SVE Floating-point Instructions
// -----------------------------------------------------------------------------

// Floating point absolute value
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^FABS_ZPmZ_[HSD]$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^FABS_ZPmZ_[HSD]_UNDEF$")>;

// Floating point negative value
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^FNEG_ZPmZ_[HSD]$")>;
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^FNEG_ZPmZ_[HSD]_UNDEF$")>;

// Floating point absolute difference
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^FABD_ZPmZ_[HSD]$")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^FABD_ZPZZ")>;

// Floating point arithmetic
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^F(ADD|SUB|SUBR)_ZPm[IZ]_[HSD]$")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^F(ADD|SUB)_ZZZ_[HSD]$")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^F(ADD|SUB|SUBR)_ZPZ[IZ]")>;


// Floating point compare
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^FACG[ET]_PPzZZ_[HSD]$",
                                                                                "^FCM(GE|GT|EQ|NE|UO)_PPzZZ_[HSD]$",
                                                                                "^FCM(GE|GT|LT|LE|EQ|NE)_PPzZ0_[HSD]$")>;

// Floating point complex add
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^FCADD_ZPmZ_[HSD]$")>;

// Floating point complex multiply add
def : InstRW<[HIP09Write_5cyc_1FSU02],                          (instregex "^FCMLA_ZPmZZ_[HSD]$",
                                                                                "^FCMLA_ZZZI_[HS]$")>;

// Floating point convert, long or narrow
def : InstRW<[HIP09Write_3cyc_2FSU02],                          (instregex "^FCVT_ZPmZ")>;

// Floating point convert to integer
def : InstRW<[HIP09Write_3cyc_2FSU02],                          (instregex "^FCVTZ[SU]_ZPmZ")>;

// Floating point copy
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^FCPY_ZPmI_[HSD]$", "^FDUP_ZI_[HSD]$")>;

// Floating point divide, F16 / f32
def : InstRW<[HIP09Write_13cyc_1FSU02_9RC],                     (instregex "^FDIVR?_ZPmZ_[HS]$")>;
def : InstRW<[HIP09Write_13cyc_1FSU02_9RC],                     (instregex "^FDIVR?_ZPZZ_[HS]_(UNDEF|ZERO)$")>;

// Floating point divide, F64
def : InstRW<[HIP09Write_15cyc_1FSU02_11RC],                    (instregex "^FDIVR?_ZPmZ_D$")>;
def : InstRW<[HIP09Write_15cyc_1FSU02_11RC],                    (instregex "^FDIVR?_ZPZZ_D_(UNDEF|ZERO)$")>;

// Floating point min/max
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^F(MAX|MIN)(NM)?_ZPm[ZI]_[HSD]$")>;
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^F(MAX|MIN)(NM)?_ZPZ[ZI]")>;

// Floating point multiply
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^FMUL_(ZPmI|ZPmZ|ZZZI?)_[HSD]$")>;
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^FMUL_ZPZ[ZI]")>;

// Floating point multiply
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^(FSCALE|FMULX)_ZPmZ_[HSD]$")>;
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^FMULX_ZPZZ")>;

// Floating point multiply accumulate
def : InstRW<[HIP09Write_4cyc_1FSU02],                          (instregex "^FN?(MLA|MLS|MAD|MSB)_ZPmZZ_[HSD]$")>;
def : InstRW<[HIP09Write_4cyc_1FSU02],                          (instregex "^FML[AS]_ZZZI_[HSD]$")>;
def : InstRW<[HIP09Write_4cyc_1FSU02],                          (instregex "^FN?ML[AS]_ZPZZZ_[HSD]_UNDEF$")>;

// Floating point reciprocal estimate
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^FR(ECPE|SQRTE)_ZZ_[HSD]$")>;
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^FRECPX_ZPmZ_[HSD]$")>;
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^FRECPX_ZPmZ_[HSD]_UNDEF$")>;

// Floating point reciprocal step
def : InstRW<[HIP09Write_4cyc_1FSU02],                          (instregex "^FR(ECPS|SQRTS)_ZZZ_[HSD]$")>;

// Floating point reduction, F16
def : InstRW<[HIP09Write_12cyc_1FSU02_4RC],                     (instregex "^FADDV_VPZ_H$")>;

// Floating point reduction, F32
def : InstRW<[HIP09Write_9cyc_1FSU02_3RC],                      (instregex "^FADDV_VPZ_S$")>;

// Floating point reduction, F64
def : InstRW<[HIP09Write_6cyc_2FSU02],                          (instregex "^FADDV_VPZ_D$")>;

// Floating point reduction, F16, F32
def : InstRW<[HIP09Write_3cyc_1FSU02],                          (instregex "^F(MAX|MIN)(NM)?V_VPZ_[HS]$")>;

// Floating point reduction, F64
def : InstRW<[HIP09Write_2cyc_1FSU02],                          (instregex "^F(MAX|MIN)(NM)?V_VPZ_D$")>;

// Floating point round to integral
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^FRINT[AMNPXZI]_ZPmZ_[HSD]$")>;
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^FRINT[AMNPXZI]_ZPmZ_[HSD]_UNDEF$")>;

// Floating point square root, F16 / F32
def : InstRW<[HIP09Write_21cyc_1FSU02_17RC],                    (instregex "^FSQRT_ZPmZ_[HS]$")>;
def : InstRW<[HIP09Write_21cyc_1FSU02_17RC],                    (instregex "^FSQRT_ZPmZ_[HS]_UNDEF$")>;

// Floating point square root, F64
def : InstRW<[HIP09Write_25cyc_1FSU02_21RC],                    (instregex "^FSQRT_ZPmZ_D$")>;
def : InstRW<[HIP09Write_25cyc_1FSU02_21RC],                    (instregex "^FSQRT_ZPmZ_D_UNDEF")>;

// Floating point trigonometric exponentiation
def : InstRW<[HIP09Write_2cyc_2FSU02],                          (instregex "^FEXPA_ZZ_[HSD]$")>;

// Floating point trigonometric multiply add
def : InstRW<[HIP09Write_4cyc_1FSU02],                          (instregex "^FTMAD_ZZI_[HSD]$")>;

// Floating point trigonometric, miscellaneous
def : InstRW<[HIP09Write_4cyc_1FSU02],                          (instregex "^FTSMUL_ZZZ_[HSD]$")>;

// Floating point trigonometric, miscellaneous
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^FTSSEL_ZZZ_[HSD]$")>;

// Floating point associative add, F16
def : InstRW<[HIP09Write_36cyc_1FSU02_32RC],                    (instrs FADDA_VPZ_H)>;

// Floating point associative add, F32
def : InstRW<[HIP09Write_20cyc_1FSU02_16RC],                    (instrs FADDA_VPZ_S)>;

// Floating point associative add, F64
def : InstRW<[HIP09Write_12cyc_1FSU02_8RC],                     (instrs FADDA_VPZ_D)>;

// SVE BFlot16 (BF16) Instructions
// -----------------------------------------------------------------------------

// Convert, F32 to BF16
def : InstRW<[HIP09Write_3cyc_2FSU02],                          (instregex "^BFCVT(NT)?_ZPmZ$")>;

// Dot product
def : InstRW<[HIP09Write_6cyc_4FSU],                            (instregex "^BFDOT_ZZ[ZI]$")>;

// Matrix multiply accumulate
def : InstRW<[HIP09Write_9cyc_1FSU_8RC],                        (instregex "^BFMMLA_ZZZ$")>;

// Multiply accumulate long
def : InstRW<[HIP09Write_5cyc_2FSU],                            (instregex "^BFMLAL[BT]_ZZZI?$")>;

// SVE Load Instructions
// -----------------------------------------------------------------------------

// Load vector
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LDR_ZXI$")>;

// Load predicate
def : InstRW<[HIP09Write_8cyc_1LD_1FSU],                        (instregex "^LDR_PXI$")>;

// Contiguous load, scalar + imm
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LD1(B|H|W|D)_IMM_REAL",
                                                                                "^LD1(B|H|W|SB|SH|SW)_[HSD]_IMM_REAL")>;

// Contiguous load, scalar + scalar
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LD1(B|H|W|D|SB|SH|SW)(_[HSD])?$")>;

// Contiguous load broadcast, scalar + imm
def : InstRW<[HIP09Write_8cyc_2LD_2FSU02],                      (instregex "^LD1R(B|H|W|D|SB|SH|SW|Q)_IMM$",
                                                                                "^LD1R(B|H|W|D|SB|SH|SW|Q)_[BHSWD]_IMM$")>;

// Contiguous load broadcast, scalar + scalar
def : InstRW<[HIP09Write_8cyc_2LD_2FSU02],                      (instregex "^LD1RQ_[BHWD]$")>;

// Non-temporal load, scalar + imm
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LDNT1[BHWD]_ZRI$")>;

// Non-temporal load, scalar + scalar
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LDNT1[BHWD]_ZRR$")>;

// Contiguous first faulting load, scalar + scalar
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LDFF1(B|H|W|D|SB|SH|SW)_REAL$",
                                                                                "^LDFF1(B|H|W|D|SB|SH|SW)_[HSD]_REAL$")>;

// Contiguous non-faulting load, scalar + imm
def : InstRW<[HIP09Write_6cyc_1LD],                             (instregex "^LDNF1(B|H|W|D|SB|SH|SW)_IMM",
                                                                                "^LDNF1(B|H|W|D|SB|SH|SW)_[HSD]_IMM")>;

// Contiguous Load two structures to two vectors, scalar + imm
// Contiguous Load two structures to two vectors, scalar + scalar
def : InstRW<[HIP09Write_9cyc_4LD_4FSU02],                      (instregex "^LD2[BHWD](_IMM)?$")>;

// Contiguous Load three structures to two vectors, scalar + imm
// Contiguous Load three structures to two vectors, scalar + scalar
def : InstRW<[HIP09Write_11cyc_6LD_6FSU02],                     (instregex "^LD3[BHWD](_IMM)?$")>;

// Contiguous Load four structures to two vectors, scalar + imm
// Contiguous Load four structures to two vectors, scalar + scalar
def : InstRW<[HIP09Write_16cyc_16LD_16FSU02],                   (instregex "^LD4[BHWD](_IMM)?$")>;

// Gather load, vector + imm, 32- bit element size
def : InstRW<[HIP09Write_18cyc_1LD_4RC],                        (instregex "^GLD(FF)?1S?[BH]_S_(IMM|[SU]XTW)(_REAL)?$",
                                                                                "^GLD(FF)?1W_(IMM|[SU]XTW)(_REAL)?")>;

// Gather load, vector + imm, 64- bit element size
def : InstRW<[HIP09Write_16cyc_1LD_4RC],                        (instregex "^GLD(FF)?1S?[BHW]_D_(IMM|REAL|SCALED)",
                                                                                "^GLD(FF)?1D_(IMM|REAL|SCALED)")>;

// Gather load, 32-bit scaled offset
def : InstRW<[HIP09Write_18cyc_1LD_4RC],                        (instregex "^GLD(FF)?1S?[HW]_S_[SU]XTW_SCALED(_REAL)?$")>;

// Gather load, 32-bit unpacked unscaled offset
def : InstRW<[HIP09Write_18cyc_1LD_4RC],                        (instregex "^GLD(FF)?1S?[BHW]_D_[SU]XTW",
                                                                                "^GLD(FF)?1D_[SU]XTW")>;

// Prefetch
def : InstRW<[HIP09Write_18cyc_1LD_4RC],                        (instregex "^PRF[BHWD]_PRI")>;
def : InstRW<[HIP09Write_18cyc_1LD_4RC],                        (instregex "^PRF[BHWD]_PRR")>;
def : InstRW<[HIP09Write_18cyc_1LD_4RC],                        (instregex "^PRF[BHW]_[SD]")>;
def : InstRW<[HIP09Write_18cyc_1LD_4RC],                        (instregex "^PRFD_[SD]")>;

// SVE Store Instructions
// -----------------------------------------------------------------------------

// Store from predicate reg
def : InstRW<[HIP09Write_3cyc_1FSU02_1ST_1STD],                 (instregex "^STR_PXI$")>;

// Store from vector reg
def : InstRW<[HIP09Write_2cyc_2ST_2STD],                        (instregex "^STR_ZXI$")>;

// SVE contiguous store (scalar plus immediate)
def : InstRW<[HIP09Write_2cyc_2ST_2STD],                        (instregex "^ST1[BHWD]_IMM$",
                                                                                "^ST1B_[HSD]_IMM$",
                                                                                "^ST1H_[SD]_IMM$",
                                                                                "^ST1W_D_IMM$")>;

// SVE contiguous store (scalar plus scalar)
def : InstRW<[HIP09Write_2cyc_2ST_2STD],                        (instregex "^ST1[BHWD]$",
                                                                                "^ST1B_[HSD]$",
                                                                                "^ST1H_[SD]$",
                                                                                "^ST1W_D$")>;

// Contiguous store two structures from two vectors
def : InstRW<[HIP09Write_6cyc_3FSU02_3ST_3STD],                 (instregex "^ST2[BHWD](_IMM)?$")>;

// Contiguous store three structures from three vectors
def : InstRW<[HIP09Write_6cyc_4FSU02_4ST_4STD],                 (instregex "^ST3[BHWD](_IMM)?$")>;

// Contiguous store four structures from four vectors
def : InstRW<[HIP09Write_8cyc_16FSU02_16ST_16STD],              (instregex "^ST4[BHWD](_IMM)?$")>;

// non-tenporal store, scalar + imm
def : InstRW<[HIP09Write_2cyc_2ST_2STD],                        (instregex "^STNT1[BHWD]_ZRI$")>;

// Non-temporal store, scalar + scala
def : InstRW<[HIP09Write_2cyc_2ST_2STD],                        (instregex "^STNT1[BHWD]_ZRR$")>;

// Scatter store vector + imm 32-bit element size
def : InstRW<[HIP09Write_4cyc_8ST_8STD],                        (instregex "^SST1[BH]_S_IMM$",
                                                                                "^SST1W_IMM$")>;

// Scatter store vector + imm 64-bit element size
def : InstRW<[HIP09Write_2cyc_4ST_4STD],                        (instregex "^SST1[BHW]_D_IMM$",
                                                                                "^SST1D_IMM$")>;

// Scatter store, 32-bit scaled offset
def : InstRW<[HIP09Write_4cyc_8ST_8STD],                        (instregex "^SST1H_S_[SU]XTW_SCALED$",
                                                                                "^SST1W_[SU]XTW_SCALED$")>;

// Scatter store, 32-bit unpacked unscaled offset
def : InstRW<[HIP09Write_4cyc_8ST_8STD],                        (instregex "^SST1[BHW]_D_[SU]XTW$",
                                                                                "^SST1D_[SU]XTW$")>;

// Scatter store, 32-bit unpacked scaled offset
def : InstRW<[HIP09Write_4cyc_8ST_8STD],                        (instregex "^SST1[HW]_D_[SU]XTW_SCALED$",
                                                                                "^SST1D_[SU]XTW_SCALED$")>;

// Scatter store, 32-bit unscaled offset
def : InstRW<[HIP09Write_4cyc_8ST_8STD],                        (instregex "^SST1[BH]_S_[SU]XTW$",
                                                                                "^SST1W_[SU]XTW$")>;

// Scatter store, 64-bit scaled offset
def : InstRW<[HIP09Write_2cyc_4ST_4STD],                        (instregex "^SST1[HW]_D_SCALED",
                                                                                "^SST1D_SCALED")>;

// Scatter store, 64-bit unscaled offset
def : InstRW<[HIP09Write_2cyc_4ST_4STD],                        (instregex "^SST1[BHW]_D$",
                                                                                "^SST1D$")>;

// SVE Miscellaneous Instructions
// -----------------------------------------------------------------------------

// Read first fault register, unpredicated
// Read first fault register, predicated
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^RDFFR_P(Pz)?_REAL$")>;

// Read first fault register and set flags
def : InstRW<[HIP09Write_1cyc_1FSU02],                          (instregex "^RDFFRS_PPz$")>;

// Set first fault register
def : InstRW<[HIP09Write_0cyc],                                 (instregex "^SETFFR$")>;

// Write to first fault register
def : InstRW<[HIP09Write_9cyc_18FSU02_9ALUM1],                  (instrs WRFFR)>;


// -----------------------------------------------------------------------------
} // SchedModel = HIP09Model

