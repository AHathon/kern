#include "kernel/kIpc.h"
#include "kernel/kScheduler.h"
#include "kernel/memory/kMemoryMap.h"
#include "kernel/memory/mmu.h"
#include "libraries/services/svc.h"
#include "libraries/services/ipc.h"
#include "libraries/libc/string.h"

typedef enum
{
    SESSION_IDLE,
    SESSION_PENDING,
    SESSION_IN_SERVICE
} SessionState;

typedef struct kSession kSession;

typedef struct
{
    uint8_t inUse;
    char name[IPC_PORT_NAME_LEN];
    kProcess *owner;
    kThread *waiter;    //server blocked in ReplyAndReceive
    kSession *pendingHead;
    kSession *pendingTail;
} kPort;

struct kSession
{
    uint8_t inUse;
    uint8_t state;
    uint8_t clientOpen;
    kPort *port;
    kProcess *client;
    kThread *clientThread;  //blocked in SendSyncRequest
    kSession *nextPending;
    long serverHandle;
    uint8_t clientBufCount;
    IpcBufDesc clientBufs[IPC_MAX_BUFS];
};

typedef struct
{
    kThread *thread;
    kProcess *proc;
    char name[IPC_PORT_NAME_LEN];
} kConnectWaiter;

static kPort ports[IPC_MAX_PORTS];
static kSession sessions[IPC_MAX_SESSIONS];
static kConnectWaiter waiters[IPC_MAX_WAITERS];

static kThread *Ipc_CurrentThread()
{
    return kScheduler_GetCurrentThread();
}

static kProcess *Ipc_CurrentProcess()
{
    return (kProcess*)kScheduler_GetCurrentThread()->parent;
}

static int Ipc_NameEq(const char *a, const char *b)
{
    for(int i = 0; i < IPC_PORT_NAME_LEN; i++)
    {
        if(a[i] != b[i])
            return 0;
        if(!a[i])
            return 1;
    }
    return 1;
}

static long Ipc_AllocHandle(kProcess *proc, uint8_t type, uint8_t index)
{
    for(int i = 0; i < MAX_HANDLES; i++)
    {
        if(proc->handles[i].type == HANDLE_NONE)
        {
            proc->handles[i].type = type;
            proc->handles[i].index = index;
            return i + 1;
        }
    }
    return SVC_ERR_LIMIT;
}

static kHandle *Ipc_GetHandle(kProcess *proc, long handle, uint8_t type)
{
    if(handle < 1 || handle > MAX_HANDLES)
        return 0;
    kHandle *h = &proc->handles[handle - 1];
    return h->type == type ? h : 0;
}

static void Ipc_FreeHandle(kProcess *proc, long handle)
{
    if(handle >= 1 && handle <= MAX_HANDLES)
        proc->handles[handle - 1].type = HANDLE_NONE;
}

static uintptr_t Ipc_Translate(kProcess *proc, uintptr_t va, uint8_t write)
{
    if(va >= USERLAND_VIRT_END)
        return (uintptr_t)-1;

    uint64_t pte = MMU_GetPageEntry(KERN_PADDR_TO_VADDR(proc->pageTables), va);
    if(!pte || (write && (pte & PT_RO)))
        return (uintptr_t)-1;

    //EL0 processes may only pass memory they can touch themselves
    if(proc->mainThread->threadType == THREAD_USER && !(pte & PT_USER))
        return (uintptr_t)-1;

    return (pte & PT_OUTPUT_ADDR_MASK) | (va & (PAGE_SIZE - 1));
}

static long Ipc_CopyBetween(kProcess *src, uintptr_t srcVa, kProcess *dst, uintptr_t dstVa, size_t size)
{
    size_t done = 0;
    while(done < size)
    {
        uintptr_t s = srcVa + done;
        uintptr_t d = dstVa + done;
        uintptr_t sPa = Ipc_Translate(src, s, 0);
        uintptr_t dPa = Ipc_Translate(dst, d, 1);
        if(sPa == (uintptr_t)-1 || dPa == (uintptr_t)-1)
            return -1;

        size_t chunk = size - done;
        if(chunk > PAGE_SIZE - (s & (PAGE_SIZE - 1)))
            chunk = PAGE_SIZE - (s & (PAGE_SIZE - 1));
        if(chunk > PAGE_SIZE - (d & (PAGE_SIZE - 1)))
            chunk = PAGE_SIZE - (d & (PAGE_SIZE - 1));

        //Both sides through the kernel's linear map, so it doesn't matter whose TTBR0 is live
        memcpy((uint8_t*)KERN_PADDR_TO_VADDR(dPa), (uint8_t*)KERN_PADDR_TO_VADDR(sPa), chunk);
        done += chunk;
    }
    return done;
}

static void Ipc_FreeSession(kSession *s)
{
    if(s->port && s->serverHandle)
        Ipc_FreeHandle(s->port->owner, s->serverHandle);
    memset((uint8_t*)s, sizeof(kSession));
}

static void Ipc_Unqueue(kSession *s)
{
    kPort *port = s->port;
    kSession **link = &port->pendingHead;
    kSession *prev = 0;
    while(*link && *link != s)
    {
        prev = *link;
        link = &(*link)->nextPending;
    }
    if(!*link)
        return;

    *link = s->nextPending;
    if(port->pendingTail == s)
        port->pendingTail = prev;
    s->nextPending = 0;
}

static void Ipc_DetachSession(kSession *s)
{
    if(!s->port)
        return;

    if(s->state == SESSION_PENDING)
        Ipc_Unqueue(s);
    if(s->state != SESSION_IDLE && s->clientThread)
        kScheduler_Wake(s->clientThread, SVC_ERR_SESSION_CLOSED);

    Ipc_FreeHandle(s->port->owner, s->serverHandle);
    s->serverHandle = 0;
    s->port = 0;
    s->state = SESSION_IDLE;
    s->clientThread = 0;

    if(!s->clientOpen)
        Ipc_FreeSession(s);
}

static long Ipc_Connect(kPort *port, kProcess *client)
{
    int idx = -1;
    for(int i = 0; i < IPC_MAX_SESSIONS; i++)
        if(!sessions[i].inUse)
        {
            idx = i;
            break;
        }
    if(idx < 0)
        return SVC_ERR_LIMIT;

    long serverHandle = Ipc_AllocHandle(port->owner, HANDLE_SESSION_SERVER, idx);
    if(serverHandle < 0)
        return serverHandle;

    long clientHandle = Ipc_AllocHandle(client, HANDLE_SESSION_CLIENT, idx);
    if(clientHandle < 0)
    {
        Ipc_FreeHandle(port->owner, serverHandle);
        return clientHandle;
    }

    kSession *s = &sessions[idx];
    memset((uint8_t*)s, sizeof(kSession));
    s->inUse = 1;
    s->state = SESSION_IDLE;
    s->clientOpen = 1;
    s->port = port;
    s->client = client;
    s->serverHandle = serverHandle;
    return clientHandle;
}

static void Ipc_Deliver(kSession *s, kThread *server)
{
    IpcMessage *req = (IpcMessage*)s->clientThread->ipcBuf;
    memcpy((uint8_t*)server->ipcBuf, (uint8_t*)req, IPC_BUFFER_SIZE);

    //Remember where the client wants reply data; the reply overwrites its message
    s->clientBufCount = req->bufCount > IPC_MAX_BUFS ? IPC_MAX_BUFS : req->bufCount;
    for(int i = 0; i < s->clientBufCount; i++)
        s->clientBufs[i] = req->bufs[i];

    s->state = SESSION_IN_SERVICE;
}

static long Ipc_Reply(kProcess *proc, kThread *server, long replyTo)
{
    kHandle *h = Ipc_GetHandle(proc, replyTo, HANDLE_SESSION_SERVER);
    if(!h)
        return SVC_ERR_INVALID_HANDLE;

    kSession *s = &sessions[h->index];
    if(s->state != SESSION_IN_SERVICE)
        return SVC_ERR_INVALID_HANDLE;

    if(s->clientOpen && s->clientThread)
    {
        IpcMessage *rep = (IpcMessage*)server->ipcBuf;
        IpcMessage *msg = (IpcMessage*)s->clientThread->ipcBuf;

        uint32_t copied[IPC_MAX_BUFS];
        for(int i = 0; i < IPC_MAX_BUFS; i++)
            copied[i] = 0;
        long status = SVC_OK;
        uint8_t n = rep->bufCount < s->clientBufCount ? rep->bufCount : s->clientBufCount;
        for(int i = 0; i < n; i++)
        {
            size_t size = rep->bufs[i].size;
            if(size > s->clientBufs[i].size) size = s->clientBufs[i].size;
            if(size > IPC_MAX_COPY) size = IPC_MAX_COPY;

            long c = Ipc_CopyBetween(proc, rep->bufs[i].va, s->client, s->clientBufs[i].va, size);
            if(c < 0)
                status = SVC_ERR_INVALID_ADDR;
            else
                copied[i] = c;
        }

        memcpy((uint8_t*)msg, (uint8_t*)rep, IPC_BUFFER_SIZE);
        msg->bufCount = s->clientBufCount;
        for(int i = 0; i < s->clientBufCount; i++)
        {
            msg->bufs[i] = s->clientBufs[i];
            msg->bufs[i].size = copied[i];
        }

        kScheduler_Wake(s->clientThread, status);
    }

    s->state = SESSION_IDLE;
    s->clientThread = 0;
    if(!s->clientOpen)
        Ipc_FreeSession(s);
    return SVC_OK;
}

static kPort *Ipc_FindPort(const char *name)
{
    for(int i = 0; i < IPC_MAX_PORTS; i++)
        if(ports[i].inUse && Ipc_NameEq(ports[i].name, name))
            return &ports[i];
    return 0;
}

static void Ipc_ClosePort(kPort *port)
{
    for(int i = 0; i < IPC_MAX_SESSIONS; i++)
        if(sessions[i].inUse && sessions[i].port == port)
            Ipc_DetachSession(&sessions[i]);
    memset((uint8_t*)port, sizeof(kPort));
}

long kIpc_CreatePort(const char *name)
{
    kProcess *proc = Ipc_CurrentProcess();
    if(Ipc_FindPort(name))
        return SVC_ERR_BUSY;

    int idx = -1;
    for(int i = 0; i < IPC_MAX_PORTS; i++)
        if(!ports[i].inUse)
        {
            idx = i;
            break;
        }
    if(idx < 0)
        return SVC_ERR_LIMIT;

    long handle = Ipc_AllocHandle(proc, HANDLE_PORT, idx);
    if(handle < 0)
        return handle;

    kPort *port = &ports[idx];
    memset((uint8_t*)port, sizeof(kPort));
    port->inUse = 1;
    port->owner = proc;
    for(int i = 0; i < IPC_PORT_NAME_LEN && name[i]; i++)
        port->name[i] = name[i];
    LOG("[ipc] %s registered port %s\n", proc->name, port->name);

    //Finish connects that were blocked waiting for this name
    for(int i = 0; i < IPC_MAX_WAITERS; i++)
    {
        if(waiters[i].thread && Ipc_NameEq(waiters[i].name, port->name))
        {
            kScheduler_Wake(waiters[i].thread, Ipc_Connect(port, waiters[i].proc));
            waiters[i].thread = 0;
        }
    }

    return handle;
}

long kIpc_ConnectToPort(const char *name)
{
    kProcess *proc = Ipc_CurrentProcess();
    kPort *port = Ipc_FindPort(name);
    if(port)
        return Ipc_Connect(port, proc);

    for(int i = 0; i < IPC_MAX_WAITERS; i++)
    {
        if(!waiters[i].thread)
        {
            waiters[i].thread = Ipc_CurrentThread();
            waiters[i].proc = proc;
            for(int j = 0; j < IPC_PORT_NAME_LEN; j++)
                waiters[i].name[j] = name[j];
            kScheduler_Block();
        }
    }
    return SVC_ERR_LIMIT;
}

long kIpc_SendSyncRequest(long session)
{
    kProcess *proc = Ipc_CurrentProcess();
    kHandle *h = Ipc_GetHandle(proc, session, HANDLE_SESSION_CLIENT);
    if(!h)
        return SVC_ERR_INVALID_HANDLE;

    kSession *s = &sessions[h->index];
    if(!s->port)
        return SVC_ERR_SESSION_CLOSED;
    if(s->state != SESSION_IDLE)
        return SVC_ERR_BUSY;

    kPort *port = s->port;
    s->clientThread = Ipc_CurrentThread();
    if(port->waiter)
    {
        kThread *server = port->waiter;
        port->waiter = 0;
        Ipc_Deliver(s, server);
        kScheduler_Wake(server, s->serverHandle);
    }
    else
    {
        s->state = SESSION_PENDING;
        s->nextPending = 0;
        if(port->pendingTail)
            port->pendingTail->nextPending = s;
        else
            port->pendingHead = s;
        port->pendingTail = s;
    }

    kScheduler_Block();
}

long kIpc_ReplyAndReceive(long portHandle, long replyTo)
{
    kProcess *proc = Ipc_CurrentProcess();
    kThread *thread = Ipc_CurrentThread();
    kHandle *h = Ipc_GetHandle(proc, portHandle, HANDLE_PORT);
    if(!h)
        return SVC_ERR_INVALID_HANDLE;
    kPort *port = &ports[h->index];

    if(replyTo)
    {
        long r = Ipc_Reply(proc, thread, replyTo);
        if(r != SVC_OK)
            return r;
    }

    if(port->pendingHead)
    {
        kSession *s = port->pendingHead;
        port->pendingHead = s->nextPending;
        if(!port->pendingHead)
            port->pendingTail = 0;
        s->nextPending = 0;
        Ipc_Deliver(s, thread);
        return s->serverHandle;
    }

    if(port->waiter)
        return SVC_ERR_BUSY;

    port->waiter = thread;
    kScheduler_Block();
}

static long Ipc_CloseHandleOf(kProcess *proc, long handle)
{
    if(handle < 1 || handle > MAX_HANDLES)
        return SVC_ERR_INVALID_HANDLE;

    kHandle h = proc->handles[handle - 1];
    switch(h.type)
    {
        case HANDLE_PORT:
            Ipc_ClosePort(&ports[h.index]);
            break;

        case HANDLE_SESSION_CLIENT:
        {
            kSession *s = &sessions[h.index];
            s->clientOpen = 0;
            if(s->state == SESSION_PENDING)
            {
                Ipc_Unqueue(s);
                s->state = SESSION_IDLE;
            }
            if(s->state == SESSION_IDLE)
                Ipc_FreeSession(s);
            break;
        }

        case HANDLE_SESSION_SERVER:
            Ipc_DetachSession(&sessions[h.index]);
            break;

        default:
            return SVC_ERR_INVALID_HANDLE;
    }

    Ipc_FreeHandle(proc, handle);
    return SVC_OK;
}

long kIpc_CloseHandle(long handle)
{
    return Ipc_CloseHandleOf(Ipc_CurrentProcess(), handle);
}

void kIpc_ProcessExit(kProcess *proc)
{
    for(int i = 0; i < MAX_HANDLES; i++)
        if(proc->handles[i].type != HANDLE_NONE)
            Ipc_CloseHandleOf(proc, i + 1);

    for(int i = 0; i < IPC_MAX_WAITERS; i++)
        if(waiters[i].proc == proc)
            waiters[i].thread = 0;
}