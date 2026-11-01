#include "kernel/kScheduler.h"
#include "kernel/kProcess.h"
#include "libraries/hardware/timer.h"
#include "kernel/kProcessManager.h"
#include "libraries/hardware/irq.h"
#include "libraries/hardware/gic.h"
#include "libraries/libc/string.h"
#include "libraries/libc/time.h"

#define MAX_TIMER_TICKS 0x7FFFFFFF

static run_queue_t runqueue;
static kThread *sleepers[MAX_THREADS];
static int sleeperCount = 0;
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

void kScheduler_Sleep(uint64_t ticks)
{
    kScheduler_SetReturn(current, 0);
    current->wakeTime = Counter() + ticks;
    current->state = STATE_WAITING;
    sleepers[sleeperCount++] = current;
    kScheduler_schedule();
    __builtin_unreachable();
}

static void WakeSleepers()
{
    uint64_t now = Counter();
    for(int i = 0; i < sleeperCount; )
    {
        kThread *t = sleepers[i];
        if(now < t->wakeTime)
        {
            i++;
            continue;
        }

        sleepers[i] = sleepers[--sleeperCount];
        if(t->state == STATE_WAITING)
            t->state = STATE_READY;
        runq_push(&runqueue, t);
    }
}

static uint64_t TicksUntilWake(uint64_t limit)
{
    uint64_t now = Counter();
    for(int i = 0; i < sleeperCount; i++)
    {
        uint64_t wake = sleepers[i]->wakeTime;
        uint64_t left = wake > now ? wake - now : 1;
        if(left < limit)
            limit = left;
    }
    return limit;
}

static void IdleUntilWake()
{
    localTimerIrqSet(TicksUntilWake(MAX_TIMER_TICKS));
    asm volatile("dsb sy");
    asm volatile("wfi");
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

    kThread *next;
    while(1)
    {
        WakeSleepers();

        while((next = runq_pop(&runqueue)) && next->state == STATE_TERMINATED)
        {
            LOG("relinquishing thread [id:%d]\n", next->id);
            kThread_Destroy(next);
        }
        if(next)
            break;

        if(sleeperCount == 0)
        {
            LOGT("No runnable threads left\n");
            disable_irq();
            panic();
        }
        IdleUntilWake();
    }

    uint8_t isNew = !next->started;
    uint8_t isKernel = next->threadType == THREAD_KERNEL;
    kProcess *parent = (kProcess*)(next->parent);
    if(next != prev)
    {
        if(prev)
            kFpu_Save(&prev->fpu);
        kFpu_Restore(&next->fpu);
    }

    current = next;
    current->started = 1;
    current->state = STATE_RUNNING;
    localTimerIrqSet(TicksUntilWake(localTimerTicks()));
    context_switch(current->entryPtr, current->sp, current->kern_sp, parent->pageTables, isNew, isKernel);
}