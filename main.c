#include <stdio.h>
#include <string.h>

#include "utility.h"


int main(int argc, char* argv[]) {
    if(argc == 2 && strcmp(argv[1], "show") == 0) {
        show_bridges();
    }

    else if(argc == 3 && strcmp(argv[1], "addbr") == 0) {
        add_bridge(argv[2]);
    }

    else if(argc == 3 && strcmp(argv[1], "delbr") == 0) {
        delete_bridge(argv[2]);
    }

    else {
        print_usage_description();
    }

    return 0;
}
