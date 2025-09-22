#include "request.h"
#include "ctype.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"
#include <asm-generic/errno.h>

#define BYTES_READ 8
#define LINE_SIZE 1024

void add_to_arr(header_t *header, array_t *array) {
  if (header == NULL || array == NULL) {
    fprintf(stderr, "add_to_arr: Error \n");
    exit(EXIT_FAILURE);
  }

  if (array->count == array->capacity) {
    array->capacity = array->capacity * 2;
    array->elements =
        realloc(array->elements, sizeof(header_t *) * array->capacity);
  }
  array->elements[array->count] = header;
  array->count++;
}

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
  char *new_str = malloc(str_length + 1);
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

bool is_special_character_allowed(char field_name_char) {
  if (field_name_char == '!' || field_name_char == '#' ||
      field_name_char == '$' || field_name_char == '%' ||
      field_name_char == '$' || field_name_char == '\'' ||
      field_name_char == '*' || field_name_char == '+' ||
      field_name_char == '-' || field_name_char == '.' ||
      field_name_char == '^' || field_name_char == '_' ||
      field_name_char == '`' || field_name_char == '|' ||
      field_name_char == '~') {
    return true;
  }

  return false;
}

bool is_valid_field_name(char *field_name) {
  if (field_name == NULL) {
    return false;
  }

  size_t length = strlen(field_name);

  if (length <= 0) {
    return false;
  }

  for (size_t i = 0; i < length; i++) {
    if (isalpha(field_name[i]) || isdigit(field_name[i]) ||
        is_special_character_allowed(field_name[i])) {
      continue;
    } else {
      return false;
    }
  }

  return true;
}

char *to_lower_case(char *field_name) {
  if (field_name == NULL) {
    return NULL;
  }

  size_t str_length = strlen(field_name);

  char *lower_case_str = malloc(sizeof(char) * str_length + 1);

  if (lower_case_str == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < str_length; i++) {
    lower_case_str[i] = tolower(field_name[i]);
  }

  lower_case_str[str_length] = '\0';

  return lower_case_str;
}

header_t *is_header_key_existed(char *field_name, array_t *arr) {
  if (arr == NULL || field_name == NULL) {
    fprintf(stderr, "Error: arr or field_name is NULL\n");
    exit(EXIT_FAILURE);
  }
  // bool is_key_existed = false;
  char *lower_field_name = to_lower_case(field_name);
  //
  // char *lower_item_field_name = to_lower_case(header->key);
  //
  // if (strcmp(lower_item_field_name, lower_field_name) == 0) {
  // 	is_key_existed = true;
  // }
  //
  // free(lower_field_name);
  // free(lower_item_field_name);
  //
  // return is_key_existed;

  header_t *found_header_ptr = NULL;

  for (int i = 0; i < arr->count; i++) {
    if (found_header_ptr != NULL) {
      break;
    }

    header_t *header = arr->elements[i];
    char *lower_item_field_name = to_lower_case(header->key);
    if (strcmp(lower_field_name, lower_item_field_name) == 0) {
      found_header_ptr = arr->elements[i];
    }
    free(lower_item_field_name);
  }

  free(lower_field_name);
  return found_header_ptr;
}

void add_value_to_header(header_t *header, array_t *arr) {
  if (header == NULL || arr == NULL) {
    fprintf(stderr, "add_value_to_header: Header or Arr is NULL\n");
    exit(EXIT_FAILURE);
  }

  header_t *header_ptr = is_header_key_existed(header->key, arr);

  if (header_ptr == NULL) {
    add_to_arr(header, arr);
    return;
  }

  // if key is existed
  char *separator = ", ";
  strncat(header_ptr->value, separator, 2);
  strncat(header_ptr->value, header->value, strlen(header->value));
}

size_t parse_body(char *body, request_t *request) {
  int content_length = -1;
  for (int i = 0; i < request->headers->count; i++) {
    if (strcmp(request->headers->elements[i]->key, "content-length") == 0) {
      content_length = atoi(request->headers->elements[i]->value);
      break;
    }
  }

  size_t bytes_consumed = 0;
  size_t body_length = 0;
  char *ptr = body;
  while (*ptr == '\r' || *ptr == '\n') {
    ptr++;
  }

  char *body_start_ptr = ptr;

  while (*ptr) {
    ptr++;
    body_length++;
  }

  if (body_length == 0 && content_length == -1) {
    printf("Empty body, no reported content length \n");
    return bytes_consumed;
  }

  if (body_length < content_length) {
    return 0;
  }

  if (body_length > content_length) {
    fprintf(stderr,
            "Body larger than reported content length | "
            "body_length: %zu - content_length %d \n",
            body_length, content_length);
    exit(EXIT_FAILURE);
  }

  strcat(request->body, body_start_ptr);
  request->state = DONE;

  return bytes_consumed + body_length;
}

size_t parse_header(char *headers, request_t *request) {

  size_t bytes_consumed = 0;
  char *ptr = headers;

  while (*ptr == ' ') {
    ptr++;
    bytes_consumed++;
  }

  if (*ptr == '\r' && *(ptr + 1) == '\n') {
    request->state = PARSING_BODY;
    return bytes_consumed;
  }

  char *header_start_ptr = ptr;
  size_t end_size = 0;
  char *line = malloc(1);
  line[0] = '\0';

  while (*ptr) {
    if (*ptr == '\r' && *(ptr + 1) == '\n') {
      line = realloc(line, end_size + 1);
      strncat(line, header_start_ptr, end_size);
      bytes_consumed += end_size + 2;
      ptr -= end_size;
      break;
    } else if (end_size + 2 == strlen(header_start_ptr)) {
      return 0;
    } else {
      ptr++;
      end_size++;
    }
  }

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
  char *key = strsep(&line, ":");

  if (key == NULL) {
    fprintf(stderr, "Invalid header \n");
    exit(EXIT_FAILURE);
  }
  char *trimmed_key = trim(key);
  char *lower_case_key = to_lower_case(trimmed_key);
  header_t *new_header = malloc(sizeof(header_t));

  if (new_header == NULL) {
    fprintf(stderr, "Cannot initialize new_header\n");
    exit(EXIT_FAILURE);
  }

  new_header->key[0] = '\0';
  new_header->value[0] = '\0';

  char *trimmed_value = trim(line);

  strncat(new_header->key, lower_case_key, strlen(lower_case_key));
  strncat(new_header->value, trimmed_value, strlen(trimmed_value));

  // array->elements[array->count] = new_header;
  // array->count++;
  free(old_ptr);
  free(trimmed_key);
  free(trimmed_value);
  free(lower_case_key);

  add_value_to_header(new_header, request->headers);

  return bytes_consumed;
}

// size_t chunk_reader_read(chunk_reader_t *cr, char *p, size_t p_len) {
//   if (cr->pos >= cr->data_size) {
//     return 0; // EOF
//   }
//
//   size_t end_index = cr->pos + cr->bytes_per_read;
//
//   if (end_index > cr->data_size) {
//     end_index = cr->data_size;
//   }
//
//   size_t bytes_to_copy = end_index - cr->pos;
//
//   if (bytes_to_copy > p_len) {
//     bytes_to_copy = p_len;
//   }
//
//   strncat(p, cr->data + cr->pos, bytes_to_copy);
//   cr->pos += bytes_to_copy;
//
//   return bytes_to_copy;
// }
//
// chunk_reader_t *chunk_reader(char *data, size_t num_bytes_per_read) {
//   if (data == NULL) {
//     return NULL;
//   }
//
//   chunk_reader_t *reader = malloc(sizeof(chunk_reader_t));
//
//   if (reader == NULL) {
//     return NULL;
//   }
//
//   reader->data = data;
//   reader->pos = 0;
//   reader->data_size = strlen(reader->data);
//   reader->bytes_per_read = num_bytes_per_read;
//
//   return reader;
// }

size_t parse(char *line, request_t *request) {
  switch (request->state) {
  case INITIALIZED: {
    size_t bytes_consumed = parse_request_line(line, request);

    if (bytes_consumed == 0) {
      return 0;
    }
    request->state = PARSING_HEADERS;
    return bytes_consumed;
  }
  case PARSING_HEADERS: {
    size_t bytes_consumed = parse_header(line, request);

    if (bytes_consumed == 0) {
      return 0;
    }

    return bytes_consumed;
  }
  case PARSING_BODY: {
    size_t bytes_consumed = parse_body(line, request);

    if (bytes_consumed == 0) {
      return 0;
    }

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

  char *buffer = malloc(BYTES_READ + 1);
  if (buffer == NULL) {
    return NULL;
  }
  buffer[BYTES_READ] = '\0';

  char *line = malloc(LINE_SIZE * sizeof(char));
  if (line == NULL) {
    return NULL;
  }
  line[0] = '\0';

  request->request_line = request_line;
  request->state = INITIALIZED;
  request->headers = malloc(sizeof(array_t));

  if (request->headers == NULL) {
    return NULL;
  }
  request->body[0] = '\0';
  request->headers->capacity = 8;
  request->headers->count = 0;
  request->headers->elements =
      malloc(sizeof(header_t *) * request->headers->capacity);

  if (request->headers->elements == NULL) {
    return NULL;
  }
  size_t read_to_index = 0;
  size_t bytes_read = 0;
  size_t bytes_consumed = 0;

  while (request->state != DONE) {
    bytes_read = recv(client_fd, buffer, BYTES_READ, 0);
    if (bytes_read == 0) {
      break;
    }

    strncat(line, buffer, strlen(buffer));
    read_to_index += bytes_read;
    bytes_consumed = parse(line, request);

    if (bytes_consumed == 0) {
      continue;
    }

    char *start_new_part_ptr = line + bytes_consumed;

    char *new_line = malloc(LINE_SIZE * sizeof(char));
    new_line[0] = '\0';
    strncat(new_line, start_new_part_ptr, read_to_index - bytes_consumed);

    free(line);
    free(buffer);

    buffer = malloc(BYTES_READ + 1);
    buffer[BYTES_READ] = '\0';

    line = new_line;
    read_to_index -= bytes_consumed;
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
