#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef struct Header {
	char key[64];
	char value[128];
	size_t bytes_consumed;
} header_t;

typedef struct Arr {
	size_t count;
	size_t capacity;
	header_t **elements;
} array_t;

char *trim(char *buffer) {
	size_t str_length = strlen(buffer);

	// trim leading whitespace
	
	char *start = buffer;

	while (*start == ' ') {
		start++;
	}

	char *end = start + strlen(start);

	while (*end == ' ') {
		*end = '\0';
		end--;
	}
	
	str_length = strlen(start);
	char* new_str = malloc(str_length + 1);
	new_str[0] = '\0';
	strncat(new_str, start, str_length);
	
	return new_str;
}

void destroy_arr(array_t *array) {
	for (size_t i = 0; i < array->count; i++) {
		free(array->elements[i]);
	}

	free(array->elements);
	free(array);
}

header_t *parse(char *headers) {
	
	char *crlf = "\r\n";
	size_t bytes_consumed = 0;
	char *ptr = headers;
	
	while (*ptr == ' ') {	
		ptr++;
		bytes_consumed++;
	}

	if (*ptr == '\r' && *(ptr + 1) == '\n') {
		return NULL;
	}
	

	char* header_start_ptr = ptr;
	size_t end_size = 0;
	char *line = malloc(1);
	line[0] = '\0';

	while (*ptr) {
		if (*ptr == '\r' && *(ptr + 1) == '\n') {
			line = realloc(line, end_size + 1);
			strncat(line, header_start_ptr, end_size);
			bytes_consumed += end_size + 2;
			ptr-= end_size;
			break;
		}
		ptr++;
		end_size++;
	}
	printf("%s \n", line);

	while (1) {
		if (*ptr != ':') {
			ptr++;
			continue;
		}

		if (*(ptr - 1) == ' ') {
			fprintf(stderr, "Header is invalid \n");
			exit(EXIT_FAILURE);
		} else {
			break;
		}
	}
	
	char *old_ptr = line;
	char* key = strsep(&line, ":");

	if (key == NULL) {
		return NULL;
	}
	char *trimmed_key = trim(key);
	header_t *new_header = malloc(sizeof(header_t));


	if (new_header == NULL) {
		fprintf(stderr, "Cannot initialize new_header\n");
		return NULL;
	}
	
	new_header->key[0] = '\0';
	new_header->value[0] = '\0';
	new_header->bytes_consumed = bytes_consumed;
	
	char *trimmed_value = trim(line);
	
	strncat(new_header->key, trimmed_key, strlen(trimmed_key));
	strncat(new_header->value, trimmed_value, strlen(trimmed_value));
	
	// array->elements[array->count] = new_header;
	// array->count++;
	free(old_ptr);
	free(trimmed_key);
	free(trimmed_value);

	return new_header;
}

int main() {
	
	// array_t *array = malloc(sizeof(array_t));
	//
	// if (array == NULL) {
	// 	exit(EXIT_FAILURE);
	// }
	//
	// array->count = 0;
	// array->capacity = 8;
	//
	// array->elements = malloc(sizeof(void *) * 8);
	//
	// if (array->elements == NULL) {
	// 	free(array);
	// 	exit(EXIT_FAILURE);
	// }

	header_t *header = parse("       Host: localhost:42069       \r\n\r\n");
	printf("%s \n", header->key);
	printf("%s \n", header->value);
	printf("%zu \n", header->bytes_consumed);
	// destroy_arr(array);
	free(header);

	return EXIT_SUCCESS;
}
