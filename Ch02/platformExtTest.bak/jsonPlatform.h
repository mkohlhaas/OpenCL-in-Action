#pragma once

#include "glib.h"
#include "json_object.h"
#include <CL/cl.h>

json_object *CreateRoot();
json_object *AddPlatforms(json_object *Root);
json_object *AddPlatform(json_object *Platforms);
void AddPlatformId(json_object *Platform, cl_platform_id platform);
void AddPlatformName(json_object *Platform, cl_platform_id platform);
void AddPlatformProfile(json_object *Platform, cl_platform_id platform);
void AddPlatformVendor(json_object *Platform, cl_platform_id platform);
void AddPlatformVersion(json_object *Platform, cl_platform_id platform);
void AddPlatformHostTimerResolution(json_object *Platform, cl_platform_id platform);
void AddPlatformExtensions(json_object *Platform, cl_platform_id platform);
void AddPlatformDevices(json_object *Platform, cl_platform_id platform);
