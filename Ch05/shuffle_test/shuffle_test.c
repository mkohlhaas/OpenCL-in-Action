#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_FILE "shuffle_test.cl"
#define KERNEL_FUNC "shuffle_test"

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
  if (err) {
    perror("Couldn't identify a platform");
    exit(EXIT_FAILURE);
  }

  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  handleError("Couldn't access any devices");

  return device;
}

cl_program build_program(cl_context context, cl_device_id device, const char *filename) {

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

  cl_program program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);
  if (err) {
    perror("Couldn't create the program");
    exit(EXIT_FAILURE);
  }
  free(program_buffer);

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err) {
    size_t log_size;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *program_log = (char *)malloc(log_size);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }

  return program;
}

int main() {
  // clang-format off

  // input/output
  float shuffle1[8];
  char  shuffle2[16];

  cl_device_id device = create_device();
  cl_context context  = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                     handleError("Couldn't create a context");

  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel  kernel  = clCreateKernel(program, KERNEL_FUNC, &err);                                               handleError("Couldn't create a kernel");

  cl_mem shuffle1_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(shuffle1), NULL, &err);             handleError("Couldn't create a buffer");
  cl_mem shuffle2_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(shuffle2), NULL, &err);             handleError("Couldn't create a buffer");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &shuffle1_buffer);                                             handleError("Couldn't set a kernel argument");
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &shuffle2_buffer);                                             handleError("Couldn't set a kernel argument");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                      handleError("Couldn't create a command queue");

  const size_t global_work_size[1] = {1};
  const size_t local_work_size[1] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);        handleError("Couldn't enqueue the kernel");

  err = clEnqueueReadBuffer(queue, shuffle1_buffer, CL_BLOCKING, 0, sizeof(shuffle1), &shuffle1, 0, NULL, NULL); handleError("Couldn't read the buffer");
  err = clEnqueueReadBuffer(queue, shuffle2_buffer, CL_BLOCKING, 0, sizeof(shuffle2), &shuffle2, 0, NULL, NULL); handleError("Couldn't read the buffer");
  // clang-format on

  printf("Shuffle1: ");
  for (int i = 0; i < 7; i++) {
    printf("%.2f, ", shuffle1[i]);
  }
  printf("%.2f\n", shuffle1[7]);

  printf("Shuffle2: ");
  for (int i = 0; i < 16; i++) {
    printf("%c", shuffle2[i]);
  }
  printf("\n");

  clReleaseMemObject(shuffle1_buffer);
  clReleaseMemObject(shuffle2_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
