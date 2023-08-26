#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void get_next_primes(int read_fd) {
    int prime, cur_num;
    int pp[2];

    // print the first prime
    if (read(read_fd, &prime, sizeof(int)) != 0) {
        // if read succeeds
        printf("prime %d\n", prime);
    }
    
    // if there's remaining num(s)
    if (read(read_fd, &cur_num, sizeof(int)) != 0) {
        pipe(pp);
        if (fork() == 0) {
            // child process
            close(pp[0]);
            // write the already read num into pipe if it's prime
            if (cur_num % prime != 0) {
                write(pp[1], &cur_num, sizeof(int));
            }
            // write the other primes (if any)
            while (read(read_fd, &cur_num, sizeof(int)) != 0) {
                if (cur_num % prime != 0) {
                    write(pp[1], &cur_num, sizeof(int));
                }
            }
            close(pp[1]);
            exit(0);
        } else {
            // parent process
            close(pp[1]);
            wait(0);
            get_next_primes(pp[0]);
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    int p[2];
    pipe(p);
    int i;

    // write into the pipe sequentially
    for (i = 2; i <= 35; i++) {
        write(p[1], &i, sizeof(int));
    }
    close(p[1]);

    get_next_primes(p[0]);
    exit(0);
}
