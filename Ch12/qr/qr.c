#include <CL/cl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define PROGRAM_FILE "qr.cl"
#define KERNEL_FUNC "qr"
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

  /* Data and buffers */
  float a_mat[MATRIX_DIM][MATRIX_DIM];
  float q_mat[MATRIX_DIM][MATRIX_DIM];
  float r_mat[MATRIX_DIM][MATRIX_DIM];
  float check_mat[MATRIX_DIM][MATRIX_DIM];

  /* Initialize A matrix */
  srand((unsigned int)time(0));
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      a_mat[i][j] = (float)rand() / RAND_MAX;
      check_mat[i][j] = 0.0f;
    }
  }

  // clang-format off

  /* Create a device and context */
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                              handleError("Couldn't create a context.");

  /* Build the program */
  cl_program program = build_program(context, device, PROGRAM_FILE);

  /* Create a kernel */
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);                                                         handleError("Couldn't create a kernel.");

  /* Create buffer */
  cl_mem a_buffer    = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(a_mat), a_mat, &err);    handleError("Couldn't create a buffer.");
  cl_mem q_buffer    = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(q_mat), NULL, &err);                            handleError("Couldn't create q buffer.");
  cl_mem p_buffer    = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(q_mat), NULL, &err);                            handleError("Couldn't create p buffer.");
  cl_mem prod_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(q_mat), NULL, &err);                            handleError("Couldn't create prod buffer.");

  /* Create kernel arguments */
  err  = clSetKernelArg(kernel, 0, MATRIX_DIM * sizeof(float), NULL);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &a_buffer);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &q_buffer);
  err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &p_buffer);
  err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &prod_buffer);                                                        handleError("Couldn't set a kernel argument.");

  /* Create a command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                              handleError("Couldn't create a command queue.");

  /* Enqueue kernel */
  size_t global_size = MATRIX_DIM;
  size_t local_size  = MATRIX_DIM;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);                        handleError("Couldn't enqueue the kernel.");

  /* Read the results */
  err  = clEnqueueReadBuffer(queue, q_buffer, CL_BLOCKING, 0, sizeof(q_mat), q_mat, 0, NULL, NULL);
  err |= clEnqueueReadBuffer(queue, a_buffer, CL_BLOCKING, 0, sizeof(r_mat), r_mat, 0, NULL, NULL);                      handleError("Couldn't read the buffers.");
  // clang-format on

  /* Compute product of Q and R */
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      for (int k = 0; k < MATRIX_DIM; k++) {
        check_mat[i][j] += q_mat[i][k] * r_mat[k][j];
      }
    }
  }

  cl_int check = CL_TRUE;
  for (int i = 0; i < MATRIX_DIM; i++) {
    for (int j = 0; j < MATRIX_DIM; j++) {
      if (fabs(a_mat[i][j] - check_mat[i][j]) > 0.01f) {
        check = CL_FALSE;
        break;
      }
    }
  }

  if (check) {
    printf("QR decomposition check SUCCEEDED.\n");
  } else {
    printf("QR decomposition check FAILED.\n");
  }

  clReleaseMemObject(a_buffer);
  clReleaseMemObject(q_buffer);
  clReleaseMemObject(p_buffer);
  clReleaseMemObject(prod_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
