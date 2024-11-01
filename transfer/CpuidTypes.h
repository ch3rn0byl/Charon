#pragma once

namespace DataTypes
{
	namespace Cpuid
	{
		//================================================================================
		// Defines 
		//================================================================================
		using uint32_t = unsigned int;

		constexpr auto MAX_CPUID_BUFFER = 12 + 1;
		using PCPUID_BASIC_INFORMATION = struct CPUID_BASIC_INFORMATION*;

		//================================================================================
		// Structures
		//================================================================================
		struct CPUID_BASIC_INFORMATION
		{
			uint32_t eax;
			uint32_t ebx;
			uint32_t ecx;
			uint32_t edx;
		};
	}
}


/// EOF