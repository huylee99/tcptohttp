#include "arpa/inet.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 42069

int main() {
  int sock_fd;
  int opt = 1;
  struct sockaddr_in server_addr;
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  socklen_t addr_size = sizeof(server_addr);

  if (sock_fd == -1) {
    perror("socket failed \n");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
  server_addr.sin_port = htons(PORT);

  char *buffer = malloc(8 * sizeof(char));

  while (1) {
    printf("> ");
    scanf("%s", buffer);
    ssize_t bytes_written = sendto(sock_fd, buffer, 8, 0,
                                   (struct sockaddr *)&server_addr, addr_size);

    if (bytes_written < 0) {
      fprintf(stderr, "Error: %s\n", strerror(errno));
      continue;
    }
  };

  free(buffer);
  return EXIT_SUCCESS;
}
