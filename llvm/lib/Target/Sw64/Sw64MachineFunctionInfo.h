//===- Sw64MachineFunctionInfo.h - Sw64 machine function info -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares Sw64-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_SW64MACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_SW64_SW64MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include <cassert>
#include <utility>
#include <vector>

namespace llvm {

// Sw64MachineFunctionInfo - This class is derived from MachineFunction private
// Sw64 target-specific information for each MachineFunction.
class Sw64MachineFunctionInfo : public MachineFunctionInfo {
private:
  // GlobalBaseReg - keeps track of the virtual register initialized for
  // use as the global base register. This is used for PIC in some PIC
  // relocation models.
  unsigned GlobalBaseReg;

  // GlobalRetAddr = keeps track of the virtual register initialized for
  // the return address value.
  unsigned GlobalRetAddr;

  // VarArgsOffset - What is the offset to the first vaarg
  int VarArgsOffset;
  // VarArgsBase - What is the base FrameIndex
  int VarArgsBase;

  virtual void anchor();
  mutable int CachedEStackSize = -1;

public:
  Sw64MachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
      : GlobalBaseReg(0), GlobalRetAddr(0), VarArgsOffset(0), VarArgsBase(0) {}

  //~Sw64MachineFunctionInfo() override;

  bool globalBaseRegSet() const;
  unsigned getGlobalBaseReg(MachineFunction &MF) const { return GlobalBaseReg; }
  void setGlobalBaseReg(unsigned Reg) { GlobalBaseReg = Reg; }

  bool globalRetAddrSet() const;
  void setGlobalRetAddr(unsigned Reg) { GlobalRetAddr = Reg; }
  unsigned getGlobalRetAddr(MachineFunction &MF) const { return GlobalRetAddr; }

  int getVarArgsOffset() const { return VarArgsOffset; }
  void setVarArgsOffset(int Offset) { VarArgsOffset = Offset; }

  int getVarArgsBase() const { return VarArgsBase; }
  void setVarArgsBase(int Base) { VarArgsBase = Base; }
  bool isLargeFrame(const MachineFunction &MF) const;
};
} // end namespace llvm
#endif // LLVM_LIB_TARGET_SW64_SW64MACHINEFUNCTIONINFO_H
