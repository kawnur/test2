#include "utility.h"


size_t get_flag_string_from_flags(unsigned flags, char* result) {
    typedef struct {
        unsigned int flag;
        char *value;
    } flag_value;

    // Constants IFF_* defined in nameless enum
    flag_value flag_value_array[16] = {
        { .flag = IFF_UP, .value = "UP" },
        { .flag = IFF_BROADCAST, .value = "BROADCAST" },
        { .flag = IFF_DEBUG, .value = "DEBUG" },
        { .flag = IFF_LOOPBACK, .value = "LOOPBACK" },
        { .flag = IFF_POINTOPOINT, .value = "POINTOPOINT" },
        { .flag = IFF_NOTRAILERS, .value = "NOTRAILERS" },
        { .flag = IFF_RUNNING, .value = "RUNNING" },
        { .flag = IFF_NOARP, .value = "NOARP" },
        { .flag = IFF_PROMISC, .value = "PROMISC" },
        { .flag = IFF_ALLMULTI, .value = "ALLMULTI" },
        { .flag = IFF_MASTER, .value = "MASTER" },
        { .flag = IFF_SLAVE, .value = "SLAVE" },
        { .flag = IFF_MULTICAST, .value = "MULTICAST" },
        { .flag = IFF_PORTSEL, .value = "PORTSEL" },
        { .flag = IFF_AUTOMEDIA, .value = "AUTOMEDIA" },
        { .flag = IFF_DYNAMIC, .value = "DYNAMIC" },
    };

    char flags_str[] = "<";
    size_t arr_size = sizeof(flag_value_array) / sizeof(flag_value_array[0]);

    for(int i = 0; i < arr_size; i++) {
        if(flags & flag_value_array[i].flag) {
            strcat(flags_str, flag_value_array[i].value);
            strcat(flags_str, ", ");
        }
    }

    flags_str[strlen(flags_str) - 2] = '>';
    flags_str[strlen(flags_str) - 1] = 0x0;

    strcpy(result, flags_str);

    return strlen(result);
}

void print_mac_address(const char *address) {
    for(int i = 0; i < 5; ++i) {
        printf("%x:", address[i] & 0xff);
    }
    printf("%x", address[5] & 0xff);
}

void print_info_repr(const info_repr *info) {
    char flag_string[256] = {0};
    get_flag_string_from_flags(info->flags, flag_string);

    printf("%s: ", info->name);
    printf("%s ", flag_string);
    printf("mtu %d ", info->mtu);
    printf("qdisc %s ", info->qdisc);
    printf("state %d ", info->state);
    printf("mode %d ", info->mode);
    printf("group %d ", info->group);
    printf("qlen %d\n\t", info->qlen);

    printf("address ");
    print_mac_address(info->address);
    printf(" broadcast ");
    print_mac_address(info->broadcast);

    printf("\n\n");
}

void print_usage_description() {
    typedef struct {
        char *command;
        char *description;
    } entry;

    entry entries[5] = {
        { .command = "show", .description = "show list of bridges" },
        { .command = "addbr <bridge_name>", .description = "add bridge" },
        { .command = "delbr <bridge_name>", .description = "delete bridge" },
        { .command = "addif <bridge_name> <interface_name>", .description = "add interface to bridge" },
        { .command = "delif <bridge_name> <interface_name>", .description = "delete interface from bridge" },
    };

    size_t size = sizeof(entries) / sizeof(entries[0]);

    printf("Usage: bridge_utility [command] \n");  // TODO get name from cmakelists.txt
    printf("commands: \n");

    for(int i = 0; i < size; ++i) {
        printf("\t %s \n", entries[i].command);
        printf("\t\t %s \n", entries[i].description);
        printf("\n");
    }
}

int show_bridges() {
    send_request_and_handle_reply(1, "");
    return 0;
}

void init_rtattr_with_int_data(rtattr_with_int_data_t *block, unsigned type, int data) {
    block->attr.rta_type = type;
    block->data = data;
    block->attr.rta_len = sizeof(struct rtattr) + sizeof(data);
}

void init_rtattr_with_char_data(rtattr_with_char_data_t *block, unsigned type, const char* data) {
    block->attr.rta_type = type;
    strcpy(block->data, data);
    block->attr.rta_len = sizeof(struct rtattr) + sizeof(data);
}

int create_socket() {
    int fd = socket(AF_NETLINK, SOCK_RAW|SOCK_CLOEXEC, NETLINK_ROUTE);

    if(fd < 0) {
        perror("Error netlink socket opening");
    }

    return fd;
}

nl_req_show_t* create_show_req(const char* arg) {
    nl_req_show_t* req = (nl_req_show_t*)malloc(sizeof(nl_req_show_t));
    memset(req, 0, sizeof(nl_req_show_t));

    req->header.nlmsg_type = RTM_GETLINK;
    req->header.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;

    req->msg.ifi_family = AF_PACKET;
    req->msg.ifi_index = 0;

    init_rtattr_with_int_data(&req->attr1, IFLA_EXT_MASK, RTEXT_FILTER_VF);

    init_rtattr_with_char_data(&req->attr2_1, IFLA_INFO_KIND, "bridge");

    req->attr2.rta_len = sizeof(struct rtattr) + req->attr2_1.attr.rta_len;
    req->attr2.rta_type = IFLA_LINKINFO;

    req->header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req->attr1.attr.rta_len + req->attr2.rta_len;

    return req;
}

nl_req_add_t* create_add_req(const char* arg) {
    nl_req_add_t* req = (nl_req_add_t*)malloc(sizeof(nl_req_add_t));
    memset(req, 0, sizeof(nl_req_add_t));

    req->header.nlmsg_type = RTM_NEWLINK;
    req->header.nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL|NLM_F_CREATE;

    req->msg.ifi_family = AF_UNSPEC;
    req->msg.ifi_index = 0;

    req->attr1.rta_len = sizeof(struct rtattr) + sizeof(req->data1);
    req->attr1.rta_type = IFLA_IFNAME;
    strcpy(req->data1, arg);

    init_rtattr_with_char_data(&req->attr2_1, IFLA_INFO_KIND, "bridge");

    req->attr2.rta_len = sizeof(struct rtattr) + req->attr2_1.attr.rta_len;
    req->attr2.rta_type = IFLA_LINKINFO;

    req->header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req->attr1.rta_len + req->attr2.rta_len;

    return req;
}

nl_req_del_t* create_del_req(const char* arg) {
    nl_req_del_t* req = (nl_req_del_t*)malloc(sizeof(nl_req_del_t));
    memset(req, 0, sizeof(nl_req_del_t));

    req->header.nlmsg_type = RTM_DELLINK;
    req->header.nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK;

    req->msg.ifi_family = AF_UNSPEC;
    req->msg.ifi_index = if_nametoindex(arg);

    req->attr1.rta_len = sizeof(struct rtattr);
    req->attr1.rta_type = IFLA_LINKINFO;

    init_rtattr_with_char_data(&req->attr2, IFLA_INFO_KIND, "bridge");

    req->header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req->attr1.rta_len + req->attr2.attr.rta_len;

    return req;
}

// char** in signature is correct
nl_req_if_newlink_t* create_addif_newlink_req(const char** arg) {
    nl_req_if_newlink_t* req = (nl_req_if_newlink_t*)malloc(sizeof(nl_req_if_newlink_t));
    memset(req, 0, sizeof(nl_req_if_newlink_t));

    req->header.nlmsg_type = RTM_NEWLINK;
    req->header.nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK;

    req->msg.ifi_family = AF_UNSPEC;
    req->msg.ifi_index = if_nametoindex(arg[1]);

    init_rtattr_with_int_data(&req->attr1, IFLA_MASTER, if_nametoindex(arg[0]));

    req->header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req->attr1.attr.rta_len;

    return req;
}

nl_req_if_getlink_t* create_if_getlink_req(const char* arg) {
    nl_req_if_getlink_t* req = (nl_req_if_getlink_t*)malloc(sizeof(nl_req_if_getlink_t));
    memset(req, 0, sizeof(nl_req_if_getlink_t));

    req->header.nlmsg_type = RTM_GETLINK;
    req->header.nlmsg_flags = NLM_F_REQUEST;

    req->msg.ifi_family = AF_UNSPEC;
    req->msg.ifi_index = 0;

    init_rtattr_with_int_data(&req->attr1, IFLA_EXT_MASK, RTEXT_FILTER_VF|RTEXT_FILTER_SKIP_STATS);

    init_rtattr_with_char_data(&req->attr2, IFLA_IFNAME, arg);

    req->header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req->attr1.attr.rta_len + req->attr2.attr.rta_len;

    return req;
}

nl_req_if_newlink_t* create_delif_newlink_req(const char* arg) {
    nl_req_if_newlink_t* req = (nl_req_if_newlink_t*)malloc(sizeof(nl_req_if_newlink_t));
    memset(req, 0, sizeof(nl_req_if_newlink_t));

    req->header.nlmsg_type = RTM_NEWLINK;
    req->header.nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK;

    req->msg.ifi_family = AF_UNSPEC;
    req->msg.ifi_index = if_nametoindex(arg);

    init_rtattr_with_int_data(&req->attr1, IFLA_MASTER, 0);

    req->header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req->attr1.attr.rta_len;

    return req;
}

int handle_if_getlink_pre_error(struct nlmsghdr *nh) {
    struct ifinfomsg *message;

    if(nh->nlmsg_type == RTM_NEWLINK) {
        message = (struct ifinfomsg *)NLMSG_DATA(nh);

        if(!message->ifi_flags) {
            return -1;
        }
    }

    return 0;
}

void handle_add_error(struct nlmsghdr *nh) {
    struct nlmsgerr *msg_err = (struct nlmsgerr *)NLMSG_DATA(nh);
    int error = -(msg_err->error);

    if(error == 0) {
        show_bridges();
    }
    else if(error == EPERM) {
        printf("Operation not permitted\n");
    }
    else if(error == EEXIST) {
        printf("File exists\n");
    }
    else if(error == EOPNOTSUPP) {
        printf("Operation not supported on transport endpoint\n");
    }

    exit(0);
}

void handle_del_error(struct nlmsghdr *nh) {
    struct nlmsgerr *msg_err = (struct nlmsgerr *)NLMSG_DATA(nh);
    int error = -(msg_err->error);

    if(error == 0) {
        show_bridges();
    }
    else if(error == EPERM) {
        printf("Operation not permitted\n");
    }
    else if(error == EINVAL) {
        printf("Invalid argument\n");
    }

    exit(0);
}

void handle_if_error(struct nlmsghdr *nh) {
    struct nlmsgerr *msg_err = (struct nlmsgerr *)NLMSG_DATA(nh);
    int error = -(msg_err->error);

    if(error == 0) {
        show_bridges();
    }
    else if(error == ENODEV) {
        printf("No such device\n");
    }
    else if(error == EPERM) {
        printf("Operation not permitted\n");
    }
    else if(error == EINVAL) {
        printf("Invalid argument\n");
    }

    exit(0);
}

void handle_show_reply_data(char *buf, struct nlmsghdr *nh, ssize_t recv_len) {
    size_t len, info_len;
    struct ifinfomsg *message;
    struct rtattr *rta;
    void* info;

    message = (struct ifinfomsg *)NLMSG_DATA(nh);

    rta = IFLA_RTA(message);
    len = nh->nlmsg_len - NLMSG_LENGTH(sizeof *message);

    info_repr repr;

    repr.flags = message->ifi_flags;

    for (; RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        info = RTA_DATA(rta);
        info_len = RTA_PAYLOAD(rta);

        if(rta->rta_type == IFLA_IFNAME) {
            repr.name = info;
        }
        if(rta->rta_type == IFLA_MTU) {
            repr.mtu = *((unsigned*)info);
        }
        if(rta->rta_type == IFLA_QDISC) {
            repr.qdisc = info;
        }
        if(rta->rta_type == IFLA_OPERSTATE) {
            repr.state = *((unsigned*)info);
        }
        if(rta->rta_type == IFLA_OPERSTATE) {
            repr.state = *((unsigned*)info);
        }
        if(rta->rta_type == IFLA_LINKMODE) {
            repr.mode = *((unsigned*)info);
        }
        if(rta->rta_type == IFLA_GROUP) {
            repr.group = *((unsigned*)info);
        }
        if(rta->rta_type == IFLA_TXQLEN) {
            repr.qlen = *((unsigned*)info);
        }
        if(rta->rta_type == IFLA_ADDRESS) {
            for(int i = 0; i < info_len; i++) {
                repr.address[i] = *((char*)info + i) & 0xff;
            }
        }
        if(rta->rta_type == IFLA_BROADCAST) {
            for(int i = 0; i < info_len; i++) {
                repr.broadcast[i] = *((char*)info + i) & 0xff;
            }
        }
    }

    print_info_repr(&repr);
}


// TODO create named constants equal operation modes
operation_mode mode_array[6] = {
    {
        // show bridges
        .mode = 1,
        .req_creation = create_show_req,
        .pre_error_handling = NULL,
        .error_handling = NULL,
        .reply_data_handling = handle_show_reply_data
    },

    {
        // add bridge
        .mode = 2,
        .req_creation = create_add_req,
        .pre_error_handling = NULL,
        .error_handling = handle_add_error,
        .reply_data_handling = NULL
    },

    {
        // delete bridge
        .mode = 3,
        .req_creation = create_del_req,
        .pre_error_handling = NULL,
        .error_handling = handle_del_error,
        .reply_data_handling = NULL
    },

    {
        // GETLINK request for addif and delif commands
        .mode = 4,
        .req_creation = create_if_getlink_req,
        .pre_error_handling = handle_if_getlink_pre_error,
        .error_handling = handle_if_error,
        .reply_data_handling = NULL
    },

    {
        // NEWLINK request for addif command
        .mode = 5,
        .req_creation = create_addif_newlink_req,
        .pre_error_handling = NULL,
        .error_handling = handle_if_error,
        .reply_data_handling = NULL
    },

    {
        // NEWLINK request for delif command
        .mode = 6,
        .req_creation = create_delif_newlink_req,
        .pre_error_handling = NULL,
        .error_handling = handle_if_error,
        .reply_data_handling = NULL
    }
};

void* create_req(int mode, const void *arg) {
    if(mode_array[mode - 1].req_creation != NULL) {
        return mode_array[mode - 1].req_creation(arg);
    }

    return NULL;
}

int handle_before_error_handling(int mode, struct nlmsghdr *nh) {
    if(mode_array[mode - 1].pre_error_handling != NULL) {
        return mode_array[mode - 1].pre_error_handling(nh);
    }

    return -1;
}

void handle_errors(int mode, struct nlmsghdr *nh) {
    if(mode_array[mode - 1].error_handling != NULL) {
        return mode_array[mode - 1].error_handling(nh);
    }

    return;
}

void handle_reply_data(int mode, char *buf, struct nlmsghdr *nh, ssize_t recv_len) {
    if(mode_array[mode - 1].reply_data_handling != NULL) {
        return mode_array[mode - 1].reply_data_handling(buf, nh, recv_len);
    }

    return;
}

int send_request_and_handle_reply(int mode, const void* arg) {
    char buf[16192] = {0};
    size_t seq_num = 0;
    struct sockaddr_nl sa = {0};
    struct iovec iov = {0};
    struct msghdr msg = {0};
    struct nlmsghdr *nh;

    nl_req_show_t *req = NULL;  // initial value to override by condition

    req = create_req(mode, arg);

    int fd = create_socket();

    req->header.nlmsg_seq = ++seq_num;
    req->header.nlmsg_pid = 0;

    req->msg.ifi_type = ARPHRD_NETROM;
    req->msg.ifi_flags = 0;
    req->msg.ifi_change = 0;

    sa.nl_family = AF_NETLINK;
    iov.iov_base = req;
    iov.iov_len = req->header.nlmsg_len;

    msg.msg_name = &sa;
    msg.msg_namelen = sizeof sa;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if(sendmsg(fd, &msg, 0) < 0) {
        perror("Error sending to netlink socket");
    }

    free(iov.iov_base);

    iov.iov_base = buf;
    iov.iov_len = sizeof buf;

    while(1) {
        ssize_t recv_len = recvmsg(fd, &msg, MSG_DONTWAIT);

        if(recv_len < 0) {
            if(errno & EINTR) {
                usleep(250000);
            }
            else if(errno & EAGAIN) {
                break;
            }
            continue;
        }

        if(iov.iov_base == NULL) {  // first reply to getlink in addif
            continue;
        }

        for (nh = (struct nlmsghdr *)buf;
             NLMSG_OK(nh, (__u32)recv_len); nh = NLMSG_NEXT(nh, recv_len)) {

            if(nh->nlmsg_type == NLMSG_DONE) {
                exit(0);
            }

            // TODO specify pre-error hanling
            int handle_before_error_result = handle_before_error_handling(mode, nh);

            if(nh->nlmsg_type == NLMSG_ERROR) {
                handle_errors(mode, nh);
            }

            handle_reply_data(mode, buf, nh, recv_len);
        }
    }

    close(fd);

    return 0;
}

void run_command(int opcode, char** args) {
    if(opcode <= 3) {
        send_request_and_handle_reply(opcode, args[0]);
    }
    else if(opcode == 4) {
        send_request_and_handle_reply(4, args[0]);
        send_request_and_handle_reply(4, args[1]);
        send_request_and_handle_reply(5, args);  // pass array of strings not string in this case
    }
    else if(opcode == 5) {
        send_request_and_handle_reply(4, args[1]);
        send_request_and_handle_reply(6, args[1]);
    }
}
