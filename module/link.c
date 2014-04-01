/* 
 * File:   link.c
 * Author: ruslanti
 *
 * Created on March 31, 2014, 5:39 PM
 */

#include <net/netlink.h>

#include "rubi.h"

struct sock *nl_sk;

int nl_stat(int pid) {
    struct nlmsghdr * nlh;
    struct sk_buff * skb;
    char *msg = "Hello from kernel";
    int res;
    struct test *t;
    
    skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb) {
        printk(KERN_WARNING "Cannot alloc sk_buff with nlmsg_new()\n");
        return -ENOMEM;
    }
    
    //msg_size = strlen(msg);
    nlh = nlmsg_put(skb, 0, 0, RUBY_NLTEST, sizeof(struct test), 0);
    if (!nlh)
      goto nlmsg_failure;
    
    //memset(nlmsg_data(nlh), 0x0, nlmsg_len(nlh));
    t = (struct test*)nlmsg_data(nlh);
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
    
    res = nlmsg_unicast(nl_sk, skb, pid);

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
