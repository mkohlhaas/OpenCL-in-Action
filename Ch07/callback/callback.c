#include <CL/cl.h>
#include <stdio.h>

#define PROGRAM_FILE "callback.cl"
#define KERNEL_FUNC "callback"

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

void CL_CALLBACK kernel_completed(cl_event _event, cl_int _status, void *data) { printf("%s", (char *)data); }

void CL_CALLBACK read_completed(cl_event _event, cl_int _status, void *data) {
  float *buffer_data = (float *)data;
  cl_bool check = CL_TRUE;
  for (int i = 0; i < 4096; i++) {
    if (buffer_data[i] != 5.0) {
      check = CL_FALSE;
      break;
    }
  }

  if (check) {
    printf("The data has been initialized successfully.\n");
  } else {
    printf("The data has NOT been initialized successfully.\n");
  }
}

int main(void) {
  // clang-format off
  cl_device_id device  = create_device();
  cl_context   context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                        handleError("Couldn't create a context.");
  cl_program   program = build_program(context, device, PROGRAM_FILE);
  cl_kernel    kernel  = clCreateKernel(program, KERNEL_FUNC, &err);                                                 handleError("Couldn't create a kernel.");

  float data[4096];
  cl_mem data_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(data), NULL, &err);                         handleError("Couldn't create a buffer.");
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);                                                     handleError("Couldn't set kernel argument.");
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                          handleError("Couldn't create a command queue.");

  size_t global_work_size[1] = {1};
  size_t local_work_size[1]  = {1};
  cl_event kernel_event, read_event;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &kernel_event);   handleError("Couldn't enqueue the kernel.");
  err = clEnqueueReadBuffer(queue, data_buffer, CL_FALSE, 0, sizeof(data), &data, 0, NULL, &read_event);             handleError("Couldn't read result buffer.");
  err = clSetEventCallback(kernel_event, CL_COMPLETE, &kernel_completed,  "The kernel finished successfully.\n\0");  handleError("Couldn't set kernel event callback.");
  err = clSetEventCallback(read_event,   CL_COMPLETE, &read_completed, data);                                        handleError("Couldn't set read event callback.");
  // clang-format on

  clReleaseMemObject(data_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
