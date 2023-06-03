#define CL_TARGET_OPENCL_VERSION 120
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#pragma warning (disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <sys/timeb.h>

// #define DEBUG 1
#define SOURCE_NAME "compute.cl"


void print_err_code(cl_int * err) {
  switch (*err) {
    case CL_INVALID_PROGRAM:
      printf("CL_INVALID_PROGRAM\n");
      break;
    case CL_INVALID_PROGRAM_EXECUTABLE:
      printf("CL_INVALID_PROGRAM_EXECUTABLE\n");
      break;
    case CL_INVALID_KERNEL_NAME:
      printf("CL_INVALID_KERNEL_NAME\n");
      break;
    case CL_INVALID_KERNEL_DEFINITION:
      printf("CL_INVALID_KERNEL_DEFINITION\n");
      break;
    case CL_INVALID_VALUE:
      printf("CL_INVALID_VALUE\n");
      break;
    case CL_OUT_OF_HOST_MEMORY:
      printf("CL_OUT_OF_HOST_MEMORY\n");
      break;
    case CL_INVALID_ARG_INDEX:
      printf("CL_INVALID_ARG_INDEX\n");
      break;
    case CL_INVALID_ARG_VALUE:
      printf("CL_INVALID_ARG_VALUE\n");
      break;
    case CL_INVALID_MEM_OBJECT:
      printf("CL_INVALID_MEM_OBJECT\n");
      break;
    case CL_INVALID_SAMPLER:
      printf("CL_INVALID_SAMPLER\n");
      break;
    case CL_INVALID_ARG_SIZE:
      printf("CL_INVALID_ARG_SIZE\n");
      break;
    case CL_INVALID_COMMAND_QUEUE:
      printf("CL_INVALID_COMMAND_QUEUE\n");
      break;
    case CL_INVALID_CONTEXT:
      printf("CL_INVALID_CONTEXT\n");
      break;
    case CL_INVALID_KERNEL_ARGS:
      printf("CL_INVALID_KERNEL_ARGS\n");
      break;
  }
}

void print_err(
  cl_context * ctx,
  cl_int * err,
  const char * f_name,
  const char * subpart
) {
  if (*err != CL_SUCCESS) {
    if (subpart) printf( "[%s] %s failed with %d\n", subpart, f_name, *err );
    else printf( "%s failed with %d\n", f_name, *err );
    print_err_code(err);
    if (*ctx) {
      clReleaseContext(*ctx);
    }
    exit(1);
  }
}


void print_arr(double *array, int n) {
  #ifdef DEBUG
    for (int i = 0; i < n; ++i) {
        printf("%f ", array[i]);
    }
    printf("\n");
  #endif
}


void print_buffer(
  cl_program *program,
  cl_command_queue *queue,
  cl_mem *dst,
  int n,
  const char * buffer_name
) {
  #ifdef DEBUG
    double * dst_host = malloc(n * sizeof(double));
    clEnqueueReadBuffer(*queue, *dst, CL_TRUE, 0, n * sizeof(cl_double), dst_host, 0, NULL, NULL);
    if (buffer_name) printf("%s ", buffer_name);
    for (int i = 0; i < n; i++) printf("%f ", dst_host[i]);
    printf("\n");
    free(dst_host);
  #endif
}


double get_time() {
  struct timeb result;
  ftime(&result);
  return 1000.0 * result.time + result.millitm;
}

// --------------- PRINTS END

void run_kernel(
  const char * kernel_name,
  cl_kernel kernel,
  cl_context *ctx,
  cl_program *program,
  cl_command_queue *queue,
  int n,
  int n_args,
  ...
) {
  cl_int err = CL_SUCCESS;
  va_list valist;
  va_start(valist, n_args);
  err = CL_SUCCESS;
  for (int i = 0; i < n_args; ++i) {
    size_t arg_size = va_arg(valist, size_t);
    void * arg = va_arg(valist, void *);
    err |= clSetKernelArg(kernel, i, arg_size, arg);
  }
  va_end(valist);
  print_err(ctx, &err, "clSetKernelArg()", kernel_name);

  size_t global_work_size = n;
  err = clEnqueueNDRangeKernel(*queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
  print_err(ctx, &err, "clEnqueueNDRangeKernel()", kernel_name);
}


// --------------- SORT & REDUCE

void init_chunked_args(
  cl_context *ctx,
  cl_mem *src_offset,
  cl_mem *src_size,
  int * src_offset_host,
  int * src_size_host,
  int sort_parts,
  int n
) {
  int n_chunk = sort_parts < 2 ? n : ceil((double) n / sort_parts);
  int n_done = 0;
  for (int i = 0; i < sort_parts; ++i) {
    int n_cur_chunk = max(min((n - n_done), n_chunk), 0);
    src_offset_host[i] = n_done;
    src_size_host[i] = n_cur_chunk;
    n_done += n_cur_chunk;
  }

  cl_int err = CL_SUCCESS;
  *src_offset = clCreateBuffer(*ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sort_parts * sizeof(cl_int), src_offset_host, &err );
  print_err(ctx, &err, "sort src_offset clCreateBuffer()", NULL);
  *src_size = clCreateBuffer(*ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sort_parts * sizeof(cl_int), src_size_host, &err );
  print_err(ctx, &err, "sort src_size clCreateBuffer()", NULL);
}


void merge_sorted(
  cl_context *ctx,
  cl_program *program,
  cl_command_queue *queue,
  cl_kernel merge_sorted_kernel,
  cl_mem *src,
  cl_mem *temp,
  int * src_offset_host,
  int * src_size_host,
  int sort_parts,
  int n
) {
  cl_int err = CL_SUCCESS;
  for (int i = 1; i < sort_parts; ++i) {
    cl_int offset_1 = 0, offset_2 = src_offset_host[i], offset_dst = 0;
    cl_int n_src_1 = src_offset_host[i], n_src_2 = src_size_host[i];

    int n_will_done = src_offset_host[i] + src_size_host[i];
    run_kernel(
      "merge_sorted", merge_sorted_kernel, ctx ,program, queue, 1, 7,
      sizeof(cl_mem *), src, sizeof(cl_mem *), temp,
      sizeof(cl_int), &offset_1, sizeof(cl_int), &offset_2, sizeof(cl_int), &offset_dst,
      sizeof(cl_int), &n_src_1, sizeof(cl_int), &n_src_2
    );
    err = clEnqueueCopyBuffer(*queue, *temp, *src, 0, 0, n_will_done * sizeof(cl_double), 0, NULL, NULL);
    print_err(ctx, &err, "sort temp -> src clEnqueueCopyBuffer()", NULL);
  }
}


void sort(
  cl_context *ctx,
  cl_program *program,
  cl_command_queue *queue,
  cl_kernel sort_kernel,
  cl_kernel merge_sorted_kernel,
  int n_parts,
  int n,
  cl_mem *src,
  cl_mem *temp
) {
  cl_int err = CL_SUCCESS;
  int * src_offset_host = malloc(n * sizeof(int));
  int * src_size_host = malloc(n * sizeof(int));
  cl_mem src_offset, src_size;

  init_chunked_args(ctx, &src_offset, &src_size, src_offset_host, src_size_host, n_parts, n);

  run_kernel(
    "sort", sort_kernel, ctx ,program, queue, n_parts, 3,
    sizeof(cl_mem *), &src_offset, sizeof(cl_mem *), &src_size, sizeof(cl_mem *), src
  );

  err = clEnqueueCopyBuffer(*queue, *src, *temp, 0, 0, n * sizeof(cl_double), 0, NULL, NULL);
  print_err(ctx, &err, "sort src -> temp clEnqueueCopyBuffer()", NULL);

  merge_sorted(
    ctx, program, queue, merge_sorted_kernel,
    src, temp, src_offset_host, src_size_host, n_parts, n
  );

  free(src_offset_host);
  free(src_size_host);
}


void reduce_sum(
  cl_context *ctx,
  cl_program *program,
  cl_command_queue *queue,
  cl_kernel reduce_sum_kernel,
  int n_parts,
  int n,
  cl_mem *src,
  double *result
) {
  cl_int err = CL_SUCCESS;
  int * src_offset_host = malloc(n * sizeof(int));
  int * src_size_host = malloc(n * sizeof(int));
  cl_mem src_offset, src_size;

  init_chunked_args(ctx, &src_offset, &src_size, src_offset_host, src_size_host, n_parts, n);
  cl_mem dst = clCreateBuffer(*ctx, CL_MEM_READ_WRITE, n * sizeof(cl_double), NULL, &err);
  print_err(ctx, &err, "reduce_sum dst clCreateBuffer()", NULL);

  run_kernel(
    "reduce_sum", reduce_sum_kernel, ctx ,program, queue, n_parts, 4,
    sizeof(cl_mem *), &src_offset, sizeof(cl_mem *), &src_size, sizeof(cl_mem *), src, sizeof(cl_mem *), &dst
  );

  double * dst_host = malloc(n * sizeof(double));
  clEnqueueReadBuffer(*queue, dst, CL_TRUE, 0, n * sizeof(cl_double), dst_host, 0, NULL, NULL);
  *result = 0;
  for (int i = 0; i < n_parts; ++i) {
    *result += dst_host[i];
  }

  free(dst_host);
  free(src_offset_host);
  free(src_size_host);
}

// ---------------

void generate(
  double * restrict m1_host,
  double * restrict m2_host,
  int n1,
  int n2,
  int i
) {
  const int A = 280;
  srand(i);

  for (int j = 0; j < n1; ++j) {
    m1_host[j] = (rand() % (A * 100)) / 100.0 + 1;
  }
  for (int j = 0; j < n2; ++j) {
    m2_host[j] = A + rand() % (A * 9);
  }
}

// --------------- INIT METHODS

void init_opencl_env(
  cl_context * ctx,
  cl_command_queue * queue,
  cl_program * program,
  const char ** source
) {
  cl_int err;
  cl_platform_id platform = 0;
  cl_device_id device = 0;
  cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };

  err = clGetPlatformIDs(1, &platform, NULL);
  print_err(ctx, &err, "clGetPlatformIDs()", NULL);

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  print_err(ctx, &err, "clGetDeviceIDs()", NULL);

  props[1] = (cl_context_properties)platform;
  *ctx = clCreateContext(props, 1, &device, NULL, NULL, &err);
  print_err(ctx, &err, "clCreateContext()", NULL);

  *queue = clCreateCommandQueue(*ctx, device, 0, &err);
  print_err(ctx, &err, "clCreateCommandQueue()", NULL);

  // Perform runtime source compilation, and obtain kernel entry point.
  *program = clCreateProgramWithSource(*ctx, 1, source, NULL, &err );
  clBuildProgram( *program, 1, &device, NULL, NULL, NULL );
  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(
      *program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len
    );
    printf("clBuildProgram() failed with %s\n", buffer);
  }
}


void init_buffers(
  cl_context * ctx,
  double * m1_host,
  double * m2_host,
  cl_mem * m1,
  cl_mem * m2,
  cl_mem * m2_cpy,
  int n1,
  int n2
) {
  cl_int err = CL_SUCCESS;
  *m1 = clCreateBuffer(*ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, n1 * sizeof(cl_double), m1_host, &err );
  print_err(ctx, &err, "M1 clCreateBuffer()", NULL);
  *m2 = clCreateBuffer(*ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, n2 * sizeof(cl_double), m2_host, &err );
  print_err(ctx, &err, "M2 clCreateBuffer()", NULL);
  *m2_cpy = clCreateBuffer(*ctx, CL_MEM_READ_WRITE , n2 * sizeof(cl_double), NULL, &err );
  print_err(ctx, &err, "M2_CPY clCreateBuffer()", NULL);
}


void init_kernels(
  cl_context * ctx,
  cl_program * program,
  cl_kernel * ctanh_sqrt,
  cl_kernel * sum_prev,
  cl_kernel * pow_log10,
  cl_kernel * max_2_src,
  cl_kernel * map_sin,
  cl_kernel * sort_kernel,
  cl_kernel * merge_sort_kernel,
  cl_kernel * reduce_sum_kernel
) {
  cl_int err = CL_SUCCESS;
  *ctanh_sqrt = clCreateKernel(*program, "ctanh_sqrt", &err);
  print_err(ctx, &err, "[ctanh_sqrt] clCreateKernel", NULL);
  *sum_prev = clCreateKernel(*program, "sum_prev", &err);
  print_err(ctx, &err, "[sum_prev] clCreateKernel", NULL);
  *pow_log10 = clCreateKernel(*program, "pow_log10", &err);
  print_err(ctx, &err, "[pow_log10] clCreateKernel", NULL);
  *max_2_src = clCreateKernel(*program, "max_2_src", &err);
  print_err(ctx, &err, "[max_2_src] clCreateKernel", NULL);
  *map_sin = clCreateKernel(*program, "map_sin", &err);
  print_err(ctx, &err, "[map_sin] clCreateKernel", NULL);

  *sort_kernel = clCreateKernel(*program, "sort", &err);
  print_err(ctx, &err, "[sort] clCreateKernel", NULL);
  *merge_sort_kernel = clCreateKernel(*program, "merge_sorted", &err);
  print_err(ctx, &err, "[merge_sorted] clCreateKernel", NULL);
  *reduce_sum_kernel = clCreateKernel(*program, "reduce_sum", &err);
  print_err(ctx, &err, "[reduce_sum] clCreateKernel", NULL);
}

// ---------------

int main(int argc, char ** argv) {
  double time_start = get_time();

  /* Read source to char buffer */
  FILE *fp;
  long lSize;

  fp = fopen(SOURCE_NAME, "rb");

  fseek( fp , 0L , SEEK_END);
  lSize = ftell(fp);
  rewind(fp);

  /* allocate memory for entire content */
  const char * source = calloc(1, lSize + 1);
  if( !source ) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

  /* copy the file into the source */
  if( 1 != fread((void *)source, lSize, 1, fp) ) {
    fclose(fp), free((void *)source), fputs("entire read fails", stderr), exit(1);
  }
  fclose(fp);

  cl_int err;
  cl_context ctx = 0;
  cl_command_queue queue = 0;
  cl_program program = NULL;

  init_opencl_env(&ctx, &queue, &program, &source);

  const int N = atoi(argv[1]);
  const int N_2 = N / 2;
  const int N_separate =  argc > 2 ? atoi(argv[2]) : 4;

  double * restrict m1_host = malloc(N * sizeof(double));
  double * restrict m2_host = malloc(N_2 * sizeof(double));
  cl_mem m1, m2, m2_cpy;
  init_buffers(&ctx, m1_host, m2_host, &m1, &m2, &m2_cpy, N, N_2);

  cl_kernel ctanh_sqrt, sum_prev, pow_log10, max_2_src, map_sin;
  cl_kernel sort_kernel, merge_sort_kernel, reduce_sum_kernel;

  init_kernels(
    &ctx, &program, &ctanh_sqrt, &sum_prev, &pow_log10, &max_2_src, &map_sin,
    &sort_kernel, &merge_sort_kernel, &reduce_sum_kernel
  );

  for (int i = 0; i < 100; i++) {
    generate(m1_host, m2_host, N, N_2, i);
    print_arr(m1_host, N);
    print_arr(m2_host, N_2);

    err = clEnqueueWriteBuffer(queue, m1, CL_TRUE, 0, N_2 * sizeof(cl_double), m1_host, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, m2, CL_TRUE, 0, N_2 * sizeof(cl_double), m2_host, 0, NULL, NULL);
    err |= clEnqueueCopyBuffer(queue, m2, m2_cpy, 0, 0, N_2 * sizeof(cl_double), 0, NULL, NULL);
    print_err(&ctx, &err, "m1, m2, m2_cpy clEnqueueWriteBuffer, clEnqueueCopyBuffer()", NULL);

    // map
    run_kernel(
      "ctanh_sqrt", ctanh_sqrt, &ctx ,&program, &queue, N, 2,
      sizeof(cl_mem *), &m1, sizeof(cl_mem *), &m1
    );
    run_kernel(
      "sum_prev", sum_prev, &ctx ,&program, &queue, N_2, 3,
      sizeof(cl_mem *), &m2, sizeof(cl_mem *), &m2_cpy, sizeof(cl_mem *), &m2
    );
    run_kernel(
      "pow_log10", pow_log10, &ctx ,&program, &queue, N_2, 2,
      sizeof(cl_mem *), &m2, sizeof(cl_mem *), &m2
    );
    run_kernel(
      "max_2_src", max_2_src, &ctx ,&program, &queue, N_2, 3,
      sizeof(cl_mem *), &m2, sizeof(cl_mem *), &m1, sizeof(cl_mem *), &m2_cpy
    );

    sort(&ctx, &program, &queue, sort_kernel, merge_sort_kernel, N_separate, N_2, &m2_cpy, &m2);

    clEnqueueReadBuffer(queue, m2_cpy, CL_TRUE, 0, N_2 * sizeof(cl_double), m2_host, 0, NULL, NULL);
    int k = 0;
    while (m2_host[k] == 0 && k < N_2 - 1) k++;
    cl_double m2_min = m2_host[k];

    run_kernel(
      "map_sin", map_sin, &ctx ,&program, &queue, N_2, 3,
      sizeof(cl_mem *), &m2_cpy, sizeof(cl_mem *), &m2_cpy, sizeof(cl_double), &m2_min
    );

    double X = 0;
    reduce_sum(&ctx ,&program, &queue, reduce_sum_kernel, N_separate, N_2, &m2_cpy, &X);
    printf("%f ", X);
  }

  clFinish( queue );
  free(m1_host);
  free(m2_host);

  double time_end = get_time();
  printf("\n%f\n", time_end - time_start);
  return 0;
}
