#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

typedef pthread_t CThread;

static inline CThread* CThreadCreate(void* func, void* ctx)
{
    CThread* thrd;
    int32_t err = pthread_create((pthread_t*)&thrd, NULL, func, ctx);
    return err < 0 ? NULL : thrd;
}

static inline void* CThreadJoin(CThread* thrd)
{
    void* ret;
    int32_t err = pthread_join((pthread_t)thrd, &ret);
    assert(err >= 0);
    return ret;
}

volatile int x1 = 0, x2 = 0;
volatile int r1 = 0, r2 = 0;

volatile int flag;

static void thread1(void* ctx)
{
    while(flag == 0);
    x1 = 1;
    x2 = r1;
}

static void thread2(void* ctx)
{
    while(flag == 0);
    r1 = 1;
    //asm volatile ("mfence" ::: "memory");
    r2 = x1;
}

void main()
{
    for (int i = 0; i < 0xfffff; ++i) {
        x1 = x2 = r1 = r2 = 0;
        flag = 0;
        CThread* thrd1 = CThreadCreate(thread1, NULL);
        CThread* thrd2 = CThreadCreate(thread2, NULL);

        sleep(0.1);
        flag = 1;
        CThreadJoin(thrd1);
        CThreadJoin(thrd2);

        if (x2 != 1 && r2 != 1) {
            printf("failed: x1: %d r2: %d, itr: %d\n", x1, r2, i);
            assert(x2 == 1 || r2 == 1);
        }
    }
}
