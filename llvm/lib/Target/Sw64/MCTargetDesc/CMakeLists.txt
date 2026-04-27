add_llvm_component_library(LLVMSw64Desc
  Sw64ABIInfo.cpp
  Sw64ABIFlagsSection.cpp
  Sw64AsmBackend.cpp
  Sw64ELFObjectWriter.cpp
  Sw64ELFStreamer.cpp
  Sw64MCAsmInfo.cpp
  Sw64MCCodeEmitter.cpp
  Sw64MCExpr.cpp
  Sw64MCTargetDesc.cpp
  Sw64OptionRecord.cpp
  Sw64TargetStreamer.cpp

  LINK_COMPONENTS
  MC
  Sw64Info
  Sw64AsmPrinter
  Support

  ADD_TO_COMPONENT
  Sw64
)
