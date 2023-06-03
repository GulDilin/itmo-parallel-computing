#define CL_TARGET_OPENCL_VERSION 120
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#pragma warning (disable : 4996)
#include <stdio.h>
#include <stdlib.h>


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
  }
}

void print_err(cl_context * ctx, cl_int * err, const char * f_name) {
  if (*err != CL_SUCCESS) {
      printf( "%s failed with %d\n", f_name, *err );
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
  int n
) {
  printf("print_buffer\n");
  double * dst_host = malloc(n * sizeof(double));
  clEnqueueReadBuffer(*queue, *dst, CL_TRUE, 0, n * sizeof(cl_double), dst_host, 0, NULL, NULL);
  for(int i = 0; i < n; i++) {
    printf("%f ", dst_host[i]);
  }
  printf("\n");
  free(dst_host);
}

void ctanh_sqrt(
  cl_context *ctx,
  cl_program *program,
  cl_command_queue *queue,
  cl_mem *src,
  cl_mem *dst,
  int n
) {
  cl_int err = CL_SUCCESS;
  cl_kernel kernel = clCreateKernel( *program, "ctanh_sqrt", &err );
  print_err(ctx, &err, "[ctanh_sqrt] clCreateKernel()");
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), src);
  print_err(ctx, &err, "[ctanh_sqrt] clSetKernelArg(src)");
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), dst);
  print_err(ctx, &err, "[ctanh_sqrt] clSetKernelArg(dst)");
  size_t global_work_size = n;
  err = clEnqueueNDRangeKernel(*queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
  print_err(ctx, &err, "[ctanh_sqrt] clEnqueueNDRangeKernel()");
}

void sum_prev(
  cl_program *program,
  cl_command_queue *queue,
  cl_mem *src1,
  cl_mem *src2,
  cl_mem *dst,
  int n
) {
  cl_kernel kernel = clCreateKernel( *program, "sum_prev", NULL );
  cl_int err = clSetKernelArg(kernel, 0, sizeof(cl_mem), src1);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), src2);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), dst);
  size_t global_work_size = n;
  clEnqueueNDRangeKernel(*queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
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
  print_err(&ctx, &err, "clGetPlatformIDs()");

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  print_err(&ctx, &err, "clGetDeviceIDs()");

  props[1] = (cl_context_properties)platform;
  ctx = clCreateContext(props, 1, &device, NULL, NULL, &err);
  print_err(&ctx, &err, "clCreateContext()");

  queue = clCreateCommandQueue(ctx, device, 0, &err);
  print_err(&ctx, &err, "clCreateCommandQueue()");

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
  generate( m1_host, m2_host, N, N_2, 1 );
  printf("M1 HOST\n");
  print_arr(m1_host, N);
  printf("M2 HOST\n");
  print_arr(m2_host, N_2);

  printf("START OCL\n");

  // 5. Create a data buffer.
  cl_mem m1 = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, N * sizeof(cl_double), m1_host, &err );
  print_err(&ctx, &err, "M1 clCreateBuffer()");
  cl_mem m2 = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, N_2 * sizeof(cl_double), m2_host, &err );
  print_err(&ctx, &err, "M2 clCreateBuffer()");
  cl_mem m2_cpy = clCreateBuffer(ctx, CL_MEM_READ_WRITE , N_2 * sizeof(cl_double), NULL, &err );
  print_err(&ctx, &err, "M2_CPY clCreateBuffer()");
  cl_mem dst = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, N * sizeof(cl_double), NULL, &err );
  print_err(&ctx, &err, "DST clCreateBuffer()");
  err = clEnqueueWriteBuffer(queue, m2_cpy, CL_TRUE, 0, N_2 * sizeof(cl_double), m2_host, 0, NULL, NULL);
  print_err(&ctx, &err, "m2_cpy clEnqueueWriteBuffer()");

  // map
  ctanh_sqrt(&ctx ,&program, &queue, &m1, &dst, N);

  clFinish( queue );
  print_buffer(&program, &queue, &dst, N);

  return 0;
}
