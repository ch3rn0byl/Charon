#include "ServiceController.h"

void
ServiceController::ServiceCleanUp()
{
	SERVICE_STATUS_PROCESS ssp = { 0 };

	DWORD dwBytesNeeded = NULL;
	DWORD dwWaitTime = NULL;

	bool bStatus = false;

	//
	// We should have the handle for the service controller, but 
	// if the service handle is not populated, open a handle to the
	// service with delete, query, and stopping rights.
	// 
	if (schSCManager != NULL && schService == NULL)
	{
		schService = OpenService(
			schSCManager,
			wszServiceName,
			DELETE |
			SERVICE_QUERY_STATUS |
			SERVICE_STOP
		);
	}

	if (schSCManager != NULL && schService != NULL)
	{
		//
		// Check to see if the service is up and running. If so, 
		// send the signal to stop it.
		// 
		bStatus = ControlService(
			schService,
			SERVICE_CONTROL_STOP,
			reinterpret_cast<LPSERVICE_STATUS>(&ssp)
		);
		if (bStatus)
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

					bStatus = QueryServiceStatusEx(
						schService,
						SC_STATUS_PROCESS_INFO,
						reinterpret_cast<LPBYTE>(&ssp),
						sizeof(SERVICE_STATUS_PROCESS),
						&dwBytesNeeded
					);
					if (!bStatus)
					{
						// handle it
						std::wcout << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
					}
				}
			}

			//
			// Delete the service now that it's stopped.
			// 
			DeleteService(schService);
		}
		else if (!bStatus && GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
		{
			//
			// If the service is not active, this means the service does exist so 
			// we can delete it.
			// 
			DeleteService(schService);
		}
	}
}

ServiceController::ServiceController()
{
	schSCManager = NULL;
	schService = NULL;

	bStatus = false;

	//
	// Grab a handle to the service controller manager.
	// 
	schSCManager = OpenSCManager(
		NULL, NULL,
		SC_MANAGER_ALL_ACCESS
	);
	if (schSCManager != NULL)
	{
		bStatus = true;
	}
}

ServiceController::~ServiceController()
{
	//
	// Check if the service is running and if it is, stop it then
	// delete it.
	// 
	ServiceCleanUp();

	if (schService != NULL)
	{
		CloseServiceHandle(schService);
		schService = NULL;
	}

	if (schSCManager != NULL)
	{
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
	}
}

bool
ServiceController::StartKernelService()
{
	//
	// Create the service on the machine. OffensiveResearch should
	// not be an installed service so no need to check for clashing
	// service names :)
	// 
	schService = CreateService(
		schSCManager,
		wszServiceName,
		wszServiceName,
		GENERIC_EXECUTE | DELETE,
		SERVICE_KERNEL_DRIVER,
		SERVICE_SYSTEM_START,
		SERVICE_ERROR_NORMAL,
		wsAbsTempPath.c_str(),
		NULL, NULL, NULL, NULL, NULL
	);
	if (schService == NULL)
	{
		std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << ": " << GetLastError() << std::endl;
		return false;
	}

	bStatus = StartService(schService, NULL, NULL);
	if (!bStatus)
	{
		std::wcerr << __FUNCTIONW__ << ":" << __LINE__ << ": " << GetLastError() << std::endl;

		return bStatus;
	}

	return bStatus;
}


/// EOF