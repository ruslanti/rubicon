/*
 * basic_rules.h
 *
 *  Created on: Nov 15, 2013
 *      Author: ruslanti
 */

#ifndef BASIC_RULES_H_
#define BASIC_RULES_H_

#define SP		'\x20'
#define HTAB	'\x09'
#define DQUOTE	'\x22'
#define CR		'\x0D'
#define LF		'\x0A'

#define PARSE_TRUE	1

extern const char CRLF[];
extern const char ERR[];

static inline int is_alpha(unsigned char ch)
{
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

static inline int is_digit(unsigned char ch)
{
	return (ch >= '0' && ch <= '9');
}

static inline int is_hex_digit(unsigned char ch)
{
	return (ch >= 'A' && ch <= 'F') || (ch >= '0' && ch <= '9');
}

static inline int is_char(unsigned char ch)
{
	return (ch >= '\x00' && ch <= '\x7F');
}

static inline int is_ctl(unsigned char ch)
{
	return (ch >= '\x00' && ch <= '\x1F') || (ch == '\x7f');
}

static inline int is_separator(unsigned char ch)
{
	return ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '@'
	    || ch == ',' || ch == ';' || ch == ':' || ch == '\\' || ch == DQUOTE
	    || ch == '/' || ch == '[' || ch == ']' || ch == '?' || ch == '='
	    || ch == '{' || ch == '}' || ch == SP || ch == HTAB;
}

const char* parse_char(unsigned char **p, unsigned char ch);
const char* parse_string(unsigned char **p, const char *tofind, int len,
		   int case_sensitive);
const char* parse_ascii(unsigned char **p);
const char* parse_upalpha(unsigned char **p);
const char* parse_loalpha(unsigned char **p);
const char* parse_alpha(unsigned char **p);
const char* parse_alphanum(unsigned char **p);
const char* parse_digit(unsigned char **p);
const char* parse_digits(unsigned char** p);
const char* parse_digit_min_max(unsigned char** p, int min, int max);
const char* parse_ctl(unsigned char **p);
const char* parse_carriage_return(unsigned char **p);
const char* parse_linefeed(unsigned char **p);
const char* parse_space(unsigned char **p);
const char* parse_tab(unsigned char **p);
const char* parse_crlf(unsigned char **p);
const char* parse_dquote(unsigned char **p);
const char* parse_wsp(unsigned char **p);
const char* parse_lws(unsigned char **p);
const char* parse_text(unsigned char **p);
const char* parse_hex_digit(unsigned char **p);
const char* parse_token(unsigned char **p);
const char* parse_separator(unsigned char **p);
const char* parse_comment(unsigned char **p);
const char* parse_ctext(unsigned char **p);
const char* parse_quoted_pair(unsigned char **p);
const char* parse_quoted_string(unsigned char **p);
const char* parse_qdtext(unsigned char **p);

#endif /* BASIC_RULES_H_ */
