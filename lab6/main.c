#define CL_TARGET_OPENCL_VERSION 120
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#pragma warning (disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>


#define NWITEMS 10
#define SOURCE_NAME "compute.cl"

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

void print_arr(double *array, int n) {
    for (int i = 0; i < n; ++i) {
        printf("%f ", array[i]);
    }
    printf("\n");
}

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

void print_buffer(
  cl_program *program,
  cl_command_queue *queue,
  cl_mem *dst,
  int n,
  const char * buffer_name
) {
  printf("print_buffer\n");
  double * dst_host = malloc(n * sizeof(double));
  clEnqueueReadBuffer(*queue, *dst, CL_TRUE, 0, n * sizeof(cl_double), dst_host, 0, NULL, NULL);
  if (buffer_name) printf("%s ", buffer_name);
  for(int i = 0; i < n; i++) printf("%f ", dst_host[i]);
  printf("\n");
  free(dst_host);
}

void run_kernel(
  const char * kernel_name,
  cl_context *ctx,
  cl_program *program,
  cl_command_queue *queue,
  int n,
  int n_args,
  ...
) {
  cl_int err = CL_SUCCESS;
  cl_kernel kernel = clCreateKernel( *program, kernel_name, &err );
  print_err(ctx, &err, "clCreateKernel()", kernel_name);

  va_list valist;
  va_start(valist, n_args);
  err = CL_SUCCESS;
  for (int i = 0; i < n_args; ++i) {
    cl_mem * arg = va_arg(valist, cl_mem *);
    err |= clSetKernelArg(kernel, i, sizeof(cl_mem), arg);
  }
  va_end(valist);
  print_err(ctx, &err, "clSetKernelArg()", kernel_name);
  size_t global_work_size = n;
  err = clEnqueueNDRangeKernel(*queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
  print_err(ctx, &err, "clEnqueueNDRangeKernel()", kernel_name);
}

void merge_sorted(double *src1, int n1, double *src2, int n2, double *dst) {
    int i = 0, i1 = 0, i2 = 0;
    while (i < n1 + n2) {
        dst[i++] = src1[i1] > src2[i2] && i2 < n2 ? src2[i2++] : src1[i1++];
    }
}

void copy_array(double *src, double *dst, int n) {
  for (int i = 0; i < n; ++i) {
    dst[i] = src[i];
  }
}

void sort(
  cl_context *ctx,
  cl_program *program,
  cl_command_queue *queue,
  int n,
  cl_mem *src,
  cl_mem *temp
) {
  const int sort_parts = 4;
  int n_chunk = sort_parts < 2 ? n : ceil((double) n / sort_parts);
  int * src_offset_host = malloc(n * sizeof(int));
  int * src_size_host = malloc(n * sizeof(int));

  int n_done = 0;
  for (int i = 0; i < sort_parts; ++i) {
    int n_cur_chunk = max(min((n - n_done), n_chunk), 0);
    src_offset_host[i] = n_done;
    src_size_host[i] = n_cur_chunk;
    n_done += n_cur_chunk;
  }

  cl_int err = CL_SUCCESS;
  cl_mem src_offset = clCreateBuffer(*ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sort_parts * sizeof(cl_double), src_offset_host, &err );
  print_err(ctx, &err, "sort src_offset clCreateBuffer()", NULL);
  cl_mem src_size = clCreateBuffer(*ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sort_parts * sizeof(cl_double), src_size_host, &err );
  print_err(ctx, &err, "sort src_size clCreateBuffer()", NULL);
  run_kernel("sort", ctx ,program, queue, sort_parts, 3, &src_offset, &src_size, src);

  err = clEnqueueCopyBuffer(*queue, *src, *temp, 0, 0, n * sizeof(cl_double), 0, NULL, NULL);
  print_err(ctx, &err, "sort src -> temp clEnqueueCopyBuffer()", NULL);

  cl_mem offset_and_sizes = clCreateBuffer(*ctx, CL_MEM_READ_WRITE, 5 * sizeof(cl_int), NULL, &err );
  int * offset_and_sizes_host = malloc(5 * sizeof(int));

  for (int i = 1; i < sort_parts; ++i)
  {
      int offset_1 = 0, offset_2 = src_offset_host[i], offset_dst = 0;
      int n_src_1 = src_offset_host[i], n_src_2 = src_size_host[i];
      offset_and_sizes_host[0] = offset_1;
      offset_and_sizes_host[1] = offset_2;
      offset_and_sizes_host[2] = 0;
      offset_and_sizes_host[3] = src_offset_host[i];
      offset_and_sizes_host[4] = src_size_host[i];
      err = clEnqueueWriteBuffer(*queue, offset_and_sizes, CL_TRUE, 0, 5 * sizeof(cl_int), offset_and_sizes_host, 0, NULL, NULL);
      print_err(ctx, &err, "offset_and_sizes_host clEnqueueWriteBuffer()", NULL);

      int n_will_done = src_offset_host[i] + src_size_host[i];
      run_kernel(
        "merge_sorted", ctx ,program, queue, 1, 3,
        src, temp, &offset_and_sizes
      );
      err = clEnqueueCopyBuffer(*queue, *temp, *src, 0, 0, n_will_done * sizeof(cl_double), 0, NULL, NULL);
      print_err(ctx, &err, "sort temp -> src clEnqueueCopyBuffer()", NULL);
  }
}

int main(int argc, char ** argv) {
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
  cl_platform_id platform = 0;
  cl_device_id device = 0;
  cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
  cl_context ctx = 0;
  cl_command_queue queue = 0;
  cl_event event = NULL;
  int ret = 0;

  /* Setup OpenCL environment. */
  err = clGetPlatformIDs(1, &platform, NULL);
  print_err(&ctx, &err, "clGetPlatformIDs()", NULL);

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  print_err(&ctx, &err, "clGetDeviceIDs()", NULL);

  props[1] = (cl_context_properties)platform;
  ctx = clCreateContext(props, 1, &device, NULL, NULL, &err);
  print_err(&ctx, &err, "clCreateContext()", NULL);

  queue = clCreateCommandQueue(ctx, device, 0, &err);
  print_err(&ctx, &err, "clCreateCommandQueue()", NULL);

  // 4. Perform runtime source compilation, and obtain kernel entry point.
  cl_program program = clCreateProgramWithSource(ctx, 1, &source, NULL, &err );
  clBuildProgram( program, 1, &device, NULL, NULL, NULL );
  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(
      program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len
    );
    printf("clBuildProgram() failed with %s\n", buffer);
  }

  const int N = NWITEMS;
  const int N_2 = N / 2;

  double * restrict m1_host = malloc(N * sizeof(double));
  double * restrict m2_host = malloc(N_2 * sizeof(double));
  generate( m1_host, m2_host, N, N_2, 2 );
  printf("M1 HOST\n");
  print_arr(m1_host, N);
  printf("M2 HOST\n");
  print_arr(m2_host, N_2);

  printf("START OCL\n");

  // 5. Create a data buffer.
  cl_mem m1 = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, N * sizeof(cl_double), m1_host, &err );
  print_err(&ctx, &err, "M1 clCreateBuffer()", NULL);
  cl_mem m2 = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, N_2 * sizeof(cl_double), m2_host, &err );
  print_err(&ctx, &err, "M2 clCreateBuffer()", NULL);
  cl_mem m2_cpy = clCreateBuffer(ctx, CL_MEM_READ_WRITE , N_2 * sizeof(cl_double), NULL, &err );
  print_err(&ctx, &err, "M2_CPY clCreateBuffer()", NULL);
  cl_mem dst = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, N * sizeof(cl_double), NULL, &err );
  print_err(&ctx, &err, "DST clCreateBuffer()", NULL);
  err = clEnqueueCopyBuffer(queue, m2, m2_cpy, 0, 0, N_2 * sizeof(cl_double), 0, NULL, NULL);
  // err = clEnqueueWriteBuffer(queue, m2_cpy, CL_TRUE, 0, N_2 * sizeof(cl_double), m2_host, 0, NULL, NULL);
  print_err(&ctx, &err, "m2_cpy clEnqueueWriteBuffer()", NULL);

  // map
  run_kernel("ctanh_sqrt", &ctx ,&program, &queue, N, 2, &m1, &m1);
  print_buffer(&program, &queue, &m1, N, "M1");
  run_kernel("sum_prev", &ctx ,&program, &queue, N_2, 3, &m2, &m2_cpy, &m2);
  // ctanh_sqrt(&ctx ,&program, &queue, &m1, &m1, N);
  // sum_prev(&ctx ,&program, &queue, &m2, &m2_cpy, &m2, N_2);
  print_buffer(&program, &queue, &m2, N_2, "M2");
  run_kernel("pow_log10", &ctx ,&program, &queue, N_2, 2, &m2, &m2);
  print_buffer(&program, &queue, &m2, N_2, "M2");
  run_kernel("max_2_src", &ctx ,&program, &queue, N_2, 3, &m2, &m1, &m2_cpy);
  print_buffer(&program, &queue, &m2_cpy, N_2, "M2_CPY");

  sort(&ctx, &program, &queue, N_2, &m2_cpy, &m2);
  print_buffer(&program, &queue, &m2_cpy, N_2, "M2_CPY");


  clFinish( queue );

  return 0;
}
