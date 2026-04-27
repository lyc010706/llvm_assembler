//===-- Sw64TargetObjectFile.h - Sw64 Object Info -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_SW64_SW64TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_SW64_SW64TARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

static const unsigned CodeModelLargeSize = 256;

class Sw64TargetObjectFile : public TargetLoweringObjectFileELF {
  MCSection *BSSSectionLarge;
  MCSection *DataSectionLarge;
  MCSection *ReadOnlySectionLarge;
  MCSection *DataRelROSectionLarge;
  MCSection *SmallDataSection;
  MCSection *SmallBSSSection;
  unsigned SSThreshold = 8;

public:
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;
  /// Return true if this global address should be placed into small data/bss
  /// section.
  bool isGlobalInSmallSection(const GlobalObject *GO,
                              const TargetMachine &TM) const;

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override;

  /// Return true if this constant should be placed into small data section.
  bool isConstantInSmallSection(const DataLayout &DL, const Constant *CN) const;

  MCSection *getSectionForConstant(const DataLayout &DL, SectionKind Kind,
                                   const Constant *C,
                                   Align &Alignment) const override;

  void getModuleMetadata(Module &M) override;

  bool isInSmallSection(uint64_t Size) const;
};
} // end namespace llvm
#endif
