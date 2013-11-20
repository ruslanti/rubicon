/*
 * url_rules.h
 *
 *  Created on: Nov 19, 2013
 *      Author: ruslanti
 */

#ifndef URL_RULES_H_
#define URL_RULES_H_

// safe           = "$" | "-" | "_" | "." | "+"
static inline int is_safe(unsigned char ch)
{
	return ch == '$' || ch == '-' || ch == '_' || ch == '.' || ch == '+';
}

// extra          = "!" | "*" | "'" | "(" | ")" | ","
static inline int is_extra(unsigned char ch)
{
	return ch == '!' || ch == '*' || ch == '\'' || ch == '(' || ch == ')' || ch == ',';
}

// unreserved     = alpha | digit | safe | extra
static inline int is_unreserved(unsigned char ch)
{
	return is_alpha(ch) || is_digit(ch) || is_safe(ch);
}

const char* parse_httpurl(unsigned char** p);
const char* parse_hpath(unsigned char** p);
const char* parse_escape(unsigned char** p);
const char* parse_uchar(unsigned char** p);
const char* parse_hsegment(unsigned char** p);
const char* parse_search(unsigned char** p);
const char* parse_hostport(unsigned char** p);
const char* parse_host(unsigned char** p);
const char* parse_hostname(unsigned char** p);
const char* parse_domainlabel(unsigned char** p);
const char* parse_toplabel(unsigned char** p);
const char* parse_hostnumber(unsigned char** p);

#endif /* URL_RULES_H_ */
