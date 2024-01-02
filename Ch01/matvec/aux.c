#define PROGRAM_FILE "matvec.cl"
#define KERNEL_FUNC "matvec_mult"

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

cl_kernel createKernel(cl_program program) {
  cl_int err;
  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError(err, "Couldn't create the kernel.");
  return kernel;
}

cl_command_queue createCommandQueue(cl_context context, cl_device_id device) {
  cl_int err;
  cl_command_queue cmdQueue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  handleError(err, "Couldn't create the command queue.");
  return cmdQueue;
}

cl_mem createInputBuffer(cl_context context, float *mem, size_t size) {
  cl_int err;
  cl_mem_flags memFlags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;
  size_t memSize = sizeof(float) * size;
  cl_mem matrixBuf = clCreateBuffer(context, memFlags, memSize, mem, &err);
  handleError(err, "Couldn't create buffer object.");
  return matrixBuf;
}

cl_mem createMatrixBuffer(cl_context context, float *mat) { return createInputBuffer(context, mat, 16); }

cl_mem createVectorBuffer(cl_context context, float *vec) { return createInputBuffer(context, vec, 4); }

cl_mem createResultBuffer(cl_context context) {
  cl_int err;
  cl_mem_flags memFlags = CL_MEM_WRITE_ONLY;
  size_t memSize = sizeof(float) * 4;
  cl_mem resultBuf = clCreateBuffer(context, memFlags, memSize, NULL, &err);
  handleError(err, "Couldn't create result buffer object.");
  return resultBuf;
}

void execKernel(cl_kernel kernel, cl_command_queue cmdQueue, cl_mem matrixBuf, cl_mem vectorBuf, cl_mem resultBuf) {
  cl_int err;

  // set arguments
  // clang-format off
  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &matrixBuf);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &vectorBuf);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &resultBuf);
  handleError(err, "Couldn't set kernel arguments.");
  // clang-format on

  // enqueue kernel
  size_t globalWorkSize = 4;
  err = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);
  handleError(err, "Couldn't enqueue the kernel execution command.");
}

void readResult(cl_command_queue cmdQueue, cl_mem resultBuf, float *result) {
  cl_int err = clEnqueueReadBuffer(cmdQueue, resultBuf, CL_BLOCKING, 0, sizeof(float) * 4, result, 0, NULL, NULL);
  handleError(err, "Couldn't enqueue the read buffer command.");
}

// clang-format off
void testResult(float *mat, float *vec, float *result) {
  float expected[] = {0, 0, 0, 0};
  for (int i = 0; i < 4; i++) {
    expected[0] += mat[i +  0] * vec[i]; // 84.000000
    expected[1] += mat[i +  4] * vec[i]; // 228.000000
    expected[2] += mat[i +  8] * vec[i]; // 372.000000
    expected[3] += mat[i + 12] * vec[i]; // 516.000000
  }

  if (result[0] == expected[0] &&
      result[1] == expected[1] &&
      result[2] == expected[2] &&
      result[3] == expected[3])
  {
    printf("Matrix-vector multiplication successful!\n");
  } else {
    printf("Matrix-vector multiplication NOT successful!\n");
  }
}

// clang-format on
void releaseResources(cl_device_id device, cl_context context, cl_program program, cl_kernel kernel,
                      cl_command_queue cmdQueue, cl_mem matrixBuf, cl_mem vectorBuf, cl_mem resultBuf) {
  clReleaseMemObject(matrixBuf);
  clReleaseMemObject(vectorBuf);
  clReleaseMemObject(resultBuf);
  clReleaseCommandQueue(cmdQueue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(context);
  clReleaseDevice(device);
}
