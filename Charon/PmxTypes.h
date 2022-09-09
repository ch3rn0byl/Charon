#pragma once
#include <minwindef.h>

///=====================================================================
/// Definitions
///=====================================================================
constexpr auto PROCESSOR_BRAND_LEAF = 0x80000002;
constexpr auto CONFIG_ADDRESS = 0xcf8;
constexpr auto CONFIG_DATA = 0xcfc;
constexpr auto MAX_DEVICE_FUNCTIONS = 8;
constexpr auto LPC_FUNCTION = 0x1f;
constexpr auto SPI_MAX_SIZE = 64;

///=====================================================================
/// ENUMs
///=====================================================================
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

///=====================================================================
/// UNIONs
///=====================================================================
union LARGE32
{
    struct
    {
        UINT16 low;
        UINT16 high;
    };
    UINT32 value;
};

///=====================================================================
/// Types
///=====================================================================
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

///=====================================================================
/// These structures were broken down based on X299 Volume 2 of 2 specs.
///=====================================================================
union FLASH_DESCREG
{
    struct 
    {
        UINT32 RB : 13;     // Region Base: This specifies address bits for the region n base.
        UINT32 Reserved : 3;// Reserved.
        UINT32 RL : 13;     // Region Limit: This specifies address bits for the region n limit.
    };
    UINT32 value;
};

union BIOS_FADDR
{
    struct
    {
        UINT32 FLA : 27;    // Flash Linear Address: The FLA is the starting byte linear address of a SPI Read or Write cycle or an address within a Block for the Block Erase command.
        UINT32 Reserved : 5;// Reserved.
    };
    UINT32 value;
};

union BIOS_HSFSTS_CTL
{
    struct
    {
        UINT32 FDONE : 1;       // Flash Cycle Done: The PCH sets this bit to 1 when the SPI Cycle completes after software previously set the FGO bit.
        UINT32 FCERR : 1;       // Flash Cycle Error: Hardware sets this bit to 1 when a program register access is blocked to the FLASH due to one of the protection policies or when any of the programmed cycle registers is written while a programmed access is already in progress.
        UINT32 H_AEL : 1;       // Access Error Log: Hardware sets this bit to a 1 when an attempt was made to access the BIOS region using the direct access method or an access to the BIOS Program Registers that violated the security restrictions.
        UINT32 Reserved : 2;    // Reserved.
        UINT32 H_SCIP : 1;      // SPI Cycle In Progress: Hardware sets this bit when software sets the Flash Cycle Go (FGO) bit in the Hardware Sequencing Flash Control register.
        UINT32 Reserved2 : 5;   // Reserved.
        UINT32 WRSDIS : 1;      // Flash Configuration Lock-Down: Write status operation disable. 
        UINT32 PRR34_LOCKDN : 1;// PRR3 PRR4 Lock-Down: When set to 1, the BIOS PRR3 and PRR4 registers cannot be written.
        UINT32 FDOPSS : 1;      // Flash Descriptor Override Pin-Strap Status: This register reflects the value the Flash Descriptor Override Pin-Strap.
        UINT32 FDV : 1;         // Flash Descriptor Valid: This bit is set to a 1 if the Flash Controller read the correct Flash Descriptor Signature.
        UINT32 FLOCKDN : 1;     // Flash Configuration Lock-Down: When set to 1, those Flash Program Registers that are locked down by this FLOCKDN bit cannot be written.
        UINT32 FGO : 1;         // Flash Cycle Go: A write to this register with a 1 in this bit initiates a request to the Flash SPI Arbiter to start a cycle.
        UINT32 FCYCLE : 4;      // Flash Cycle: This field defines the Flash SPI cycle type generated to the FLASH when the FGO bit is set as defined. 
        UINT32 WET : 1;         // Write Enable Type: Uses 0 (06h) or 1 (50h) as the write enable instruction. 
        UINT32 Reserved3 : 2;   // Reserved.
        UINT32 FDBC : 6;        // Flash Data Byte Count: This field specifies the number of bytes to shift in or out during the data portion of the SPI cycle. The contents of this register are 0s based with 0b representing 1 byte and 3Fh representing 64 bytes.
        UINT32 Reserved4 : 1;   // Reserved.
        UINT32 FSMIE : 1;       // Flash SPI SMI# Enable: When set to 1, the SPI asserts an SMI# request whenever the Flash Cycle Done bit is 1.
    };
    UINT32 value;
};

union BIOS_BFPREG
{
    struct
    {
        UINT32 PRB : 15;        // BIOS Flash Primary Region Base.
        UINT32 Reserved : 1;    // Reserved.
        UINT32 PRL : 15;        // BIOS Flash Primary Region Limit.
        UINT32 Reserved2 : 1;   // Reserved.
    };
    UINT32 value;
};

struct SPI_REGISTER_MAP
{
    BIOS_BFPREG BFPREG;     // SPI BIOS MMIO PRI.
    BIOS_HSFSTS_CTL HSFSTS; // Hardware Sequencing Flash Status and Control.
    BIOS_FADDR FADDR;       // Flash Address.
    UINT32 DLOCK;           // Discrete Lock Bits.
    UINT32 FDATA[16];       // Flash Data.
    UINT32 FRACC;           // Flash Region Access Permissions.
    FLASH_DESCREG FREG[12]; // Flash Region.
    UINT32 FPR[5];          // Flash Protected Range.
    UINT32 GPR;             // Global Protected Range.
    UINT8 Reserved2[22];    // Reserved.
    UINT32 SFRACC;          // Secondary Flash Region Access Permissions.
    UINT32 FDOC;            // Flash Descriptor Obersvability Control.
    UINT32 FDOD;            // Flash Descriptor Obersvability Data.
    UINT8 Reserved3[4];     // Reserved.
    UINT32 AFC;             // Additional Flash Control.
    UINT32 VSCCC0;          // Vendor Specific Component Capabilities for Component 0.
    UINT32 VSCCC1;          // Vendor Specific Component Capabilities for Component 1.
    UINT32 PTINX;           // Parameter Table Index.
    UINT32 PTDATA;          // Parameter Table Data.
    UINT32 SBRS;            // SPI Bus Requester Status.
};

typedef IO_PORT_DATA* PIO_PORT_DATA;
typedef MAPPED_MEMORY* PMAPPED_MEMORY;
typedef SPI_REGISTER_MAP* PSPI_REGISTER_MAP;
typedef SECTION_INFORMATION* PSECTION_INFORMATION;


/// EOF