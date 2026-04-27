//===-- RuntimeDyldELFSw64.h ---- ELF/Sw64 specific code. -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_EXECUTIONENGINE_RUNTIMEDYLD_TARGETS_RUNTIMEDYLDELFSw64_H
#define LLVM_LIB_EXECUTIONENGINE_RUNTIMEDYLD_TARGETS_RUNTIMEDYLDELFSw64_H

#include "../RuntimeDyldELF.h"
#include <string>

#define DEBUG_TYPE "dyld"

namespace llvm {

class RuntimeDyldELFSw64 : public RuntimeDyldELF {
public:
  typedef uint64_t TargetPtrT;

  RuntimeDyldELFSw64(RuntimeDyld::MemoryManager &MM,
                     JITSymbolResolver &Resolver)
      : RuntimeDyldELF(MM, Resolver) {}

  void resolveRelocation(const RelocationEntry &RE, uint64_t Value) override;

protected:
  void resolveSw64Relocation(const SectionEntry &Section, uint64_t Offset,
                             uint64_t Value, uint32_t Type, int64_t Addend,
                             uint64_t SymOffset, SID SectionID);

  uint64_t GOTOffset = 0;
  uint64_t GPOffset_Modify = 0;

private:
  /// A object file specific relocation resolver
  /// \param RE The relocation to be resolved
  /// \param Value Target symbol address to apply the relocation action
  uint64_t evaluateRelocation(const RelocationEntry &RE, uint64_t Value,
                              uint64_t Addend);

  /// A object file specific relocation resolver
  /// \param RE The relocation to be resolved
  /// \param Value Target symbol address to apply the relocation action
  void applyRelocation(const RelocationEntry &RE, uint64_t Value);

  int64_t evaluateSw64Relocation(const SectionEntry &Section, uint64_t Offset,
                                 uint64_t Value, uint32_t Type, int64_t Addend,
                                 uint64_t SymOffset, SID SectionID);

  void applySw64Relocation(uint8_t *TargetPtr, int64_t CalculatedValue,
                           uint32_t Type);
};
} // namespace llvm

#undef DEBUG_TYPE

#endif
