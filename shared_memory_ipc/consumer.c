#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include "shm_ipc.h"


#define POLL_ITR_NUM 1024

static inline void read_message(uint64_t latency[], struct shm_ipc_ringb* ringb, uint64_t idx)
{
    struct shm_ipc_msg* msg = &ringb->buffer[idx % SHM_IPC_MSG_PER_BUFFER];

    //simulate data access
    if (unlikely(msg->payload.askVolume + msg->payload.bidVolume +
                 msg->payload.askPrice + msg->payload.bidPrice == 0)) {
        printf("Incredibly rare event!\n");
        _exit(0);
    }

    latency[idx & (SHM_IPC_LATENCY_ENTRIES_PER_BUFFER - 1)] = get_time_ns() - msg->ts;
}

static void consume_data(struct shm_ipc_header* header)
{
    if (header->magic != SHM_IPC_MAGIC) {
        printf("Invalid ipc magic number: %x\n", header->magic);
        return;
    }

    /* We can use multiple per-cpu buffers to increase throughput */
    struct shm_ipc_ringb* ringb = &header->ringb[0];

    struct sched_param param = {
        .sched_priority = 1,
    };
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param))
        printf("Can't change scheduling priority: %s\n", strerror(errno));

    header->consumer_online = 1;
    futex_wake(&header->consumer_online, 1);

    while (likely(header->producer_online)) {
        uint64_t available = read_once(ringb->tail) - ringb->head;

        int loops = POLL_ITR_NUM;
        while (likely(!available)) {

            /* Adaptive polling. Wakeup will incur latency costs for certain messages,
             * but it will also relieve the cpu.
             */
            if (unlikely(!loops--)) {
                header->consumer_awaiting = 1;

                memory_barrier();

                available = read_once(ringb->tail) - ringb->head;
                if (unlikely(available)) {
                    header->consumer_awaiting = 0;
                    break;
                }

                futex_wait(&header->consumer_awaiting, 1);

                loops = POLL_ITR_NUM;
            }
            compiler_barrier();

            available = read_once(ringb->tail) - ringb->head;
        }

        for (int i = 0; i < available; i++)
            read_message(header->latency, ringb, ringb->head + i);

        ringb->head += available;
    }

    header->consumer_online = 0;
}

int main()
{
    int shm = shm_open(SHM_IPC_LINK_NAME, O_RDWR, 0777);
    if (shm == 1) {
        printf("Open '%s' failed: %s\n", SHM_IPC_LINK_NAME, strerror(errno));
        return -2;
    }

    uint8_t* addr = mmap(0, SHM_SPACE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (addr == (uint8_t*)-1) {
        close(shm);
        printf("mmap(%s) failed: %s\n", SHM_IPC_LINK_NAME, strerror(errno));
        return -4;
    }

    consume_data((struct shm_ipc_header*)addr);

    munmap(addr, SHM_SPACE_SIZE);
    close(shm);

    printf("exited\n");

    return 0;
}
