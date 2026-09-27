#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

ssize_t write_all(int fd, const char *buffer, size_t length) {
  size_t written = 0;

  while (written < length) {
    ssize_t n = write(fd, buffer + written, length - written);

    if (n <= 0) {
      return -1;
    }

    written += n;
  }

  return written;
}

int main() {
  char message1[] = "Enter file name for first child: ";
  write(1, message1, sizeof(message1) - 1);

  char filename1[100];

  ssize_t n1 = read(0, filename1, 100);

  if (n1 > 0 && filename1[n1 - 1] == '\n') {
    filename1[n1 - 1] = '\0';
  }

  int fd1 = open(filename1, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (fd1 < 0) {
    return 1;
  }

  char message2[] = "Enter file name for second child: ";
  write(1, message2, sizeof(message2) - 1);

  char filename2[100];

  ssize_t n2 = read(0, filename2, 100);

  if (n2 > 0 && filename2[n2 - 1] == '\n') {
    filename2[n2 - 1] = '\0';
  }

  int fd2 = open(filename2, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (fd2 < 0) {
    return 1;
  }

  int pipe1[2];
  int pipe2[2];

  if (pipe(pipe1) < 0) {
    return 1;
  }

  if (pipe(pipe2) < 0) {
    return 1;
  }

  pid_t p1 = fork();

  if (p1 < 0) {
    return 1;
  }

  else if (p1 == 0) {
    // child1

    close(pipe2[0]);
    close(pipe2[1]);
    close(pipe1[1]);

    dup2(pipe1[0], 0);
    dup2(fd1, 1);

    close(pipe1[0]);
    close(fd1);

    execl("./child1", "./child1", NULL);

    return 1;
  }

  else {
    pid_t p2 = fork();

    if (p2 < 0) {
      return 1;
    }

    else if (p2 == 0) {
      // child2

      close(pipe1[0]);
      close(pipe1[1]);
      close(pipe2[1]);

      dup2(pipe2[0], 0);
      dup2(fd2, 1);

      close(pipe2[0]);
      close(fd2);

      execl("./child2", "./child2", NULL);

      return 1;
    }

    else {
      // parent

      close(pipe1[0]);
      close(pipe2[0]);

      char buffer[BUFFER_SIZE];
      char line[BUFFER_SIZE];

      size_t line_length = 0;

      while (1) {
        ssize_t n = read(0, buffer, BUFFER_SIZE);

        if (n == 0) {
          break;
        }

        if (n < 0) {
          break;
        }

        size_t i = 0;

        while (i < (size_t)n) {
          if (line_length >= BUFFER_SIZE) {
            return 1;
          }

          line[line_length] = buffer[i];
          line_length++;
          i++;

          if (line[line_length - 1] == '\n') {
            int choice = rand() % 10;

            if (choice < 8) {
              write_all(pipe1[1], line, line_length);
            } else {
              write_all(pipe2[1], line, line_length);
            }

            line_length = 0;
          }
        }
      }

      close(pipe1[1]);
      close(pipe2[1]);

      waitpid(p1, NULL, 0);
      waitpid(p2, NULL, 0);
    }
  }

  return 0;
}