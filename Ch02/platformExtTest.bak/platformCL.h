#pragma once

#include <CL/cl.h>

// clang-format off
char            *platformName(cl_platform_id platform);
char            *platformProfile(cl_platform_id platform);
char            *platformVendor(cl_platform_id platform);
char            *platformVersion(cl_platform_id platform);
cl_device_id    *deviceIDs(cl_platform_id platform, cl_uint *numDevices);
cl_name_version *platformExtensions(cl_platform_id platform, size_t *);
cl_platform_id  *platforms(cl_uint *numPlatforms);
cl_ulong         platformHostTimerResolution(cl_platform_id platform);
