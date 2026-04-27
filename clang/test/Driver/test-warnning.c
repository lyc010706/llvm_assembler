// REQUIRES: build_for_openeuler

// RUN: %clang -v -fgcc-compatible -Wno-format-security -Werror=format=2 -Wall %s
// RUN: %clang -v -Wall %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// CHECK-ERROR: warning: format string is not a string literal (potentially insecure)
// RUN: %clang -v -Wno-format-security -Werror=format=2 -Wall %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// CHECK-ERROR: error: format string is not a string literal (potentially insecure)

#include <stdio.h>

int main() {
    char *str = "llvm-project";
    printf(str);
    return 0;
}