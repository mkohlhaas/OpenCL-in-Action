#include <unistd.h>
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_GLX

#define PROGRAM_FILE "basic_interop.cl"
#define KERNEL_FUNC "basic_interop"
#define VERTEX_SHADER "basic_interop.vert"
#define FRAGMENT_SHADER "basic_interop.frag"

// clang-format off
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <GLFW/glfw3.h>
#include <glad.h>
#include <GLFW/glfw3native.h>
// clang-format on

#include <stdio.h>
// #include "linmath.h"

int err;
cl_platform_id platform;
cl_device_id device;
cl_context context;
cl_kernel kernel;
cl_command_queue queue;
cl_program program;
GLFWwindow *window;

GLuint vao[3], vbo[6];
cl_mem mem_objects[6];

/* Read a character buffer from a file */
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

/* Initialize OpenCl processing */
void init_cl(GLFWwindow *window) {

  char *program_buffer, *program_log;
  size_t program_size, log_size;

  /* Identify a platform */
  err = clGetPlatformIDs(1, &platform, NULL);
  if (err < 0) {
    perror("Couldn't identify a platform");
    exit(1);
  }

  /* Access a device */
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err < 0) {
    perror("Couldn't access any devices");
    exit(1);
  }

  cl_context_properties properties[] = {CL_GL_CONTEXT_KHR,
                                        (cl_context_properties)glfwGetGLXContext(window),
                                        CL_GLX_DISPLAY_KHR,
                                        (cl_context_properties)glfwGetGLXWindow(window),
                                        CL_CONTEXT_PLATFORM,
                                        (cl_context_properties)platform,
                                        0};

  /* Create context */
  context = clCreateContext(properties, 1, &device, contextCreationCB, NULL, &err);
  fprintf(stderr, "Context error code: %d\n", err);
  if (err < 0) {
    fprintf(stderr, "Couldn't create a context.\n");
    exit(1);
  }

  /* Create program from file */
  program_buffer = read_file(PROGRAM_FILE, &program_size);
  program = clCreateProgramWithSource(context, 1, (const char **)&program_buffer, &program_size, &err);
  if (err) {
    perror("Couldn't create the program");
    exit(1);
  }
  free(program_buffer);

  /* Build program */
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err < 0) {

    /* Find size of log and print to std output */
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    program_log = (char *)malloc(log_size + 1);
    program_log[log_size] = '\0';
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(1);
  }

  /* Create a command queue */
  queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  if (err < 0) {
    perror("Couldn't create a command queue");
    exit(1);
  };

  /* Create kernel */
  kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  fprintf(stderr, "Kernel created: %d\n", err);
  if (err < 0) {
    printf("Couldn't create a kernel: %d", err);
    exit(1);
  };
}

/* Compile the shader */
void compile_shader(GLint shader) {

  GLint success;
  GLsizei log_size;
  GLchar *log;

  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
    log = (char *)malloc(log_size + 1);
    log[log_size] = '\0';
    glGetShaderInfoLog(shader, log_size + 1, NULL, log);
    printf("%s\n", log);
    free(log);
    exit(1);
  }
}

/* Create, compile, and deploy shaders */
void init_shaders(void) {

  GLuint vs, fs, prog;
  char *vs_source, *fs_source;
  size_t vs_length, fs_length;

  vs = glCreateShader(GL_VERTEX_SHADER);
  fs = glCreateShader(GL_FRAGMENT_SHADER);

  vs_source = read_file(VERTEX_SHADER, &vs_length);
  fs_source = read_file(FRAGMENT_SHADER, &fs_length);

  glShaderSource(vs, 1, (const char **)&vs_source, (GLint *)&vs_length);
  glShaderSource(fs, 1, (const char **)&fs_source, (GLint *)&fs_length);

  compile_shader(vs);
  compile_shader(fs);

  prog = glCreateProgram();

  glBindAttribLocation(prog, 0, "in_coords");
  glBindAttribLocation(prog, 1, "in_color");

  glAttachShader(prog, vs);
  glAttachShader(prog, fs);

  glLinkProgram(prog);
  glUseProgram(prog);
}

static void error_callback(int error, const char *description) { fprintf(stderr, "Error: %s\n", description); }

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/* Initialize the rendering objects and context properties */
void init_gl() {

  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);

  window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);

  /* Create and compile shaders */
  init_shaders();
}

void configure_shared_data() {

  /* Create 3 vertex array objects - one for each square */
  glGenVertexArrays(3, vao);
  glBindVertexArray(vao[0]);

  /* Create 6 vertex buffer objects (VBOs) - one for each set of coordinates and colors */
  glGenBuffers(6, vbo);

  /* VBO for coordinates of first square */
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  /* VBO for colors of first square */
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  /* VBO for coordinates of second square */
  glBindVertexArray(vao[1]);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  /* VBO for colors of second square */
  glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
  glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  /* VBO for coordinates of third square */
  glBindVertexArray(vao[2]);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
  glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  /* VBO for colors of third square */
  glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
  glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  /* Create memory objects from the VBOs */
  for (int i = 0; i < 6; i++) {
    mem_objects[i] = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, vbo[i], &err);
    if (err < 0) {
      perror("Couldn't create a buffer object from the VBO");
      exit(1);
    }
    err = clSetKernelArg(kernel, i, sizeof(cl_mem), &mem_objects[i]);
    if (err < 0) {
      printf("Couldn't set a kernel argument");
      exit(1);
    };
  }
}

void execute_kernel() {

  /* Complete OpenGL processing */
  glFinish();

  /* Execute the kernel */
  err = clEnqueueAcquireGLObjects(queue, 6, mem_objects, 0, NULL, NULL);
  fprintf(stderr, "  clEnqueueAcquireGLObjects finished: %d\n", err);
  if (err < 0) {
    perror("Couldn't acquire the GL objects");
    exit(1);
  }

  size_t global_work_size[] = {1};
  size_t local_work_size[] = {1};
  cl_event kernel_event;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &kernel_event);
  fprintf(stderr, "  clEnqueueNDRangeKernel finished: %d\n", err);
  if (err < 0) {
    perror("Couldn't enqueue the kernel");
    exit(1);
  }

  sleep(5);

  fprintf(stderr, "  Waiting for kernel.\n");
  err = clWaitForEvents(1, &kernel_event);
  fprintf(stderr, "  clWaitForEvents finished: %d\n", err);
  if (err < 0) {
    perror("Waiting for event failed somehow.");
    exit(1);
  }

  fprintf(stderr, "  Releasing GL objects.\n");
  err = clEnqueueReleaseGLObjects(queue, 6, mem_objects, 0, NULL, NULL);
  fprintf(stderr, "  clEnqueueReleaseGLObjects finished: %d\n", err);
  err = clFinish(queue);
  fprintf(stderr, "  clFinish finished: %d\n", err);
  err = clReleaseEvent(kernel_event);
  fprintf(stderr, "  clReleaseEvent finished: %d\n", err);
}

void display(GLFWwindow *window) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(vao[2]);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindVertexArray(vao[1]);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindVertexArray(vao[0]);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindVertexArray(0);
  glfwSwapBuffers(window);
}

void reshape(int w, int h) { glViewport(0, 0, (GLsizei)w, (GLsizei)h); }

int main() {

  /* Start GL processing */
  init_gl();

  /* Initialize CL data structures */
  init_cl(window);
  fprintf(stderr, "Init OpenCL finished.\n");

  /* Create CL and GL data objects */
  configure_shared_data();
  fprintf(stderr, "Configuring shared data finished.\n");

  /* Execute kernel */
  execute_kernel();
  fprintf(stderr, "Executing kernel finished.\n");

  for (int i = 0; i < 6; i++) {
    clReleaseMemObject(mem_objects[i]);
  }
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);

  glDeleteBuffers(6, vbo);
}
