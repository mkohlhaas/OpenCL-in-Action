#include "glib.h"
#include "platformCL.h"

GArray *getPlatforms() {
  cl_uint numPlatforms;
  cl_platform_id *platformsOrig = platforms(&numPlatforms);
  GArray *platforms = g_array_new(FALSE, FALSE, sizeof(cl_platform_id *));
  for (int i = 0; i < numPlatforms; i++) {
    g_array_append_val(platforms, platformsOrig[i]);
  }
  return platforms;
}

cl_platform_id currPlatform(GArray *platforms, int i) {
  cl_platform_id platform = g_array_index(platforms, cl_platform_id, i);
  return platform;
}
