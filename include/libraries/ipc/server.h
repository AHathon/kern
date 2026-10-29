#pragma once

#include "libraries/types.h"
#include "libraries/ipc/service.h"

typedef struct
{
    uint32_t cmd;
    const void *in;
    size_t inSize;
    uint8_t bufCount;
    uint32_t bufCapacity[IPC_MAX_BUFS];

    void *out;
    size_t outCapacity;
    size_t outSize;
    SfBuffer replyBufs[IPC_MAX_BUFS];
} ServerRequest;

//Return value is the result the client's serviceDispatch sees; reply data is only sent on 0
typedef long (*ServerHandler)(ServerRequest *req, void *ctx);

#ifdef __cplusplus
extern "C" {
#endif

long serverRegister(const char *name);
long serverRun(long port, ServerHandler handler, void *ctx); //only returns on a fatal receive error

#ifdef __cplusplus
}
#endif
