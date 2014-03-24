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

static int my_input(struct nl_msg *msg, void *arg)
{
  return 0;
}

/*
 * 
 */
int main(int argc, char** argv) {
    struct nl_sock *sk;

    struct nl_msg *msg = nlmsg_alloc();
    if (msg == NULL) {
        perror("nlmsg_alloc()");
        exit(EXIT_FAILURE);
    }

    sk = nl_socket_alloc();
    if (!sk) {
        perror("nl_socket_alloc()");
        exit(EXIT_FAILURE);
    }

    nl_socket_disable_seq_check(sk);

    nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, my_input, NULL);

    if (nl_connect(sk, 31) != 0) {
        perror("nl_connect");
        exit(EXIT_FAILURE);
    }

    if (nl_send_auto(sk, msg)) {
        perror("nl_send_auto");
        exit(EXIT_FAILURE);
    }

    if (nl_recvmsgs_default(sk)) {
        perror("nl_recvmsgs_default");
        exit(EXIT_FAILURE);
    }

    nl_socket_free(sk);
    nlmsg_free(msg);
}

