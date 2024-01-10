#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_FILE "select_test.cl"
#define KERNEL_FUNC "select_test"

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

cl_device_id create_device() {
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't identify a platform");

  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  handleError("Couldn't access any devices");

  return device;
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename) {

  FILE *program_handle = fopen(filename, "r");
  if (program_handle == NULL) {
    perror("Couldn't find the program file");
    exit(EXIT_FAILURE);
  }
  fseek(program_handle, 0, SEEK_END);
  size_t program_size = ftell(program_handle);
  rewind(program_handle);
  char *program_buffer = (char *)malloc(program_size);
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  /* Create program from file */
  cl_program program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program");
  free(program_buffer);

  /* Build program */
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err) {
    size_t log_size;
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *program_log = (char *)malloc(log_size);
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }

  return program;
}

int main() {
  float select1[4];
  unsigned char select2[2];

  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                   handleError("Couldn't create a context");

  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);                                              handleError("Couldn't create a kernel");

  cl_mem select1_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(select1), NULL, &err);            handleError("Couldn't create a buffer");
  cl_mem select2_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(select2), NULL, &err);            handleError("Couldn't create a buffer");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &select1_buffer);                                           handleError("Couldn't set a kernel argument");
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &select2_buffer);                                           handleError("Couldn't set a kernel argument");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                   handleError("Couldn't create a command queue");

  const size_t global_work_size[1] = {1};
  const size_t local_work_size[1] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);     handleError("Couldn't enqueue the kernel");

  err = clEnqueueReadBuffer(queue, select1_buffer, CL_BLOCKING, 0, sizeof(select1), &select1, 0, NULL, NULL); handleError("Couldn't read the buffer");
  err = clEnqueueReadBuffer(queue, select2_buffer, CL_BLOCKING, 0, sizeof(select2), &select2, 0, NULL, NULL); handleError("Couldn't read the buffer");

  printf("select: ");
  for (int i = 0; i < 3; i++) {
    printf("%.2f, ", select1[i]);
  }
  printf("%.2f\n", select1[3]);

  printf("bitselect: %X, %X\n", select2[0], select2[1]);

  clReleaseMemObject(select1_buffer);
  clReleaseMemObject(select2_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
