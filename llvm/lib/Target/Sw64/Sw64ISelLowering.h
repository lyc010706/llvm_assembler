//===-- Sw64ISelLowering.h - Sw64 DAG Lowering Interface ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Sw64 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SW64_SW64ISELLOWERING_H
#define LLVM_LIB_TARGET_SW64_SW64ISELLOWERING_H

#include "Sw64.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

// Forward delcarations
class Sw64Subtarget;
class Sw64TargetMachine;

namespace Sw64ISD {
enum NodeType : unsigned {
  // Start the numbering where the builtin ops and target ops leave off.
  FIRST_NUMBER = ISD::BUILTIN_OP_END,

  // These corrospond to the identical Instruction
  CVTQT_,
  CVTQS_,
  CVTTQ_,
  CVTTS_,
  CVTST_,
  /// GPRelHi/GPRelLo - These represent the high and low 16-bit
  /// parts of a global address respectively.
  GPRelHi,
  GPRelLo,
  /// TPRelHi/TPRelLo - These represent the high and low 16-bit
  /// parts of a TLS global address respectively.
  TPRelHi,
  TPRelLo,
  TLSGD,  // SW
  TLSLDM, // SW
  DTPRelHi,
  DTPRelLo,
  RelGottp, // SW
  SysCall,
  /// RetLit - Literal Relocation of a Global
  RelLit,

  /// GlobalRetAddr - used to restore the return address
  GlobalRetAddr,

  /// CALL - Normal call.
  CALL,

  ///  Jump and link (call)
  JmpLink,
  /// DIVCALL - used for special library calls for div and rem
  DivCall,
  /// return flag operand
  RET_FLAG,
  Ret,
  LDAWC,
  MEMBARRIER,
  /// CHAIN = COND_BRANCH CHAIN, OPC, (G|F)PRC, DESTBB [, INFLAG] - This
  /// corresponds to the COND_BRANCH pseudo instruction.
  /// *PRC is the input register to compare to zero,
  /// OPC is the branch opcode to use (e.g. Sw64::BEQ),
  /// DESTBB is the destination block to branch to, and INFLAG is
  /// an optional input flag argument.
  COND_BRANCH_I,
  COND_BRANCH_F,

  Z_S_FILLCS,
  Z_S_FILLDE,
  Z_FILLDE,
  Z_FILLDE_E,
  Z_FILLCS,
  Z_FILLCS_E,
  Z_E_FILLCS,
  Z_E_FILLDE,
  Z_FLUSHD,

  GPRel,
  TPRel,
  DTPRel,
  LDIH,
  LDI,

  FRECS,
  FRECD,
  ADDPI,
  ADDPIS,
  SBT,
  CBT,
  REVBH,
  REVBW,

  ROLW,
  CRC32B,
  CRC32H,
  CRC32W,
  CRC32L,
  CRC32CB,
  CRC32CH,
  CRC32CW,
  CRC32CL,

  VBROADCAST_LD,
  VBROADCAST,

  // Vector load.
  VLDWE,
  VLDSE,
  VLDDE,

  // Vector comparisons.
  // These take a vector and return a boolean.
  VALL_ZERO,
  VANY_ZERO,
  VALL_NONZERO,
  VANY_NONZERO,

  // This is vcmpgew.
  VSETGE,

  // These take a vector and return a vector bitmask.
  VCEQ,
  VCLE_S,
  VCLE_U,
  VCLT_S,
  VCLT_U,
  // These is vector select.
  VFCMOVEQ,
  VFCMOVLE,
  VFCMOVLT,
  VSELEQW,
  VSELLTW,
  VSELLEW,
  VSELLBCW,

  VMAX,
  VMIN,
  VUMAX,
  VUMIN,
  VSQRT,
  VSUMF,
  VFREC,
  VFCMPEQ,
  VFCMPLE,
  VFCMPLT,
  VFCMPUN,
  VFCVTSD,
  VFCVTDS,
  VFCVTLS,
  VFCVTLD,
  VFCVTSH,
  VFCVTHS,
  VFCVTDL,
  VFCVTDLG,
  VFCVTDLP,
  VFCVTDLZ,
  VFCVTDLN,
  VFRIS,
  VFRISG,
  VFRISP,
  VFRISZ,
  VFRISN,
  VFRID,
  VFRIDG,
  VFRIDP,
  VFRIDZ,
  VFRIDN,
  VMAXF,
  VMINF,
  VINSECTL,
  VCPYB,
  VCPYH,
  // Vector Shuffle with mask as an operand
  VSHF,  // Generic shuffle
  SHF,   // 4-element set shuffle.
  ILVEV, // Interleave even elements
  ILVOD, // Interleave odd elements
  ILVL,  // Interleave left elements
  ILVR,  // Interleave right elements
  PCKEV, // Pack even elements
  PCKOD, // Pack odd elements
  VCON_W,
  VCON_S,
  VCON_D,

  VSHL_BY_SCALAR,
  VSRL_BY_SCALAR,
  VSRA_BY_SCALAR,
  // Vector Lane Copy
  INSVE, // Copy element from one vector to another

  // Combined (XOR (OR $a, $b), -1)
  VNOR,
  VEQV,
  VORNOT,

  VCTPOP,
  VCTLZ,

  VLOG,
  VCOPYF,
  V8SLL,
  V8SLLi,
  V8SRL,
  V8SRLi,
  VROTR,
  VROTRi,
  V8SRA,
  V8SRAi,
  VROLB,
  VROLBi,
  VROLH,
  VROLHi,
  VROLL,
  VROLLi,
  VECREDUCE_FADD,
  VECT_VUCADDW,
  VECT_VUCADDH,
  VECT_VUCADDB,
  VECT_VUCSUBW,
  VECT_VUCSUBH,
  VECT_VUCSUBB,
  // Extended vector element extraction
  VEXTRACT_SEXT_ELT,
  VEXTRACT_ZEXT_ELT,

  VTRUNCST = ISD::FIRST_TARGET_MEMORY_OPCODE
};
} // namespace Sw64ISD

//===--------------------------------------------------------------------===//
// TargetLowering Implementation
//===--------------------------------------------------------------------===//
class Sw64TargetLowering : public TargetLowering {
  const TargetMachine &TM;
  const Sw64Subtarget &Subtarget;

public:
  explicit Sw64TargetLowering(const TargetMachine &TM,
                              const Sw64Subtarget &Subtarget);

  MVT getScalarShiftAmountTy(const DataLayout &DL, EVT LHSTy) const override;

  virtual MVT getShiftAmountTy(EVT LHSTy) const { return MVT::i64; };

  bool generateFMAsInMachineCombiner(EVT VT,
                                     CodeGenOpt::Level OptLevel) const override;

  /// getSetCCResultType - Get the SETCC result ValueType
  virtual EVT getSetCCResultType(const DataLayout &, LLVMContext &,
                                 EVT VT) const override;
  bool isLegalICmpImmediate(int64_t Imm) const override;
  bool isLegalAddImmediate(int64_t Imm) const override;
  bool isZExtFree(SDValue Val, EVT VT2) const override;
  bool isSExtCheaperThanZExt(EVT SrcVT, EVT DstVT) const override;

  /// LowerOperation - Provide custom lowering hooks for some operations.
  virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  /// ReplaceNodeResults - Replace the results of node with an illegal result
  /// type with new values built out of custom code.
  virtual void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue> &Results,
                                  SelectionDAG &DAG) const override;

  /// getTargetNodeName - This method returns the name of a target specific
  /// DAG node.
  const char *getTargetNodeName(unsigned Opcode) const override;
  template <class NodeTy> SDValue getAddr(NodeTy *N, SelectionDAG &DAG) const;
  SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                          CallingConv::ID CallConv, bool isVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc &dl,
                          SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals,
                          const SDNode *CallNode, const Type *RetTy) const;
  ConstraintType getConstraintType(const std::string &Constraint) const;

  unsigned MatchRegName(StringRef Name) const;
  Register getRegisterByName(const char *RegName, LLT VT,
                             const MachineFunction &MF) const override;
  /// Examine constraint string and operand type and determine a weight value.
  /// The operand object must already have been set up with the operand type.
  ConstraintWeight
  getSingleConstraintMatchWeight(AsmOperandInfo &info,
                                 const char *constraint) const override;

  // Inline asm support
  std::pair<unsigned, const TargetRegisterClass *>
  getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                               StringRef Constraint, MVT VT) const override;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *BB) const override;

  virtual bool
  isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const override;

  EVT getOptimalMemOpType(
      const MemOp &Op, const AttributeList & /*FuncAttributes*/) const override;

  /// isFPImmLegal - Returns true if the target can instruction select the
  /// specified FP immediate natively. If false, the legalizer will
  /// materialize the FP immediate as a load from a constant pool.
  virtual bool isFPImmLegal(const APFloat &Imm, EVT VT) const;
  struct LTStr {
    bool operator()(const char *S1, const char *S2) const {
      return strcmp(S1, S2) < 0;
    }
  };
  /// If a physical register, this returns the register that receives the
  /// exception address on entry to an EH pad.
  Register
  getExceptionPointerRegister(const Constant *PersonalityFn) const override {
    return Sw64::R16;
  }

  /// If a physical register, this returns the register that receives the
  /// exception typeid on entry to a landing pad.
  Register
  getExceptionSelectorRegister(const Constant *PersonalityFn) const override {
    return Sw64::R17;
  }
  SDValue PerformDAGCombineV(SDNode *N, DAGCombinerInfo &DCI) const;
  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

  /// Enable SIMD support for the given integer type and Register
  /// class.
  void addSIMDIntType(MVT::SimpleValueType Ty, const TargetRegisterClass *RC);

  /// Enable SIMD support for the given floating-point type and
  /// Register class.
  void addSIMDFloatType(MVT::SimpleValueType Ty, const TargetRegisterClass *RC);

private:
  // Helpers for custom lowering.
  void LowerVAARG(SDNode *N, SDValue &Chain, SDValue &DataPtr,
                  SelectionDAG &DAG) const;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  virtual SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                            SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &ArgsFlags,
                      LLVMContext &Context) const override;

  // Lower Operand specifics
  SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerINTRINSIC_WO_CHAIN(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSRL_PARTS(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSRA_PARTS(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSHL_PARTS(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSINT_TO_FP(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFP_TO_SINT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFP_TO_INT_SAT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSUREM(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSUDIV(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVAARG(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVACOPY(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerOR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSUDIVI128(SDValue Op, SelectionDAG &DAG) const;
  std::pair<SDValue, SDValue> LowerCallExtraResult(SDValue &Chain,
                                                   SDValue &DemoteStackSlot,
                                                   unsigned DemoteStackIdx,
                                                   SelectionDAG &DAG) const;
  SDValue LowerROLW(SDNode *N, SelectionDAG &DAG) const;

  SDValue LowerVectorShift(SDValue Op, SelectionDAG &DAG) const;

  ISD::NodeType getExtendForAtomicOps() const override {
    return ISD::ANY_EXTEND;
  }

  SDValue LowerPREFETCH(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerATOMIC_FENCE(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerATOMIC_LOAD(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerATOMIC_STORE(SDValue Op, SelectionDAG &DAG) const;

  MachineMemOperand::Flags
  getTargetMMOFlags(const Instruction &I) const override;

  bool shouldInsertFencesForAtomic(const Instruction *I) const override {
    return true;
  }
  Instruction *emitLeadingFence(IRBuilderBase &Builder, Instruction *Inst,
                                AtomicOrdering Ord) const override;
  Instruction *emitTrailingFence(IRBuilderBase &Builder, Instruction *Inst,
                                 AtomicOrdering Ord) const override;
  /// This function parses registers that appear in inline-asm constraints.
  /// It returns pair (0, 0) on failure.

  SDValue LowerSTORE(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerINTRINSIC_W_CHAIN(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerINTRINSIC_VOID(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerEXTRACT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerINSERT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBUILD_VECTOR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVECTOR_SHUFFLE(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerShift(SDValue Op, SelectionDAG &DAG, unsigned ByScalar) const;

  MachineBasicBlock *emitReduceSum(MachineInstr &MI,
                                   MachineBasicBlock *BB) const;
  MachineBasicBlock *emitITOFSInstruct(MachineInstr &MI,
                                       MachineBasicBlock *BB) const;
  MachineBasicBlock *emitFSTOIInstruct(MachineInstr &MI,
                                       MachineBasicBlock *BB) const;
  SDValue LowerVectorMemIntr(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) const;
  /// Return true if an FMA operation is faster than a pair of fmul and fadd
  /// instructions. fmuladd intrinsics will be expanded to FMAs when this method
  /// returns true, otherwise fmuladd is expanded to fmul + fadd.
  bool isFMAFasterThanFMulAndFAdd(const MachineFunction &MF,
                                  EVT VT) const override;
  bool isFMAFasterThanFMulAndFAdd(const Function &F, Type *Ty) const override;

  std::pair<unsigned, const TargetRegisterClass *>
  parseRegForInlineAsmConstraint(StringRef C, MVT VT) const;

  MachineBasicBlock *emitAtomicBinary(MachineInstr &MI,
                                      MachineBasicBlock *BB) const;
  MachineBasicBlock *emitAtomicCmpSwap(MachineInstr &MI, MachineBasicBlock *BB,
                                       unsigned Size) const;
  MachineBasicBlock *emitAtomicBinaryPartword(MachineInstr &MI,
                                              MachineBasicBlock *BB,
                                              unsigned Size) const;
  MachineBasicBlock *emitAtomicCmpSwapPartword(MachineInstr &MI,
                                               MachineBasicBlock *BB,
                                               unsigned Size) const;
  MachineBasicBlock *emitPrefetch(MachineInstr &MI,
                                  MachineBasicBlock *BB) const;

  SDValue getRecipEstimate(SDValue Operand, SelectionDAG &DAG, int Enabled,
                           int &RefinementSteps) const override;
  bool getPostIndexedAddressParts(SDNode *N, SDNode *Op, SDValue &Base,
                                  SDValue &Offset, ISD::MemIndexedMode &AM,
                                  SelectionDAG &DAG) const override;
  const TargetRegisterClass *getRepRegClassFor(MVT VT) const override;

  SDValue LowerFDIV(SDValue Op, SelectionDAG &DAG) const;
  bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM, Type *Ty,
                             unsigned AS,
                             Instruction *I = nullptr) const override;
};
} // namespace llvm
#endif
