#pragma once

#include <Windows.h>
#include <string>
#include <iostream>

#include "resource.h"

constexpr auto wszDriverName = L"OffensiveResearch.sys";

class Resources
{
private:
	bool bRetVal;
	bool bWasFileCreated;

	DWORD dwSizeOfResources;
	LPVOID lpDataBlob;

	HRSRC hResources;
	HGLOBAL hGlobal;

protected:
	std::wstring wsAbsTempPath;

public:
	Resources();
	~Resources();

	/// <summary>
	/// This method will dump the embedded driver to disk in 
	/// the %TEMP% directory.
	/// </summary>
	/// <returns>true if successful</returns>
	bool DumpDataBlobToDisk();

};


/// EOF