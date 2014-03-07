/*
 ============================================================================
 Name        : stat.c
 Author      : ruslanti
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "stat.h"


#define MAX_PAYLOAD 1024	/* maximum payload size */
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main(void)
{
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if (sock_fd < 0) {
		perror("socket");
		return -1;
	}
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();	/* self pid */
	src_addr.nl_groups = 1;
	/* interested in group 1<<0 */
	bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

	printf("Waiting for message from kernel\n");

	/* Read message from kernel */
	recvmsg(sock_fd, &msg, 0);
	printf(" Received message payload: %s\n", NLMSG_DATA(nlh));
	close(sock_fd);
}
