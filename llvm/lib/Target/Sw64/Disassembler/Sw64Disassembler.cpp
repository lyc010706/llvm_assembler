//===-- Sw64Disassembler.cpp - Disassembler for Sw64 --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Sw64Disassembler class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Sw64MCTargetDesc.h"
#include "TargetInfo/Sw64TargetInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"

using namespace llvm;

#define DEBUG_TYPE "Sw64-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {
class Sw64Disassembler : public MCDisassembler {

public:
  Sw64Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  ~Sw64Disassembler() {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};
} // end anonymous namespace

static MCDisassembler *createSw64Disassembler(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new Sw64Disassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSw64Disassembler() {
  // Register the disassembler for each target.
  TargetRegistry::RegisterMCDisassembler(getTheSw64Target(),
                                         createSw64Disassembler);
}

static const unsigned GPRDecoderTable[] = {
    Sw64::R0,  Sw64::R1,  Sw64::R2,  Sw64::R3,  Sw64::R4,  Sw64::R5,  Sw64::R6,
    Sw64::R7,  Sw64::R8,  Sw64::R9,  Sw64::R10, Sw64::R11, Sw64::R12, Sw64::R13,
    Sw64::R14, Sw64::R15, Sw64::R16, Sw64::R17, Sw64::R18, Sw64::R19, Sw64::R20,
    Sw64::R21, Sw64::R22, Sw64::R23, Sw64::R24, Sw64::R25, Sw64::R26, Sw64::R27,
    Sw64::R28, Sw64::R29, Sw64::R30, Sw64::R31};

// This instruction does not have a working decoder, and needs to be
// fixed. This "fixme" function was introduced to keep the backend comiling
// while making changes to tablegen code.
static DecodeStatus DecodeFIXMEInstruction(MCInst &Inst, uint64_t RegNo,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  return MCDisassembler::Fail;
}

static DecodeStatus DecodeGPRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder) {
  if (RegNo > std::size(GPRDecoderTable))
    return MCDisassembler::Fail;

  // We must define our own mapping from RegNo to register identifier.
  // Accessing index RegNo in the register class will work in the case that
  // registers were added in ascending order, but not in general.
  unsigned Reg = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static const unsigned FPRDecoderTable[] = {
    Sw64::F0,  Sw64::F1,  Sw64::F2,  Sw64::F3,  Sw64::F4,  Sw64::F5,  Sw64::F6,
    Sw64::F7,  Sw64::F8,  Sw64::F9,  Sw64::F10, Sw64::F11, Sw64::F12, Sw64::F13,
    Sw64::F14, Sw64::F15, Sw64::F16, Sw64::F17, Sw64::F18, Sw64::F19, Sw64::F20,
    Sw64::F21, Sw64::F22, Sw64::F23, Sw64::F24, Sw64::F25, Sw64::F26, Sw64::F27,
    Sw64::F28, Sw64::F29, Sw64::F30, Sw64::F31};

static DecodeStatus DecodeF4RCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  if (RegNo > 32) {
    return MCDisassembler::Fail;
  }
  unsigned Reg = FPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeF8RCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  if (RegNo > 32) {
    return MCDisassembler::Fail;
  }
  unsigned Reg = FPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeV256LRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  if (RegNo > 32) {
    return MCDisassembler::Fail;
  }
  unsigned Reg = FPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeFPRC_loRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  if (RegNo > 32) {
    return MCDisassembler::Fail;
  }
  unsigned Reg = FPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeFPRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  if (RegNo > 32) {
    return MCDisassembler::Fail;
  }
  unsigned Reg = FPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t Imm,
                                      int64_t Address, const void *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeUImmNonZeroOperand(MCInst &Inst, uint64_t Imm,
                                             int64_t Address,
                                             const void *Decoder) {
  if (Imm == 0)
    return MCDisassembler::Fail;
  return decodeUImmOperand<N>(Inst, Imm, Address, Decoder);
}

template <unsigned N>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint64_t Imm,
                                      int64_t Address, const void *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  // Sign-extend the number in the bottom N bits of Imm
  Inst.addOperand(MCOperand::createImm(SignExtend64<N>(Imm)));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeSImmNonZeroOperand(MCInst &Inst, uint64_t Imm,
                                             int64_t Address,
                                             const void *Decoder) {
  if (Imm == 0)
    return MCDisassembler::Fail;
  return decodeSImmOperand<N>(Inst, Imm, Address, Decoder);
}

static DecodeStatus decodeFloatCopyInstruction(uint32_t func, MCInst &MI,
                                               uint32_t Insn, uint64_t Address,
                                               const void *Decoder) {
  switch (func) {
  default:
    return MCDisassembler::Fail;
  case 0x30:
    MI.setOpcode(Sw64::CPYSS);
    break;
  case 0x31:
    MI.setOpcode(Sw64::CPYSNS);
    break;
  case 0x32:
    MI.setOpcode(Sw64::CPYSES);
    break;
  }
  uint32_t RegOp1 = Insn << 6 >> 27;  // Inst {25-21} Reg operand 1
  uint32_t RegOp2 = Insn << 11 >> 27; // Inst [20-16] Reg operand 2
  uint32_t RegOp3 = Insn & 0x1F;      // Inst [4-0 ] Reg operand 3
  MI.addOperand(MCOperand::createReg(FPRDecoderTable[RegOp3]));
  MI.addOperand(MCOperand::createReg(FPRDecoderTable[RegOp1]));
  MI.addOperand(MCOperand::createReg(FPRDecoderTable[RegOp2]));
  return MCDisassembler::Success;
}

static DecodeStatus decodeFloatInstruction(MCInst &MI, uint32_t Insn,
                                           uint64_t Address,
                                           const void *Decoder) {
  uint32_t func = (Insn & 0x1FE0) >> 5;
  switch ((func & 0xF0) >> 4) {
  default:
    return MCDisassembler::Fail;
  case 0x3:
    return decodeFloatCopyInstruction(func, MI, Insn, Address, Decoder);
  }
}

static DecodeStatus decodeFloatSelectInstruction(MCInst &MI, uint32_t Insn,
                                                 uint64_t Address,
                                                 const void *Decoder) {
  uint32_t func = (Insn & 0xFC00) >> 10;
  switch (func) {
  default:
    return MCDisassembler::Fail;
  case 0x10:
    MI.setOpcode(Sw64::FSELEQS);
    break;
  case 0x11:
    MI.setOpcode(Sw64::FSELNES);
    break;
  case 0x12:
    MI.setOpcode(Sw64::FSELLTS);
    break;
  case 0x13:
    MI.setOpcode(Sw64::FSELLES);
    break;
  case 0x14:
    MI.setOpcode(Sw64::FSELGTS);
    break;
  case 0x15:
    MI.setOpcode(Sw64::FSELGES);
    break;
  }
  uint32_t RegOp1 = Insn << 6 >> 27;     // Inst {25-21} Reg operand 1
  uint32_t RegOp2 = Insn << 11 >> 27;    // Inst [20-16] Reg operand 2
  uint32_t RegOp3 = (Insn & 0x3E0) >> 5; // Inst [4-0 ] Reg operand 3
  uint32_t RegOp4 = Insn & 0x1F;         // Inst [4-0 ] Reg operand 3
  MI.addOperand(MCOperand::createReg(FPRDecoderTable[RegOp4]));
  MI.addOperand(MCOperand::createReg(FPRDecoderTable[RegOp3]));
  MI.addOperand(MCOperand::createReg(FPRDecoderTable[RegOp2]));
  MI.addOperand(MCOperand::createReg(FPRDecoderTable[RegOp1]));
  return MCDisassembler::Success;
}

static DecodeStatus decodePostLSInstruction(MCInst &MI, uint32_t Insn,
                                            uint64_t Address,
                                            const void *Decoder) {
  uint32_t func = (Insn & 0xFC00) >> 12;
  bool isFloat = false;
  bool isStore = false;
  switch (func) {
  default:
    return MCDisassembler::Fail;
  case 0x0:
    MI.setOpcode(Sw64::LDBU_A);
    break;
  case 0x1:
    MI.setOpcode(Sw64::LDHU_A);
    break;
  case 0x2:
    MI.setOpcode(Sw64::LDW_A);
    break;
  case 0x3:
    MI.setOpcode(Sw64::LDL_A);
    break;
  case 0x4:
    MI.setOpcode(Sw64::LDS_A);
    isFloat = true;
    break;
  case 0x5:
    MI.setOpcode(Sw64::LDD_A);
    isFloat = true;
    break;
  case 0x6:
    MI.setOpcode(Sw64::STB_A);
    break;
  case 0x7:
    MI.setOpcode(Sw64::STH_A);
    break;
  case 0x8:
    MI.setOpcode(Sw64::STW_A);
    break;
  case 0x9:
    MI.setOpcode(Sw64::STL_A);
    break;
  case 0xA:
    MI.setOpcode(Sw64::STS_A);
    isFloat = true;
    isStore = true;
    break;
  case 0xB:
    MI.setOpcode(Sw64::STD_A);
    isFloat = true;
    isStore = true;
    break;
  }
  uint32_t RegOp1 = Insn << 6 >> 27;  // Inst {25-21} Reg operand 1
  uint32_t RegOp2 = Insn << 11 >> 27; // Inst [20-16] Reg operand 2
  unsigned RegOp3 = Insn & 0xFFF;     // Inst [11-0 ] Reg operand 3
  uint32_t RegOp4 = Insn << 11 >> 27;
  MI.addOperand((isFloat && !isStore)
                    ? MCOperand::createReg(FPRDecoderTable[RegOp1])
                    : MCOperand::createReg(GPRDecoderTable[RegOp1]));
  MI.addOperand((isFloat && isStore)
                    ? MCOperand::createReg(FPRDecoderTable[RegOp4])
                    : MCOperand::createReg(GPRDecoderTable[RegOp4]));
  MI.addOperand(MCOperand::createReg(GPRDecoderTable[RegOp2]));
  MI.addOperand(MCOperand::createImm(RegOp3));
  return MCDisassembler::Success;
}

static DecodeStatus decodeBarrierInstruction(MCInst &MI, uint32_t Insn,
                                             uint64_t Address,
                                             const void *Decoder) {
  uint32_t func = Insn & 0xFFFF;
  switch (func) {
  default:
    return MCDisassembler::Fail;
  case 0x00:
    MI.setOpcode(Sw64::MB);
    break;
  case 0x01:
    MI.setOpcode(Sw64::IMEMB);
    break;
  case 0x02:
    MI.setOpcode(Sw64::WMEMB);
    break;
  }
  return MCDisassembler::Success;
}

static DecodeStatus decodeConlictInstruction(MCInst &MI, uint32_t Insn,
                                             uint64_t Address,
                                             const void *Decoder) {
  uint32_t Opcode = Insn >> 26;
  switch (Opcode) {
  default:
    return MCDisassembler::Fail;
  case 0x06:
    return decodeBarrierInstruction(MI, Insn, Address, Decoder);
  case 0x18:
    return decodeFloatInstruction(MI, Insn, Address, Decoder);
  case 0x19:
    return decodeFloatSelectInstruction(MI, Insn, Address, Decoder);
  case 0x1E:
    return decodePostLSInstruction(MI, Insn, Address, Decoder);
  }
}

#include "Sw64GenDisassemblerTables.inc"

DecodeStatus Sw64Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CStream) const {
  // TODO: This will need modification when supporting instruction set
  // extensions with instructions > 32-bits (up to 176 bits wide).
  uint32_t Insn;
  DecodeStatus Result;

  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }
  Insn = support::endian::read32le(Bytes.data());
  LLVM_DEBUG(dbgs() << "Trying Decode Conflict Instruction :\n");
  Result = decodeConlictInstruction(Instr, Insn, Address, this);
  if (Result != MCDisassembler::Fail) {
    Size = 4;
    return Result;
  }
  LLVM_DEBUG(dbgs() << "Trying Sw64 table :\n");
  Result = decodeInstruction(DecoderTable32, Instr, Insn, Address, this, STI);
  Size = 4;

  return Result;
}
