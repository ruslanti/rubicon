/* 
 * File:   link.h
 * Author: ruslanti
 *
 * Created on March 31, 2014, 5:39 PM
 */

#include <linux/ip.h>
#include <linux/tcp.h>

#ifndef LINK_H
#define	LINK_H

int link_tcp_stat(struct sk_buff *skb);
void link_callback(struct sk_buff *skb);
int link_init(void);
void link_release(void);

#endif	/* LINK_H */

