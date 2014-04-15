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

#include <link.h>

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
    nl_msg_dump(msg, stdout);
    
    nlh = nlmsg_hdr(msg);
    
    struct nlattr *hdr = nlmsg_attrdata(nlh, sizeof(struct conn_stat));
    
    int remaining = nlmsg_attrlen(nlh, sizeof(struct conn_stat));
    
    printf("attr len = %d\n", remaining);
    
    while (nla_ok(hdr, remaining)) {
        printf("nla_type = %X\n", nla_type(hdr));
        printf("nla_data = %X\n", nla_data(hdr));
        hdr = nla_next(hdr, &remaining);
    };
    
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

