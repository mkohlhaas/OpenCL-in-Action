#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

cl_int err;

void handleError(char *message) {
  if (err) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

int main(void) {

  /* Identify a platform */
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't find any platforms");

  /* Access a device */
  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  handleError("Couldn't access any devices");

  /* Check float-processing features */
  cl_device_fp_config flag;
  err = clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(flag), &flag, NULL);
  handleError("Couldn't read floating-point properties");

  printf("Float Processing Features:\n");
  if (flag & CL_FP_INF_NAN)
    printf("INF and NaN values supported.\n");
  if (flag & CL_FP_DENORM)
    printf("Denormalized numbers supported.\n");
  if (flag & CL_FP_ROUND_TO_NEAREST)
    printf("Round To Nearest Even mode supported.\n");
  if (flag & CL_FP_ROUND_TO_INF)
    printf("Round To Infinity mode supported.\n");
  if (flag & CL_FP_ROUND_TO_ZERO)
    printf("Round To Zero mode supported.\n");
  if (flag & CL_FP_FMA)
    printf("Floating-point multiply-and-add operation supported.\n");
  if (flag & CL_FP_SOFT_FLOAT)
    printf("Basic floating-point processing performed in software.\n");
}
