#include "aux.h"
#include <CL/cl.h>

int main(void) {

  // set up OpenCL
  cl_platform_id platform = getFirstPlatform();
  cl_device_id device = getFirstGPUDevice(platform);
  cl_context context = createContext(device);
  cl_program program = createProgram(context, device);
  cl_kernel kernel = createKernel(program);
  cl_command_queue cmdQueue = createCommandQueue(context, device);

  // input data
  float matrix[16];
  for (int i = 0; i < 16; i++) {
    matrix[i] = i * 2.0f;
  }

  float vector[4];
  for (int i = 0; i < 4; i++) {
    vector[i] = i * 3.0f;
  }

  // create memory objects and write to device
  cl_mem matrixBuf = createMatrixBuffer(context, matrix);
  cl_mem vectorBuf = createVectorBuffer(context, vector);
  cl_mem resultBuf = createResultBuffer(context);

  execKernel(kernel, cmdQueue, matrixBuf, vectorBuf, resultBuf);

  // read from device
  float result[4];
  readResult(cmdQueue, resultBuf, result);

  testResult(matrix, vector, result);

  releaseResources(device, context, program, kernel, cmdQueue, matrixBuf, vectorBuf, resultBuf);
}
