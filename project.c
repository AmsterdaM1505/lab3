#include "lib.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

typedef struct args {
    int n; //номер потока
    int* arrays[3];
    size_t M; //задается пользователем
    FILE* file_to_write;

    pthread_mutex_t mutex;
    pthread_mutex_t* write_mutex;


    pthread_cond_t wait_to_create; //conditional variables
    pthread_cond_t wait_to_insert;
    pthread_cond_t wait_to_load;

    int blocks_created;
    int blocks_inserted;
    int blocks_loaded;
} Args;

enum {
    FIRST_BLOCK_SIZE = 10,
    SECOND_BLOCK_SIZE = 50,
    THIRD_BLOCK_SIZE = 100,
    MAX_FILENAME_SIZE = 100,
};

void* Task1(void* void_args) {
    Args* args = (Args*)(void_args);
    for (size_t i = 0; i + 1 < args->M; ++i) {
        args->blocks_loaded = 0;
        args->arrays[0] = (int*)myCalloc(FIRST_BLOCK_SIZE, sizeof(int));
        args->arrays[1] = (int*)myCalloc(SECOND_BLOCK_SIZE, sizeof(int));
        args->arrays[2] = (int*)myCalloc(THIRD_BLOCK_SIZE, sizeof(int));
        pthread_mutex_lock(&(args->mutex));
        args->blocks_created = 1;
        pthread_cond_signal(&(args->wait_to_insert)); //будит все потоки в состоянии wait, при этом по коду идет только один, который может анлочить
        while(!args->blocks_loaded) {
            pthread_cond_wait(&(args->wait_to_create), &(args->mutex));
        }
        pthread_mutex_unlock(&(args->mutex));
    }
    args->arrays[0] = (int*)myCalloc(FIRST_BLOCK_SIZE, sizeof(int));
    args->arrays[1] = (int*)myCalloc(SECOND_BLOCK_SIZE, sizeof(int));
    args->arrays[2] = (int*)myCalloc(THIRD_BLOCK_SIZE, sizeof(int));
    pthread_mutex_lock(&(args->mutex));
    args->blocks_created = 1;
    pthread_cond_signal(&(args->wait_to_insert));
    pthread_mutex_unlock(&(args->mutex));
    return 0;
}

void* Task2(void* void_args) {
    Args* args = (Args*)(void_args);
    for (size_t i = 0; i < args->M; ++i) {
        pthread_mutex_lock(&(args->mutex));
        while(!args->blocks_created) {
            pthread_cond_wait(&(args->wait_to_insert), &(args->mutex));
        }
        args->blocks_created = 0;
        pthread_mutex_unlock(&(args->mutex));
        for (size_t j = 0; j < FIRST_BLOCK_SIZE; ++j) {
            args->arrays[0][j] = args->n;
        }
        for (size_t j = 0; j < SECOND_BLOCK_SIZE; ++j) {
            args->arrays[1][j] = args->n;
        }
        for (size_t j = 0; j < THIRD_BLOCK_SIZE; ++j) {
            args->arrays[2][j] = args->n;
        }
        pthread_mutex_lock(&(args->mutex));
        args->blocks_inserted = 1;
        pthread_cond_signal(&(args->wait_to_load));
        pthread_mutex_unlock(&(args->mutex));
    }
    return 0;
}

void* Task3(void* void_args) {
    Args* args = (Args*)(void_args);
    for (size_t i = 0; i < args->M; ++i) {
        pthread_mutex_lock(&(args->mutex));
        while(!args->blocks_inserted) {
            pthread_cond_wait(&(args->wait_to_load), &(args->mutex));
        }
        args->blocks_inserted = 0;
        pthread_mutex_unlock(&(args->mutex));
        pthread_mutex_lock(args->write_mutex);

        fprintf(args->file_to_write, "%p\n", args->arrays[0]);
        for (size_t j = 0; j < FIRST_BLOCK_SIZE; ++j) {
            fprintf(args->file_to_write, "%d ", args->arrays[0][j]);
        }
        fprintf(args->file_to_write, "\n");

        fprintf(args->file_to_write, "%p\n", args->arrays[1]);
        for (size_t j = 0; j < SECOND_BLOCK_SIZE; ++j) {
            fprintf(args->file_to_write, "%d ", args->arrays[1][j]);
        }
        fprintf(args->file_to_write, "\n");

        fprintf(args->file_to_write, "%p\n", args->arrays[2]);
        for (size_t j = 0; j < THIRD_BLOCK_SIZE; ++j) {
            fprintf(args->file_to_write, "%d ", args->arrays[2][j]);
        }
        fprintf(args->file_to_write, "\n");
        pthread_mutex_unlock(args->write_mutex);
        myFree(args->arrays[0]);
        myFree(args->arrays[1]);
        myFree(args->arrays[2]);
        pthread_mutex_lock(&(args->mutex));
        args->blocks_loaded = 1;
        pthread_cond_signal(&(args->wait_to_create));
        pthread_mutex_unlock(&(args->mutex));
    }
    return 0;
}

int main(){
    int N = 0;
    int M = 0;
    printf("enter N: ");
    scanf("%d", &N);
    N -= N % 3;
    printf("enter M: ");
    scanf("%d", &M);
    char mass[MAX_FILENAME_SIZE];
    printf("enter filename: ");
    scanf("%s", mass);
    FILE* file = fopen(mass, "w");
    if (!file){
        printf("file did not open");
        return 1;
    }
    Args* args = (Args*)myCalloc(N / 3, sizeof(Args));
    pthread_t* threads = (pthread_t*)myCalloc(N, sizeof(pthread_t));
    pthread_mutex_t write_mutex;
    pthread_mutex_init(&write_mutex, NULL);
    for (size_t i = 0; i < N / 3; ++i) {
        args[i].n = i;
        args[i].M = M;
        args[i].write_mutex = &write_mutex;
        pthread_mutex_init(&(args[i].mutex), NULL);
        pthread_cond_init(&(args[i].wait_to_create), NULL);
        pthread_cond_init(&(args[i].wait_to_insert), NULL);
        pthread_cond_init(&(args[i].wait_to_load), NULL);
        args[i].file_to_write = file;
        args[i].blocks_loaded = 0;
        args[i].blocks_created = 0;
        args[i].blocks_inserted = 0;
        pthread_create(threads + (i * 3), NULL, Task1, args + i);
        pthread_create(threads + (i * 3) + 1, NULL, Task2, args + i);
        pthread_create(threads + (i * 3) + 2, NULL, Task3, args + i);
    }
    for (size_t i = 0; i < N; ++i) {
        pthread_join(threads[i], NULL);
    }
    for (size_t i = 0; i < N/3; ++i) {
        pthread_mutex_destroy(&(args[i].mutex));
        pthread_cond_destroy(&(args[i].wait_to_create));
        pthread_cond_destroy(&(args[i].wait_to_insert));
        pthread_cond_destroy(&(args[i].wait_to_load));
    }
    pthread_mutex_destroy(&write_mutex);
    fclose(file);
    myFree(threads);
    myFree(args);
}
