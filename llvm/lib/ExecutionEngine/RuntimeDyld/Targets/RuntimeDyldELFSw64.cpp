//===-- RuntimeDyldELFSw64.cpp ---- ELF/Sw64 specific code. -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RuntimeDyldELFSw64.h"
#include "llvm/BinaryFormat/ELF.h"

#define DEBUG_TYPE "dyld"

void RuntimeDyldELFSw64::resolveRelocation(const RelocationEntry &RE,
                                           uint64_t Value) {
  const SectionEntry &Section = Sections[RE.SectionID];

  resolveSw64Relocation(Section, RE.Offset, Value, RE.RelType, RE.Addend,
                        RE.SymOffset, RE.SectionID);
}

uint64_t RuntimeDyldELFSw64::evaluateRelocation(const RelocationEntry &RE,
                                                uint64_t Value,
                                                uint64_t Addend) {
  const SectionEntry &Section = Sections[RE.SectionID];
  Value = evaluateSw64Relocation(Section, RE.Offset, Value, RE.RelType, Addend,
                                 RE.SymOffset, RE.SectionID);
  return Value;
}

void RuntimeDyldELFSw64::applyRelocation(const RelocationEntry &RE,
                                         uint64_t Value) {
  const SectionEntry &Section = Sections[RE.SectionID];
  applySw64Relocation(Section.getAddressWithOffset(RE.Offset), Value,
                      RE.RelType);
  return;
}

int64_t RuntimeDyldELFSw64::evaluateSw64Relocation(
    const SectionEntry &Section, uint64_t Offset, uint64_t Value, uint32_t Type,
    int64_t Addend, uint64_t SymOffset, SID SectionID) {

  LLVM_DEBUG(dbgs() << "evaluateSw64Relocation, LocalAddress: 0x"
                    << format("%llx", Section.getAddressWithOffset(Offset))
                    << " GOTAddr: 0x"
                    << format("%llx",
                              getSectionLoadAddress(SectionToGOTMap[SectionID]))
                    << " FinalAddress: 0x"
                    << format("%llx", Section.getLoadAddressWithOffset(Offset))
                    << " Value: 0x" << format("%llx", Value) << " Type: 0x"
                    << format("%x", Type) << " Addend: 0x"
                    << format("%llx", Addend)
                    << " Offset: " << format("%llx", Offset)
                    << " SID: " << format("%d", SectionID)
                    << " SymOffset: " << format("%x", SymOffset) << "\n");

  switch (Type) {
  default:
    llvm_unreachable("Not implemented relocation type!");
    break;
  case ELF::R_SW_64_GPDISP: {
    uint64_t GOTAddr = getSectionLoadAddress(SectionToGOTMap[SectionID]);
    uint32_t *LocalAddress =
        reinterpret_cast<uint32_t *>(Section.getAddressWithOffset(Offset));

    uint8_t *LocalGOTAddr =
        getSectionAddress(SectionToGOTMap[SectionID]) + SymOffset;
    uint64_t GOTEntry = readBytesUnaligned(LocalGOTAddr, getGOTEntrySize());

    LLVM_DEBUG(dbgs() << "Debug gpdisp: "
                      << " GOTAddr: 0x" << format("%llx", GOTAddr)
                      << " GOTEntry: 0x" << format("%llx", GOTEntry)
                      << " LocalGOTAddr: 0x" << format("%llx", LocalGOTAddr)
                      << " LocalAddress: 0x" << format("%llx", LocalAddress)
                      << "\n");
    if (GOTEntry)
      assert(GOTEntry == Value && "GOT entry has two different addresses.");
    else
      writeBytesUnaligned(Value, LocalGOTAddr, getGOTEntrySize());

    return (int64_t)GOTAddr + 0x8000 - (int64_t)LocalAddress;
  }
  case ELF::R_SW_64_LITERAL: {
    uint64_t GOTAddr = getSectionLoadAddress(SectionToGOTMap[SectionID]);
    uint32_t *LocalAddress =
        reinterpret_cast<uint32_t *>(Section.getAddressWithOffset(Offset));

    uint8_t *LocalGOTAddr =
        getSectionAddress(SectionToGOTMap[SectionID]) + SymOffset;
    uint64_t GOTEntry = readBytesUnaligned(LocalGOTAddr, getGOTEntrySize());

    LLVM_DEBUG(dbgs() << "Debug literal: "
                      << " GOTAddr: 0x" << format("%llx", GOTAddr)
                      << " GOTEntry: 0x" << format("%llx", GOTEntry)
                      << " LocalGOTAddr: 0x" << format("%llx", LocalGOTAddr)
                      << " LocalAddress: 0x" << format("%llx", LocalAddress)
                      << "\n");

    Value += Addend;
    if (GOTEntry)
      assert(GOTEntry == Value && "GOT entry has two different addresses.");
    else
      writeBytesUnaligned(Value, LocalGOTAddr, getGOTEntrySize());

    if (SymOffset > 65536)
      report_fatal_error(".got subsegment exceeds 64K (literal)!!\n");

    if ((SymOffset) < 32768)
      return (int64_t)(SymOffset - 0x8000);
    else
      return (int64_t)(0x8000 - SymOffset);
  }
  case ELF::R_SW_64_GPRELHIGH: {
    // Get the higher 16-bits.
    uint64_t GOTAddr = getSectionLoadAddress(SectionToGOTMap[SectionID]);
    uint64_t Disp = Value + Addend - (GOTAddr + 0x8000);
    if (Disp & 0x8000)
      return ((Disp + 0x8000) >> 16) & 0xffff;
    else
      return (Disp >> 16) & 0xffff;
  }
  case ELF::R_SW_64_GPRELLOW: {
    uint64_t GOTAddr = getSectionLoadAddress(SectionToGOTMap[SectionID]);

    return (Value + Addend - (GOTAddr + 0x8000)) & 0xffff;
  }
  case ELF::R_SW_64_REFQUAD: {
    return Value + Addend;
  }
  case ELF::R_SW_64_SREL32: {
    uint64_t FinalAddress = Section.getLoadAddressWithOffset(Offset);
    return Value + Addend - FinalAddress;
  }
  case ELF::R_SW_64_GPREL32: {
    uint64_t GOTAddr = getSectionLoadAddress(SectionToGOTMap[SectionID]);
    return Value + Addend - (GOTAddr + 0x7ff0);
  }
  case ELF::R_SW_64_TPRELHI:
  case ELF::R_SW_64_TPRELLO:
    report_fatal_error("Current Sw64 JIT does not support TPREL relocs");
    break;
  case ELF::R_SW_64_LITERAL_GOT:
  case ELF::R_SW_64_HINT:
  case ELF::R_SW_64_LITUSE:
    return 0;
  }
  return 0;
}

void RuntimeDyldELFSw64::applySw64Relocation(uint8_t *TargetPtr, int64_t Value,
                                             uint32_t Type) {
  uint32_t Insn = readBytesUnaligned(TargetPtr, 4);
  int64_t Disp_hi, Disp_lo;

  switch (Type) {
  default:
    llvm_unreachable("Unknown relocation type!");
    break;
  case ELF::R_SW_64_GPDISP: {
    uint32_t Insn1 = readBytesUnaligned(TargetPtr + 4, 4);
    if ((Value > 2147483647LL) || (Value < -2147483648LL)) {
      llvm::dbgs() << "gpdisp Value=" << Value << "\n";
      report_fatal_error(".got subsegment exceeds 2GB (gpdisp)!!\n");
    }

    Disp_hi = (Value + 0x8000) >> 16;
    Disp_lo = Value & 0xffff;

    Insn = (Insn & 0xffff0000) | (Disp_hi & 0x0000ffff);
    Insn1 = (Insn1 & 0xffff0000) | (Disp_lo & 0x0000ffff);

    writeBytesUnaligned(Insn, TargetPtr, 4);
    writeBytesUnaligned(Insn1, TargetPtr + 4, 4);
    break;
  }
  case ELF::R_SW_64_LITERAL:
    Insn = (Insn & 0xffff0000) | (Value & 0x0000ffff);
    writeBytesUnaligned(Insn, TargetPtr, 4);
    break;
  case ELF::R_SW_64_LITERAL_GOT:
    Insn = (Insn & 0xffff0000) | (Value & 0x0000ffff);
    writeBytesUnaligned(Insn, TargetPtr, 4);
    break;
  case ELF::R_SW_64_GPRELHIGH:
  case ELF::R_SW_64_GPRELLOW:
    Insn = (Insn & 0xffff0000) | (Value & 0x0000ffff);
    writeBytesUnaligned(Insn, TargetPtr, 4);
    break;
  case ELF::R_SW_64_REFQUAD:
    writeBytesUnaligned(Value, TargetPtr, 8);
    break;
  case ELF::R_SW_64_SREL32:
    writeBytesUnaligned(Value & 0xffffffff, TargetPtr, 4);
    break;
  case ELF::R_SW_64_GPREL32:
    writeBytesUnaligned(Value & 0xffffffff, TargetPtr, 4);
    break;
  }
}

void RuntimeDyldELFSw64::resolveSw64Relocation(const SectionEntry &Section,
                                               uint64_t Offset, uint64_t Value,
                                               uint32_t Type, int64_t Addend,
                                               uint64_t SymOffset,
                                               SID SectionID) {
  uint32_t r_type = Type & 0xff;

  // RelType is used to keep information for which relocation type we are
  // applying relocation.
  uint32_t RelType = r_type;
  int64_t CalculatedValue = evaluateSw64Relocation(
      Section, Offset, Value, RelType, Addend, SymOffset, SectionID);

  applySw64Relocation(Section.getAddressWithOffset(Offset), CalculatedValue,
                      RelType);
}
