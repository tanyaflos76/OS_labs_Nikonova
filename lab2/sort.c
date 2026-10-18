#include "sort.h"
#include <stdio.h>
#include <stdlib.h>

void *sort_thread(void *arg)
{
    Sort_arg *arguments = arg;

    parallel_sort(arguments->data, arguments->left, arguments->right, arguments->control);
    return NULL;
}

void quick_sort(int data[], int left, int right)
{
    if (left >= right)
    {
        return;
    }

    int pivot = data[left + (right - left) / 2];
    int i = left;
    int j = right;

    while (i <= j)
    {
        while (data[i] < pivot)
        {
            i++;
        }
        while (data[j] > pivot)
        {
            j--;
        }
        if (i <= j)
        {
            int temp = data[i];
            data[i] = data[j];
            data[j] = temp;

            i++;
            j--;
        }
    }
    if (left < j)
    {
        quick_sort(data, left, j);
    }
    if (right > i)
    {
        quick_sort(data, i, right);
    }
}

void parallel_sort(int data[], int left, int right, Control_threads *control)
{
    if (left >= right)
    {
        return;
    }

    int pivot = data[left + (right - left) / 2];
    int i = left;
    int j = right;

    while (i <= j)
    {
        while (data[i] < pivot)
        {
            i++;
        }
        while (data[j] > pivot)
        {
            j--;
        }
        if (i <= j)
        {
            int temp = data[i];
            data[i] = data[j];
            data[j] = temp;

            i++;
            j--;
        }
    }
    int has_left_part = left < j;
    int has_right_part = right > i;

    if (!has_left_part && !has_right_part)
    {
        return;
    }

    if (has_left_part && has_right_part)
    {
        int can_create_new_thread = 0;
        pthread_mutex_lock(&control->mutex);

        if (control->active_threads < control->mx_threads)
        {
            control->active_threads++;
            can_create_new_thread = 1;
        }

        pthread_mutex_unlock(&control->mutex);

        if (can_create_new_thread)
        {
            pthread_t thread;
            Sort_arg task = {.data = data, .left = left, .right = j, .control = control};
            int error = pthread_create(&thread, NULL, sort_thread, &task);

            if (error == 0)
            {
                parallel_sort(data, i, right, control);
                error = pthread_join(thread, NULL);

                if (error != 0)
                {
                    fprintf(stderr, "pthread_join failed\n");
                    exit(EXIT_FAILURE);
                }
                pthread_mutex_lock(&control->mutex);
                control->active_threads--;
                pthread_mutex_unlock(&control->mutex);
            }
            else
            {
                pthread_mutex_lock(&control->mutex);
                control->active_threads--;
                pthread_mutex_unlock(&control->mutex);

                quick_sort(data, left, j);
                quick_sort(data, i, right);
            }
        }
        else
        {
            quick_sort(data, left, j);
            quick_sort(data, i, right);
        }
    }
    else if (has_left_part)
    {
        parallel_sort(data, left, j, control);
    }
    else
    {
        parallel_sort(data, i, right, control);
    }
}