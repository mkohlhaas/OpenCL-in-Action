#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
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

void printProgramLog(cl_program program, cl_device_id device) {
  size_t log_size;
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  char *program_log = (char *)malloc(log_size + 1);
  program_log[log_size] = '\0';
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
  printf("%s\n", program_log);
  free(program_log);
  exit(EXIT_FAILURE);
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

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename) {

  cl_program program;
  FILE *program_handle;
  char *program_buffer, *program_log;
  size_t program_size, log_size;
  int err;

  /* Read program file and place content into buffer */
  program_handle = fopen(filename, "r");
  if (program_handle == NULL) {
    perror("Couldn't find the program file");
    exit(1);
  }
  fseek(program_handle, 0, SEEK_END);
  program_size = ftell(program_handle);
  rewind(program_handle);
  program_buffer = (char *)malloc(program_size + 1);
  program_buffer[program_size] = '\0';
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  /* Create program from file */
  program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
  if (err < 0) {
    perror("Couldn't create the program");
    exit(1);
  }
  free(program_buffer);

  /* Build program */
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err < 0) {

    /* Find size of log and print to std output */
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    program_log = (char *)malloc(log_size + 1);
    program_log[log_size] = '\0';
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(1);
  }

  return program;
}

int main(void) {

  /* OpenCL data structures */
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;
  cl_int i, j, err;

  /* Data and buffers */
  float full_matrix[80], zero_matrix[80];
  const size_t buffer_origin[3] = {5 * sizeof(float), 3, 0};
  const size_t host_origin[3] = {1 * sizeof(float), 1, 0};
  const size_t region[3] = {4 * sizeof(float), 4, 1};
  cl_mem matrix_buffer;

  /* Initialize data */
  for (i = 0; i < 80; i++) {
    full_matrix[i] = i * 1.0f;
    zero_matrix[i] = 0.0;
  }

  /* Create a device and context */
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Build the program and create the kernel */
  program = build_program(context, device, PROGRAM_FILE);
  kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError("Couldn't create a kernel");

  /* Create a buffer to hold 80 floats */
  matrix_buffer =
      clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(full_matrix), full_matrix, &err);
  handleError("Couldn't create a buffer object");

  /* Set buffer as argument to the kernel */
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &matrix_buffer);
  handleError("Couldn't set the buffer as the kernel argument");

  /* Create a command queue */
  queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
  handleError("Couldn't create a command queue");

  /* Enqueue kernel */
  err = clEnqueueTask(queue, kernel, 0, NULL, NULL);
  handleError("Couldn't enqueue the kernel");

  /* Enqueue command to write to buffer */
  err = clEnqueueWriteBuffer(queue, matrix_buffer, CL_TRUE, 0, sizeof(full_matrix), full_matrix, 0, NULL, NULL);
  handleError("Couldn't write to the buffer object");

  /* Enqueue command to read rectangle of data */
  err = clEnqueueReadBufferRect(queue, matrix_buffer, CL_TRUE, buffer_origin, host_origin, region, 10 * sizeof(float),
                                0, 10 * sizeof(float), 0, zero_matrix, 0, NULL, NULL);
  handleError("Couldn't read the rectangle from the buffer object");

  /* Display updated buffer */
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 10; j++) {
      printf("%6.1f", zero_matrix[j + i * 10]);
    }
    printf("\n");
  }

  /* Deallocate resources */
  clReleaseMemObject(matrix_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
