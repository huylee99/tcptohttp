#include "stdlib.h"
#include "request.h"
#include "string.h"
#include "stdio.h"

#define BYTES_READ 16

int chunk_reader_read(chunk_reader_t *cr, char *p, size_t p_len) {
	if (cr->pos >= cr->data_size) {
		return 0; // EOF
	}

	size_t end_index = cr->pos + cr->bytes_per_read;

	if (end_index > cr->data_size) {
		end_index = cr->data_size;
	}

	size_t bytes_to_copy = end_index - cr->pos;

	if (bytes_to_copy > p_len) {
		bytes_to_copy = p_len;
	}
	
	memcpy(p, cr->data + cr->pos, bytes_to_copy);
	p[p_len] = '\0';		
	cr->pos += bytes_to_copy;

	return bytes_to_copy;
}

chunk_reader_t *chunk_reader(char *data, size_t num_bytes_per_read) {
	if (data == NULL) {
		return NULL;
	}

	chunk_reader_t *reader = malloc(sizeof(chunk_reader_t));

	if (reader == NULL) {
		return NULL;
	}

	reader->data = data;
	reader->pos = 0;
	reader->data_size = strlen(reader->data);
	reader->bytes_per_read = num_bytes_per_read;

	return reader;
}


size_t parse(char *bytes_slice, size_t bytes_read, char* line, request_t *request) {
	if (bytes_read == 0) {
		return 0;
	}
	if(strcat(line, bytes_slice) == NULL) {
		perror("Error when parsing, exit...\n");
		exit(EXIT_FAILURE);
	}
	size_t bytes_consumed = parse_request_line(line, request);

	if (bytes_consumed > 0) {
		request->state = DONE;
		printf("Consumed %zu bytes | Current header line length: %zu \n", bytes_consumed, strlen(line));
		return bytes_consumed;
	}
	
	return bytes_consumed;
}

size_t parse_request_line(char *line, request_t *request) {
	char *copied_line = strdup(line);
	if (copied_line == NULL) {
		return 0;
	}

	size_t line_completed = 0;
	char *temp = copied_line;
	while (*temp) {
		if (*temp == '\r' && *(temp + 1) == '\n') {
			line_completed = 1;
		}
		temp++;
	}
	if (line_completed == 0) {
		free(copied_line);
		return 0;
	}

	char *old_ptr = copied_line;

	size_t bytes_consumed = strlen(copied_line);	
	char *token = strsep(&copied_line, " ");	
	if (token == NULL) {
		return 0;
	}
	strncpy(request->request_line->method, token, sizeof(request->request_line->method));
	
	token = strsep(&copied_line, " ");
	if (token == NULL) {
		return 0;
	}
	strncpy(request->request_line->request_target, token, sizeof(request->request_line->request_target));
	
	token = strsep(&copied_line, " ");
	if (token == NULL) {
		return 0;
	}
	char *version = token;
	version = strsep(&token, "/");
	if (token == NULL) {
		return 0;
	}
	version = strsep(&token, "/");
	strncpy(request->request_line->http_version, version, sizeof(request->request_line->http_version));
	
	free(old_ptr);
	return bytes_consumed;
}

request_t *request_from_reader(chunk_reader_t *reader) {
	request_line_t *request_line = malloc(sizeof(request_line_t));
	
	if (request_line == NULL) {
		return NULL;
	}
	
	request_t *request = malloc(sizeof(request_t));
	
	if (request == NULL) {
		return NULL;
	}
	
	char *line = malloc(sizeof(char) * 128);
	request->request_line = request_line;
	request->state = INITIALIZED;
	size_t bytes_read = 0;
	char buffer[reader->bytes_per_read];
	
	while (request->state != DONE) {
		bytes_read = chunk_reader_read(reader, buffer, reader->bytes_per_read);
		parse(buffer, bytes_read, line, request);
	}

	free(line);

	return request;
}

int main() {

	chunk_reader_t *reader = chunk_reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n", BYTES_READ);

	if (reader == NULL) {
		perror("Cannot initilize reader\n");
		return EXIT_FAILURE;
	}
	
	request_t *request = request_from_reader(reader);
	printf("%s \n", request->request_line->method);
	printf("%s \n", request->request_line->request_target);
	printf("%s \n", request->request_line->http_version);

	free(request->request_line);
	free(request);
	free(reader);

	return EXIT_SUCCESS;
}

