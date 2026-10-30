#pragma once

typedef __SIZE_TYPE__ size_t;

//Kernel builds with -nostdinc; services see clang's headers and need to agree with them
#if __has_include(<stdint.h>)
#include <stdint.h>
#include <stdarg.h>
#else
typedef __UINTPTR_TYPE__ uintptr_t;

typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;

typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;

#define UINT32_MAX 0xFFFFFFFF
#define UINT64_MAX 0xFFFFFFFFFFFFFFFF
#endif

#define UNUSED(x) (void)x

#define SET_BIT(x, y) (x |= (y))
#define CLR_BIT(x, y) (x &= ~(y))

#if !__has_include(<stdarg.h>)
typedef __builtin_va_list va_list;
#define va_start(ap,last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap,type) __builtin_va_arg(ap,type)
#define va_copy(dest, src) __builtin_va_copy(dest,src)
#endif