#pragma once

#include "json_types.h"
#include <CL/cl.h>

// clang-format off
json_object* AddPlatform(json_object *Platforms);
json_object* AddPlatforms(json_object *Root);
json_object* CreateRoot();
void         PrintRoot(json_object *Root);
void         AddPlatformDevices(json_object *Platform, cl_platform_id platform);
void         AddPlatform_Extensions(json_object *Platform, cl_platform_id platform);
void         AddPlatform_HostTimerResolution(json_object *Platform, cl_platform_id platform);
void         AddPlatform_Id(json_object *Platform, cl_platform_id platform);
void         AddPlatform_Name(json_object *Platform, cl_platform_id platform);
void         AddPlatform_Profile(json_object *Platform, cl_platform_id platform);
void         AddPlatform_Vendor(json_object *Platform, cl_platform_id platform);
void         AddPlatform_Version(json_object *Platform, cl_platform_id platform);
