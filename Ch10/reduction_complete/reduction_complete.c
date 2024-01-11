#include <CL/cl.h>
#include <math.h>
#include <stdio.h>

#define PROGRAM_FILE "reduction_complete.cl"
#define ARRAY_SIZE 1048576
#define KERNEL_1 "reduction_vector"
#define KERNEL_2 "reduction_complete"

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

  // initialize data
  float data[ARRAY_SIZE];
  for (int i = 0; i < ARRAY_SIZE; i++) {
    data[i] = 1.0f * i;
  }

  // clang-format off
  cl_device_id device = create_device();
  size_t local_size;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL);                            handleError("Couldn't obtain device information.");

  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                       handleError("Couldn't create a context.");
  cl_program program = build_program(context, device, PROGRAM_FILE);

  cl_mem data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), data, &err);  handleError("Couldn't create a buffer.");
  cl_mem sum_buffer  = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float), NULL, &err);                                     handleError("Couldn't create a buffer.");

  cl_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, properties, &err);                                 handleError("Couldn't create a command queue.");

  cl_kernel vector_kernel   = clCreateKernel(program, KERNEL_1, &err);                                                            handleError("Couldn't create a kernel.");
  cl_kernel complete_kernel = clCreateKernel(program, KERNEL_2, &err);                                                            handleError("Couldn't create a kernel.");

  // vector kernel
  err  = clSetKernelArg(vector_kernel,   0, sizeof(cl_mem), &data_buffer);
  err |= clSetKernelArg(vector_kernel,   1, local_size * 4 * sizeof(float), NULL);                                                handleError("Couldn't set kernel arguements.");
  // complete kernel
  err  = clSetKernelArg(complete_kernel, 0, sizeof(cl_mem), &data_buffer);
  err |= clSetKernelArg(complete_kernel, 1, local_size * 4 * sizeof(float), NULL);
  err |= clSetKernelArg(complete_kernel, 2, sizeof(cl_mem), &sum_buffer);                                                         handleError("Couldn't set kernel arguement.");

  size_t global_size = ARRAY_SIZE / 4;
  cl_event start_event, end_event;
  err = clEnqueueNDRangeKernel(queue, vector_kernel, 1, NULL, &global_size, &local_size, 0, NULL, &start_event);                  handleError("Couldn't enqueue the kernel.");
  printf("Global size = %zu\n", global_size);

  // perform successive stages of the reduction
  while (global_size / local_size > local_size) {
    global_size = global_size / local_size;
    printf("Global size = %zu\n", global_size);
    err = clEnqueueNDRangeKernel(queue, vector_kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);                        handleError("Couldn't enqueue the kernel.");
  }

  global_size = global_size / local_size;
  printf("Global size = %zu\n", global_size);
  err = clEnqueueNDRangeKernel(queue, complete_kernel, 1, NULL, &global_size, NULL, 0, NULL, &end_event);                         handleError("Couldn't enqueue the kernel.");
  clFinish(queue);

  cl_ulong time_start, time_end;
  err = clGetEventProfilingInfo(start_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);                  handleError("Couldn't get profiling information.");
  err = clGetEventProfilingInfo(end_event,   CL_PROFILING_COMMAND_END,   sizeof(time_end),   &time_end,   NULL);                  handleError("Couldn't get profiling information.");
  cl_ulong time_total = time_end - time_start;

  // read results
  float sum;
  err = clEnqueueReadBuffer(queue, sum_buffer, CL_BLOCKING, 0, sizeof(float), &sum, 0, NULL, NULL);                               handleError("Couldn't read the buffer.");
  // clang-format on

  // check results
  float actual_sum = (ARRAY_SIZE / 2.0f) * (ARRAY_SIZE - 1); // when Gauss was in primrary school
  if (fabs(sum - actual_sum) > 0.01 * fabs(sum)) {
    printf("Check failed.\n");
  } else {
    printf("Check passed.\n");
  }
  printf("Total time = %lu\n", time_total);

  clReleaseEvent(start_event);
  clReleaseEvent(end_event);
  clReleaseMemObject(sum_buffer);
  clReleaseMemObject(data_buffer);
  clReleaseKernel(vector_kernel);
  clReleaseKernel(complete_kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
