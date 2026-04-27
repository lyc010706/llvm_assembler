//===- Sw64SchedCore3SIMD.td - Sw64 Scheduling Definitions -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

let SchedModel = GenericSw64Model in {

def :  WriteRes<WriteFLDS, [C3PortFPU]> { let Latency = 4; }
def :  WriteRes<WriteFSTDS, [C3PortFPU0]> { let Latency = 4; }
def :  WriteRes<WriteVEADD, [C3PortFPU]> { let Latency = 2; }
def :  WriteRes<WriteVESHT2, [C3PortFPU0]> { let Latency = 2; }
def :  WriteRes<WriteVESHT3, [C3PortFPU0]> { let Latency = 3; }
def :  WriteRes<WriteVECNT2, [C3PortFPU0]> { let Latency = 2; }
def :  WriteRes<WriteVECNT3, [C3PortFPU0]> { let Latency = 3; }
def :  WriteRes<WriteVESEL, [C3PortFPU0]> { let Latency = 2; }
def :  WriteRes<WriteFMA2, [C3PortFPU]> { let Latency = 2; }
def :  WriteRes<WriteFMA6, [C3PortFPU]> { let Latency = 6; }
def :  WriteRes<WriteFMA17, [C3PortFPU]> { let Latency = 17; }
def :  WriteRes<WriteVPM1, [C3PortFPU0]> { let Latency = 1; }
def :  WriteRes<WriteVPM2, [C3PortFPU0]> { let Latency = 2; }
def :  WriteRes<WriteVCON1, [C3PortFPU]> { let Latency = 1; }
def :  WriteRes<WriteVCON2, [C3PortFPU]> { let Latency = 2; }
def :  WriteRes<WriteVSUM, [C3PortFPU]> { let Latency = 2; }
def :  WriteRes<WriteVFREC, [C3PortFPU]> { let Latency = 3; }
def :  WriteRes<WriteVFCT, [C3PortFPU]> { let Latency = 4; }
def :  WriteRes<WriteVFRIS, [C3PortFPU]> { let Latency = 3; }

def : InstRW<[WriteFLDS], (instregex "^(VLD)(W|S|D)(E)$")>;
def : InstRW<[WriteFLDS], (instregex "^(VLD)(S|D)$")>;

def : InstRW<[WriteFLDS], (instregex "^(VLD)(W|S|D)(U)$")>;
def : InstRW<[WriteFSTDS], (instregex "^(VST)(W|S|D)(U)$")>;
def : InstRW<[WriteFSTDS], (instregex "^(VST)(WU|SU|DU)(L|H)$")>;

def : InstRW<[WriteFLDS], (instrs VLDDNC)>;
def : InstRW<[WriteFSTDS], (instrs VSTDNC)>;

def : InstRW<[WriteFMA6], (instregex "^(V)(ADD|SUB|MUL)(S|D)$")>;

def : InstRW<[WriteFMA6], (instregex "^(VFCMP)(EQ|LE|LT|UN)$")>;
def : InstRW<[WriteFMA2], (instregex "^(VCPY)(S|SE|SN)$")>;
def : InstRW<[WriteFMA2], (instregex "^(V)(M|NM)(A|S)(S|D)$")>;

def : InstRW<[WriteFMA2], (instregex "^(VFSEL)(EQ|LT|LE)$")>;
def : InstRW<[WriteVPM1], (instregex "^(V)(INS|EXT|CPY)(W|FS|FD)$")>;

def : InstRW<[WriteVPM1], (instregex "^(VINSECTL)(H|W|L|B)$")>;

def : InstRW<[WriteVCON1], (instregex "^(VCON)(W|S|D)$")>;
def : InstRW<[WriteVCON2], (instrs VSHFW)>;

def : InstRW<[WriteVPM2], (instrs VLOGZZ)>;
}
