#pragma once
#include <minwindef.h>

///===================================================
/// Definitions
///===================================================
constexpr auto PROCESSOR_BRAND_LEAF = 0x80000002;
constexpr auto CONFIG_ADDRESS = 0xcf8;
constexpr auto CONFIG_DATA = 0xcfc;
constexpr auto MAX_DEVICE_FUNCTIONS = 8;
constexpr auto LPC_FUNCTION = 0x1f;

///===================================================
/// ENUMs
///===================================================
enum PMX_IOCTLS : DWORD32
{
    IA32EIoctl = 0x222a80,
    IOPortIoctl = IA32EIoctl + 36,
    MapPhysicalMemoryIoctl = IOPortIoctl + 4 + 16,
    UnMapPhysicalMemoryIoctl = MapPhysicalMemoryIoctl + 4
};

enum class IO_PORT_OPERATION : DWORD32
{
    ReadByte = 1,
    ReadWord = 2,
    ReadDword = 3,
    WriteByte = 4,
    WriteWord = 5,
    WriteDword = 6
};

enum FCYCLE_OPERATION : UINT16
{
    ReadBlock = 0,
    WriteBlock = 1,
    EraseBlock = 3
};

///===================================================
/// UNIONs
///===================================================
union LARGE32
{
    struct
    {
        UINT16 low;
        UINT16 high;
    };
    UINT32 value;
};

///===================================================
/// Types
///===================================================
struct DATA_HEADER
{
    UINT32 Size;
};

struct IO_PORT_DATA : DATA_HEADER
{
    IO_PORT_OPERATION IOPortOperation;
    UINT16 IOPortInput;
    LARGE32 IOPortValue;
};

#include <pshpack1.h>
struct MAPPED_MEMORY : DATA_HEADER
{
    LARGE_INTEGER SectionOffset;
    UINT32 CommitSize;
    UINT64 Result;
    UINT32 Padding;
};
#include <poppack.h>

struct UNMAP_MEMORY : DATA_HEADER
{
    UINT32 Padding[2];
    PVOID Address;
};

struct SECTION_INFORMATION
{
    UINT32 BaseImage;
    UINT32 SectionSize;
};

template <typename T>
struct INPUT_BUFFER
{
    T* Data;
    UINT32 Size;
    UINT32 Padding;
};

union BIOS_FADDR
{
    struct
    {
        UINT32 FLA : 27;    // Flash Linear Address - bits 0 - 26
        UINT32 Reserved : 5;// bits 27 - 31
    };
    UINT32 value;
};

union BIOS_HSFSTS_CTL
{
    struct
    {
        UINT32 FDONE : 1;       // Flash Cycle Done - bit 0
        UINT32 FCERR : 1;       // Flash Cycle Error - bit 1
        UINT32 H_AEL : 1;       // Access Error Log - bit 2
        UINT32 Reserved : 2;    // bits 3 - 4
        UINT32 H_SCIP : 1;      // SPI Cycle In Progress - bit 5
        UINT32 Reserved2 : 5;   // bits 6 - 10
        UINT32 WRSDIS : 1;      // Write Status Disable - bit 11
        UINT32 PRR34_LOCKDN : 1;// PRR3 PRR4 Lock-Down - bit 12
        UINT32 FDOPSS : 1;      // Flash Descriptor Override Pin-Strap Status - bit 13
        UINT32 FDV : 1;         // Flash Descriptor Valid - bit 14
        UINT32 FLOCKDN : 1;     // Flash Configuration Lock-Down - bit 15
        UINT32 FGO : 1;         // Flash Cycle Go - bit 16
        UINT32 FCYCLE : 4;      // Flash Cycle - bits 17 - 20
        UINT32 WET : 1;         // Write Enable Type - bit 21
        UINT32 Reserved3 : 2;   // bits 22 - 23
        UINT32 FDBC : 6;        // Flash Data Byte Count - bits 24 - 29
        UINT32 Reserved4 : 1;   // bit 30
        UINT32 FSMIE : 1;       // Flash SPI SMI# Enable bit 31
    };
    UINT32 value;
};

union BIOS_BFPREG
{
    struct
    {
        UINT32 PrimaryRegionBase : 15;  // bits 0 - 14
        UINT32 Reserved : 1;            // bit 15
        UINT32 PrimaryRegionLimit : 15; // bits 16 - 30
        UINT32 Reserved2 : 1;           // bit 31
    };
    UINT32 value;
};

struct SPI_REGISTER_MAP
{
    BIOS_BFPREG BFPREG;
    BIOS_HSFSTS_CTL HSFSTS;
    BIOS_FADDR FADDR;
    UINT32 DiscreteLock;
    UINT32 FlashData[16];
    UINT32 FlashRegionAccessPermissions;
    UINT32 FlashRegion[12];
    UINT32 FlashProtectedRange[5];
    UINT32 GlobalProtectedRange;
    UINT8 Reserved2[22];
    UINT32 SecondaryFlashRegionAccessPermissions;
    UINT32 FlashDescriptorObservabilityControl;
    UINT32 FlashDescriptorObservabilityData;
    UINT8 Reserved3[4];
    UINT32 AdditionalFlashControl;
    UINT32 VSCCC0;
    UINT32 VSCCC1;
    UINT32 ParameterTableIndex;
    UINT32 ParameterTableData;
    UINT32 SPIBusRequesterStatus;
};

typedef IO_PORT_DATA* PIO_PORT_DATA;
typedef MAPPED_MEMORY* PMAPPED_MEMORY;
typedef SPI_REGISTER_MAP* PSPI_REGISTER_MAP;
typedef SECTION_INFORMATION* PSECTION_INFORMATION;


/// EOF