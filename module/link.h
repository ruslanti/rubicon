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

#define NETLINK_RUBICON         31
#define RUBINETLN_STATS         1
#define RUBY_NLTEST             0x11

#define ATTR_IP_SRC_ADDR        1
#define ATTR_IP_DST_ADDR        2
#define ATTR_TCP_SRC_PORT       3
#define ATTR_TCP_DST_PORT       4
#define ATTR_TCP_CONN_FLAG      5
#define ATTR_PKG_SIZE           6


struct conn_stat {
    u_int64_t conn_id;
    u_int8_t protocol;
};

int link_tcp_stat(struct nf_conn *ct, struct iphdr *ip, struct tcphdr *tcp);
void link_callback(struct sk_buff *skb);
int link_init(void);
void link_release(void);

#endif	/* LINK_H */

