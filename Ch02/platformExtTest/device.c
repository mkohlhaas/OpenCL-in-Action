#include "deviceCL.h"
#include "glib.h"
#include "json_object.h"
#include "platformCL.h"
#include <CL/cl.h>

void getProperties(GArray *queueProperties, char *name, json_object *Device) {
  json_object *QueueProperties = json_object_new_array();
  for (int i = 0; i < queueProperties->len; i++) {
    property queueProperty = g_array_index(queueProperties, property, i);
    json_object *QueueProperty = json_object_new_object();
    json_object_object_add(QueueProperty, queueProperty.property, json_object_new_boolean(queueProperty.enabled));
    json_object_array_add(QueueProperties, QueueProperty);
  }
  json_object_object_add(Device, name, QueueProperties);
}

GArray *getDevices(cl_platform_id platform) {

  // Devices
  cl_uint numDevices;
  cl_device_id *devicesOrig = deviceIDs(platform, &numDevices);
  GArray *devices = g_array_new(FALSE, FALSE, sizeof(cl_device_id *));
  for (int i = 0; i < numDevices; i++) {
    g_array_append_val(devices, devicesOrig[i]);
  }
  return devices;
}
