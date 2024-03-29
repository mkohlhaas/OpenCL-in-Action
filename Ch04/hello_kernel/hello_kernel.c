#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_FILE "hello_kernel.cl"
#define KERNEL_FUNC "hello_kernel"

cl_int err;

void handleError(char *message) {
  if (err) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {

  /* Identify a platform */
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't find any platforms");

  /* Access a device */
  cl_device_id dev;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
  }
  handleError("Couldn't find any devices");

  return dev;
}

void printProgramLog(cl_program program, cl_device_id device) {

  /* Find size of log and print to std output */
  size_t log_size;
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  char *program_log = (char *)malloc(log_size + 1);
  program_log[log_size] = '\0';
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
  printf("%s\n", program_log);
  free(program_log);
  exit(EXIT_FAILURE);
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename) {

  /* Read program file and place content into buffer */
  FILE *program_handle = fopen(filename, "r");
  err = !program_handle;
  handleError("Couldn't find the program file");
  fseek(program_handle, 0, SEEK_END);
  size_t program_size = ftell(program_handle);
  rewind(program_handle);
  char *program_buffer = (char *)malloc(program_size + 1);
  program_buffer[program_size] = '\0';
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  /* Create program from file */
  cl_program program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program");
  free(program_buffer);

  /* Build program */
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err) {
    printProgramLog(program, dev);
  }

  return program;
}

int main(void) {

  /* Create a device and context */
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Build a program and create a kernel */
  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError("Couldn't create a kernel");

  /* Create a buffer to hold the output data */
  char msg[16];
  cl_mem msg_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(msg), NULL, &err);
  handleError("Couldn't create a buffer");

  /* Create kernel argument */
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &msg_buffer);
  handleError("Couldn't set a kernel argument");

  /* Create a command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
  handleError("Couldn't create a command queue");

  /* Enqueue kernel */
  const size_t global_work_size[1] = {1};
  const size_t local_work_size[1] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
  handleError("Couldn't enqueue the kernel");

  /* Read and print the result */
  err = clEnqueueReadBuffer(queue, msg_buffer, CL_BLOCKING, 0, sizeof(msg), &msg, 0, NULL, NULL);
  handleError("Couldn't read the output buffer");
  printf("Kernel output: %s\n", msg);

  /* Deallocate resources */
  clReleaseMemObject(msg_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
