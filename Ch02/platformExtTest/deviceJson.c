#include "device.h"
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

void AddDevice_Id(json_object *Device, cl_device_id device) { json_object_object_add(Device, "id", json_object_new_uint64((intptr_t)device)); }

void AddDevice_PlatformId(json_object *Device, cl_device_id device) {
  cl_platform_id platformID = getDevicePlatform(device);
  json_object_object_add(Device, "platform_id", json_object_new_uint64((intptr_t)platformID));
}

void AddDevice_Name(json_object *Device, cl_device_id device) {
  char *name = getDeviceName(device);
  json_object_object_add(Device, "name", json_object_new_string(name));
}

void AddDevice_Vendor(json_object *Device, cl_device_id device) {
  char *vendor = getDeviceVendor(device);
  json_object_object_add(Device, "vendor", json_object_new_string(vendor));
}

void AddDevice_Version(json_object *Device, cl_device_id device) {
  char *version = getDeviceVersion(device);
  json_object_object_add(Device, "version", json_object_new_string(version));
}

void AddDevice_DriverVersion(json_object *Device, cl_device_id device) {
  char *driverVersion = getDeviceDriverVersion(device);
  json_object_object_add(Device, "driver_version", json_object_new_string(driverVersion));
}

void AddDevice_Profile(json_object *Device, cl_device_id device) {
  char *profile = getDeviceProfile(device);
  json_object_object_add(Device, "profile", json_object_new_string(profile));
}

void AddDevice_Types(json_object *Device, cl_device_id device) {
  json_object *Types = json_object_new_array();
  GArray *deviceTypes = getDeviceTypes(device);
  for (int i = 0; i < deviceTypes->len; i++) {
    json_object_array_add(Types, json_object_new_string(g_array_index(deviceTypes, char *, i)));
  }
  json_object_object_add(Device, "types", Types);
}

void AddDevice_AtomicMemoryCapabilities(json_object *Device, cl_device_id device) {
  getProperties(getDeviceAtomicMemoryCapabilities(device), "atomic_memory_capabilities", Device);
}

void AddDevice_AtomicFenceCapabilities(json_object *Device, cl_device_id device) {
  getProperties(getDeviceAtomicFenceCapabilities(device), "atomic_fence_capabilities", Device);
}

void AddDevice_AffinityDomain(json_object *Device, cl_device_id device) {
  getProperties(getDeviceAffinityDomain(device), "affinity_domain", Device);
}

void AddDevice_AddressBits(json_object *Device, cl_device_id device) {
  cl_uint addressBits = getDeviceAddressBits(device);
  json_object_object_add(Device, "address_bits", json_object_new_uint64(addressBits));
}

void AddDevice_Available(json_object *Device, cl_device_id device) {
  cl_uint available = getDeviceAvailable(device);
  json_object_object_add(Device, "available", json_object_new_boolean(available));
}

void AddDevice_BuiltInKernels(json_object *Device, cl_device_id device) {
  size_t numBuiltInKernels;
  cl_name_version *builtInKernels = getDeviceBuiltInKernels(device, &numBuiltInKernels);
  json_object *BuiltInKernels = json_object_new_array();
  for (int i = 0; i < numBuiltInKernels; i++) {
    json_object *BuiltInKernel = json_object_new_object();
    json_object_object_add(BuiltInKernel, "name", json_object_new_string(builtInKernels[i].name));
    json_object_object_add(BuiltInKernel, "version", json_object_new_string(versionStr(builtInKernels[i].version)));
    json_object_array_add(BuiltInKernels, BuiltInKernel);
  }
  json_object_object_add(Device, "built_in_kernels", BuiltInKernels);
}

void AddDevice_CompilerAvailable(json_object *Device, cl_device_id device) {
  cl_uint compilerAvailable = getDeviceCompilerAvailable(device);
  json_object_object_add(Device, "compiler_available", json_object_new_boolean(compilerAvailable));
}

void AddDevice_DoubleFPConfig(json_object *Device, cl_device_id device) {
  getProperties(getDeviceDoubleFPConfig(device), "double_fp_config", Device);
}

void AddDevice_EndianLittle(json_object *Device, cl_device_id device) {
  cl_uint endianLittle = getDeviceEndianLittle(device);
  json_object_object_add(Device, "endian_little", json_object_new_boolean(endianLittle));
}

void AddDevice_EnqueueCapabilities(json_object *Device, cl_device_id device) {
  getProperties(getDeviceEnqueueCapabilities(device), "enqueue_capabilities", Device);
}

void AddDevice_ErrorCorrectionSupport(json_object *Device, cl_device_id device) {
  cl_uint errorCorrectionSupport = getDeviceErrorCorrectionSupport(device);
  json_object_object_add(Device, "error_correction_support", json_object_new_boolean(errorCorrectionSupport));
}

void AddDevice_ExecCapabilities(json_object *Device, cl_device_id device) {
  getProperties(getDeviceExecCapabilities(device), "exec_capabilities", Device);
}

void AddDevice_Extensions(json_object *Device, cl_device_id device) {
  size_t numDeviceExtensions;
  cl_name_version *extensions = getDeviceExtensionsWithVersion(device, &numDeviceExtensions);
  json_object *Extensions = json_object_new_array();
  for (int i = 0; i < numDeviceExtensions; i++) {
    json_object *Extension = json_object_new_object();
    json_object_object_add(Extension, "name", json_object_new_string(extensions[i].name));
    json_object_object_add(Extension, "version", json_object_new_string(versionStr(extensions[i].version)));
    json_object_array_add(Extensions, Extension);
  }
  json_object_object_add(Device, "extensions", Extensions);
}

void AddDevice_GenericAddressSpaceSupport(json_object *Device, cl_device_id device) {
  cl_uint genericAddressSpaceSupport = getDeviceGenericAddressSpaceSupport(device);
  json_object_object_add(Device, "generic_address_space_support", json_object_new_boolean(genericAddressSpaceSupport));
}

void AddDevice_GlobalMemSize(json_object *Device, cl_device_id device) {
  cl_ulong globalMemSize = getDeviceGlobalMemSize(device);
  json_object_object_add(Device, "global_mem_size", json_object_new_uint64(globalMemSize));
}

void AddDevice_GlobalMemCacheSize(json_object *Device, cl_device_id device) {
  cl_ulong globalMemCacheSize = getDeviceGlobalMemCacheSize(device);
  json_object_object_add(Device, "global_mem_cache_size", json_object_new_uint64(globalMemCacheSize));
}

void AddDevice_GlobalMemCachelineSize(json_object *Device, cl_device_id device) {
  cl_uint globalMemCachelineSize = getDeviceGlobalMemCachelineSize(device);
  json_object_object_add(Device, "global_mem_cacheline_size", json_object_new_uint64(globalMemCachelineSize));
}

void AddDevice_GlobalVariablePreferredTotalSize(json_object *Device, cl_device_id device) {
  size_t globalVariablePreferredTotalSize = getDeviceGlobalVariablePreferredTotalSize(device);
  json_object_object_add(Device, "global_variable_preferred_total_size", json_object_new_uint64(globalVariablePreferredTotalSize));
}

void AddDevice_IlVersion(json_object *Device, cl_device_id device) {
  char *ilVersion = getDeviceIlVersion(device);
  json_object_object_add(Device, "il_version", json_object_new_string(ilVersion));
}

void AddDevice_IlsWithVersion(json_object *Device, cl_device_id device) {
  size_t numIls;
  cl_name_version *ils = getDeviceIlsWithVersion(device, &numIls);
  json_object *Ils = json_object_new_array();
  for (int i = 0; i < numIls; i++) {
    json_object *Il = json_object_new_object();
    json_object_object_add(Il, "name", json_object_new_string(ils[i].name));
    json_object_object_add(Il, "version", json_object_new_string(versionStr(ils[i].version)));
    json_object_array_add(Ils, Il);
  }
  json_object_object_add(Device, "ils", Ils);
}

void AddDevice_ImageBaseAddressAlignment(json_object *Device, cl_device_id device) {
  cl_uint imageBaseAddressAlignment = getDeviceImageBaseAddressAlignment(device);
  json_object_object_add(Device, "image_base_address_alignment", json_object_new_uint64(imageBaseAddressAlignment));
}

void AddDevice_ImageMaxArraySize(json_object *Device, cl_device_id device) {
  size_t imageMaxArraySize = getDeviceImageMaxArraySize(device);
  json_object_object_add(Device, "image_max_array_size", json_object_new_uint64(imageMaxArraySize));
}

void AddDevice_ImageMaxBufferSize(json_object *Device, cl_device_id device) {
  size_t imageMaxBufferSize = getDeviceImageMaxBufferSize(device);
  json_object_object_add(Device, "image_max_buffer_size", json_object_new_uint64(imageMaxBufferSize));
}

void AddDevice_ImagePitchAlignment(json_object *Device, cl_device_id device) {
  cl_uint imagePitchAlignment = getDeviceImagePitchAlignment(device);
  json_object_object_add(Device, "image_pitch_alignment", json_object_new_uint64(imagePitchAlignment));
}

void AddDevice_ImageSupport(json_object *Device, cl_device_id device) {
  cl_uint imageSupport = getDeviceImageSupport(device);
  json_object_object_add(Device, "image_support", json_object_new_boolean(imageSupport));
}

void AddDevice_Image2dMaxHeight(json_object *Device, cl_device_id device) {
  size_t image2dMaxHeight = getDeviceImage2dMaxHeight(device);
  json_object_object_add(Device, "image2d_max_height", json_object_new_uint64(image2dMaxHeight));
}

void AddDevice_Image2dMaxWidth(json_object *Device, cl_device_id device) {
  size_t image2dMaxWidth = getDeviceImage2dMaxWidth(device);
  json_object_object_add(Device, "image2d_max_width", json_object_new_uint64(image2dMaxWidth));
}

void AddDevice_Image3dMaxDepth(json_object *Device, cl_device_id device) {
  size_t image3dMaxDepth = getDeviceImage3dMaxDepth(device);
  json_object_object_add(Device, "image3d_max_depth", json_object_new_uint64(image3dMaxDepth));
}

void AddDevice_Image3dMaxHeight(json_object *Device, cl_device_id device) {
  size_t image3dMaxHeight = getDeviceImage3dMaxHeight(device);
  json_object_object_add(Device, "image3d_max_height", json_object_new_uint64(image3dMaxHeight));
}

void AddDevice_Image3dmaxWidth(json_object *Device, cl_device_id device) {
  size_t image3dmaxWidth = getDeviceImage3dmaxWidth(device);
  json_object_object_add(Device, "image3d_max_width", json_object_new_uint64(image3dmaxWidth));
}

void AddDevice_LatestConformanceVersionPassed(json_object *Device, cl_device_id device) {
  char *latestConformanceVersionPassed = getDeviceLatestConformanceVersionPassed(device);
  json_object_object_add(Device, "latest_conformance_version_passed", json_object_new_string(latestConformanceVersionPassed));
}

void AddDevice_LinkerAvailable(json_object *Device, cl_device_id device) {
  cl_uint linkerAvailable = getDeviceLinkerAvailable(device);
  json_object_object_add(Device, "linker_available", json_object_new_boolean(linkerAvailable));
}

void AddDevice_LocalMemSize(json_object *Device, cl_device_id device) {
  cl_ulong localMemSize = getDeviceLocalMemSize(device);
  json_object_object_add(Device, "local_mem_size", json_object_new_uint64(localMemSize));
}

void AddDevice_LocalMemType(json_object *Device, cl_device_id device) {
  getProperties(getDeviceLocalMemType(device), "local_mem_type", Device);
}
