// RUN: %clang -target aarch64_be -mcpu=hip11 -### -c %s 2>&1 | FileCheck -check-prefix=HIP11-BE %s
// RUN: %clang -target aarch64 -mbig-endian -mcpu=hip11 -### -c %s 2>&1 | FileCheck -check-prefix=HIP11-BE %s
// RUN: %clang -target aarch64_be -mbig-endian -mcpu=hip11 -### -c %s 2>&1 | FileCheck -check-prefix=HIP11-BE %s
// RUN: %clang -target aarch64_be -mtune=hip11 -### -c %s 2>&1 | FileCheck -check-prefix=HIP11-BE-TUNE %s
// RUN: %clang -target aarch64 -mbig-endian -mtune=hip11 -### -c %s 2>&1 | FileCheck -check-prefix=HIP11-BE-TUNE %s
// RUN: %clang -target aarch64_be -mbig-endian -mtune=hip11 -### -c %s 2>&1 | FileCheck -check-prefix=HIP11-BE-TUNE %s
// HIP11-BE: "-cc1"{{.*}} "-triple" "aarch64_be{{.*}}" "-target-cpu" "hip11"
// HIP11-BE-TUNE: "-cc1"{{.*}} "-triple" "aarch64_be{{.*}}" "-target-cpu" "generic"