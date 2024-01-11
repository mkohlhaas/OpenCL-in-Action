#include <CL/cl.h>
#include <stdio.h>

#define PROGRAM_FILE "profile_items.cl"
#define KERNEL_FUNC "profile_items"
#define NUM_INTS 4096
#define NUM_ITEMS 2048
#define NUM_ITERATIONS 20000

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

int main() {
  /* Initialize data */
  int data[NUM_INTS];
  for (int i = 0; i < NUM_INTS; i++) {
    data[i] = i;
  }

  // clang-format off
  cl_device_id device  = create_device();
  cl_context   context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                        handleError("Couldn't create a context.");
  cl_program   program = build_program(context, device, PROGRAM_FILE);
  cl_kernel    kernel  = clCreateKernel(program, KERNEL_FUNC, &err);                                                 handleError("Couldn't create a kernel.");

  cl_mem data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data), data, &err);  handleError("Couldn't create buffer.");

  size_t num_items = NUM_ITEMS;
  cl_int num_ints = NUM_INTS;
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);                                                     handleError("Couldn't set kernel argument.");
  err = clSetKernelArg(kernel, 1, sizeof(num_ints), &num_ints);                                                      handleError("Couldn't set kernel argument.");

  cl_queue_properties queue_properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, queue_properties, &err);              handleError("Couldn't create a command queue.");

  cl_event prof_event;
  cl_ulong time_start, time_end;
  cl_ulong time_total = 0.0f;

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &num_items, NULL, 0, NULL, &prof_event);                    handleError("Couldn't enqueue the kernel.");
    err = clFinish(queue);                                                                                           handleError("Couldn't finish.");
    err = clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);    handleError("Couldn't get #1 profiling information.");
    err = clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_END,   sizeof(time_end),   &time_end,   NULL);    handleError("Couldn't get #2 profiling information.");
    time_total += time_end - time_start;
  }
  printf("Average time = %lu\n", time_total / NUM_ITERATIONS);

  clReleaseEvent(prof_event);
  clReleaseKernel(kernel);
  clReleaseMemObject(data_buffer);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
