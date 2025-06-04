#include "kernel/kScheduler.h"
#include "kernel/kProcess.h"
#include "libraries/hardware/timer.h"
#include "kernel/kProcessManager.h"
#include "libraries/hardware/irq.h"

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
    kprintf("Initialized kScheduler\n");
}

void kScheduler_AddThread(kThread *thread)
{
    runq_push(&runqueue, thread);
}

kThread *GetCurrentThread()
{
    return current;
}

void kScheduler_schedule()
{
    kThread *next = runq_pop(&runqueue);

    //If theres a task currently, and its still good, push it back
    if (current && current->state != STATE_TERMINATED)
    {
        runq_push(&runqueue, current);
    }

    //Switch tasks
    current = next;
    if (current)
    {
        uint8_t isNew = current->state == STATE_READY;
        uint8_t isKernel = current->threadType == THREAD_KERNEL;
        kProcess *parent = (kProcess*)(current->parent);
        kprintf("switching: %s\n", parent->name);
        context_switch(current->sp, isKernel, isNew, current->entryPtr, parent->pageTables);
    }
    else 
    {
        localTimerIrqReset();
    }
}