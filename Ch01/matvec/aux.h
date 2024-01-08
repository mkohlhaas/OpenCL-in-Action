#include <CL/cl.h>

// clang-format off
cl_command_queue createCommandQueue (cl_context, cl_device_id);
cl_context       createContext      (cl_device_id);
cl_device_id     getFirstGPUDevice  (cl_platform_id);
cl_kernel        createKernel       (cl_program);
cl_mem           createMatrixBuffer (cl_context, float *);
cl_mem           createResultBuffer (cl_context);
cl_mem           createVectorBuffer (cl_context, float *);
cl_platform_id   getFirstPlatform   ();
cl_program       createProgram      (cl_context, cl_device_id);
void             execKernel         (cl_kernel, cl_command_queue, cl_mem, cl_mem, cl_mem);
void             readResult         (cl_command_queue, cl_mem, float *);
void             releaseResources   (cl_device_id, cl_context, cl_program, cl_kernel, cl_command_queue, cl_mem, cl_mem, cl_mem);
void             testResult         (float *, float *, float *);
