#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define PROGRAM_FILE "blank.cl"
#define KERNEL_FUNC "blank"

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

  /* Initialize arrays */
  float data_one[100], data_two[100], result_array[100];
  for (int i = 0; i < 100; i++) {
    data_one[i] = 1.0f * i;
    data_two[i] = -1.0f * i;
    result_array[i] = 0.0f;
  }

  /* Create a device and context */
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Build the program and create the kernel */
  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError("Couldn't create a kernel");

  /* Create buffers */
  cl_mem buffer_one =
      clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data_one), data_one, &err);
  handleError("Couldn't create a buffer one object");
  cl_mem buffer_two =
      clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data_two), data_two, &err);
  handleError("Couldn't create a buffer two object");

  /* Set buffers as arguments to the kernel */
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_one);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_two);
  handleError("Couldn't set the buffers as kernel argument");

  /* Create a command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
  handleError("Couldn't create a command queue");

  /* Enqueue kernel */
  err = clEnqueueTask(queue, kernel, 0, NULL, NULL);
  handleError("Couldn't enqueue the kernel");

  /* Enqueue command to copy buffer one to buffer two */
  err = clEnqueueCopyBuffer(queue, buffer_one, buffer_two, 0, 0, sizeof(data_one), 0, NULL, NULL);
  handleError("Couldn't perform the buffer copy");

  /* Enqueue command to map buffer two to host memory */
  void *mapped_memory =
      clEnqueueMapBuffer(queue, buffer_two, CL_TRUE, CL_MAP_READ, 0, sizeof(data_two), 0, NULL, NULL, &err);
  handleError("Couldn't map the buffer to host memory");

  /* Transfer memory and unmap the buffer */
  memcpy(result_array, mapped_memory, sizeof(data_two));
  err = clEnqueueUnmapMemObject(queue, buffer_two, mapped_memory, 0, NULL, NULL);
  handleError("Couldn't unmap the buffer");

  /* Display updated buffer */
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      printf("%6.1f", result_array[j + i * 10]);
    }
    printf("\n");
  }

  /* Deallocate resources */
  clReleaseMemObject(buffer_one);
  clReleaseMemObject(buffer_two);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
