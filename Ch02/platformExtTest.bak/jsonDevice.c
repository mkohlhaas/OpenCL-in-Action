#include "deviceCL.h"
#include "json_object.h"
#include "version.h"
#include <CL/cl.h>

json_object *AddDevices(json_object *Platform) {

  json_object *Devices = json_object_new_array();
  json_object_object_add(Platform, "devices", Devices);
  return Devices;
}

json_object *AddDevice(json_object *Devices) {
  json_object *Device = json_object_new_object();
  json_object_array_add(Devices, Device);
  return Device;
}

void AddDeviceId(json_object *Device, cl_device_id device) { json_object_object_add(Device, "id", json_object_new_uint64((intptr_t)device)); }

void AddDevicePlatformId(json_object *Device, cl_device_id device) {
  cl_platform_id platformID = getDevicePlatform(device);
  json_object_object_add(Device, "platform_id", json_object_new_uint64((intptr_t)platformID));
}
