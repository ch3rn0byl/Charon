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

	/// <summary>
	/// Retrieves the name/model of the system's processor. This method queries the
	/// hardware information to obtain the processor's model.
	/// </summary>
	/// <returns>The string containing the processor's name (e.g., 12th Gen Intel(R) Core(TM) i7-1270PE).</returns>
	std::string GetProcessorInformation();

	/// <summary>
	/// Extracts and dumps the complete UEFI firmware image from the SPI flash memory
	/// to a specified output file.
	/// </summary>
	/// <param name="OutputToFile">The full path and filename where the UEFI firmware image will be saved. Must be a valid path with write permissions.</param>
	/// <returns>Returns true if the firmware was successfully dumped to the specified file; false if the operation failed (e.g., insufficient permissions or I/O error)</returns>
	bool DumpFullSpiFlashMemory(_In_ const std::wstring& OutputToFile);

	Charon(const Charon&) = delete;
	Charon& operator=(const Charon&) = delete;

	Charon(Charon&&) = delete;
	Charon& operator=(Charon&&) = delete;

private:
	std::unique_ptr<Privileges> m_Privileges;
	std::unique_ptr<ServiceController> m_ServiceController;
	std::unique_ptr<UefiAcquisition::IntelPmx> m_PmxClient;

	PVOID m_pMappedMemory;

	/// <summary>
	/// Verifies if the processor is manufactured by Intel by checking the CPUID 
	/// manufacturuer ID string. This method uses CPUID instruction (leaf 0) to 
	/// retrieve the processor's vendor identification string.
	/// </summary>
	/// <returns>Returns true if the processor is Intel (GenuineIntel); false if the endpoint is running an AMD, or any other processor.</returns>
	bool IsProcessorSupported();

	/// <summary>
	/// Reads from the specified I/O port to retrieve the SPI flash controller's 
	/// BAR0 physical address. This address is required for memory mapping of the 
	/// SPI flash controller. 
	/// </summary>
	/// <param name="IOPortData">The I/O port address to read from. This should be the correct port address for accessing the SPI flash controller's BAR0.</param>
	/// <param name="IOPortReadData">Pointer to receive the physical address read from BAR0. This address can be used for subsequent memory mapping operations.</param>
	/// <returns>Returns true if the I/O port read operation was successful and the BAR0 address was retrieved; false if the operation failed.</returns>
	bool ReadIOPort(_In_ DWORD IOPortData, _Out_ PDWORD IOPortReadData);

	/// <summary>
	/// Maps physical memory into the process' virtual address space. This method 
	/// allows direct access to physical memory regions by creating a virtual memory
	/// mapping. Commonly used to access memory-mapped hardware registers or device 
	/// memory. Used for hardware sequencing.
	/// </summary>
	/// <param name="MemoryToMap">The physical memory address to map. This should be a valid physical address, typically obtained from hardware registeres like BAR.</param>
	/// <param name="NumberOfBytesToMap">The size (in bytes) of the physical memory region to map into virtual memory. Must be greater than 0.</param>
	/// <returns>Returns true if the physical memory was successfully mapped into virtual address space; false if the mapping operation failed (e.g., invalid address, insufficient permissions, or invalid size.</returns>
	bool MapPhysicalMemory(
		_In_reads_bytes_(NumberOfBytesToMap) DWORD64 MemoryToMap, 
		_In_ DWORD NumberOfBytesToMap
	);

	/// <summary>
	/// Unmaps the previously mapped physical memory from the process' virtual 
	/// address space. This method releases the virtual memory mapping created by
	/// MapPhysicalMemory(), freeing system resources and removing access to the 
	/// previously mapped physical memory.
	/// </summary>
	/// <returns>Returns true if the memory was successfully unmapped; false if the unmapping operation failed or if no memory was previously mapped.</returns>
	bool UnmapMappedMemory();

	/// <summary>
	/// Calculates the I/O port address for a specific PCIe device using the bus, 
	/// device, function, and offset parameters. 
	/// </summary>
	/// <param name="Bus">The PCI bus number where the target device is located.</param>
	/// <param name="Device">The device number on the specified bus.</param>
	/// <param name="Function">The function number.</param>
	/// <param name="Offset">The offset within the device's configuration space.</param>
	/// <returns></returns>
	DWORD GetPCIValue(
		_In_ UINT16 Bus,
		_In_ UINT16 Device,
		_In_ UINT16 Function,
		_In_ UINT16 Offset
	);

	/// <summary>
	/// Write the extracted UEFI firmware image to a specified output file. This method
	/// handles the file I/O operations for saving the dumped UEFI firmware image to disk.
	/// </summary>
	/// <param name="OutputFile">The full path and filename where the UEFI firmware image will be saved on disk.</param>
	/// <param name="pUefiRegionAsBytes">Pointer to the buffer containing the UEFI firmware image data to be written.</param>
	/// <param name="dwNumberOfBytesToWrite">The size (in bytes) of the UEFI firmware image data to write to the file.</param>
	/// <returns>Returns true if the file was successfully written; false if the operation failed (e.g., insufficient disk space, permission denied, or other I/O error).</returns>
	bool WriteToFile(
		_In_ const std::wstring& OutputFile, 
		_In_reads_bytes_(dwNumberOfBytesToWrite) const PVOID pUefiRegionAsBytes, 
		_In_ DWORD dwNumberOfBytesToWrite
	);
};


/// EOF