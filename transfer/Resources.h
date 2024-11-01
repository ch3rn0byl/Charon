#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include "resource.h"

class Resources
{
public:
	Resources();
	~Resources();

	bool DumpDataBlobToDisk();
	std::wstring GetAbsTempPath() const;

protected:
	std::wstring m_AbsTempPath;

private:
	HRSRC m_hResources;
	HGLOBAL m_hGlobal;
	LPVOID m_lpImageDataBlob;
	bool m_fWasFileCreated;
};

