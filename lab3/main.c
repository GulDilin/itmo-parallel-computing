#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>


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
    unsigned long long delta_ms = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\n%llu\n", delta_ms);
}

int main(int argc, char *argv[]) {
    struct timeval T1, T2;
    gettimeofday(&T1, NULL); /* запомнить текущее время T1 */

    const int N = atoi(argv[1]); /* N - array size, equals first cmd param */

    const int N_2 = N / 2;
    const int A = 280;

    double * restrict m1 = malloc(N * sizeof(double));
    double * restrict m2 = malloc(N_2 * sizeof(double));
    double * restrict m2_cpy = malloc(N_2 * sizeof(double));

    #if defined(_OPENMP)
        omp_set_dynamic(0);
        const int M = atoi(argv[2]); /* M - amount of threads */
        omp_set_num_threads(M);
    #endif

    for (unsigned int i = 0; i < 1; i++) /* 100 экспериментов */
    {
        double X = 0;
        unsigned int seedp = i;


        for (int j = 0; j < N; ++j) {
            m1[j] = (rand_r(&seedp) % (A * 100)) / 100.0 + 1;
        }

        // generate 2
        for (int j = 0; j < N_2; ++j) {
            m2[j] = A + rand_r(&seedp) % (A * 9);
        }

        #pragma omp parallel default(none) shared(N, N_2, A, m1, m2, m2_cpy, i, X)
        {
            #pragma omp for
            for (int j = 0; j < N_2; ++j) {
                m2_cpy[j] = m2[j];
            }

            // map
            #pragma omp for
            for (int j = 0; j < N; ++j) {
                m1[j] = 1 / tanh(sqrt(m1[j]));
            }
            #pragma omp for
            for (int j = 1; j < N_2; ++j) {
                m2[j] = m2[j] + m2_cpy[j - 1];
            }
            #pragma omp for
            for (int j = 1; j < N_2; ++j) {
                m2[j] = pow(log10(m2[j]), M_E);
            }

            #pragma omp for
            for (int j = 0; j < N_2; ++j) {
                m2[j] = m2[j] > m1[j] ? m2[j] : m1[j] ;
            }

            // sort_stupid(m2, N_2);

            // reduce
            int k = 0;
            while (m2[k] == 0 && k < N_2 - 1) k++;
            double m2_min = m2[k];

            #pragma omp for
            for (int j = 0; j < N_2; ++j) {
                m2_cpy[j] = 0;
                if((int)(m2[j] / m2_min) % 2 == 0) m2_cpy[j] = sin(m2[j]);
            }

            #pragma omp for reduction(+ : X)
            for (int j = 0; j < N_2; ++j) {
                X += m2_cpy[j];
            }
        }
        printf("%f ", X);
    }
    gettimeofday(&T2, NULL);
    print_delta(T1, T2);
    return 0;
}
