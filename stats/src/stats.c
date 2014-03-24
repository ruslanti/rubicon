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

static int my_input(struct nl_msg *msg, void *arg) {
    printf("received %d bytes\n", sizeof (msg));
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

    nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, my_input, NULL);

    if (nl_connect(sk, NETLINK_RUBICON) != 0) {
        perror("nl_connect()");
        exit(EXIT_FAILURE);
    }


    /* Subscribe to link notifications group */
    nl_socket_add_memberships(sk, RUBINETLN_STATS, 0);

    struct nl_msg *msg = nlmsg_alloc_simple(RTM_SETLINK, 0);
    struct ifinfomsg ifi = {
        .ifi_family = AF_INET,
        .ifi_index = 0,
    };

    /* Append the protocol specific header (struct ifinfomsg)*/
    if (nlmsg_append(msg, &ifi, sizeof (ifi), NLMSG_ALIGNTO) < 0) {
        perror("nlmsg_append()");
        exit(EXIT_FAILURE);
    }

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

