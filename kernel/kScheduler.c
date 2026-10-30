#include "kernel/kScheduler.h"
#include "kernel/kProcess.h"
#include "libraries/hardware/timer.h"
#include "kernel/kProcessManager.h"
#include "libraries/hardware/irq.h"
#include "libraries/hardware/gic.h"
#include "libraries/libc/string.h"

static run_queue_t runqueue;
kThread *current = 0;

void runq_push(run_queue_t *rq, kThread *t) {
    if (rq->size < MAX_THREADS) {
        rq->queue[rq->back] = t;
        rq->back = (rq->back + 1) % MAX_THREADS;
        rq->size++;
    }
}

kThread *runq_pop(run_queue_t *rq) {
    if (rq->size == 0) 
        return 0;
    kThread *t = rq->queue[rq->front];
    rq->front = (rq->front + 1) % MAX_THREADS;
    rq->size--;
    return t;
}

void kScheduler_Init()
{
    localTimerIrqInit();
    localTimerIrqReset();
    LOGT("Initialized kScheduler\n");
    schedulerInit = 1;
    //Unmask irq
    enable_irq();
}

void kScheduler_AddThread(kThread *thread)
{
    runq_push(&runqueue, thread);
}

kThread *kScheduler_GetCurrentThread()
{
    return current;
}

//svc entry records the trap frame so a blocking svc can be resumed from it
void kScheduler_SaveTrapFrame(uintptr_t sp)
{
    if(current)
        current->kern_sp = sp;
}

//Result a blocked/yielding thread sees in x0 once it's resumed (x0 sits at the bottom of the frame)
void kScheduler_SetReturn(kThread *thread, uint64_t value)
{
    *(uint64_t*)thread->kern_sp = value;
}

void kScheduler_Block()
{
    current->state = STATE_WAITING;
    kScheduler_schedule();
    __builtin_unreachable();
}

void kScheduler_Wake(kThread *thread, uint64_t retval)
{
    kScheduler_SetReturn(thread, retval);
    thread->state = STATE_READY;
    runq_push(&runqueue, thread);
}

void kScheduler_Yield()
{
    kScheduler_SetReturn(current, 0);
    kScheduler_schedule();
    __builtin_unreachable();
}

void kScheduler_schedule()
{
    if(!schedulerInit)
        return;

    kThread *prev = current;
    if(current)
    {
        if(current->state == STATE_TERMINATED)
        {
            LOG("relinquishing thread [id:%d]\n", current->id);
            kThread_Destroy(current);
            prev = 0;
        }
        else if(current->state != STATE_WAITING)
        {
            //Push before popping so a lone runnable thread keeps running
            current->state = STATE_READY;
            runq_push(&runqueue, current);
        }
    }

    //Threads killed while queued get cleaned up here
    kThread *next;
    while((next = runq_pop(&runqueue)) && next->state == STATE_TERMINATED)
    {
        LOG("relinquishing thread [id:%d]\n", next->id);
        kThread_Destroy(next);
    }
    //Only threads wake threads (no IRQ-driven wakeups yet), so an empty queue means
    //everything has exited or is blocked for good
    if(!next)
    {
        LOGT("No runnable threads left\n");
        disable_irq();
        panic();
    }

    uint8_t isNew = !next->started;
    uint8_t isKernel = next->threadType == THREAD_KERNEL;
    kProcess *parent = (kProcess*)(next->parent);
    if(next != current)
        LOG("switching: %s @ 0x%X [proc is %s and is %s]\n", parent->name, next->entryPtr, isNew ? "new" : "old", isKernel ? "kernel" : "user");

    if(next != prev)
    {
        if(prev)
            kFpu_Save(&prev->fpu);
        kFpu_Restore(&next->fpu);
    }

    current = next;
    current->started = 1;
    current->state = STATE_RUNNING;
    localTimerIrqReset();
    context_switch(current->entryPtr, current->sp, current->kern_sp, parent->pageTables, isNew, isKernel);
}