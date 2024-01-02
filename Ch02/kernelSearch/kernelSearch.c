#include "aux.h"
#include <stdio.h>

int main(void) {

  // set up OpenCL
  cl_platform_id platform = getFirstPlatform();
  cl_device_id device = getFirstGPUDevice(platform);
  cl_context context = createContext(device);
  cl_program program = createProgram(context, device);
  cl_uint numKernels;
  cl_kernel *kernels = createAllKernels(program, &numKernels);

  printf("Number of kernels: %d.\n", numKernels);
  int index = findNamedKernel(kernels, numKernels, "mult");
  if (index < 0)
    printf("Kernel not found.\n");
  else
    printf("Found kernel at index %d.\n", index);
  releaseResources(context, device, program, kernels, numKernels);
}
