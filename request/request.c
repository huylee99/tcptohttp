#include "request.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"

#define BYTES_READ 100

size_t chunk_reader_read(chunk_reader_t *cr, char *p, size_t p_len) {
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

  strncat(p, cr->data + cr->pos, bytes_to_copy);
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

size_t parse(char *line, request_t *request) {
  switch (request->state) {
  case INITIALIZED: {
    size_t bytes_consumed = parse_request_line(line, request);

    if (bytes_consumed == 0) {
      return 0;
    }
    request->state = DONE;
    return bytes_consumed;
  }
  case DONE: {
    return 0;
  }
  default: {
    return 0;
  }
  }
}

size_t parse_request_line(char *line, request_t *request) {
  size_t line_completed = 0;
  size_t end_index = 0;
  char *temp = line;
  while (*temp) {
    if (*temp == '\r' && *(temp + 1) == '\n') {
      line_completed = 1;
      break;
    }
    end_index++;
    temp++;
  }

  if (line_completed == 0) {
    return 0;
  }
  char *start_line = malloc(end_index + 1);

  if (start_line == NULL) {
    return 0;
  }
  strncpy(start_line, line, end_index);
  start_line[end_index] = '\0';

  char *old_ptr = start_line;
  char *token = strsep(&start_line, " ");
  if (token == NULL) {
    return 0;
  }
  strncpy(request->request_line->method, token,
          sizeof(request->request_line->method));

  token = strsep(&start_line, " ");
  if (token == NULL) {
    return 0;
  }
  strncpy(request->request_line->request_target, token,
          sizeof(request->request_line->request_target));

  token = strsep(&start_line, " ");
  if (token == NULL) {
    return 0;
  }
  char *version = token;
  version = strsep(&token, "/");
  if (version == NULL) {
    return 0;
  }
  version = strsep(&token, "/");
  strncpy(request->request_line->http_version, version,
          sizeof(request->request_line->http_version));

  free(old_ptr);
  return end_index + 2;
}

request_t *request_from_reader(int client_fd) {

  request_line_t *request_line = malloc(sizeof(request_line_t));

  if (request_line == NULL) {
    return NULL;
  }

  request_t *request = malloc(sizeof(request_t));

  if (request == NULL) {
    return NULL;
  }

  size_t allocated_size = BYTES_READ + 1;
  char *line = malloc(allocated_size);
  line[0] = '\0';
  request->request_line = request_line;
  request->state = INITIALIZED;
  size_t read_to_index = 0;
  size_t bytes_read = 0;
  size_t bytes_consumed = 0;

  while (request->state != DONE) {
    if (read_to_index >= allocated_size) {
      allocated_size += BYTES_READ + 1;
      char *temp = realloc(line, allocated_size);

      if (temp == NULL) {
        free(line);
        return NULL;
      }
      line = temp;
    }

    bytes_read = recv(client_fd, line, BYTES_READ, 0);

    if (bytes_read == 0) {
      break;
    }
    read_to_index += bytes_read;

    bytes_consumed = parse(line, request);

    if (bytes_consumed == 0) {
      continue;
    }

    char *start_new_part_ptr = line + bytes_consumed;
    char *new_line = malloc(read_to_index - bytes_consumed + 1);
    new_line[0] = '\0';
    strncat(new_line, start_new_part_ptr, read_to_index - bytes_consumed);
    free(line);
    line = new_line;
    read_to_index -= bytes_consumed;
    allocated_size -= bytes_consumed;
  }

  free(line);

  return request;
}

// int main() {
//
//   chunk_reader_t *reader =
//       chunk_reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: "
//                    "curl/7.81.0\r\nAccept: */*\r\n\r\n",
//                    BYTES_READ);
//
//   if (reader == NULL) {
//     perror("Cannot initilize reader\n");
//     return EXIT_FAILURE;
//   }
//
//   request_t *request = request_from_reader(reader);
//   printf("%s \n", request->request_line->method);
//   printf("%s \n", request->request_line->request_target);
//   printf("%s \n", request->request_line->http_version);
//
//   free(request->request_line);
//   free(request);
//   free(reader);
//
//   return EXIT_SUCCESS;
// }
