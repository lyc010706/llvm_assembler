//===---- Sw64ABIInfo.h - Information about SW64 ABI's --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64ABIINFO_H
#define LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64ABIINFO_H

#include "llvm/IR/CallingConv.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/TargetParser/Triple.h"

namespace llvm {

template <typename T> class ArrayRef;
class MCTargetOptions;
class StringRef;
class TargetRegisterClass;

class Sw64ABIInfo {
public:
  enum class ABI { Unknown, S64 };

protected:
  ABI ThisABI;

public:
  Sw64ABIInfo(ABI ThisABI) : ThisABI(ThisABI) {}

  static Sw64ABIInfo Unknown() { return Sw64ABIInfo(ABI::Unknown); }
  static Sw64ABIInfo S64() { return Sw64ABIInfo(ABI::S64); }
  static Sw64ABIInfo computeTargetABI(const Triple &TT, StringRef CPU,
                                      const MCTargetOptions &Options);

  bool IsKnown() const { return ThisABI != ABI::Unknown; }
  bool IsS64() const { return ThisABI == ABI::S64; }
  ABI GetEnumValue() const { return ThisABI; }

  /// The registers to use for byval arguments.
  ArrayRef<MCPhysReg> GetByValArgRegs() const;

  /// The registers to use for the variable argument list.
  ArrayRef<MCPhysReg> GetVarArgRegs() const;

  /// Obtain the size of the area allocated by the callee for arguments.
  /// CallingConv::FastCall affects the value for S32.
  unsigned GetCalleeAllocdArgSizeInBytes(CallingConv::ID CC) const;

  /// Ordering of ABI's
  /// Sw64GenSubtargetInfo.inc will use this to resolve conflicts when given
  /// multiple ABI options.
  bool operator<(const Sw64ABIInfo Other) const {
    return ThisABI < Other.GetEnumValue();
  }

  unsigned GetStackPtr() const;
  unsigned GetFramePtr() const;
  unsigned GetBasePtr() const;
  unsigned GetGlobalPtr() const;
  unsigned GetNullPtr() const;
  unsigned GetZeroReg() const;
  unsigned GetPtrAdduOp() const;
  unsigned GetPtrAddiuOp() const;
  unsigned GetPtrSubuOp() const;
  unsigned GetPtrAndOp() const;
  unsigned GetGPRMoveOp() const;
  inline bool ArePtrs64bit() const { return IsS64(); }
  inline bool AreGprs64bit() const { return IsS64(); }

  unsigned GetEhDataReg(unsigned I) const;
};
} // namespace llvm
#endif
