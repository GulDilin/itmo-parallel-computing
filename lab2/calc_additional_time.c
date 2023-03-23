#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "FW_1.3.1_Lin64/fwBase.h"
#include "FW_1.3.1_Lin64/fwSignal.h"


void print_delta(struct timeval T1, struct timeval T2) {
    long delta_us = 1000000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec);
    printf("%ld\n", delta_us);
}

int main(int argc, char *argv[]) {
    struct timeval T1, T2;
    gettimeofday(&T1, NULL);
    const int N = atoi(argv[1]); /* N - max threads amount */
    int s = 0;
    fwSetNumThreads(N);
    s++;
    gettimeofday(&T2, NULL);
    print_delta(T1, T2);
    return 0;
}
