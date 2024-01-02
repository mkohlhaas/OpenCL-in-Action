#pragma once

#include "glib.h"
#include "json_types.h"
#include <CL/cl.h>

void getProperties(GArray *queueProperties, char *name, json_object *Device);
GArray *getDevices(cl_platform_id platform);
cl_device_id currDevice(GArray *devices, int i);
