// REQUIRES: build_for_openeuler
// RUN: not %clang -fgcc-compatible -mabi=lp64 -target aarch64 %s 2>&1 | FileCheck --check-prefix=CHECK-MABI-WARN %s
// CHECK-MABI-WARN: warning: 'lp64' is the default abi for this target.
// RUN: not %clang -mabi=lp64 -target aarch64 %s 2>&1 | FileCheck --check-prefix=CHECK-MABI-ERROR %s
// CHECK-MABI-ERROR: error: unknown target ABI 'lp64'
