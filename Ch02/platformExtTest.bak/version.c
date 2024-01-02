#include <CL/cl.h>
#include <glib.h>

typedef struct {
  cl_uint major;
  cl_uint minor;
  cl_uint patch;
} unpackedVersion;

static unpackedVersion unpackVersion(cl_uint version) {
  unpackedVersion retValue;
  retValue.major = (version >> 22);
  retValue.minor = (version >> 12) & ((1 << 10) - 1);
  retValue.patch = version & ((1 << 12) - 1);
  return retValue;
}

char *versionStr(cl_uint version) {
  unpackedVersion unpacked = unpackVersion(version);
  GString *retValue = g_string_new(NULL);
  g_string_append_printf(retValue, "%u.%u.%u", unpacked.major, unpacked.minor, unpacked.patch);
  return g_string_free(retValue, FALSE);
}
