#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "shm_ipc.h"


#define NUM_REQS_TO_SEND 5000000

//#define THROTTLE_MSG_SENDING 100 //us


static inline void generate_message(struct shm_ipc_ringb* ringb)
{
    struct ask_bid payload = {
        .askVolume = rand(),
        .bidVolume = rand(),
        .askPrice = rand(),
        .bidPrice = rand(),
    };

    uint64_t ts = get_time_ns();

    compiler_barrier();

    struct shm_ipc_msg* msg = &ringb->buffer[ringb->tail % SHM_IPC_MSG_PER_BUFFER];

    *msg = (struct shm_ipc_msg) {
        .ts = ts,
        .payload = payload,
    };
}

static int produce_data(struct shm_ipc_header* header)
{
    if (header->magic != SHM_IPC_MAGIC) {
        header->magic = SHM_IPC_MAGIC;

        printf("Initialize new shared memory ipc structures.\n");
    }

    /* We can use multiple per-cpu buffers to increase throughput */
    struct shm_ipc_ringb* ringb = &header->ringb[0];

    srand(time(NULL));

    header->producer_online = 1;

    futex_wait(&header->consumer_online, 0);

    for (int i = 0; i < NUM_REQS_TO_SEND; i++) {
        assert(read_once(ringb->head) <= ringb->tail);

        while (ringb->tail == read_once(ringb->head) + SHM_IPC_MSG_PER_BUFFER) {
            /* The ring buffer size should be sufficient to reduce the likelihood of the buffer becoming full.
             */
            printf("The ring buffer is full! Is consumer alive?\n");

            /* TODO: Based on a specific optimization strategy, we can keep the last N actual messages
             * that can be sent as soon as the consumer comes to life.
             */
            usleep(100);
        }

        generate_message(ringb); //touch ringb->tail inside generate_message(). compiler_barrier is not required.

        /* TODO: We want to publish the tail after recording the message. To achieve this we don't
         * need to do anything on x86 arch. But other architectures require WB() barrier.
         */
        ringb->tail++;

        if (header->consumer_awaiting) {
            header->consumer_awaiting = 0;

            futex_wake(&header->consumer_awaiting, 0);
        }

#ifdef THROTTLE_MSG_SENDING
        usleep(THROTTLE_MSG_SENDING);
#endif
    }

    header->producer_online = 0;

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc == 2 && !strcmp(argv[1],"reset")) {
        printf("reset ipc link: %s\n", SHM_IPC_LINK_NAME);

        shm_unlink(SHM_IPC_LINK_NAME);
    }

    int shm = shm_open(SHM_IPC_LINK_NAME, O_CREAT | O_RDWR, 0777);
    if (shm  == 1) {
        printf("Open '%s' failed: %s\n", SHM_IPC_LINK_NAME, strerror(errno));
        return -2;
    }

    if (ftruncate(shm, SHM_SPACE_SIZE) == -1) {
        close(shm);
        printf("ftruncate(%s) failed: %s\n", SHM_IPC_LINK_NAME, strerror(errno));
        return -3;
    }

    uint8_t* addr = mmap(0, SHM_SPACE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (addr == (uint8_t*)-1) {
        close(shm);
        printf("mmap(%s) failed: %s\n", SHM_IPC_LINK_NAME, strerror(errno));
        return -4;
    }

    produce_data((struct shm_ipc_header*)addr);

    munmap(addr, SHM_SPACE_SIZE);
    close(shm);

    printf("exited\n");

    return 0;
}
