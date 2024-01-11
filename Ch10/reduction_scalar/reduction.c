#include <CL/cl.h>
#include <math.h>
#include <stdio.h>

#define PROGRAM_FILE "reduction.cl"
#define KERNEL_FUNC "reduction_scalar"
#define ARRAY_SIZE (1024 * 1024)

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

  // initialize input
  float data[ARRAY_SIZE];
  for (int i = 0; i < ARRAY_SIZE; i++) {
    data[i] = (float)i;
  }

  cl_device_id device = create_device();

  // clang-format off
  size_t wg_size;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(wg_size), &wg_size, NULL);                                                    handleError("Couldn't obtain device information.");
  fprintf(stderr, "work group size: %zu\n", wg_size);

  // initialize output
  cl_int num_groups = ARRAY_SIZE / wg_size;
  float *scalar_sum = (float *)malloc(num_groups * sizeof(float));

  for (int i = 0; i < num_groups; i++) {
    scalar_sum[i] = 0.0f;
  }

  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                                         handleError("Couldn't create a context.");
  cl_program program = build_program(context, device, PROGRAM_FILE);

  cl_mem data_buffer       = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), data, &err);              handleError("Couldn't create a buffer.");
  cl_mem scalar_sum_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, num_groups * sizeof(float), scalar_sum, &err);       handleError("Couldn't create a buffer.");

  cl_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, properties, &err);                                                   handleError("Couldn't create a command queue.");

  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);                                                                                    handleError("Couldn't create a kernel.");
  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
  err |= clSetKernelArg(kernel, 1, wg_size * sizeof(float), NULL);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &scalar_sum_buffer);                                                                             handleError("Couldn't set kernel arguments.");

  cl_event prof_event;
  size_t global_size = ARRAY_SIZE;
  fprintf(stderr, "global size: %zu\n", global_size);
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &wg_size, 0, NULL, &prof_event);                                               handleError("Couldn't enqueue the kernel.");
  clFinish(queue);

  cl_ulong time_start, time_end;
  err = clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);                                     handleError("Couldn't get profiling information.");
  err = clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_END,   sizeof(time_end), &time_end, NULL);                                         handleError("Couldn't get profiling information.");

  cl_ulong time_total = time_end - time_start;

  // read results
  float sum;
  err = clEnqueueReadBuffer(queue, scalar_sum_buffer, CL_BLOCKING, 0, num_groups * sizeof(float), scalar_sum, 0, NULL, NULL);                       handleError("Couldn't read the buffer.");
  // clang-format on

  // sum up partial sums
  sum = 0.0f;
  for (int j = 0; j < num_groups; j++) {
    sum += scalar_sum[j];
  }

  // check results
  printf("%s: ", KERNEL_FUNC);
  float actual_sum = ARRAY_SIZE / 2.0 * (ARRAY_SIZE - 1);
  if (fabs(sum - actual_sum) > 0.01 * fabs(sum)) {
    printf("Check FAILED. ");
  } else {
    printf("Check PASSED. ");
  }
  printf("Total time = %lu.\n", time_total);

  clReleaseEvent(prof_event);
  free(scalar_sum);
  clReleaseKernel(kernel);
  clReleaseMemObject(scalar_sum_buffer);
  clReleaseMemObject(data_buffer);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
