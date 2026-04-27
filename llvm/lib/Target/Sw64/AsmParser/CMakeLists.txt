add_llvm_component_library(LLVMSw64AsmParser
  Sw64AsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  Sw64Desc
  Sw64Info
  Support

  ADD_TO_COMPONENT
  Sw64
  )
