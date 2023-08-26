#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    char *buf[MAXARG - 1];
    int i, offset;
    char ch;

    // alloc space for buffer
    for (i = 0; i < MAXARG - 1; i++) {
        buf[i] = malloc(sizeof(buf));
        memset(buf[i], 0, sizeof(buf[i]));  // set all el to 0
    }

    // save all args to the arr
    for (i = 1; i < argc; i++) {
        strcpy(buf[i - 1], argv[i]);
    }

    // each time reading an element into str
    offset = 0;
    while (read(0, &ch, 1) != 0) {
        if (ch != '\n') {
            // if this character != \n, add it to the last of buf
            *(buf[argc - 1] + (offset++)) = ch;
        } else {
            // if the ch == \n
            *(buf[argc - 1] + (offset++)) = '\0';  // finish the input
            if (fork() == 0) {
                // child process
                exec(buf[0], buf);
                exit(0);
            } else {
                // parent process
                wait(0);
            }
        }
    }
    exit(0);
}
