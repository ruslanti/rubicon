#include <stdio.h>
#include <stdlib.h>
#include <linux/netlink.h>
#include <netlink/socket.h>

#include "stat.h"

#define MAX_PAYLOAD 1024	/* maximum payload size */

static int my_input(struct nl_msg *msg, void *arg)
{
  return 0;
}

int main(void)
{
  struct nl_sock *sk;
  
  struct nl_msg *msg = nlmsg_alloc();

  sk = nl_socket_alloc();
  if (!sk)
    {
      perror("cannot alloc nl socket");
      return -1;
    }

  nl_socket_disable_seq_check(sk);

  nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, my_input, NULL);

  if (nl_connect(sk, 21) != 0)
    perror("nl_connect");

  if (nl_send_auto(sk, msg))
    perror("nl_send_auto");

  if (nl_recvmsgs_default(sk))
    perror("nl_recvmsgs_default");

  nl_socket_free(sk);
}
