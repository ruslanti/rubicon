/* 
 * File:   rubi_tcp_hook.c
 * Author: ruslanti
 *
 * Created on October 30, 2013, 8:11 PM
 */
#include <linux/netfilter.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <net/netfilter/nf_conntrack.h>

int rubi_tcp_hook(struct sk_buff *skb)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		printk(KERN_INFO "This is --ctstate INVALID\n");
	else if (ct == &nf_conntrack_untracked)
		printk(KERN_INFO "This one is not tracked\n");
	else if (ctinfo % IP_CT_IS_REPLY == IP_CT_NEW)
		printk(KERN_INFO "This is the first packet in a connection\n");
	else if (ctinfo % IP_CT_IS_REPLY == IP_CT_RELATED)
		printk(KERN_INFO
		       "Welcome Mr. Bond, we have been expecting you\n");
	else if (ctinfo % IP_CT_IS_REPLY == IP_CT_ESTABLISHED)
		printk(KERN_INFO "You can figure out this one!\n");
	return NF_ACCEPT;
}
