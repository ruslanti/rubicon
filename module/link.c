/* 
 * File:   link.c
 * Author: ruslanti
 *
 * Created on March 31, 2014, 5:39 PM
 */

#include <linux/netfilter.h>
#include <linux/netlink.h>

#include <net/netlink.h>
#include <net/netfilter/nf_conntrack.h>

#include "link.h"

struct sock *nl_sk;

int pid;

int link_tcp_stat(struct nf_conn *ct, struct iphdr *ip, struct tcphdr *tcp) {
    struct nlmsghdr * nlh;
    struct sk_buff * skb;
    int res;
    struct conn_stat *t;
    
    skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_ATOMIC);
    if (!skb) {
        printk(KERN_WARNING "Cannot alloc sk_buff with nlmsg_new()\n");
        return -ENOMEM;
    }
    
    nlh = nlmsg_put(skb, 0, 0, RUBY_NLTEST, sizeof(struct conn_stat), 0);
    if (!nlh)
      goto nlmsg_failure;

    t = (struct conn_stat*)nlmsg_data(nlh);
    t->conn_id = (u_int64_t)ct;
    t->protocol = IPPROTO_TCP;
    
    if (nla_put_net32(skb, ATTR_IP_SRC_ADDR, ip->saddr))
        goto nla_put_failure;
    
    if (nla_put_net32(skb, ATTR_IP_DST_ADDR, ip->daddr))
        goto nla_put_failure;
    
    if (nla_put_net16(skb, ATTR_TCP_SRC_PORT, tcp->source))
        goto nla_put_failure;
    
    if (nla_put_net16(skb, ATTR_TCP_DST_PORT, tcp->dest))
        goto nla_put_failure;
    
    if (nla_put_flag(skb, ATTR_TCP_CONN_FLAG))
        goto nla_put_failure;
    

    res = nlmsg_end(skb, nlh);
    printk(KERN_INFO "Prepared %d bytes to send (%d) \n", res, nlmsg_len(nlh));
    
    if (pid)
        res = nlmsg_unicast(nl_sk, skb, pid);
    else
        res = nlmsg_multicast(nl_sk, skb, pid, RUBINETLN_STATS, GFP_ATOMIC);
    
    if (res < 0) {
        printk(KERN_WARNING "Error while sending message: %d\n", res);
    }
    return res;
        
    nlmsg_failure:
    nla_put_failure:
        nlmsg_cancel(skb, nlh);
        kfree_skb(skb);
        return -ENOMEM;
}

static int link_step(struct sk_buff *skb, struct nlmsghdr *nlh) {
    printk(KERN_INFO "nl_step(%d, %d)\n", nlh->nlmsg_type, nlh->nlmsg_pid);
    pid = nlh->nlmsg_pid;
    return 0;//link_tcp_stat();
}

void link_callback(struct sk_buff *skb) {
    netlink_rcv_skb(skb, &link_step);
}

int link_init(void) {
    struct netlink_kernel_cfg cfg = {
        .groups = 0,
        .input = link_callback,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_RUBICON, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating netlink socket.\n");
        return -ENODEV;
    }
    return 1;
}

void link_release(void) {
    if (nl_sk)
        netlink_kernel_release(nl_sk);
}