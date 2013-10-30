/* 
 * File:   rubi_tcp_hook.c
 * Author: ruslanti
 *
 * Created on October 30, 2013, 8:11 PM
 */
#include <linux/netfilter.h>

static inline int rubi_tcp_hook(unsigned char *tcp) {
  return NF_ACCEPT;
}