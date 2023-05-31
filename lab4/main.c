#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <omp.h>
#include <string.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))
// #define DEBUG 1
#define BENCHMARK 1

#ifdef _OPENMP
    // here omp_get_num_procs implementation exists
#else
    int omp_get_num_procs() { return 1; }
    double omp_get_wtime() {
        struct timeval t;
        gettimeofday(&t, NULL);
        return t.tv_sec + t.tv_usec / 1000000.0;
    }
#endif

void swap(double *a, double *b) {
    double t;
    t = *a, *a = *b, *b = t;
}


void print_arr(double *array, int n) {
    for (int i = 0; i < n; ++i)
    {
        printf("%f ", array[i]);
    }
    printf("\n");
}

void print_arr_dbg(double *array, int n) {
    #ifdef DEBUG
        print_arr(array, n);
    #endif
}

void sort_stupid(double *array, int n) {
    #ifdef DEBUG
        printf("sort stupid\n");
        print_arr(array, n);
    #endif

    int i = 0;
    while (i < n - 1) {
        if (array[i + 1] < array[i]) swap(array + i, array + i + 1), i = 0;
        else i++;
    }

    #ifdef DEBUG
        printf("sort stupid end\n");
        print_arr(array, n);
    #endif
}
void merge_sorted(double *src1, int n1, double *src2, int n2, double *dst) {
    #ifdef DEBUG
        printf("merge_sorted\n");
        print_arr(src1, n1);
        print_arr(src2, n2);
    #endif

    int i = 0, i1 = 0, i2 = 0;
    while (i < n1 + n2) {
        dst[i++] = src1[i1] > src2[i2] && i2 < n2 ? src2[i2++] : src1[i1++];
    }

    #ifdef DEBUG
        printf("merge_sorted end\n");
        print_arr(dst, n1 + n2);
    #endif
}

void copy_array(double *dst, double *src, int n) {
    for (int i = 0; i < n; ++i)
    {
        dst[i] = src[i];
    }
}

static inline void sort(double *array, int n, double *dst) {
    #ifdef DEBUG
        printf("sort\n");
        print_arr(array, n);
    #endif

    int n1 = n / 2;
    int n2 = n - n1;
    #pragma omp sections
    {
        #pragma omp section
        {
            sort_stupid(array, n1);
        }

        #pragma omp section
        {
            sort_stupid(array + n1, n2);
        }
    }
    merge_sorted(array, n1, array + n1, n2, dst);

    #ifdef DEBUG
        printf("sort end\n");
        print_arr(dst, n);
    #endif
}

void sort_dynamic(double *array, int n, double *dst, int n_threads) {
    #ifdef DEBUG
        printf("sort_dynamic\n");
        print_arr(array, n);
    #endif

    int n_chunk = 0;
    #pragma omp single
    {
        n_chunk = n_threads < 2 ? n : ceil((double) n / n_threads);
    }
    #pragma omp for
    {
        for (int k = 0; k < n_threads; ++k)
        {
            int n_done = n_chunk * k;
            int n_cur_chunk = min((n - n_done), n_chunk);
            // for debug
            #ifdef DEBUG
                printf("parallel for k: %d  n_chunk: %d n_done: %d n_cur_chunk: %d\n", k, n_chunk, n_done, n_cur_chunk);
            #endif
            sort_stupid(array + n_done, n_cur_chunk);
        }
    }

    #pragma omp single
    {
        double * restrict cpy = malloc(n * sizeof(double));

        copy_array(cpy, array, n);
        copy_array(dst, array, n);
        for (int k = 1; k < n_threads; ++k)
        {
            int n_done = n_chunk * k;
            int n_cur_chunk = min(n - n_done, n_chunk);
            int n_will_done = n_done + n_cur_chunk;
            merge_sorted(cpy, n_done, array + n_done, n_cur_chunk, dst);
            copy_array(cpy, dst, n_will_done);
        }
        free(cpy);
    }

    #ifdef DEBUG
        printf("sort_dynamic end\n");
        print_arr(dst, n);
    #endif
}

void print_delta(double T1, double T2) {
    printf("\n%f", (T2 - T1) * 1000.0);
}

// --------------- BENCHMARK

struct benchmarks_info {
    double * restrict benchmarking_results;
    double * restrict benchmarking_time;
};

void init_benchmarks(volatile struct benchmarks_info * t_info, int n) {
    #ifdef BENCHMARK
        t_info -> benchmarking_time = malloc(n * sizeof(double));
        t_info -> benchmarking_results = malloc(n * sizeof(double));
        for (int i = 0; i < n; ++i) {
            t_info -> benchmarking_results[i] = 0;
        }
    #endif
}

void start_benchmark(volatile struct benchmarks_info * t_info, int idx) {
    #pragma omp single
    {
        #ifdef BENCHMARK
            t_info -> benchmarking_time[idx] = omp_get_wtime();
        #endif
    }
}

void finish_benchmark(volatile struct benchmarks_info * t_info, int idx) {
    #pragma omp single
    {
        #ifdef BENCHMARK
            t_info -> benchmarking_results[idx] += omp_get_wtime() - t_info -> benchmarking_time[idx];
        #endif
    }
}

void show_benchmark_results(volatile struct benchmarks_info * t_info, int n) {
    #ifdef BENCHMARK
        printf("\n\nBENCHMARK\n");
        for (int i = 0; i < n; ++i) {
            printf("%f\n", t_info -> benchmarking_results[i] * 1000);
        }
        printf("\n");
        free(t_info -> benchmarking_time);
        free(t_info -> benchmarking_results);
    #endif
}

// --------------- BENCHMARK END


int main(int argc, char *argv[]) {
    double T1, T2;
    T1 = omp_get_wtime();

    int finished = 0;
    int i = 0;

    #pragma omp parallel sections num_threads(2) shared(i, finished)
    {
        #ifdef _OPENMP
            #pragma omp section
            {
                double time = 0;
                while (finished < 1) {
                    double time_temp = omp_get_wtime();
                    if (time_temp - time < 1) {
                        usleep(100);
                        continue;
                    };
                    printf("\nPROGRESS: %d\n", i);
                    time = time_temp;
                }
            }
        #endif

        #pragma omp section
        {
            volatile struct benchmarks_info t_info;
            int N_benchmarks = 4;
            init_benchmarks(&t_info, N_benchmarks);

            const int N = atoi(argv[1]); /* N - array size, equals first cmd param */
            const int N_sort_threads = argc > 3 ? atoi(argv[3]) : 2;

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

            for (i = 0; i < 100; i++) /* 100 экспериментов */
            {
                double X = 0;

                // generate

                #pragma omp parallel default(none) shared(N, N_2, A, m1, m2, m2_cpy, i, X, N_sort_threads, t_info)
                {
                    start_benchmark(&t_info, 0);
                    #pragma omp single
                    {
                        unsigned int seedp = i;
                        for (int j = 0; j < N; ++j) {
                            m1[j] = (rand_r(&seedp) % (A * 100)) / 100.0 + 1;
                        }
                        for (int j = 0; j < N_2; ++j) {
                            m2[j] = A + rand_r(&seedp) % (A * 9);
                        }
                    }
                    finish_benchmark(&t_info, 0);

                    // count ctanh from sqrt of x
                    start_benchmark(&t_info, 1);
                    #pragma omp for
                    for (int j = 0; j < N_2; ++j) {
                        m2_cpy[j] = m2[j];
                    }
                    #pragma omp for
                    for (int j = 0; j < N; ++j) {
                        m1[j] = 1 / tanh(sqrt(m1[j]));
                    }

                    // sum with previous
                    #pragma omp for
                    for (int j = 1; j < N_2; ++j) {
                        m2[j] = m2[j] + m2_cpy[j - 1];
                    }

                    // count log10(x) ^ E
                    #pragma omp for
                    for (int j = 0; j < N_2; ++j) {
                        m2[j] = pow(log10(m2[j]), M_E);
                    }

                    // max between m1 and m2 per item
                    #pragma omp for
                    for (int j = 0; j < N_2; ++j) {
                        m2_cpy[j] = m2[j] > m1[j] ? m2[j] : m1[j] ;
                    }
                    finish_benchmark(&t_info, 1);

                    // sort
                    start_benchmark(&t_info, 2);
                    if (N_sort_threads == 2) {
                        sort(m2_cpy, N_2, m2);
                    } else {
                        sort_dynamic(m2_cpy, N_2, m2, omp_get_num_procs());
                    }
                    finish_benchmark(&t_info, 2);


                    start_benchmark(&t_info, 3);
                    int k = 0;
                    while (m2[k] == 0 && k < N_2 - 1) k++;
                    double m2_min = m2[k];

                    #ifdef DEBUG
                        print_arr(m2, N_2);
                        printf("min %f\n", m2_min);
                    #endif

                    // reduce
                    #pragma omp for
                    for (int j = 0; j < N_2; ++j) {
                        m2_cpy[j] = 0;
                        if((int)(m2[j] / m2_min) % 2 == 0) m2_cpy[j] = sin(m2[j]);
                    }

                    #pragma omp for reduction(+ : X)
                    for (int j = 0; j < N_2; ++j) {
                        X += m2_cpy[j];
                    }

                    #pragma omp barrier
                    finish_benchmark(&t_info, 3);

                }
                printf("%f ", X);
            }
            finished = 1;
            show_benchmark_results(&t_info, N_benchmarks);
        }
    }
    T2 = omp_get_wtime();
    print_delta(T1, T2);
    return 0;
}
