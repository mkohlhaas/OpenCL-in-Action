#include <stdio.h>

#include <CL/cl.h>
#include <CL/cl_gl.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_GLX
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#define PROGRAM_FILE "basic_interop.cl"
#define KERNEL_FUNC "basic_interop"

// Error handling

cl_int err;

void handleError(char *message) {
  if (err) {
    fprintf(stderr, "Error Code: %d\n", err);
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

// Callback functions

void clContextCreationCB(const char *errinfo, const void *private_info, size_t cb, void *user_data) {
  fprintf(stderr, "CB: %s %zu\n", errinfo, cb);
}

void errorCB(int error, const char *description) { fprintf(stderr, "Error: %s\n", description); }

void keyCB(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void initGlfw() {
  if (!glfwInit()) {
    handleError("Couldn't initialize GLFW.\n");
  }
}

GLFWwindow *createGlfwWindow() {
  GLFWwindow *window = glfwCreateWindow(640, 480, "OpenGL-OpenCL Interop", NULL, NULL);
  if (!window) {
    glfwTerminate();
    handleError("Couldn't create window.\n");
  }
  return window;
}

// OpenCL initialization
void initCl(GLFWwindow *window) {

  cl_platform_id platform;
  err = clGetPlatformIDs(1, &platform, NULL);
  handleError("Couldn't identify a platform.");

  cl_device_id device;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  handleError("Couldn't access any devices.");

  // TODO: Are these the correct functions ?
  GLXContext glxContext = glfwGetGLXContext(window);
  Display *glxDisplay = glfwGetX11Display();
  cl_context_properties properties[] = {CL_GL_CONTEXT_KHR,
                                        (cl_context_properties)glxContext,
                                        CL_GLX_DISPLAY_KHR,
                                        (cl_context_properties)glxDisplay,
                                        CL_CONTEXT_PLATFORM,
                                        (cl_context_properties)platform,
                                        0};

  cl_context context = clCreateContext(properties, 1, &device, clContextCreationCB, NULL, &err);
  handleError("Couldn't create a context.");
}

int main(void) {
  // glfwSetErrorCallback(errorCB);

  initGlfw();

  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = createGlfwWindow();

  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, keyCB);
  glfwSwapInterval(1);

  initCl(window);

  while (!glfwWindowShouldClose(window)) {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}
