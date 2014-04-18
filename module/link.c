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
#include "rubi.h"

struct sock *nl_sk;

int pid;

int link_tcp_stat(struct sk_buff *skb) {
    enum ip_conntrack_info ctinfo;
    struct nf_conn *ct;
    
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    
    struct nlmsghdr * nlh;
    struct sk_buff * nl_skb;
    
    int res;
    struct conn_stat *stat;
    
    if (!pid)
        return 0;
    
    ct = nf_ct_get(skb, &ctinfo);

    if (ct == NULL) {
        return 0;
    }
    
    ip_header = (struct iphdr *) skb_network_header(skb);
    tcp_header = tcp_hdr(skb);
     
    printk(KERN_DEBUG "src ip %pI4:%d, dst ip %pI4:%d, ct %p, ctinfo %d, flags=%c%c%c%c%c%c",
                    &ip_header->saddr, ntohs(tcp_header->source),
                     &ip_header->daddr, ntohs(tcp_header->dest),
                     ct, ctinfo, 
                     tcp_header->urg ? 'U' : '-',
                     tcp_header->ack ? 'A' : '-',
                     tcp_header->psh ? 'P' : '-',
                     tcp_header->rst ? 'R' : '-',
                     tcp_header->syn ? 'S' : '-',
                     tcp_header->fin ? 'F' : '-'
                     );
             
    if (ctinfo % IP_CT_IS_REPLY == IP_CT_NEW) {
        // new connection
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
    
    nl_skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_ATOMIC);
    if (!nl_skb) {
        printk(KERN_WARNING "Cannot alloc sk_buff with nlmsg_new()\n");
        return -ENOMEM;
    }
    
    nlh = nlmsg_put(nl_skb, 0, 0, RUBY_NLTEST, sizeof(struct conn_stat), 0);
    if (!nlh)
      goto nlmsg_failure;

    stat = (struct conn_stat*)nlmsg_data(nlh);
    stat->conn_id = (u_int64_t)ct;
    stat->protocol = IPPROTO_TCP;
    
    //TODO: separate case for IS_REPLY
    if (ctinfo == IP_CT_NEW) {
        if (nla_put_net32(nl_skb, ATTR_IP_SADDR, ip_header->saddr))
            goto nla_put_failure;
        if (nla_put_net32(nl_skb, ATTR_IP_DADDR, ip_header->daddr))
            goto nla_put_failure;
        if (nla_put_net16(nl_skb, ATTR_TCP_SOURCE, tcp_header->source))
            goto nla_put_failure;
        if (nla_put_net16(nl_skb, ATTR_TCP_DEST, tcp_header->dest))
            goto nla_put_failure;
        if (nla_put_flag(nl_skb, ATTR_IP_CT_NEW))
            goto nla_put_failure;
    } else if (ctinfo == IP_CT_ESTABLISHED) {
        if (nla_put_flag(nl_skb, ATTR_IP_CT_ESTABLISHED))
            goto nla_put_failure;    
    } else if (ctinfo == IP_CT_IS_REPLY) {
        if (nla_put_flag(nl_skb, ATTR_IP_CT_IS_REPLY))
            goto nla_put_failure;    
    }
    
    if (skb->len) {
        if (nla_put_u16(nl_skb, ATTR_SKB_SOURCE_LEN, skb->len))
            goto nla_put_failure;   
    }
    
    if (skb->data_len) {
        if (nla_put_u16(nl_skb, ATTR_SKB_SOURCE_DATA_LEN, skb->data_len))
            goto nla_put_failure;     
    }
    
    
    res = nlmsg_end(nl_skb, nlh);
    
    res = nlmsg_unicast(nl_sk, nl_skb, pid);
    //else
    //    res = nlmsg_multicast(nl_sk, nl_skb, pid, RUBINETLN_STATS, GFP_ATOMIC);
    
    if (res < 0) {
        printk(KERN_WARNING "Error while sending message: %d\n", res);
        pid = -1;
    }
    
    return res;
        
    nlmsg_failure:
    nla_put_failure:
        nlmsg_cancel(nl_skb, nlh);
        kfree_skb(nl_skb);
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

void link_bind(int group) {
    printk(KERN_INFO "bind(%d).\n", group);
}

int link_init(void) {
    struct netlink_kernel_cfg cfg = {
        .groups = RUBINETLN_STATS,
        .input = link_callback,
        .bind = link_bind,
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