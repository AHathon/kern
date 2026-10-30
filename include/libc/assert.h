#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void __assert_fail(const char *expr, const char *file);

#ifdef __cplusplus
}
#endif

#define assert(expr) ((expr) ? (void)0 : __assert_fail(#expr, __FILE__))
