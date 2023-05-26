#include "lib.h"
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void* myMalloc(size_t memory) {
    int fd = open("/dev/zero", O_RDWR);
    char* data = mmap(0, memory + sizeof(int), PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    (*(int*)(data)) = memory;
    return (int*)(data) + 1;
}

void* myCalloc(size_t memory, size_t size) {
    int fd = open("/dev/zero", O_RDWR);
    char* data = mmap(0, memory * size + sizeof(int), PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    (*(int*)(data)) = memory * size;
    return (int*)(data) + 1;
}

void* myFree(void* pointer) {
    int* data = (int*)pointer - 1;
    munmap(data, *data + sizeof(int));
}

// проверка на отрицательное число
