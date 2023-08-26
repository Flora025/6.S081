#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


// read_fp: the file descriptor to read from
void sieve(int prime, int read_fd) {
    printf("prime %d\n", prime);

    int p[2];
    int num; // to be read into later
    pipe(p);

    if (fork() == 0) {
        // child process: create a new process
        close(p[1]); // only have to read, close the write end
        // 下个进程会从p[0]的fd里读取【在parent中写进p[1]】的num
        sieve(prime, p[0]); 
        exit(0);
    } else {
        // parent process: sieve primes and send them to the next process 
        close(p[0]); // close read end

        while (read(read_fd, &num, sizeof(int)) != 0) {
            // i.e., the write end of the pipe is open || READ is successful
            if (num % prime != 0) {
                // is prime, write to the next process
                write(p[1], &num, sizeof(int));
            }
        }

        close(p[1]); // close write end
        wait(0); // wait for the child process
    }
}

void generate_primes(int start, int end) {
    int p[2];
    pipe(p);

    if (fork() == 0) {
        // child process: start the sieving process
        close(p[1]);
        sieve(2, p[0]);
        exit(0);
    } else {
        // parent process: write all nums into the pipe sequentially
        close(p[0]); // no need to read

        for (int i = start; i <= end; i++) {
            write(p[1], &i, sizeof(int));
        }
        close(p[1]);

        wait(0);
    }
}

int main(int argc, char *argv[]) {
    generate_primes(2, 35);
    return 0;
}
