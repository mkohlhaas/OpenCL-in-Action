#include <CL/cl.h>
#include <stdio.h>

#define PROGRAM_FILE "transpose.cl"
#define KERNEL_FUNC "transpose"
#define MATRIX_DIM 64

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "Error Code: %d\n", err);
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
    perror("Couldn't find the program file.");
    exit(EXIT_FAILURE);
  }
  fseek(program_handle, 0, SEEK_END);
  size_t program_size = ftell(program_handle);
  rewind(program_handle);
  char *program_buffer = (char *)malloc(program_size);
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  cl_program program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program.");
  free(program_buffer);

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  size_t log_size;
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  char *program_log = (char *)malloc(log_size);
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
  printf("Build log:\n%s\n", program_log);
  free(program_log);
  if (err) {
    exit(EXIT_FAILURE);
  }
  return program;
}

int main(void) {

  float data[MATRIX_DIM][MATRIX_DIM];
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      data[i][j] = i * MATRIX_DIM + j;
    }
  }

  // clang-format off
  cl_device_id device = create_device();
  cl_context context  = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                           handleError("Couldn't create a context.");
  cl_program program  = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel    = clCreateKernel(program, KERNEL_FUNC, &err);                                                    handleError("Couldn't create a kernel.");
  cl_mem data_buffer  = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data), data, &err);   handleError("Couldn't create a buffer.");

  size_t global_size = (MATRIX_DIM / 4 * (MATRIX_DIM / 4 + 1)) / 2;
  fprintf(stderr, "global size: %zu\n", global_size);
  cl_ulong mem_size;
  err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);                          handleError("Couldn't get device info.");

  cl_uint matrix_dim = MATRIX_DIM / 4;
  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem),     &data_buffer);
  err |= clSetKernelArg(kernel, 1, (size_t)mem_size,   NULL);
  err |= clSetKernelArg(kernel, 2, sizeof(matrix_dim), &matrix_dim);                                                   handleError("Couldn't set a kernel argument.");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                            handleError("Couldn't create a command queue.");
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);                             handleError("Couldn't enqueue the kernel.");
  // TODO: use clGetKernelWorkGroupInfo to find out error cause
  err = clEnqueueReadBuffer(queue, data_buffer, CL_BLOCKING, 0, sizeof(data), data, 0, NULL, NULL);                    handleError("Couldn't read the buffer.");
  // clang-format on

  cl_int check = CL_TRUE;
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      if (data[i][j] != j * MATRIX_DIM + i) {
        check = CL_FALSE;
        break;
      }
    }
  }

  if (check) {
    printf("Transpose check SUCCEEDED.\n");
  } else {
    printf("Transpose check FAILED.\n");
  }

  clReleaseMemObject(data_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
