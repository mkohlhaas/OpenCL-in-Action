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
  char *kernelName = "mult";
  int index = findNamedKernel(kernels, numKernels, kernelName);
  if (index < 0)
    printf("Kernel %s not found.\n", kernelName);
  else
    printf("Found kernel \"%s\" at index %d.\n", kernelName, index);
  releaseResources(context, device, program, kernels, numKernels);
}
