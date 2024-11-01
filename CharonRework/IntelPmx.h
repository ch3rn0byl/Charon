#pragma once
#include "BaseDriverClient.h"
#include "IntelPmxTypes.h"

using namespace DataTypes;

namespace UefiAcquisition
{
    class IntelPmx :
        public BaseDriver
    {
    public:
        IntelPmx();
        ~IntelPmx();

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
                _In_ DWORD IOPortData,
                _Out_ PDWORD IOPortReadData
            );

        /// <summary>
        /// This method will map a physical address into the process space
        /// of the calling process.
        /// </summary>
        /// <returns>true if successful</returns>
        _Success_(return)
            _Must_inspect_result_
            bool MapPhysicalMemory(
                _In_ DWORD64 MemoryToMap,
				_In_ DWORD NumberOfBytes,
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

    private:

    };

}
