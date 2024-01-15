#include <CL/cl.h>
#include <stdio.h>

#define PROGRAM_FILE "vec_reflect.cl"
#define KERNEL_FUNC "vec_reflect"

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

cl_program build_program(cl_context ctx, cl_device_id device, const char *filename) {
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
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *program_log = (char *)malloc(log_size);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }
  return program;
}

int main(void) {

  // clang-format off

  /* Create a device and context */
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                  handleError("Couldn't create a context.");

  /* Build the program */
  cl_program program = build_program(context, device, PROGRAM_FILE);

  /* Create a kernel */
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);                                             handleError("Couldn't create a kernel.");

  /* Create buffer */
  cl_mem reflect_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 4 * sizeof(float), NULL, &err);         handleError("Couldn't create a buffer.");

  /* Create kernel argument */
  float x[4] = {1.0f, 2.0f, 3.0f, 4.0f};
  float u[4] = {0.0f, 5.0f, 0.0f, 0.0f};
  err  = clSetKernelArg(kernel, 0, sizeof(x), x);
  err |= clSetKernelArg(kernel, 1, sizeof(u), u);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &reflect_buffer);                                         handleError("Couldn't set a kernel argument.");

  /* Create a command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                  handleError("Couldn't create a command queue");

  /* Enqueue kernel */
  const size_t global_work_offset[] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_offset, NULL, 0, NULL, NULL);             handleError("Couldn't enqueue the kernel");

  /* Read and print the result */
  float reflect[4];
  err = clEnqueueReadBuffer(queue, reflect_buffer, CL_BLOCKING, 0, sizeof(reflect), reflect, 0, NULL, NULL); handleError("Couldn't read the buffer");
  // clang-format on
  printf("Result: %f %f %f %f\n", reflect[0], reflect[1], reflect[2], reflect[3]);

  clReleaseMemObject(reflect_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
