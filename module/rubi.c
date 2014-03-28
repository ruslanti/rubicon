/* 
 * File:   rubicon.c
 * Author: ruslanti
 *
 * Created on October 23, 2013, 5:39 PM
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/netfilter.h>
#include <linux/netlink.h>

#include <net/netfilter/nf_conntrack.h>

#include "rubi.h"

#define HOOKS_NUM   			2
/* IP Hooks */
#define NF_IP_PRE_ROUTING       0
/* If the packet is destined for this box. */
#define NF_IP_LOCAL_IN          1
/* If the packet is destined for another interface. */
#define NF_IP_FORWARD           2
/* Packets coming from a local process. */
#define NF_IP_LOCAL_OUT         3
/* Packets about to hit the wire. */
#define NF_IP_POST_ROUTING      4

#define MAX_PAYLOAD 			1024

static struct sock *nl_sk = NULL;

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

int rubi_tcp_hook(struct sk_buff *skb) {
    enum ip_conntrack_info ctinfo;
    struct nf_conn *ct;
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;

    ip_header = (struct iphdr *) skb_network_header(skb);
    tcp_header = tcp_hdr(skb);

    if (ntohs(tcp_header->source) == 80 || ntohs(tcp_header->dest) == 80) {
        ct = nf_ct_get(skb, &ctinfo);

        if (ct != NULL) {
            if (ctinfo % IP_CT_IS_REPLY == IP_CT_NEW) {
                struct nf_conntrack_tuple *t = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
                printk(KERN_DEBUG
                        "%p req tuple %p: %pI4:%hu -> %pI4:%hu\n",
                        ct, t,
                        &t->src.u3.ip, ntohs(t->src.u.all),
                        &t->dst.u3.ip, ntohs(t->dst.u.all));

                t = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;

                printk(KERN_DEBUG
                        "%p res tuple %p: %pI4:%hu -> %pI4:%hu\n",
                        ct, t,
                        &t->src.u3.ip, ntohs(t->src.u.all),
                        &t->dst.u3.ip, ntohs(t->dst.u.all));
            }
            //printk(KERN_INFO "src ip %pI4:%d, dst ip %pI4:%d, ct %p, ctinfo %d, flags=%c%c%c%c%c%c",
            /*       &ip_header->saddr, ntohs(tcp_header->source),
                     &ip_header->daddr, ntohs(tcp_header->dest),
                     ct, ctinfo, 
                     tcp_header->urg ? 'U' : '-',
                     tcp_header->ack ? 'A' : '-',
                     tcp_header->psh ? 'P' : '-',
                     tcp_header->rst ? 'R' : '-',
                     tcp_header->syn ? 'S' : '-',
                     tcp_header->fin ? 'F' : '-'
                     );
             */
        }
        //      nl_stat(1);
    }
    return NF_ACCEPT;
}

int rubi_udp_hook(struct sk_buff *skb) {
    return NF_ACCEPT;
}

static unsigned int rubi_hook_fn(const struct nf_hook_ops *ops,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn) (struct sk_buff *)) {

    struct iphdr *ip_header;
    if (!skb)
        return NF_ACCEPT;

    ip_header = (struct iphdr *) skb_network_header(skb);

    switch (ip_header->protocol) {
        case IPPROTO_UDP:
            return rubi_udp_hook(skb);
        case IPPROTO_TCP:
            return rubi_tcp_hook(skb);
        default:
            return NF_ACCEPT;
    }
}

static int nl_step(struct sk_buff *skb, struct nlmsghdr *nlh) {
    printk(KERN_INFO "nl_step(%d, %d)\n", nlh->nlmsg_type, nlh->nlmsg_pid);
    return nl_stat(nlh->nlmsg_pid);
}

static void nl_callback(struct sk_buff *skb) {
    netlink_rcv_skb(skb, &nl_step);
}

static int rubi_netlink_init(void) {
    struct netlink_kernel_cfg cfg = {
        .groups = 0,
        .input = nl_callback,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_RUBICON, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating netlink socket.\n");
        return -ENODEV;
    }
    return 1;
}

static struct nf_hook_ops rubi_hooks_ops[] = {
    {
        .pf = NFPROTO_IPV4,
        .priority = NF_IP_PRI_LAST,
        .hooknum = NF_IP_LOCAL_OUT,
        .hook = rubi_hook_fn,
    },
    {
        .pf = NFPROTO_IPV4,
        .priority = NF_IP_PRI_LAST,
        .hooknum = NF_IP_LOCAL_IN,
        .hook = rubi_hook_fn,
    }
};

static int __init rubi_hook_init(void) {
    printk(KERN_INFO "rubi_hook_init() called\n");
    return rubi_netlink_init() && nf_register_hooks(rubi_hooks_ops, HOOKS_NUM);
}

static void __exit rubi_hook_exit(void) {
    printk(KERN_INFO "rubi_hook_exit() called\n");
    if (nl_sk)
        netlink_kernel_release(nl_sk);
    nf_unregister_hooks(rubi_hooks_ops, HOOKS_NUM);
}

module_init(rubi_hook_init);
module_exit(rubi_hook_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("netfilter demo");
MODULE_AUTHOR("Ruslan Pislari");

