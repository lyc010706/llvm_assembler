//===- Sw64.td - Describe the Sw64 Target Machine --------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

// Get the target-independent interfaces which we are implementing...

include "llvm/Target/Target.td"

//Sw64 is little endian

//===----------------------------------------------------------------------===//
// Subtarget Features
//===----------------------------------------------------------------------===//

def FeatureCIX : SubtargetFeature<"cix", "HasCT", "true",
                                  "Enable CIX extensions">;

// argument, type, value, help text

def Featurecore3b : SubtargetFeature<"core3b", "Sw64ArchVersion", "core3b",
                                  "Enable core4b Feature">;

def Featurecore4 : SubtargetFeature<"core4", "Sw64ArchVersion", "core4",
                                  "Enable core4 Feature">;

def FeatureRelax : SubtargetFeature<"relax", "relax", "true",
                                  "Enable relax ld attribute">;

def FeatureEv : SubtargetFeature<"swEv", "Ev", "true",
                                  "Enable Sw6a Feature test">;
foreach i = {1-14, 22-25} in
    def FeatureReserve#i : SubtargetFeature<"reserve-r"#i, "ReserveRegister["#i#"]", "true",
                                             "Reserve "#i#", making it unavailable "
                                             "as a GPR">;

def FeatureOptMul : SubtargetFeature<"swOptMul", "Sw64OptMul", "true",
                                     "Enable Sw6b optimize mul">;

def Featureintarith : SubtargetFeature<"swIntArith", "Sw64EnableIntAri", "true",
                                       "Enable core4 integer arithmetic instructions">;
def Featureintshift : SubtargetFeature<"swIntShift", "Sw64EnableIntShift", "true",
                                       "Enable core4 integer shift instructions">;
def Featurebyteinst : SubtargetFeature<"swByteInst", "Sw64EnableByteInst", "true",
                                       "Enable core4 byte manipulation instructions">;
def Featurefloatarith : SubtargetFeature<"swFloatArith", "Sw64EnableFloatAri", "true",
                                       "Enable core4 float arithmetic instructions">;
def Featurefloatround : SubtargetFeature<"swFloatRound", "Sw64EnableFloatRound", "true",
                                       "Enable core4 float round instructions">;
def Featurepostinc : SubtargetFeature<"swPostInc", "Sw64EnablePostInc", "true",
                                       "Enable core4 post-inc load and store instructions">;
def Featurecrcinst : SubtargetFeature<"swCrcInst", "Sw64EnableCrcInst", "true",
                                       "Enable core4 crc32 instructions">;

def FeatureSIMD : SubtargetFeature<"simd", "HasSIMD", "true",
				  "Sw64 SIMD Instruction">;

//***********************
// Subtarget Support test
//***********************
def HasMieee : Predicate<"MF->getSubtarget<Sw64Subtarget>().hasMieee()">,
               AssemblerPredicate<(all_of FeatureCIX)>;

def HasCore3b : Predicate<"Subtarget->hasCore3b()">,
              AssemblerPredicate<(all_of Featurecore3b)>;

def HasCore4 : Predicate<"Subtarget->hasCore4()">,
              AssemblerPredicate<(all_of Featurecore4)>;

def enRelax : Predicate<"Subtarget->enRelax()">,
              AssemblerPredicate<(all_of FeatureRelax)>;

def HasEv : Predicate<"Subtarget->hasEv()">,
            AssemblerPredicate<(all_of FeatureEv)>;


//===----------------------------------------------------------------------===//
// Register File Description
//===----------------------------------------------------------------------===//

include "Sw64RegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Convention Description
//===----------------------------------------------------------------------===//

include "Sw64CallingConv.td"

//===----------------------------------------------------------------------===//
// Base Schedule Description
//===----------------------------------------------------------------------===//

include "Sw64Schedule.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//

include "Sw64InstrInfo.td"


//===----------------------------------------------------------------------===//
// MicroArchitechural Schedule Descriptions
//===----------------------------------------------------------------------===//

include "Sw64SchedCore3.td"
include "Sw64SchedCore4.td"
include "Sw64SchedCore3SIMD.td"

def Sw64InstrInfo : InstrInfo {
}

//===----------------------------------------------------------------------===//
// Sw64 Processor Definitions
//===----------------------------------------------------------------------===//

//***********************
//  Sw processor test
//***********************

class Proc<string Name, list<SubtargetFeature> Features>
    : ProcessorModel<Name, GenericSw64Model, Features>;

def : Proc<"sw_64", []>;
def : Proc<"sw6a", [Featurecore3b]>;
def : Proc<"sw6b", [Featurecore3b]>;
def : Proc<"sw4d", [Featurecore3b]>;
def : Proc<"sw8a", [Featurecore3b, Featurecore4]>;

//===----------------------------------------------------------------------===//
// The Sw64 Target
//===----------------------------------------------------------------------===//
def Sw64AsmWriter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def Sw64AsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 0;
}

def Sw64 : Target {
  // Pull in Instruction Info:
  let InstructionSet = Sw64InstrInfo;
  let AssemblyWriters = [Sw64AsmWriter];
  let AssemblyParsers = [Sw64AsmParser];
}
