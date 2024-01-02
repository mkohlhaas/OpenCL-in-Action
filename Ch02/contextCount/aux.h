#include <CL/cl.h>

cl_context createContext(cl_device_id);
cl_device_id getFirstGPUDevice(cl_platform_id);
cl_platform_id getFirstPlatform();
cl_uint getReferenceCount(cl_context);
void releaseContext(cl_context context);
void releaseResources(cl_context, cl_device_id);
void retainContext(cl_context context);
