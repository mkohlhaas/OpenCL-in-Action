#include "fft_check.c"
#include <CL/cl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PROGRAM_FILE "fft.cl"
#define INIT_FUNC "fft_init"
#define STAGE_FUNC "fft_stage"
#define SCALE_FUNC "fft_scale"
/* Each point contains 2 floats - 1 real, 1 imaginary */
#define NUM_POINTS 8192
/* 1 - forward FFT, -1 - inverse FFT */
#define DIRECTION 1

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

  /* initialize data */
  float data[NUM_POINTS * 2];
  double check_input[NUM_POINTS][2];
  srand(time(NULL));
  for (int i = 0; i < NUM_POINTS; i++) {
    data[2 * i] = rand();
    data[2 * i + 1] = rand();
    check_input[i][0] = data[2 * i];
    check_input[i][1] = data[2 * i + 1];
  }

  // clang-format off
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                            handleError("Couldn't create a context.");
  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel init_kernel = clCreateKernel(program, INIT_FUNC, &err);                                                                    handleError("Couldn't create the initial kernel.");
  cl_kernel stage_kernel = clCreateKernel(program, STAGE_FUNC, &err);                                                                  handleError("Couldn't create the stage kernel.");
  cl_kernel scale_kernel = clCreateKernel(program, SCALE_FUNC, &err);                                                                  handleError("Couldn't create the scale kernel.");
  cl_mem input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 2 * NUM_POINTS * sizeof(float), data, &err);   handleError("Couldn't create a buffer.");
  cl_mem data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, 2 * NUM_POINTS * sizeof(float), NULL, &err);                         handleError("Couldn't create a buffer.");

  /* Determine maximum work-group size */
  size_t local_size;
  err = clGetKernelWorkGroupInfo(init_kernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL);               handleError("Couldn't find the maximum work-group size.");
  local_size = (int)pow(2, trunc(log2(local_size)));

  /* Determine local memory size */
  cl_ulong local_mem_size;
  err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, NULL);                              handleError("Couldn't determine the local memory size.");
  local_mem_size -= 2 * 1024;

  /* initialize kernel arguments */
  int direction = DIRECTION;
  unsigned num_points = NUM_POINTS;
  unsigned points_per_group = local_mem_size / (2 * sizeof(float));
  if (points_per_group > num_points) {
    points_per_group = num_points;
  }

  /* Set kernel arguments */
  err  = clSetKernelArg(init_kernel, 0, sizeof(cl_mem), &input_buffer);
  err |= clSetKernelArg(init_kernel, 1, sizeof(cl_mem), &data_buffer);
  err |= clSetKernelArg(init_kernel, 2, local_mem_size, NULL);
  err |= clSetKernelArg(init_kernel, 3, sizeof(points_per_group), &points_per_group);
  err |= clSetKernelArg(init_kernel, 4, sizeof(num_points), &num_points);
  err |= clSetKernelArg(init_kernel, 5, sizeof(direction), &direction);                                                                handleError("Couldn't set a kernel argument.");

  /* Create a command queue */
  cl_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, properties, &err);                                      handleError("Couldn't create a command queue.");

  /* Enqueue initial kernel */
  size_t global_size = (num_points / points_per_group) * local_size;
  err = clEnqueueNDRangeKernel(queue, init_kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);                                 handleError("Couldn't enqueue the initial kernel.");

  /* Enqueue further stages of the FFT */
  if (num_points > points_per_group) {
    err = clSetKernelArg(stage_kernel, 0, sizeof(cl_mem), &data_buffer);
    err |= clSetKernelArg(stage_kernel, 2, sizeof(points_per_group), &points_per_group);
    err |= clSetKernelArg(stage_kernel, 3, sizeof(direction), &direction);                                                             handleError("Couldn't set a kernel argument.");

    for (unsigned stage = 2; stage <= num_points / points_per_group; stage <<= 1) {
      err = clSetKernelArg(stage_kernel, 1, sizeof(stage), &stage);
      handleError("Couldn't set a kernel argument.");
      err = clEnqueueNDRangeKernel(queue, stage_kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);                            handleError("Couldn't enqueue the stage kernel");
    }
  }

  /* Scale values if performing the inverse FFT */
  if (direction < 0) {
    err = clSetKernelArg(scale_kernel, 0, sizeof(cl_mem), &data_buffer);
    err |= clSetKernelArg(scale_kernel, 1, sizeof(points_per_group), &points_per_group);
    err |= clSetKernelArg(scale_kernel, 2, sizeof(num_points), &num_points);                                                           handleError("Couldn't set a kernel argument.");
    err = clEnqueueNDRangeKernel(queue, scale_kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);                              handleError("Couldn't enqueue the scale kernel");
  }

  /* Read the results */
  err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0, 2 * NUM_POINTS * sizeof(float), data, 0, NULL, NULL);                      handleError("Couldn't read the buffer.");
  // clang-format on

  /* Compute accurate values */
  double check_output[NUM_POINTS][2];
  if (direction > 0) {
    fft(NUM_POINTS, check_input, check_output);
  } else {
    ifft(NUM_POINTS, check_output, check_input);
  }

  double error = 0.0;
  for (int i = 0; i < NUM_POINTS; i++) {
    error += fabs(check_output[i][0] - data[2 * i]) / fmax(fabs(check_output[i][0]), 0.0001);
    error += fabs(check_output[i][1] - data[2 * i + 1]) / fmax(fabs(check_output[i][1]), 0.0001);
  }
  error = error / (NUM_POINTS * 2);

  printf("%u-point ", num_points);
  if (direction > 0) {
    printf("FFT ");
  } else {
    printf("IFFT ");
  }
  printf("completed with %f average relative error.\n", error);

  clReleaseMemObject(input_buffer);
  clReleaseMemObject(data_buffer);
  clReleaseKernel(init_kernel);
  clReleaseKernel(stage_kernel);
  clReleaseKernel(scale_kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
