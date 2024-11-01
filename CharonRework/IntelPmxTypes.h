#pragma once
#include <Windows.h>

namespace DataTypes
{
	namespace IntelPmxTypes
	{
		//================================================================================
		// Defines
		//================================================================================
		constexpr auto CONFIG_ADDRESS = 0xcf8;
		constexpr auto CONFIG_DATA = 0xcfc;
		constexpr auto MAX_DEVICE_FUNCTIONS = 8;
		constexpr auto MAX_LPC_FUNCTION = 31;

		//================================================================================
		// Unions
		//================================================================================
		union LARGE32
		{
			struct
			{
				WORD low;
				WORD high;
			};
			DWORD value;
		};

		//================================================================================
		// Enums
		//================================================================================
		enum class INTEL_PMX_IOCTLS : DWORD
		{
			Ia32eIoctl = 0x222a80,
			IoPortIoctl = 0x222aa4, //Ia32eIoctl + 36,
			MapPhysicalMemoryIoctl = 0x222ab8, //IoPortIoctl + 4 + 16,
			UnMapPhysicalMemoryIoctl = 0x222abc //MapPhysicalMemoryIoctl + 4
		};

		enum class INTEL_PMX_IO_PORT_OPERATION : DWORD
		{
			ReadByte = 1,
			ReadWord = 2,
			ReadDword = 3,
			WriteByte = 4,
			WriteWord = 5,
			WriteDword = 6
		};

		//================================================================================
		// Structures
		//================================================================================
		template <typename T>
		struct INTEL_PMX_INPUT_BUFFER
		{
			T* Data;
			DWORD Size;
			DWORD Padding;
		}; 
		
		struct INTEL_PMX_DATA_HEADER
		{
			DWORD dwSize;
		};

		struct INTEL_PMX_IO_PORT : INTEL_PMX_DATA_HEADER
		{
			INTEL_PMX_IO_PORT_OPERATION IOPortOperation;
			WORD IOPortInput;
			LARGE32 IOPortValue;
		};

#include <pshpack1.h>
		struct INTEL_PMX_MAP_MEMORY_TO_UM : INTEL_PMX_DATA_HEADER
		{
			LARGE_INTEGER SectionOffset;
			DWORD CommitSize;
			DWORD64 Result;
			DWORD Padding;
		};
#include <poppack.h>

		struct INTEL_PMX_UNMAP_MEMORY_FROM_UM : INTEL_PMX_DATA_HEADER
		{
			DWORD Padding[2];
			PVOID MappedAddress;
		};
	}
}


/// EOF