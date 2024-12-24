#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "types.h"

typedef struct Allocator Allocator;
typedef struct Heap Heap;
typedef struct Block Block;

void InitAllocator(Allocator *allocator);

size PageSize();
size TinyHeapSize(const Allocator *allocator);
size SmallHeapSize(const Allocator *allocator);
size TinyBlockSize(const Allocator *allocator);
size SmallBlockSize(const Allocator *allocator);

#endif //ALLOCATOR_H
