#pragma once

#include "libraries/types.h"

//Every thread gets one IPC message page mapped here; requests and replies are copied between them
#define IPC_BUFFER_VA       0x000000000F000000ULL
#define IPC_BUFFER_SIZE     0x100

#define IPC_PORT_NAME_LEN   8
#define IPC_MAX_BUFS        2
#define IPC_MAX_COPY        0x10000

typedef struct
{
    uint64_t va;
    uint32_t size;
    uint32_t _pad;
} IpcBufDesc;

typedef struct
{
    uint32_t cmd;
    uint16_t dataSize;
    uint8_t bufCount;
    uint8_t _pad;
    int32_t copyHandle; //requests only; the kernel duplicates it into the server's handle table
    uint32_t _pad2;
    IpcBufDesc bufs[IPC_MAX_BUFS];
    uint8_t data[IPC_BUFFER_SIZE - 16 - IPC_MAX_BUFS * sizeof(IpcBufDesc)];
} IpcMessage;

static inline IpcMessage *Ipc_GetMessage()
{
    return (IpcMessage*)IPC_BUFFER_VA;
}
