#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define PROGRAM_FILE "blank.cl"
#define KERNEL_NAME "blank"

cl_int err;

void handleError(char *message) {
  if (err) {
    perror(message);
    exit(EXIT_FAILURE);
  }
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

int main(void) {

  /* Host/device data structures */
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;

  /* Program/kernel data structures */
  cl_program program;
  FILE *program_handle;
  char *program_buffer, *program_log;
  size_t program_size, log_size;
  cl_kernel kernel;

  /* Access the first installed platform */
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't find any platforms");

  /* Access the first GPU/CPU */
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  handleError("Couldn't find any devices");

  /* Create a context */
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Read program file and place content into buffer */
  program_handle = fopen(PROGRAM_FILE, "r");
  err = !program_handle;
  handleError("Couldn't find the program file");
  fseek(program_handle, 0, SEEK_END);
  program_size = ftell(program_handle);
  rewind(program_handle);
  program_buffer = (char *)malloc(program_size + 1);
  program_buffer[program_size] = '\0';
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  /* Create program from file */
  program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program");
  free(program_buffer);

  /* Build program */
  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  if (err) {
    printProgramLog(program, device);
  }

  /* Create the kernel */
  kernel = clCreateKernel(program, KERNEL_NAME, &err);
  handleError("Couldn't create the kernel");

  /* Create the command queue */
  queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  handleError("Couldn't create the command queue");

  /* Enqueue the kernel execution command */
  const size_t global_work_size[1] = {1};
  const size_t local_work_size[1] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
  handleError("Couldn't enqueue the kernel execution command");
  printf("Successfully queued kernel.\n");

  /* Deallocate resources */
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(context);
}
