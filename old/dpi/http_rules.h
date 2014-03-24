/*
 * http_rules.h
 *
 *  Created on: Nov 15, 2013
 *      Author: ruslanti
 */

#ifndef HTTP_RULES_H_
#define HTTP_RULES_H_

extern const char GET[];
extern const char PUT[];
extern const char POST[];
extern const char HEAD[];
extern const char OPTIONS[];
extern const char DELETE[];
extern const char CONNECT[];
extern const char TRACE[];

const char* parse_http_version(unsigned char** p);
const char* parse_content_coding(unsigned char** p);
const char* parse_transfer_coding(unsigned char** p);
const char* parse_transfer_extension(unsigned char** p);
const char* parse_parameter(unsigned char** p);
const char* parse_value(unsigned char** p);
const char* parse_media_type(unsigned char** p);
const char* parse_generic_message(unsigned char** p);
const char* parse_start_line(unsigned char** p);
const char* parse_message_header(unsigned char** p);
const char* parse_request_line(unsigned char** p);
const char* parse_method(unsigned char** p);
const char* parse_request_uri(unsigned char** p);
const char* parse_status_line(unsigned char** p);
const char* parse_status_code(unsigned char** p);
const char* parse_reason_phrase(unsigned char** p);
const char* parse_message_body(unsigned char** p);

#endif /* HTTP_RULES_H_ */
