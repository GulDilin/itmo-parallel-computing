#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>


#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
// #define DEBUG 1


void swap(double *a, double *b) {
    double t;
    t = *a, *a = *b, *b = t;
}

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1000000.0;
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
    for (int i = 0; i < n; ++i) {
        dst[i] = src[i];
    }
}

void print_delta(struct timeval T1, struct timeval T2) {
    unsigned long long delta_ms = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\n%llu\n", delta_ms);
}

struct map_data {
    double * src;
    double * dst;
    void * args;
    int arg_size;
    void * callback;
    int length;
    int n_start;
};

struct arg_src2 {
    double src2;
};

void* map_routine(void * arg) {
    struct map_data *data = arg;
    double (*fun_ptr)(double, void*) = data->callback;
    if (data->length < 1) return NULL;
    for (int i = 0; i < data->length; ++i) {
        data -> dst[i] = (*fun_ptr)(data -> src[i], data -> args + i * data -> arg_size);
    }
    pthread_exit(0);
}

void* reduce_routine(void * arg) {
    struct map_data *data = arg;
    double (*fun_ptr)(double, void*) = data->callback;
    if (data->length < 1) return NULL;
    for (int i = 0; i < data->length; ++i) {
        *data->dst = (*fun_ptr)(data->src[i], data->dst);
    }
    pthread_exit(0);
}

void* sort_routine(void * arg) {
    struct map_data *data = arg;
    if (data->length < 1) return NULL;
    int i = 0;
    while (i < data -> length - 1) {
        if (data -> src[i + 1] < data -> src[i]) swap(data -> src + i, data -> src + i + 1), i = 0;
        else i++;
    }
    pthread_exit(0);
}

double copy(double x) {
    return  x;
}

void parallel_separate(
    void* callback,
    void* routine,
    int n_threads,
    double * src,
    double * dst,
    void * args,
    int arg_size,
    int n
) {
    pthread_t * threads = malloc(n_threads * sizeof(pthread_t));
    struct map_data * restrict map_datas = malloc(n_threads * sizeof(struct map_data));
    int n_chunk = n_threads < 2 ? n : ceil((double) n / n_threads);

    for (int i = 0; i < n_threads; ++i) {
        int n_done = n_chunk * i;
        int n_cur_chunk = max(min((n - n_done), n_chunk), 0);
        map_datas[i].callback = callback;
        map_datas[i].src = src + n_done;
        map_datas[i].args = args + n_done * arg_size;
        map_datas[i].arg_size = arg_size;
        if (routine == reduce_routine) {
            map_datas[i].dst = dst;
        } else {
            map_datas[i].dst = dst + n_done;
        }
        map_datas[i].length = n_cur_chunk;
        map_datas[i].n_start = n_done;
    }

    for (int i = 0; i < n_threads; ++i) {
        pthread_create(&threads[i], NULL, routine, &map_datas[i]);
    }
    for (int i = 0; i < n_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
}

void sort_dynamic(double *src, int n, double *dst, int n_threads) {
    #ifdef DEBUG
        printf("sort_dynamic\n");
        print_arr(src, n);
    #endif
    int n_chunk = n_threads < 2 ? n : ceil((double) n / n_threads);

    parallel_separate(NULL, sort_routine, n_threads, src, dst, NULL, 0, n);
    double * restrict cpy = malloc(n * sizeof(double));

    parallel_separate(copy, map_routine, n_threads, src, cpy, NULL, 0, n);
    parallel_separate(copy, map_routine, n_threads, src, dst, NULL, 0, n);
    for (int k = 1; k < n_threads; ++k)
    {
        int n_done = n_chunk * k;
        int n_cur_chunk = min(n - n_done, n_chunk);
        int n_will_done = n_done + n_cur_chunk;
        merge_sorted(cpy, n_done, src + n_done, n_cur_chunk, dst);
        parallel_separate(copy, map_routine, n_threads, dst, cpy, NULL, 0, n_will_done);
    }
    free(cpy);

    #ifdef DEBUG
        printf("sort_dynamic end\n");
        print_arr(dst, n);
    #endif
}

// callbacks

double sum(double x, void* arg) {
    return  1 / tanh(sqrt(x));
}

double ctanh_sqrt(double x, void * arg) {
    return  1 / tanh(sqrt(x));
}

double pow_log10(double x, void * arg) {
    return  pow(log10(x), M_E);
}

double sum_prev(double x, void * arg) {
    struct arg_src2 * data = arg;
    return data -> src2 + x;
}

double get_max(double x, void * arg) {
    struct arg_src2 * data = arg;
    return max(data -> src2, x);
}

double map_sin(double x, void* arg) {
    struct arg_src2 *data = arg;
    #ifdef DEBUG
        printf("map_sin x: %f min: %f\n", x, data -> src2);
    #endif
    if((int)(x / (data -> src2)) % 2 == 0) return sin(x);
    else return 0;
}

double sum_reduce(double x, void * arg) {
    double * acc = arg;
    return (* acc) + x;
}

struct progress_arg {
    volatile int * progress;
    volatile int * is_finished;
};

void* progress_routine(void * arg) {
    struct progress_arg *data = arg;
    double time = 0;
    while (*(data -> is_finished) < 1) {
        double time_temp = get_time();
        if (time_temp - time < 1) {
            usleep(100);
            continue;
        };
        printf("\nPROGRESS: %d\n", *(data -> progress));
        time = time_temp;
    }
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    struct timeval T1, T2;
    gettimeofday(&T1, NULL);

    const int N = atoi(argv[1]); /* N - array size, equals first cmd param */

    const int N_2 = N / 2;
    const int A = 280;

    double * restrict m1 = malloc(N * sizeof(double));
    double * restrict m2 = malloc(N_2 * sizeof(double));
    double * restrict m2_cpy = malloc(N_2 * sizeof(double));

    const int M = atoi(argv[2]); /* M - amount of threads */

    volatile int i = 0;
    volatile int is_finished = 0;

    pthread_t thread_progress;
    struct progress_arg arg_progress;
    arg_progress.progress = &i;
    arg_progress.is_finished = &is_finished;
    pthread_create(&thread_progress, NULL, progress_routine, &arg_progress);

    for (i = 0; i < 100; i++) /* 100 экспериментов */
    {
        double X = 0;
        unsigned int seedp = i;

        for (int j = 0; j < N; ++j) {
            m1[j] = (rand_r(&seedp) % (A * 100)) / 100.0 + 1;
        }

        for (int j = 0; j < N_2; ++j) {
            m2[j] = A + rand_r(&seedp) % (A * 9);
        }

        print_arr_dbg(m2, N_2);
        parallel_separate(copy, map_routine, M, m2, m2_cpy, NULL, 0, N_2);
        print_arr_dbg(m2_cpy, N_2);
        parallel_separate(ctanh_sqrt, map_routine, M, m1, m1, NULL, 0, N);
        print_arr_dbg(m1, N);

        struct arg_src2 * restrict args_sum = malloc(N_2 * sizeof(struct arg_src2));
        args_sum[0].src2 = 0;
        for (int j = 1; j < N_2; ++j) {
            args_sum[j].src2 = m2_cpy[j - 1];
        }

        parallel_separate(sum_prev, map_routine, M, m2, m2, args_sum, sizeof(struct arg_src2), N_2);
        print_arr_dbg(m2, N_2);
        parallel_separate(pow_log10, map_routine, M, m2, m2, NULL, 0, N_2);
        print_arr_dbg(m2, N_2);

        struct arg_src2 * args_max = malloc(N_2 * sizeof(struct arg_src2));
        for (int j = 0; j < N_2; ++j) {
            args_sum[j].src2 = m1[j];
        }
        parallel_separate(get_max, map_routine, M, m2, m2_cpy, args_max, sizeof(struct arg_src2), N_2);
        print_arr_dbg(m2_cpy, N_2);

        sort_dynamic(m2_cpy, N_2, m2, M);
        print_arr_dbg(m2, N_2);


        int k = 0;
        while (m2[k] == 0 && k < N_2 - 1) k++;
        double m2_min = m2[k];

        #ifdef DEBUG
            print_arr(m2, N_2);
            printf("min %f\n", m2_min);
        #endif

        // reduce
        struct arg_src2 * args_sin_min = malloc(N_2 * sizeof(struct arg_src2));
        for (int j = 0; j < N_2; ++j) {
            args_sin_min[j].src2 = m2_min;
        }
        parallel_separate(map_sin, map_routine, M, m2, m2_cpy, args_sin_min, sizeof(struct arg_src2), N_2);
        print_arr_dbg(m2_cpy, N_2);

        double * restrict reduce_sum = malloc(M * sizeof(double));
        for (int j = 0; j < M; ++j) {
            reduce_sum[j] = 0;
        }
        parallel_separate(sum_reduce, reduce_routine, M, m2_cpy, reduce_sum, NULL, 0, N_2);
        for (int j = 0; j < M; ++j) {
            X += reduce_sum[j];
        }

        printf("%f ", X);
    }

    is_finished = 1;
    pthread_join(thread_progress, NULL);

    gettimeofday(&T2, NULL);
    print_delta(T1, T2);
    return 0;
}
