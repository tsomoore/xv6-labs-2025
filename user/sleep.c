#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc,char **argv) {
    if (argc != 2) {
        fprintf(2, "usage: sleep <duration>");
        exit(1);
    }

    int duration = atoi(argv[1]);
    pause(duration);

    exit(0);
}