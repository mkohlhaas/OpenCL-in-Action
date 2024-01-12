#include <CL/cl.h>
#include <stdio.h>

#define PROGRAM_FILE "bsort8.cl"
#define KERNEL_FUNC "bsort8"
#define ASCENDING 0
#define DESCENDING -1

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

int main(void) {

  /* Initialize data */
  float data[] = {3.0f, 5.0f, 4.0f, 6.0f, 0.0f, 7.0f, 2.0f, 1.0f};
  printf("Input:  %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

  /* Create device and context */
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context.");

  /* Create kernel */
  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError("Couldn't create a kernel");

  /* Create buffer */
  cl_mem data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data), data, &err);
  handleError("Couldn't create a buffer");

  /* Create kernel arguments */
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
  handleError("Couldn't set a kernel argument");

  cl_int dir = ASCENDING;
  err = clSetKernelArg(kernel, 1, sizeof(int), &dir);
  handleError("Couldn't set a kernel argument");

  /* Create command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  handleError("Couldn't create a command queue");

  /* Enqueue kernel */
  const size_t global_work_size[] = {1};
  const size_t local_work_size[] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
  handleError("Couldn't enqueue the kernel");

  /* Read and print the result */
  err = clEnqueueReadBuffer(queue, data_buffer, CL_BLOCKING, 0, sizeof(data), &data, 0, NULL, NULL);
  handleError("Couldn't read the buffer");
  printf("Output: %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

  /* Check the result */
  cl_int check = 1;

  /* Check ascending sort */
  if (dir == ASCENDING) {
    for (int i = 1; i < 8; i++) {
      if (data[i] < data[i - 1]) {
        check = CL_FALSE;
        break;
      }
    }
  }
  /* Check descending sort */
  if (dir == DESCENDING) {
    for (int i = 1; i < 8; i++) {
      if (data[i] > data[i - 1]) {
        check = CL_FALSE;
        break;
      }
    }
  }

  if (check) {
    printf("Bitonic sort SUCCEEDED.\n");
  } else {
    printf("Bitonic sort FAILED.\n");
  }

  clReleaseMemObject(data_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
