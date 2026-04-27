// REQUIRES: build_for_openeuler
// RUN: %clang -### -znow 2>&1 | FileCheck -check-prefix=CHECK-LINKER %s
// CHECK-LINKER: "-z" "now"

// RUN: %clang -### -Wl,-z now 2>&1 | FileCheck -check-prefix=CHECK-WLCOMMAZ %s
// CHECK-WLCOMMAZ: "-z" "now"
// RUN: %clang -### -Wl,-z -Wl,now 2>&1 | FileCheck \
// RUN: -check-prefix=CHECK-WLCOMMAZ1 %s
// CHECK-WLCOMMAZ1: "-z" "now"
// RUN: %clang -### -Wl,-z -O3 now 2>&1 | FileCheck \
// RUN: -check-prefix=CHECK-WLCOMMAZ2 %s
// CHECK-WLCOMMAZ2: "-z" "now"
// RUN: %clang -### -Wl,-z stack-size=1 2>&1 | FileCheck \
// RUN: -check-prefix=CHECK-WLCOMMAZ3 %s
// CHECK-WLCOMMAZ3: "-z" "stack-size=1"