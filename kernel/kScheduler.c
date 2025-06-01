#include "kernel/kScheduler.h"
#include "libraries/hardware/timer.h"

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

void kScheduler_schedule()
{
    kThread *next = runq_pop(&runqueue);

    //If theres a task currently, and its still good, push it back
    if (current && current->state == STATE_RUNNING)
    {
        current->state = STATE_READY;
        runq_push(&runqueue, current);
    }

    //Switch tasks
    current = next;
    if (current)
    {
        current->state = STATE_RUNNING;
        //kprintf("switching: %d\n", current->id);
        context_switch(current);
    }
}