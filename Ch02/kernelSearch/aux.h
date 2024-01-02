#include <CL/cl.h>

cl_context createContext(cl_device_id);
cl_device_id getFirstGPUDevice(cl_platform_id);
cl_kernel *createAllKernels(cl_program, cl_uint *);
cl_platform_id getFirstPlatform();
cl_program createProgram(cl_context, cl_device_id);
int findNamedKernel(cl_kernel *, cl_uint, char *);
void releaseResources(cl_context, cl_device_id, cl_program, cl_kernel *, cl_uint);
