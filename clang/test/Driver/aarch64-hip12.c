// RUN: %clang -target aarch64_be -mcpu=hip12 -### -c %s 2>&1 | FileCheck -check-prefix=hip12-BE %s
// RUN: %clang -target aarch64 -mbig-endian -mcpu=hip12 -### -c %s 2>&1 | FileCheck -check-prefix=hip12-BE %s
// RUN: %clang -target aarch64_be -mbig-endian -mcpu=hip12 -### -c %s 2>&1 | FileCheck -check-prefix=hip12-BE %s
// RUN: %clang -target aarch64_be -mtune=hip12 -### -c %s 2>&1 | FileCheck -check-prefix=hip12-BE-TUNE %s
// RUN: %clang -target aarch64 -mbig-endian -mtune=hip12 -### -c %s 2>&1 | FileCheck -check-prefix=hip12-BE-TUNE %s
// RUN: %clang -target aarch64_be -mbig-endian -mtune=hip12 -### -c %s 2>&1 | FileCheck -check-prefix=hip12-BE-TUNE %s
// hip12-BE: "-cc1"{{.*}} "-triple" "aarch64_be{{.*}}" "-target-cpu" "hip12"
// hip12-BE-TUNE: "-cc1"{{.*}} "-triple" "aarch64_be{{.*}}" "-target-cpu" "generic"