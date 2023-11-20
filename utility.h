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

typedef struct {
    int opcode;
    void* (*create_req_data)(const char*);
    void (*manage_reply_data)(char*, struct nlmsghdr*, ssize_t);
} operation_mode;

size_t get_flag_string_from_flags(unsigned, char*);
void print_mac_address(const char*);
void print_info_repr(const info_repr*);
void print_usage_description();
int show_bridges();

int create_socket();

nl_req_show_t* create_req_show_data(const char*);
nl_req_add_t* create_req_add_data(const char*);
nl_req_del_t* create_req_del_data(const char*);

void manage_show_reply_data(char*, struct nlmsghdr*, ssize_t);
void manage_add_reply_data(char*, struct nlmsghdr*, ssize_t);
void manage_del_reply_data(char*, struct nlmsghdr*, ssize_t);

void* create_req(int, const char*);
void manage_data(int, char*, struct nlmsghdr*, ssize_t);

int operate(int, const char*);

#endif // UTILITY_H
