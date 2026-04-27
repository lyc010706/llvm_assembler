// REQUIRES: build_for_openeuler
// RUN: %clang -### -fgcc-compatible -Wa,--generate-missing-build-notes=yes %s 2>&1 | FileCheck -check-prefix=CHECK-NO-ERROR %s
// RUN: %clang -### -fgcc-compatible -Wa,--generate-missing-build-notes=no %s 2>&1 | FileCheck -check-prefix=CHECK-NO-ERROR %s
// RUN: %clang -### -fGNU-compatibility -Wa,--generate-missing-build-notes=no %s 2>&1 | FileCheck -check-prefix=CHECK-NO-ERROR %s
// CHECK-NO-ERROR-NOT: --generate-missing-build-notes=
// RUN: %clang -### -Wa,--generate-missing-build-notes=yes %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN: %clang -### -Wa,--generate-missing-build-notes=no %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN: %clang -### -fno-gcc-compatible -Wa,--generate-missing-build-notes=yes %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN: %clang -### -fno-gcc-compatible -Wa,--generate-missing-build-notes=no %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// CHECK-ERROR: error: unsupported argument '--generate-missing-build-notes=

int main() {
    return 1;
}
