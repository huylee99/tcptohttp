#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "ctype.h"
#include "headers.h"

void add_to_arr(header_t *header, array_t *array) {
	if (header == NULL || array == NULL) {
		fprintf(stderr, "add_to_arr: Error \n");
		exit(EXIT_FAILURE);
	}

	if (array->count == array->capacity) {
		array->capacity = array->capacity * 2;
		array->elements = realloc(array->elements, sizeof(header_t *) * array->capacity);
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

bool is_special_character_allowed(char field_name_char) {
	if (field_name_char == '!' || field_name_char == '#' || field_name_char == '$' || field_name_char == '%' || field_name_char == '$' || field_name_char == '\'' || field_name_char == '*' || field_name_char == '+' || field_name_char == '-' || field_name_char == '.' || field_name_char == '^' || field_name_char == '_' || field_name_char == '`' || field_name_char == '|' || field_name_char == '~') {
		return true;
	}

	return false;
}

bool is_valid_field_name(char* field_name) {
	if (field_name == NULL) {
		return false;
	}

	size_t length = strlen(field_name);
	
	if (length <= 0) {
		return false;
	}

	for (size_t i = 0; i < length; i++) {
		if (isalpha(field_name[i]) || isdigit(field_name[i]) || is_special_character_allowed(field_name[i])) {
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


header_t *parse_header(char *headers) {
	
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
	
	array_t *array = malloc(sizeof(array_t));
	//
	if (array == NULL) {
		exit(EXIT_FAILURE);
	}

	array->count = 0;
	array->capacity = 8;

	array->elements = malloc(sizeof(void *) * 8);

	if (array->elements == NULL) {
		free(array);
		exit(EXIT_FAILURE);
	}

	header_t *header = parse_header("Host: localhost:42069\r\n\r\n");

	add_to_arr(header, array);
	printf("%s \n", header->key);
	printf("%s \n", header->value);

	for (int i = 0; i < array->count; i++) {
		printf("key: %s - value: %s \n", array->elements[i]->key, array->elements[i]->value);
	}

	destroy_arr(array);

	return EXIT_SUCCESS;
}
