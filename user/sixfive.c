#include "kernel/types.h"
#include "user/user.h"

typedef int (*state_handler)(char);
typedef enum {
    ST_LOOKING,
    ST_DIGIT,
    ST_INVALID
}state;

char buf[512];
int p = 0;
const char *seps = " -\r\t\n./,";

int handle_looking(char c) {
    if (strchr(seps, c)) return ST_LOOKING;
    if (c >= '0' && c <= '9') {
        p = 0;
        buf[p++] = c;
        return ST_DIGIT;
    }
    return ST_INVALID;
}

int handle_digit(char c) {
    if (c >= '0' && c <= '9') {
        if (p < 31) buf[p++] = c;
        return ST_DIGIT;
    }
    if (strchr(seps, c)) {
        buf[p] = '\0';
        int n = atoi(buf);
        if (n > 0 && (n % 5 == 0 || n % 6 ==0)){
            printf("%d\n", n);
        }
        return ST_LOOKING;
    }
    return ST_INVALID;
}

int handle_invalid(char c) {
    if (strchr(seps, c)) return ST_LOOKING;
    return ST_INVALID;
}

state_handler machine[] = {
    [ST_LOOKING] = handle_looking,
    [ST_DIGIT] = handle_digit,
    [ST_INVALID] = handle_invalid
};

int main(int argc,char **argv) {
    if (argc < 2) {
        fprintf(2, "usage: sixfive <file1> <file2> ...");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], 0);
        if (fd < 0) {
            fprintf(2, "sixfive: cannot open file%s\n", argv[i]);
            continue;
        }

        int state = ST_LOOKING;
        char c;
        p = 0;

        while(read(fd, &c, 1) > 0) {
            state = machine[state](c);
        } 

        machine[state]('\n');
        close(fd);
    }
    exit(0);
}