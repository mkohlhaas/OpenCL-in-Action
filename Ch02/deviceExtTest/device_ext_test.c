#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cl_int err; // OpenCL errors

void handleError(char *message) {
  if (err) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

int main() {

  /* Identify a platform */
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't find any platforms");

  /* Access a device, preferably a GPU */
  /* Changed on 2/12 to fix the CL_INVALID_VALUE error */
  cl_device_id dev;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
  }
  handleError("Couldn't access any devices");

  /* Access device name */
  char name_data[64];
  err = clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(name_data), name_data, NULL);
  handleError("Couldn't read extension data");

  /* Access device address size */
  cl_uint addr_data;
  err = clGetDeviceInfo(dev, CL_DEVICE_ADDRESS_BITS, sizeof(addr_data), &addr_data, NULL);
  handleError("Couldn't get address bits.");

  /* Access device extensions */
  char ext_data[4096];
  err = clGetDeviceInfo(dev, CL_DEVICE_EXTENSIONS, sizeof(ext_data), ext_data, NULL);
  handleError("Couldn't get device extensions.");

  printf("NAME: %s\nADDRESS_WIDTH: %u\nEXTENSIONS: %s\n", name_data, addr_data, ext_data);
}
