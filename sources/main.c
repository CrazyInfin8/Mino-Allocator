#include <stdio.h>
#include <strings.h>
#include "types.h"

#include "Allocator.h"
#include "AllocatorInternals.h"

void *buffers[1024 * 32];

int main() {
    Allocator allocator;

    InitAllocator(&allocator);
    Heap *tinyHeaps[3] = {
            [0] = AddHeap(&allocator, HeapTiny, 0),
            [1] = AddHeap(&allocator, HeapTiny, 0),
            [2] = AddHeap(&allocator, HeapTiny, 0),
    };

    int blockCount = 0;
    while (true) {
        Block *block = GetFreeBlockFromHeap(tinyHeaps[0], TinyBlockSize(&allocator));
        if (block == nil) break;
        block = SplitBlock(block, TinyBlockSize(&allocator));
        blockCount++;
        printf("Allocated buffer %d in Heap 0 at %p\n", blockCount, block);
    }
    

    for( int i = 0; i < len(buffers); i++) {
        void* buffer = AllocateBuffer(&allocator, len("Hello, World!"));
        buffers[i] = buffer;
        strcpy(buffer, "Hello, World!");
        printf("Allocated buffer %d at %p\n", i, buffer);
    }

    for (int i = 0; i < len(buffers); i++) {
        printf("Freeing buffer %d at %p\n", i, buffers[i]);
        FreePointer(&allocator, buffers[i]);
    }

    Block *block = tinyHeaps[0]->blocks;
//    while (true) {
//        if (block == nil) break;
//
//        printf("Freeing block at %p\n", block);
//        FreeBlock(&allocator, block);
//
//        block = block->next;
//    }

    return 0;
}
