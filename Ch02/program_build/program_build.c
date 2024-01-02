#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define NUM_FILES 2
#define PROGRAM_FILE_1 "good.cl"
#define PROGRAM_FILE_2 "bad.cl"

cl_int err; // OpenCL errors

void handleError(char *message) {
  if (err) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

void printProgramLog(cl_program program, cl_device_id device) {
  /* Find size of log and print to std output */
  size_t log_size;
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  char *program_log = (char *)malloc(log_size + 1);
  program_log[log_size] = '\0';
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
  printf("%s\n", program_log);
  free(program_log);
  exit(EXIT_FAILURE);
}

int main(void) {

  /* Access the first installed platform */
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't find any platforms");

  /* Access the first GPU/CPU */
  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  handleError("Couldn't find any devices");

  /* Create a context */
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Read each program file and place content into buffer array */
  char *program_buffer[NUM_FILES];
  const char *file_name[] = {PROGRAM_FILE_1, PROGRAM_FILE_2};
  size_t program_size[NUM_FILES];
  for (cl_int i = 0; i < NUM_FILES; i++) {
    FILE *program_handle = fopen(file_name[i], "r");
    err = !program_handle;
    handleError("Couldn't find the program file");
    fseek(program_handle, 0, SEEK_END);
    program_size[i] = ftell(program_handle);
    rewind(program_handle);
    program_buffer[i] = (char *)malloc(program_size[i] + 1);
    program_buffer[i][program_size[i]] = '\0';
    fread(program_buffer[i], sizeof(char), program_size[i], program_handle);
    fclose(program_handle);
  }

  /* Create a program containing all program content */
  cl_program program = clCreateProgramWithSource(context, NUM_FILES, (const char **)program_buffer, program_size, &err);
  handleError("Couldn't create the program");

  /* Build program */
  const char options[] = "-cl-finite-math-only -cl-no-signed-zeros";
  err = clBuildProgram(program, 1, &device, options, NULL, NULL);
  if (err < 0) {
    printProgramLog(program, device);
  }

  /* Deallocate resources */
  for (cl_int i = 0; i < NUM_FILES; i++) {
    free(program_buffer[i]);
  }
  clReleaseProgram(program);
  clReleaseContext(context);
}
