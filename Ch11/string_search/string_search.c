#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_FILE "string_search.cl"
#define KERNEL_FUNC "string_search"
#define TEXT_FILE "kafka.txt"

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
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

  // clang-format off

  cl_platform_id platform;
  cl_device_id   device;
  err = clGetPlatformIDs(1, &platform, NULL);                                                                               handleError("Couldn't identify a platform.");
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);                                                     handleError("Couldn't access any devices.");

  size_t global_size;
  size_t local_size;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,   sizeof(global_size), &global_size, NULL);                    handleError("Couldn't get device info.");
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(local_size),  &local_size,  NULL);                    handleError("Couldn't get device info.");
  fprintf(stderr, "Device:\n");
  fprintf(stderr, "  compute units: %zu\n", global_size);
  global_size *= local_size;
  fprintf(stderr, "  local size:    %zu\n  global size:   %zu\n\n", local_size, global_size);

  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                 handleError("Couldn't create a context.");

  /* read program file and place content into buffer */
  FILE *program_handle = fopen(PROGRAM_FILE, "r");
  if (!program_handle) {
    fprintf(stderr, "Couldn't find the program file.");
    exit(EXIT_FAILURE);
  }
  fseek(program_handle, 0, SEEK_END);
  size_t program_size = ftell(program_handle);
  rewind(program_handle);
  char *program_buffer = (char *)calloc(program_size, sizeof(char));
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  /* read text file and place content into buffer */
  FILE *text_handle = fopen(TEXT_FILE, "r");
  if (!text_handle) {
    fprintf(stderr, "Couldn't find the text file.");
    exit(EXIT_FAILURE);
  }
  fseek(text_handle, 0, SEEK_END);
  size_t text_size = ftell(text_handle) - 1;
  rewind(text_handle);
  char *text = (char *)calloc(text_size, sizeof(char));
  fread(text, sizeof(char), text_size, text_handle);
  fclose(text_handle);
  int chars_per_work_item = text_size / global_size + 1;
  fprintf(stderr, "Text:\n");
  fprintf(stderr, "  text size:           %zu\n",  text_size);
  fprintf(stderr, "  chars per work item: %d\n\n", chars_per_work_item);

  cl_program program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);          handleError("Couldn't create the program.");
  free(program_buffer);

  /* build program and print build log */
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  size_t log_size;
  err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);                                   handleError("Couldn't get build info.");
  char *program_log = (char *)calloc(log_size, sizeof(char));
  err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);                          handleError("Couldn't get build info.");
  fprintf(stderr, "Build info:\n%s\n", program_log);
  free(program_log);
  if (err) {
    exit(EXIT_FAILURE);
  }

  int result[4] = {0, 0, 0, 0};
  cl_kernel kernel     = clCreateKernel(program, KERNEL_FUNC, &err);                                                        handleError("Couldn't create a kernel.");
  cl_mem text_buffer   = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, text_size, text, &err);          handleError("Couldn't create a buffer.");
  cl_mem result_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(result), result, &err);   handleError("Couldn't create a buffer.");

  char pattern[16] = "thatwithhavefrom";
  err  = clSetKernelArg(kernel, 0, sizeof(pattern),             pattern);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem),              &text_buffer);
  err |= clSetKernelArg(kernel, 2, sizeof(chars_per_work_item), &chars_per_work_item);
  err |= clSetKernelArg(kernel, 3, 4 * sizeof(int),             NULL);
  err |= clSetKernelArg(kernel, 4, sizeof(cl_mem),              &result_buffer);                                            handleError("Couldn't set kernel arguments.");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                                 handleError("Couldn't create a command queue.");

  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);                           handleError("Couldn't enqueue the kernel.");
  err = clEnqueueReadBuffer(queue, result_buffer, CL_BLOCKING, 0, sizeof(result), &result, 0, NULL, NULL);                  handleError("Couldn't read the buffer.");

  fprintf(stderr, "Results:\n");
  fprintf(stderr, "  Occurrences of `that`: %d\n", result[0]);
  fprintf(stderr, "  Occurrences of `with`: %d\n", result[1]);
  fprintf(stderr, "  Occurrences of `have`: %d\n", result[2]);
  fprintf(stderr, "  Occurrences of `from`: %d\n", result[3]);

  clReleaseMemObject(result_buffer);
  clReleaseMemObject(text_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
