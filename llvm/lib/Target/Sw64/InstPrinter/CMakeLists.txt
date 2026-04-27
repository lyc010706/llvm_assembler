add_llvm_component_library(LLVMSw64AsmPrinter
  Sw64InstPrinter.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  Sw64
  )
