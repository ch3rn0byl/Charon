#include "BaseDriverClient.h"

BaseDriverClient::BaseDriverClient()
{
	hFile = INVALID_HANDLE_VALUE;
}

BaseDriverClient::~BaseDriverClient()
{
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
}

_Use_decl_annotations_
bool BaseDriverClient::init(
	PCWSTR pwszFileName
)
{
	hFile = CreateFile(
		pwszFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// handle it
		return false;
	}
	return true;
}

_Use_decl_annotations_
bool BaseDriverClient::SendIoRequest(
	DWORD dwIoControlCode,
	LPVOID lpInBuffer,
	DWORD dwInBufferSize,
	LPVOID lpOutBuffer,
	DWORD dwOutBufferSize,
	LPDWORD lpBytesReturned
)
{
	bool bStatus = false;

	bStatus = DeviceIoControl(
		hFile,
		dwIoControlCode,
		lpInBuffer,
		dwInBufferSize,
		lpOutBuffer,
		dwOutBufferSize,
		lpBytesReturned,
		NULL
	);
	if (!bStatus)
	{
		// handle error
	}
	return bStatus;
}


/// EOF