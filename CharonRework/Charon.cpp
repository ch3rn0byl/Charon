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

	if (!IsProcessorSupported())
	{
		throw std::runtime_error("Processor is not supported.");
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
	//
	// If memory was mapped, unmap it to release system resources.
	//
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

_Use_decl_annotations_
bool Charon::DumpFullSpiFlashMemory(const std::wstring& OutputFile)
{
	bool fStatus = false;

	UefiTypes::PSPI_MEMORY_MAPPED pSpiFlashMemory = nullptr;
	std::unique_ptr<BYTE[]> pUefiRegionAsBytes = nullptr;

	UefiTypes::BIOS_SPI_BAR0 bar0 = { 0 };

	//
	// The I/O port address can be queried at Device 31, Function 5, and Offset 16. This will give 
	// the bar0 address for the SPI flash memory.
	//
	fStatus = ReadIOPort(GetPCIValue(0, 31, 5, 16), &bar0.AsULong);
	if (!fStatus)
	{
		return fStatus;
	}

	//
	// Check to see if the memory space indicator is hardwired to zero. If it isn't, this means
	// this bar is not a memory BAR, as per 7.1.5 SPI BAR0 MMIO of Intel's datasheet volume 2 of 2.
	//
	if (bar0.MemorySpace != 0)
	{
		return false;
	}

	//
	// If bar0.MEMSIZE is 0, this means 4kb of memory space is indicated.
	//
	DWORD dwSpiBaseAddress = bar0.MemoryBar << 12;
	DWORD dwSpiSize = 1 << 12;

	//
	// Map the physical memory into the process's address space as read/write.
	//
	fStatus = MapPhysicalMemory(dwSpiBaseAddress, dwSpiSize);
	if (!fStatus)
	{
		return fStatus;
	}

	pSpiFlashMemory = reinterpret_cast<UefiTypes::PSPI_MEMORY_MAPPED>(m_pMappedMemory);

	//
	// Get the size of the SPI flash memory
	//
	DWORD dwUefiRegionSize = 0;

	//
	// Calculate the size of the UEFI region.
	// 
	dwUefiRegionSize = pSpiFlashMemory->BiosFlashPrimaryRegion.BiosFlashPrimaryRegionLimit;
	dwUefiRegionSize = ((dwUefiRegionSize << 12) | 0xfff) + 1;

	pUefiRegionAsBytes = std::make_unique<BYTE[]>(dwUefiRegionSize);

	//
	// Make sure HardwareSequencingFlashStatusAndControl.FDV is enabled. If this value is not
	// 1, software cannot use the Hardware Sequencing registers, but must use the software 
	// sequencing registers. Any attempt to use the Hardware Sequencing registers will result 
	// in the FCERR bit being set. 
	//
	if (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FDV != 1)
	{
		return false;
	}

	//
	// Check if the SPI Cycle is in progress. If it is, wait for it to complete. Ghetto 
	// polling, ftw!
	//
	if (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.SpiCycleInProgress)
	{
		while (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.SpiCycleInProgress)
		{
		}
	}

	//
	// Set the flash linear address to 0 to begin reading the flash memory.
	//
	pSpiFlashMemory->FlashAddress.FlashLinearAddress = 0;

	//
	// Clear the FDONE, FERR, and AEL bits
	//
	pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleDone = 0;
	pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleError = 0;
	pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.AccessErrorLog = 0;

	//
	// Iterate through the different regions.
	//
	for (DWORD dwRegionIndex = 0; dwRegionIndex < ((sizeof(pSpiFlashMemory->FlashRegion) / sizeof(pSpiFlashMemory->FlashRegion[0])) << 1); dwRegionIndex++)
	{
		UefiTypes::BIOS_FREG BiosFreg = pSpiFlashMemory->FlashRegion[dwRegionIndex];

		DWORD dwRegionBase = BiosFreg.RegionBase;
		DWORD dwRegionLimit = BiosFreg.RegionLimit;

		//
		// Since the memory space is 0, this means the size can be calculated by adjusting 
		// it to a 4kb boundary. 
		// 
		dwRegionBase <<= 12;

		//
		// The limit is the end of the region. We need to adjust it to a 4kb boundary, 
		// plus 1. 
		//
		dwRegionLimit = ((dwRegionLimit << 12) | 0xfff) + 1;

		//
		// If the region base is greater than the region limit, this means the region is
		// not valid. 
		//
		if (dwRegionBase > dwRegionLimit)
		{
			continue;
		}

		//
		// Set the flash linear address to the start address of the UEFI region.
		//
		pSpiFlashMemory->FlashAddress.FlashLinearAddress = dwRegionBase;

		do
		{
			std::printf("[+] Reading %04x of %04x\r", pSpiFlashMemory->FlashAddress.FlashLinearAddress, dwRegionLimit);

			//
			// The contents of this register are 0s based with 0b representing 1 byte 
			// and 3fh representing 64 bytes.
			//
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FDBC = 64 - 1;

			//
			// Tell the SPI controller to read from flash memory.
			//
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FCYCLE = static_cast<DWORD>(UefiTypes::HSFSTS_CYCLE::Read);

			//
			// Tell the SPI flash controller to initiate the flash cycle. 
			//
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FGO = 1;

			while (pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleDone != 1)
			{
				//
				// Poll until the cycle completes.
				//
			}

			//
			// If no errors occured, start copying the data from the flash memory to 
			// the address space of the process.
			//
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
			//
			// Clear the FDONE, FERR, and AEL bits
			//
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleDone = 0;
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.FlashCycleError = 0;
			pSpiFlashMemory->HardwareSequencingFlashStatusAndControl.AccessErrorLog = 0;

			//
			// Increment the flash linear address by 64 bytes.
			//
			pSpiFlashMemory->FlashAddress.FlashLinearAddress += UefiTypes::SPI_MAX_SIZE;
		} while (pSpiFlashMemory->FlashAddress.FlashLinearAddress < dwRegionLimit);
	}

	//
	// Write the UEFI region to a file specified by the user.
	//
	return WriteToFile(OutputFile, pUefiRegionAsBytes.get(), dwUefiRegionSize);
}

bool Charon::IsProcessorSupported()
{
	std::string VendorId(Cpuid::MAX_CPUID_BUFFER, '\0');
	std::string ProcessorBrand(Cpuid::MAX_CPUID_BUFFER, '\0');

	Cpuid::CPUID_BASIC_INFORMATION BasicInformation = { 0 };

	__cpuidex(reinterpret_cast<int*>(&BasicInformation), static_cast<int>(Cpuid::CPUID_LEAF::BASIC_INFORMATION), 0);

	*reinterpret_cast<int*>(&VendorId[0]) = BasicInformation.ebx;
	*reinterpret_cast<int*>(&VendorId[4]) = BasicInformation.edx;
	*reinterpret_cast<int*>(&VendorId[8]) = BasicInformation.ecx;

	return VendorId == "GenuineIntel";
}

_Use_decl_annotations_
bool Charon::ReadIOPort(DWORD IOPortData, PDWORD IOPortReadData)
{
	return m_PmxClient->ReadIOPort(IOPortData, IOPortReadData);
}

_Use_decl_annotations_
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

_Use_decl_annotations_
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


/// EOF