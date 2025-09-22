#include "arpa/inet.h"
#include "netinet/in.h"
#include "stdio.h"
#include "string.h"
#include "sys/socket.h"
#include "unistd.h"
#include <asm-generic/socket.h>
#include "server.h"
#include "request.h"
#include "stdlib.h"

void write_status_line(char* buffer, enum StatusCode status_code) {
	switch (status_code) {
		case OK :
			strcat(buffer, "HTTP/1.1 200 OK\r\n");
			break;
		case BAD_REQUEST:
			strcat(buffer, "HTTP/1.1 400 Bad Request\r\n");
			break;
		case INTERNAL_SERVER_ERROR:
			strcat(buffer, "HTTP/1.1 500 Internal Server Error\r\n");
			break;
		default: {
			char status_line[20];
			snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d\r\n", status_code);
			strcat(buffer, status_line);
			break;
		}
	}
}

array_t *get_default_headers(int content_length) {
	array_t *headers = malloc(sizeof(array_t));
	if (headers == NULL) {
		return NULL;
	}

	headers->capacity = 8;
	headers->elements = malloc(sizeof(header_t *) * headers->capacity);
	headers->count = 0;

	if (headers->elements == NULL) {
		free(headers);
		return NULL;
	}

	header_t *header = malloc(sizeof(header_t));
	strcpy(header->key, "content-length");
	sprintf(header->value, "%d", content_length);
	
	headers->elements[0] = header;
	headers->count++;

	header = malloc(sizeof(header_t));
	strcpy(header->key, "connection");
	strcpy(header->value, "close");

	headers->elements[1] = header;
	headers->count++;

	header = malloc(sizeof(header_t));
	strcpy(header->key, "content-type");
	strcpy(header->value, "text/plain");

	headers->elements[2] = header;
	headers->count++;

	return headers;
}

void write_headers(char *buffer, array_t *headers) {
	for (int i = 0; i < headers->count; i++) {
		char header[512];
		sprintf(header, "%s: %s\r\n", headers->elements[i]->key, headers->elements[i]->value);
		strcat(buffer, header);
	}
}

#define PORT 42069

int main() {
	int sock_fd;
	int opt = 1;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	socklen_t server_addr_size = sizeof(server_addr);
	socklen_t client_addr_size = sizeof(client_addr);

	if (sock_fd == -1) {
		perror("Socket failed \n");
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("set sock failed \n");
		exit(EXIT_FAILURE);
	}

	if (bind(sock_fd, (struct sockaddr*) &server_addr, server_addr_size) < 0) {
		perror("bind error \n");
		exit(EXIT_FAILURE);
	}

	if (listen(sock_fd, 5) < 0) {
		perror("Listen failed \n");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	printf("Listening on port %d\n", PORT);
	
	array_t *headers = get_default_headers(0);
	char *buffer = malloc(1024);
	buffer[0] = '\0';

	while (1) {
		int client_sockfd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_size);

		if (client_sockfd == -1) {
			perror("Accept failed \n");
			continue;
		}

		write_status_line(buffer, OK);
		write_headers(buffer, headers);

		char *buf = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello World!\n";

		send(client_sockfd, buffer, strlen(buffer), 0);
		close(client_sockfd);
		free(buffer);
		buffer = malloc(1024);
		buffer[0] = '\0';
		printf("Message sent!\n");
	}
	
	free(buffer);
	destroy_arr(headers);
	close(sock_fd);
	return 0;
}
