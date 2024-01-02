#include "json_object.h"
#include "platformCL.h"
#include "version.h"
#include <CL/cl.h>

json_object *CreateRoot() { return json_object_new_object(); }

json_object *AddPlatforms(json_object *Root) {

  json_object *Platforms = json_object_new_array();
  json_object_object_add(Root, "platforms", Platforms);
  return Platforms;
}

json_object *AddPlatform(json_object *Platforms) {
  json_object *Platform = json_object_new_object();
  json_object_array_add(Platforms, Platform);
  return Platform;
}

void AddPlatformId(json_object *Platform, cl_platform_id platform) {
  json_object_object_add(Platform, "id", json_object_new_uint64((intptr_t)platform));
}

void AddPlatformName(json_object *Platform, cl_platform_id platform) {
  char *name = platformName(platform);
  json_object_object_add(Platform, "name", json_object_new_string(name));
}

void AddPlatformProfile(json_object *Platform, cl_platform_id platform) {
  char *profile = platformProfile(platform);
  json_object_object_add(Platform, "profile", json_object_new_string(profile));
}

void AddPlatformVendor(json_object *Platform, cl_platform_id platform) {
  char *vendor = platformVendor(platform);
  json_object_object_add(Platform, "vendor", json_object_new_string(vendor));
}

void AddPlatformVersion(json_object *Platform, cl_platform_id platform) {
  char *version = platformVersion(platform);
  json_object_object_add(Platform, "version", json_object_new_string(version));
}

void AddPlatformHostTimerResolution(json_object *Platform, cl_platform_id platform) {
  cl_ulong htr = platformHostTimerResolution(platform);
  json_object_object_add(Platform, "host_timer_resolution", json_object_new_uint64(htr));
}

void AddPlatformExtensions(json_object *Platform, cl_platform_id platform) {
  size_t numExtensions;
  cl_name_version *extensions = platformExtensions(platform, &numExtensions);
  json_object *Extensions = json_object_new_array();
  for (int i = 0; i < numExtensions; i++) {
    json_object *Extension = json_object_new_object();
    json_object_object_add(Extension, "name", json_object_new_string(extensions[i].name));
    json_object_object_add(Extension, "version", json_object_new_string(versionStr(extensions[i].version)));
    json_object_array_add(Extensions, Extension);
  }
  json_object_object_add(Platform, "extensions", Extensions);
}
