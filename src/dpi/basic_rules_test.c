/*
 * basic_rules_test.c
 *
 *  Created on: Nov 15, 2013
 *      Author: ruslanti
 */

#include <stdio.h>
#include "minunit.h"

int tests_run = 0;

#define PARSE_ASSERT(testcase, target, string)	static char *test_##testcase(){		\
	printf("[%s]\n", string);														\
	do { if (parse_##target(&string)) return #testcase; } while (0);				\
	return 0; 																		\
	}

#define PARSE_FAIL(testcase, target, string)	static char *fail_##testcase(){		\
	printf("[%s]\n", string);														\
	do { if (!parse_##target(&string)) return #testcase; } while (0);				\
	return 0; 																		\
	}

char *test = "a2\n\r\n \t\r\n  t  \r\n  Xlongtext\r\n  including LWS\x11\"Dtoken<A(text)(comment2(test))(fail comment";

PARSE_ASSERT(alpha, alpha, test)
PARSE_ASSERT(digit, digit, test)
PARSE_FAIL(digit, digit, test)
PARSE_ASSERT(ctl, ctl, test)
PARSE_ASSERT(carriage_return, carriage_return, test)
PARSE_ASSERT(linefeed, linefeed, test)
PARSE_ASSERT(space, space, test)
PARSE_ASSERT(tab, tab, test)
PARSE_ASSERT(crlf, crlf, test)
PARSE_FAIL(crlf, crlf, test)
PARSE_ASSERT(lws, lws, test)
PARSE_ASSERT(loalpha, loalpha, test)
PARSE_ASSERT(lws2, lws, test)
PARSE_ASSERT(upalpha, upalpha, test)
PARSE_ASSERT(text, text, test)
PARSE_ASSERT(ctl2, ctl, test)
PARSE_ASSERT(dquote, dquote, test)
PARSE_ASSERT(hex_digit, hex_digit, test)
PARSE_ASSERT(token, token, test)
PARSE_ASSERT(separator, separator, test)
PARSE_FAIL(separator, separator, test)
PARSE_ASSERT(hex_digit2, hex_digit, test)
PARSE_ASSERT(comment, comment, test)
PARSE_ASSERT(comment2, comment, test)
PARSE_FAIL(comment, comment, test)

static char *all_tests()
{
	mu_run_test(test_alpha);
	mu_run_test(test_digit);
	mu_run_test(fail_digit);
	mu_run_test(test_ctl);
	mu_run_test(test_carriage_return);
	mu_run_test(test_linefeed);
	mu_run_test(test_space);
	mu_run_test(test_tab);
	mu_run_test(test_crlf);
	mu_run_test(fail_crlf);
	mu_run_test(test_lws);
	mu_run_test(test_loalpha);
	mu_run_test(test_lws2);
	mu_run_test(test_upalpha);
	mu_run_test(test_text);
	mu_run_test(test_ctl2);
	mu_run_test(test_dquote);
	mu_run_test(test_hex_digit);
	mu_run_test(test_token);
	mu_run_test(test_separator);
	mu_run_test(fail_separator);
	mu_run_test(test_hex_digit2);
	mu_run_test(test_comment);
	mu_run_test(test_comment2);
	mu_run_test(fail_comment);
	return 0;
}

int main(int argc, char **argv)
{
	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	} else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}
