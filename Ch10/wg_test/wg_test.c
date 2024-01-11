#include <CL/cl.h>
#include <stdio.h>

#define PROGRAM_FILE "blank.cl"
#define KERNEL_FUNC "blank"

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

int main(int argc, char **argv) {

  // clang-format off
  char *program_name, *kernel_func;
  switch (argc) {
  case 1:
    program_name = PROGRAM_FILE;
    kernel_func  = KERNEL_FUNC;
    break;
  case 3:
    program_name = argv[1];
    kernel_func  = argv[2];
    break;
  default:
    printf("Usage: wg_test <program_file> <kernel_func>\n");
    exit(EXIT_FAILURE);
    break;
  }

  cl_device_id device = create_device();

  char device_name[48];
  cl_ulong local_mem;
  err  = clGetDeviceInfo(device, CL_DEVICE_NAME,           sizeof(device_name), device_name, NULL);                   handleError("Couldn't obtain device information.");
  err |= clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem), &local_mem, NULL);                      handleError("Couldn't obtain device information.");

  cl_context       context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                     handleError("Couldn't create a context.");
  cl_program       program = build_program(context, device, program_name);
  cl_command_queue queue   = clCreateCommandQueueWithProperties(context, device, NULL, &err);                         handleError("Couldn't create a command queue.");
  cl_kernel        kernel  = clCreateKernel(program, kernel_func, &err);                                              handleError("Couldn't create the kernel.");

  size_t wg_size, wg_multiple;
  cl_ulong local_usage, private_usage;
  err  = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE,                    sizeof(wg_size),       &wg_size,       NULL);
  err |= clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(wg_multiple),   &wg_multiple,   NULL);
  err |= clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_LOCAL_MEM_SIZE,                     sizeof(local_usage),   &local_usage,   NULL);
  err |= clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_PRIVATE_MEM_SIZE,                   sizeof(private_usage), &private_usage, NULL);
  handleError("Couldn't obtain kernel work-group size information");

  printf("\"%s\" kernel on %s:\n  maximum work-group size: %zu\n  work-group multiple: %zu\n", kernel_func, device_name, wg_size, wg_multiple);
  printf("  local usage: %zu\n  local memory: %zu\n  private memory: %zu\n",    local_usage, local_mem,   private_usage);

  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
