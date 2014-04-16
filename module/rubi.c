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
#include "link.h"

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


int rubi_tcp_hook(struct sk_buff *skb) {
    struct tcphdr *tcp_header = tcp_hdr(skb);

    if (ntohs(tcp_header->source) == 80 || ntohs(tcp_header->dest) == 80) {
        link_tcp_stat(skb);
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
    return link_init() && nf_register_hooks(rubi_hooks_ops, HOOKS_NUM);
}

static void __exit rubi_hook_exit(void) {
    printk(KERN_INFO "rubi_hook_exit() called\n");
    link_release();
    nf_unregister_hooks(rubi_hooks_ops, HOOKS_NUM);
}

module_init(rubi_hook_init);
module_exit(rubi_hook_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("netfilter demo");
MODULE_AUTHOR("Ruslan Pislari");

