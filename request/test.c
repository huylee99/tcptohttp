#include "string.h"
#include "stdio.h"


int main() {
	char *string, *found;

	string = strdup("HTTP/1.1");
	printf("%s\n", string);

	found = strsep(&string, "/");
	printf("%s\n", found);

	found = strsep(&string, "/");
	printf("%s\n", found);


	return 0;
}
