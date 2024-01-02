#include "json_object.h"
#include "platformCL.h"
#include "version.h"
#include <CL/cl.h>

json_object *AddDevices(json_object *Platform);
json_object *AddDevice(json_object *Devices);
void AddDeviceId(json_object *Device, cl_device_id device);
void AddDevicePlatformId(json_object *Device, cl_device_id device);
