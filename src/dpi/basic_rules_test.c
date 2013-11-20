/*
 * basic_rules_test.c
 *
 *  Created on: Nov 15, 2013
 *      Author: ruslanti
 */

#include <stdio.h>
#include "minunit.h"
#include "basic_rules.h"
#include "http_rules.h"

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

unsigned char *test = "a2\n\r\n \t\r\n  t  \r\n  Xlongtext\r\n  including LWS\x11\"Dtoken<A(text)(comment2(test))(fail comment";

unsigned char *get_http_1 = "GET http://www.yahoo.com HTTP/1.1\r\n"
		"User-Agent: Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.15\r\n"
		"Host: yahoo.com\r\n"
		"Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/webp, image/jpeg, image/gif, image/x-xbitmap, */*;q=0.1\r\n"
		"Accept-Language: en-US,en;q=0.9\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Cookie: ucs=bnas=0; AO=o=0; YLS=v=1&p=0&n=0; B=7eopdkl8n9ved&b=4&d=2RKTEPJpYEKzBM_ZCCrs_w--&s=3v&i=ShHlxDorPb1zfFJkUn5h; F=a=rE8YZCkMvSlv5uvR29EFFBi2RF11SWomYDW_9Yqai7PwVygjZtk54P4rAiDQdrsW0Tb0nzn8lkDEIF9.sDKCN.kEJg--&b=wYf7; PH=l=en-US%2Cro-RO&i=us&fn=D7ef21eyv991l84vf0Q-; uvts=HRJCdL8XnP6iPjf; U=mt=jcfs4Z2MhYi9.3er2uYOnAK910qrBZqsl0OZHw--&ux=t9QeSB&un=6klo2idggoveg\r\n"
		"Connection: Keep-Alive\r\n"
		"\r\n";

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

PARSE_ASSERT(get_http_1, generic_message, get_http_1)

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

	mu_run_test(test_get_http_1);
	return 0;
}

int main(int argc, char **argv)
{
	char *result = all_tests();
	if (result != 0) {
		printf("FAIL %s\n", result);
	} else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}
