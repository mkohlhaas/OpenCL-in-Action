#include "mmio.h"
#include <CL/cl.h>
#include <stdio.h>
#include <time.h>

#define PROGRAM_FILE "conj_grad.cl"
#define KERNEL_FUNC "conj_grad"
#define MM_FILE "bcsstk05.mtx"

/* Rearrange data to be sorted by row instead of by column */
cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

void sort(int num, int *rows, int *cols, float *values) {
  int index = 0;
  for (int i = 0; i < num; i++) {
    for (int j = index; j < num; j++) {
      if (rows[j] == i) {
        if (j == index) {
          index++;
        }

        /* Swap row/column/values as necessary */
        else if (j > index) {
          int int_swap = rows[index];
          rows[index] = rows[j];
          rows[j] = int_swap;

          int_swap = cols[index];
          cols[index] = cols[j];
          cols[j] = int_swap;

          float float_swap = values[index];
          values[index] = values[j];
          values[j] = float_swap;
          index++;
        }
      }
    }
  }
}

FILE *openMatrixFile(char *matrix_file) {
  FILE *mm_handle = fopen(matrix_file, "r");
  if (!mm_handle) {
    perror("Couldn't open the MatrixMarket file.");
    exit(EXIT_FAILURE);
  }
  return mm_handle;
}

cl_program build_program(cl_context ctx, cl_device_id device, const char *filename) {
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
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *program_log = (char *)malloc(log_size);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }
  return program;
}

int main() {
  double value_double;

  /* Read sparse file */
  FILE *mm_handle = openMatrixFile(MM_FILE);
  MM_typecode code;
  mm_read_banner(mm_handle, &code);
  int num_rows, num_cols, num_values;
  mm_read_mtx_crd_size(mm_handle, &num_rows, &num_cols, &num_values);

  /* Check for symmetry and allocate memory */
  if (mm_is_symmetric(code) || mm_is_skew(code) || mm_is_hermitian(code)) {
    num_values += num_values - num_rows;
  }
  int *rows = malloc(num_values * sizeof(int));
  int *cols = malloc(num_values * sizeof(int));
  float *values = malloc(num_values * sizeof(float));
  float *b_vec = malloc(num_rows * sizeof(float));

  /* Read matrix data and close file */
  int i = 0;
  while (i < num_values) {
    fscanf(mm_handle, "%d %d %f\n", &rows[i], &cols[i], &values[i]);
    cols[i]--;
    rows[i]--;
    if ((rows[i] != cols[i]) && (mm_is_symmetric(code) || mm_is_skew(code) || mm_is_hermitian(code))) {
      i++;
      rows[i] = cols[i - 1];
      cols[i] = rows[i - 1];
      values[i] = values[i - 1];
    }
    i++;
  }
  sort(num_values, rows, cols, values);
  fclose(mm_handle);

  srand(time(0));
  for (i = 0; i < num_rows; i++) {
    b_vec[i] = (float)(rand() - RAND_MAX / 2);
  }

  // clang-format off
  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);                                                                                          handleError("Couldn't identify a platform.");

  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);                                                                handleError("Couldn't access any devices.");

  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                            handleError("Couldn't create a context.");
  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel  kernel  = clCreateKernel(program, KERNEL_FUNC, &err);                                                                     handleError("Couldn't create a kernel.");

  cl_mem rows_buffer   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, num_values * sizeof(int), rows, &err);                                                                            handleError("Couldn't create rows buffer.");
  cl_mem cols_buffer   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, num_values * sizeof(int), cols, &err);                                                                            handleError("Couldn't create cols buffer.");
  cl_mem values_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, num_values * sizeof(float), values, &err);                                                                            handleError("Couldn't create values buffer.");
  cl_mem b_buffer      = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, num_values * sizeof(float), b_vec, &err);                                                                            handleError("Couldn't create b buffer.");
  cl_mem result_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 2 * sizeof(float), NULL, NULL);                                    handleError("Couldn't create result buffer.");

  /* Create kernel argument */
  err  = clSetKernelArg(kernel, 0, sizeof(num_rows), &num_rows);
  err |= clSetKernelArg(kernel, 1, sizeof(num_values), &num_values);
  err |= clSetKernelArg(kernel, 2, num_rows * sizeof(float), NULL);
  err |= clSetKernelArg(kernel, 3, num_rows * sizeof(float), NULL);
  err |= clSetKernelArg(kernel, 4, num_rows * sizeof(float), NULL);
  err |= clSetKernelArg(kernel, 5, num_rows * sizeof(float), NULL);
  err |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &rows_buffer);
  err |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &cols_buffer);
  err |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &values_buffer);
  err |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &b_buffer);
  err |= clSetKernelArg(kernel, 10, sizeof(cl_mem), &result_buffer);                                                                   handleError("Couldn't set a kernel argument.");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                                            handleError("Couldn't create a command queue.");

  size_t global_size = num_rows;
  size_t local_size  = num_rows;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);                                      handleError("Couldn't enqueue the kernel.");

  /* Read the results */
  float result[2];
  err = clEnqueueReadBuffer(queue, result_buffer, CL_BLOCKING, 0, 2 * sizeof(float), result, 0, NULL, NULL);                           handleError("Couldn't read the buffer");
  // clang-format on

  printf("After %d iterations, the residual length is %f.\n", (int)result[0], result[1]);

  free(b_vec);
  free(rows);
  free(cols);
  free(values);
  clReleaseMemObject(b_buffer);
  clReleaseMemObject(rows_buffer);
  clReleaseMemObject(cols_buffer);
  clReleaseMemObject(values_buffer);
  clReleaseMemObject(result_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
