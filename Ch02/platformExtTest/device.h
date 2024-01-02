#pragma once

#include "deviceCL.h"
#include "glib.h"
#include "json_object.h"

void getProperties(GArray *queueProperties, char *name, json_object *Device);
GArray *getDevices(cl_platform_id platform);
