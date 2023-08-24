#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int p[2];

    pipe(p);
    if (fork() == 0) {
        // child process
        close(p[1]);  // close unused write end

        char byte;
        read(p[0], &byte, sizeof(byte));

        printf("%d: received ping\n", getpid());
        close(p[0]); // close read end
        exit(0);
    } else {
        // parent process
        close(p[0]); // close read end

        char byte = 'A';
        write(p[1], &byte, sizeof(byte));

        wait(0);
        printf("%d: received pong\n", getpid());
        
        close(p[1]); // close write end
        exit(0);
    }
}
