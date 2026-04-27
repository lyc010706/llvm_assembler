// RUN: %clang -target aarch64_be -mcpu=hip10c -### -c %s 2>&1 | FileCheck -check-prefix=hip10c-BE %s
// RUN: %clang -target aarch64 -mbig-endian -mcpu=hip10c -### -c %s 2>&1 | FileCheck -check-prefix=hip10c-BE %s
// RUN: %clang -target aarch64_be -mbig-endian -mcpu=hip10c -### -c %s 2>&1 | FileCheck -check-prefix=hip10c-BE %s
// RUN: %clang -target aarch64_be -mtune=hip10c -### -c %s 2>&1 | FileCheck -check-prefix=hip10c-BE-TUNE %s
// RUN: %clang -target aarch64 -mbig-endian -mtune=hip10c -### -c %s 2>&1 | FileCheck -check-prefix=hip10c-BE-TUNE %s
// RUN: %clang -target aarch64_be -mbig-endian -mtune=hip10c -### -c %s 2>&1 | FileCheck -check-prefix=hip10c-BE-TUNE %s
// hip10c-BE: "-cc1"{{.*}} "-triple" "aarch64_be{{.*}}" "-target-cpu" "hip10c"
// hip10c-BE-TUNE: "-cc1"{{.*}} "-triple" "aarch64_be{{.*}}" "-target-cpu" "generic"
