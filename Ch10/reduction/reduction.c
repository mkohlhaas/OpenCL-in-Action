#include <CL/cl.h>
#include <math.h>
#include <stdio.h>

#define PROGRAM_FILE "reduction.cl"
#define ARRAY_SIZE 1048576
#define NUM_KERNELS 2

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
  size_t local_size;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL);                                          handleError("Couldn't obtain device information.");
  fprintf(stderr, "work group size: %zu\n", local_size);

  // initialize output
  cl_int num_groups = ARRAY_SIZE / local_size;
  float *scalar_sum = (float *)malloc(num_groups     * sizeof(float));
  float *vector_sum = (float *)malloc(num_groups / 4 * sizeof(float));

  for (int i = 0; i < num_groups; i++) {
    scalar_sum[i] = 0.0f;
  }

  for (int i = 0; i < num_groups / 4; i++) {
    vector_sum[i] = 0.0f;
  }

  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                                     handleError("Couldn't create a context.");
  cl_program program = build_program(context, device, PROGRAM_FILE);

  cl_mem data_buffer       = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), data,       &err);   handleError("Couldn't create a buffer.");
  cl_mem scalar_sum_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, num_groups * sizeof(float), scalar_sum, &err);   handleError("Couldn't create a buffer.");
  cl_mem vector_sum_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, num_groups * sizeof(float), vector_sum, &err);   handleError("Couldn't create a buffer.");

  cl_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, properties, &err);                                               handleError("Couldn't create a command queue.");

  cl_kernel  kernel[NUM_KERNELS];
  char kernel_names[NUM_KERNELS][20] = {"reduction_scalar", "reduction_vector"};

  for (int i = 0; i < NUM_KERNELS; i++) {
    kernel[i] = clCreateKernel(program, kernel_names[i], &err);                                                                                 handleError("Couldn't create a kernel.");

    size_t global_size;
    err = clSetKernelArg(kernel[i], 0, sizeof(cl_mem), &data_buffer);
    if (i == 0) {
      global_size = ARRAY_SIZE;
      err |= clSetKernelArg(kernel[i], 1, local_size * 1 * sizeof(float), NULL);
      err |= clSetKernelArg(kernel[i], 2, sizeof(cl_mem), &scalar_sum_buffer);
    } else {
      global_size = ARRAY_SIZE / 4;
      err |= clSetKernelArg(kernel[i], 1, local_size * 4 * sizeof(float), NULL);
      err |= clSetKernelArg(kernel[i], 2, sizeof(cl_mem), &vector_sum_buffer);
    }                                                                                                                                           handleError("Couldn't set kernel arguments.");

    cl_event prof_event;
    err = clEnqueueNDRangeKernel(queue, kernel[i], 1, NULL, &global_size, &local_size, 0, NULL, &prof_event);                                   handleError("Couldn't enqueue the kernel.");
    clFinish(queue);

    cl_ulong time_start, time_end;
    err = clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);                               handleError("Couldn't get profiling information.");
    err = clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_END,   sizeof(time_end),   &time_end,   NULL);                               handleError("Couldn't get profiling information.");

    cl_ulong time_total = time_end - time_start;

    // read results
    float sum;
    if (i == 0) {
      err = clEnqueueReadBuffer(queue, scalar_sum_buffer, CL_BLOCKING, 0, num_groups * sizeof(float),     scalar_sum, 0, NULL, NULL);           handleError("Couldn't read the buffer.");
      sum = 0.0f;
      for (int j = 0; j < num_groups; j++) {
        sum += scalar_sum[j];
      }
    } else {
      err = clEnqueueReadBuffer(queue, vector_sum_buffer, CL_BLOCKING, 0, num_groups / 4 * sizeof(float), vector_sum, 0, NULL, NULL);           handleError("Couldn't read the buffer.");
      sum = 0.0f;
      for (int j = 0; j < num_groups / 4; j++) {
        sum += vector_sum[j];
      }
    }

    // check results
    printf("%s: ", kernel_names[i]);
    float actual_sum = 1.0f * ARRAY_SIZE / 2 * (ARRAY_SIZE - 1);
    if (fabs(sum - actual_sum) > 0.01 * fabs(sum)) {
      printf("Check FAILED. ");
    } else {
      printf("Check PASSED. ");
    }
    printf("Total time = %lu.\n", time_total);

    clReleaseEvent(prof_event);
  }

  free(scalar_sum);
  free(vector_sum);
  for (int i = 0; i < NUM_KERNELS; i++) {
    clReleaseKernel(kernel[i]);
  }
  clReleaseMemObject(scalar_sum_buffer);
  clReleaseMemObject(vector_sum_buffer);
  clReleaseMemObject(data_buffer);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
