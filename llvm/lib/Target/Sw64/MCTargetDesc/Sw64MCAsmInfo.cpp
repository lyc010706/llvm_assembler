//===-- Sw64MCAsmInfo.cpp - Sw64 asm properties -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the Sw64MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "Sw64MCAsmInfo.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

void Sw64MCAsmInfo::anchor() {}

Sw64MCAsmInfo::Sw64MCAsmInfo(const Triple &TheTriple,
                             const MCTargetOptions &Options) {
  IsLittleEndian = TheTriple.isLittleEndian();
  assert(IsLittleEndian == true && "sw_64 machine is litter endian!");

  CodePointerSize = CalleeSaveStackSlotSize = 8;

  PrivateGlobalPrefix = ".L";
  AlignmentIsInBytes = false;
  Data16bitsDirective = "\t.2byte\t";
  Data32bitsDirective = "\t.4byte\t";
  Data64bitsDirective = "\t.8byte\t";
  WeakRefDirective = "\t.weak\t";
  CommentString = "#";
  // For chang assemble directer ".set LA, LB" to "LA = LB"
  HasSw64SetDirective = true;
  UsesELFSectionDirectiveForBSS = true;
  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::DwarfCFI;
  DwarfRegNumForCFI = true;
  UseIntegratedAssembler = true;
}
