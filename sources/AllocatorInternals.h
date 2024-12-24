#ifndef ALLOCATORINTERNALS_H
#define ALLOCATORINTERNALS_H

#include "types.h"
#include "Allocator.h"

typedef enum HeapType {
    HeapTiny = 1,
    HeapSmall = 2,
    HeapLarge = 0,
} HeapType;

void *requestMemory(size bufferSize);
bool releaseMemory(void *ptr, size size);

struct Allocator {
    // pageSize is the size that pages align and conform to.
    size pageSize;
    // tinyScale determines how much memory to allocate for tiny heaps. The
    // actual size of the heap should be [pageSize * tinyScale].
    size tinyScale;
    // tinyCount determines approximately how many blocks should fit inside a
    // tiny heap. It sets the maximum size of each block to
    // [pageSize * tinyScale / tinyCount].
    size tinyCount;
    // smallScale determines how much memory to allocate for small heaps. The
    // actual size of the heap should be [pageSize * smallScale].
    size smallScale;
    // smallCount determines approximately how many blocks should fit inside a
    // small heap. It sets the maximum size of each block to
    // [pageSize * smallScale / smallCount].
    size smallCount;

    Heap *tinyHeap, *smallHeap, *largeHeap;
    Heap *lastTinyHeap, *lastSmallHeap, *lastLargeHeap;
};

struct Heap {
    Heap *prev, *next;
    size size;
    Block *blocks;
};

struct Block {
    bool free;
    size size;
    Block *prev, *next;
};

Heap *AddHeap(Allocator *allocator, HeapType heapType, size bufferSize);
void RemoveHeap(Allocator *allocator, Heap *heap);
Block *GetFreeBlockFromHeap(const Heap *heap, size minSize);
Block *SplitBlock(Block *block, size minSize);
Heap* FindBlockHeap(const Allocator *allocator, const Block *block);
void FreeBlock(Allocator* allocator, Block *block);
Block* BlockFromPointer(const void *ptr);
void FreePointer(Allocator *allocator, void *ptr);
void *AllocateBuffer(Allocator *allocator, size size);


#endif //ALLOCATORINTERNALS_H
