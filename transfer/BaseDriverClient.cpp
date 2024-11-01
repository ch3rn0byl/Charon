#include "BaseDriverClient.h"

_Use_decl_annotations_
#ifdef UNICODE
BaseDriver::BaseDriver(LPCWSTR lpFileName) :
	m_ErrorHandler(nullptr),
	m_hFileHandle(INVALID_HANDLE_VALUE)
{
	m_hFileHandle = CreateFile(
		lpFileName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (m_hFileHandle == INVALID_HANDLE_VALUE)
	{
		m_ErrorHandler = std::make_unique<ErrorHandler>(GetLastError());
		throw std::runtime_error(m_ErrorHandler->GetLastErrorAsStringA());
	}
}
#else
BaseDriver::BaseDriver(LPCSTR lpFileName) :
	m_ErrorHandler(nullptr),
	m_hFileHandle(INVALID_HANDLE_VALUE)
{
	m_hFileHandle = CreateFile(
		lpFileName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (m_hFileHandle == INVALID_HANDLE_VALUE)
	{
		m_ErrorHandler = std::make_unique<ErrorHandler>(GetLastError());
		throw std::runtime_error(m_ErrorHandler->GetLastErrorAsStringA());
	}
}
#endif // UNICODE

BaseDriver::~BaseDriver() noexcept
{
	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
}

_Use_decl_annotations_
BOOL BaseDriver::SendIoControlRequest(
	DWORD dwIoControlCode,
	LPVOID lpInBuffer,
	DWORD dwInBufferSize,
	LPVOID lpOutBuffer,
	DWORD dwOutBufferSize,
	LPDWORD lpBytesReturned
)
{
	if (lpBytesReturned != nullptr)
	{
		*lpBytesReturned = 0;
	}

	return DeviceIoControl(
		m_hFileHandle,
		dwIoControlCode,
		lpInBuffer,
		dwInBufferSize,
		lpOutBuffer,
		dwOutBufferSize,
		lpBytesReturned,
		NULL);
}


/// EOF