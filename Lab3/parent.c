#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "shared.h"
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int create_shm(const char *name, Shared_data **shared, int *fd)
{
  *fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (*fd == -1)
  {
    perror("shm_open");
    return -1;
  }
  if (ftruncate(*fd, sizeof(Shared_data)) == -1)
  {
    perror("ftruncate");
    close(*fd);
    shm_unlink(name);
    return -1;
  }
  *shared = mmap(NULL, sizeof(Shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
  if (*shared == MAP_FAILED)
  {
    perror("mmap");
    close(*fd);
    shm_unlink(name);
    return -1;
  }
  memset(*shared, 0, sizeof(Shared_data));
  return 0;
}

sem_t *create_sem(const char *name, unsigned int value)
{
  sem_t *sem = sem_open(name, O_CREAT | O_EXCL, 0600, value);
  if (sem == SEM_FAILED)
  {
    perror("sem");
    return SEM_FAILED;
  }
  return sem;
}

int read_line(char **line, size_t *line_len)
{
  size_t capacity = 128;
  size_t len = 0;
  *line = malloc(capacity);
  if (*line == NULL)
  {
    perror("malloc failed");
    return -1;
  }
  char c;
  ssize_t n;
  while ((n = read(STDIN_FILENO, &c, 1)) > 0)
  {
    if (c == '\n')
    {
      break;
    }
    if (len + 1 >= capacity)
    {
      capacity *= 2;
      char *temp = realloc(*line, capacity);
      if (temp == NULL)
      {
        perror("realloc failed");
        free(*line);
        *line = NULL;
        return -1;
      }
      *line = temp;
    }
    (*line)[len] = c;
    len++;
  }
  if (n == -1)
  {
    perror("read failed");
    free(*line);
    *line = NULL;
    return -1;
  }
  if (n == 0 && len == 0)
  {
    free(*line);
    *line = NULL;
    return 0;
  }
  (*line)[len] = '\0';
  *line_len = len;
  return 1;
}

int send_line(Shared_data *shared, sem_t *empty, sem_t *full, const char *line, size_t line_len)
{
  size_t sent = 0;
  do
  {
    size_t part_len = line_len - sent;
    if (part_len > BUFFER_SIZE)
    {
      part_len = BUFFER_SIZE;
    }
    if (sem_wait(empty) == -1)
    {
      perror("sem_wait failed");
      return -1;
    }
    memcpy(shared->data, line + sent, part_len);
    shared->len = part_len;
    shared->end_of_line = (sent + part_len == line_len);
    shared->finished = 0;
    if (sem_post(full) == -1)
    {
      perror("sem_post failed");
      return -1;
    }
    sent += part_len;
  } while (sent < line_len);
  return 0;
}

int message_end(Shared_data *shared, sem_t *empty, sem_t *full)
{
  if (sem_wait(empty) == -1)
  {
    perror("sem_wait failed");
    return -1;
  }
  shared->len = 0;
  shared->end_of_line = 0;
  shared->finished = 1;
  if (sem_post(full) == -1)
  {
    perror("sem_post failed");
    return -1;
  }
  return 0;
}

int main()
{
  char shm_name1[64];
  char shm_name2[64];
  snprintf(shm_name1, sizeof(shm_name1), "/lab3_shm1_%d", getpid());
  snprintf(shm_name2, sizeof(shm_name2), "/lab3_shm2_%d", getpid());
  Shared_data *shared1 = NULL;
  Shared_data *shared2 = NULL;
  int shm_fd1 = -1;
  int shm_fd2 = -1;
  if (create_shm(shm_name1, &shared1, &shm_fd1) == -1)
  {
    return EXIT_FAILURE;
  }
  if (create_shm(shm_name2, &shared2, &shm_fd2) == -1)
  {
    munmap(shared1, sizeof(Shared_data));
    close(shm_fd1);
    shm_unlink(shm_name1);
    return EXIT_FAILURE;
  }

  char empty1_name[64];
  char full1_name[64];
  char empty2_name[64];
  char full2_name[64];
  snprintf(full1_name, sizeof(full1_name), "/lab3_full1_%d", getpid());
  snprintf(empty1_name, sizeof(empty1_name), "/lab3_empty1_%d", getpid());
  snprintf(full2_name, sizeof(full2_name), "/lab3_full2_%d", getpid());
  snprintf(empty2_name, sizeof(empty2_name), "/lab3_empty2_%d", getpid());
  sem_t *empty1 = create_sem(empty1_name, 1);
  sem_t *full1 = create_sem(full1_name, 0);
  sem_t *empty2 = create_sem(empty2_name, 1);
  sem_t *full2 = create_sem(full2_name, 0);
  if (empty1 == SEM_FAILED || full1 == SEM_FAILED || empty2 == SEM_FAILED || full2 == SEM_FAILED)
  {
    return EXIT_FAILURE;
  }

  char message1[] = "Enter file name for first child: ";
  write(1, message1, sizeof(message1) - 1);
  char filename1[100];
  ssize_t n1 = read(0, filename1, 100);
  if (n1 > 0 && filename1[n1 - 1] == '\n')
  {
    filename1[n1 - 1] = '\0';
  }

  char message2[] = "Enter file name for second child: ";
  write(1, message2, sizeof(message2) - 1);
  char filename2[100];
  ssize_t n2 = read(0, filename2, 100);
  if (n2 > 0 && filename2[n2 - 1] == '\n')
  {
    filename2[n2 - 1] = '\0';
  }

  pid_t p1 = fork();
  if (p1 < 0)
  {
    perror("fork failed");
    return EXIT_FAILURE;
  }

  else if (p1 == 0)
  {
    execl("./child1", "./child1", filename1, shm_name1, empty1_name, full1_name, NULL);
    perror("execl_1 failed");
    _exit(EXIT_FAILURE);
  }

  else
  {
    pid_t p2 = fork();

    if (p2 < 0)
    {
      perror("fork failed");
      return EXIT_FAILURE;
    }

    else if (p2 == 0)
    {
      execl("./child2", "./child2", filename2, shm_name2, empty2_name, full2_name, NULL);
      perror("execl_2 failed");
      _exit(EXIT_FAILURE);
    }

    else
    {
      srand(time(NULL));
      while (1)
      {
        char *line = NULL;
        size_t line_len = 0;
        int result = read_line(&line, &line_len);
        if (result == -1)
        {
          break;
        }
        if (result == 0)
        {
          break;
        }
        if (rand() % 10 < 8)
        {
          if (send_line(shared1, empty1, full1, line, line_len) == -1)
          {
            free(line);
            break;
          }
        }
        else
        {
          if (send_line(shared2, empty2, full2, line, line_len) == -1)
          {
            free(line);
            break;
          }
        }
        free(line);
      }
      message_end(shared1, empty1, full1);
      message_end(shared2, empty2, full2);
      waitpid(p1, NULL, 0);
      waitpid(p2, NULL, 0);
      munmap(shared2, sizeof(Shared_data));
      close(shm_fd1);
      close(shm_fd2);
      shm_unlink(shm_name1);
      shm_unlink(shm_name2);
      sem_close(empty1);
      sem_close(full1);
      sem_close(empty2);
      sem_close(full2);
      sem_unlink(empty1_name);
      sem_unlink(full1_name);
      sem_unlink(empty2_name);
      sem_unlink(full2_name);
    }
  }
  return 0;
}