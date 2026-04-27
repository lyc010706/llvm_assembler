add_llvm_component_library(LLVMSw64Info
  Sw64TargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  Sw64

  )
