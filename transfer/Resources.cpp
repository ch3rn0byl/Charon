#include "Resources.h"

Resources::Resources() : 
	//m_AbsTempPath(m_AbsTempPath.resize(MAX_PATH)),
	m_hResources(nullptr),
	m_hGlobal(nullptr),
	m_lpImageDataBlob(nullptr),
	m_fWasFileCreated(false)
{
	m_AbsTempPath.resize(MAX_PATH);

	DWORD dwNameSize = GetTempPath(MAX_PATH, &m_AbsTempPath[0]);
	if (dwNameSize > MAX_PATH || dwNameSize == NULL)
	{
		throw std::runtime_error("The name size returned is greater than MAX_PATH or is NULL");
	}

	m_AbsTempPath.resize(dwNameSize);
	m_AbsTempPath.insert(dwNameSize, L"OffensiveResearch.sys");

	m_hResources = FindResource(NULL, MAKEINTRESOURCE(IDR_INTELPMX1), RT_RCINTELPMX1);
	if (m_hResources != NULL)
	{
		m_hGlobal = LoadResource(NULL, m_hResources);
		if (m_hGlobal != NULL)
		{
			DWORD dwSizeOfResources = SizeofResource(NULL, m_hResources);
			if (dwSizeOfResources != NULL)
			{
				m_lpImageDataBlob = LockResource(m_hGlobal);
				if (m_lpImageDataBlob == nullptr)
				{
					throw std::runtime_error("The data blob is null");
				}
			}
			else
			{
				throw std::runtime_error("The size of the resources is null");
			}
		}
		else
		{
			throw std::runtime_error("The global handle is null");
		}
	}
	else
	{
		throw std::runtime_error("The resources handle is null");
	}
}

Resources::~Resources()
{
	if (m_lpImageDataBlob != nullptr)
	{
		UnlockResource(m_hGlobal);
	}

	if (m_hGlobal != nullptr)
	{
		FreeResource(m_hGlobal);
	}

	if (m_fWasFileCreated)
	{
		DeleteFile(m_AbsTempPath.c_str());
	}

	if (!m_AbsTempPath.empty())
	{
		//DeleteFile(AbsTempPath.c_str());
		m_AbsTempPath.erase();
	}
}

bool Resources::DumpDataBlobToDisk()
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwNumberOfBytes = 0;

	bool fStatus = false;

	hFile = CreateFile(
		m_AbsTempPath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		fStatus = WriteFile(
			hFile,
			m_lpImageDataBlob,
			SizeofResource(NULL, m_hResources),
			&dwNumberOfBytes,
			NULL
		);
		if (fStatus)
			m_fWasFileCreated = true;
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return fStatus;
}

std::wstring Resources::GetAbsTempPath() const
{
	return m_AbsTempPath;
}
