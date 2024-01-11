#include <CL/cl.h>
#include <stdio.h>

#define PROGRAM_FILE "profile_read.cl"
#define KERNEL_FUNC "profile_read"
#define NUM_BYTES 131072
#define NUM_ITERATIONS 2000
#define PROFILE_READ 1

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
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }

  return program;
}

int main() {

  /* OpenCL data structures */
  cl_int num_vectors;

  /* Data and events */
  char data[NUM_BYTES];
  cl_mem data_buffer;
  cl_event prof_event;
  cl_ulong time_start, time_end, total_time;
  void *mapped_memory;

  // clang-format off
  cl_device_id device  = create_device();
  cl_context   context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                    handleError("Couldn't create a context.");
  cl_program   program = build_program(context, device, PROGRAM_FILE);
  cl_kernel    kernel  = clCreateKernel(program, KERNEL_FUNC, &err);                                                             handleError("Couldn't create a kernel.");

  /* Create a buffer to hold data */
  data_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(data), NULL, &err);
  if (err < 0) {
    perror("Couldn't create a buffer");
    exit(1);
  };

  /* Create kernel argument */
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
  if (err < 0) {
    perror("Couldn't set a kernel argument");
    exit(1);
  };

  /* Tell kernel number of char16 vectors */
  num_vectors = NUM_BYTES / 16;
  clSetKernelArg(kernel, 1, sizeof(num_vectors), &num_vectors);

  cl_queue_properties queue_properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, queue_properties, &err);                          handleError("Couldn't create a command queue.");

  size_t offset = 0;
  size_t global_size = 1;
  size_t local_size = 1;

  total_time = 0.0f;

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    err = clEnqueueNDRangeKernel(queue, kernel, 1, &offset, &global_size, &local_size, 0, NULL, NULL);                           handleError("Couldn't enqueue the kernel.");
#ifdef PROFILE_READ
    err = clEnqueueReadBuffer(queue, data_buffer, CL_BLOCKING, 0, sizeof(data), data, 0, NULL, &prof_event);
    if (err < 0) {
      perror("Couldn't read the buffer");
      exit(1);
    }
#else
    mapped_memory = clEnqueueMapBuffer(queue, data_buffer, CL_BLOCKING, CL_MAP_READ, 0, sizeof(data), 0, NULL, &prof_event, &err);   handleError("Couldn't map the buffer to host memory");
#endif
    err = clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);                handleError("Couldn't read #1 profiling information.");
    err = clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);                      handleError("Couldn't read #2 profiling information.");
    total_time += time_end - time_start;
#ifndef PROFILE_READ
    err = clEnqueueUnmapMemObject(queue, data_buffer, mapped_memory, 0, NULL, NULL);                                             handleError("Couldn't unmap the buffer");
#endif
  }

#ifdef PROFILE_READ
  printf("Average read time: %lu\n", total_time / NUM_ITERATIONS);
#else
  printf("Average map time: %lu\n", total_time / NUM_ITERATIONS);
#endif

  clReleaseEvent(prof_event);
  clReleaseMemObject(data_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
