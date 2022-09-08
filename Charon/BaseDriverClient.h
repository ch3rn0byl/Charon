#pragma once
#include <Windows.h>

class BaseDriverClient
{
private:
	HANDLE hFile;

public:
	BaseDriverClient();
	~BaseDriverClient();

	/// <summary>
	/// Initializes the driver object. 
	/// </summary>
	/// <param name="pwszFileName"></param>
	/// <returns>True if successful.</returns>
	/// <returns>False if unsuccessful.</returns>
	bool init(
		_In_z_ PCWSTR pwszFileName
	);

	/// <summary>
	/// Wrapper for DeviceIoControl. 
	/// </summary>
	/// <param name="dwIoControlCode"></param>
	/// <param name="lpInBuffer"></param>
	/// <param name="dwInBufferSize"></param>
	/// <param name="lpOutBuffer"></param>
	/// <param name="dwOutBufferSize"></param>
	/// <param name="lpBytesReturned"></param>
	/// <returns>True if successful.</returns>
	/// <returns>False if unsuccessful.</returns>
	bool SendIoRequest(
		_In_ DWORD dwIoControlCode,
		_In_reads_bytes_opt_(dwInBufferSize) LPVOID lpInBuffer = NULL,
		_In_ DWORD dwInBufferSize = NULL,
		_Out_writes_bytes_to_opt_(dwOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer = NULL,
		_In_ DWORD dwOutBufferSize = NULL,
		_Out_opt_ LPDWORD lpBytesReturned = NULL
	);
};


/// EOF