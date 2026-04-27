add_llvm_component_library(LLVMRISCVAsmParser
  RISCVAsmParser.cpp
  RISCVMCdce.cpp
  RISCVMCconstant.cpp

  LINK_COMPONENTS
  MC
  MCParser
  RISCVDesc
  RISCVInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  RISCV
  )
