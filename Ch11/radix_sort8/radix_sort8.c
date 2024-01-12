#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PROGRAM_FILE "radix_sort8.cl"
#define KERNEL_FUNC "radix_sort8"
#define NUM_SHORTS 8

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

  cl_program program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program");
  free(program_buffer);

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

  /* Initialize data */
  unsigned short data[NUM_SHORTS];
  srand(time(NULL));
  for (int i = 0; i < NUM_SHORTS; i++) {
    data[i] = (unsigned short)i;
  }
  for (int i = 0; i < NUM_SHORTS - 1; i++) {
    int j = i + (rand() % (NUM_SHORTS - i));
    int temp = data[i];
    data[i] = data[j];
    data[j] = temp;
  }

  /* Print input */
  printf("Input: \n");
  for (int i = 0; i < NUM_SHORTS; i++) {
    printf("data[%d]: %hu\n", i, data[i]);
  }

  /* Identify a platform */
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't identify a platform");

  /* Access a device */
  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  handleError("Couldn't access any devices");

  /* Create a context */
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Read program file and place content into buffer */
  FILE *program_handle = fopen(PROGRAM_FILE, "r");
  if (program_handle == NULL) {
    perror("Couldn't find the program file");
    exit(EXIT_FAILURE);
  }
  fseek(program_handle, 0, SEEK_END);
  size_t program_size = ftell(program_handle);
  rewind(program_handle);
  char *program_buffer = (char *)calloc(program_size + 1, sizeof(char));
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  /* Create program from file */
  cl_program program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program");
  free(program_buffer);

  /* Build program */
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err) {
    /* Find size of log and print to std output */
    size_t log_size;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *program_log = (char *)calloc(log_size + 1, sizeof(char));
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(1);
  }

  /* Create a kernel */
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError("Couldn't create a kernel");

  /* Create buffer to hold sorted data */
  cl_mem data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data), data, &err);
  handleError("Couldn't create a buffer");

  /* Create kernel argument */
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
  handleError("Couldn't set a kernel argument");

  /* Create a command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  handleError("Couldn't create a command queue");

  /* Enqueue kernel */
  const size_t global_work_size[] = {1};
  const size_t local_work_size[] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
  handleError("Couldn't enqueue the kernel");

  /* Read and print the result */
  err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0, sizeof(data), &data, 0, NULL, NULL);
  handleError("Couldn't read the buffer");

  /* Print output */
  printf("\nOutput:\n");
  for (int i = 0; i < NUM_SHORTS; i++) {
    printf("data[%d]: %hu\n", i, data[i]);
  }

  /* Check the output and display test result */
  cl_int check = CL_TRUE;
  for (int i = 0; i < NUM_SHORTS; i++) {
    if (data[i] != i) {
      check = CL_FALSE;
      break;
    }
  }

  if (check)
    printf("The radix sort SUCCEEDED.\n");
  else
    printf("The radix sort FAILED.\n");

  clReleaseMemObject(data_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
