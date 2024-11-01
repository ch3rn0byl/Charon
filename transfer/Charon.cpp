#include "Charon.h"

Charon::Charon() : 
	m_Privileges(std::make_unique<Privileges>()),
	m_ServiceController(nullptr),
	m_PmxClient(nullptr),
	m_pMappedMemory(nullptr)
{
	if (!m_Privileges->CheckAdminPrivileges())
	{
		throw std::runtime_error("Administrative privileges are required. Please try again as Administrator.");
	}

	m_ServiceController = std::make_unique<ServiceController>();
	if (!m_ServiceController->StartKernelService())
	{
		throw std::runtime_error("Failed to start kernel service");
	}

	m_PmxClient = std::make_unique<UefiAcquisition::IntelPmx>();
}

Charon::~Charon()
{
	if (m_pMappedMemory != nullptr)
	{
		if (UnmapMappedMemory())
		{
			m_pMappedMemory = nullptr;
		}
	}
}

std::string Charon::GetProcessorInformation()
{
	return m_PmxClient->GetProcessorInformation();
}

bool Charon::DumpFullSpiFlashMemory(const std::wstring& OutputFile)
{
	bool fStatus = false;

	UefiTypes::PSPI_MEMORY_MAPPED pSpiFlashMemory = nullptr;
	std::unique_ptr<BYTE[]> pUefiRegionAsBytes = nullptr;

	UefiTypes::BIOS_SPI_BAR0 bar0 = { 0 };

	fStatus = ReadIOPort(GetPCIValue(0, 31, 5, 16), &bar0.AsULong);
	if (!fStatus)
	{
		return fStatus;
	}

	if (bar0.MemorySpace != 0)
	{
		return false;
	}

	DWORD dwSpiBaseAddress = bar0.MemoryBar << 12;
	DWORD dwSpiSize = 1 << 12;

	fStatus = MapPhysicalMemory(dwSpiBaseAddress, dwSpiSize);
	if (!fStatus)
	{
		return fStatus;
	}

	pSpiFlashMemory = reinterpret_cast<UefiTypes::PSPI_MEMORY_MAPPED>(m_pMappedMemory);

	// Get the size of the SPI flash memory
	DWORD dwUefiRegionSize = 0;

	dwUefiRegionSize = pSpiFlashMemory->BiosFlashPrimaryRegion.BiosFlashPrimaryRegionLimit;
	dwUefiRegionSize = ((dwUefiRegionSize << 12) | 0xfff) + 1;

	pUefiRegionAsBytes = std::make_unique<BYTE[]>(dwUefiRegionSize);

	// Make sure HardwareSequencingFlashStatusAndControl.FDV is enabled
	if (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FDV != 1)
	{
		return false;
	}

	// Check if the SPI Cycle is in progress
	if (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.SpiCycleInProgress)
	{
		while (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.SpiCycleInProgress)
		{
		}
	}

	pSpiFlashMemory->FlashAddress.FlashLinearAddress = 0;

	// Clear the FDONE, FERR, and AEL bits
	pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleDone = 0;
	pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleError = 0;
	pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.AccessErrorLog = 0;

	for (DWORD dwRegionIndex = 0; dwRegionIndex < ((sizeof(pSpiFlashMemory->FlashRegion) / sizeof(pSpiFlashMemory->FlashRegion[0])) << 1); dwRegionIndex++)
	{
		UefiTypes::BIOS_FREG BiosFreg = pSpiFlashMemory->FlashRegion[dwRegionIndex];

		DWORD dwRegionBase = BiosFreg.RegionBase;
		DWORD dwRegionLimit = BiosFreg.RegionLimit;

		dwRegionBase <<= 12;
		dwRegionLimit = ((dwRegionLimit << 12) | 0xfff) + 1;

		if (dwRegionBase > dwRegionLimit)
		{
			continue;
		}

		pSpiFlashMemory->FlashAddress.FlashLinearAddress = dwRegionBase;

		do
		{
			std::printf("[+] Reading %04x of %04x\r", pSpiFlashMemory->FlashAddress.FlashLinearAddress, dwRegionLimit);

			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FDBC = 64 - 1;
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FCYCLE = static_cast<DWORD>(UefiTypes::HSFSTS_CYCLE::Read);
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FGO = 1;

			while (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleDone != 1)
			{
				// Wait for the flash cycle to complete.
			}

			if (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleError == 0 &&
				pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.AccessErrorLog == 0 &&
				pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.SpiCycleInProgress == 0)
			{
				for (DWORD dwIndex = 0; dwIndex < sizeof(pSpiFlashMemory->FlashData) / sizeof(pSpiFlashMemory->FlashData[0]); dwIndex++)
				{
					RtlCopyMemory(
						&pUefiRegionAsBytes[(pSpiFlashMemory->FlashAddress.FlashLinearAddress) + (dwIndex * sizeof(DWORD))], 
						&pSpiFlashMemory->FlashData[dwIndex], 
						sizeof(DWORD)
					);
				}
			}
#ifdef _DEBUG
			// We should never see this error. 
			else
			{
				std::printf("\n[!] FlashCycleError occurred at %04x\n", pSpiFlashMemory->FlashAddress.FlashLinearAddress);
				system("pause");
			}
#endif // _DEBUG
			// Clear the FDONE, FERR, and AEL bits
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleDone = 0;
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleError = 0;
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.AccessErrorLog = 0;

			pSpiFlashMemory->FlashAddress.FlashLinearAddress += UefiTypes::SPI_MAX_SIZE;
		} while (pSpiFlashMemory->FlashAddress.FlashLinearAddress < dwRegionLimit);
	}

	return WriteToFile(OutputFile, pUefiRegionAsBytes.get(), dwUefiRegionSize);
	/*
	HANDLE hFile = CreateFile(OutputFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesWritten = 0;

		if (!WriteFile(hFile, pUefiRegionAsBytes.get(), dwUefiRegionSize, &dwBytesWritten, nullptr))
		{
			return false;
		}
	}
	*/

	/*
	fStatus = UnmapMappedMemory();
	if (!fStatus)
	{
		return fStatus;
	}

	return fStatus;
	*/
}

bool Charon::ReadIOPort(DWORD IOPortData, PDWORD IOPortReadData)
{
	return m_PmxClient->ReadIOPort(IOPortData, IOPortReadData);
}

bool Charon::MapPhysicalMemory(DWORD64 MemoryToMap, DWORD NumberOfBytesToMap)
{
	return m_PmxClient->MapPhysicalMemory(MemoryToMap, NumberOfBytesToMap , &m_pMappedMemory);
}

bool Charon::UnmapMappedMemory()
{
	return m_PmxClient->UnmapMappedMemory(m_pMappedMemory);
}

_Use_decl_annotations_
DWORD Charon::GetPCIValue(UINT16 Bus, UINT16 Device, UINT16 Function, UINT16 Offset)
{
	return 0x80000000 | Bus << 16 | Device << 11 | Function << 8 | Offset;
}

bool Charon::WriteToFile(const std::wstring& OutputFile, const PVOID pUefiRegionAsBytes, DWORD dwNumberOfBytesToWrite)
{
	DWORD dwBytesWritten = 0;
	bool fStatus = false;

	HANDLE hFile = CreateFile(OutputFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return fStatus;
	}

	fStatus = WriteFile(hFile, pUefiRegionAsBytes, dwNumberOfBytesToWrite, &dwBytesWritten, nullptr);

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	return fStatus;
}
