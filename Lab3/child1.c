#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include "shared.h"

ssize_t write_all(int fd, const char *buffer, size_t len)
{
  size_t written = 0;
  while (written < len)
  {
    ssize_t n = write(fd, buffer + written, len - written);
    if (n <= 0)
    {
      return -1;
    }
    written += n;
  }
  return written;
}

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    return EXIT_FAILURE;
  }
  char *filename = argv[1];
  char *shm_name = argv[2];
  char *empty_name = argv[3];
  char *full_name = argv[4];
  int shm_fd = shm_open(shm_name, O_RDWR, 0600);
  if (shm_fd == -1)
  {
    perror("shm_open failed");
    return EXIT_FAILURE;
  }
  Shared_data *shared = mmap(NULL, sizeof(Shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shared == MAP_FAILED)
  {
    perror("mmap failed");
    close(shm_fd);
    return EXIT_FAILURE;
  }
  sem_t *empty = sem_open(empty_name, 0);
  sem_t *full = sem_open(full_name, 0);
  if (empty == SEM_FAILED || full == SEM_FAILED)
  {
    perror("sem_open failed");
    munmap(shared, sizeof(Shared_data));
    close(shm_fd);
    return EXIT_FAILURE;
  }
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1)
  {
    perror("fd_open failed");
    return EXIT_FAILURE;
  }
  char *line = NULL;
  size_t line_len = 0;
  size_t capacity = 0;
  int data_caught = 0;
  int status = EXIT_SUCCESS;

  while (!data_caught)
  {
    if (sem_wait(full) == -1)
    {
      perror("sem_wait");
      status = EXIT_FAILURE;
      break;
    }
    if (shared->finished)
    {
      data_caught = 1;
    }
    else
    {
      if (line_len + shared->len > capacity)
      {
        size_t new_capacity = capacity == 0 ? BUFFER_SIZE : capacity;
        while (new_capacity < line_len + shared->len)
        {
          new_capacity *= 2;
        }
        char *temp = realloc(line, new_capacity);
        if (temp == NULL)
        {
          free(line);
          sem_post(empty);
          return EXIT_FAILURE;
        }
        line = temp;
        capacity = new_capacity;
      }
      if (shared->len > 0)
      {
        memcpy(line + line_len, shared->data, shared->len);
        line_len += shared->len;
      }
      int end_of_line = shared->end_of_line;
      if (end_of_line)
      {
        for (size_t i = 0; i < line_len / 2; i++)
        {
          char temp = line[i];
          line[i] = line[line_len - 1 - i];
          line[line_len - 1 - i] = temp;
        }
        if (write_all(fd, line, line_len) == -1 || write_all(fd, "\n", 1) == -1)
        {
          perror("write failed");
          status = EXIT_FAILURE;
          sem_post(empty);
          break;
        }
        line_len = 0;
      }
    }
    sem_post(empty);
  }
  free(line);
  close(fd);
  sem_close(empty);
  sem_close(full);
  munmap(shared, sizeof(Shared_data));
  close(shm_fd);

  return status;
}
