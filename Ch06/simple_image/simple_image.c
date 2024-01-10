#include <CL/cl.h>
#include <png.h>

#define PROGRAM_FILE "simple_image.cl"
#define KERNEL_FUNC "simple_image"
#define PNG_DEBUG 3
#define INPUT_FILE "blank.png"
#define OUTPUT_FILE "output.png"

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
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(EXIT_FAILURE);
  }

  return program;
}

void read_image_data(const char *filename, png_bytep *data, size_t *w, size_t *h) {

  /* Open input file */
  FILE *png_input;
  if ((png_input = fopen(filename, "rb")) == NULL) {
    perror("Can't read input image file");
    exit(1);
  }

  /* Read image data */
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, png_input);
  png_read_info(png_ptr, info_ptr);
  *w = png_get_image_width(png_ptr, info_ptr);
  *h = png_get_image_height(png_ptr, info_ptr);

  /* Allocate memory and read image data */
  *data = malloc(*h * png_get_rowbytes(png_ptr, info_ptr));
  for (int i = 0; i < *h; i++) {
    png_read_row(png_ptr, *data + i * png_get_rowbytes(png_ptr, info_ptr), NULL);
  }

  /* Close input file */
  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
  fclose(png_input);
}

void write_image_data(const char *filename, png_bytep data, size_t w, size_t h) {

  /* Open output file */
  FILE *png_output;
  if ((png_output = fopen(filename, "wb")) == NULL) {
    perror("Create output image file");
    exit(1);
  }

  /* Write image data */
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, png_output);
  png_set_IHDR(png_ptr, info_ptr, w, h, 16, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr, info_ptr);
  for (int i = 0; i < h; i++) {
    png_write_row(png_ptr, data + i * png_get_rowbytes(png_ptr, info_ptr));
  }

  /* Close file */
  png_write_end(png_ptr, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(png_output);
}

int main(void) {
  size_t global_size[2];
  png_bytep pixels;
  cl_image_format png_format;
  size_t origin[3], region[3];
  size_t width, height;

  read_image_data(INPUT_FILE, &pixels, &width, &height);

  // clang-format off
  cl_device_id device = create_device();
  cl_context context  = clCreateContext(NULL, 1, &device, NULL, NULL, &err);                                                               handleError("Couldn't create a context.");

  cl_program program = build_program(context, device, PROGRAM_FILE);
  cl_kernel kernel   = clCreateKernel(program, KERNEL_FUNC, &err);                                                                         handleError("Couldn't create a kernel.");

  png_format.image_channel_order     = CL_LUMINANCE;
  png_format.image_channel_data_type = CL_UNORM_INT16;
  cl_image_desc imageDesc;
  imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
  imageDesc.image_width = width;
  imageDesc.image_height = height;
  cl_mem input_image = clCreateImage(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &png_format, &imageDesc, pixels, &err);             handleError("Couldn't create input image object.");
  cl_mem output_image = clCreateImage(context, CL_MEM_WRITE_ONLY, &png_format, &imageDesc, NULL, &err);                                    handleError("Couldn't create output image object.");

  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_image);                                                                          handleError("Couldn't set a kernel argument.");
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_image);                                                                         handleError("Couldn't set a kernel argument.");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);                                                handleError("Couldn't create a command queue.");

  global_size[0] = width;
  global_size[1] = height;
  err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);                                                  handleError("Couldn't enqueue the kernel.");

  origin[0] = 0;
  origin[1] = 0;
  origin[2] = 0;
  region[0] = width;
  region[1] = height;
  region[2] = 1;
  err = clEnqueueReadImage(queue, output_image, CL_BLOCKING, origin, region, 0, 0, pixels, 0, NULL, NULL);                                 handleError("Couldn't read from the image object.");

  write_image_data(OUTPUT_FILE, pixels, width, height);

  free(pixels);
  clReleaseMemObject(input_image);
  clReleaseMemObject(output_image);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
}
