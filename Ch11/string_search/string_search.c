#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_FILE "string_search.cl"
#define KERNEL_FUNC "string_search"
#define TEXT_FILE "kafka.txt"

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

int main() {

  /* Identify a platform */
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't identify a platform.");

  /* Access a device */
  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  handleError("Couldn't access any devices.");

  /* Determine global size and local size */
  size_t global_size;
  size_t local_size;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(global_size), &global_size, NULL);
  handleError("Couldn't get device info.");
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL);
  handleError("Couldn't get device info.");
  global_size *= local_size;

  /* Create a context */
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context.");

  /* Read program file and place content into buffer */
  FILE *program_handle = fopen(PROGRAM_FILE, "r");
  if (!program_handle) {
    fprintf(stderr, "Couldn't find the program file.");
    exit(EXIT_FAILURE);
  }
  fseek(program_handle, 0, SEEK_END);
  size_t program_size = ftell(program_handle);
  rewind(program_handle);
  char *program_buffer = (char *)calloc(program_size, sizeof(char));
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  /* Read text file and place content into buffer */
  FILE *text_handle = fopen(TEXT_FILE, "r");
  if (!text_handle) {
    fprintf(stderr, "Couldn't find the text file.");
    exit(EXIT_FAILURE);
  }
  fseek(text_handle, 0, SEEK_END);
  size_t text_size = ftell(text_handle) - 1;
  rewind(text_handle);
  char *text = (char *)calloc(text_size, sizeof(char));
  fread(text, sizeof(char), text_size, text_handle);
  fclose(text_handle);
  int chars_per_item = text_size / global_size + 1;

  /* Create program from file */
  cl_program program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program.");
  free(program_buffer);

  /* Build program */
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err) {
    /* Find size of log and print to std output */
    size_t log_size;
    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    handleError("Couldn't get build info.");
    char *program_log = (char *)calloc(log_size, sizeof(char));
    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
    handleError("Couldn't get build info.");
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }

  /* Create a kernel */
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError("Couldn't create a kernel.");

  /* Create buffers to hold the text characters and count */
  cl_mem text_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, text_size, text, &err);
  handleError("Couldn't create a buffer.");
  int result[4] = {0, 0, 0, 0};
  cl_mem result_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(result), result, &err);
  handleError("Couldn't create a buffer.");

  /* Create kernel argument */
  char pattern[16] = "thatwithhavefrom";
  err = clSetKernelArg(kernel, 0, sizeof(pattern), pattern);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &text_buffer);
  err |= clSetKernelArg(kernel, 2, sizeof(chars_per_item), &chars_per_item);
  err |= clSetKernelArg(kernel, 3, 4 * sizeof(int), NULL);
  err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &result_buffer);
  handleError("Couldn't set kernel arguments.");

  /* Create a command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  handleError("Couldn't create a command queue.");

  /* Enqueue kernel */
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
  handleError("Couldn't enqueue the kernel.");

  /* Read and print the result */
  err = clEnqueueReadBuffer(queue, result_buffer, CL_TRUE, 0, sizeof(result), &result, 0, NULL, NULL);
  handleError("Couldn't read the buffer.");

  printf("Number of occurrences of 'that': %d\n", result[0]);
  printf("Number of occurrences of 'with': %d\n", result[1]);
  printf("Number of occurrences of 'have': %d\n", result[2]);
  printf("Number of occurrences of 'from': %d\n", result[3]);

  /* Deallocate resources */
  clReleaseMemObject(result_buffer);
  clReleaseMemObject(text_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
