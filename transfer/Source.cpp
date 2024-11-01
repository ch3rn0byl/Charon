// CharonRework.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Charon.h"

#include <iostream>
#include <chrono>

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
	std::wcout << "  -o, --output\t\tOutputs to a file specified. Default is image.bin" << std::endl;

	std::wcout << "\nThis is a tool to dump firmware via software; consequently, if a bootkit is suspected, ";
	std::wcout << "it is not safe to assume the integrity of this data being clean as it is possible for ";
	std::wcout << "modified firmware to taint data being transferred to the application." << std::endl;
}

int wmain(int argc, wchar_t* argv[])
{
	std::unique_ptr<Charon> pCharon = nullptr;

	std::wstring OutputFile = L"image.bin";
	bool fDumpUefiImage = false;

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
	std::wcout << "                                            A UEFI extraction tool" << std::endl;
	std::wcout << std::endl;

	

	if (argc < 2)
	{
		PrintUsage(argv[0]);
		return EXIT_FAILURE;
	}

	for (int i = 1; i < argc; i++)
	{
		if (wcscmp(argv[i], L"-h") == 0 || wcscmp(argv[i], L"--help") == 0)
		{
			PrintUsage(argv[0]);
			return EXIT_SUCCESS;
		}
		else if (wcscmp(argv[i], L"-d") == 0 || wcscmp(argv[i], L"--dump-all") == 0)
		{
			//std::wcout << "[+] Dumping SPI flash memory in its entirety" << std::endl;
			fDumpUefiImage = true;
		}
		else if (wcscmp(argv[i], L"-o") == 0 || wcscmp(argv[i], L"--output") == 0)
		{
			if (i + 1 < argc)
			{
				OutputFile = argv[i + 1];
				i++;
			}
		}
	}

	try
	{
		pCharon = std::make_unique<Charon>();
	}
	catch (const std::exception&e)
	{
		std::wcerr << "[!] " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "[+] Processor information: " << pCharon->GetProcessorInformation() << std::endl;

	if (fDumpUefiImage)
	{
		auto start = std::chrono::system_clock::now();
		std::time_t StartTime = std::chrono::system_clock::to_time_t(start);

		std::wcout << "[+] Beginning the firmware acquisition..." << std::endl;
		if (!pCharon->DumpFullSpiFlashMemory(OutputFile))
		{
			//std::wcerr << "uh-oh!" << std::endl;
			std::wcerr << "[!] Failed to dump SPI flash memory." << std::endl;
			return EXIT_FAILURE;
		}
		else
		{
			//std::wcout << "done!" << std::endl;
			std::wcout << "[+] UEFI firmware has been successfully dumped to " << OutputFile << "." << std::endl;
		}

		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> duration = end - start;
		std::time_t EndTime = std::chrono::system_clock::to_time_t(end);

		std::wcout << "[+] This process took " << duration.count() << " seconds." << std::endl;
	}

	std::wcout << "[+] Finished." << std::endl;
	return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
