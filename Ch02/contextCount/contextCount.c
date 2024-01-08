#include "aux.h"
#include <CL/cl.h>
#include <stdio.h>

int main(void) {

  // set up OpenCL
  cl_platform_id platform = getFirstPlatform();
  cl_device_id device = getFirstGPUDevice(platform);
  cl_context context = createContext(device);

  printf("Initial reference count: %u\n", getReferenceCount(context)); // 1
  retainContext(context);
  printf("Reference count: %u\n", getReferenceCount(context)); // 2
  releaseContext(context);
  printf("Reference count: %u\n", getReferenceCount(context)); // 1
  // results in segmentation fault as context does not exist anymore
  releaseContext(context);
  releaseResources(context, device);
}
