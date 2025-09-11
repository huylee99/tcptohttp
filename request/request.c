#include "stdlib.h"
#include "request.h"
#include "string.h"
#include "stdio.h"



char *version_extract(char *http_version) {
	
	if (http_version == NULL || strlen(http_version) < 1) {
		return NULL;
	}
	size_t index = 0;
	size_t start_pos = 0;
	char *version = malloc(sizeof(char) * 4);
	size_t version_index = 0;
	while (1) {

		if (http_version[index] == '\0' || version_index == 4) {
			version[version_index] = '\0';
			break;
		}

		if (start_pos == 1) {
			version[version_index] = http_version[index];
			version_index++;
		}

		if (http_version[index] == '/') {
			start_pos = 1; 
		}

		index++;
	}

	return version;
}

request_line_t request_from_reader(char *request_line) {
	request_line_t request_line_struct = {
		.http_version = NULL,
		.request_target = NULL	,
		.method = NULL,
	};
	char *line = malloc(sizeof(char) * 128);
	char *request_line_arr[3];
	size_t line_size = 0;
	size_t request_index = 0;
	size_t request_line_arr_index = 0;

	while (1) {
		if (request_line[request_index] == '\n') {
			break;
		}

		if (request_line[request_index] != ' ' && request_line[request_index] != '\r') {
			line[line_size] = request_line[request_index];
			line_size++;
		} else {
			line[line_size] = '\0';
			request_line_arr[request_line_arr_index] = line;
			line = malloc(sizeof(char) * 128);
			line_size = 0;
			request_line_arr_index++;
		}
	
		request_index++;
	}
	
	request_line_struct.method = request_line_arr[0];
	request_line_struct.request_target = request_line_arr[1];
	request_line_struct.http_version = version_extract(request_line_arr[2]);
	

	free(line);

	return request_line_struct;
}

// int main() {
// 	request_line_t request = request_from_reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n");
//
// 	printf("%s \n", request.method);
// 	printf("%s \n", request.request_target);
// 	printf("%s \n", request.http_version);
//
// 	return EXIT_SUCCESS;
// }

