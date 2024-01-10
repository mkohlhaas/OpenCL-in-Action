#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_FILE "id_check.cl"
#define KERNEL_FUNC "id_check"

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
  char *program_buffer = (char *)malloc(program_size + 1);
  program_buffer[program_size] = '\0';
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  cl_program program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program");
  free(program_buffer);

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err) {
    size_t log_size;
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *program_log = (char *)malloc(log_size);
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }

  return program;
}

int main() {
  // clang-format off
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                      handleError("Couldn't create a context");

  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);                                                 handleError("Couldn't create a kernel");

  float test[24];
  cl_mem test_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(test), NULL, &err);                     handleError("Couldn't create a buffer");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &test_buffer);                                                 handleError("Couldn't set a kernel argument");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                      handleError("Couldn't create a command queue");

  size_t dim = 2;
  // size_t global_offset[] = {3, 5};
  size_t global_offset[] = {0, 0};
  size_t global_size[] = {6, 4};
  size_t local_size[] = {3, 2};
  err = clEnqueueNDRangeKernel(queue, kernel, dim, global_offset, global_size, local_size, 0, NULL, NULL);       handleError("Couldn't enqueue the kernel");

  err = clEnqueueReadBuffer(queue, test_buffer, CL_BLOCKING, 0, sizeof(test), &test, 0, NULL, NULL);             handleError("Couldn't read the buffer");
  // clang-format on

  for (int i = 0; i < 24; i += 6) {
    printf("%05.2f  %.2f  %.2f  %.2f  %.2f  %.2f\n", test[i], test[i + 1], test[i + 2], test[i + 3], test[i + 4], test[i + 5]);
  }

  clReleaseMemObject(test_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
