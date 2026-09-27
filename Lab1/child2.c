#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
  char buffer[BUFFER_SIZE];
  char line[BUFFER_SIZE];

  size_t line_length = 0;

  while (1) {
    ssize_t n = read(0, buffer, BUFFER_SIZE);

    if (n == 0) {
      break;
    }
    if (n < 0) {
      return 1;
    }
    for (size_t i = 0; i < (size_t)n; i++) {
      line[line_length] = buffer[i];
      line_length++;

      if (buffer[i] == '\n') {
        for (ssize_t j = line_length - 2; j >= 0; j--) {
          write(1, &line[j], 1);
        }
        write(1, "\n", 1);
        line_length = 0;
      }
    }
  }

  return 0;
}