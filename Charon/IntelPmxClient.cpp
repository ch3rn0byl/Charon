#include "IntelPmxClient.h"
#include <iostream>

std::string
IntelPmxClient::GetProcessorInformation()
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
bool
IntelPmxClient::ReadIOPort(
	UINT32 IOPortData,
	PUINT32 IOPortReadData
)
{
	bool bStatus = false;

	INPUT_BUFFER<IO_PORT_DATA> InputBuffer = { 0 };

	InputBuffer.Size = sizeof(InputBuffer) + sizeof(IO_PORT_DATA);
	InputBuffer.Data = new IO_PORT_DATA();

	if (InputBuffer.Data == nullptr)
	{
		return bStatus;
	}

	InputBuffer.Data->Size = sizeof(IO_PORT_DATA);
	InputBuffer.Data->IOPortOperation = IO_PORT_OPERATION::WriteDword;
	InputBuffer.Data->IOPortInput = CONFIG_ADDRESS;
	InputBuffer.Data->IOPortValue.value = IOPortData;

	//
	// Write to IO port 0xcf8 with the address you are wanting to query first.
	// 
	bStatus = SendIoRequest(IOPortIoctl, &InputBuffer, sizeof(InputBuffer));
	if (bStatus)
	{
		InputBuffer.Data->IOPortOperation = IO_PORT_OPERATION::ReadDword;
		InputBuffer.Data->IOPortInput = CONFIG_DATA;

		//
		// Read IO port 0xcfc for the output of the previous IO operation.
		// 
		bStatus = SendIoRequest(IOPortIoctl, &InputBuffer, sizeof(InputBuffer));
		if (bStatus)
		{
			if (InputBuffer.Data->IOPortValue.value != 0xffffffff)
			{
				*IOPortReadData = InputBuffer.Data->IOPortValue.value;
			}
			else
			{
				*IOPortReadData = NULL;
			}
		}
	}

	RtlSecureZeroMemory(InputBuffer.Data, sizeof(IO_PORT_DATA));
	delete InputBuffer.Data;
	InputBuffer.Data = nullptr;

	return bStatus;
}

_Use_decl_annotations_
bool
IntelPmxClient::MapPhysicalMemory(
	UINT64 MemoryToMap,
	PVOID* MappedMemory
)
{
	INPUT_BUFFER<MAPPED_MEMORY> InputBuffer = { 0 };

	bool bStatus = false;

	InputBuffer.Size = sizeof(InputBuffer) + sizeof(MAPPED_MEMORY);
	InputBuffer.Data = new MAPPED_MEMORY();

	if (InputBuffer.Data == nullptr)
	{
		return bStatus;
	}

	InputBuffer.Data->Size = 0x18;
	InputBuffer.Data->SectionOffset.QuadPart = MemoryToMap;
	InputBuffer.Data->CommitSize = 0x1;

	bStatus = SendIoRequest(
		MapPhysicalMemoryIoctl,
		&InputBuffer,
		sizeof(InputBuffer)
	);
	if (bStatus)
	{
		InputBuffer.Data->SectionOffset.QuadPart &= 0xfff;
		InputBuffer.Data->Result -= InputBuffer.Data->SectionOffset.QuadPart;

		*MappedMemory = reinterpret_cast<PVOID>(InputBuffer.Data->Result);
	}

	RtlSecureZeroMemory(InputBuffer.Data, sizeof(MAPPED_MEMORY));
	delete InputBuffer.Data;
	InputBuffer.Data = nullptr;

	return bStatus;
}

_Use_decl_annotations_
bool
IntelPmxClient::UnmapMappedMemory(
	PVOID MappedAddress
)
{
	bool bStatus = false;

	INPUT_BUFFER<UNMAP_MEMORY> InputBuffer = { 0 };

	InputBuffer.Size = sizeof(InputBuffer) + sizeof(UNMAP_MEMORY);
	InputBuffer.Data = new UNMAP_MEMORY();

	if (InputBuffer.Data == nullptr)
	{
		return bStatus;
	}

	InputBuffer.Data->Size = sizeof(UNMAP_MEMORY);
	InputBuffer.Data->Address = MappedAddress;

	bStatus = SendIoRequest(
		UnMapPhysicalMemoryIoctl,
		&InputBuffer,
		sizeof(InputBuffer)
	);

	RtlSecureZeroMemory(InputBuffer.Data, sizeof(UNMAP_MEMORY));
	delete InputBuffer.Data;
	InputBuffer.Data = nullptr;

	return bStatus;
}

_Use_decl_annotations_
UINT32
IntelPmxClient::GetPCIValue(
	UINT16 Bus,
	UINT16 Device,
	UINT16 Function,
	UINT16 Offset
)
{
	return 0x80000000 | Bus << 16 | Device << 11 | Function << 8 | Offset;
}


/// EOF