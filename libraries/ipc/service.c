#include "libraries/ipc/service.h"
#include "libraries/ipc/server.h"
#include "libraries/services/svc.h"

static void ipc_Copy(void *dst, const void *src, size_t size)
{
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    for(size_t i = 0; i < size; i++)
        d[i] = s[i];
}

long serviceOpen(Service *s, const char *name)
{
    long h = svcConnectToPort(name);
    s->session = h > 0 ? h : 0;
    return h < 0 ? h : SVC_OK;
}

void serviceClose(Service *s)
{
    if(serviceIsActive(s))
        svcCloseHandle(s->session);
    s->session = 0;
}

static long ipc_Dispatch(Service *s, uint32_t cmd, const void *in, size_t inSize, void *out, size_t outSize,
                        const SfBuffer *bufs, size_t bufCount, long handle)
{
    IpcMessage *msg = Ipc_GetMessage();
    if(!serviceIsActive(s))
        return SVC_ERR_INVALID_HANDLE;
    if(inSize > sizeof(msg->data) || bufCount > IPC_MAX_BUFS)
        return SVC_ERR_INVALID_SIZE;

    msg->cmd = cmd;
    msg->dataSize = inSize;
    msg->bufCount = bufCount;
    msg->copyHandle = handle;
    ipc_Copy(msg->data, in, inSize);
    for(size_t i = 0; i < bufCount; i++)
    {
        msg->bufs[i].va = (uintptr_t)bufs[i].ptr;
        msg->bufs[i].size = bufs[i].size;
    }

    long r = svcSendSyncRequest(s->session);
    if(r != SVC_OK)
        return r;

    long result = (int32_t)msg->cmd;
    if(result != SVC_OK)
        return result;

    if(msg->dataSize < outSize)
        return SVC_ERR_INVALID_SIZE;
    ipc_Copy(out, msg->data, outSize);
    return SVC_OK;
}

long serviceDispatch(Service *s, uint32_t cmd, const void *in, size_t inSize, void *out, size_t outSize,
                     const SfBuffer *bufs, size_t bufCount)
{
    return ipc_Dispatch(s, cmd, in, inSize, out, outSize, bufs, bufCount, 0);
}

long serviceDispatchHandle(Service *s, uint32_t cmd, const void *in, size_t inSize, long handle)
{
    return ipc_Dispatch(s, cmd, in, inSize, 0, 0, 0, 0, handle);
}

//Requests are copied out so handlers can read them while the reply is built in the IPC buffer
static IpcMessage request;

long serverRegister(const char *name)
{
    return svcCreatePort(name);
}

long serverRun(long port, ServerHandler handler, void *ctx)
{
    IpcMessage *msg = Ipc_GetMessage();
    long reply = 0;

    while(1)
    {
        long session = svcReplyAndReceive(port, reply);
        if(session < 0)
        {
            //A failed reply still leaves the port usable; anything else is fatal
            if(!reply)
                return session;
            reply = 0;
            continue;
        }
        ipc_Copy(&request, msg, IPC_BUFFER_SIZE);

        ServerRequest req;
        req.cmd = request.cmd;
        req.in = request.data;
        req.inSize = request.dataSize > sizeof(request.data) ? sizeof(request.data) : request.dataSize;
        req.copyHandle = request.copyHandle;
        req.bufCount = request.bufCount > IPC_MAX_BUFS ? IPC_MAX_BUFS : request.bufCount;
        for(int i = 0; i < IPC_MAX_BUFS; i++)
        {
            req.bufCapacity[i] = i < req.bufCount ? request.bufs[i].size : 0;
            req.replyBufs[i].ptr = 0;
            req.replyBufs[i].size = 0;
        }
        req.out = msg->data;
        req.outCapacity = sizeof(msg->data);
        req.outSize = 0;

        long result = handler(&req, ctx);

        msg->cmd = (uint32_t)result;
        msg->dataSize = 0;
        msg->bufCount = 0;
        msg->copyHandle = 0;
        if(result == SVC_OK)
        {
            msg->dataSize = req.outSize > req.outCapacity ? req.outCapacity : req.outSize;
            for(int i = 0; i < IPC_MAX_BUFS; i++)
            {
                msg->bufs[i].va = (uintptr_t)req.replyBufs[i].ptr;
                msg->bufs[i].size = req.replyBufs[i].size;
                if(req.replyBufs[i].ptr)
                    msg->bufCount = i + 1;
            }
        }
        reply = session;
    }
}
