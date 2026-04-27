// REQUIRES: aarch64-registered-target
// RUN: %clang_cc1 -triple aarch64-none-linux-gnu -target-feature +sme -S -O1 -Werror -emit-llvm -o - %s | FileCheck %s -check-prefixes=CHECK,CHECK-C
// RUN: %clang_cc1 -triple aarch64-none-linux-gnu -target-feature +sme -S -O1 -Werror -emit-llvm -o - -x c++ %s | FileCheck %s -check-prefixes=CHECK,CHECK-CXX
// RUN: %clang_cc1 -triple aarch64-none-linux-gnu -target-feature +sme -S -O1 -Werror -o /dev/null %s

#include <arm_sme.h>

// CHECK-C-LABEL: @test_svzero_mask_za(
// CHECK-CXX-LABEL: @_Z19test_svzero_mask_zav(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    tail call void @llvm.aarch64.sme.zero(i32 0)
// CHECK-NEXT:    ret void
//
void test_svzero_mask_za(void) __arm_inout("za") {
  svzero_mask_za(0);
}

// CHECK-C-LABEL: @test_svzero_mask_za_1(
// CHECK-CXX-LABEL: @_Z21test_svzero_mask_za_1v(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    tail call void @llvm.aarch64.sme.zero(i32 176)
// CHECK-NEXT:    ret void
//
void test_svzero_mask_za_1(void) __arm_inout("za") {
  svzero_mask_za(176);
}

// CHECK-C-LABEL: @test_svzero_mask_za_2(
// CHECK-CXX-LABEL: @_Z21test_svzero_mask_za_2v(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    tail call void @llvm.aarch64.sme.zero(i32 255)
// CHECK-NEXT:    ret void
//
void test_svzero_mask_za_2(void) __arm_inout("za") {
  svzero_mask_za(255);
}

// CHECK-C-LABEL: define dso_local void @test_svzero_za(
// CHECK-C-SAME: ) local_unnamed_addr #[[ATTR2:[0-9]+]] {
// CHECK-C-NEXT:  entry:
// CHECK-C-NEXT:    tail call void @llvm.aarch64.sme.zero(i32 255)
// CHECK-C-NEXT:    ret void
//
// CHECK-CXX-LABEL: define dso_local void @_Z14test_svzero_zav(
// CHECK-CXX-SAME: ) local_unnamed_addr #[[ATTR2:[0-9]+]] {
// CHECK-CXX-NEXT:  entry:
// CHECK-CXX-NEXT:    tail call void @llvm.aarch64.sme.zero(i32 255)
// CHECK-CXX-NEXT:    ret void
//
void test_svzero_za(void) __arm_out("za") {
  svzero_za();
}
