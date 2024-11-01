#pragma once
#include <Windows.h>
#include <stdexcept>
#include <memory>

#include "ErrorHandler.h"

class BaseDriver
{
public:
#ifdef UNICODE
	explicit BaseDriver(_In_z_ LPCWSTR lpFileName);
#else
	explicit BaseDriver(_In_z_ LPCSTR lpFileName);
#endif
	~BaseDriver() noexcept;

	BaseDriver() = delete;

	BaseDriver(const BaseDriver&) = delete;
	BaseDriver& operator=(const BaseDriver&) = delete;

	BaseDriver(BaseDriver&&) = delete;
	BaseDriver& operator=(BaseDriver&&) = delete;

protected:
	_Success_impl_(return != FALSE)
		BOOL SendIoControlRequest(
			_In_ DWORD dwIoControlCode,
			_In_reads_bytes_opt_(dwInBufferSize) LPVOID lpInBuffer = nullptr,
			_In_ DWORD dwInBufferSize = 0,
			_Out_writes_bytes_to_opt_(dwOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer = nullptr,
			_In_ DWORD dwOutBufferSize = 0,
			_Out_opt_ LPDWORD lpBytesReturned = nullptr
		);

private:
	std::unique_ptr<ErrorHandler> m_ErrorHandler;
	HANDLE m_hFileHandle;

};


/// EOF