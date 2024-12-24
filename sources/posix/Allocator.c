#include "Allocator.h"
#include "types.h"

#include <sys/mman.h>
#include <unistd.h>

size PageSize() {
    return sysconf(_SC_PAGESIZE);
}

void *requestMemory(size bufferSize) {
    return mmap(nil, bufferSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
}

bool releaseMemory(void *ptr, size bufferSize) {
    return munmap(ptr, bufferSize) == 0;
}
