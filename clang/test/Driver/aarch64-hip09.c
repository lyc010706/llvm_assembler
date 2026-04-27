// RUN: %clang -target aarch64_be -mcpu=hip09 -### -c %s 2>&1 | FileCheck -check-prefix=hip09-BE %s
// RUN: %clang -target aarch64 -mbig-endian -mcpu=hip09 -### -c %s 2>&1 | FileCheck -check-prefix=hip09-BE %s
// RUN: %clang -target aarch64_be -mbig-endian -mcpu=hip09 -### -c %s 2>&1 | FileCheck -check-prefix=hip09-BE %s
// RUN: %clang -target aarch64_be -mtune=hip09 -### -c %s 2>&1 | FileCheck -check-prefix=hip09-BE-TUNE %s
// RUN: %clang -target aarch64 -mbig-endian -mtune=hip09 -### -c %s 2>&1 | FileCheck -check-prefix=hip09-BE-TUNE %s
// RUN: %clang -target aarch64_be -mbig-endian -mtune=hip09 -### -c %s 2>&1 | FileCheck -check-prefix=hip09-BE-TUNE %s
// hip09-BE: "-cc1"{{.*}} "-triple" "aarch64_be{{.*}}" "-target-cpu" "hip09"
// hip09-BE-TUNE: "-cc1"{{.*}} "-triple" "aarch64_be{{.*}}" "-target-cpu" "generic"
