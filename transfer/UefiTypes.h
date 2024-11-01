#pragma once
#include <Windows.h>

namespace DataTypes
{
	namespace UefiTypes
	{
		constexpr auto SPI_MAX_SIZE = 64;

		union BIOS_SPI_BAR0
		{
			struct
			{
				ULONG MemorySpace : 1;
				ULONG Type : 2;
				ULONG Prefetchable : 1;
				ULONG MemorySize : 8;
				ULONG MemoryBar : 20;
			};

			ULONG AsULong;
		};

		union BIOS_BFPREG
		{
			struct
			{
				ULONG BiosFlashPrimaryRegionBase : 15;
				ULONG Reserved : 1;
				ULONG BiosFlashPrimaryRegionLimit : 15;
				ULONG Reserved2 : 1;
			};

			ULONG AsULong;
		};

		union BIOS_HSFSTS_CTL
		{
			struct
			{
				ULONG FlashCycleDone : 1;
				ULONG FlashCycleError : 1;
				ULONG AccessErrorLog : 1;
				ULONG Reserved : 2;
				ULONG SpiCycleInProgress : 1;
				ULONG Reserved2 : 5;
				ULONG WriteStatusDisable : 1;
				ULONG PRR34_LOCKDN : 1;
				ULONG FDOPSS : 1;
				ULONG FDV : 1;
				ULONG FLOCKDN : 1;
				ULONG FGO : 1;
				ULONG FCYCLE : 4;
				ULONG WET : 1;
				ULONG Reserved3 : 2;
				ULONG FDBC : 6;
				ULONG Reserved4 : 1;
				ULONG FSMIE : 1;
			};

			ULONG AsULong;
		};

		enum class HSFSTS_CYCLE : CHAR
		{
			Read = 0,
			Reserved = 1,
			Write = 2,
			BlockErase4k = 3,
			SectorErase64k = 4,
			ReadSFDP = 5,
			ReadJedecID = 6,
			WriteStatus = 7,
			ReadStatus = 8,
			RpmcOp1 = 9,
			RpmcOp2 = 10
		};

		union BIOS_FADDR
		{
			struct
			{
				ULONG FlashLinearAddress : 27;
				ULONG Reserved : 5;
			};

			ULONG AsULong;
		};

		union BIOS_DLOCK
		{
			struct
			{
				ULONG BMWAGLOCKDN : 1;
				ULONG BMRAGLOCKDN : 1;
				ULONG SBMWAGLOCKDN : 1;
				ULONG SBMRAGLOCKDN : 1;
				ULONG Spare7 : 1;
				ULONG Spare6 : 1;
				ULONG Spare5 : 1;
				ULONG Spare4 : 1;
				ULONG PR0LOCKDN : 1;
				ULONG PR1LOCKDN : 1;
				ULONG PR2LOCKDN : 1;
				ULONG PR3LOCKDN : 1;
				ULONG PR4LOCKDN : 1;
				ULONG Spare3 : 1;
				ULONG Spare2 : 1;
				ULONG Spare1 : 1;
				ULONG SSEQLOCKDN : 1;
				ULONG Reserved : 15;
			};

			ULONG AsULong;
		};

		using BIOS_FDATA = ULONG;

		union BIOS_FRACC
		{
			struct
			{
				ULONG BRRA : 8;
				ULONG BRWA : 8;
				ULONG BMRAG : 8;
				ULONG BMWAG : 8;
			};

			ULONG AsULong;
		};

		union BIOS_FREG
		{
			struct
			{
				ULONG RegionBase : 15;
				ULONG Reserved : 1;
				ULONG RegionLimit : 15;
				ULONG Reserved2 : 1;
			};

			ULONG AsULong;
		};

		union BIOS_FPR
		{
			struct
			{
				ULONG ProtectedRangeBase : 15;
				ULONG ReadProtectionEnable : 1;
				ULONG ProtectedRangeLimit : 15;
				ULONG WriteProtectionEnable : 1;
			};

			ULONG AsULong;
		};

		union BIOS_GPR
		{
			struct
			{
				ULONG ProtectedRangeBase : 15;
				ULONG ReadProtectionEnable : 1;
				ULONG ProtectedRangeLimit : 15;
				ULONG WriteProtectionEnable : 1;
			};

			ULONG AsULong;
		};

		union BIOS_SFRACC
		{
			struct
			{
				ULONG Reserved : 16;
				ULONG SECONDARYBIOS_MRAG : 8;
				ULONG SECONDARYBIOS_MWAG : 8;
			};

			ULONG AsULong;
		};

		union BIOS_FDOC
		{
			struct
			{
				ULONG Reserved : 2;
				ULONG FDSI : 10;
				ULONG FDSS : 3;
				ULONG Reserved2 : 17;
			};

			ULONG AsULong;
		};

		union BIOS_FDOD
		{
			ULONG FDSD;
		};

		union BIOS_AFC
		{
			struct
			{
				ULONG SPFP : 1;
				ULONG Reserved : 31;
			};

			ULONG AsULong;
		};

		union BIOS_SFDP0_VSCC0
		{
			struct
			{
				ULONG Reserved : 2;
				ULONG WG : 1;
				ULONG WSR : 1;
				ULONG WEWS : 1;
				ULONG QER : 3;
				ULONG EO_4K : 8;
				ULONG EO_64K : 8;
				ULONG SOFT_RST_SUPPORTED : 1;
				ULONG SUSPEND_RESUME_SUPPORTED : 1;
				ULONG DEEP_PWRDN_SUPPORTED : 1;
				ULONG RPMC_SUPPORTED : 1;
				ULONG EO_4K_VALID : 1;
				ULONG EO_64K_VALID : 1;
				ULONG VCL : 1;
				ULONG CPPTV : 1;
			};

			ULONG AsULong;
		};

		union BIOS_SFDP1_VSCC1
		{
			struct
			{
				ULONG Reserved : 2;
				ULONG WG : 1;
				ULONG WSR : 1;
				ULONG WEWS : 1;
				ULONG QER : 3;
				ULONG EO_4K : 8;
				ULONG EO_64K : 8;
				ULONG SOFT_RST_SUPPORTED : 1;
				ULONG SUSPEND_RESUME_SUPPORTED : 1;
				ULONG DEEP_PWRDN_SUPPORTED : 1;
				ULONG RPMC_SUPPORTED : 1;
				ULONG EO_4K_VALID : 1;
				ULONG EO_64K_VALID : 1;
				ULONG Reserved2 : 1;
				ULONG CPPTV : 1;
			};

			ULONG AsULong;
		};

		union BIOS_PTINX
		{
			struct
			{
				ULONG Reserved : 2;
				ULONG PTDWI : 10;
				ULONG HORD : 2;
				ULONG SPT : 2;
				ULONG Reserved2 : 16;
			};

			ULONG AsULong;
		};

		union BIOS_PTDATA
		{
			ULONG PTDWD;
		};

		union BIOS_SBRS
		{
			struct
			{
				ULONG M1Status : 3;
				ULONG M2Status : 3;
				ULONG M3Status : 3;
				ULONG M4Status : 3;
				ULONG M6Status : 3;
				ULONG M5Status : 3;
				ULONG Reserved : 12;
				ULONG ESPI_ACC_ONG : 1;
				ULONG TPM_ACC_ONG : 1;
			};

			ULONG AsULong;
		};

		struct SPI_MEMORY_MAPPED
		{
			BIOS_BFPREG BiosFlashPrimaryRegion;
			BIOS_HSFSTS_CTL HardwareSequencingFlashStatusAndControl;
			BIOS_FADDR FlashAddress;
			BIOS_DLOCK DiscreteLockBits;
			BIOS_FDATA FlashData[16];
			BIOS_FRACC FlashRegionAccessPermissions;
			BIOS_FREG FlashRegion[6];
			BIOS_FREG Padding[6];
			BIOS_FPR FlashProtectedRange[5];
			BIOS_GPR GlobalProtectedRange;
			BIOS_GPR Padding2[5];
			BIOS_SFRACC SecondaryFlashRegionAccessPermissions;
			BIOS_FDOC FlashDescriptorObservabilityControl;
			BIOS_FDOD FlashDescriptorObservabilityData;
			BIOS_AFC AdditionalFlashControl;
			BIOS_SFDP0_VSCC0 VendorSpecificComponentCapabilities0;
			BIOS_SFDP1_VSCC1 VendorSpecificComponentCapabilities1;
			BIOS_PTINX ParameterTableIndex;
			BIOS_SBRS SpiBusRequesterStatus;
		};

		using PSPI_MEMORY_MAPPED = SPI_MEMORY_MAPPED*;

	}
}