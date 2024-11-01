#include "ServiceController.h"

ServiceController::ServiceController() : 
	m_Resources(std::make_unique<Resources>()),
	m_schSCManager(NULL),
	m_schService(NULL)
{
	if (!m_Resources->DumpDataBlobToDisk() && GetLastError() != ERROR_SHARING_VIOLATION)
	{
		std::printf("Getlasterror: %d\n", GetLastError());

		throw std::runtime_error("Failed to dump data blob to disk");
	}

	m_schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (m_schSCManager == NULL)
	{
		throw std::runtime_error("Failed to open service manager");
	}
}

ServiceController::~ServiceController()
{
	ServiceCleanUp();

	if (m_schService != NULL)
	{
		CloseServiceHandle(m_schService);
		m_schService = NULL;
	}

	if (m_schSCManager != NULL)
	{
		CloseServiceHandle(m_schSCManager);
		m_schSCManager = NULL;
	}
}

bool ServiceController::StartKernelService()
{
	//
	// Create the service on the machine. OffensiveResearch should
	// not be an installed service so no need to check for clashing
	// service names :)
	// 
	m_schService = CreateService(
		m_schSCManager,
		L"OffensiveResearch",
		L"OffensiveResearch",
		GENERIC_EXECUTE | DELETE,
		SERVICE_KERNEL_DRIVER,
		SERVICE_SYSTEM_START,
		SERVICE_ERROR_NORMAL,
		m_Resources->GetAbsTempPath().c_str(),
		NULL, NULL, NULL, NULL, NULL
	);
	if (m_schService == NULL)
	{
		std::printf("Getlasterror: %d\n", GetLastError());
		return false;
	}

	return StartService(m_schService, 0, NULL);
}

void ServiceController::ServiceCleanUp()
{
	SERVICE_STATUS_PROCESS ssp = { 0 };

	DWORD dwBytesNeeded = NULL;
	DWORD dwWaitTime = NULL;

	bool fStatus = false;

	if (m_schSCManager != NULL && m_schService == NULL)
	{
		m_schService = OpenService(
			m_schSCManager,
			L"OffensiveResearch",
			DELETE |
			SERVICE_QUERY_STATUS |
			SERVICE_STOP
		);
	}

	if (m_schSCManager != NULL && m_schService != NULL)
	{
		//
		// Check to see if the service is up and running. If so, 
		// send the signal to stop it.
		// 
		fStatus = ControlService(
			m_schService,
			SERVICE_CONTROL_STOP,
			reinterpret_cast<LPSERVICE_STATUS>(&ssp)
		);
		if (fStatus)
		{
			// 
			// Query the status of the service. If it is stopped, go ahead and 
			// delete; otherwise, wait for the service to stop.
			// 
			if (ssp.dwCurrentState != SERVICE_STOPPED)
			{
				//
				// Keep querying the state of the service until the service has
				// stopped.
				// 
				while (ssp.dwCurrentState != SERVICE_STOPPED)
				{
					Sleep(ssp.dwWaitHint);

					fStatus = QueryServiceStatusEx(
						m_schService,
						SC_STATUS_PROCESS_INFO,
						reinterpret_cast<LPBYTE>(&ssp),
						sizeof(SERVICE_STATUS_PROCESS),
						&dwBytesNeeded
					);
					if (!fStatus)
					{
						// Something
					}
				}
			}

			//
			// Delete the service now that it's stopped.
			// 
			DeleteService(m_schService);
		}
		else if (!fStatus && GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
		{
			//
			// If the service is not active, this means the service does exist so 
			// we can delete it.
			// 
			DeleteService(m_schService);
		}
	}
	/*
	if (QueryServiceStatusEx(
		m_schService,
		SC_STATUS_PROCESS_INFO,
		reinterpret_cast<LPBYTE>(&ssp),
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded
	))
	{
		if (ssp.dwCurrentState != SERVICE_STOPPED)
		{
			ControlService(m_schService, SERVICE_CONTROL_STOP, reinterpret_cast<LPSERVICE_STATUS>(&ssp));
		}
	}
	*/

}