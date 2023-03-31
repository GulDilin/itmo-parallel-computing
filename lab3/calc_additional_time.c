#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>


void print_delta(struct timeval T1, struct timeval T2) {
    long delta_us = 1000000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec);
    printf("%ld\n", delta_us);
}

int main(int argc, char *argv[]) {
    struct timeval T1, T2;
    gettimeofday(&T1, NULL);
    const int M = atoi(argv[1]); /* M - amount of threads */
    omp_set_dynamic(0);
    omp_set_num_threads(M);
    int s = 0;
    #pragma omp parallel
    {
        s++;
    }
    gettimeofday(&T2, NULL);
    print_delta(T1, T2);
    return 0;
}
