// REQUIRES: build_for_openeuler

// RUN: %clang  %s
// RUN: %clang  -iprefix $(dirname `which clang`)/../lib/clang/*/   -iwithprefix  include %s
// RUN: %clang  -iwithprefix include %s
// RUN: %clang  -nostdinc -iwithprefix include %s -fgcc-compatible
// RUN: %clang  -nostdinc -iprefix $(dirname `which clang`)/../lib/clang/*/ -iwithprefix  include %s -fgcc-compatible
// RUN: not %clang -nostdinc   -iwithprefix  include %s
// RUN: not %clang -nostdinc  -iprefix ""  -iwithprefix  include %s

#include <stdarg.h>
int main(void) {
return 0;
}
