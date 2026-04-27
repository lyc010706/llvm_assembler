add_llvm_component_group(Sw64)

set(LLVM_TARGET_DEFINITIONS Sw64.td)

tablegen(LLVM Sw64GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM Sw64GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM Sw64GenCallingConv.inc -gen-callingconv)
tablegen(LLVM Sw64GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM Sw64GenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM Sw64GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM Sw64GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM Sw64GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM Sw64GenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM Sw64GenMCPseudoLowering.inc -gen-pseudo-lowering)

add_public_tablegen_target(Sw64CommonTableGen)

add_llvm_target(Sw64CodeGen
  Sw64AsmPrinter.cpp
  Sw64FrameLowering.cpp
  Sw64LLRP.cpp
  Sw64BranchSelector.cpp
  Sw64InstrInfo.cpp
  Sw64ISelDAGToDAG.cpp
  Sw64ISelLowering.cpp
  Sw64MCInstLower.cpp
  Sw64MachineFunctionInfo.cpp
  Sw64MacroFusion.cpp
  Sw64RegisterInfo.cpp
  Sw64Subtarget.cpp
  Sw64TargetMachine.cpp
  Sw64TargetObjectFile.cpp
  Sw64SelectionDAGInfo.cpp
  Sw64ExpandPseudo.cpp
  Sw64ExpandPseudo2.cpp
  Sw64PreLegalizerCombiner.cpp
  Sw64CombineLS.cpp
  Sw64IEEEConstraint.cpp
  Sw64TargetTransformInfo.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  MC
  SelectionDAG
  Support
  Target
  TransformUtils
  Sw64AsmPrinter
  Sw64Desc
  Sw64Info
  GlobalISel

  ADD_TO_COMPONENT
  Sw64
  )

add_subdirectory(InstPrinter)
add_subdirectory(MCTargetDesc)
add_subdirectory(Disassembler)
add_subdirectory(TargetInfo)
add_subdirectory(AsmParser)
