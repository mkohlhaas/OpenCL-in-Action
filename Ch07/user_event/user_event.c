#include <CL/cl.h>
#include <stdio.h>
#include <unistd.h>

#define PROGRAM_FILE "user_event.cl"
#define KERNEL_FUNC "user_event"

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

void CL_CALLBACK read_complete(cl_event e, cl_int status, void *data) {

  float *float_data = (float *)data;
  printf("New data: %4.2f, %4.2f, %4.2f, %4.2f\n", float_data[0], float_data[1], float_data[2], float_data[3]);
}

int main(void) {

  /* Data and events */

  /* Initialize data */
  float data[4];
  for (int i = 0; i < 4; i++) {
    data[i] = i * 1.0;
  }

  // clang-format off
  /* Create a device and context */
  cl_device_id device  = create_device();
  cl_context   context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                        handleError("Couldn't create a context.");
  cl_program   program = build_program(context, device, PROGRAM_FILE);
  cl_kernel    kernel  = clCreateKernel(program, KERNEL_FUNC, &err);                                                 handleError("Couldn't create a kernel.");

  cl_mem data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data), data, &err);  handleError("Couldn't create a buffer");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer); handleError("Couldn't set a kernel argument");

  cl_queue_properties queue_properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, queue_properties, &err);              handleError("Couldn't create a command queue.");

  cl_event user_event;
  user_event = clCreateUserEvent(context, &err);                                                                     handleError("Couldn't enqueue the kernel");

  cl_event kernel_event, read_event;
  const size_t gws[1] = {1};
  const size_t lws[1] = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, gws, lws, 1, &user_event, &kernel_event);                     handleError("Couldn't enqueue the kernel.");
  err = clEnqueueReadBuffer(queue, data_buffer, CL_FALSE, 0, sizeof(data), data, 1, &kernel_event, &read_event);     handleError("Couldn't read the buffer");
  err = clSetEventCallback(read_event, CL_COMPLETE, &read_complete, data);                                           handleError("Couldn't set callback for event");

  // clang-format on
  /* Sleep for a second to demonstrate that commands haven't started executing. Then prompt user */
  sleep(1);
  printf("Old data: %4.2f, %4.2f, %4.2f, %4.2f\n", data[0], data[1], data[2], data[3]);
  printf("Press ENTER to continue.\n");
  getchar();

  err = clSetUserEventStatus(user_event, CL_SUCCESS);
  handleError("Couldn't set user event status.");

  clReleaseEvent(read_event);
  clReleaseEvent(kernel_event);
  clReleaseEvent(user_event);
  clReleaseMemObject(data_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
