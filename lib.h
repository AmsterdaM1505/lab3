#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void* myMalloc(size_t memory);
void* myCalloc(size_t memory, size_t size);
void* myFree(void* pointer);
