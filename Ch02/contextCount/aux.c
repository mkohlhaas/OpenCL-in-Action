#include <CL/cl.h>
#include <stdio.h>

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

cl_uint getReferenceCount(cl_context context) {
  cl_uint refCount;
  cl_int err = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(refCount), &refCount, NULL);
  handleError(err, "Couldn't read the reference count.");
  return refCount;
}

void releaseResources(cl_context context, cl_device_id device) {
  clReleaseDevice(device);
  clReleaseContext(context);
}

void retainContext(cl_context context) {
  cl_int err = clRetainContext(context);
  handleError(err, "Couldn't retain the reference count.");
}

void releaseContext(cl_context context) {
  cl_int err = clReleaseContext(context);
  handleError(err, "Couldn't release context.");
}
