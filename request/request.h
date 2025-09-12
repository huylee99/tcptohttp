#include "stddef.h"

#define MAX_BODY_LENGTH 1024
#define MAX_HEADER_LINES 

typedef enum State {
	INITIALIZED = 0,
	DONE = 1
} state_t;

typedef struct Header {
	char key;
	char value;
} header_t;

typedef struct RequestLine {
	char http_version[8];
	char request_target[128];
	char method[8];
} request_line_t;

typedef struct Request {
	request_line_t *request_line;
	char *headers[16];
	char body[MAX_BODY_LENGTH];
	state_t state;
} request_t;

typedef struct ChunkReader {
	char *data;
	size_t data_size;
	size_t pos;
	size_t bytes_per_read;
} chunk_reader_t;

 

request_t *request_from_reader(chunk_reader_t *reader);
size_t parse_request_line(char *line, request_t *request);
