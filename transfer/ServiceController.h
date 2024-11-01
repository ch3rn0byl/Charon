#pragma once
#include "Resources.h"

class ServiceController 
{
public:
	ServiceController();
	~ServiceController();

	bool StartKernelService();

private:
	std::unique_ptr<Resources> m_Resources;

	SC_HANDLE m_schSCManager;
	SC_HANDLE m_schService;

	void ServiceCleanUp();
};

