
#ifndef __SW64INTRIN_H
#define __SW64INTRIN_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef int8_t charv32 __attribute__((__vector_size__(32), __aligned__(32)));
typedef uint8_t ucharv32 __attribute__((__vector_size__(32), __aligned__(32)));
typedef int16_t shortv16 __attribute__((__vector_size__(32), __aligned__(32)));
typedef uint16_t ushortv16
    __attribute__((__vector_size__(32), __aligned__(32)));
typedef int32_t intv8 __attribute__((__vector_size__(32), __aligned__(32)));
typedef uint32_t uintv8 __attribute__((__vector_size__(32), __aligned__(32)));
typedef int64_t longv4 __attribute__((__vector_size__(32), __aligned__(32)));
typedef uint64_t ulongv4 __attribute__((__vector_size__(32), __aligned__(32)));

// as sw64 target float4v4 is a very special cases, we leaving this for now.
typedef float floatv4 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double doublev4 __attribute__((__vector_size__(32), __aligned__(32)));
// special case for int256
typedef long long int256 __attribute__((__vector_size__(32), __aligned__(32)));
typedef unsigned long long uint256
    __attribute__((__vector_size__(32), __aligned__(32)));

// special case for bytes compare
typedef int32_t int1v32_t;
// special case for half transform
typedef unsigned short float16v4_t
    __attribute__((__vector_size__(8), __aligned__(8)));
#define __DEFAULT_FN_ATTRS                                                     \
  __attribute__((__always_inline__, __nodebug__, __target__("simd"),           \
                 __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS_CORE4                                               \
  __attribute__((__always_inline__, __nodebug__, __target__("core4,simd"),     \
                 __min_vector_width__(256)))

static __inline void simd_fprint_charv32(FILE *fp, charv32 a) {
  union {
    char __a[32];
    charv32 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %d, %d, %d, %d, %d, %d, %d, %d \n", __u.__a[31], __u.__a[30],
          __u.__a[29], __u.__a[28], __u.__a[27], __u.__a[26], __u.__a[25],
          __u.__a[24]);
  fprintf(fp, " %d, %d, %d, %d, %d, %d, %d, %d \n", __u.__a[23], __u.__a[22],
          __u.__a[21], __u.__a[20], __u.__a[19], __u.__a[18], __u.__a[17],
          __u.__a[16]);
  fprintf(fp, " %d, %d, %d, %d, %d, %d, %d, %d \n", __u.__a[15], __u.__a[14],
          __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9],
          __u.__a[8]);
  fprintf(fp, " %d, %d, %d, %d, %d, %d, %d, %d ]\n", __u.__a[7], __u.__a[6],
          __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1],
          __u.__a[0]);
}

static __inline void simd_fprint_ucharv32(FILE *fp, ucharv32 a) {
  union {
    unsigned char __a[32];
    ucharv32 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %u, %u, %u, %u, %u, %u, %u, %u \n", __u.__a[31], __u.__a[30],
          __u.__a[29], __u.__a[28], __u.__a[27], __u.__a[26], __u.__a[25],
          __u.__a[24]);
  fprintf(fp, " %u, %u, %u, %u, %u, %u, %u, %u \n", __u.__a[23], __u.__a[22],
          __u.__a[21], __u.__a[20], __u.__a[19], __u.__a[18], __u.__a[17],
          __u.__a[16]);
  fprintf(fp, " %u, %u, %u, %u, %u, %u, %u, %u \n", __u.__a[15], __u.__a[14],
          __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9],
          __u.__a[8]);
  fprintf(fp, " %u, %u, %u, %u, %u, %u, %u, %u ]\n", __u.__a[7], __u.__a[6],
          __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1],
          __u.__a[0]);
}

static __inline void simd_fprint_shortv16(FILE *fp, shortv16 a) {
  union {
    short __a[16];
    shortv16 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %d, %d, %d, %d, %d, %d, %d, %d \n", __u.__a[15], __u.__a[14],
          __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9],
          __u.__a[8]);
  fprintf(fp, " %d, %d, %d, %d, %d, %d, %d, %d ]\n", __u.__a[7], __u.__a[6],
          __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1],
          __u.__a[0]);
}

static __inline void simd_fprint_ushortv16(FILE *fp, ushortv16 a) {
  union {
    unsigned short __a[16];
    ushortv16 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %u, %u, %u, %u, %u, %u, %u, %u \n", __u.__a[15], __u.__a[14],
          __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9],
          __u.__a[8]);
  fprintf(fp, " %u, %u, %u, %u, %u, %u, %u, %u ]\n", __u.__a[7], __u.__a[6],
          __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1],
          __u.__a[0]);
}

static __inline void simd_fprint_intv8(FILE *fp, intv8 a) {
  union {
    int __a[8];
    intv8 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %d, %d, %d, %d, %d, %d, %d, %d ]\n", __u.__a[7], __u.__a[6],
          __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1],
          __u.__a[0]);
}

static __inline void simd_fprint_uintv8(FILE *fp, uintv8 a) {
  union {
    unsigned int __a[8];
    uintv8 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %u, %u, %u, %u, %u, %u, %u, %u ]\n", __u.__a[7], __u.__a[6],
          __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1],
          __u.__a[0]);
}

static __inline void simd_fprint_longv4(FILE *fp, longv4 a) {
  union {
    long __a[4];
    longv4 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %ld, %ld, %ld, %ld ]\n", __u.__a[3], __u.__a[2], __u.__a[1],
          __u.__a[0]);
}

static __inline void simd_fprint_ulongv4(FILE *fp, ulongv4 a) {
  union {
    unsigned long __a[4];
    ulongv4 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %lu, %lu, %lu, %lu ]\n", __u.__a[3], __u.__a[2], __u.__a[1],
          __u.__a[0]);
}

static __inline void simd_fprint_floatv4(FILE *fp, floatv4 a) {
  union {
    float __a[4];
    floatv4 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %.8e, %.8e, %.8e, %.8e ]\n", __u.__a[3], __u.__a[2],
          __u.__a[1], __u.__a[0]);
}

static __inline void simd_fprint_doublev4(FILE *fp, doublev4 a) {
  union {
    double __a[4];
    doublev4 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ %.16e, %.16e, %.16e, %.16e ]\n", __u.__a[3], __u.__a[2],
          __u.__a[1], __u.__a[0]);
}

static __inline void simd_fprint_int256(FILE *fp, int256 a) {
  volatile union {
    long __a[4];
    int256 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ 0x%lx, 0x%lx, 0x%lx, 0x%lx ]\n", __u.__a[3], __u.__a[2],
          __u.__a[1], __u.__a[0]);
}

static __inline void simd_fprint_uint256(FILE *fp, uint256 a) {
  volatile union {
    unsigned long __a[4];
    uint256 __v;
  } __u;
  __u.__v = a;
  fprintf(fp, "[ 0x%lx, 0x%lx, 0x%lx, 0x%lx ]\n", __u.__a[3], __u.__a[2],
          __u.__a[1], __u.__a[0]);
}

static __inline void simd_print_charv32(charv32 arg) {
  simd_fprint_charv32(stdout, arg);
}
static __inline void simd_print_ucharv32(ucharv32 arg) {
  simd_fprint_ucharv32(stdout, arg);
}
static __inline void simd_print_shortv16(shortv16 arg) {
  simd_fprint_shortv16(stdout, arg);
}
static __inline void simd_print_ushortv16(ushortv16 arg) {
  simd_fprint_ushortv16(stdout, arg);
}
static __inline void simd_print_intv8(intv8 arg) {
  simd_fprint_intv8(stdout, arg);
}
static __inline void simd_print_uintv8(uintv8 arg) {
  simd_fprint_uintv8(stdout, arg);
}
static __inline void simd_print_longv4(longv4 arg) {
  simd_fprint_longv4(stdout, arg);
}
static __inline void simd_print_ulongv4(ulongv4 arg) {
  simd_fprint_ulongv4(stdout, arg);
}
static __inline void simd_print_floatv4(floatv4 arg) {
  simd_fprint_floatv4(stdout, arg);
}
static __inline void simd_print_doublev4(doublev4 arg) {
  simd_fprint_doublev4(stdout, arg);
}
static __inline void simd_print_int256(int256 arg) {
  simd_fprint_int256(stdout, arg);
}
static __inline void simd_print_uint256(uint256 arg) {
  simd_fprint_uint256(stdout, arg);
}

// Vector Load Intrinsic

#define simd_load(dest, src)                                                   \
  do {                                                                         \
    (dest) = __builtin_sw_vload(src);                                          \
  } while (0)

#define simd_loadu(dest, src)                                                  \
  do {                                                                         \
    (dest) = __builtin_sw_vloadu(src);                                         \
  } while (0)

#define simd_load_u(dest, src)                                                 \
  do {                                                                         \
    (dest) = __builtin_sw_vload_u(src);                                        \
  } while (0)

#define simd_loade(dest, src)                                                  \
  do {                                                                         \
    (dest) = __builtin_sw_vloade(src);                                         \
  } while (0)

#define simd_vload_nc(dest, src)                                               \
  do {                                                                         \
    (dest) = __builtin_sw_vloadnc(src);                                        \
  } while (0)

#define simd_store(src, dest)                                                  \
  do {                                                                         \
    __builtin_sw_vstore(src, dest);                                            \
  } while (0)

#define simd_storeu(src, dest)                                                 \
  do {                                                                         \
    __builtin_sw_vstoreu(src, dest);                                           \
  } while (0)

#define simd_store_u(src, dest)                                                \
  do {                                                                         \
    __builtin_sw_vstore_u(src, dest);                                          \
  } while (0)

#define simd_storeuh(src, dest)                                                \
  do {                                                                         \
    uint64_t __ptr = (uint64_t)dest + (uint64_t)sizeof(src);                   \
    __builtin_sw_vstoreuh(src, (__typeof__(dest))__ptr);                       \
  } while (0)

#define simd_storeul(src, dest)                                                \
  do {                                                                         \
    __builtin_sw_vstoreul(src, dest);                                          \
  } while (0)

#define simd_vstore_nc(src, dest)                                              \
  do {                                                                         \
    __builtin_sw_vstorenc(src, dest);                                          \
  } while (0)

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_loads(const float *__ptr) {
  return *(floatv4 *)__ptr;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_loadd(const double *__ptr) {
  return *(doublev4 *)__ptr;
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_stores(const float *__ptr,
                                                         floatv4 a) {
  *(floatv4 *)__ptr = a;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_stored(const double *__ptr,
                                                          doublev4 a) {
  *(doublev4 *)__ptr = a;
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_loadew(const int32_t *__ptr) {
  int32_t __a = *__ptr;
  return __extension__(intv8){__a, __a, __a, __a, __a, __a, __a, __a};
}

static __inline__ longv4 __DEFAULT_FN_ATTRS simd_loadel(const int64_t *__ptr) {
  int64_t __a = *__ptr;
  return __extension__(longv4){__a, __a, __a, __a};
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_loades(const float *__ptr) {
  float __a = *__ptr;
  return __extension__(floatv4){__a, __a, __a, __a};
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_loaded(const double *__ptr) {
  double __a = *__ptr;
  return __extension__(doublev4){__a, __a, __a, __a};
}

// Vector Setting Intrinsic Sw64

static __inline__ charv32 __DEFAULT_FN_ATTRS simd_set_charv32(
    int8_t __b31, int8_t __b30, int8_t __b29, int8_t __b28, int8_t __b27,
    int8_t __b26, int8_t __b25, int8_t __b24, int8_t __b23, int8_t __b22,
    int8_t __b21, int8_t __b20, int8_t __b19, int8_t __b18, int8_t __b17,
    int8_t __b16, int8_t __b15, int8_t __b14, int8_t __b13, int8_t __b12,
    int8_t __b11, int8_t __b10, int8_t __b09, int8_t __b08, int8_t __b07,
    int8_t __b06, int8_t __b05, int8_t __b04, int8_t __b03, int8_t __b02,
    int8_t __b01, int8_t __b00) {
  return __extension__(charv32){__b31, __b30, __b29, __b28, __b27, __b26, __b25,
                                __b24, __b23, __b22, __b21, __b20, __b19, __b18,
                                __b17, __b16, __b15, __b14, __b13, __b12, __b11,
                                __b10, __b09, __b08, __b07, __b06, __b05, __b04,
                                __b03, __b02, __b01, __b00};
}
#define simd_set_ucharv32 simd_set_charv32

static __inline__ shortv16 __DEFAULT_FN_ATTRS
simd_set_shortv16(int16_t __b15, int16_t __b14, int16_t __b13, int16_t __b12,
                  int16_t __b11, int16_t __b10, int16_t __b09, int16_t __b08,
                  int16_t __b07, int16_t __b06, int16_t __b05, int16_t __b04,
                  int16_t __b03, int16_t __b02, int16_t __b01, int16_t __b00) {
  return __extension__(shortv16){__b15, __b14, __b13, __b12, __b11, __b10,
                                 __b09, __b08, __b07, __b06, __b05, __b04,
                                 __b03, __b02, __b01, __b00};
}
#define simd_set_ushortv16 simd_set_shortv16

static __inline__ intv8 __DEFAULT_FN_ATTRS
simd_set_intv8(int32_t __b07, int32_t __b06, int32_t __b05, int32_t __b04,
               int32_t __b03, int32_t __b02, int32_t __b01, int32_t __b00) {
  return __extension__(intv8){__b07, __b06, __b05, __b04,
                              __b03, __b02, __b01, __b00};
}
#define simd_set_uintv8 simd_set_intv8

static __inline__ longv4 __DEFAULT_FN_ATTRS simd_set_longv4(int64_t __b03,
                                                            int64_t __b02,
                                                            int64_t __b01,
                                                            int64_t __b00) {
  return __extension__(longv4){__b03, __b02, __b01, __b00};
}
#define simd_set_ulongv4 simd_set_longv4
#define simd_set_int256 simd_set_longv4
#define simd_set_uint256 simd_set_longv4

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_set_floatv4(float __b03,
                                                              float __b02,
                                                              float __b01,
                                                              float __b00) {
  return __extension__(floatv4){__b03, __b02, __b01, __b00};
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_set_doublev4(double __b03,
                                                                double __b02,
                                                                double __b01,
                                                                double __b00) {
  return __extension__(doublev4){__b03, __b02, __b01, __b00};
}

// Integer Araith Intrinsic Sw64
// Caculate adden for given vector as int32_tx8,
// it isn't normal overflow result.
static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vaddw(intv8 a, intv8 b) {
  return a + b;
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vaddwi(intv8 a,
                                                       const int32_t b) {
  intv8 tmp = __extension__(intv8){b, b, b, b, b, b, b, b};
  return a + tmp;
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vsubw(intv8 a, intv8 b) {
  return a - b;
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vsubwi(intv8 a,
                                                       const int32_t b) {
  intv8 tmp = __extension__(intv8){b, b, b, b, b, b, b, b};
  return a - tmp;
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vucaddw(intv8 a, intv8 b) {
  return __builtin_sw_vucaddw(a, b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vucaddwi(intv8 a,
                                                         const int32_t b) {
  intv8 tmp = __extension__(intv8){b, b, b, b, b, b, b, b};
  return __builtin_sw_vucaddw(a, tmp);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vucsubw(intv8 a, intv8 b) {
  return __builtin_sw_vucsubw(a, b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vucsubwi(intv8 a,
                                                         const int32_t b) {
  intv8 tmp = __extension__(intv8){b, b, b, b, b, b, b, b};
  return __builtin_sw_vucsubw(a, tmp);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS simd_vaddl(longv4 a, longv4 b) {
  return a + b;
}

static __inline__ longv4 __DEFAULT_FN_ATTRS simd_vaddli(longv4 a,
                                                        const int64_t __b) {
  longv4 __tmp = __extension__(longv4){__b, __b, __b, __b};
  return a + __tmp;
}

static __inline__ longv4 __DEFAULT_FN_ATTRS simd_vsubl(longv4 a, longv4 b) {
  return a - b;
}

static __inline__ longv4 __DEFAULT_FN_ATTRS simd_vsubli(longv4 a,
                                                        const int64_t __b) {
  longv4 __tmp = __extension__(longv4){__b, __b, __b, __b};
  return a - __tmp;
}

// for core3 simd doesn't support v16i16, v32i8
// it must use v8i32 instead.
#ifdef __sw_64_sw8a__
static __inline__ shortv16 __DEFAULT_FN_ATTRS simd_vucaddh(shortv16 a,
                                                           shortv16 b) {
  return __builtin_sw_vucaddh_v16hi(a, b);
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS simd_vucaddhi(shortv16 a,
                                                            const int b) {
  int16_t __b = (int16_t)b;
  shortv16 tmp =
      __extension__(shortv16){__b, __b, __b, __b, __b, __b, __b, __b,
                              __b, __b, __b, __b, __b, __b, __b, __b};
  return __builtin_sw_vucaddh_v16hi(a, tmp);
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS simd_vucsubh(shortv16 a,
                                                           shortv16 b) {
  return __builtin_sw_vucsubh_v16hi(a, b);
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS simd_vucsubhi(shortv16 a,
                                                            const int b) {
  int16_t __b = (int16_t)b;
  shortv16 tmp =
      __extension__(shortv16){__b, __b, __b, __b, __b, __b, __b, __b,
                              __b, __b, __b, __b, __b, __b, __b, __b};
  return __builtin_sw_vucsubh_v16hi(a, tmp);
}

static __inline__ charv32 __DEFAULT_FN_ATTRS simd_vucaddb(charv32 a,
                                                          charv32 b) {
  return __builtin_sw_vucaddb_v32qi(a, b);
}

static __inline__ charv32 __DEFAULT_FN_ATTRS simd_vucaddbi(charv32 a,
                                                           const int b) {
  int8_t __b = (int8_t)b;
  charv32 tmp = __extension__(charv32){__b, __b, __b, __b, __b, __b, __b, __b,
                                       __b, __b, __b, __b, __b, __b, __b, __b,
                                       __b, __b, __b, __b, __b, __b, __b, __b,
                                       __b, __b, __b, __b, __b, __b, __b, __b};
  return __builtin_sw_vucaddb_v32qi(a, tmp);
}

static __inline__ charv32 __DEFAULT_FN_ATTRS simd_vucsubb(charv32 a,
                                                          charv32 b) {
  charv32 tmp =
      __extension__(charv32){b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b,
                             b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b};
  return __builtin_sw_vucsubb_v32qi(a, b);
}

static __inline__ charv32 __DEFAULT_FN_ATTRS simd_vucsubbi(charv32 a,
                                                           const int b) {
  int8_t __b = (int8_t)b;
  charv32 tmp = __extension__(charv32){__b, __b, __b, __b, __b, __b, __b, __b,
                                       __b, __b, __b, __b, __b, __b, __b, __b,
                                       __b, __b, __b, __b, __b, __b, __b, __b,
                                       __b, __b, __b, __b, __b, __b, __b, __b};
  return __builtin_sw_vucsubb_v32qi(a, tmp);
}
#else
static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vucaddh(intv8 a, intv8 b) {
  return __builtin_sw_vucaddh(a, b);
}

#define simd_vucaddhi __builtin_sw_vucaddhi

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vucsubh(intv8 a, intv8 b) {
  return __builtin_sw_vucsubh(a, b);
}

#define simd_vucsubhi __builtin_sw_vucsubhi

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vucaddb(intv8 a, intv8 b) {
  return __builtin_sw_vucaddb(a, b);
}

#define simd_vucaddbi __builtin_sw_vucaddbi

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vucsubb(intv8 a, intv8 b) {
  return __builtin_sw_vucsubb(a, b);
}

#define simd_vucsubbi __builtin_sw_vucsubbi
#endif

static __inline__ int32_t __DEFAULT_FN_ATTRS_CORE4 simd_vsumw(intv8 a) {
  return __builtin_sw_vsumw(a);
}

static __inline__ int64_t __DEFAULT_FN_ATTRS_CORE4 simd_vsuml(longv4 a) {
  return __builtin_sw_vsuml(a);
}

static __inline__ int32_t __DEFAULT_FN_ATTRS simd_ctpopow(int256 a) {
  return __builtin_sw_ctpopow(a);
}

static __inline__ int32_t __DEFAULT_FN_ATTRS simd_ctlzow(int256 a) {
  return __builtin_sw_ctlzow(a);
}

// Vector Shift intrinsics
// Gerate vsll(b|h|w|l) instruction due to Type define

static __inline__ uintv8 __DEFAULT_FN_ATTRS simd_vsllw(uintv8 a, int i) {
  return __builtin_sw_vsll(a, (int64_t)i);
}

static __inline__ uintv8 __DEFAULT_FN_ATTRS simd_vsrlw(uintv8 a, int i) {
  return __builtin_sw_vsrl(a, (int64_t)i);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vsraw(intv8 a, int i) {
  return __builtin_sw_vsra(a, (int64_t)i);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vrolw(intv8 a, int i) {
  return __builtin_sw_vrol(a, (int64_t)i);
}

#define simd_vsllwi simd_vsllw
#define simd_vsrlwi simd_vsrlw
#define simd_vsrawi simd_vsraw
#define simd_vrolwi simd_vrolw

static __inline__ charv32 __DEFAULT_FN_ATTRS_CORE4 simd_vsllb(charv32 a,
                                                              int i) {
  return __builtin_sw_vsll(a, (int64_t)i);
}

static __inline__ ucharv32 __DEFAULT_FN_ATTRS_CORE4 simd_vsrlb(ucharv32 a,
                                                               int i) {
  return __builtin_sw_vsrl(a, (int64_t)i);
}

static __inline__ charv32 __DEFAULT_FN_ATTRS_CORE4 simd_vsrab(charv32 a,
                                                              int i) {
  return __builtin_sw_vsra(a, (int64_t)i);
}

static __inline__ charv32 __DEFAULT_FN_ATTRS_CORE4 simd_vrolb(charv32 a,
                                                              int i) {
  return __builtin_sw_vrol(a, (int64_t)i);
}

#define simd_vsllbi simd_vsllb
#define simd_vsrlbi simd_vsrlb
#define simd_vsrabi simd_vsrab
#define simd_vrolbi simd_vrolb

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vslll(longv4 a, int i) {
  return __builtin_sw_vsll(a, (int64_t)i);
}

static __inline__ ulongv4 __DEFAULT_FN_ATTRS_CORE4 simd_vsrll(ulongv4 a,
                                                              int i) {
  return __builtin_sw_vsrl(a, (int64_t)i);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vsral(longv4 a, int i) {
  return __builtin_sw_vsra(a, (int64_t)i);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vroll(longv4 a, int i) {
  return __builtin_sw_vrol(a, (int64_t)i);
}

#define simd_vsllli simd_vslll
#define simd_vsrlli simd_vsrll
#define simd_vsrali simd_vsral
#define simd_vrolli simd_vroll

static __inline__ shortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vsllh(shortv16 a,
                                                               int i) {
  return __builtin_sw_vsll(a, (int64_t)i);
}

static __inline__ ushortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vsrlh(ushortv16 a,
                                                                int i) {
  return __builtin_sw_vsrl(a, (int64_t)i);
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vsrah(shortv16 a,
                                                               int i) {
  return __builtin_sw_vsra(a, (int64_t)i);
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vrolh(shortv16 a,
                                                               int i) {
  return __builtin_sw_vrol(a, (int64_t)i);
}

#define simd_vsllhi simd_vsllh
#define simd_vsrlhi simd_vsrlh
#define simd_vsrahi simd_vsrah
#define simd_vrolhi simd_vrolh

static __inline__ int256 __DEFAULT_FN_ATTRS simd_srlow(int256 a, int i) {
  return __builtin_sw_srlow(a, (int64_t)i);
}

static __inline__ int256 __DEFAULT_FN_ATTRS simd_sllow(int256 a, int i) {
  return __builtin_sw_sllow(a, (int64_t)i);
}

static __inline__ int256 __DEFAULT_FN_ATTRS simd_sraow(int256 a, int i) {
  return __builtin_sw_sraow(a, (int64_t)i);
}

#define simd_srlowi simd_srlow
#define simd_sllowi simd_sllow
#define simd_sraowi simd_sraow

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vslls1(floatv4 a) {
  return __builtin_sw_vslls(a, 64);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vslls2(floatv4 a) {
  return __builtin_sw_vslls(a, 128);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vslls3(floatv4 a) {
  return __builtin_sw_vslls(a, 192);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vslld1(doublev4 a) {
  return __builtin_sw_vslld(a, 64);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vslld2(doublev4 a) {
  return __builtin_sw_vslld(a, 128);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vslld3(doublev4 a) {
  return __builtin_sw_vslld(a, 192);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vsrls1(floatv4 a) {
  return __builtin_sw_vsrls(a, 64);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vsrls2(floatv4 a) {
  return __builtin_sw_vsrls(a, 128);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vsrls3(floatv4 a) {
  return __builtin_sw_vsrls(a, 192);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vsrld1(doublev4 a) {
  return __builtin_sw_vsrld(a, 64);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vsrld2(doublev4 a) {
  return __builtin_sw_vsrld(a, 128);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vsrld3(doublev4 a) {
  return __builtin_sw_vsrld(a, 192);
}

// Integer Compare Inst

static __inline__ int32_t __DEFAULT_FN_ATTRS simd_vcmpgew(intv8 a, intv8 b) {
  return __builtin_sw_vcmpgew(a, b);
}

static __inline__ int32_t __DEFAULT_FN_ATTRS simd_vcmpgewi(intv8 a,
                                                           const int32_t b) {
  intv8 tmp = __extension__(intv8){b, b, b, b, b, b, b, b};
  return __builtin_sw_vcmpgew(a, tmp);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vcmpeqw(intv8 a, intv8 b) {
  return __builtin_sw_vcmpeqw(a, b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vcmpeqwi(intv8 a,
                                                         const int32_t b) {
  intv8 tmp = __extension__(intv8){b, b, b, b, b, b, b, b};
  return __builtin_sw_vcmpeqw(a, tmp);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vcmplew(intv8 a, intv8 b) {
  return __builtin_sw_vcmplew(a, b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vcmplewi(intv8 a,
                                                         const int32_t b) {
  intv8 tmp = __extension__(intv8){b, b, b, b, b, b, b, b};
  return __builtin_sw_vcmplew(a, tmp);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vcmpltw(intv8 a, intv8 b) {
  return __builtin_sw_vcmpltw(a, b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vcmpltwi(intv8 a,
                                                         const int32_t b) {
  intv8 tmp = __extension__(intv8){b, b, b, b, b, b, b, b};
  return __builtin_sw_vcmpltw(a, tmp);
}

static __inline__ uintv8 __DEFAULT_FN_ATTRS simd_vcmpulew(uintv8 a, uintv8 b) {
  return __builtin_sw_vcmpulew(a, b);
}

static __inline__ uintv8 __DEFAULT_FN_ATTRS simd_vcmpulewi(uintv8 a,
                                                           const uint32_t b) {
  uintv8 tmp = __extension__(uintv8){b, b, b, b, b, b, b, b};
  return __builtin_sw_vcmpulew(a, tmp);
}

static __inline__ uintv8 __DEFAULT_FN_ATTRS simd_vcmpultw(uintv8 a, uintv8 b) {
  return __builtin_sw_vcmpultw(a, b);
}

static __inline__ uintv8 __DEFAULT_FN_ATTRS simd_vcmpultwi(uintv8 a,
                                                           const uint32_t b) {
  uintv8 tmp = __extension__(uintv8){b, b, b, b, b, b, b, b};
  return __builtin_sw_vcmpultw(a, tmp);
}

static __inline__ ucharv32 __DEFAULT_FN_ATTRS_CORE4 simd_vcmpueqb(ucharv32 a,
                                                                  ucharv32 b) {
  ucharv32 res = (ucharv32)__builtin_sw_vcmpueqb(a, b);
  return res;
}

static __inline__ ucharv32 __DEFAULT_FN_ATTRS_CORE4
simd_vcmpueqbi(ucharv32 a, const uint32_t b) {
  uint8_t __b = (uint8_t)b;
  ucharv32 tmp = __extension__(ucharv32){
      __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b,
      __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b,
      __b, __b, __b, __b, __b, __b, __b, __b, __b, __b};
  ucharv32 res = (ucharv32)__builtin_sw_vcmpueqb(a, tmp);
  return res;
}

static __inline__ ucharv32 __DEFAULT_FN_ATTRS_CORE4 simd_vcmpugtb(ucharv32 a,
                                                                  ucharv32 b) {
  ucharv32 res = (ucharv32)__builtin_sw_vcmpugtb(a, b);
  return res;
}

static __inline__ ucharv32 __DEFAULT_FN_ATTRS_CORE4
simd_vcmpugtbi(ucharv32 a, const uint32_t b) {
  uint8_t __b = (uint8_t)b;
  ucharv32 tmp = __extension__(ucharv32){
      __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b,
      __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b,
      __b, __b, __b, __b, __b, __b, __b, __b, __b, __b};
  ucharv32 res = (ucharv32)__builtin_sw_vcmpugtb(a, tmp);
  return res;
}

static __inline__ charv32 __DEFAULT_FN_ATTRS_CORE4 simd_vmaxb(charv32 a,
                                                              charv32 b) {
  return __builtin_sw_vmaxb(a, b);
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vmaxh(shortv16 a,
                                                               shortv16 b) {
  return __builtin_sw_vmaxh(a, b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS_CORE4 simd_vmaxw(intv8 a, intv8 b) {
  return __builtin_sw_vmaxw(a, b);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vmaxl(longv4 a,
                                                             longv4 b) {
  return __builtin_sw_vmaxl(a, b);
}

static __inline__ ucharv32 __DEFAULT_FN_ATTRS_CORE4 simd_vumaxb(ucharv32 a,
                                                                ucharv32 b) {
  return __builtin_sw_vumaxb(a, b);
}

static __inline__ ushortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vumaxh(ushortv16 a,
                                                                 ushortv16 b) {
  return __builtin_sw_vumaxh(a, b);
}

static __inline__ uintv8 __DEFAULT_FN_ATTRS_CORE4 simd_vumaxw(uintv8 a,
                                                              uintv8 b) {
  return __builtin_sw_vumaxw(a, b);
}

static __inline__ ulongv4 __DEFAULT_FN_ATTRS_CORE4 simd_vumaxl(ulongv4 a,
                                                               ulongv4 b) {
  return __builtin_sw_vumaxl(a, b);
}

static __inline__ charv32 __DEFAULT_FN_ATTRS_CORE4 simd_vminb(charv32 a,
                                                              charv32 b) {
  return __builtin_sw_vminb(a, b);
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vminh(shortv16 a,
                                                               shortv16 b) {
  return __builtin_sw_vminh(a, b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS_CORE4 simd_vminw(intv8 a, intv8 b) {
  return __builtin_sw_vminw(a, b);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vminl(longv4 a,
                                                             longv4 b) {
  return __builtin_sw_vminl(a, b);
}

static __inline__ ucharv32 __DEFAULT_FN_ATTRS_CORE4 simd_vuminb(ucharv32 a,
                                                                ucharv32 b) {
  return __builtin_sw_vuminb(a, b);
}

static __inline__ ushortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vuminh(ushortv16 a,
                                                                 ushortv16 b) {
  return __builtin_sw_vuminh(a, b);
}

static __inline__ uintv8 __DEFAULT_FN_ATTRS_CORE4 simd_vuminw(uintv8 a,
                                                              uintv8 b) {
  return __builtin_sw_vuminw(a, b);
}

static __inline__ ulongv4 __DEFAULT_FN_ATTRS_CORE4 simd_vuminl(ulongv4 a,
                                                               ulongv4 b) {
  return __builtin_sw_vuminl(a, b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vseleqw(intv8 a, intv8 b,
                                                        intv8 c) {
  return __builtin_sw_vseleqw(a, b, c);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vsellew(intv8 a, intv8 b,
                                                        intv8 c) {
  return __builtin_sw_vsellew(a, b, c);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vselltw(intv8 a, intv8 b,
                                                        intv8 c) {
  return __builtin_sw_vselltw(a, b, c);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vsellbcw(intv8 a, intv8 b,
                                                         intv8 c) {
  return __builtin_sw_vsellbcw(a, b, c);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vseleqwi(intv8 a, intv8 b,
                                                         int32_t c) {
  intv8 tmp = __extension__(intv8){c, c, c, c, c, c, c, c};
  return __builtin_sw_vseleqw(a, b, tmp);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vsellewi(intv8 a, intv8 b,
                                                         int32_t c) {
  intv8 tmp = __extension__(intv8){c, c, c, c, c, c, c, c};
  return __builtin_sw_vsellew(a, b, tmp);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vselltwi(intv8 a, intv8 b,
                                                         int32_t c) {
  intv8 tmp = __extension__(intv8){c, c, c, c, c, c, c, c};
  return __builtin_sw_vselltw(a, b, tmp);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vsellbcwi(intv8 a, intv8 b,
                                                          int32_t c) {
  intv8 tmp = __extension__(intv8){c, c, c, c, c, c, c, c};
  return __builtin_sw_vsellbcw(a, b, tmp);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS simd_vseleql(longv4 a, longv4 b,
                                                         longv4 c) {
  doublev4 tmp_a = (doublev4)a;
  doublev4 tmp_b = (doublev4)b;
  doublev4 tmp_c = (doublev4)c;
  return (longv4)__builtin_sw_vfseleqd(tmp_a, tmp_b, tmp_c);
}

// Vector Logic Operation

#define simd_vlog(a, b, c, opcode) __builtin_sw_vlogzz(a, b, c, opcode)

#define simd_vand(SUFFIX, TYPE)                                                \
  static __inline__ TYPE __DEFAULT_FN_ATTRS simd_vand##SUFFIX(TYPE a,          \
                                                              TYPE b) {        \
    return a & b;                                                              \
  }

simd_vand(b, charv32)
simd_vand(h, shortv16)
simd_vand(w, intv8)
simd_vand(l, longv4)

#define simd_vbic(SUFFIX, TYPE)                                                \
  static __inline__ TYPE __DEFAULT_FN_ATTRS simd_vbic##SUFFIX(TYPE a,          \
                                                              TYPE b) {        \
    return a & ~b;                                                             \
  }

simd_vbic(b, charv32)
simd_vbic(h, shortv16)
simd_vbic(w, intv8)
simd_vbic(l, longv4)

#define simd_vbis(SUFFIX, TYPE)                                                \
  static __inline__ TYPE __DEFAULT_FN_ATTRS simd_vbis##SUFFIX(TYPE a,          \
                                                              TYPE b) {        \
    return a | b;                                                              \
  }

simd_vbis(b, charv32)
simd_vbis(h, shortv16)
simd_vbis(w, intv8)
simd_vbis(l, longv4)

#define simd_vornot(SUFFIX, TYPE)                                              \
  static __inline__ TYPE __DEFAULT_FN_ATTRS simd_vornot##SUFFIX(TYPE a,        \
                                                                TYPE b) {      \
    return a | ~b;                                                             \
  }

simd_vornot(b, charv32)
simd_vornot(h, shortv16)
simd_vornot(w, intv8)
simd_vornot(l, longv4)

#define simd_vxor(SUFFIX, TYPE)                                                \
  static __inline__ TYPE __DEFAULT_FN_ATTRS simd_vxor##SUFFIX(TYPE a,          \
                                                              TYPE b) {        \
    return a ^ b;                                                              \
  }

simd_vxor(b, charv32)
simd_vxor(h, shortv16)
simd_vxor(w, intv8)
simd_vxor(l, longv4)

#define simd_veqv(SUFFIX, TYPE)                                                \
  static __inline__ TYPE __DEFAULT_FN_ATTRS simd_veqv##SUFFIX(TYPE a,          \
                                                              TYPE b) {        \
    return ~(a ^ b);                                                           \
  }

simd_veqv(b, charv32)
simd_veqv(h, shortv16)
simd_veqv(w, intv8)
simd_veqv(l, longv4)

// float arithmetic Operation

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vadds(floatv4 a, floatv4 b) {
  return a + b;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vaddd(doublev4 a,
                                                         doublev4 b) {
  return a + b;
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vsubs(floatv4 a, floatv4 b) {
  return a - b;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vsubd(doublev4 a,
                                                         doublev4 b) {
  return a - b;
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vmuls(floatv4 a, floatv4 b) {
  return a * b;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vmuld(doublev4 a,
                                                         doublev4 b) {
  return a * b;
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vdivs(floatv4 a, floatv4 b) {
  return a / b;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vdivd(doublev4 a,
                                                         doublev4 b) {
  return a / b;
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vsqrts(floatv4 a) {
  return __builtin_sw_vsqrts(a);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vsqrtd(doublev4 a) {
  return __builtin_sw_vsqrtd(a);
}

static __inline__ float __DEFAULT_FN_ATTRS_CORE4 simd_vsums(floatv4 a) {
  return __builtin_sw_vsums(a);
}

static __inline__ double __DEFAULT_FN_ATTRS_CORE4 simd_vsumd(doublev4 a) {
  return __builtin_sw_vsumd(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfrecs(floatv4 a) {
  return __builtin_sw_vfrecs(a);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vfrecd(doublev4 a) {
  return __builtin_sw_vfrecd(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vfcmpeqs(floatv4 a,
                                                           floatv4 b) {
  return __builtin_sw_vfcmpeqs(a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vfcmples(floatv4 a,
                                                           floatv4 b) {
  return __builtin_sw_vfcmples(a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vfcmplts(floatv4 a,
                                                           floatv4 b) {
  return __builtin_sw_vfcmplts(a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vfcmpuns(floatv4 a,
                                                           floatv4 b) {
  return __builtin_sw_vfcmpuns(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vfcmpeqd(doublev4 a,
                                                            doublev4 b) {
  return __builtin_sw_vfcmpeqd(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vfcmpled(doublev4 a,
                                                            doublev4 b) {
  return __builtin_sw_vfcmpled(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vfcmpltd(doublev4 a,
                                                            doublev4 b) {
  return __builtin_sw_vfcmpltd(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vfcmpund(doublev4 a,
                                                            doublev4 b) {
  return __builtin_sw_vfcmpund(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtsd(floatv4 a) {
  return __builtin_sw_vfcvtsd(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtds(doublev4 a) {
  return __builtin_sw_vfcvtds(a);
}

#define simd_vfcvtsh(a, b, c) __builtin_sw_vfcvtsh(a, b, c)
#define simd_vfcvths(a, b) __builtin_sw_vfcvths(a, b)

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtls(longv4 a) {
  return __builtin_sw_vfcvtls(a);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtld(longv4 a) {
  return __builtin_sw_vfcvtld(a);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtsl(floatv4 a) {
  doublev4 tmp = __builtin_sw_vfcvtsd(a);
  return __builtin_sw_vfcvtdl(tmp);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtdl(doublev4 a) {
  return __builtin_sw_vfcvtdl(a);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtdl_g(doublev4 a) {
  return __builtin_sw_vfcvtdl_g(a);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtdl_p(doublev4 a) {
  return __builtin_sw_vfcvtdl_p(a);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtdl_z(doublev4 a) {
  return __builtin_sw_vfcvtdl_z(a);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfcvtdl_n(doublev4 a) {
  return __builtin_sw_vfcvtdl_n(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfris(floatv4 a) {
  return __builtin_sw_vfris(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfris_g(floatv4 a) {
  return __builtin_sw_vfris_g(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfris_p(floatv4 a) {
  return __builtin_sw_vfris_p(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfris_z(floatv4 a) {
  return __builtin_sw_vfris_z(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vfris_n(floatv4 a) {
  return __builtin_sw_vfris_n(a);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vfrid(doublev4 a) {
  return __builtin_sw_vfrid(a);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vfrid_g(doublev4 a) {
  return __builtin_sw_vfrid_g(a);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vfrid_p(doublev4 a) {
  return __builtin_sw_vfrid_p(a);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vfrid_z(doublev4 a) {
  return __builtin_sw_vfrid_z(a);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vfrid_n(doublev4 a) {
  return __builtin_sw_vfrid_n(a);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vmaxs(floatv4 a,
                                                              floatv4 b) {
  return __builtin_sw_vmaxs(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vmaxd(doublev4 a,
                                                               doublev4 b) {
  return __builtin_sw_vmaxd(a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS_CORE4 simd_vmins(floatv4 a,
                                                              floatv4 b) {
  return __builtin_sw_vmins(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS_CORE4 simd_vmind(doublev4 a,
                                                               doublev4 b) {
  return __builtin_sw_vmind(a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vcpyss(floatv4 a, floatv4 b) {
  return __builtin_sw_vcpyss(a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vcpyses(floatv4 a,
                                                          floatv4 b) {
  return __builtin_sw_vcpyses(a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vcpysns(floatv4 a,
                                                          floatv4 b) {
  return __builtin_sw_vcpysns(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vcpysd(doublev4 a,
                                                          doublev4 b) {
  return __builtin_sw_vcpysd(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vcpysed(doublev4 a,
                                                           doublev4 b) {
  return __builtin_sw_vcpysed(a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vcpysnd(doublev4 a,
                                                           doublev4 b) {
  return __builtin_sw_vcpysnd(a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vfseleqs(floatv4 cond,
                                                           floatv4 a,
                                                           floatv4 b) {
  return __builtin_sw_vfseleqs(cond, a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vfsellts(floatv4 cond,
                                                           floatv4 a,
                                                           floatv4 b) {
  return __builtin_sw_vfsellts(cond, a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vfselles(floatv4 cond,
                                                           floatv4 a,
                                                           floatv4 b) {
  return __builtin_sw_vfselles(cond, a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vfseleqd(doublev4 cond,
                                                            doublev4 a,
                                                            doublev4 b) {
  return __builtin_sw_vfseleqd(cond, a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vfselltd(doublev4 cond,
                                                            doublev4 a,
                                                            doublev4 b) {
  return __builtin_sw_vfselltd(cond, a, b);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vfselled(doublev4 cond,
                                                            doublev4 a,
                                                            doublev4 b) {
  return __builtin_sw_vfselled(cond, a, b);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vmas(floatv4 a, floatv4 b,
                                                       floatv4 c) {
  return a * b + c;
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vmss(floatv4 a, floatv4 b,
                                                       floatv4 c) {
  return a * b - c;
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vnmas(floatv4 a, floatv4 b,
                                                        floatv4 c) {
  return -a * b + c;
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vnmss(floatv4 a, floatv4 b,
                                                        floatv4 c) {
  return -(a * b + c);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vmad(doublev4 a, doublev4 b,
                                                        doublev4 c) {
  return a * b + c;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vmsd(doublev4 a, doublev4 b,
                                                        doublev4 c) {
  return a * b - c;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vnmad(doublev4 a, doublev4 b,
                                                         doublev4 c) {
  return -a * b + c;
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vnmsd(doublev4 a, doublev4 b,
                                                         doublev4 c) {
  return -(a * b + c);
}

// SIMD element Operation

#ifdef __sw_64_sw8a__
#define simd_vinsb(elt, vect, num) __builtin_sw_vinsb(elt, vect, num)
#define simd_vinsh(elt, vect, num) __builtin_sw_vinsh(elt, vect, num)
#endif

#define simd_vinsw(elt, vect, num) __builtin_sw_vinsw(elt, vect, num)
#define simd_vinsl(elt, vect, num) __builtin_sw_vinsl(elt, vect, num)
#define simd_vinsfs(elt, vect, num) __builtin_sw_vinsfs(elt, vect, num)
#define simd_vinsfd(elt, vect, num) __builtin_sw_vinsfd(elt, vect, num)

#define simd_vinsw0(elt, vect) simd_vinsw(elt, vect, 0)
#define simd_vinsw1(elt, vect) simd_vinsw(elt, vect, 1)
#define simd_vinsw2(elt, vect) simd_vinsw(elt, vect, 2)
#define simd_vinsw3(elt, vect) simd_vinsw(elt, vect, 3)
#define simd_vinsw4(elt, vect) simd_vinsw(elt, vect, 4)
#define simd_vinsw5(elt, vect) simd_vinsw(elt, vect, 5)
#define simd_vinsw6(elt, vect) simd_vinsw(elt, vect, 6)
#define simd_vinsw7(elt, vect) simd_vinsw(elt, vect, 7)

#define simd_vinsl0(elt, vect) simd_vinsl(elt, vect, 0)
#define simd_vinsl1(elt, vect) simd_vinsl(elt, vect, 1)
#define simd_vinsl2(elt, vect) simd_vinsl(elt, vect, 2)
#define simd_vinsl3(elt, vect) simd_vinsl(elt, vect, 3)

#define simd_vinsfs0(elt, vect) simd_vinsfs(elt, vect, 0)
#define simd_vinsfs1(elt, vect) simd_vinsfs(elt, vect, 1)
#define simd_vinsfs2(elt, vect) simd_vinsfs(elt, vect, 2)
#define simd_vinsfs3(elt, vect) simd_vinsfs(elt, vect, 3)

#define simd_vinsfd0(elt, vect) simd_vinsfd(elt, vect, 0)
#define simd_vinsfd1(elt, vect) simd_vinsfd(elt, vect, 1)
#define simd_vinsfd2(elt, vect) simd_vinsfd(elt, vect, 2)
#define simd_vinsfd3(elt, vect) simd_vinsfd(elt, vect, 3)

static __inline__ charv32 __DEFAULT_FN_ATTRS_CORE4 simd_vinsectlb(charv32 __a,
                                                                  charv32 __b) {
  return __builtin_shufflevector(
      __a, __b, 0, 0 + 32, 1, 1 + 32, 2, 2 + 32, 3, 3 + 32, 4, 4 + 32, 5,
      5 + 32, 6, 6 + 32, 7, 7 + 32, 8, 8 + 32, 9, 9 + 32, 10, 10 + 32, 11,
      11 + 32, 12, 12 + 32, 13, 13 + 32, 14, 14 + 32, 15, 15 + 32);
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS_CORE4
simd_vinsectlh(shortv16 __a, shortv16 __b) {
  return __builtin_shufflevector(__a, __b, 0, 0 + 16, 1, 1 + 16, 2, 2 + 16, 3,
                                 3 + 16, 4, 4 + 16, 5, 5 + 16, 6, 6 + 16, 7,
                                 7 + 16);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS_CORE4 simd_vinsectlw(intv8 __a,
                                                                intv8 __b) {
  return __builtin_shufflevector(__a, __b, 0, 0 + 8, 1, 1 + 8, 2, 2 + 8, 3,
                                 3 + 8);
}

static __inline__ longv4 __DEFAULT_FN_ATTRS_CORE4 simd_vinsectll(longv4 __a,
                                                                 longv4 __b) {
  return __builtin_shufflevector(__a, __b, 0, 0 + 4, 1, 1 + 4);
}

#ifdef __sw_64_sw8a__
#define simd_vshfq(__a, __b, idx) __builtin_sw_vshfq(__a, __b, idx)
#endif

static __inline__ charv32 __DEFAULT_FN_ATTRS_CORE4 simd_vshfqb(charv32 __a,
                                                               charv32 __b) {
  return __builtin_sw_vshfqb(__a, __b);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vshfw(intv8 __a, intv8 __b,
                                                      int64_t idx) {
  return __builtin_sw_vshfw(__a, __b, idx);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vconw(intv8 __a, intv8 __b,
                                                      void *ptr) {
  return __builtin_sw_vconw(__a, __b, ptr);
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vconl(intv8 __a, intv8 __b,
                                                      void *ptr) {
  return __builtin_sw_vconl(__a, __b, ptr);
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vcons(floatv4 __a,
                                                        floatv4 __b,
                                                        void *ptr) {
  return __builtin_sw_vcons(__a, __b, ptr);
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vcond(doublev4 __a,
                                                         doublev4 __b,
                                                         void *ptr) {
  return __builtin_sw_vcond(__a, __b, ptr);
}

#define simd_vextw(vect, num) __builtin_sw_vextw(vect, num)
#define simd_vextl(vect, num) __builtin_sw_vextl(vect, num)
#define simd_vextfs(vect, num) __builtin_sw_vextfs(vect, num)
#define simd_vextfd(vect, num) __builtin_sw_vextfd(vect, num)

#define simd_vextw0(args) simd_vextw(args, 0)
#define simd_vextw1(args) simd_vextw(args, 1)
#define simd_vextw2(args) simd_vextw(args, 2)
#define simd_vextw3(args) simd_vextw(args, 3)
#define simd_vextw4(args) simd_vextw(args, 4)
#define simd_vextw5(args) simd_vextw(args, 5)
#define simd_vextw6(args) simd_vextw(args, 6)
#define simd_vextw7(args) simd_vextw(args, 7)

#define simd_vextl0(args) simd_vextl(args, 0)
#define simd_vextl1(args) simd_vextl(args, 1)
#define simd_vextl2(args) simd_vextl(args, 2)
#define simd_vextl3(args) simd_vextl(args, 3)

#define simd_vextfs0(args) simd_vextfs(args, 0)
#define simd_vextfs1(args) simd_vextfs(args, 1)
#define simd_vextfs2(args) simd_vextfs(args, 2)
#define simd_vextfs3(args) simd_vextfs(args, 3)

#define simd_vextfd0(args) simd_vextfd(args, 0)
#define simd_vextfd1(args) simd_vextfd(args, 1)
#define simd_vextfd2(args) simd_vextfd(args, 2)
#define simd_vextfd3(args) simd_vextfd(args, 3)

static __inline__ charv32 __DEFAULT_FN_ATTRS_CORE4 simd_vcpyb(int8_t b) {
  return __extension__(charv32){b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b,
                                b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b};
}

static __inline__ shortv16 __DEFAULT_FN_ATTRS_CORE4 simd_vcpyh(int16_t b) {
  return __extension__(shortv16){b, b, b, b, b, b, b, b,
                                 b, b, b, b, b, b, b, b};
}

static __inline__ intv8 __DEFAULT_FN_ATTRS simd_vcpyw(int32_t b) {
  return __extension__(intv8){b, b, b, b, b, b, b, b};
}

static __inline__ longv4 __DEFAULT_FN_ATTRS simd_vcpyl(int64_t __a) {
  return __extension__(longv4){__a, __a, __a, __a};
}

static __inline__ floatv4 __DEFAULT_FN_ATTRS simd_vcpyfs(float __a) {
  return __extension__(floatv4){__a, __a, __a, __a};
}

static __inline__ doublev4 __DEFAULT_FN_ATTRS simd_vcpyfd(double __a) {
  return __extension__(doublev4){__a, __a, __a, __a};
}

// Test for core3

static __inline__ int32_t __DEFAULT_FN_ATTRS simd_reduc_plusw(intv8 __a) {
  intv8 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3, 5, 5, 7, 7);
  __a = __a + __shf;
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2, 6, 6, 6, 6);
  __a = __a + __shf;
  __shf = __builtin_shufflevector(__a, __a, 4, 4, 4, 4, 4, 4, 4, 4);
  __a = __a + __shf;
  return __builtin_sw_vextw(__a, 0);
}

static __inline__ float __DEFAULT_FN_ATTRS simd_reduc_pluss(floatv4 __a) {
  floatv4 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3);
  __a = __a + __shf;
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2);
  __a = __a + __shf;
  return __builtin_sw_vextfs(__a, 0);
}

static __inline__ double __DEFAULT_FN_ATTRS simd_reduc_plusd(doublev4 __a) {
  doublev4 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3);
  __a = __a + __shf;
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2);
  __a = __a + __shf;
  return __builtin_sw_vextfd(__a, 0);
}

static __inline__ int32_t __DEFAULT_FN_ATTRS simd_reduc_smaxw(intv8 __a) {
  intv8 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3, 5, 5, 7, 7);
  intv8 __cmp = simd_vcmpltw(__a, __shf);
  __a = simd_vseleqw(__cmp, __a, __shf);
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2, 6, 6, 6, 6);
  __cmp = simd_vcmpltw(__a, __shf);
  __a = simd_vseleqw(__cmp, __a, __shf);
  __shf = __builtin_shufflevector(__a, __a, 4, 4, 4, 4, 4, 4, 4, 4);
  __cmp = simd_vcmpltw(__a, __shf);
  __a = simd_vseleqw(__cmp, __a, __shf);
  return __builtin_sw_vextw(__a, 0);
}

static __inline__ uint32_t __DEFAULT_FN_ATTRS simd_reduc_umaxw(uintv8 __a) {
  uintv8 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3, 5, 5, 7, 7);
  uintv8 __cmp = simd_vcmpultw(__a, __shf);
  __a = simd_vseleqw(__cmp, __a, __shf);
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2, 6, 6, 6, 6);
  __cmp = simd_vcmpultw(__a, __shf);
  __a = simd_vseleqw(__cmp, __a, __shf);
  __shf = __builtin_shufflevector(__a, __a, 4, 4, 4, 4, 4, 4, 4, 4);
  __cmp = simd_vcmpultw(__a, __shf);
  __a = simd_vseleqw(__cmp, __a, __shf);
  return __builtin_sw_vextw(__a, 0);
}

static __inline__ int32_t __DEFAULT_FN_ATTRS simd_reduc_sminw(intv8 __a) {
  intv8 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3, 5, 5, 7, 7);
  intv8 __cmp = simd_vcmpltw(__a, __a);
  __a = simd_vseleqw(__cmp, __shf, __a);
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2, 6, 6, 6, 6);
  __cmp = simd_vcmpltw(__a, __shf);
  __a = simd_vseleqw(__cmp, __shf, __a);
  __shf = __builtin_shufflevector(__a, __a, 4, 4, 4, 4, 4, 4, 4, 4);
  __cmp = simd_vcmpltw(__a, __shf);
  __a = simd_vseleqw(__cmp, __shf, __a);
  return __builtin_sw_vextw(__a, 0);
}

static __inline__ uint32_t __DEFAULT_FN_ATTRS simd_reduc_uminw(intv8 __a) {
  intv8 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3, 5, 5, 7, 7);
  intv8 __cmp = simd_vcmpultw(__a, __shf);
  __a = simd_vseleqw(__cmp, __shf, __a);
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2, 6, 6, 6, 6);
  __cmp = simd_vcmpultw(__a, __shf);
  __a = simd_vseleqw(__cmp, __shf, __a);
  __shf = __builtin_shufflevector(__a, __a, 4, 4, 4, 4, 4, 4, 4, 4);
  __cmp = simd_vcmpultw(__a, __shf);
  __a = simd_vseleqw(__cmp, __shf, __a);
  return __builtin_sw_vextw(__a, 0);
}

static __inline__ float __DEFAULT_FN_ATTRS simd_reduc_smaxs(floatv4 __a) {
  floatv4 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3);
  floatv4 __cmp = simd_vfcmplts(__a, __shf);
  __a = simd_vfseleqs(__cmp, __a, __shf);
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2);
  __cmp = simd_vfcmplts(__a, __shf);
  __a = simd_vfseleqs(__cmp, __a, __shf);
  return __builtin_sw_vextfs(__a, 0);
}

static __inline__ double __DEFAULT_FN_ATTRS simd_reduc_smaxd(doublev4 __a) {
  doublev4 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3);
  doublev4 __cmp = simd_vfcmpltd(__a, __shf);
  __a = simd_vfseleqd(__cmp, __a, __shf);
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2);
  __cmp = simd_vfcmpltd(__a, __shf);
  __a = simd_vfseleqd(__cmp, __a, __shf);
  return __builtin_sw_vextfd(__a, 0);
}

static __inline__ float __DEFAULT_FN_ATTRS simd_reduc_smins(floatv4 __a) {
  floatv4 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3);
  floatv4 __cmp = simd_vfcmplts(__a, __shf);
  __a = simd_vfseleqs(__cmp, __shf, __a);
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2);
  __cmp = simd_vfcmplts(__a, __shf);
  __a = simd_vfseleqs(__cmp, __shf, __a);
  return __builtin_sw_vextfs(__a, 0);
}

static __inline__ double __DEFAULT_FN_ATTRS simd_reduc_smind(doublev4 __a) {
  doublev4 __shf = __builtin_shufflevector(__a, __a, 1, 1, 3, 3);
  doublev4 __cmp = simd_vfcmpltd(__a, __shf);
  __a = simd_vfseleqd(__cmp, __shf, __a);
  __shf = __builtin_shufflevector(__a, __a, 2, 2, 2, 2);
  __cmp = simd_vfcmpltd(__a, __shf);
  __a = simd_vfseleqd(__cmp, __shf, __a);
  return __builtin_sw_vextfd(__a, 0);
}
#endif
