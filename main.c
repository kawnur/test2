#include <stdio.h>
#include <string.h>

#include "utility.h"


// Opcodes:
//   1 - show
//   2 - addbr <bridge_name>
//   3 - delbr <bridge_name>
//   4 - addif <bridge_name> <if_name>
//   5 - delif <bridge_name> <if_name>

// TODO create named constants equal opcodes above

int main(int argc, char* argv[]) {
    if(argc == 2 && strcmp(argv[1], "show") == 0) {
        char* args[] = { "" };
        run_command(1, args);
    }

    else if(argc == 3 && strcmp(argv[1], "addbr") == 0) {
        char* args[] = { argv[2] };
        run_command(2, args);
    }

    else if(argc == 3 && strcmp(argv[1], "delbr") == 0) {
        char* args[] = { argv[2] };
        run_command(3, args);
    }

    else if(argc == 4 && strcmp(argv[1], "addif") == 0) {
        char* args[] = { argv[2], argv[3] };
        run_command(4, args);
    }

    else if(argc == 4 && strcmp(argv[1], "delif") == 0) {
        char* args[] = { argv[2], argv[3] };
        run_command(5, args);
    }

    else {
        print_usage_description();
    }

    return 0;
}
