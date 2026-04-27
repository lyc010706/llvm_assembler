// REQUIRES: build_for_openeuler
// RUN: clang %s -shared -fno-plt  -O2 -fno-inline  -fPIC   -o noplt.so
// RUN: llvm-objdump -d noplt.so | FileCheck %s --check-prefix=CHECK-NO-PLT

// RUN: clang %s -shared  -O2 -fno-inline  -fPIC   -o plt.so
// RUN: llvm-objdump -d plt.so | FileCheck %s --check-prefix=CHECK-PLT

// CHECK-PLT: bar@plt
// CHECK-PLT: bar1@plt
// CHECK-NO-PLT-NOT: bar@plt
// CHECK-NO-PLT-NOT: bar1@plt
// CHECK-NO-PLT-NOT: bar2@plt

__attribute__((optnone))
void bar(int a) {
    return;
}

__attribute__((optnone))
extern void bar1(int);

__attribute__((optnone))
static void bar2(int a) {
    return;
}

void foo(int a) {
    bar(a);
    bar1(a);
    bar2(a);
    return;
}

