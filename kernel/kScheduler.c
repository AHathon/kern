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
    schedulerInit = 1;
    LOGT("Initialized kScheduler\n");
}

void kScheduler_AddThread(kThread *thread)
{
    runq_push(&runqueue, thread);
}

kThread *kScheduler_GetCurrentThread()
{
    return current;
}

void kScheduler_schedule()
{
    ASSERT(runqueue.size > 0);

    kThread *next = runq_pop(&runqueue);

    if(schedulerInit)
    {
        //If theres a task currently, and its still good, push it back
        if (current && current->state != STATE_TERMINATED)
        {
            kProcess *parent = (kProcess*)(current->parent);
            LOG("pushing: %s\n", parent->name);
            current->state = STATE_RUNNING;
            runq_push(&runqueue, current);
        }
        if(current && current->state == STATE_TERMINATED)
        {
            //Cleanup terminated threads
            LOG("relinquishing thread [id:%d]\n", current->id);
            kThread_Destroy(current);
        }

        //Switch tasks
        current = next;
        if (current && current->state != STATE_TERMINATED)
        {
            uint8_t isNew = current->state == STATE_READY;
            uint8_t isKernel = current->threadType == THREAD_KERNEL;
            kProcess *parent = (kProcess*)(current->parent);
            LOG("switching: %s @ %X [proc is %s and is %s]\n", parent->name, current->entryPtr, isNew ? "new" : "old", isKernel ? "kernel" : "user");
            context_switch(current->sp, current->kern_sp, isKernel, isNew, current->entryPtr, parent->pageTables);
        }
    }
}