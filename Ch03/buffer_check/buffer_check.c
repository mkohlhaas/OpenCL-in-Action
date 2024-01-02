#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

cl_int err;

void handleError(char *message) {
  if (err) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {

  /* Identify a platform */
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't find any platforms");

  /* Access a device */
  cl_device_id dev;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
  }
  handleError("Couldn't find any devices");

  return dev;
}

int main(void) {

  /* Create device and context */
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Create a buffer to hold 100 floating-point values */
  float main_data[100];
  cl_mem main_buffer =
      clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(main_data), main_data, &err);
  handleError("Couldn't create a buffer");

  cl_buffer_region region;
  region.origin = 0x100;
  region.size = 20 * sizeof(float);
  cl_mem sub_buffer = clCreateSubBuffer(main_buffer, CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region, &err);
  printf("%d\n", err);
  handleError("Couldn't create a sub-buffer");

  /* Obtain size information about the buffers */
  size_t main_buffer_size;
  size_t sub_buffer_size;
  clGetMemObjectInfo(main_buffer, CL_MEM_SIZE, sizeof(main_buffer_size), &main_buffer_size, NULL);
  clGetMemObjectInfo(sub_buffer, CL_MEM_SIZE, sizeof(sub_buffer_size), &sub_buffer_size, NULL);
  printf("Main buffer size: %lu\n", main_buffer_size);
  printf("Sub-buffer size:  %lu\n", sub_buffer_size);

  /* Obtain the host pointers */
  void *main_buffer_mem;
  void *sub_buffer_mem;
  clGetMemObjectInfo(main_buffer, CL_MEM_HOST_PTR, sizeof(main_buffer_mem), &main_buffer_mem, NULL);
  clGetMemObjectInfo(sub_buffer, CL_MEM_HOST_PTR, sizeof(sub_buffer_mem), &sub_buffer_mem, NULL);
  printf("Main buffer memory address: %p\n", main_buffer_mem);
  printf("Sub-buffer memory address:  %p\n", sub_buffer_mem);

  /* Print the address of the main data */
  printf("Main array address: %p\n", main_data);

  /* Deallocate resources */
  clReleaseMemObject(main_buffer);
  clReleaseMemObject(sub_buffer);
  clReleaseContext(context);
}
