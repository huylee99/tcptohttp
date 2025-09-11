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
	char* http_version;
	char* request_target;
	char* method;
} request_line_t;

typedef struct Request {
	request_line_t request_line;
	char *headers[16];
	char body[MAX_BODY_LENGTH];
} request_t;

request_line_t request_from_reader(char *request_line);

