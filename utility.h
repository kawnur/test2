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
    struct rtattr attr;
    int data;
} rtattr_with_int_data_t;

typedef struct {
    struct rtattr attr;
    char data[IFNAMSIZ];
} rtattr_with_char_data_t;

typedef struct {
    struct nlmsghdr header;
    struct ifinfomsg msg;
    rtattr_with_int_data_t attr1;
    struct rtattr attr2;
    rtattr_with_char_data_t attr2_1;
} nl_req_show_t;

typedef struct {
    struct nlmsghdr header;
    struct ifinfomsg msg;

    // did not switch to rtattr_with_char_data_t type for attr1 in this struct
    // because of strange 1-byte-difference in request bytestrings
    // between (rtattr + char[]) and rtattr_with_char_data_t variants
    // still solving this problem

    struct rtattr attr1;
    char data1[IFNAMSIZ];
//    rtattr_with_char_data_t attr1;

    struct rtattr attr2;
    rtattr_with_char_data_t attr2_1;
} nl_req_add_t;

typedef struct {
    struct nlmsghdr header;
    struct ifinfomsg msg;
    struct rtattr attr1;
    rtattr_with_char_data_t attr2;
} nl_req_del_t;

typedef struct {
    struct nlmsghdr header;
    struct ifinfomsg msg;
    rtattr_with_int_data_t attr1;
    rtattr_with_char_data_t attr2;
} nl_req_if_getlink_t;

typedef struct {
    struct nlmsghdr header;
    struct ifinfomsg msg;
    rtattr_with_int_data_t attr1;
} nl_req_if_newlink_t;

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
    int mode;
    void* (*req_creation)(const char*);
    int (*pre_error_handling)(struct nlmsghdr*);
    void (*error_handling)(struct nlmsghdr*);
    void (*reply_data_handling)(char*, struct nlmsghdr*, ssize_t);
} operation_mode;

size_t get_flag_string_from_flags(unsigned, char*);
void print_mac_address(const char*);
void print_info_repr(const info_repr*);
void print_usage_description();
int show_bridges();

void init_rtattr_with_int_data(rtattr_with_int_data_t*, unsigned, int);
void init_rtattr_with_char_data(rtattr_with_char_data_t*, unsigned, const char*);

int create_socket();

nl_req_show_t* create_show_req(const char*);
nl_req_add_t* create_add_req(const char*);
nl_req_del_t* create_del_req(const char*);
nl_req_if_newlink_t* create_addif_newlink_req(const char**);
nl_req_if_getlink_t* create_if_getlink_req(const char*);
nl_req_if_newlink_t* create_delif_newlink_req(const char*);

void handle_add_error(struct nlmsghdr*);
void handle_del_error(struct nlmsghdr*);
void handle_if_error(struct nlmsghdr*);
void handle_show_reply_data(char*, struct nlmsghdr*, ssize_t);

void* create_req(int, const void*);
int handle_before_error_handling(int, struct nlmsghdr*);
void handle_errors(int, struct nlmsghdr*);
void handle_reply_data(int, char*, struct nlmsghdr*, ssize_t);

int send_request_and_handle_reply(int, const void*);
void run_command(int, char**);

#endif // UTILITY_H
