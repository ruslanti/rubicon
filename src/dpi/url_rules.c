/*
 * url_rules.c
 *
 *  Created on: Nov 19, 2013
 *      Author: ruslanti
 */
#include <stdio.h>
#include "basic_rules.h"
#include "url_rules.h"

// HTTP
// httpurl        = "http://" hostport [ "/" hpath [ "?" search ]]
const char* parse_httpurl(unsigned char** p)
{
	const unsigned char STR_HTTP[] = "http://";
	if (parse_string(p, STR_HTTP, sizeof(STR_HTTP)-1, 0))
		return ERR;
	if (parse_hostport(p))
		return ERR;
	if (parse_char(p, '/'))
		return NULL;
	if (parse_hpath(p))
		return ERR;
	if (parse_char(p, '?'))
		return NULL;
	if (parse_search(p))
		return ERR;
	return NULL;
}

// hpath          = hsegment *[ "/" hsegment ]
const char* parse_hpath(unsigned char** p)
{
	if (parse_hsegment(p))
		return ERR;
	while (0 != (char)**p) {
		if (parse_char(p, '/'))
			break;
		if (parse_hsegment(p))
			break;
	}
	return NULL;
}

// hex            = digit | "A" | "B" | "C" | "D" | "E" | "F" | "a" | "b" | "c" | "d" | "e" | "f"
// escape         = "%" hex hex
const char* parse_escape(unsigned char** p)
{
	if (parse_char(p, '%'))
		return ERR;

	if ((**p >= 'A' && **p <= 'F') || (**p >= 'a' && **p <= 'f') || (**p >= '0' && **p <= '9'))
		(*p)++;
	else
		return ERR;

	if ((**p >= 'A' && **p <= 'F') || (**p >= 'a' && **p <= 'f') || (**p >= '0' && **p <= '9'))
		(*p)++;
	else
		return ERR;

	return NULL;
}

// uchar          = unreserved | escape
const char* parse_uchar(unsigned char** p)
{
	if (is_unreserved(**p)) {
		(*p)++;
		return NULL;
	}
	return parse_escape(p);
}

// hsegment       = *[ uchar | ";" | ":" | "@" | "&" | "=" ]
const char* parse_hsegment(unsigned char** p)
{
	while (0 != (char)**p) {
		if (parse_uchar(p)) {
			if (**p != ';' && **p != ':' && **p != '@' && **p != '&' && **p != '=')
				break;
		}
	}
	return NULL;
}

// search         = *[ uchar | ";" | ":" | "@" | "&" | "=" ]
const char* parse_search(unsigned char** p)
{
	return parse_hsegment(p);
}

// hostport       = host [ ":" port ]
const char* parse_hostport(unsigned char** p)
{
	if (parse_host(p))
		return ERR;
	if (!parse_char(p, ':')) {
		return parse_digits(p);
	}
	return NULL;
}

// host           = hostname | hostnumber
const char* parse_host(unsigned char** p)
{
	if (parse_hostname(p))
		return parse_hostnumber(p);
	return NULL;
}

// hostname       = *[ domainlabel "." ] toplabel
const char* parse_hostname(unsigned char** p)
{
	while (0 != (char)**p) {
		if (parse_domainlabel(p))
			break;
		if (parse_char(p, '.'))
			return ERR;
	}
	return parse_toplabel(p);
}

// domainlabel    = alphadigit | alphadigit *[ alphadigit | "-" ] alphadigit
const char* parse_domainlabel(unsigned char** p)
{
	if (parse_alphanum(p))
		return ERR;

	int len = 0;
	while (0 != (char)**p) {
		if (parse_alphanum(p)) {
			if (parse_char(p, '-'))
				break;
		}
		len++;
	}

	if (len > 0)
		return parse_alphanum(p);

	return NULL;
}

// toplabel       = alpha | alpha *[ alphadigit | "-" ] alphadigit
const char* parse_toplabel(unsigned char** p)
{
	if (parse_alpha(p))
		return ERR;

	int len = 0;
	while (0 != (char)**p) {
		if (parse_alphanum(p)) {
			if (parse_char(p, '-'))
				break;
		}
		len++;
	}

	if (len > 0)
		return parse_alphanum(p);

	return NULL;
}

// hostnumber     = digits "." digits "." digits "." digits
const char* parse_hostnumber(unsigned char** p)
{
	if (parse_digits(p))
		return ERR;
	if (parse_char(p, '.'))
		return ERR;
	if (parse_digits(p))
		return ERR;
	if (parse_char(p, '.'))
		return ERR;
	if (parse_digits(p))
		return ERR;
	if (parse_char(p, '.'))
		return ERR;
	if (parse_digits(p))
		return ERR;
	return NULL;
}

// port           = digits

