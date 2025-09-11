#include "../munit/munit.h"
#include "request.h"

MunitResult my_test() {
	munit_assert_string_equal("Hello", "Hello");
	return MUNIT_OK;
}

MunitResult case_1() {
	request_line_t request_line = request_from_reader("GET /coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n");

	munit_assert_string_equal("GET", request_line.method);
	munit_assert_string_equal("/coffee", request_line.request_target);
	munit_assert_string_equal("1.1", request_line.http_version);
	
	free(request_line.method);
	free(request_line.request_target);
	free(request_line.http_version);

	return MUNIT_OK;
}

MunitResult case_2() {
	request_line_t request_line = request_from_reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n");

	munit_assert_string_equal("GET", request_line.method);
	munit_assert_string_equal("/", request_line.request_target);
	munit_assert_string_equal("1.1", request_line.http_version);
	
	free(request_line.method);
	free(request_line.request_target);
	free(request_line.http_version);

	return MUNIT_OK;
}

MunitResult case_3() {
	request_line_t request_line = request_from_reader("/coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n");
	
	munit_assert_not_null(request_line.http_version);
	munit_assert_not_null(request_line.method);	
	munit_assert_not_null(request_line.request_target);

	munit_assert_string_equal("GET", request_line.method);
	munit_assert_string_equal("/coffee", request_line.request_target);
	munit_assert_string_equal("1.1", request_line.http_version);
	
	free(request_line.method);
	free(request_line.request_target);
	free(request_line.http_version);

	return MUNIT_OK;
}

MunitResult case_4() {
	request_line_t request_line = request_from_reader("/coffee HTTP/1.1 GET\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n");
	
	munit_assert_not_null(request_line.http_version);
	munit_assert_not_null(request_line.method);	
	munit_assert_not_null(request_line.request_target);

	munit_assert_string_equal("GET", request_line.method);
	munit_assert_string_equal("/coffee", request_line.request_target);
	munit_assert_string_equal("1.1", request_line.http_version);
	
	free(request_line.method);
	free(request_line.request_target);
	free(request_line.http_version);

	return MUNIT_OK;
}

MunitResult case_5() {
	request_line_t request_line = request_from_reader("GET /coffee HTTP/1.2\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n");
	
	munit_assert_not_null(request_line.http_version);
	munit_assert_not_null(request_line.method);	
	munit_assert_not_null(request_line.request_target);

	munit_assert_string_equal("GET", request_line.method);
	munit_assert_string_equal("/coffee", request_line.request_target);
	munit_assert_string_equal("1.1", request_line.http_version);
	
	free(request_line.method);
	free(request_line.request_target);
	free(request_line.http_version);

	return MUNIT_OK;
}



int main() {
	MunitTest tests[] = {
		{
			"good GET request",
			case_1
		},
		{
			"good request line with path",
			case_2
		},
		{
			"invalid number of parts in request line",
			case_3
		},
{
			"invalid method (out of order) request line",
			case_4
		},
		{
			"invalid version in request line",
			case_5
		},

		{ NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
	};
	
	MunitSuite suite = {
		"[ request ]: ",
		tests
	};

	return munit_suite_main(&suite, NULL, 0, NULL); 
}
