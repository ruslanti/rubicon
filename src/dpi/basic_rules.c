/*
 * basic_rules.c
 *
 *  Created on: Nov 15, 2013
 *      Author: ruslanti
 */
#include <stdio.h>

#include "basic_rules.h"

const char CRLF[] = "\x0D\x0A";
const char ERR[] = "fail";

char *parse_char(unsigned char **p, unsigned char ch)
{
	if (ch == **p)
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

char *parse_string(unsigned char **p, const unsigned char *tofind, int len,
		   int case_sensitive)
{
	int ret = -1;
	if (case_sensitive)
		ret = memcmp(*p, tofind, len);
	else
		ret = strncasecmp(*p, tofind, len);

	if (0 == ret)
		(*p) += len;
	else
		return (char *)ERR;

	return NULL;
}

// CHAR = <any US-ASCII character (octets 0 - 127)>
char *parse_ascii(unsigned char **p)
{
	if (**p >= 0 && **p <= 127)
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// UPALPHA = <any US-ASCII uppercase letter "A".."Z">
char *parse_upalpha(unsigned char **p)
{
	if (**p >= 'A' && **p <= 'Z')
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// LOALPHA = <any US-ASCII lowercase letter "a".."z">
char *parse_loalpha(unsigned char **p)
{
	if (**p >= 'a' && **p <= 'z')
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// ALPHA = UPALPHA | LOALPHA
char *parse_alpha(unsigned char **p)
{
	if ((**p >= 'a' && **p <= 'z') || (**p >= 'A' && **p <= 'Z'))
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

char *parse_alphanum(unsigned char **p)
{
	if ((**p >= 'a' && **p <= 'z') || (**p >= 'A' && **p <= 'Z')
	    || (**p >= '0' && **p <= '9'))
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// DIGIT = <any US-ASCII digit "0".."9">
char *parse_digit(unsigned char **p)
{
	if (**p >= '0' && **p <= '9') {
		(*p)++;
		return NULL;
	}

	return (char *)ERR;
}

// CTL = <any US-ASCII control character (octets 0 - 31) and DEL (127)>
char *parse_ctl(unsigned char **p)
{
	if ((**p >= '\x00' && **p <= '\x1F') || **p == '\x7F') {
		(*p)++;
		return NULL;
	}

	return (char *)ERR;
}

// CR = <US-ASCII CR, carriage return (13)>
char *parse_carriage_return(unsigned char **p)
{
	if (CR == **p)
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// LF = <US-ASCII LF, linefeed (10)>
char *parse_linefeed(unsigned char **p)
{
	if (LF == **p)
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// SP = <US-ASCII SP, space (32)>
char *parse_space(unsigned char **p)
{
	if (SP == **p)
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// HTAB = <US-ASCII HTAB, horizontal-tab (9)>
char *parse_tab(unsigned char **p)
{
	if (HTAB == **p)
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// CRLF = CR LF
char *parse_crlf(unsigned char **p)
{
	return parse_string(p, CRLF, 2, PARSE_TRUE);
}

// <"> = <US-ASCII double-quote mark (34)>
char *parse_dquote(unsigned char **p)
{
	if (DQUOTE == **p)
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// WSP = SP / HTAB
char *parse_wsp(unsigned char **p)
{
	if (SP == **p || HTAB == **p)
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// LWS = [*WSP CRLF] 1*WSP
// LWS = [CRLF] 1*WSP
char *parse_lws(unsigned char **p)
{
	unsigned char *ptmp = *p;
	int arr[3][4] = { {1, 1, 3, 3}, {2, 2, -1, -1}, {-1, 4, -1, 4} };

	int line = 0, state = 0;
	do {
		if (!parse_wsp(p))
			line = 0;
		else if (!parse_crlf(p))
			line = 1;
		else
			line = 2;

		state = arr[line][state];
		if (-1 == state) {
			*p = ptmp;
			return (char *)ERR;
		}
	}
	while (state != 4);

	return NULL;
}

// Rule text = byte-string
//       byte-string = 1*(0x01..0x09|0x0b|0x0c|0x0e..0xff)         ;any byte except NUL, CR or LF
// TEXT = <any OCTET except CTLs, but including LWS>
char *parse_text(unsigned char **p)
{
	int len = 0;
	while (0 != (char)**p) {
		if (!parse_lws(p)) {
			len++;
			continue;
		} else if (is_ctl(**p)) {
			break;
		}
		len++;
		(*p)++;
	}

	return len ? NULL : (char *)ERR;
}

// HEX = "A" | "B" | "C" | "D" | "E" | "F"| "a" | "b" | "c" | "d" | "e" | "f" | DIGIT
char *parse_hex_digit(unsigned char **p)
{
	if ((**p >= 'A' && **p <= 'F') || (**p >= 'a' && **p <= 'f')
	    || (**p >= '0' && **p <= '9'))
		(*p)++;
	else
		return (char *)ERR;

	return NULL;
}

// token = 1*<any CHAR except CTLs or separators>
char *parse_token(unsigned char **p)
{
	int len = 0;
	while (1) {
		if (is_char(**p) && !(is_ctl(**p) || is_separator(**p))) {
			(*p)++;
			len++;
			continue;
		} else {
			if (len > 0) {
				return NULL;
			}
			return (char *)ERR;
		}
	}
}

// separators     = "(" | ")" | "<" | ">" | "@" | "," | ";" | ":" | "\" | <"> |
//                  "/" | "[" | "]" | "?" | "=" | "{" | "}" | SP | HTAB
char *parse_separator(unsigned char **p)
{
	switch (**p) {
	case '(':
	case ')':
	case '<':
	case '>':
	case '@':
	case ',':
	case ';':
	case ':':
	case '\\':
	case DQUOTE:
	case '/':
	case '[':
	case ']':
	case '?':
	case '=':
	case '{':
	case '}':
	case SP:
	case HTAB:
		(*p)++;
		break;
	default:
		return (char *)ERR;
	}

	return NULL;
}

// comment = "(" *( ctext | quoted-pair | comment ) ")"
char *parse_comment(unsigned char **p)
{
	if (parse_char(p, '('))
		return (char *)ERR;

	if (!parse_char(p, ')'))
		return NULL;

	int len = 0;

	while(0 != (char)**p)
	{
		if (!parse_ctext(p))
		{
			len++;
			continue;
		}
		else if (!parse_quoted_pair(p))
		{
			len++;
			continue;
		}
		else if (!parse_comment(p))
		{
			len++;
			continue;
		}
		else
		{
			break;
		}
	}

	if (!parse_char(p, ')'))
		return NULL;
	else
		return (char *)ERR;
}

// ctext = <any TEXT excluding "(" and ")">
char *parse_ctext(unsigned char **p)
{
	int len = 0;
	while (0 != (char)**p) {
		if (!parse_lws(p)) {
			len++;
			continue;
		} else if (is_ctl(**p) || **p == '(' || **p == ')') {
			break;
		}
		len++;
		(*p)++;
	}

	return len ? NULL : (char *)ERR;
}

// quoted-pair    = "\" CHAR
char *parse_quoted_pair(unsigned char **p)
{
	if (parse_char(p, '\\'))
		return (char *)ERR;

	if (is_char(**p))
		(*p)++;
	else {
		(*p)--;
		return (char *)ERR;
	}

	return NULL;
}

// quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
char *parse_quoted_string(unsigned char **p)
{
	unsigned char *ptmp = *p;
	int arr[4][4] =
	    { {1, -1, -1, -1},
	      {-1, 2, 99, 99},
	      {-1, -1, 3, 3},
	      {-1, -1, -1,-1} };

	int line = 0, state = 0;
	do {
		if (state == 0 && !parse_lws(p))
			line = 0;
		else if (!parse_dquote(p))
			line = 1;
		else if (!parse_qdtext(p) || parse_quoted_pair(p))
			line = 2;
		else
			line = 3;

		state = arr[line][state];
		if (-1 == state) {
			*p = ptmp;
			return (char *)ERR;
		}
	}
	while (state != 99);

	return NULL;
}

// qdtext = <any TEXT except <">>
char *parse_qdtext(unsigned char **p)
{
	int len = 0;
	while (0 != (char)**p) {
		if (!parse_lws(p)) {
			len++;
			continue;
		} else if (is_ctl(**p) || DQUOTE == **p) {
			break;
		}
		len++;
		(*p)++;
	}

	return len ? NULL : (char *)ERR;
}
