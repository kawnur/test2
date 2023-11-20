#ifndef UTILITY_H
#define UTILITY_H

#include <asm/types.h>
#include <errno.h>
#include <linux/if_bridge.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


typedef struct {
    struct nlmsghdr header;
    struct ifinfomsg msg;
    struct rtattr attr1;
    int data1;
    struct rtattr attr2;
    struct rtattr attr2_1;
    char data2[6];
} nl_req_show_t;

typedef struct {
    struct nlmsghdr header;
    struct ifinfomsg msg;
    struct rtattr attr1;
    char data1[IFNAMSIZ];
    struct rtattr attr2;
    struct rtattr attr2_1;
    char data2[6];
} nl_req_add_t;

typedef struct {
    struct nlmsghdr header;
    struct ifinfomsg msg;
    struct rtattr attr1;
    struct rtattr attr2;
    char data2[IFNAMSIZ];
} nl_req_del_t;

typedef struct {
    char *name;
    unsigned int flags;
    unsigned int mtu;
    char *qdisc;
    unsigned int state;
    unsigned int mode;
    unsigned int group;
    unsigned int qlen;
    char address[6];
    char broadcast[6];
} info_repr;

size_t get_flag_string_from_flags(unsigned, char*);
void print_mac_address(const char*);
void print_info_repr(const info_repr*);
int show_bridges();
int add_bridge(const char*);
int delete_bridge(const char*);
void print_usage_description();

#endif // UTILITY_H
