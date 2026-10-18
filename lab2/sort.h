
#ifndef SORT_H
#define SORT_H

#include <pthread.h>

typedef struct
{
    int mx_threads;
    int active_threads;
    pthread_mutex_t mutex;
} Control_threads;

typedef struct
{
    int *data;
    int left;
    int right;
    Control_threads *control;
} Sort_arg;

void quick_sort(int data[], int left, int right);

void parallel_sort(int data[], int left, int right, Control_threads *control);

void *sort_thread(void *arg);

#endif
