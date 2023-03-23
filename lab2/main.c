#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "FW_1.3.1_Lin64/fwBase.h"
#include "FW_1.3.1_Lin64/fwSignal.h"


void swap(Fw32f *a, Fw32f *b) {
    Fw32f * restrict t = NULL;
    *t = *a, *a = *b, *b = *t;
}


void sort_stupid(Fw32f *array, int n) {
    int i = 0;
    while (i < n - 1) {
        if (array[i + 1] < array[i]) swap(array + i, array + i + 1), i = 0;
        else i++;
    }
}

void print_arr(Fw32f *array, int n) {
    for (int i = 0; i < n; ++i)
    {
        printf("%f ", array[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    struct timeval T1, T2;
    long delta_ms;
    gettimeofday(&T1, NULL); /* save current time for benchmarking T1 */

    const int N = atoi(argv[1]); /* N - array size, equals first cmd param */
    const int M = atoi(argv[2]); /* M - amount of threads */
    fwSetNumThreads(M);

    const int N_2 = N / 2;
    const int A = 280;

    Fw32f * restrict m1 = fwsMalloc_32f(N);
    Fw32f * restrict m2 = fwsMalloc_32f(N_2);
    Fw32f * restrict m2_cpy = fwsMalloc_32f(N_2);
    Fw64f * restrict m2_cpy_2 = fwsMalloc_64f(N_2);


    for (unsigned int i = 0; i < 100; i++) /* 100 experiments */
    {

        unsigned int seedp = i;

        // generate 1
        for (int j = 0; j < N; ++j) {
            m1[j] = (rand_r(&seedp) % (A * 100)) / 100.0 + 1;
        }

        // generate 2
        for (int j = 0; j < N_2; ++j) {
            m2[j] = A + rand_r(&seedp) % (A * 9);
        }

        fwsCopy_32f(m2, m2_cpy, N_2);
        // m1[j] = 1 / tanh(sqrt(m1[j]));
        fwsSqrt_32f(m1, m1, N);
        fwsTanh_32f_A24(m1, m1, N);
        fwsDivCRev_32f(m1, 1, m1, N);

        // m2[j] = m2[j] + m2_cpy[j - 1]
        fwsAdd_32f(m2 + 1, m2_cpy, m2 + 1, N_2 - 1);

        // m2[j] = pow(log10(m2[j]), M_E)
        fwsLog10_32f_A24(m2, m2, N_2);
        fwsPowx_32f_A24(m2, M_E, m2, N_2);

        for (int j = 0; j < N_2; ++j) {
            m2[j] = fmax(m2[j], m1[j]);
        }

        // sort_stupid(m2, N_2);

        // reduce
        Fw32f m2_min;
        fwsMin_32f(m2, N_2, &m2_min);

        fwsZero_32f(m2_cpy, N_2);
        fwsZero_64f(m2_cpy_2, N_2);
        fwsDivC_32f(m2, m2_min, m2_cpy, N_2);

        Fw64f X = 0;
        for (int j = 0; j < N_2; ++j) {
            // find where m2/m2_min % 2 == 0 and copy to m2_cpy_2
            if((int) m2_cpy[j] % 2 == 0) m2_cpy_2[j] = m2[j];
        }
        fwsSin_64f_A50(m2_cpy_2, m2_cpy_2, N_2);
        fwsSum_64f(m2_cpy_2, N_2, &X);

        printf("%f ", X);
    }

    fwsFree(m1);
    fwsFree(m2);
    fwsFree(m2_cpy);
    fwsFree(m2_cpy_2);

    gettimeofday(&T2, NULL);
    /* запомнить текущее время T2 */
    delta_ms = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\n%ld\n", delta_ms); /* T2 - T1 */
    return 0;
}
