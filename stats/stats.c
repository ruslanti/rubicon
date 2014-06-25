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

#include <mongo.h>

#include <rubi.h>

union ip {
    uint32_t i;
    struct {
       uint8_t b[4];
    } a;
};

mongo_sync_connection *conn;

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
    //union ip saddr, daddr;
    uint16_t source, dest, len = 0, data_len = 0;
    bson *doc, *selector;    
    //nl_msg_dump(msg, stdout);
    nlh = nlmsg_hdr(msg);
    char saddr[INET_ADDRSTRLEN], daddr[INET_ADDRSTRLEN];
    struct in_addr in;

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
                in.s_addr = nla_get_u32(nla);
                if (!inet_ntop(AF_INET, &in, saddr, INET_ADDRSTRLEN)) {
                    perror("inet_ntop()");
                }
                break;
            case ATTR_IP_DADDR:
                in.s_addr = nla_get_u32(nla);
                if (!inet_ntop(AF_INET, &in, daddr, INET_ADDRSTRLEN)) {
                    perror("inet_ntop()");
                }
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
        printf("[%p] new: %s:%d - %s:%d (%d, %d)\n", stat->conn_id,
                saddr, ntohs(source), 
                daddr, ntohs(dest), 
                len, data_len);
        
        doc = bson_build(
                BSON_TYPE_INT64, "conn_id", stat->conn_id,
                BSON_TYPE_STRING, "saddr", saddr, -1,
                BSON_TYPE_STRING, "daddr", daddr, -1,
                BSON_TYPE_INT32, "source", ntohs(source),
                BSON_TYPE_INT32, "dest", ntohs(dest),
                BSON_TYPE_INT32, "tx", len,
                BSON_TYPE_INT32, "rx", 0,
                BSON_TYPE_NONE);
        bson_finish(doc);
        
        if (!mongo_sync_cmd_insert(conn, "rubicon.st", doc, NULL)) {
            perror("mongo_sync_cmd_insert()");
        }
        
        bson_free(doc);
        
    } else if (ip_ct_established) {
        printf("[%p] established: (%d, %d)\n", stat->conn_id, len, data_len);
        
        selector = bson_build(BSON_TYPE_INT64, "conn_id", stat->conn_id,
                BSON_TYPE_NONE);
        bson_finish(selector);
        
        doc = bson_build_full (BSON_TYPE_DOCUMENT, "$inc", TRUE,
                    bson_build (BSON_TYPE_INT32, "tx", len, BSON_TYPE_NONE),
                    BSON_TYPE_NONE);
        bson_finish(doc);
        
        if (!mongo_sync_cmd_update(conn, "rubicon.st", 0, selector, doc)) {
            perror("mongo_sync_cmd_update()");
        }
        
        bson_free(selector);
        bson_free(doc);
        
    } else if (ip_ct_is_reply) {
        printf("[%p] reply: (%d, %d)\n", stat->conn_id, len, data_len);
        
        selector = bson_build(BSON_TYPE_INT64, "conn_id", stat->conn_id,
                BSON_TYPE_NONE);
        bson_finish(selector);
        
        doc = bson_build_full (BSON_TYPE_DOCUMENT, "$inc", TRUE,
                    bson_build (BSON_TYPE_INT32, "rx", len, BSON_TYPE_NONE),
                    BSON_TYPE_NONE);
        bson_finish(doc);
        
        if (!mongo_sync_cmd_update(conn, "rubicon.st", 0, selector, doc)) {
            perror("mongo_sync_cmd_update()");
        }
        
        bson_free(selector);
        bson_free(doc);
    }
    
    return NL_OK;
}

#define ATTR_MY_STRUCT 423

/*
 * 
 */
int main(int argc, char** argv) {
    struct nl_sock *sk;
    
    conn = mongo_sync_connect("localhost", 27017, TRUE);
    if (!conn) {
        perror("mongo_sync_connect()");
        exit(1);
    }
    
    mongo_sync_cmd_create (conn, "rubicon", "st", MONGO_COLLECTION_DEFAULTS);
    //print_coll_info (mongo_sync_cmd_exists (conn, "rubicon", "st"));

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

    mongo_sync_disconnect(conn);
}

