/* 
 * File:   link.h
 * Author: ruslanti
 *
 * Created on March 31, 2014, 5:39 PM
 */

#ifndef LINK_H
#define	LINK_H

#ifndef NETLINK_RUBICON
#define NETLINK_RUBICON 31
#endif

#ifndef RUBINETLN_STATS
#define RUBINETLN_STATS 1
#endif

#ifndef RUBY_NLTEST
#define RUBY_NLTEST     0x11
#endif

struct tcp_stat {
    int a1;
    int a2;
};

int link_tcp_stat(int pid);
void link_callback(struct sk_buff *skb);
int link_init(void);
void link_release(void);

#endif	/* LINK_H */

