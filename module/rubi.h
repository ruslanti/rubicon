/* 
 * File:   rubi.h
 * Author: ruslanti
 *
 * Created on March 24, 2014, 4:21 PM
 */

#ifndef RUBI_H

#define NETLINK_RUBICON         31
#define RUBINETLN_STATS         1
#define RUBY_NLTEST             0x11

enum rubi_stat_attrs {
    ATTR_IP_CT_ESTABLISHED,
    ATTR_IP_CT_RELATED,
    ATTR_IP_CT_NEW,
    ATTR_IP_CT_IS_REPLY,

    ATTR_IP_SADDR,
    ATTR_IP_DADDR,
    ATTR_TCP_SOURCE,
    ATTR_TCP_DEST,
    ATTR_SKB_SOURCE_LEN,
    ATTR_SKB_SOURCE_DATA_LEN
};


struct conn_stat {
    u_int64_t conn_id;
    u_int8_t protocol;
};


#define	RUBI_H

#endif	/* RUBI_H */

