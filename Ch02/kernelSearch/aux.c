#define PROGRAM_FILE "test.cl"

#include <CL/cl.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

void handleError(cl_int err, char *message) {
  if (err) {
    fprintf(stderr, "%s\nError: %d\n", message, err);
    exit(EXIT_FAILURE);
  }
}

cl_platform_id getFirstPlatform() {
  cl_platform_id platform;
  cl_int err = clGetPlatformIDs(1, &platform, NULL);
  handleError(err, "Couldn't find any platforms.");
  return platform;
}

cl_device_id getFirstGPUDevice(cl_platform_id platform) {
  cl_device_id device;
  cl_int err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  handleError(err, "Couldn't find any devices.");
  return device;
}

cl_context createContext(cl_device_id device) {
  cl_int err;
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError(err, "Couldn't create a context.");
  return context;
}

void printProgramBuildLog(cl_program program, cl_device_id device) {

  // get size of build log
  size_t logSize;
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);

  // load build log
  GString *programLog = g_string_sized_new(logSize);
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, programLog, NULL);

  // print build log
  printf("%s\n", (char *)programLog);
  g_free(programLog);
  exit(EXIT_FAILURE);
}

cl_program createProgram(cl_context context, cl_device_id device) { // device necessary for build log
  cl_int err;

  // load program file
  gchar *contents = NULL;
  gsize length;
  gboolean progLoaded = g_file_get_contents(PROGRAM_FILE, &contents, &length, NULL);
  handleError(!progLoaded, "Couldn't open program file.");

  // create program from source
  cl_program program = clCreateProgramWithSource(context, 1, (const char **)&contents, &length, &err);
  handleError(err, "Couldn't create the program.");
  g_free(contents);

  // build program
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err) {
    printProgramBuildLog(program, device);
  }
  return program;
}

cl_kernel *createAllKernels(cl_program program, cl_uint *numKernels) {
  cl_int err = clCreateKernelsInProgram(program, 0, NULL, numKernels);
  handleError(err, "Couldn't find any kernels");

  // create all kernels in program
  cl_kernel *kernels = (cl_kernel *)malloc(sizeof(cl_kernel) * (*numKernels));
  err = clCreateKernelsInProgram(program, *numKernels, kernels, NULL);
  handleError(err, "Couldn't create kernel.");
  return kernels;
}

// returns index of found kernel or -1 if not found
int findNamedKernel(cl_kernel *kernels, cl_uint numKernels, char *searchName) {
  char kernelName[20];
  for (int i = 0; i < numKernels; i++) {
    cl_int err = clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, sizeof(kernelName), kernelName, NULL);
    handleError(err, "Couldn't get kernel function name.");
    if (!strcmp(kernelName, searchName)) {
      return i;
    }
  }
  return -1;
}

void releaseResources(cl_context context, cl_device_id device, cl_program program, cl_kernel *kernels, cl_uint numKernels) {
  for (int i = 0; i < numKernels; i++) {
    clReleaseKernel(kernels[i]);
  }
  free(kernels);
  clReleaseProgram(program);
  clReleaseDevice(device);
  clReleaseContext(context);
}
