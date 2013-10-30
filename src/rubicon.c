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
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

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
#define NF_IP_NUMHOOKS          5

static unsigned int rubi_hook_fn(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
  struct iphdr *ip_header;
  if(!skb) { return NF_ACCEPT;}

  ip_header = (struct iphdr *)skb_network_header(skb);   

  return NF_ACCEPT;
}

static struct nf_hook_ops rubi_hook_ops __read_mostly = {
  .pf       = NFPROTO_IPV4,
  .priority = NF_IP_PRI_FIRST + 1,
  .hooknum  = NF_IP_LOCAL_OUT,
  .hook     = rubi_hook_fn,
};

static int __init rubi_hook_init(void)
{
  printk(KERN_INFO "init_module() called\n");
  return nf_register_hook(&rubi_hook_ops);
}

static void __exit rubi_hook_exit(void)
{
  printk(KERN_INFO "cleanup_module() called\n");
  nf_unregister_hook(&rubi_hook_ops);
}

module_init(rubi_hook_init)
module_exit(rubi_hook_exit)

MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("netfilter demo");
MODULE_AUTHOR("Ruslan Pislari");
