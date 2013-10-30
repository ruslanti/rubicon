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
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>

//#include "rubi_udp_hook.h"
//#include "rubi_tcp_hook.h"

/* IP Hooks */
/* After promisc drops, checksum checks. */
#define NF_IP_PRE_ROUTING       0
/* If the packet is destined for this box. */
#define NF_IP_LOCAL_IN          1
/* If the packet is destined for another interface. */
#define NF_IP_FORWARD           2
/* Packets coming from a local process. */
#define NF_IP_LOCAL_OUT         3
/* Packets about to hit the wire. */
#define NF_IP_POST_ROUTING      4
#define NF_IP_NUMHOOKS          

int rubi_tcp_hook(struct sk_buff *skb) {
  enum ip_conntrack_info ctinfo;
  struct nf_conn *ct;
  struct iphdr *ip_header;
  struct tcphdr *tcp_header;
 
  ct = nf_ct_get(skb, &ctinfo);
  if (ct == NULL)
    printk(KERN_INFO "This is --ctstate INVALID\n");
  else if (ct == &nf_conntrack_untracked)
    printk(KERN_INFO "This one is not tracked\n");
  else if (ctinfo % IP_CT_IS_REPLY == IP_CT_NEW)
    printk(KERN_INFO "This is the first packet in a connection\n");
  else if (ctinfo % IP_CT_IS_REPLY == IP_CT_RELATED)
    printk(KERN_INFO "Welcome Mr. Bond, we have been expecting you\n");
  else if (ctinfo % IP_CT_IS_REPLY == IP_CT_ESTABLISHED) {
    //(KERN_INFO "You can figure out this one!\n");
    return NF_ACCEPT;
  }
  
  ip_header = (struct iphdr *)skb_network_header(skb);
  tcp_header = tcp_hdr(skb);
  printk(KERN_INFO "src ip %X:%d, dst ip %X:%d protocol %d", 
	 ip_header->saddr, ntohs(tcp_header->source), ip_header->daddr, ntohs(tcp_header->dest), ip_header->protocol);
    
  return NF_ACCEPT;
}

int rubi_udp_hook(struct sk_buff *skb) {
  ip_conntrack_get(skb);
  return NF_ACCEPT;
}

static unsigned int rubi_hook_fn(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
  struct iphdr *ip_header;
  if(!skb) { return NF_ACCEPT;}

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

static struct nf_hook_ops rubi_hook_ops __read_mostly = {
  .pf       = NFPROTO_IPV4,
  .priority = NF_IP_PRI_LAST,
  .hooknum  = NF_IP_LOCAL_OUT,
  .hook     = rubi_hook_fn,
};

static int __init rubi_hook_init(void)
{
  printk(KERN_INFO "rubi_hook_init() called\n");
  return nf_register_hook(&rubi_hook_ops);
}

static void __exit rubi_hook_exit(void)
{
  printk(KERN_INFO "rubi_hook_exit() called\n");
  nf_unregister_hook(&rubi_hook_ops);
}

module_init(rubi_hook_init)
module_exit(rubi_hook_exit)

MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("netfilter demo");
MODULE_AUTHOR("Ruslan Pislari");
