#include "kernel/kProcess.h"

void kProcess_Initialize(kProcess *proc)
{
    proc->mainThread = kThread_Create();
}

void kProcess_Start(kProcess *proc)
{
    //TODO
}