#include "device.h"
#include "deviceJson.h"
#include "platform.h"
#include "platformJson.h"
#include <CL/cl.h>

int main(void) {

  // TODO
  // BuildRoot(...);
  // PrintRoot(...);

  json_object *Root = CreateRoot();
  json_object *Platforms = AddPlatforms(Root);

  // Loop over platforms
  GArray *platforms = getPlatforms();
  for (int i = 0; i < platforms->len; i++) {
    cl_platform_id platform = currPlatform(platforms, i);
    json_object *Platform = AddPlatform(Platforms);

    AddPlatform_Id(Platform, platform);
    AddPlatform_Name(Platform, platform);
    AddPlatform_Profile(Platform, platform);
    AddPlatform_Vendor(Platform, platform);
    AddPlatform_Version(Platform, platform);
    AddPlatform_HostTimerResolution(Platform, platform);
    AddPlatform_Extensions(Platform, platform);

    json_object *Devices = AddDevices(Platform);

    // Loop over devices
    GArray *devices = getDevices(platform);
    for (int i = 0; i < devices->len; i++) {
      cl_device_id device = currDevice(devices, i);
      json_object *Device = AddDevice(Devices);

      AddDevice_Id(Device, device);
      AddDevice_PlatformId(Device, device);
      AddDevice_Name(Device, device);
      AddDevice_Vendor(Device, device);
      AddDevice_Version(Device, device);
      AddDevice_DriverVersion(Device, device);
      AddDevice_Profile(Device, device);
      AddDevice_Types(Device, device);
      AddDevice_AtomicMemoryCapabilities(Device, device);
      AddDevice_AtomicFenceCapabilities(Device, device);
      AddDevice_AffinityDomain(Device, device);
      AddDevice_AddressBits(Device, device);
      AddDevice_Available(Device, device);
      AddDevice_BuiltInKernels(Device, device);
      AddDevice_EndianLittle(Device, device);
      AddDevice_EnqueueCapabilities(Device, device);
      AddDevice_ExecCapabilities(Device, device);
      AddDevice_Extensions(Device, device);
      AddDevice_GenericAddressSpaceSupport(Device, device);
      AddDevice_GlobalMemSize(Device, device);
      AddDevice_GlobalMemCacheSize(Device, device);
      AddDevice_GlobalMemCachelineSize(Device, device);
      AddDevice_GlobalVariablePreferredTotalSize(Device, device);
      AddDevice_IlVersion(Device, device);
      AddDevice_IlsWithVersion(Device, device);
      AddDevice_ImageBaseAddressAlignment(Device, device);
      AddDevice_ImageMaxArraySize(Device, device);
      AddDevice_ImageMaxBufferSize(Device, device);
      AddDevice_ImagePitchAlignment(Device, device);
      AddDevice_ImageSupport(Device, device);
      AddDevice_Image2dMaxHeight(Device, device);
      AddDevice_Image2dMaxWidth(Device, device);
      AddDevice_Image3dMaxDepth(Device, device);
      AddDevice_Image3dMaxHeight(Device, device);
      AddDevice_Image3dmaxWidth(Device, device);
      AddDevice_LatestConformanceVersionPassed(Device, device);
      AddDevice_LinkerAvailable(Device, device);
      AddDevice_LocalMemSize(Device, device);
      AddDevice_LocalMemType(Device, device);
    }
  }
  PrintRoot(Root);
}

//   // Max Compute Units
//   cl_uint maxComputeUnits = getDeviceMaxComputeUnits(devices[i]);
//   json_object_object_add(Device, "max_compute_units", json_object_new_uint64(maxComputeUnits));
//
//   // Max Constant Args
//   cl_uint maxConstantArgs = getDeviceMaxConstantArgs(devices[i]);
//   json_object_object_add(Device, "max_constant_args", json_object_new_uint64(maxConstantArgs));
//
//   // Max Clock Frequency
//   cl_uint maxClockFrequency = getDeviceMaxClockFrequency(devices[i]);
//   json_object_object_add(Device, "max_clock_frequency", json_object_new_uint64(maxClockFrequency));
//
//   // Max Constant Buffer Size
//   cl_ulong maxConstantBufferSize = getDeviceMaxConstantBufferSize(devices[i]);
//   json_object_object_add(Device, "max_constant_buffer_size", json_object_new_uint64(maxConstantBufferSize));
//
//   // Max Global Variable Size
//   size_t maxGlobalVariableSize = getDeviceMaxGlobalVariableSize(devices[i]);
//   json_object_object_add(Device, "max_global_variable_size", json_object_new_uint64(maxGlobalVariableSize));
//
//   // Max Mem Alloc Size
//   cl_ulong maxMemAllocSize = getDeviceMaxMemAllocSize(devices[i]);
//   json_object_object_add(Device, "max_mem_alloc_size", json_object_new_uint64(maxMemAllocSize));
//
//   // Max Num Sub Groups
//   cl_uint maxNumSubGroups = getDeviceMaxNumSubGroups(devices[i]);
//   json_object_object_add(Device, "max_num_sub_groups", json_object_new_uint64(maxNumSubGroups));
//
//   // Max On Device Events
//   cl_uint maxOnDeviceEvents = getDeviceMaxOnDeviceEvents(devices[i]);
//   json_object_object_add(Device, "max_on_device_events", json_object_new_uint64(maxOnDeviceEvents));
//
//   // Max On Device Queues
//   cl_uint maxOnDeviceQueues = getDeviceMaxOnDeviceQueues(devices[i]);
//   json_object_object_add(Device, "max_on_device_queues", json_object_new_uint64(maxOnDeviceQueues));
//
//   // Max Parameter Size
//   size_t maxParameterSize = getDeviceMaxParameterSize(devices[i]);
//   json_object_object_add(Device, "max_parameter_size", json_object_new_uint64(maxParameterSize));
//
//   // Max Pipe Args
//   cl_uint maxPipeArgs = getDeviceMaxPipeArgs(devices[i]);
//   json_object_object_add(Device, "max_pipe_args", json_object_new_uint64(maxPipeArgs));
//
//   // Max Read Image Args
//   cl_uint maxReadImageArgs = getDeviceMaxReadImageArgs(devices[i]);
//   json_object_object_add(Device, "max_read_image_args", json_object_new_uint64(maxReadImageArgs));
//
//   // Max Read Write Image Args
//   cl_uint maxReadWriteImageArgs = getDeviceMaxReadWriteImageArgs(devices[i]);
//   json_object_object_add(Device, "max_read_write_image_args", json_object_new_uint64(maxReadWriteImageArgs));
//
//   // Max Samplers
//   cl_uint maxSamplers = getDeviceMaxSamplers(devices[i]);
//   json_object_object_add(Device, "max_samplers", json_object_new_uint64(maxSamplers));
//
//   // Max Work Group Size
//   size_t maxWorkGroupSize = getDeviceMaxWorkGroupSize(devices[i]);
//   json_object_object_add(Device, "max_work_group_size", json_object_new_uint64(maxWorkGroupSize));
//
//   // Max Work Item Dimensions
//   cl_uint maxWorkItemDimensions = getDeviceMaxComputeUnits(devices[i]);
//   json_object_object_add(Device, "max_work_item_dimensions", json_object_new_uint64(maxWorkItemDimensions));
//
//   // Max Work Item Sizes
//   char buffer[12];
//   size_t numDims;
//   size_t *maxWorkItemSizes = getDeviceMaxWorkItemSizes(devices[i], &numDims);
//   json_object *Sizes = json_object_new_array();
//   for (int i = 0; i < numDims; i++) {
//     json_object *Size = json_object_new_object();
//     sprintf(buffer, "%d", i);
//     json_object_object_add(Size, (char *)buffer, json_object_new_uint64(maxWorkItemSizes[i]));
//     json_object_array_add(Sizes, Size);
//   }
//   json_object_object_add(Device, "max_work_item_sizes", Sizes);
//
//   // Max Write Image Args
//   cl_uint maxWriteImageArgs = getDeviceMaxWriteImageArgs(devices[i]);
//   json_object_object_add(Device, "max_write_image_args", json_object_new_uint64(maxWriteImageArgs));
//
//   // Mem Base AddrA lign
//   cl_uint memBaseAddrAlign = getDeviceMemBaseAddrAlign(devices[i]);
//   json_object_object_add(Device, "mem_base_addr_align ", json_object_new_uint64(memBaseAddrAlign));
//
//   // Mem Cache Type
//   getProperties(getDeviceMemCacheType(devices[i]), "mem_cache_type", Device);
//
//   // Native Vector Width Char
//   cl_uint nativeVectorWidthChar = getDeviceNativeVectorWidthChar(devices[i]);
//   json_object_object_add(Device, "native_vector_width_char", json_object_new_uint64(nativeVectorWidthChar));
//
//   // Native Vector Width Double
//   cl_uint nativeVectorWidthDouble = getDeviceNativeVectorWidthDouble(devices[i]);
//   json_object_object_add(Device, "native_vector_width_double", json_object_new_uint64(nativeVectorWidthDouble));
//
//   // Native Vector Width Float
//   cl_uint nativeVectorWidthFloat = getDeviceNativeVectorWidthFloat(devices[i]);
//   json_object_object_add(Device, "native_vector_width_float", json_object_new_uint64(nativeVectorWidthFloat));
//
//   // Native Vector Width Half
//   cl_uint nativeVectorWidthHalf = getDeviceNativeVectorWidthHalf(devices[i]);
//   json_object_object_add(Device, "native_vector_width_half", json_object_new_uint64(nativeVectorWidthHalf));
//
//   // Native Vector Width Int
//   cl_uint nativeVectorWidthInt = getDeviceNativeVectorWidthInt(devices[i]);
//   json_object_object_add(Device, "native_vector_width_int", json_object_new_uint64(nativeVectorWidthInt));
//
//   // Native Vector Width Long
//   cl_uint nativeVectorWidthLong = getDeviceNativeVectorWidthLong(devices[i]);
//   json_object_object_add(Device, "native_vector_width_long", json_object_new_uint64(nativeVectorWidthLong));
//
//   // Native Vector Width Short
//   cl_uint nativeVectorWidthShort = getDeviceNativeVectorWidthShort(devices[i]);
//   json_object_object_add(Device, "native_vector_width_short", json_object_new_uint64(nativeVectorWidthShort));
//
//   // Non Uniform Work Group Support
//   cl_uint nonUniformWorkGroupSupport = getDeviceNonUniformWorkGroupSupport(devices[i]);
//   json_object_object_add(Device, "non_uniform_work_group_support", json_object_new_boolean(nonUniformWorkGroupSupport));
//
//   // Numeric Version
//   cl_uint numericVersion = getDeviceNumericVersion(devices[i]);
//   json_object_object_add(Device, "numeric_version", json_object_new_string(versionStr(numericVersion)));
//
//   // OpenCl C All Versions
//   size_t numC;
//   cl_name_version *cs = getDeviceOpenClCAllVersions(devices[i], &numC);
//   json_object *Cs = json_object_new_array();
//   for (int i = 0; i < numC; i++) {
//     json_object *C = json_object_new_object();
//     json_object_object_add(C, "name", json_object_new_string(cs[i].name));
//     json_object_object_add(C, "version", json_object_new_string(versionStr(cs[i].version)));
//     json_object_array_add(Cs, C);
//   }
//   json_object_object_add(Device, "opencl_c_all_versions", Cs);
//
//   // OpenCl C Features
//   size_t numCFeatures;
//   cl_name_version *cfs = getDeviceOpenClCFeatures(devices[i], &numCFeatures);
//   json_object *Cfs = json_object_new_array();
//   for (int i = 0; i < numCFeatures; i++) {
//     json_object *Cf = json_object_new_object();
//     json_object_object_add(Cf, "name", json_object_new_string(cfs[i].name));
//     json_object_object_add(Cf, "version", json_object_new_string(versionStr(cfs[i].version)));
//     json_object_array_add(Cfs, Cf);
//   }
//   json_object_object_add(Device, "opencl_c_features", Cfs);
//
//   // Parent ID
//   cl_device_id parentId = getDeviceParentId(devices[i]);
//   json_object_object_add(Device, "parent_id", json_object_new_uint64((intptr_t)parentId));
//
//   // Partition Max SubDevices
//   cl_uint partitionMaxSubDevices = getDevicePartitionMaxSubDevices(devices[i]);
//   json_object_object_add(Device, "partition_max_sub_devices", json_object_new_uint64(partitionMaxSubDevices));
//
//   // Partition Properties
//   getProperties(getDevicePartitionProperties(devices[i]), "partition_properties", Device);
//
//   // Partition Types
//   getProperties(getDevicePartitionType(devices[i]), "partition_types", Device);
//
//   // Pipe Max Active Reservations
//   cl_uint pipeMaxActiveReservations = getDevicePipeMaxActiveReservations(devices[i]);
//   json_object_object_add(Device, "pipe_max_active_reservations", json_object_new_uint64(pipeMaxActiveReservations));
//
//   // Pipe Max Packet Size
//   cl_uint pipeMaxPacketSize = getDevicePipeMaxPacketSize(devices[i]);
//   json_object_object_add(Device, "pipe_max_packet_size ", json_object_new_uint64(pipeMaxPacketSize));
//
//   // Pipe Support
//   cl_uint pipeSupport = getDevicePipeSupport(devices[i]);
//   json_object_object_add(Device, "pipe_support", json_object_new_boolean(pipeSupport));
//
//   // Preferred Global Atomic Alignment
//   cl_uint preferredGlobalAtomicAlignment = getDevicePreferredGlobalAtomicAlignment(devices[i]);
//   json_object_object_add(Device, "preferred_global_atomic_alignment", json_object_new_uint64(preferredGlobalAtomicAlignment));
//
//   // Preferred Interop User Sync
//   cl_uint preferredInteropUserSync = getDevicePreferredInteropUserSync(devices[i]);
//   json_object_object_add(Device, "preferred_interop_user_sync", json_object_new_boolean(preferredInteropUserSync));
//
//   // Preferred Local Atomic Alignment
//   cl_uint preferredLocalAtomicAlignment = getDevicePreferredLocalAtomicAlignment(devices[i]);
//   json_object_object_add(Device, "preferred_local_atomic_alignment", json_object_new_uint64(preferredLocalAtomicAlignment));
//
//   // Preferred Platform Atomic Alignment
//   cl_uint preferredPlatformAtomicAlignment = getDevicePreferredPlatformAtomicAlignment(devices[i]);
//   json_object_object_add(Device, "preferred_platform_atomic_alignment", json_object_new_uint64(preferredPlatformAtomicAlignment));
//
//   // Preferred Vector Width Char
//   cl_uint preferredVectorWidthChar = getDevicePreferredVectorWidthChar(devices[i]);
//   json_object_object_add(Device, "preferred_vector_width_char", json_object_new_uint64(preferredVectorWidthChar));
//
//   // Preferred Vector Width Double
//   cl_uint preferredVectorWidthDouble = getDevicePreferredVectorWidthChar(devices[i]);
//   json_object_object_add(Device, "preferred_vector_width_double", json_object_new_uint64(preferredVectorWidthDouble));
//
//   // Preferred Vector Width Float
//   cl_uint preferredVectorWidthFloat = getDevicePreferredVectorWidthChar(devices[i]);
//   json_object_object_add(Device, "preferred_vector_width_float", json_object_new_uint64(preferredVectorWidthFloat));
//
//   // Preferred Vector Width Half
//   cl_uint preferredVectorWidthHalf = getDevicePreferredVectorWidthHalf(devices[i]);
//   json_object_object_add(Device, "preferred_vector_width_half", json_object_new_uint64(preferredVectorWidthHalf));
//
//   // Preferred Vector Width Int
//   cl_uint preferredVectorWidthInt = getDevicePreferredVectorWidthChar(devices[i]);
//   json_object_object_add(Device, "preferred_vector_width_int", json_object_new_uint64(preferredVectorWidthInt));
//
//   // Preferred Vector Width Long
//   cl_uint preferredVectorWidthLong = getDevicePreferredVectorWidthChar(devices[i]);
//   json_object_object_add(Device, "preferred_vector_width_long", json_object_new_uint64(preferredVectorWidthLong));
//
//   // Preferred Vector Width Short
//   cl_uint preferredVectorWidthShort = getDevicePreferredVectorWidthChar(devices[i]);
//   json_object_object_add(Device, "preferred_vector_width_short", json_object_new_uint64(preferredVectorWidthShort));
//
//   // Preferred Work Group Size Multiple
//   size_t preferredWorkGroupSizeMultiple = getDevicePreferredWorkGroupSizeMultiple(devices[i]);
//   json_object_object_add(Device, "preferred_work_group_size_multiple", json_object_new_uint64(preferredWorkGroupSizeMultiple));
//
//   // Printf Buffer Size
//   size_t printfBufferSize = getDevicePrintfBufferSize(devices[i]);
//   json_object_object_add(Device, "printf_buffer_size", json_object_new_uint64(printfBufferSize));
//
//   // Profiling Timer Resolution
//   size_t profilingTimerResolution = getDeviceProfilingTimerResolution(devices[i]);
//   json_object_object_add(Device, "profiling_timer_resolution", json_object_new_uint64(profilingTimerResolution));
//
//   // Queue On Device MaxSize
//   cl_uint queueOnDeviceMaxSize = getDeviceQueueOnDeviceMaxSize(devices[i]);
//   json_object_object_add(Device, "queue_on_device_max_size", json_object_new_uint64(queueOnDeviceMaxSize));
//
//   // Queue On Device Properties
//   getProperties(getDeviceQueueOnDeviceProperties(devices[i]), "queue_on_device_properties", Device);
//
//   // Queue On Device Properties
//   getProperties(getDeviceQueueOnHostProperties(devices[i]), "queue_on_host_properties", Device);
//
//   // Queue On Device Preferred Size
//   cl_uint queueOnDevicePreferredSize = getDeviceQueueOnDevicePreferredSize(devices[i]);
//   json_object_object_add(Device, "queue_on_device_preferred_size", json_object_new_uint64(queueOnDevicePreferredSize));
//
//   // Reference Count
//   cl_uint referenceCount = getDeviceReferenceCount(devices[i]);
//   json_object_object_add(Device, "reference_count", json_object_new_uint64(referenceCount));
//
//   // Single FP Config
//   getProperties(getDeviceSingleFPConfig(devices[i]), "single_fp_config", Device);
//
//   // Sub Group Independent Forward Progress
//   cl_uint subGroupIndependentForwardProgress = getDeviceSubGroupIndependentForwardProgress(devices[i]);
//   json_object_object_add(Device, "sub_group_independent_forward_progress", json_object_new_boolean(subGroupIndependentForwardProgress));
//
//   // SVM Capabilities
//   getProperties(getDeviceSVMCapabilities(devices[i]), "svm_capabilities", Device);
//
//   // Vendor ID
//   cl_uint vendorID = getDeviceVendorID(devices[i]);
//   json_object_object_add(Device, "vendor_id", json_object_new_uint64(vendorID));
//
//   // Work Group Collective Functions Support
//   cl_uint workGroupCollectiveFunctionsSupport = getDeviceWorkGroupCollectiveFunctionsSupport(devices[i]);
//   json_object_object_add(Device, "work_group_collective_functions_support", json_object_new_boolean(workGroupCollectiveFunctionsSupport));
//
//   json_object_array_add(Devices, Device);
// }
// }
