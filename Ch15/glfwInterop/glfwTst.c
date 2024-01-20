#include <CL/cl.h>
#define GLFW_EXPOSE_NATIVE_GLX
#define GLFW_EXPOSE_NATIVE_X11
// #define GLFW_INCLUDE_NONE
#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// clang-format off
// #include <glad.h>
#include <CL/cl_gl.h>
// #include <glad.h>
// clang-format on

#define PROGRAM_FILE "basic_interop.cl"
#define KERNEL_FUNC "basic_interop"

GLFWwindow *window;
cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "Error Code: %d\n", err);
    fprintf(stderr, "%s\n", message);
    sleep(2);
    exit(EXIT_FAILURE);
  }
}

char *read_file(const char *filename, size_t *size) {

  FILE *handle;
  char *buffer;

  /* Read program file and place content into buffer */
  handle = fopen(filename, "r");
  if (handle == NULL) {
    perror("Couldn't find the file");
    exit(1);
  }
  fseek(handle, 0, SEEK_END);
  *size = (size_t)ftell(handle);
  rewind(handle);
  buffer = (char *)malloc(*size + 1);
  buffer[*size] = '\0';
  fread(buffer, sizeof(char), *size, handle);
  fclose(handle);

  return buffer;
}

void contextCreationCB(const char *errinfo, const void *private_info, size_t cb, void *user_data) {
  fprintf(stderr, "CB: %s %zu\n", errinfo, cb);
}

void init_cl(GLFWwindow *window) {

  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't identify a platform.");

  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  handleError("Couldn't access any devices.");

  // clang-format off
  GLXContext  glxContext = glfwGetGLXContext(window);
  Display    *glxDisplay = glfwGetX11Display();
  cl_context_properties properties[] = {CL_GL_CONTEXT_KHR,   (cl_context_properties)glxContext,
                                        CL_GLX_DISPLAY_KHR,  (cl_context_properties)glxDisplay,
                                        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
                                        0};
  // clang-format on

  cl_context context = clCreateContext(properties, 1, &device, contextCreationCB, NULL, &err);
  handleError("Couldn't create a context.");

  size_t program_size;
  char *program_buffer = read_file(PROGRAM_FILE, &program_size);
  cl_program program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);
  handleError("Couldn't create the program.");

  size_t log_size;
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  char *program_log = (char *)malloc(log_size);
  err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
  handleError("Couldn't get program build info.");
  printf("Build log:\n%s\n", program_log);

  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
  handleError("Couldn't create a command queue.");

  cl_kernel kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  handleError("Couldn't create a kernel.");
}

void error_callback(int error, const char *description) { fprintf(stderr, "Error: %s\n", description); }

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

GLFWwindow *createWindow() {
  GLFWwindow *window = glfwCreateWindow(640, 480, "", NULL, NULL);
  if (!window) {
    glfwTerminate();
    handleError("Couldn't create window.\n");
  }
  return window;
}

void initGlfw() {
  if (!glfwInit()) {
    handleError("Couldn't initialize GLFW.\n");
  }
}

int main(void) {
  glfwSetErrorCallback(error_callback);
  initGlfw();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow *window = createWindow();
  glfwSetKeyCallback(window, key_callback);
  glfwMakeContextCurrent(window);
  // gladLoadGL();
  glfwSwapInterval(1);

  init_cl(window);

  while (!glfwWindowShouldClose(window)) {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}
