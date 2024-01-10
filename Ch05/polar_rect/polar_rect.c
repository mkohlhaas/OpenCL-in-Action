#include <CL/cl.h>
#include <math.h>
#include <stdio.h>

#define PROGRAM_FILE "polar_rect.cl"
#define KERNEL_FUNC "polar_rect"

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

/* Find a GPU or CPU associated with the first available platform */
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

/* Create program from a file and compile it */
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
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }

  return program;
}

int main() {
  // input
  float angles[4] = {3 * M_PI / 8, 3 * M_PI / 4, 4 * M_PI / 3, 11 * M_PI / 6};
  float r_coords[4] = {2, 1, 3, 4};

  // output
  float x_coords[4];
  float y_coords[4];

  // clang-format off
  cl_device_id device = create_device();
  cl_context context  = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                        handleError("Couldn't create a context");
  cl_program program  = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel    = clCreateKernel(program, KERNEL_FUNC, &err);                                                                 handleError("Couldn't create a kernel");

  cl_mem r_coords_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(r_coords), r_coords, &err);      handleError("Couldn't create a buffer");
  cl_mem angles_buffer   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(angles), angles, &err);          handleError("Couldn't create a buffer");
  cl_mem x_coords_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,                       sizeof(x_coords), NULL, &err);          handleError("Couldn't create a buffer");
  cl_mem y_coords_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,                       sizeof(y_coords), NULL, &err);          handleError("Couldn't create a buffer");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &r_coords_buffer);                                                                handleError("Couldn't set a kernel argument");
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &angles_buffer);                                                                  handleError("Couldn't set a kernel argument");
  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &x_coords_buffer);                                                                handleError("Couldn't set a kernel argument");
  err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &y_coords_buffer);                                                                handleError("Couldn't set a kernel argument");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                                         handleError("Couldn't create a command queue");

  const size_t global_work_size[1] = {1};
  const size_t local_work_size[1] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);                           handleError("Couldn't enqueue the kernel");

  err = clEnqueueReadBuffer(queue, x_coords_buffer, CL_BLOCKING, 0, sizeof(x_coords), &x_coords, 0, NULL, NULL);                    handleError("Couldn't read the buffer");
  err = clEnqueueReadBuffer(queue, y_coords_buffer, CL_BLOCKING, 0, sizeof(y_coords), &y_coords, 0, NULL, NULL);                    handleError("Couldn't read the buffer");
  // clang-format on

  for (int i = 0; i < 4; i++) {
    printf("(%6.3f, %6.3f)\n", x_coords[i], y_coords[i]);
  }

  clReleaseMemObject(r_coords_buffer);
  clReleaseMemObject(angles_buffer);
  clReleaseMemObject(x_coords_buffer);
  clReleaseMemObject(y_coords_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
