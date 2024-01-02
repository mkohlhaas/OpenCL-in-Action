#pragma once

#include "glib.h"
#include <CL/cl.h>

GArray *getPlatforms();
cl_platform_id currPlatform(GArray *platforms, int i);
