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

int create_socket() {
    int fd = socket(AF_NETLINK, SOCK_RAW|SOCK_CLOEXEC, NETLINK_ROUTE);

    if(fd < 0) {
        perror("Error netlink socket opening");
    }

    return fd;
}

int show_bridges() {
    char buf[16192] = {0};
    size_t seq_num = 0;
    struct sockaddr_nl sa = {0};
    struct iovec iov = {0};
    struct msghdr msg = {0};
    struct nlmsghdr *nh;
    nl_req_show_t req = {0};

    int fd = create_socket();

    req.header.nlmsg_type = RTM_GETLINK;
    req.header.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.header.nlmsg_seq = ++seq_num;
    req.header.nlmsg_pid = 0;

    req.msg.ifi_family = AF_PACKET;
    req.msg.ifi_type = ARPHRD_NETROM;
    req.msg.ifi_index = 0;
    req.msg.ifi_flags = 0;
    req.msg.ifi_change = 0;

    req.attr1.rta_len = sizeof(struct rtattr) + sizeof(req.data1);
    req.attr1.rta_type = IFLA_EXT_MASK;
    req.data1 = RTEXT_FILTER_VF;

    req.attr2_1.rta_len = sizeof(struct rtattr) + sizeof(req.data2);
    req.attr2_1.rta_type = IFLA_INFO_KIND;
    strcpy(req.data2, "bridge");

    req.attr2.rta_len = sizeof(struct rtattr) + req.attr2_1.rta_len;
    req.attr2.rta_type = IFLA_LINKINFO;

    req.header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req.attr1.rta_len + req.attr2.rta_len;

    sa.nl_family = AF_NETLINK;
    iov.iov_base = &req;
    iov.iov_len = req.header.nlmsg_len;

    msg.msg_name = &sa;
    msg.msg_namelen = sizeof sa;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if(sendto(fd, &req, sizeof(req), 0, NULL, 0) < 0) {
        perror("Error sending to netlink socket");
    }

    iov.iov_base = buf;
    iov.iov_len = sizeof buf;

    while(1) {
        ssize_t recv_len = recvmsg(fd, &msg, MSG_DONTWAIT);

        if (recv_len < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                usleep(250000);
            }
            continue;
        }

        for (nh = (struct nlmsghdr *)buf;
             NLMSG_OK(nh, (__u32)recv_len); nh = NLMSG_NEXT(nh, recv_len)) {

            size_t len, info_len;
            struct ifinfomsg *message;
            struct rtattr *rta;
            void* info;

            if(nh->nlmsg_type == NLMSG_DONE) {
                exit(0);
            }

            message = (struct ifinfomsg *)NLMSG_DATA(nh);

            rta = IFLA_RTA(message);
            len = nh->nlmsg_len - NLMSG_LENGTH(sizeof *message);

            info_repr repr;

            repr.flags = message->ifi_flags;

            for (; RTA_OK(rta, len); rta = RTA_NEXT(rta, len))
            {
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
    }

    return 0;
}

int add_bridge(const char* bridge_name) {
    size_t seq_num = 0;
    struct sockaddr_nl sa = {0};
    struct iovec iov = {0};
    struct msghdr msg = {0};
    nl_req_add_t req = {0};

    int fd = create_socket();

    req.header.nlmsg_type = RTM_NEWLINK;
    req.header.nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL|NLM_F_CREATE;
    req.header.nlmsg_seq = ++seq_num;
    req.header.nlmsg_pid = 0;

    req.msg.ifi_family = AF_UNSPEC;
    req.msg.ifi_type = ARPHRD_NETROM;
    req.msg.ifi_index = 0;
    req.msg.ifi_flags = 0;
    req.msg.ifi_change = 0;

    req.attr1.rta_len = sizeof(struct rtattr) + sizeof(req.data1);
    req.attr1.rta_type = IFLA_IFNAME;
    strcpy(req.data1, bridge_name);

    req.attr2_1.rta_len = sizeof(struct rtattr) + sizeof(req.data2);
    req.attr2_1.rta_type = IFLA_INFO_KIND;
    strcpy(req.data2, "bridge");

    req.attr2.rta_len = sizeof(struct rtattr) + req.attr2_1.rta_len;
    req.attr2.rta_type = IFLA_LINKINFO;

    req.header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req.attr1.rta_len + req.attr2.rta_len;

    sa.nl_family = AF_NETLINK;
    iov.iov_base = &req;
    iov.iov_len = req.header.nlmsg_len;

    msg.msg_name = &sa;
    msg.msg_namelen = sizeof sa;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if(sendmsg(fd, &msg, 0) < 0) {
        perror("Error sending to netlink socket");
    }

    // TODO add recvmsg

    show_bridges();

    return 0;
}

int delete_bridge(const char* bridge_name) {
    size_t seq_num = 0;
    struct sockaddr_nl sa = {0};
    struct iovec iov = {0};
    struct msghdr msg = {0};
    nl_req_del_t req = {0};

    int fd = create_socket();

    req.header.nlmsg_type = RTM_DELLINK;
    req.header.nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK;
    req.header.nlmsg_seq = ++seq_num;
    req.header.nlmsg_pid = 0;

    req.msg.ifi_family = AF_UNSPEC;
    req.msg.ifi_type = ARPHRD_NETROM;
    req.msg.ifi_index = if_nametoindex(bridge_name);
    req.msg.ifi_flags = 0;
    req.msg.ifi_change = 0;

    req.attr1.rta_len = sizeof(struct rtattr);
    req.attr1.rta_type = IFLA_LINKINFO;

    req.attr2.rta_len = sizeof(struct rtattr) + sizeof(req.data2);
    req.attr2.rta_type = IFLA_INFO_KIND;
    strcpy(req.data2, "bridge");

    req.header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg))
            + req.attr1.rta_len + req.attr2.rta_len;

    sa.nl_family = AF_NETLINK;
    iov.iov_base = &req;
    iov.iov_len = req.header.nlmsg_len;

    msg.msg_name = &sa;
    msg.msg_namelen = sizeof sa;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if(sendmsg(fd, &msg, 0) < 0) {
        perror("Error sending to netlink socket");
    }

    // TODO add recvmsg

    show_bridges();

    return 0;
}

void print_usage_description() {
    typedef struct {
        char *command;
        char *description;
    } entry;

    entry entries[3] = {
        { .command = "show", .description = "show list of bridges" },
        { .command = "addbr <bridge_name>", .description = "add bridge" },
        { .command = "delbr <bridge_name>", .description = "delete bridge" },
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