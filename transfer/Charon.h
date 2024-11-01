#pragma once
#include "Privileges.h"
#include "ServiceController.h"
#include "IntelPmx.h"
#include "ErrorHandler.h"
#include "DataTypes.h"

class Charon
{
public:
	Charon();
	~Charon();

	std::string GetProcessorInformation();
	bool DumpFullSpiFlashMemory(const std::wstring& OutputToFile);

private:
	std::unique_ptr<Privileges> m_Privileges;
	std::unique_ptr<ServiceController> m_ServiceController;
	std::unique_ptr<UefiAcquisition::IntelPmx> m_PmxClient;

	PVOID m_pMappedMemory;

	bool ReadIOPort(DWORD IOPortData, PDWORD IOPortReadData);
	bool MapPhysicalMemory(DWORD64 MemoryToMap, DWORD NumberOfBytesToMap);
	bool UnmapMappedMemory();

	DWORD GetPCIValue(
		_In_ UINT16 Bus,
		_In_ UINT16 Device,
		_In_ UINT16 Function,
		_In_ UINT16 Offset
	);

	bool WriteToFile(const std::wstring& OutputFile, const PVOID pUefiRegionAsBytes, DWORD dwNumberOfBytesToWrite);
};