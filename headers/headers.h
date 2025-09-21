#include "stddef.h"
#include "stdbool.h"

typedef struct Header {
	char key[64];
	char value[128];
} header_t;

typedef struct Array {
	size_t count;
	size_t capacity;
	header_t **elements;
} array_t;

void add_to_arr(header_t *header, array_t *array);
void destroy_arr(array_t *array);
bool is_special_character_allowed(char field_name_char);
bool is_valid_field_name(char* field_name);
char *to_lower_case(char* field_name);
header_t *is_header_key_existed(char * field_name, array_t *arr);
void add_value_to_header(header_t *header, array_t *arr);
header_t *parse_header(char *headers);


char *trim(char *buffer);

