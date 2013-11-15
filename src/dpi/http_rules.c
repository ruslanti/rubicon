/*
 * http_rules.c
 *
 *  Created on: Nov 15, 2013
 *      Author: ruslanti
 */

#ifndef HTTP_RULES_C_
#define HTTP_RULES_C_

#include <stdio.h>

#include "basic_rules.h"

// HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
char* parse_http_version(unsigned char** pstr)
{
    const unsigned char STR_HTTP[] = "HTTP/";

    unsigned char* ptmp = *pstr;

    if(parse_string(pstr, STR_HTTP, sizeof(STR_HTTP)-1, 1))
        return (char*)ERR;

    int failed = 0;
    if(parse_1_digit(pstr))
        failed = 1;

    if(!failed && parse_char(pstr, '.'))
        failed = 1;

    if(!failed && parse_1_digit(pstr))
        failed = 1;

    if(failed)
    {
        *pstr = ptmp;
        return ERR;
    }

    return NULL;
}

#endif /* HTTP_RULES_C_ */
