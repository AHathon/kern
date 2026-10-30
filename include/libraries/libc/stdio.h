#pragma once

#include "libraries/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int printf(const char *fmt, ...);
int sscanf(const char *str, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
