#include "Utils/AArch64SMEAttributes.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace llvm;
using SA = SMEAttrs;

std::unique_ptr<Module> parseIR(const char *IR) {
  static LLVMContext C;
  SMDiagnostic Err;
  return parseAssemblyString(IR, Err, C);
}

TEST(SMEAttributes, Constructors) {
  LLVMContext Context;

  ASSERT_TRUE(SA(*parseIR("declare void @foo()")->getFunction("foo"))
                  .hasNonStreamingInterfaceAndBody());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_sm_body\"")
                      ->getFunction("foo"))
                  .hasNonStreamingInterface());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_sm_enabled\"")
                      ->getFunction("foo"))
                  .hasStreamingInterface());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_sm_body\"")
                      ->getFunction("foo"))
                  .hasStreamingBody());

  ASSERT_TRUE(
      SA(*parseIR("declare void @foo() \"aarch64_pstate_sm_compatible\"")
              ->getFunction("foo"))
          .hasStreamingCompatibleInterface());

  ASSERT_TRUE(
      SA(*parseIR("declare void @foo() \"aarch64_in_za\"")->getFunction("foo"))
          .isInZA());
  ASSERT_TRUE(
      SA(*parseIR("declare void @foo() \"aarch64_out_za\"")->getFunction("foo"))
          .isOutZA());
  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_inout_za\"")
                      ->getFunction("foo"))
                  .isInOutZA());
  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_preserves_za\"")
                      ->getFunction("foo"))
                  .isPreservesZA());
  ASSERT_TRUE(
      SA(*parseIR("declare void @foo() \"aarch64_new_za\"")->getFunction("foo"))
          .isNewZA());

  // Invalid combinations.
  EXPECT_DEBUG_DEATH(SA(SA::SM_Enabled | SA::SM_Compatible),
                     "SM_Enabled and SM_Compatible are mutually exclusive");

  // Test that the set() methods equally check validity.
  EXPECT_DEBUG_DEATH(SA(SA::SM_Enabled).set(SA::SM_Compatible),
                     "SM_Enabled and SM_Compatible are mutually exclusive");
  EXPECT_DEBUG_DEATH(SA(SA::SM_Compatible).set(SA::SM_Enabled),
                     "SM_Enabled and SM_Compatible are mutually exclusive");
}

TEST(SMEAttributes, Basics) {
  // Test PSTATE.SM interfaces.
  ASSERT_TRUE(SA(SA::Normal).hasNonStreamingInterfaceAndBody());
  ASSERT_TRUE(SA(SA::SM_Enabled).hasStreamingInterface());
  ASSERT_TRUE(SA(SA::SM_Body).hasStreamingBody());
  ASSERT_TRUE(SA(SA::SM_Body).hasNonStreamingInterface());
  ASSERT_FALSE(SA(SA::SM_Body).hasNonStreamingInterfaceAndBody());
  ASSERT_FALSE(SA(SA::SM_Body).hasStreamingInterface());
  ASSERT_TRUE(SA(SA::SM_Compatible).hasStreamingCompatibleInterface());
  ASSERT_TRUE(
      SA(SA::SM_Compatible | SA::SM_Body).hasStreamingCompatibleInterface());
  ASSERT_TRUE(SA(SA::SM_Compatible | SA::SM_Body).hasStreamingBody());
  ASSERT_FALSE(SA(SA::SM_Compatible | SA::SM_Body).hasNonStreamingInterface());

  // Test ZA State interfaces
  SA ZA_In = SA(SA::encodeZAState(SA::StateValue::In));
  ASSERT_TRUE(ZA_In.isInZA());
  ASSERT_FALSE(ZA_In.isOutZA());
  ASSERT_FALSE(ZA_In.isInOutZA());
  ASSERT_FALSE(ZA_In.isPreservesZA());
  ASSERT_FALSE(ZA_In.isNewZA());
  ASSERT_TRUE(ZA_In.sharesZA());
  ASSERT_TRUE(ZA_In.hasZAState());
  ASSERT_TRUE(ZA_In.hasSharedZAInterface());
  ASSERT_FALSE(ZA_In.hasPrivateZAInterface());

  SA ZA_Out = SA(SA::encodeZAState(SA::StateValue::Out));
  ASSERT_TRUE(ZA_Out.isOutZA());
  ASSERT_FALSE(ZA_Out.isInZA());
  ASSERT_FALSE(ZA_Out.isInOutZA());
  ASSERT_FALSE(ZA_Out.isPreservesZA());
  ASSERT_FALSE(ZA_Out.isNewZA());
  ASSERT_TRUE(ZA_Out.sharesZA());
  ASSERT_TRUE(ZA_Out.hasZAState());
  ASSERT_TRUE(ZA_Out.hasSharedZAInterface());
  ASSERT_FALSE(ZA_Out.hasPrivateZAInterface());

  SA ZA_InOut = SA(SA::encodeZAState(SA::StateValue::InOut));
  ASSERT_TRUE(ZA_InOut.isInOutZA());
  ASSERT_FALSE(ZA_InOut.isInZA());
  ASSERT_FALSE(ZA_InOut.isOutZA());
  ASSERT_FALSE(ZA_InOut.isPreservesZA());
  ASSERT_FALSE(ZA_InOut.isNewZA());
  ASSERT_TRUE(ZA_InOut.sharesZA());
  ASSERT_TRUE(ZA_InOut.hasZAState());
  ASSERT_TRUE(ZA_InOut.hasSharedZAInterface());
  ASSERT_FALSE(ZA_InOut.hasPrivateZAInterface());

  SA ZA_Preserved = SA(SA::encodeZAState(SA::StateValue::Preserved));
  ASSERT_TRUE(ZA_Preserved.isPreservesZA());
  ASSERT_FALSE(ZA_Preserved.isInZA());
  ASSERT_FALSE(ZA_Preserved.isOutZA());
  ASSERT_FALSE(ZA_Preserved.isInOutZA());
  ASSERT_FALSE(ZA_Preserved.isNewZA());
  ASSERT_TRUE(ZA_Preserved.sharesZA());
  ASSERT_TRUE(ZA_Preserved.hasZAState());
  ASSERT_TRUE(ZA_Preserved.hasSharedZAInterface());
  ASSERT_FALSE(ZA_Preserved.hasPrivateZAInterface());

  SA ZA_New = SA(SA::encodeZAState(SA::StateValue::New));
  ASSERT_TRUE(ZA_New.isNewZA());
  ASSERT_FALSE(ZA_New.isInZA());
  ASSERT_FALSE(ZA_New.isOutZA());
  ASSERT_FALSE(ZA_New.isInOutZA());
  ASSERT_FALSE(ZA_New.isPreservesZA());
  ASSERT_FALSE(ZA_New.sharesZA());
  ASSERT_TRUE(ZA_New.hasZAState());
  ASSERT_FALSE(ZA_New.hasSharedZAInterface());
  ASSERT_TRUE(ZA_New.hasPrivateZAInterface());

  ASSERT_FALSE(SA(SA::Normal).isInZA());
  ASSERT_FALSE(SA(SA::Normal).isOutZA());
  ASSERT_FALSE(SA(SA::Normal).isInOutZA());
  ASSERT_FALSE(SA(SA::Normal).isPreservesZA());
  ASSERT_FALSE(SA(SA::Normal).isNewZA());
  ASSERT_FALSE(SA(SA::Normal).sharesZA());
  ASSERT_FALSE(SA(SA::Normal).hasZAState());
}

TEST(SMEAttributes, Transitions) {
  // Normal -> Normal
  ASSERT_FALSE(SA(SA::Normal).requiresSMChange(SA(SA::Normal)));
  // Normal -> Normal + LocallyStreaming
  ASSERT_FALSE(SA(SA::Normal).requiresSMChange(SA(SA::Normal | SA::SM_Body)));

  // Normal -> Streaming
  ASSERT_TRUE(SA(SA::Normal).requiresSMChange(SA(SA::SM_Enabled)));
  // Normal -> Streaming + LocallyStreaming
  ASSERT_TRUE(
      SA(SA::Normal).requiresSMChange(SA(SA::SM_Enabled | SA::SM_Body)));

  // Normal -> Streaming-compatible
  ASSERT_FALSE(SA(SA::Normal).requiresSMChange(SA(SA::SM_Compatible)));
  // Normal -> Streaming-compatible + LocallyStreaming
  ASSERT_FALSE(
      SA(SA::Normal).requiresSMChange(SA(SA::SM_Compatible | SA::SM_Body)));

  // Streaming -> Normal
  ASSERT_TRUE(SA(SA::SM_Enabled).requiresSMChange(SA(SA::Normal)));
  // Streaming -> Normal + LocallyStreaming
  ASSERT_TRUE(
      SA(SA::SM_Enabled).requiresSMChange(SA(SA::Normal | SA::SM_Body)));

  // Streaming -> Streaming
  ASSERT_FALSE(SA(SA::SM_Enabled).requiresSMChange(SA(SA::SM_Enabled)));
  // Streaming -> Streaming + LocallyStreaming
  ASSERT_FALSE(
      SA(SA::SM_Enabled).requiresSMChange(SA(SA::SM_Enabled | SA::SM_Body)));

  // Streaming -> Streaming-compatible
  ASSERT_FALSE(SA(SA::SM_Enabled).requiresSMChange(SA(SA::SM_Compatible)));
  // Streaming -> Streaming-compatible + LocallyStreaming
  ASSERT_FALSE(
      SA(SA::SM_Enabled).requiresSMChange(SA(SA::SM_Compatible | SA::SM_Body)));

  // Streaming-compatible -> Normal
  ASSERT_TRUE(SA(SA::SM_Compatible).requiresSMChange(SA(SA::Normal)));
  ASSERT_TRUE(
      SA(SA::SM_Compatible).requiresSMChange(SA(SA::Normal | SA::SM_Body)));

  // Streaming-compatible -> Streaming
  ASSERT_TRUE(SA(SA::SM_Compatible).requiresSMChange(SA(SA::SM_Enabled)));
  // Streaming-compatible -> Streaming + LocallyStreaming
  ASSERT_TRUE(
      SA(SA::SM_Compatible).requiresSMChange(SA(SA::SM_Enabled | SA::SM_Body)));

  // Streaming-compatible -> Streaming-compatible
  ASSERT_FALSE(SA(SA::SM_Compatible).requiresSMChange(SA(SA::SM_Compatible)));
  // Streaming-compatible -> Streaming-compatible + LocallyStreaming
  ASSERT_FALSE(SA(SA::SM_Compatible)
                   .requiresSMChange(SA(SA::SM_Compatible | SA::SM_Body)));
}
