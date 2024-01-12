#include <CL/cl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define PROGRAM_FILE "bsort.cl"
#define BSORT_INIT "bsort_init"
#define BSORT_STAGE_0 "bsort_stage_0"
#define BSORT_STAGE_N "bsort_stage_n"
#define BSORT_MERGE "bsort_merge"
#define BSORT_MERGE_LAST "bsort_merge_last"
/* Ascending: 0, Descending: -1 */
#define DIRECTION 0
#define NUM_FLOATS 1048576

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

cl_device_id create_device() {
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't identify a platform");

  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  handleError("Couldn't access any devices");

  return device;
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename) {
  FILE *program_handle = fopen(filename, "r");
  if (program_handle == NULL) {
    perror("Couldn't find the program file");
    exit(EXIT_FAILURE);
  }
  fseek(program_handle, 0, SEEK_END);
  size_t program_size = ftell(program_handle);
  rewind(program_handle);
  char *program_buffer = (char *)malloc(program_size);
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  cl_program program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program");
  free(program_buffer);

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err) {
    size_t log_size;
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *program_log = (char *)malloc(log_size);
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }

  return program;
}

int main(void) {

  /* Initialize data */
  float data[NUM_FLOATS];
  srand(time(NULL));
  for (int i = 0; i < NUM_FLOATS; i++) {
    data[i] = rand();
  }

  cl_device_id device = create_device();
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleError("Couldn't create a context.");

  cl_program program = build_program(context, device, PROGRAM_FILE);

  /* Create kernels */
  cl_kernel kernel_init = clCreateKernel(program, BSORT_INIT, &err);
  if (err < 0) {
    perror("Couldn't create the initial kernel");
    exit(1);
  };
  cl_kernel kernel_stage_0 = clCreateKernel(program, BSORT_STAGE_0, &err);
  if (err < 0) {
    perror("Couldn't create the stage_0 kernel");
    exit(1);
  };
  cl_kernel kernel_stage_n = clCreateKernel(program, BSORT_STAGE_N, &err);
  if (err < 0) {
    perror("Couldn't create the stage_n kernel");
    exit(1);
  };
  cl_kernel kernel_merge = clCreateKernel(program, BSORT_MERGE, &err);
  if (err < 0) {
    perror("Couldn't create the merge kernel");
    exit(1);
  };
  cl_kernel kernel_merge_last = clCreateKernel(program, BSORT_MERGE_LAST, &err);
  if (err < 0) {
    perror("Couldn't create the merge_last kernel");
    exit(1);
  };

  /* Determine maximum work-group size */
  size_t local_size;
  err = clGetKernelWorkGroupInfo(kernel_init, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL);
  if (err < 0) {
    perror("Couldn't find the maximum work-group size");
    exit(1);
  };
  local_size = (int)pow(2, trunc(log2(local_size)));

  /* Create buffer */
  cl_mem data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data), data, &err);
  if (err < 0) {
    perror("Couldn't create a buffer");
    exit(1);
  };

  /* Create kernel argument */
  err = clSetKernelArg(kernel_init, 0, sizeof(cl_mem), &data_buffer);
  err |= clSetKernelArg(kernel_stage_0, 0, sizeof(cl_mem), &data_buffer);
  err |= clSetKernelArg(kernel_stage_n, 0, sizeof(cl_mem), &data_buffer);
  err |= clSetKernelArg(kernel_merge, 0, sizeof(cl_mem), &data_buffer);
  err |= clSetKernelArg(kernel_merge_last, 0, sizeof(cl_mem), &data_buffer);
  if (err < 0) {
    printf("Couldn't set a kernel argument");
    exit(1);
  };

  /* Create kernel argument */
  err = clSetKernelArg(kernel_init, 1, 8 * local_size * sizeof(float), NULL);
  err |= clSetKernelArg(kernel_stage_0, 1, 8 * local_size * sizeof(float), NULL);
  err |= clSetKernelArg(kernel_stage_n, 1, 8 * local_size * sizeof(float), NULL);
  err |= clSetKernelArg(kernel_merge, 1, 8 * local_size * sizeof(float), NULL);
  err |= clSetKernelArg(kernel_merge_last, 1, 8 * local_size * sizeof(float), NULL);
  if (err < 0) {
    printf("Couldn't set a kernel argument");
    exit(1);
  };

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  handleError("Couldn't create a command queue");

  /* Enqueue initial sorting kernel */
  size_t global_size = NUM_FLOATS / 8;
  if (global_size < local_size) {
    local_size = global_size;
  }
  err = clEnqueueNDRangeKernel(queue, kernel_init, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
  if (err < 0) {
    perror("Couldn't enqueue the kernel");
    exit(1);
  }

  /* Execute further stages */
  cl_uint num_stages = global_size / local_size;
  for (cl_uint high_stage = 2; high_stage < num_stages; high_stage <<= 1) {

    err = clSetKernelArg(kernel_stage_0, 2, sizeof(int), &high_stage);
    err |= clSetKernelArg(kernel_stage_n, 3, sizeof(int), &high_stage);
    if (err < 0) {
      printf("Couldn't set a kernel argument");
      exit(1);
    };

    for (cl_uint stage = high_stage; stage > 1; stage >>= 1) {

      err = clSetKernelArg(kernel_stage_n, 2, sizeof(int), &stage);
      if (err < 0) {
        printf("Couldn't set a kernel argument");
        exit(1);
      };

      err = clEnqueueNDRangeKernel(queue, kernel_stage_n, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
      if (err < 0) {
        perror("Couldn't enqueue the kernel");
        exit(1);
      }
    }

    err = clEnqueueNDRangeKernel(queue, kernel_stage_0, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    if (err < 0) {
      perror("Couldn't enqueue the kernel");
      exit(1);
    }
  }

  /* Set the sort direction */
  cl_int direction = DIRECTION;
  err = clSetKernelArg(kernel_merge, 3, sizeof(int), &direction);
  err |= clSetKernelArg(kernel_merge_last, 2, sizeof(int), &direction);
  if (err < 0) {
    printf("Couldn't set a kernel argument");
    exit(1);
  };

  /* Perform the bitonic merge */
  for (cl_int stage = num_stages; stage > 1; stage >>= 1) {

    err = clSetKernelArg(kernel_merge, 2, sizeof(int), &stage);
    if (err < 0) {
      printf("Couldn't set a kernel argument");
      exit(1);
    };

    err = clEnqueueNDRangeKernel(queue, kernel_merge, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    if (err < 0) {
      perror("Couldn't enqueue the kernel");
      exit(1);
    }
  }
  err = clEnqueueNDRangeKernel(queue, kernel_merge_last, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
  if (err < 0) {
    perror("Couldn't enqueue the kernel");
    exit(1);
  }

  /* Read the result */
  err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0, sizeof(data), &data, 0, NULL, NULL);
  if (err < 0) {
    perror("Couldn't read the buffer");
    exit(1);
  }

  cl_int check = 1;

  /* Check ascending sort */
  if (direction == 0) {
    for (int i = 1; i < NUM_FLOATS; i++) {
      if (data[i] < data[i - 1]) {
        check = CL_FALSE;
        break;
      }
    }
  }

  /* Check descending sort */
  if (direction == -1) {
    for (int i = 1; i < NUM_FLOATS; i++) {
      if (data[i] > data[i - 1]) {
        check = CL_FALSE;
        break;
      }
    }
  }

  /* Display check result */
  printf("Local size: %zu\n", local_size);
  printf("Global size: %zu\n", global_size);
  if (check) {
    printf("Bitonic sort succeeded.\n");
  } else {
    printf("Bitonic sort failed.\n");
  }

  clReleaseMemObject(data_buffer);
  clReleaseKernel(kernel_init);
  clReleaseKernel(kernel_stage_0);
  clReleaseKernel(kernel_stage_n);
  clReleaseKernel(kernel_merge);
  clReleaseKernel(kernel_merge_last);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
