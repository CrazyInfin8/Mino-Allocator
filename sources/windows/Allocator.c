#include "Allocator.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

size PageSize() {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    return systemInfo.dwPageSize;
}

void *requestMemory(size size) {
    return VirtualAlloc(nil, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

bool releaseMemory(void *ptr, size size) {
    unused(size);
    return VirtualFree(ptr, 0, MEM_RELEASE) != false;
}
