#include "stddef.h"
#include "stdbool.h"

#define MAX_BODY_LENGTH 1024
#define MAX_HEADER_LINES 


typedef struct Header {
	char key[64];
	char value[128];
} header_t;

typedef struct Array {
	size_t count;
	size_t capacity;
	header_t **elements;
} array_t;

typedef enum State {
	INITIALIZED = 0,
	PARSING_HEADERS = 1,
	PARSING_BODY = 2,
	DONE = 3
} state_t;

typedef struct RequestLine {
	char http_version[8];
	char request_target[128];
	char method[8];
} request_line_t;

typedef struct Request {
	request_line_t *request_line;
	array_t *headers;
	char body[MAX_BODY_LENGTH];
	state_t state;
} request_t;

typedef struct ChunkReader {
	char *data;
	size_t data_size;
	size_t pos;
	size_t bytes_per_read;
} chunk_reader_t;

void add_to_arr(header_t *header, array_t *array);
void destroy_arr(array_t *array);
bool is_special_character_allowed(char field_name_char);
bool is_valid_field_name(char* field_name);
char *to_lower_case(char* field_name);
header_t *is_header_key_existed(char * field_name, array_t *arr);
void add_value_to_header(header_t *header, array_t *arr);
size_t parse_header(char *headers, request_t *request);


char *trim(char *buffer);


request_t *request_from_reader(int client_fd);
size_t parse_request_line(char *line, request_t *request);

