#pragma once

#include "libraries/types.h"
#include "libraries/services/ipc.h"

typedef struct
{
    long session;
} Service;

typedef struct
{
    void *ptr;
    size_t size;
} SfBuffer;

#ifdef __cplusplus
extern "C" {
#endif

//Blocks until a service with this name registers
long serviceOpen(Service *s, const char *name);
void serviceClose(Service *s);

static inline int serviceIsActive(Service *s)
{
    return s->session > 0;
}

//Sends cmd with `in` as the request data and copies the reply data into `out` on success.
//Returns SVC_ERR_* if the kernel failed the call, otherwise the server's result.
long serviceDispatch(Service *s, uint32_t cmd, const void *in, size_t inSize, void *out, size_t outSize, const SfBuffer *bufs, size_t bufCount);
long serviceDispatchHandle(Service *s, uint32_t cmd, const void *in, size_t inSize, long handle);

#ifdef __cplusplus
}
#endif
