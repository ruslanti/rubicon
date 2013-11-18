/*
 * http_rules.c
 *
 *  Created on: Nov 15, 2013
 *      Author: ruslanti
 */

#ifndef HTTP_RULES_C_
#define HTTP_RULES_C_

#include <stdio.h>
#include <string.h>

#include "basic_rules.h"
#include "http_rules.h"

const char GET[] 		= "GET";
const char PUT[] 		= "PUT";
const char POST[] 		= "POST";
const char HEAD[] 		= "HEAD";
const char OPTIONS[] 	= "OPTIONS";
const char DELETE[] 	= "DELETE";
const char CONNECT[] 	= "CONNECT";
const char TRACE[] 		= "TRACE";

// HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
const char* parse_http_version(unsigned char** p)
{
    const unsigned char STR_HTTP[] = "HTTP/";

    unsigned char* ptmp = *p;

    if(parse_string(p, STR_HTTP, sizeof(STR_HTTP)-1, 1))
        return (char*)ERR;

    int failed = 0;
    if(parse_1_digit(p))
        failed = 1;

    if(!failed && parse_char(p, '.'))
        failed = 1;

    if(!failed && parse_1_digit(p))
        failed = 1;

    if(failed)
    {
        *p = ptmp;
        return ERR;
    }

    return NULL;
}

// content-coding   = token
const char* parse_content_coding(unsigned char** p)
{
	return parse_token(p);
}

// value        = token | quoted-string
const char* parse_value(unsigned char** p)
{
	if (!parse_token(p))
		return NULL;
	return parse_quoted_string(p);
}

// parameter    = attribute "=" value
// attribute    = token
const char* parse_parameter(unsigned char** p)
{
	if (parse_token(p))
		return ERR;
	if (parse_char(p, '='))
		return ERR;
	if (parse_value(p))
		return ERR;
	return NULL;
}

// transfer-extension      = token *( ";" parameter )
const char* parse_transfer_extension(unsigned char** p)
{
	if (parse_token(p))
		return ERR;
	while (0 != (char)**p) {
		if (parse_char(p, ';'))
			break;
		if (parse_parameter(p))
			return ERR;
	}
	return NULL;
}

// transfer-coding         = "chunked" | transfer-extension
const char* parse_transfer_coding(unsigned char** p)
{
	const unsigned char STR_CHUNKED[] = "chunked";
	if(!parse_string(p, STR_CHUNKED, sizeof(STR_CHUNKED)-1, 1))
		return NULL;

	return parse_transfer_extension(p);
}




// media-type     = type "/" subtype *( ";" parameter )
// type           = token
// subtype        = token
const char* parse_media_type(unsigned char** p)
{
	if (parse_token(p))
		return ERR;
	if (parse_char(p, '/'))
		return ERR;
	if (parse_token(p))
		return ERR;

	while (0 != (char)**p) {
		if (parse_char(p, ';'))
			break;
		if (parse_parameter(p))
			return ERR;
	}
	return NULL;
}

// generic-message = start-line
//                   *(message-header CRLF)
//                   CRLF
//                   [ message-body ]
const char* parse_generic_message(unsigned char** p)
{
	if (parse_start_line(p))
		return ERR;

	while (0 != (char)**p) {
		if (parse_message_header(p))
			break;
		if (parse_crlf(p))
			return ERR;
	}

	if (parse_crlf(p))
		return ERR;

	return parse_message_body(p);
}

const char* parse_message_body(unsigned char** p)
{
	return NULL;
}

// start-line      = Request-Line | Status-Line
const char* parse_start_line(unsigned char** p)
{
	if (!parse_request_line(p))
		return NULL;
	return parse_status_line(p);
}

// message-header = field-name ":" [ field-value ]
// field-name     = token
// field-value    = *( field-content | LWS )
// field-content  = <the OCTETs making up the field-value
//                 and consisting of either *TEXT or combinations
//                 of token, separators, and quoted-string>
const char* parse_message_header(unsigned char** p)
{
	if (parse_token(p))
		return ERR;
	if (parse_char(p, ':'))
		return ERR;

	while (0 != (char)**p) {
		if (parse_text(p))
			break;
	}
	return NULL;
}

// Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
const char* parse_request_line(unsigned char** p)
{
	if (parse_method(p))
		return ERR;
	if (parse_space(p))
		return ERR;
	if (parse_request_uri(p))
		return ERR;
	if (parse_space(p))
		return ERR;
	if (parse_http_version(p))
		return ERR;
	return parse_crlf(p);
}

//  Method      = "OPTIONS"
//				| "GET"
//				| "HEAD"
//				| "POST"
//				| "PUT"
//				| "DELETE"
//				| "TRACE"
//				| "CONNECT"
//				| extension-method
//	extension-method = token
const char* parse_method(unsigned char** p)
{
	int len = 0;
	if (0 == memcmp(*p, GET, strlen(GET)))
	{
		len = strlen(GET);
	}
	else if (0 == memcmp(*p, PUT, strlen(PUT)))
	{
		len = strlen(PUT);
	}
	else if (0 == memcmp(*p, POST, strlen(POST)))
	{
		len = strlen(POST);
	}
	else if (0 == memcmp(*p, HEAD, strlen(HEAD)))
	{
		len = strlen(HEAD);
	}
	else if (0 == memcmp(*p, OPTIONS, strlen(OPTIONS)))
	{
		len = strlen(OPTIONS);
	}
	else if (0 == memcmp(*p, DELETE, strlen(DELETE)))
	{
		len = strlen(DELETE);
	}
	else if (0 == memcmp(*p, CONNECT, strlen(CONNECT)))
	{
		len = strlen(CONNECT);
	}
	else if (0 == memcmp(*p, TRACE, strlen(TRACE)))
	{
		len = strlen(TRACE);
	}
	else
	{
		if (parse_token(p))
			return ERR;
		else
			return NULL;
	}
	*p += len;
	return NULL;
}

// Request-URI    = "*" | absoluteURI | abs_path | authority
const char* parse_request_uri(unsigned char** p)
{
	// TODO
	return NULL;
}

// Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
const char* parse_status_line(unsigned char** p)
{
	if (parse_http_version(p))
		return ERR;
	if (parse_space(p))
		return ERR;
	if (parse_status_code(p))
		return ERR;
	if (parse_space(p))
		return ERR;
	if (parse_reason_phrase(p))
		return ERR;
	return parse_crlf(p);
}

// Status-Code = 3DIGIT
const char* parse_status_code(unsigned char** p)
{
	return parse_digit_min_max(p, 3, 3);
}

// Reason-Phrase  = *<TEXT, excluding CR, LF>
const char* parse_reason_phrase(unsigned char** p)
{
	return parse_text(p);
}

#endif /* HTTP_RULES_C_ */
