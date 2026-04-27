add_llvm_component_library(LLVMSw64Disassembler
  Sw64Disassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  Sw64Info
  Support

  ADD_TO_COMPONENT
  Sw64
  )
