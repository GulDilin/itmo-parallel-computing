#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>


void swap(double *a, double *b) {
    double t;
    t = *a, *a = *b, *b = t;
}


void sort_stupid(double *array, int n) {
    int i = 0;
    while (i < n - 1) {
        if (array[i + 1] < array[i]) swap(array + i, array + i + 1), i = 0;
        else i++;
    }
}

void print_arr(double *array, int n) {
    for (int i = 0; i < n; ++i)
    {
        printf("%f ", array[i]);
    }
    printf("\n");
}

void print_delta(struct timeval T1, struct timeval T2) {
    long delta_ms = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\n%ld\n", delta_ms);
}

int main(int argc, char *argv[]) {
    struct timeval T1, T2;
    gettimeofday(&T1, NULL); /* запомнить текущее время T1 */

    for (unsigned int i = 0; i < 100; i++) /* 100 экспериментов */
    {
        unsigned int N, N_2;
        N = atoi(argv[1]); /* N равен первому параметру командной строки */
        N_2 = N / 2;

        double * restrict m1 = malloc(N * sizeof(double));
        double * restrict m2 = malloc(N_2 * sizeof(double));
        double * restrict m2_cpy = malloc(N_2 * sizeof(double));
        int A = 280;

        unsigned int seedp = i;

        // generate 1
        for (int j = 0; j < N; ++j) {
            m1[j] = (rand_r(&seedp) % (A * 100)) / 100.0 + 1;
        }

        // generate 2
        for (int j = 0; j < N_2; ++j) {
            m2[j] = A + rand_r(&seedp) % (A * 9);
        }

        for (int j = 0; j < N_2; ++j) {
            m2_cpy[j] = m2[j];
        }

        // map
        for (int j = 0; j < N; ++j) {
            m1[j] = 1 / tanh(sqrt(m1[j]));
        }
        for (int j = 1; j < N_2; ++j) {
            m2[j] = m2[j] + m2_cpy[j - 1];
        }
        for (int j = 1; j < N_2; ++j) {
            m2[j] = pow(log10(m2[j]), M_E);
        }

        for (int j = 0; j < N_2; ++j) {
            m2[j] = m2[j] > m1[j] ? m2[j] : m1[j] ;
        }

        // sort_stupid(m2, N_2);

        // reduce
        double X = 0;
        int k = 0;
        while (m2[k] == 0 && k < N_2 - 1) k++;
        double m2_min = m2[k];

        for (int j = 0; j < N_2; ++j) {
            if((int)(m2[j] / m2_min) % 2 == 0) X += sin(m2[j]);
        }
        printf("%f ", X);
    }
    gettimeofday(&T2, NULL);
    print_delta(T1, T2);
    return 0;
}
