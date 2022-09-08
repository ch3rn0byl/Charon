// Charon.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS // ctime because I'm too 'tarded to use ctime_s.

#include <Windows.h>

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "privileges.h"
#include "ServiceController.h"
#include "IntelPmxClient.h"

void
PrintUsage(
    _In_ const wchar_t* pwszProgramName
)
{
    std::wcout << "Usage: " << pwszProgramName << " options parameters\n" << std::endl;

    std::wcout << pwszProgramName << " can be used for dumping all modules that reside in SPI flash memory ";
    std::wcout << "of a live system for vulnerability research or analysis. The use case for this tool would ";
    std::wcout << "be to discover potential vulnerabilities or compromise. " << pwszProgramName << " was ";
    std::wcout << "created with the intention of aiding both Offensive Research and Defensive Research ";
    std::wcout << "purposes." << std::endl;

    std::wcout << "\nOptions:" << std::endl;
    std::wcout << "  -h, --help\t\tShows this help message and exits" << std::endl;
    std::wcout << "  -d, --dump-all\tDumps the SPI flash memory in its entirety" << std::endl;
    std::wcout << "  -u, --uefi-only\tDumps the UEFI module only" << std::endl;
    std::wcout << "  -o, --output\t\tOutputs to a file specified. Default is image.bin" << std::endl;

    std::wcout << "\nThis is a tool to dump firmware via software; consequently, if a bootkit is suspected, ";
    std::wcout << "it is not safe to assume the integrity of this data being clean as it is possible for ";
    std::wcout << "modified firmware to taint data being transferred to the application." << std::endl;
}

int wmain(
    _In_ int argc,
    _In_ wchar_t* wargv[]
)
{
    std::unique_ptr<ServiceController> pServiceController = nullptr;
    std::unique_ptr<IntelPmxClient> pIntelPmxClient = nullptr;
    PSPI_REGISTER_MAP pSPIRegisterMapping = nullptr;

    std::wstring Outfile = L"image.bin";
    std::string ProcessorInformation{};
    std::vector<DWORD> pdata{};
    std::vector<PSECTION_INFORMATION> please{};

    UINT32 SPIPhysicalMemory = NULL;
    UINT32 SizeOfImage = NULL;

    HANDLE hFile = INVALID_HANDLE_VALUE;

    bool bDumpAll = false;
    bool bUefiOnly = false;

    std::wcout << std::endl;
    std::wcout << "            ('-. .-.   ('-.     _  .-')                    .-') _  " << std::endl;
    std::wcout << "           ( OO )  /  ( OO ).-.( \\( -O )                  ( OO ) ) " << std::endl;
    std::wcout << "   .-----. ,--. ,--.  / . --. / ,------.  .-'),-----. ,--./ ,--,'  " << std::endl;
    std::wcout << "  '  .--./ |  | |  |  | \\-.  \\  |   /`. '( OO'  .-.  '|   \\ |  |\\  " << std::endl;
    std::wcout << "  |  |('-. |   .|  |.-'-'  |  | |  /  | |/   |  | |  ||    \\|  | ) " << std::endl;
    std::wcout << " /_) |OO  )|       | \\| |_.'  | |  |_.' |\\_) |  |\\|  ||  .     |/  " << std::endl;
    std::wcout << " ||  |`-'| |  .-.  |  |  .-.  | |  .  '.'  \\ |  | |  ||  |\\    |   " << std::endl;
    std::wcout << "(_'  '--'\\ |  | |  |  |  | |  | |  |\\  \\    `'  '-'  '|  | \\   |   " << std::endl;
    std::wcout << "   `-----' `--' `--'  `--' `--' `--' '--'     `-----' `--'  `--'   " << std::endl;
    std::wcout << "                                      A UEFI extraction tool" << std::endl;
    std::wcout << std::endl;

    if (!privileges::RunningAsAdmin())
    {
        std::wcerr << "[!] This tool requires administrator privileges." << std::endl;
        return EXIT_FAILURE;
    }

    if (argc <= 1)
    {
        PrintUsage(wargv[0]);
        return EXIT_FAILURE;
    }
    else
    {
        for (int i = 0; i < argc; i++)
        {
            if (wcscmp(wargv[i], L"-h") == 0 || wcscmp(wargv[i], L"--help") == 0)
            {
                PrintUsage(wargv[0]);
                return EXIT_SUCCESS;
            }
            else if (wcscmp(wargv[i], L"-o") == 0 || wcscmp(wargv[i], L"--output") == 0)
            {
                Outfile = wargv[i + 1];
            }
            else if (wcscmp(wargv[i], L"-d") == 0 || wcscmp(wargv[i], L"--dump-all") == 0)
            {
                bDumpAll = true;
            }
            else if (wcscmp(wargv[i], L"-u") == 0 || wcscmp(wargv[i], L"--uefi-only") == 0)
            {
                bUefiOnly = true;
            }
        }

        if (!bDumpAll && !bUefiOnly)
        {
            std::wcerr << "[!] A required parameter is missing: -d/--dump-all or -u/--uefi-only." << std::endl;
            return EXIT_FAILURE;
        }
    }

    //
    // Get the time as of now and then get it again later to see how long the application
    // took to run.
    // 
    auto start = std::chrono::system_clock::now();
    std::time_t StartTime = std::chrono::system_clock::to_time_t(start);

    //
    // std::endl is not included because std::ctime() inserts a newline.
    // 
    std::wcout << "[+] " << wargv[0] << " starting " << std::ctime(&StartTime);

    pServiceController = std::make_unique<ServiceController>();
    pIntelPmxClient = std::make_unique<IntelPmxClient>();

    //
    // Dump the driver to disk.
    // 
    if (!pServiceController->DumpDataBlobToDisk())
    {
        std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
        return EXIT_FAILURE;
    }

    //
    // Create the service to interact with the driver.
    // 
    if (!pServiceController->StartKernelService())
    {
        std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
        return EXIT_FAILURE;
    }

    //
    // Initialize Intel's driver for use. 
    // 
    if (!pIntelPmxClient->init(L"\\\\.\\Pmxdrv"))
    {
        std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
        return EXIT_FAILURE;
    }

    //
    // Get the user's chip information.
    // 
    ProcessorInformation = pIntelPmxClient->GetProcessorInformation();
    std::wcout << "[+] Processor Information: " << ProcessorInformation.c_str() << std::endl;

    //
    // The SPI controller's physical address can be found by querying BAR0 of the 
    // PCI. This entry is at offset 0x10. 
    // 
    if (!pIntelPmxClient->ReadIOPort(pIntelPmxClient->GetPCIValue(0, 31, 5, 0x10), &SPIPhysicalMemory))
    {
        std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
        return EXIT_FAILURE;
    }

    std::wcout << std::hex << "[+] SPI located at " << SPIPhysicalMemory << "." << std::endl;

    //
    // Map the memory into userspace to be able to read and write to it. 
    // 
    if (!pIntelPmxClient->MapPhysicalMemory(
        SPIPhysicalMemory,
        reinterpret_cast<PVOID*>(&pSPIRegisterMapping))
        )
    {
        std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
        return EXIT_FAILURE;
    }

    //
    // Dump the entire SPI flash region.
    // 
    if (bDumpAll)
    {
        //
        // Get the size of the entire SPI flash region.
        // 
        SizeOfImage = pSPIRegisterMapping->BFPREG.PrimaryRegionLimit;

        //
        // Align the page and then add 1 to round up the size.
        // 
        SizeOfImage <<= 12;
        SizeOfImage |= 0xfff;
        SizeOfImage++;

        for (int i = 0; i < sizeof(pSPIRegisterMapping->FlashRegion) / 4; i++)
        {
            if (pSPIRegisterMapping->FlashRegion[i] != NULL &&
                pSPIRegisterMapping->FlashRegion[i] != 0x7fff)
            {
                PSECTION_INFORMATION psi = new SECTION_INFORMATION();

                BIOS_BFPREG bfpr = { 0 };

                bfpr.value = pSPIRegisterMapping->FlashRegion[i];

                // 
                // Get the base of the section.
                // 
                psi->BaseImage = bfpr.PrimaryRegionBase;

                //
                // Get the size of the image with the limit. 
                // 
                psi->SectionSize = bfpr.PrimaryRegionLimit + 1;
                psi->SectionSize -= psi->BaseImage;

                psi->BaseImage <<= 12;
                psi->SectionSize <<= 12;

                please.push_back(psi);
            }
        }

        //
        // Reverse the order that way we follow the structure of the firmware image. 
        // 
        std::reverse(please.begin(), please.end());

        //
        // Set the base address to the base of the image at offset 0.  
        // 
        pSPIRegisterMapping->FADDR.FLA = NULL;

        //
        // Read up until the address of the first module.
        //
        do
        {
            std::wcout << "\r[+] Reading " << std::hex << pSPIRegisterMapping->FADDR.value;
            std::wcout << " of " << SizeOfImage << "." << std::flush;

            pSPIRegisterMapping->HSFSTS.FDBC = 64 - 1;
            pSPIRegisterMapping->HSFSTS.FCYCLE = ReadBlock;
            pSPIRegisterMapping->HSFSTS.FGO = 1;

            while (pSPIRegisterMapping->HSFSTS.FDONE != 1)
            {
                //
                // Wait until the controller is finished with its cycle.
                // Ghetto polling, ftw!
                // 
            }

            if (pSPIRegisterMapping->HSFSTS.FCERR == 0 &&
                pSPIRegisterMapping->HSFSTS.H_AEL == 0 &&
                pSPIRegisterMapping->HSFSTS.H_SCIP == 0)
            {
                for (int i = 0; i < 16; i++)
                {
                    pdata.push_back(pSPIRegisterMapping->FlashData[i]);
                }
            }

            pSPIRegisterMapping->FADDR.FLA += 64;
        } while (pSPIRegisterMapping->FADDR.FLA != please[0]->BaseImage);

        //
        // Now loop through the addresses.
        // 
        for (auto i : please)
        {
            pSPIRegisterMapping->FADDR.FLA = i->BaseImage;

            do
            {
                std::wcout << "\r[+] Reading " << std::hex << pSPIRegisterMapping->FADDR.value;
                std::wcout << " of " << SizeOfImage << "." << std::flush;

                pSPIRegisterMapping->HSFSTS.FDBC = 64 - 1;
                pSPIRegisterMapping->HSFSTS.FCYCLE = ReadBlock;
                pSPIRegisterMapping->HSFSTS.FGO = 1;

                while (pSPIRegisterMapping->HSFSTS.FDONE != 1)
                {
                    //
                    // Again, wait until the controller is finished with its cycle.
                    // Ghetto polling, ftw!
                    // 
                }

                if (pSPIRegisterMapping->HSFSTS.FCERR == 0 &&
                    pSPIRegisterMapping->HSFSTS.H_AEL == 0 &&
                    pSPIRegisterMapping->HSFSTS.H_SCIP == 0)
                {
                    for (int i = 0; i < 16; i++)
                    {
                        pdata.push_back(pSPIRegisterMapping->FlashData[i]);
                    }

                    pSPIRegisterMapping->FADDR.FLA += 64;
                }
                else
                {
                    //
                    // We will end up here on certain parts of the region where we do not
                    // have the permissions to read so pad it. 
                    // 
                    for (UINT32 padding = 0; padding < i->SectionSize / 4; padding++)
                    {
                        pdata.push_back(0xffffffff);
                    }
                    break;
                }
            } while (pSPIRegisterMapping->FADDR.FLA < i->BaseImage + i->SectionSize);
        }
    }

    //
    // Dump only the UEFI image. 
    // 
    if (bUefiOnly)
    {
        //
        // Read SPI flash memory starting from the primary region base. 
        // 
        UINT32 PrimaryRegionBase = pSPIRegisterMapping->BFPREG.PrimaryRegionBase;

        // 
        // Get the size of the image with the limit and the primary region base.
        //
        SizeOfImage = pSPIRegisterMapping->BFPREG.PrimaryRegionLimit + 1;
        SizeOfImage -= PrimaryRegionBase;

        //
        // Align the base address and the size by page.
        // 
        PrimaryRegionBase <<= 12;
        SizeOfImage <<= 12;

        //
        // Start reading at the base address of the UEFI image. 
        // 
        pSPIRegisterMapping->FADDR.FLA = PrimaryRegionBase;

        do
        {
            std::wcout << "\r[+] Reading " << std::hex << pSPIRegisterMapping->FADDR.value;
            std::wcout << " of " << PrimaryRegionBase + SizeOfImage << "." << std::flush;

            pSPIRegisterMapping->HSFSTS.FDBC = 64 - 1;
            pSPIRegisterMapping->HSFSTS.FCYCLE = ReadBlock;
            pSPIRegisterMapping->HSFSTS.FGO = 1;

            while (pSPIRegisterMapping->HSFSTS.FDONE != 1)
            {
                //
                // Wait until the controller is finished with its cycle.
                // Ghetto polling, ftw!
                // 
            }

            //
            // We do not need to push padding here because we have the permissions to read
            // the range of the UEFI image.
            // 
            if (pSPIRegisterMapping->HSFSTS.FCERR == 0 &&
                pSPIRegisterMapping->HSFSTS.H_AEL == 0 &&
                pSPIRegisterMapping->HSFSTS.H_SCIP == 0)
            {
                for (int i = 0; i < 16; i++)
                {
                    pdata.push_back(pSPIRegisterMapping->FlashData[i]);
                }
            }

            pSPIRegisterMapping->FADDR.FLA += 64;
        } while (pSPIRegisterMapping->FADDR.FLA < PrimaryRegionBase + SizeOfImage);
    }

    std::wcout << "\n[+] Dumping " << pdata.size() * sizeof(DWORD) << " bytes to " << Outfile << "...";
    hFile = CreateFile(
        Outfile.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (!WriteFile(hFile, pdata.data(), pdata.size() * 4, NULL, NULL))
        {
            std::wcerr << "uh-oh!" << std::endl;
            std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
        }
        else
        {
            std::wcout << "done." << std::endl;
        }
    }
    else
    {
        std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    for (auto i : please)
    {
        RtlSecureZeroMemory(i, sizeof(SECTION_INFORMATION));
        delete i;
        i = nullptr;
    }

    if (pSPIRegisterMapping != nullptr)
    {
        if (!pIntelPmxClient->UnmapMappedMemory(pSPIRegisterMapping))
        {
            std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
            return EXIT_FAILURE;
        }

        pSPIRegisterMapping = nullptr;
    }

    //
    // Calculate the time it took for all this to run and for reporting purposes.
    // 
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::time_t EndTime = std::chrono::system_clock::to_time_t(end);

    std::wcout << "[+] " << wargv[0] << " ending " << std::ctime(&EndTime);
    std::wcout << "[+] This process took " << duration.count() << " seconds." << std::endl;
    std::wcout << "[+] Done." << std::endl;
    return EXIT_SUCCESS;
}


/// EOF