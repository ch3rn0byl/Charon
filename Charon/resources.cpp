#include "resources.h"

Resources::Resources()
{
	bRetVal = false;
	bWasFileCreated = false;

	wsAbsTempPath.resize(MAX_PATH);

	dwSizeOfResources = NULL;
	lpDataBlob = nullptr;

	hResources = NULL;
	hGlobal = NULL;

	DWORD dwNameSize = NULL;

	//
	// Get the absolute path of the user's TEMP directory.
	// 
	dwNameSize = GetTempPath(MAX_PATH, &wsAbsTempPath[0]);
	if (dwNameSize > MAX_PATH || dwNameSize == NULL)
	{
		std::wcout << __FUNCTIONW__ << ": " << __LINE__ << std::endl;
		return;
	}

	//
	// Resize the string to its actual length and then append
	// OffensiveResearch.sys.
	// 
	wsAbsTempPath.resize(dwNameSize);
	wsAbsTempPath.insert(dwNameSize, wszDriverName);

	//
	// Extract the data blob and then lock the resource into 
	// memory.
	// 
	hResources = FindResource(
		NULL,
		MAKEINTRESOURCE(IDR_INTELPMX1),
		RT_RCINTELPMX1
	);
	if (hResources != NULL)
	{
		hGlobal = LoadResource(NULL, hResources);
		if (hGlobal != NULL)
		{
			dwSizeOfResources = SizeofResource(NULL, hResources);
			if (dwSizeOfResources != NULL)
			{
				lpDataBlob = LockResource(hGlobal);
				if (lpDataBlob != nullptr)
				{
					bRetVal = true;
				}
				else
				{
					std::wcout << __FUNCTIONW__ << ": " << __LINE__ << std::endl;
				}
			}
			else
			{
				std::wcout << __FUNCTIONW__ << ": " << __LINE__ << std::endl;
			}
		}
		else
		{
			std::wcout << __FUNCTIONW__ << ": " << __LINE__ << std::endl;
		}
	}
	else
	{
		std::wcout << __FUNCTIONW__ << ": " << __LINE__ << std::endl;
	}
}

Resources::~Resources()
{
	if (bWasFileCreated)
	{
		DeleteFile(wsAbsTempPath.c_str());
	}

	if (!wsAbsTempPath.empty())
	{
		wsAbsTempPath.erase();
	}
}

bool
Resources::DumpDataBlobToDisk()
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	bool bStatus = false;
	DWORD dwNumberOfBytesWritten = NULL;

	//
	// Create the file on disk. 
	// 
	hFile = CreateFile(
		wsAbsTempPath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	//
	// Write the data of the embedded driver to disk.
	// 
	bStatus = WriteFile(
		hFile,
		lpDataBlob,
		dwSizeOfResources,
		&dwNumberOfBytesWritten,
		NULL
	);
	if (bStatus)
	{
		bWasFileCreated = true;
	}

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	return bStatus;
}


/// EOF