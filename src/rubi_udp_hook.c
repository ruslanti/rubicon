/* 
 * File:   rubi_udp_hook.c
 * Author: ruslanti
 *
 * Created on October 30, 2013, 8:10 PM
 */
#include <linux/netfilter.h>

static inline int rubi_upd_hook(unsigned char *udp) {
  return NF_ACCEPT;
}