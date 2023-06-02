#define CL_TARGET_OPENCL_VERSION 120
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <stdio.h>

#define NWITEMS 512
// A simple memset kernel
const char *source =
"kernel void memset(   global uint *dst )             \n"
"{                                                    \n"
"    dst[get_global_id(0)] = get_global_id(0);        \n"
"}                                                    \n";

int main(int argc, char ** argv)
{
  cl_int err;
  cl_platform_id platform = 0;
  cl_device_id device = 0;
  cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
  cl_context ctx = 0;
  cl_command_queue queue = 0;
  cl_mem bufSA, bufSB, bufC, bufS;
  cl_event event = NULL;
  int ret = 0;
  // 1. Get a platform.

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

  printf("cl_device_id %d\n", device);

  props[1] = (cl_context_properties)platform;
  for (int i = 0; i < 3; ++i) {
    printf("properties[%d] = %ld", i, props[i]);
  }
  ctx = clCreateContext(props, 1, &device, NULL, NULL, &err);
  if (err != CL_SUCCESS) {
      printf( "clCreateContext() failed with %d\n", err );
      return 1;
  }
  printf("context %d\n", ctx);

  queue = clCreateCommandQueue(ctx, device, 0, &err);
  if (err != CL_SUCCESS) {
      printf( "clCreateCommandQueue() failed with %d\n", err );
      clReleaseContext(ctx);
      return 1;
  }
  printf("cl_command_queue %d\n", queue);
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
  cl_mem buffer = clCreateBuffer( ctx,
                                  CL_MEM_WRITE_ONLY,
                                  NWITEMS * sizeof(cl_uint),
                                  NULL, NULL );

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
