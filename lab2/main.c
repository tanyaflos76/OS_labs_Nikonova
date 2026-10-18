#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "sort.h"

int read_lst(const char *filename, int **data, int *size)
{
  FILE *file = fopen(filename, "r");

  if (file == NULL)
  {
    return -1;
  }

  int capacity = 16;
  int count = 0;
  int *lst = malloc(capacity * sizeof(int));

  if (lst == NULL)
  {
    fclose(file);
    return -1;
  }

  int value;

  while (fscanf(file, "%d", &value) == 1)
  {
    if (count == capacity)
    {
      capacity *= 2;
      int *temp = realloc(lst, capacity * sizeof(int));
      if (temp == NULL)
      {
        free(lst);
        fclose(file);
        return -1;
      }
      lst = temp;
    }
    lst[count] = value;
    count++;
  }

  if (!feof(file))
  {
    free(lst);
    fclose(file);
    return -1;
  }
  fclose(file);
  *data = lst;
  *size = count;

  return 0;
}

int write_lst(const char *filename, const int data[], int size)
{
  FILE *file = fopen(filename, "w");

  if (file == NULL)
  {
    return -1;
  }
  for (int i = 0; i < size; i++)
  {
    if (fprintf(file, "%d ", data[i]) < 0)
    {
      fclose(file);
      return -1;
    }
  }
  if (fprintf(file, "\n") < 0)
  {
    fclose(file);
    return -1;
  }
  if (fclose(file) == EOF)
  {
    return -1;
  }
  return 0;
}

double get_elapsed(struct timespec start, struct timespec end)
{
  return (end.tv_sec - start.tv_sec) * 1000.0 +
         (end.tv_nsec - start.tv_nsec) / 1e6;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Error: incorrect number of arguments\n");
    return EXIT_FAILURE;
  }

  char *endptr;
  errno = 0;
  long max_threads = strtol(argv[1], &endptr, 10);

  if (errno == ERANGE ||
      argv[1][0] == '\0' ||
      *endptr != '\0' ||
      max_threads < 1 ||
      max_threads > 2147483647L)
  {
    fprintf(stderr, "Error: max_threads must be a positive integer\n");
    return EXIT_FAILURE;
  }

  int *data = NULL;
  int size = 0;

  if (read_lst("input.txt", &data, &size) == -1)
  {
    fprintf(stderr, "Error: cannot read input.txt\n");
    return EXIT_FAILURE;
  }

  int *sequance_data = malloc((size > 0 ? size : 1) * sizeof(int));
  if (sequance_data == NULL)
  {
    fprintf(stderr, "Error: memory allocation failed\n");
    free(data);
    return EXIT_FAILURE;
  }
  for (int i = 0; i < size; i++)
  {
    sequance_data[i] = data[i];
  }

  struct timespec start, end;

  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
  {
    perror("clock_gettime");
    free(sequance_data);
    free(data);
    return EXIT_FAILURE;
  }
  quick_sort(sequance_data, 0, size - 1);
  if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
  {
    perror("clock_gettime");
    free(sequance_data);
    free(data);
    return EXIT_FAILURE;
  }
  double sequential_ms = get_elapsed(start, end);

  Control_threads control = {.mx_threads = (int)max_threads, .active_threads = 1};
  if (pthread_mutex_init(&control.mutex, NULL) != 0)
  {
    fprintf(stderr, "Error: cannot initialize mutex\n");
    free(data);
    return EXIT_FAILURE;
  }

  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
  {
    perror("clock_gettime");
    pthread_mutex_destroy(&control.mutex);
    free(data);
    return EXIT_FAILURE;
  }
  parallel_sort(data, 0, size - 1, &control);
  if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
  {
    perror("clock_gettime");
    pthread_mutex_destroy(&control.mutex);
    free(data);
    return EXIT_FAILURE;
  }

  double parallel_ms = get_elapsed(start, end);
  pthread_mutex_destroy(&control.mutex);

  if (parallel_ms <= 0.0)
  {
    fprintf(stderr, "Error: invalid parallel sorting time\n");
    free(sequance_data);
    free(data);
    return EXIT_FAILURE;
  }

  double speedup = sequential_ms / parallel_ms;
  double efficiency = speedup / max_threads * 100.0;

  if (write_lst("output.txt", data, size) == -1)
  {
    fprintf(stderr, "Error: cannot write output.txt\n");
    free(data);
    return EXIT_FAILURE;
  }

  printf("Size of sorted list: %d\n", size);
  printf("Maximum number of threads: %d\n", control.mx_threads);
  printf("Sequential time: %.6f ms\n", sequential_ms);
  printf("Parallel time: %.6f ms\n", parallel_ms);
  printf("Speedup: %.4f\n", speedup);
  printf("Efficiency: %.2f%%\n", efficiency);
  free(data);
  free(sequance_data);
  return EXIT_SUCCESS;
}
