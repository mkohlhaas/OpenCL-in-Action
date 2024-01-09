#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

int main(void) {

  /* Identify a platform */
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't find any platforms.");

  /* Access a device */
  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  handleError("Couldn't access any devices.");

  /* Obtain the device data */
  cl_uint vector_width;
  err = clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(vector_width), &vector_width, NULL);
  handleError("Couldn't read device properties.");

  printf("Preferred vector width in chars: %u\n", vector_width);
  // clang-format off
  clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,  sizeof(vector_width), &vector_width, NULL);
  printf("Preferred vector width in shorts: %u.\n",  vector_width);
  clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,    sizeof(vector_width), &vector_width, NULL);
  printf("Preferred vector width in ints: %u.\n",    vector_width);
  clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,   sizeof(vector_width), &vector_width, NULL);
  printf("Preferred vector width in longs: %u.\n",   vector_width);
  clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,  sizeof(vector_width), &vector_width, NULL);
  printf("Preferred vector width in floats: %u.\n",  vector_width);
  clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(vector_width), &vector_width, NULL);
  printf("Preferred vector width in doubles: %u.\n", vector_width);
  clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,   sizeof(vector_width), &vector_width, NULL);
  printf("Preferred vector width in halfs: %u.\n",   vector_width);
  // clang-format on
}
