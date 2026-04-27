// RUN: %clang --target=x86_64 -### -c -mcmodel=tiny %s 2>&1 | FileCheck --check-prefix=TINY %s
// RUN: %clang --target=x86_64 -### -c -mcmodel=small %s 2>&1 | FileCheck --check-prefix=SMALL %s
// RUN: %clang --target=x86_64 -### -S -mcmodel=kernel %s 2>&1 | FileCheck --check-prefix=KERNEL %s
// RUN: %clang --target=x86_64 -### -c -mcmodel=medium %s 2>&1 | FileCheck --check-prefix=MEDIUM %s
// RUN: %clang --target=x86_64 -### -S -mcmodel=large %s 2>&1 | FileCheck --check-prefix=LARGE %s
// RUN: %clang --target=powerpc-unknown-aix -### -S -mcmodel=medium %s 2> %t.log
// RUN: FileCheck --check-prefix=AIX-MCMEDIUM-OVERRIDE %s < %t.log
// RUN: not %clang -c -mcmodel=lager %s 2>&1 | FileCheck --check-prefix=INVALID %s
// RUN: not %clang -c --target=aarch64 -mcmodel=medium %s 2>&1 | FileCheck --check-prefix=AARCH64-MEDIUM %s
// RUN: not %clang -c --target=aarch64 -mcmodel=kernel %s 2>&1 | FileCheck --check-prefix=AARCH64-KERNEL %s
// RUN: %clang --target=loongarch64 -### -S -mcmodel=normal %s 2>&1 | FileCheck --check-prefix=SMALL %s
// RUN: %clang --target=loongarch64 -### -S -mcmodel=medium %s 2>&1 | FileCheck --check-prefix=MEDIUM %s
// RUN: %clang --target=loongarch64 -### -S -mcmodel=extreme %s 2>&1 | FileCheck --check-prefix=LARGE %s
// RUN: not %clang -c --target=loongarch64 -mcmodel=tiny %s 2>&1 | FileCheck --check-prefix=ERR-LOONGARCH64-TINY %s
// RUN: not %clang -c --target=loongarch64 -mcmodel=small %s 2>&1 | FileCheck --check-prefix=ERR-LOONGARCH64-SMALL %s
// RUN: not %clang -c --target=loongarch64 -mcmodel=kernel %s 2>&1 | FileCheck --check-prefix=ERR-LOONGARCH64-KERNEL %s
// RUN: not %clang -c --target=loongarch64 -mcmodel=large %s 2>&1 | FileCheck --check-prefix=ERR-LOONGARCH64-LARGE %s
// RUN: not %clang -c --target=loongarch64 -mcmodel=extreme -fplt %s 2>&1 | FileCheck --check-prefix=ERR-LOONGARCH64-PLT-EXTREME %s

// TINY: "-mcmodel=tiny"
// SMALL: "-mcmodel=small"
// KERNEL: "-mcmodel=kernel"
// MEDIUM: "-mcmodel=medium"
// LARGE: "-mcmodel=large"
// AIX-MCMEDIUM-OVERRIDE: "-mcmodel=large"

// INVALID: error: invalid argument 'lager' to -mcmodel=

// AARCH64-MEDIUM: error: invalid argument 'medium' to -mcmodel=
// AARCH64-KERNEL: error: invalid argument 'kernel' to -mcmodel=

// ERR-LOONGARCH64-TINY:   error: invalid argument 'tiny' to -mcmodel=
// ERR-LOONGARCH64-SMALL:  error: invalid argument 'small' to -mcmodel=
// ERR-LOONGARCH64-KERNEL: error: invalid argument 'kernel' to -mcmodel=
// ERR-LOONGARCH64-LARGE:  error: invalid argument 'large' to -mcmodel=

// ERR-LOONGARCH64-PLT-EXTREME: error: invalid argument '-mcmodel=extreme' not allowed with '-fplt'
