/* 
 * File:   rubi_udp_hook.c
 * Author: ruslanti
 *
 * Created on October 30, 2013, 8:10 PM
 */
#include <linux/netfilter.h>

int rubi_udp_hook(struct sk_buff *skb) {
  return NF_ACCEPT;
}