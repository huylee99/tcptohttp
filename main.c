#include "arpa/inet.h"
#include "netinet/in.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"
#include "unistd.h"
#include <asm-generic/socket.h>

#define PORT 42069
#define MAX_SIZE_LENGTH 1024
#define BUFFER_SIZE 8
#define INITIAL_CAPACITY 8
#define INITIAL_LINE_SIZE 256

typedef struct ArrayStruct {
  int count;
  int capacity;
  void **elements;
} array_t;

void arr_destroy(array_t *arr) {
  if (arr == NULL) {
    return;
  }

  for (int i = 0; i < arr->count; i++) {
    free(arr->elements[i]);
  }

  free(arr->elements);
  free(arr);
}

void add_line_to_arr(array_t *arr, char *line) {
  if (arr == NULL || line == NULL) {
    return;
  }

  if (arr->count > arr->capacity) {
    arr->capacity = arr->capacity * 2;
    arr->elements = realloc(arr->elements, sizeof(char *) * arr->capacity);
  }

  arr->elements[arr->count] = line;
  arr->count++;
}

array_t *getLines(int client_fd) {
  array_t *arr = malloc(sizeof(array_t));

  if (arr == NULL) {
    return NULL;
  }
  arr->count = 0;
  arr->capacity = INITIAL_CAPACITY;
  arr->elements = malloc(sizeof(char *) * arr->capacity);
  if (arr->elements == NULL) {
    return NULL;
  }

  char buffer[BUFFER_SIZE];
  char *line = malloc(sizeof(char) * INITIAL_LINE_SIZE);

  if (line == NULL) {
    free(arr->elements);
    free(arr);
    return NULL;
  }
  size_t current_line_size = 0;
  size_t bytes_size = 0;
  size_t line_size = INITIAL_LINE_SIZE;

  while ((bytes_size = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
    if (current_line_size > line_size) {
      line_size *= 2;
      line = realloc(line, line_size);
    }

    for (int i = 0; i < bytes_size; i++) {
      if (buffer[i] != '\n') {
        line[current_line_size] = buffer[i];
        current_line_size++;
      } else {
        line[current_line_size] = '\0';
        add_line_to_arr(arr, line);
        line = malloc(sizeof(char) * INITIAL_LINE_SIZE);
        current_line_size = 0;
      }
    }
  }

  free(line);

  return arr;
}

int main() {
  int sock_fd;
  int opt = 1;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  socklen_t server_addr_size = sizeof(server_addr);
  socklen_t client_addr_size = sizeof(server_addr);

  if (sock_fd == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

  if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("set sockopts failed");
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  if (bind(sock_fd, (struct sockaddr *)&server_addr, server_addr_size) < 0) {
    perror("bind failed failed");
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(sock_fd, 5) < 0) {
    perror("listen failed");
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  printf("Listening on port %d\n", PORT);

  while (1) {
    int client_sockfd =
        accept(sock_fd, (struct sockaddr *)&client_addr, &client_addr_size);

    if (client_sockfd == -1) {
      perror("accept failed \n");
      continue;
    }
    printf("connection accepted \n");
    printf("Information: %s:%d \n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
    array_t *arr = getLines(client_sockfd);

    for (int i = 0; i < arr->count; i++) {
      printf("read: %s \n", (char *)arr->elements[i]);
    }

    arr_destroy(arr);
    printf("Connection closed\n!");

    close(client_sockfd);
  }

  close(sock_fd);

  return EXIT_SUCCESS;
}
