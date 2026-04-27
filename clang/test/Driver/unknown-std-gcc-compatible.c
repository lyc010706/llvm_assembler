// REQUIRES: build_for_openeuler
// RUN: not %clang -fgcc-compatible -std=c++11 %s 2>&1 | FileCheck --check-prefix=CHECK-WARN-STD %s
// CHECK-WARN-STD: warning: '-std=c++11' is not valid for 'C'
// RUN: not %clang -std=c++11 %s 2>&1 | FileCheck --check-prefix=CHECK-ERROR-STD %s
// CHECK-ERROR-STD: error: invalid argument '-std=c++11' not allowed with 'C'
