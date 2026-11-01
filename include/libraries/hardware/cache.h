#pragma once

#include "libraries/types.h"

#ifdef __cplusplus
extern "C" {
#endif

//EL1 only; EL0 has no access to cache maintenance
void Cache_CleanInvalidate(const volatile void *addr, size_t size);
void Cache_SyncCode(const void *addr, size_t size);

#ifdef __cplusplus
}
#endif
