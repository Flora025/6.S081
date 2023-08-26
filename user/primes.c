#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void printPrimes(int *input, int count) {
    // termination
    if (count == 0) {
        return;
    }

    int num; // to be read into later
    int i = 0, prime = *input;
    int p[2];
    pipe(p);

    printf("prime %d\n", prime);

    if (fork() == 0) {
        // child process: write into the next process
        close(p[0]);
        for (; i < count; i++) {
            // write the input into the pipe
            write(p[1], (int *)(input + i), sizeof(int));
        }
        close(p[1]);
        exit(0);
    } else {
        // parent process: sieve the nums and send to the next stage
        close(p[1]);
        count = 0; // count of prime nums so far
        // continue to read from the pipe || while p[1] is open
        while (read(p[0], &num, sizeof(int)) != 0) {
            if (num % prime != 0) {
                // is prime
                *input = num;
                input++;
                count++;
            }
        }
        // INPUT pointer restored to the initial position
        printPrimes(input - count, count); // (startingPosition, number of nums)
    
        close(p[0]);
        wait(0);
    }
}

int main(int argc, char *argv[]) {
    int input[34], i = 0;
    // init the input array
    for (; i < 34; i++) {
        input[i] = i + 2;
    }
    printPrimes(input, 34);
    exit(0);
}
