#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_FILE "double_test.cl"
#define KERNEL_FUNC "double_test"

cl_int err;

void handleError(char *message) {
  if (err) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

/* Find a GPU or CPU associated with the first available platform */
cl_device_id getDevice() {

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
cl_program buildProgram(cl_context ctx, cl_device_id dev, const char *filename, char *options) {

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
  err = clBuildProgram(program, 0, NULL, options, NULL, NULL);
  if (err) {
    printProgramLog(program, dev);
  }

  return program;
}

int main(void) {

  /* Create a device and context */
  cl_device_id device = getDevice();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Obtain the device data */
  cl_uint addr_data;
  err = clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(addr_data), &addr_data, NULL);
  handleError("Couldn't read extension data");
  printf("Address width: %u\n", addr_data);

  /* Define "FP_64" option if doubles are supported */
  char *ext_data;
  size_t ext_size;
  clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(ext_data), NULL, &ext_size);
  ext_data = (char *)malloc(ext_size + 1);
  ext_data[ext_size] = '\0';
  clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, ext_size + 1, ext_data, NULL);
  char fp64_ext[] = "cl_khr_fp64";
  char options[20] = "";
  if (strstr(ext_data, fp64_ext)) {
    printf("The %s extension is supported.\n", fp64_ext);
    strcat(options, "-DFP_64 ");
  } else
    printf("The %s extension is not supported.\n", fp64_ext);
  free(ext_data);

  /* Build the program and create the kernel */
  cl_program program = buildProgram(context, device, PROGRAM_FILE, options);
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError("Couldn't create a kernel");

  /* Create CL buffers to hold input and output data */
  float a = 6.0;
  float b = 2.0;
  cl_mem a_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float), &a, &err);
  handleError("Couldn't create a memory object");
  cl_mem b_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float), &b, &err);
  handleError("Couldn't create a memory object");
  cl_mem output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float), NULL, &err);
  handleError("Couldn't create a memory object");

  /* Create kernel arguments */
  // clang-format off
  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &a_buffer);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &b_buffer);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_buffer);
  handleError("Couldnt create a memory object");
  // clang-format off

  /* Create a command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  handleError("Couldn't create a command queue");

  /* Enqueue kernel */
  err = clEnqueueTask(queue, kernel, 0, NULL, NULL);
  handleError("Couldn't enqueue the kernel");

  /* Read and print the result */
  float result;
  err = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, sizeof(float), &result, 0, NULL, NULL);
  handleError("Couldn't read the output buffer");
  printf("The kernel result is %f\n", result);

  /* Deallocate resources */
  clReleaseMemObject(a_buffer);
  clReleaseMemObject(b_buffer);
  clReleaseMemObject(output_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
