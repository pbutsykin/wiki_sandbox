#ifndef __SHM_IPC
#define __SHM_IPC

#include <stdint.h>
#include <time.h>

#define compiler_barrier() asm volatile("": : :"memory")

#define cpu_relax() __asm__ volatile("pause\n": : :"memory")

#define read_once(var) (*((volatile typeof(var) *)(&(var))))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define SHM_IPC_LINK_NAME "SHM_IPC"

#define SHM_SPACE_SIZE (64 << 20) //64MB

#define SHM_IPC_MAGIC 0x10771377

#define SHM_IPC_RING_BUFFER_SIZE (512 << 10) //512kb
#define SHM_IPC_MSG_PER_BUFFER   (SHM_IPC_RING_BUFFER_SIZE / sizeof(struct shm_ipc_msg))

#define SHM_IPC_LATENCY_BUFFER_SIZE (1 << 20) //1mb
#define SHM_IPC_LATENCY_ENTRIES_PER_BUFFER (SHM_IPC_LATENCY_BUFFER_SIZE / sizeof(uint64_t))


struct shm_ipc_msg {
    uint64_t ts;
    struct ask_bid {
        int64_t askVolume;
        int64_t bidVolume;
        double askPrice;
        double bidPrice;
    } payload;
};

#ifndef __cplusplus
_Static_assert(sizeof(struct shm_ipc_msg) == 40, "struct shm_ipc_msg invalid size");
#endif

struct shm_ipc_ringb {
    uint64_t head, tail;
    struct shm_ipc_msg buffer[SHM_IPC_MSG_PER_BUFFER];
};

struct shm_ipc_header {
    uint32_t magic;
    uint16_t producer_online, consumer_online;
    struct shm_ipc_ringb ringb[1];
    uint64_t latency[SHM_IPC_LATENCY_ENTRIES_PER_BUFFER];
};

/* TODO: invariant tsc is more accurate, but it's uconvenient to compare
 * the result in cycles between different hardware.
 */
static inline uint64_t get_time_ns()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (now.tv_sec * 1000000000UL) + now.tv_nsec;
}

#endif //__SHM_IPC
