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

#include <net/netfilter/nf_conntrack.h>

//#include "rubi_udp_hook.h"
//#include "rubi_tcp_hook.h"

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

int rubi_tcp_hook(struct sk_buff *skb)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	struct iphdr *ip_header;
	struct tcphdr *tcp_header;

	ip_header = (struct iphdr *)skb_network_header(skb);
	tcp_header = tcp_hdr(skb);

	if (ntohs(tcp_header->source) == 80 || ntohs(tcp_header->dest) == 80) {

		ct = nf_ct_get(skb, &ctinfo);

		if (ct != NULL) {
			if (ctinfo % IP_CT_IS_REPLY == IP_CT_NEW) {
				struct nf_conntrack_tuple *t =
				    &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;

				printk(KERN_INFO
				       "%p req tuple %p: %pI4:%hu -> %pI4:%hu\n",
				       ct, t,
				       &t->src.u3.ip, ntohs(t->src.u.all),
				       &t->dst.u3.ip, ntohs(t->dst.u.all));

				t = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;

				printk(KERN_INFO
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

	}

	return NF_ACCEPT;
}

int rubi_udp_hook(struct sk_buff *skb)
{
	return NF_ACCEPT;
}

static unsigned int rubi_hook_fn(unsigned int hooknum, struct sk_buff *skb,
				 const struct net_device *in,
				 const struct net_device *out,
				 int (*okfn) (struct sk_buff *))
{
	struct iphdr *ip_header;
	if (!skb) {
		return NF_ACCEPT;
	}

	ip_header = (struct iphdr *)skb_network_header(skb);

	switch (ip_header->protocol) {
	case IPPROTO_UDP:
		return rubi_udp_hook(skb);
	case IPPROTO_TCP:
		return rubi_tcp_hook(skb);
	default:
		return NF_ACCEPT;
	}
}

#define HOOKS_NUM   2

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

static int __init rubi_hook_init(void)
{
	printk(KERN_INFO "rubi_hook_init() called");
	return nf_register_hooks(rubi_hooks_ops, HOOKS_NUM);
}

static void __exit rubi_hook_exit(void)
{
	printk(KERN_INFO "rubi_hook_exit() called");
	nf_unregister_hooks(rubi_hooks_ops, HOOKS_NUM);
}

module_init(rubi_hook_init);
module_exit(rubi_hook_exit);

MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("netfilter demo");
MODULE_AUTHOR("Ruslan Pislari");
