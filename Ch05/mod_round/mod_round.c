#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_FILE "mod_round.cl"
#define KERNEL_FUNC "mod_round"

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
  if (err) {
    perror("Couldn't access any devices");
    exit(EXIT_FAILURE);
  }

  return device;
}

cl_program build_program(cl_context context, cl_device_id device, const char *filename) {
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

  cl_program program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);
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
  // clang-format off
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                                 handleError("Couldn't create a context");

  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);                                                                            handleError("Couldn't create a kernel");

  float mod_input[2] = {317.0f, 23.0f};
  float mod_output[2];
  float round_input[4] = {-6.5f, -3.5f, 3.5f, 6.5f};
  float round_output[20];
  cl_mem mod_input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(mod_input), mod_input, &err);           handleError("Couldn't create a buffer");

  cl_mem mod_output_buffer   = clCreateBuffer(context, CL_MEM_WRITE_ONLY,                       sizeof(mod_output),   NULL,        &err);   handleError("Couldn't create a buffer");
  cl_mem round_input_buffer  = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(round_input),  round_input, &err);   handleError("Couldn't create a buffer");
  cl_mem round_output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,                       sizeof(round_output), NULL,        &err);   handleError("Couldn't create a buffer");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mod_input_buffer);                                                                       handleError("Couldn't set a kernel argument");
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &mod_output_buffer);                                                                      handleError("Couldn't set a kernel argument");
  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &round_input_buffer);                                                                     handleError("Couldn't set a kernel argument");
  err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &round_output_buffer);                                                                    handleError("Couldn't set a kernel argument");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                                                 handleError("Couldn't create a command queue");

  const size_t global_work_size[1] = {1};
  const size_t local_work_size[1]  = {1};
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);                                   handleError("Couldn't enqueue the kernel");

  err = clEnqueueReadBuffer(queue, mod_output_buffer,   CL_BLOCKING, 0, sizeof(mod_output),   &mod_output,   0, NULL, NULL);                handleError("Couldn't read the buffer");
  err = clEnqueueReadBuffer(queue, round_output_buffer, CL_BLOCKING, 0, sizeof(round_output), &round_output, 0, NULL, NULL);                handleError("Couldn't read the buffer");

  printf("fmod(%.1f, %.1f)      = %.1f\n",   mod_input[0], mod_input[1], mod_output[0]);
  printf("remainder(%.1f, %.1f) = %.1f\n\n", mod_input[0], mod_input[1], mod_output[1]);

  printf("Input: %.1f  %.1f  %.1f  %.1f\n", round_input[0],   round_input[1],   round_input[2],   round_input[3]);
  printf("rint:  %.1f  %.1f  %.1f  %.1f\n", round_output[0],  round_output[1],  round_output[2],  round_output[3]);
  printf("round: %.1f  %.1f  %.1f  %.1f\n", round_output[4],  round_output[5],  round_output[6],  round_output[7]);
  printf("ceil:  %.1f  %.1f  %.1f  %.1f\n", round_output[8],  round_output[9],  round_output[10], round_output[11]);
  printf("floor: %.1f  %.1f  %.1f  %.1f\n", round_output[12], round_output[13], round_output[14], round_output[15]);
  printf("trunc: %.1f  %.1f  %.1f  %.1f\n", round_output[16], round_output[17], round_output[18], round_output[19]);
  // clang-format on

  clReleaseMemObject(mod_input_buffer);
  clReleaseMemObject(mod_output_buffer);
  clReleaseMemObject(round_input_buffer);
  clReleaseMemObject(round_output_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
