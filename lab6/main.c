#define CL_TARGET_OPENCL_VERSION 120
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#pragma warning (disable : 4996)
#include <stdio.h>


#define NWITEMS 512

void read_source(char *buffer) {
}

// void read_source(char *buffer) {
//   FILE *fp;
//   long lSize;

//   fp = fopen("compute.cl", "rb");
//   if (!fp) perror("blah.txt"), exit(1);

//   fseek( fp , 0L , SEEK_END);
//   lSize = ftell(fp);
//   rewind(fp);

//   /* allocate memory for entire content */
//   buffer = calloc(1, lSize + 1);
//   if( !buffer ) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

//   /* copy the file into the buffer */
//   if( 1 != fread(buffer, lSize, 1, fp) ) {
//     fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);
//   }
//   fclose(fp);
// }


int main(int argc, char ** argv) {
  /* Read source to char buffer */
  FILE *fp;
  long lSize;

  fp = fopen("compute.cl", "rb");
  if (!fp) perror("blah.txt"), exit(1);

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
  cl_mem bufSA, bufSB, bufC, bufS;
  cl_event event = NULL;
  int ret = 0;

  /* Setup OpenCL environment. */
  err = clGetPlatformIDs(1, &platform, NULL);
  if (err != CL_SUCCESS) {
      printf( "clGetPlatformIDs() failed with %d\n", err );
      return 1;
  }

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err != CL_SUCCESS) {
      printf( "clGetDeviceIDs() failed with %d\n", err );
      return 1;
  }

  props[1] = (cl_context_properties)platform;
  ctx = clCreateContext(props, 1, &device, NULL, NULL, &err);
  if (err != CL_SUCCESS) {
      printf( "clCreateContext() failed with %d\n", err );
      return 1;
  }

  queue = clCreateCommandQueue(ctx, device, 0, &err);
  if (err != CL_SUCCESS) {
      printf( "clCreateCommandQueue() failed with %d\n", err );
      clReleaseContext(ctx);
      return 1;
  }

  // 4. Perform runtime source compilation, and obtain kernel entry point.
  cl_program program = clCreateProgramWithSource(ctx, 1, &source, NULL, &err );

  clBuildProgram( program, 1, &device, NULL, NULL, NULL );
  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(
      program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len
    );
    printf("%s\n", buffer);
  }

  cl_kernel kernel = clCreateKernel( program, "memset", NULL );

  // 5. Create a data buffer.
  cl_mem buffer = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, NWITEMS * sizeof(cl_uint), NULL, NULL );

  // 6. Launch the kernel. Let OpenCL pick the local work size.
  size_t global_work_size = NWITEMS;
  clSetKernelArg(kernel, 0, sizeof(buffer), (void*) &buffer);

  clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);

  clFinish( queue );

  // 7. Look at the results via synchronous buffer map.
  cl_uint *ptr;
  ptr = (cl_uint *) clEnqueueMapBuffer(queue, buffer, CL_TRUE, CL_MAP_READ, 0, NWITEMS * sizeof(cl_uint), 0, NULL, NULL, NULL );

  int i;

  for(i=0; i < NWITEMS; i++)
      printf("%d %d\n", i, ptr[i]);

  return 0;
}
