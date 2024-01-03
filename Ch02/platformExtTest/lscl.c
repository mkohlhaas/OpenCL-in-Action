#include "device.h"
#include "deviceJson.h"
#include "platform.h"
#include "platformJson.h"
#include <CL/cl.h>

int main(void) {

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
      AddDevice_MaxComputeUnits(Device, device);
      AddDevice_MaxConstantArgs(Device, device);
      AddDevice_MaxClockFrequency(Device, device);
      AddDevice_MaxConstantBufferSize(Device, device);
      AddDevice_MaxGlobalVariableSize(Device, device);
      AddDevice_MaxMemAllocSize(Device, device);
      AddDevice_MaxNumSubGroups(Device, device);
      AddDevice_MaxOnDeviceEvents(Device, device);
      AddDevice_MaxOnDeviceQueues(Device, device);
      AddDevice_MaxParameterSize(Device, device);
      AddDevice_MaxPipeArgs(Device, device);
      AddDevice_MaxReadImageArgs(Device, device);
      AddDevice_MaxReadWriteImageArgs(Device, device);
      AddDevice_MaxSamplers(Device, device);
      AddDevice_MaxWorkGroupSize(Device, device);
      AddDevice_MaxComputeUnits(Device, device);
      AddDevice_MaxWriteImageArgs(Device, device);
      AddDevice_MemBaseAddrAlign(Device, device);
      AddDevice_MemCacheType(Device, device);
      AddDevice_NativeVectorWidthChar(Device, device);
      AddDevice_NativeVectorWidthDouble(Device, device);
      AddDevice_NativeVectorWidthFloat(Device, device);
      AddDevice_NativeVectorWidthHalf(Device, device);
      AddDevice_NativeVectorWidthInt(Device, device);
      AddDevice_NativeVectorWidthLong(Device, device);
      AddDevice_NativeVectorWidthShort(Device, device);
      AddDevice_NonUniformWorkGroupSupport(Device, device);
      AddDevice_NumericVersion(Device, device);
      AddDevice_OpenClCAllVersions(Device, device);
      AddDevice_OpenClCFeatures(Device, device);
      AddDevice_ParentId(Device, device);
      AddDevice_PartitionMaxSubDevices(Device, device);
      AddDevice_PartitionProperties(Device, device);
      AddDevice_PartitionType(Device, device);
      AddDevice_PipeMaxActiveReservations(Device, device);
      AddDevice_PipeMaxPacketSize(Device, device);
      AddDevice_PipeSupport(Device, device);
      AddDevice_PreferredGlobalAtomicAlignment(Device, device);
      AddDevice_PreferredInteropUserSync(Device, device);
      AddDevice_PreferredLocalAtomicAlignment(Device, device);
      AddDevice_PreferredPlatformAtomicAlignment(Device, device);
      AddDevice_PreferredVectorWidthChar(Device, device);
      AddDevice_PreferredVectorWidthChar(Device, device);
      AddDevice_PreferredVectorWidthChar(Device, device);
      AddDevice_PreferredVectorWidthHalf(Device, device);
      AddDevice_PreferredVectorWidthChar(Device, device);
      AddDevice_PreferredVectorWidthChar(Device, device);
      AddDevice_PreferredVectorWidthChar(Device, device);
      AddDevice_PreferredWorkGroupSizeMultiple(Device, device);
      AddDevice_PrintfBufferSize(Device, device);
      AddDevice_ProfilingTimerResolution(Device, device);
      AddDevice_QueueOnDeviceMaxSize(Device, device);
      AddDevice_QueueOnDeviceProperties(Device, device);
      AddDevice_QueueOnHostProperties(Device, device);
      AddDevice_QueueOnDevicePreferredSize(Device, device);
      AddDevice_ReferenceCount(Device, device);
      AddDevice_SubGroupIndependentForwardProgress(Device, device);
      AddDevice_SVMCapabilities(Device, device);
      AddDevice_VendorID(Device, device);
      AddDevice_WorkGroupCollectiveFunctionsSupport(Device, device);
    }
  }
  PrintRoot(Root);
}
