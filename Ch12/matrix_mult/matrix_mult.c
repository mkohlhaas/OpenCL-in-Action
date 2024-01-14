#include <CL/cl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define PROGRAM_FILE "matrix_mult.cl"
#define TRANSPOSE_FUNC "transpose"
#define MULT_FUNC "matrix_mult"
#define MATRIX_DIM 32

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

  float a_mat[MATRIX_DIM][MATRIX_DIM];
  float b_mat[MATRIX_DIM][MATRIX_DIM];
  float c_mat[MATRIX_DIM][MATRIX_DIM];
  float check_mat[MATRIX_DIM][MATRIX_DIM];

  /* Initialize A, B, and Check matrices */
  srand((unsigned int)time(0));
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      a_mat[i][j] = (float)rand() / RAND_MAX;
    }
  }
  srand((unsigned int)time(0));
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      b_mat[i][j] = (float)rand() / RAND_MAX;
      check_mat[i][j] = 0.0f;
    }
  }
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      for (int k = 0; k < MATRIX_DIM; k++) {
        check_mat[i][j] += a_mat[i][k] * b_mat[k][j];
      }
    }
  }

  // clang-format off

  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                             handleError("Couldn't create a context.");

  cl_program program         = build_program(context, device, PROGRAM_FILE);
  cl_kernel transpose_kernel = clCreateKernel(program, TRANSPOSE_FUNC, &err);                                           handleError("Couldn't create a kernel.");
  cl_kernel mult_kernel      = clCreateKernel(program, MULT_FUNC, &err);                                                handleError("Couldn't create a kernel.");

  cl_mem a_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(a_mat), a_mat, &err);      handleError("Couldn't create a buffer.");
  cl_mem b_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(b_mat), b_mat, &err);      handleError("Couldn't create a buffer.");
  cl_mem c_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(c_mat), NULL, &err);                              handleError("Couldn't create a buffer.");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                             handleError("Couldn't create a command queue");

  size_t global_size = (MATRIX_DIM / 4 * (MATRIX_DIM / 4 + 1)) / 2;
  cl_ulong mem_size;
  err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);                           handleError("Couldn't get device info.");

  cl_uint matrix_dim = MATRIX_DIM / 4;
  err  = clSetKernelArg(transpose_kernel, 0, sizeof(cl_mem), &b_buffer);
  err |= clSetKernelArg(transpose_kernel, 1, (size_t)mem_size, NULL);
  err |= clSetKernelArg(transpose_kernel, 2, sizeof(matrix_dim), &matrix_dim);                                          handleError("Couldn't set an argument for the transpose kernel.");
  err = clEnqueueNDRangeKernel(queue, transpose_kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);                    handleError("Couldn't enqueue the transpose kernel.");

  global_size = MATRIX_DIM;
  err  = clSetKernelArg(mult_kernel, 0, sizeof(cl_mem), &a_buffer);
  err |= clSetKernelArg(mult_kernel, 1, sizeof(cl_mem), &b_buffer);
  err |= clSetKernelArg(mult_kernel, 2, sizeof(cl_mem), &c_buffer);                                                     handleError("Couldn't set an argument for the multiplication kernel.");
  err = clEnqueueNDRangeKernel(queue, mult_kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);                         handleError("Couldn't enqueue the multiplication kernel.");

  err = clEnqueueReadBuffer(queue, c_buffer, CL_BLOCKING, 0, sizeof(c_mat), c_mat, 0, NULL, NULL);                      handleError("Couldn't read the buffer");
  // clang-format on

  cl_int check = CL_TRUE;
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      if (fabs(c_mat[i][j] - check_mat[i][j]) > 0.01f) {
        check = CL_FALSE;
        break;
      }
    }
  }

  if (check) {
    printf("Multiplication check SUCCEEDED.\n");
  } else {
    printf("Multiplication check FAILED.\n");
  }

  clReleaseMemObject(a_buffer);
  clReleaseMemObject(b_buffer);
  clReleaseMemObject(c_buffer);
  clReleaseKernel(mult_kernel);
  clReleaseKernel(transpose_kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
