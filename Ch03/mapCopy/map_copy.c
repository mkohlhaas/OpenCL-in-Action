#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't find any platforms");

  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  handleError("Couldn't find any devices");

  return device;
}

int main(void) {

  float data_one[100], data_two[100], result_array[100];

  for (int i = 0; i < 100; i++) {
    data_one[i] = 1.0f * i;
    data_two[i] = -1.0f * i;
    result_array[i] = 0.0f;
  }

  /* Create a device and context */
  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context");

  /* Create buffers */
  cl_mem buffer_one = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data_one), data_one, &err);
  handleError("Couldn't create a buffer one object");
  cl_mem buffer_two = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data_two), data_two, &err);
  handleError("Couldn't create a buffer two object");

  /* Create a command queue */
  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
  handleError("Couldn't create a command queue");

  /* Enqueue command to copy buffer one to buffer two */
  err = clEnqueueCopyBuffer(queue, buffer_one, buffer_two, 0, 0, sizeof(data_one), 0, NULL, NULL);
  handleError("Couldn't perform the buffer copy");

  /* Enqueue command to map buffer two to host memory */
  void *mapped_memory = clEnqueueMapBuffer(queue, buffer_two, CL_BLOCKING, CL_MAP_READ, 0, sizeof(data_two), 0, NULL, NULL, &err);
  handleError("Couldn't map the buffer to host memory");

  /* Transfer memory and unmap the buffer */
  memcpy(result_array, mapped_memory, sizeof(data_two));
  err = clEnqueueUnmapMemObject(queue, buffer_two, mapped_memory, 0, NULL, NULL);
  handleError("Couldn't unmap the buffer");

  /* Display updated buffer */
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      printf("%6.1f", result_array[j + i * 10]);
    }
    printf("\n");
  }

  /* Deallocate resources */
  clReleaseMemObject(buffer_one);
  clReleaseMemObject(buffer_two);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);
}
