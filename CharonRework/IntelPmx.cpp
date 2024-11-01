#include "IntelPmx.h"

UefiAcquisition::IntelPmx::IntelPmx() : 
#ifdef UNICODE
	BaseDriver(L"\\\\.\\Pmxdrv")
#else
	BaseDriver("\\\\.\\Pmxdrv")
#endif // UNICODE
{
}

UefiAcquisition::IntelPmx::~IntelPmx()
{
}

std::string UefiAcquisition::IntelPmx::GetProcessorInformation()
{
	//
		// CPUID uses the registers eax, ebx, ecx, and edx to transfer the output.
		// 
	std::string szArchitecture(4 * 4 * 4, '\0');

	//
	// 0x80000002, 0x80000003, and 0x80000004 are the leafs for getting the 
	// processor information.
	// 
	DWORD dwProcessorInformationLeaf = 0x80000002;
	DWORD dwOffset = NULL;

	for (DWORD temp = dwProcessorInformationLeaf;
		temp < dwProcessorInformationLeaf + 3;
		temp++)
	{
		int counter = NULL;

		do
		{
			int BasicInfo[4] = { 0 };

			__cpuidex(BasicInfo, temp, NULL);

			*reinterpret_cast<int*>(&szArchitecture[dwOffset]) = BasicInfo[counter];

			dwOffset += 4;
			counter++;
		} while (counter < 4);
	}

	//
	// Resize the string.
	// 
	szArchitecture.resize(strlen(szArchitecture.c_str()));

	//
	// Iterate through the string and check for double spaces. Doesn't look great
	// when it's printed out.
	// 
	for (size_t i = 0; i < szArchitecture.size(); i++)
	{
		if (szArchitecture[i] == 0x20 &&
			szArchitecture[i + 1] == 0x20)
		{
			szArchitecture.erase(i, 1);
		}
	}

	return szArchitecture;
}

_Use_decl_annotations_
bool UefiAcquisition::IntelPmx::ReadIOPort(DWORD IOPortData, PDWORD IOPortReadData)
{
	bool fStatus = false;

	IntelPmxTypes::INTEL_PMX_INPUT_BUFFER<IntelPmxTypes::INTEL_PMX_IO_PORT> InputBuffer = { 0 };

	InputBuffer.Size = sizeof(InputBuffer) + sizeof(IntelPmxTypes::INTEL_PMX_IO_PORT);
	InputBuffer.Data = new IntelPmxTypes::INTEL_PMX_IO_PORT();

	if (InputBuffer.Data == nullptr)
	{
		return fStatus;
	}

	InputBuffer.Data->dwSize = sizeof(IntelPmxTypes::INTEL_PMX_IO_PORT);
	InputBuffer.Data->IOPortOperation = IntelPmxTypes::INTEL_PMX_IO_PORT_OPERATION::WriteDword;
	InputBuffer.Data->IOPortInput = IntelPmxTypes::CONFIG_ADDRESS;
	InputBuffer.Data->IOPortValue.value = IOPortData;

	fStatus = SendIoControlRequest(static_cast<DWORD>(IntelPmxTypes::INTEL_PMX_IOCTLS::IoPortIoctl), &InputBuffer, sizeof(InputBuffer));
	if (fStatus)
	{
		InputBuffer.Data->IOPortOperation = IntelPmxTypes::INTEL_PMX_IO_PORT_OPERATION::ReadDword;
		InputBuffer.Data->IOPortInput = IntelPmxTypes::CONFIG_DATA;

		fStatus = SendIoControlRequest(static_cast<DWORD>(IntelPmxTypes::INTEL_PMX_IOCTLS::IoPortIoctl), &InputBuffer, sizeof(InputBuffer));
		if (fStatus)
		{
			if (InputBuffer.Data->IOPortValue.value != ~0ll)//0xffffffff)
			{
				*IOPortReadData = InputBuffer.Data->IOPortValue.value;
			}
			else
			{
				*IOPortReadData = NULL;
			}
		}
	}

	RtlSecureZeroMemory(InputBuffer.Data, sizeof(IntelPmxTypes::INTEL_PMX_IO_PORT));
	delete InputBuffer.Data;
	InputBuffer.Data = nullptr;

	return fStatus;
}

_Use_decl_annotations_
bool UefiAcquisition::IntelPmx::MapPhysicalMemory(DWORD64 MemoryToMap, DWORD NumberOfBytes, PVOID* MappedMemory)
{
	if (MappedMemory == nullptr)
	{
		return false;
	}

	*MappedMemory = nullptr;

	bool fStatus = false;

	IntelPmxTypes::INTEL_PMX_INPUT_BUFFER<IntelPmxTypes::INTEL_PMX_MAP_MEMORY_TO_UM> InputBuffer = { 0 };

	InputBuffer.Size = sizeof(InputBuffer) + sizeof(IntelPmxTypes::INTEL_PMX_MAP_MEMORY_TO_UM);
	InputBuffer.Data = new IntelPmxTypes::INTEL_PMX_MAP_MEMORY_TO_UM();

	if (InputBuffer.Data == nullptr)
	{
		return fStatus;
	}

	InputBuffer.Data->dwSize = 0x18; // sizeof(IntelPmxTypes::INTEL_PMX_MAP_MEMORY_TO_UM);
	InputBuffer.Data->SectionOffset.QuadPart = MemoryToMap;
	InputBuffer.Data->CommitSize = NumberOfBytes;

	fStatus = SendIoControlRequest(static_cast<DWORD>(IntelPmxTypes::INTEL_PMX_IOCTLS::MapPhysicalMemoryIoctl), &InputBuffer, sizeof(InputBuffer));
	if (fStatus)
	{
		*MappedMemory = reinterpret_cast<PVOID>(InputBuffer.Data->Result);
	}

	RtlSecureZeroMemory(InputBuffer.Data, sizeof(IntelPmxTypes::INTEL_PMX_MAP_MEMORY_TO_UM));
	delete InputBuffer.Data;
	InputBuffer.Data = nullptr;

	return fStatus;
}

_Use_decl_annotations_
bool UefiAcquisition::IntelPmx::UnmapMappedMemory(PVOID MappedAddress)
{
	bool fStatus = false;

	IntelPmxTypes::INTEL_PMX_INPUT_BUFFER<IntelPmxTypes::INTEL_PMX_UNMAP_MEMORY_FROM_UM> InputBuffer = { 0 };

	InputBuffer.Size = sizeof(InputBuffer) + sizeof(IntelPmxTypes::INTEL_PMX_UNMAP_MEMORY_FROM_UM);
	InputBuffer.Data = new IntelPmxTypes::INTEL_PMX_UNMAP_MEMORY_FROM_UM();

	if (InputBuffer.Data == nullptr)
	{
		return fStatus;
	}

	InputBuffer.Data->MappedAddress = MappedAddress;
	InputBuffer.Data->dwSize = sizeof(IntelPmxTypes::INTEL_PMX_UNMAP_MEMORY_FROM_UM);

	fStatus = SendIoControlRequest(static_cast<DWORD>(IntelPmxTypes::INTEL_PMX_IOCTLS::UnMapPhysicalMemoryIoctl), &InputBuffer, sizeof(InputBuffer));

	RtlSecureZeroMemory(InputBuffer.Data, sizeof(IntelPmxTypes::INTEL_PMX_UNMAP_MEMORY_FROM_UM));
	delete InputBuffer.Data;
	InputBuffer.Data = nullptr;

	return fStatus;
}
