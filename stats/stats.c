/* 
 * File:   stats.c
 * Author: ruslanti
 *
 * Created on March 24, 2014, 12:12 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <linux/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>

#include <rubi.h>

union ip {
    uint32_t i;
    struct {
       uint8_t b[4];
    } a;
};

static int cb_msg_in(struct nl_msg *msg, void *arg) {
    struct nlmsghdr *nlh;
    
    nl_msg_dump(msg, stdout);
    
    nlh = nlmsg_hdr(msg);

    struct nlattr *hdr = nlmsg_attrdata(nlh, 0);
    int remaining = nlmsg_attrlen(nlh, 0);
        
    while (nla_ok(hdr, remaining)) {
        /* parse attribute here */
        hdr = nla_next(hdr, &remaining);
    };
    return NL_OK;
}

static int cb_valid(struct nl_msg *msg, void *arg) {
    struct nlmsghdr *nlh;
    int remaining, ip_ct_new = 0, ip_ct_established = 0, ip_ct_is_reply = 0;
    union ip saddr, daddr;
    uint16_t source, dest, len = 0, data_len = 0;
    
    //nl_msg_dump(msg, stdout);
    nlh = nlmsg_hdr(msg);

    struct conn_stat* stat = (struct conn_stat*)nlmsg_data(nlh);
    struct nlattr *nla = nlmsg_attrdata(nlh, sizeof(struct conn_stat));
    remaining = nlmsg_attrlen(nlh, sizeof(struct conn_stat));
    
    while (nla_ok(nla, remaining)) {
        //printf("nla_type = %X\n", nla_type(nla));
        //printf("nla_data = %X\n", nla_data(nla));
        switch (nla_type(nla)) {
            case ATTR_IP_CT_NEW:
                ip_ct_new = nla_get_flag(nla);
                break;
            case ATTR_IP_CT_ESTABLISHED:
                ip_ct_established = nla_get_flag(nla);
                break;
            case ATTR_IP_CT_IS_REPLY:
                ip_ct_is_reply = nla_get_flag(nla);
                break;
            case ATTR_IP_SADDR:
                saddr.i = nla_get_u32(nla);
                break;
            case ATTR_IP_DADDR:
                daddr.i = nla_get_u32(nla);
                break;
            case ATTR_TCP_SOURCE:
                source = nla_get_u16(nla);
                break;
            case ATTR_TCP_DEST:
                dest = nla_get_u16(nla);
                break;
            case ATTR_SKB_SOURCE_LEN:
                len = nla_get_u16(nla);
                break;
            case ATTR_SKB_SOURCE_DATA_LEN:
                data_len = nla_get_u16(nla);
                break;
        }
        nla = nla_next(nla, &remaining);
    };
    
    if (ip_ct_new) {
        printf("[%p] new: %d.%d.%d.%d:%d - %d.%d.%d.%d:%d (%d, %d)\n", stat->conn_id,
                saddr.a.b[0], saddr.a.b[1], saddr.a.b[2], saddr.a.b[3], ntohs(source), 
                daddr.a.b[0], daddr.a.b[1], daddr.a.b[2], daddr.a.b[3], ntohs(dest), 
                len, data_len);
    } else if (ip_ct_established) {
        printf("[%p] established: (%d, %d)\n", stat->conn_id, len, data_len);
    } else if (ip_ct_is_reply) {
        printf("[%p] reply: (%d, %d)\n", stat->conn_id, len, data_len);
    }
    
    return NL_OK;
}

#define ATTR_MY_STRUCT 423

/*
 * 
 */
int main(int argc, char** argv) {
    struct nl_sock *sk;

    sk = nl_socket_alloc();
    if (!sk) {
        perror("nl_socket_alloc()");
        exit(EXIT_FAILURE);
    }

    nl_socket_disable_seq_check(sk);

    //nl_socket_modify_cb(sk, NL_CB_MSG_IN, NL_CB_CUSTOM, cb_msg_in, NULL);
    nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, cb_valid, NULL);

    if (nl_connect(sk, NETLINK_RUBICON) != 0) {
        perror("nl_connect()");
        exit(EXIT_FAILURE);
    }


    /* Subscribe to link notifications group */
    nl_socket_add_memberships(sk, RUBINETLN_STATS, 0);

    struct nl_msg *msg = nlmsg_alloc_simple(RTM_SETLINK, 0);

    NLA_PUT_U32(msg, ATTR_MY_STRUCT, 0x1010);

    if (nl_send_auto(sk, msg) < 0) {
        perror("nl_send_auto()");
        exit(EXIT_FAILURE);
    }

    /*
     * Start receiving messages. The function nl_recvmsgs_default() will block
     * until one or more netlink messages (notification) are received which
     * will be passed on to my_input().
     */
    while (1)
        nl_recvmsgs_default(sk);

nla_put_failure:
    nlmsg_free(msg);

    nl_socket_free(sk);

}

