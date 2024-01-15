#include "fft_check.c"
#include <CL/cl.h>
#include <stdio.h>

#define PROGRAM_FILE "rdft.cl"
#define KERNEL_FUNC "rdft"
#define NUM_POINTS 256

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

  /* initialize data with a rectangle function */
  float input[NUM_POINTS];
  double check_input[NUM_POINTS][2];
  for (int i = 0; i < NUM_POINTS / 4; i++) {
    input[i] = 1.0f;
    check_input[i][0] = 1.0;
    check_input[i][1] = 0.0;
  }
  for (int i = NUM_POINTS / 4; i < NUM_POINTS; i++) {
    input[i] = 0.0f;
    check_input[i][0] = 0.0;
    check_input[i][1] = 0.0;
  }

  // clang-format off
  cl_device_id device      = create_device();
  cl_context   context     = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                          handleError("Couldn't create a context.");
  cl_program   program     = build_program(context, device, PROGRAM_FILE);
  cl_kernel    kernel      = clCreateKernel(program, KERNEL_FUNC, &err);                                                                   handleError("Couldn't create the initial kernel.");
  cl_mem       data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_POINTS * sizeof(float), input, &err);   handleError("Couldn't create a buffer.");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);                                                                           handleError("Couldn't set a kernel argument.");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                                                handleError("Couldn't create a command queue.");

  size_t global_size = (NUM_POINTS / 2) + 1;
  size_t local_size  = (NUM_POINTS / 2) + 1;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);                                          handleError("Couldn't enqueue the kernel.");

  float output[NUM_POINTS];
  err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0, NUM_POINTS * sizeof(float), output, 0, NULL, NULL);                            handleError("Couldn't read the buffer");
  // clang-format on

  int check = CL_TRUE;
  double check_output[NUM_POINTS][2];
  fft(NUM_POINTS, check_input, check_output);
  if ((fabs(output[0] - check_output[0][0]) > 0.001) || (fabs(output[1] - check_output[NUM_POINTS / 2][0]) > 0.001)) {
    check = CL_FALSE;
  }

  for (int i = 2; i < NUM_POINTS / 2; i += 2) {
    if ((fabs(output[i] - check_output[i / 2][0]) > 0.001) || (fabs(output[i + 1] - check_output[i / 2][1]) > 0.001)) {
      check = CL_FALSE;
      break;
    }
  }

  if (check) {
    printf("Real-valued DFT check SUCCEEDED.\n");
  } else {
    printf("Real-valued DFT check FAILED.\n");
  }

  clReleaseMemObject(data_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
