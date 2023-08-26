#include "stdio.h"
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <vector>
#include <algorithm>

#include "shm_ipc.h"


#define MIN_STATS_SAMPLE_SIZE 1000

static void show_latency(struct shm_ipc_header* header)
{
    if (header->magic != SHM_IPC_MAGIC) {
        printf("Invalid ipc magic number: %x\n", header->magic);
        return;
    }

    std::vector<uint64_t> latency_list;

    for (int i = 0; i < SHM_IPC_LATENCY_ENTRIES_PER_BUFFER; i++) {
        uint64_t latency_value = header->latency[i];

        if (!latency_value)
            continue;

        latency_list.push_back(latency_value);
    }

    if (latency_list.size() < MIN_STATS_SAMPLE_SIZE) {
        printf("The collected data doesn't contain enough requests to show meaningful statistics.\n");
        return;
    }

    std::sort(latency_list.begin(), latency_list.end());

    printf("Requests latency:\n\tmin: %lu ns, max: %lu ns\n", latency_list.front(), latency_list.back());

    uint64_t percl90 = ((latency_list.size() * 90) / 100);
    uint64_t percl95 = ((latency_list.size() * 95) / 100);
    uint64_t percl99 = ((latency_list.size() * 99) / 100);

    printf("Percentiles:\n\t");
    printf("90th: %llu ns, 95th: %llu ns, 99th: %llu ns\n", latency_list[percl90], latency_list[percl95], latency_list[percl99]);
}

int main()
{
    int shm = shm_open(SHM_IPC_LINK_NAME, O_RDWR, 0777);
    if (shm == 1) {
        printf("Open '%s' failed: %s\n", SHM_IPC_LINK_NAME, strerror(errno));
        return -2;
    }

    void* addr = mmap(nullptr, SHM_SPACE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (addr == MAP_FAILED) {
        close(shm);
        printf("mmap(%s) failed: %s\n", SHM_IPC_LINK_NAME, strerror(errno));
        return -4;
    }

    show_latency(reinterpret_cast<struct shm_ipc_header*>(addr));

    munmap(addr, SHM_SPACE_SIZE);
    close(shm);

    return 0;
}
