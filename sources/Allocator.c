#include "types.h"
#include "Allocator.h"
#include "AllocatorInternals.h"

void InitAllocator(Allocator *allocator) {
    *allocator = (Allocator) {
            .pageSize = PageSize(),
            .tinyScale = 8,
            .tinyCount = 128,
            .smallScale = 64,
            .smallCount = 128,
    };
}

size TinyHeapSize(const Allocator *allocator) { return allocator->pageSize * allocator->tinyScale; }

size SmallHeapSize(const Allocator *allocator) { return allocator->pageSize * allocator->smallScale; }

size TinyBlockSize(const Allocator *allocator) {
    return allocator->pageSize * allocator->tinyScale / allocator->tinyCount;
}

size SmallBlockSize(const Allocator *allocator) {
    return allocator->pageSize * allocator->smallScale / allocator->smallCount;
}

Heap *AddHeap(Allocator *allocator, HeapType heapType, size bufferSize) {
    Heap **targetHeap;
    if (heapType == HeapTiny) {
        bufferSize = TinyHeapSize(allocator);
        targetHeap = &allocator->tinyHeap;
    } else if (heapType == HeapSmall) {
        bufferSize = SmallHeapSize(allocator);
        targetHeap = &allocator->smallHeap;
    } else targetHeap = &allocator->largeHeap;

    void *buffer = requestMemory(bufferSize);
    if (buffer == nil) return nil;

    Heap *newHeap = (Heap *) buffer;

    *newHeap = (Heap) {
            .size = bufferSize,
            .blocks = buffer + sizeof(struct Heap),
    };

    *newHeap->blocks = (Block) {
            .free = true,
            .size = bufferSize - sizeof(Heap),
    };

    if (*targetHeap != nil) {
        newHeap->next = *targetHeap;
        (*targetHeap)->prev = newHeap;
    }
    *targetHeap = newHeap;

    return newHeap;
}

void RemoveHeap(Allocator *allocator, Heap *heap) {
    if (heap->prev != nil) {
        heap->prev->next = heap->next;
    } else {
        Heap **targetHeap;
        if (allocator->tinyHeap == heap) targetHeap = &allocator->tinyHeap;
        else if (allocator->smallHeap == heap) targetHeap = &allocator->smallHeap;
        else if (allocator->largeHeap == heap) targetHeap = &allocator->largeHeap;
        else
            panic("heap is detached from this allocator");

        *targetHeap = heap->next;
    }

    if (heap->next != nil) {
        heap->next->prev = heap->prev;
    }

    if (releaseMemory(heap, heap->size) == false)
        panic("Unable to release memory");
}

Block *GetFreeBlockFromHeap(const Heap *heap, size minSize) {
    Block *block = heap->blocks;
    while (block != nil) {
        if (block->size > minSize && block->free) return block;
        block = block->next;
    }
    return nil;
}

Block *SplitBlock(Block *block, size minSize) {
    minSize += sizeof(Block);
    block->size -= minSize;
    Block *newBlock = (Block *) ((byte *) block + block->size);
    *newBlock = (Block) {
            .size = minSize,
            .free = false,
            .next = block->next,
            .prev = block,
    };
    if (block->next != nil) block->next->prev = newBlock;
    block->next = newBlock;
    return newBlock;
}

Heap *FindBlockHeap(const Allocator *allocator, const Block *block) {
    if (block->size <= TinyBlockSize(allocator) + sizeof(Block)) {
        for (Heap *heap = allocator->tinyHeap; heap != nil; heap = heap->next) {
            if ((byte *) block >= (byte *) heap->blocks && (byte *) block < (byte *) heap + heap->size) {
                return heap;
            }
        }
    } else if (block->size <= SmallBlockSize(allocator) + sizeof(Block)) {
        for (Heap *heap = allocator->smallHeap; heap != nil; heap = heap->next) {
            if ((byte *) block >= (byte *) heap->blocks && (byte *) block < (byte *) heap + heap->size) {
                return heap;
            }
        }
    } else {
        for (Heap *heap = allocator->largeHeap; heap != nil; heap = heap->next) {
            if ((byte *) block >= (byte *) heap->blocks && (byte *) block < (byte *) heap + heap->size) {
                return heap;
            }
        }
    }
    return nil;
}

void FreeBlock(Allocator *allocator, Block *block) {
    Heap *heap = FindBlockHeap(allocator, block);
    if (heap == nil) panic("Block is not part of allocator's heaps");

    // Merge with previous block if it is free
    block->free = true;
    if (block->prev != nil && block->prev->free) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next != nil) block->next->prev = block->prev;
        block = block->prev;
    }

    // Merge with next block if it is free
    if (block->next != nil && block->next->free) {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next != nil) block->next->prev = block;
    }

    // If the block is the only block in the heap, free the heap
    if (block->prev == nil && block->next == nil) {
        printf("Freeing entire heap at %p\n", heap);
        RemoveHeap(allocator, heap);
    }
}

Block *BlockFromPointer(const void *ptr) {
    return (Block *) ((byte *) ptr - sizeof(Block));
}

void FreePointer(Allocator *allocator, void *ptr) {
    FreeBlock(allocator, BlockFromPointer(ptr));
}

void *AllocateBuffer(Allocator *allocator, size size) {
    Heap *targetHeap;
    HeapType heapType;
    if (size <= TinyBlockSize(allocator)) {
        targetHeap = allocator->tinyHeap;
        heapType = HeapTiny;
    } else if (size <= SmallBlockSize(allocator)) {
        targetHeap = allocator->smallHeap;
        heapType = HeapSmall;
    } else {
        targetHeap = AddHeap(allocator, HeapLarge, size + sizeof(Heap));
        if (targetHeap == nil) return nil;
        Block *block = targetHeap->blocks;
        block->free = false;
        return block + 1;
    }
    size += sizeof(Block);

    if (targetHeap == nil) {
        targetHeap = AddHeap(allocator, heapType, 0);
        if (targetHeap == nil) return nil;
    }

    Block *block;
    do {
        block = GetFreeBlockFromHeap(targetHeap, size);
        if (block != nil) break;
        targetHeap = targetHeap->next;
    } while (targetHeap != nil);

    if (block == nil) {
        targetHeap = AddHeap(allocator, heapType, 0);
        if (targetHeap == nil) return nil;
        block = targetHeap->blocks;
    }

    if (block->size - size > sizeof(Block)) {
        block = SplitBlock(block, size);
    }
    block->free = false;
    return block + 1;
}
