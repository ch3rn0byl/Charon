#pragma once
#include "resources.h"

constexpr auto wszServiceName = L"OffensiveResearch";

class ServiceController :
    public Resources
{
private:
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool bStatus;

    /// <summary>
    /// This private method will unregister the services created
    /// and delete the driver that was dumped to disk.
    /// </summary>
    void ServiceCleanUp();

public:
    ServiceController();
    ~ServiceController();

    /// <summary>
    /// This method will create a service on the machine and 
    /// start the driver.
    /// </summary>
    /// <returns>true if successful</returns>
    bool StartKernelService();
};


/// EOF