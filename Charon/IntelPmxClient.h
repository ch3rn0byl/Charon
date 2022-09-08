#pragma once
#include "BaseDriverClient.h"
#include "PmxTypes.h"

#include <string>

class IntelPmxClient :
    public BaseDriverClient
{
public:
    /// <summary>
    /// This method queries the processor information such as type, 
    /// clockspeed.
    /// </summary>
    /// <returns>std::string</returns>
    std::string GetProcessorInformation();

    /// <summary>
    /// This method will read IO ports.
    /// </summary>
    /// <returns>true if successful</returns>
    _Success_(return)
        _Must_inspect_result_
        bool ReadIOPort(
            _In_ UINT32 IOPortData,
            _Out_ PUINT32 IOPortReadData
        );

    /// <summary>
    /// This method will map a physical address into the process space
    /// of the calling process.
    /// </summary>
    /// <returns>true if successful</returns>
    _Success_(return)
        _Must_inspect_result_
        bool MapPhysicalMemory(
            _In_ UINT64 MemoryToMap,
            _Outptr_result_maybenull_ PVOID* MappedMemory
        );

    /// <summary>
    /// This method will unmap the mapped address.
    /// </summary>
    /// <param name="MappedAddress"></param>
    /// <returns>true if successful</returns>
    bool UnmapMappedMemory(
        _In_ PVOID MappedAddress
    );

    /// <summary>
    /// This method will convert bus information into a useable address
    /// for reading or writing to IO ports. 
    /// </summary>
    /// <param name="Bus"></param>
    /// <param name="Device"></param>
    /// <param name="Function"></param>
    /// <param name="Offset"></param>
    /// <returns>UINT32</returns>
    UINT32 GetPCIValue(
        _In_ UINT16 Bus,
        _In_ UINT16 Device,
        _In_ UINT16 Function,
        _In_ UINT16 Offset
    );
};


/// EOF