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

int link_tcp_stat(int pid) {
    struct nlmsghdr * nlh;
    struct sk_buff * skb;
    char *msg = "Hello from kernel";
    int res;
    struct tcp_stat *t;
    
    skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb) {
        printk(KERN_WARNING "Cannot alloc sk_buff with nlmsg_new()\n");
        return -ENOMEM;
    }
    
    nlh = nlmsg_put(skb, 0, 0, RUBY_NLTEST, sizeof(struct tcp_stat), 0);
    if (!nlh)
      goto nlmsg_failure;
    
    //memset(nlmsg_data(nlh), 0x0, nlmsg_len(nlh));
    t = (struct tcp_stat*)nlmsg_data(nlh);
    t->a1 = 0x10;
    t->a2 = 0x11;
    
    if (nla_put_u32(skb, 0xEE, 0xFF))
        goto nla_put_failure;
    
    if (nla_put_string(skb, 0xAA, msg))
        goto nla_put_failure;
   /* if (!nlh) {
        printk(KERN_WARNING "nlmsg_put(): Insufficient to store for message header or payload\n");
        return -ENOMEM; 
    }
    
    strncpy(nlmsg_data(nlh), msg, msg_size);
    */ 
    res = nlmsg_end(skb, nlh);
    printk(KERN_INFO "Prepared %d bytes to send (%d) \n", res, nlmsg_len(nlh));
    
    if (pid)
        res = nlmsg_unicast(nl_sk, skb, pid);
    else
        res = nlmsg_multicast(nl_sk, skb, pid, 0, GFP_KERNEL);
    
    if (res < 0) {
        printk(KERN_WARNING "Error while sending unicast message\n");
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
    return link_tcp_stat(nlh->nlmsg_pid);
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