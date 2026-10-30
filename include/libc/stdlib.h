#pragma once

#include "libraries/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void qsort(void *base, size_t count, size_t size, int (*compare)(const void *, const void *));
int atoi(const char *s);
double atof(const char *s);
int abs(int v);

#ifdef __cplusplus
}
#endif
