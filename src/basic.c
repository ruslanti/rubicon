/*
 * 
 */

#include "basic.h"

inline int is_char(char* p) {
  return (*p >= 0 && *p <= 127);
}

inline int is_upalpha(char* p) {
  return (*p >= 'A' && *p <= 'Z');
}

inline int is_lopalpha(char* p) {
  return (*p >= 'a' && *p <= 'z');
}

inline int is_alpha(char* p) {
  return is_upalpha(p) | is_lopalpha(p);
}

inline int is_digit(char* p) {
  return (*p >= '0' && *p <= '9');
}

inline int is_ctl(char* p) {
  return ((*p >= 0 && *p <= 31) || *p == 127);
}

inline int is_cr(char* p) {
  return (*p == 13);
}

inline int is_lf(char* p) {
  return (*p == 10);
}

inline int is_sp(char* p) {
  return (*p == 32);
}

inline int is_ht(char* p) {
  return (*p == 9);
}

inline int is_double_quote(char* p) {
  return (*p == 34);
}


inline int is_crlf(char* p) {
  return (*p == 34);
}
