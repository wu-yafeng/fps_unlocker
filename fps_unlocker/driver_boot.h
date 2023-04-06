#pragma once

#include <Windows.h>

int StartDriverService(const char* driverFileName)
{
	printf("install hook driver : %s \n", driverFileName);

	SC_HANDLE hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hManager == NULL)
	{
		return GetLastError();
	}

	SC_HANDLE hService = OpenService(hManager, "dxgkrnl_hook", SERVICE_ALL_ACCESS);

	if (hService != NULL)
	{
		DeleteService(hService);
		CloseServiceHandle(hService);

		hService = NULL;
	}

	// create if not exists.
	if (hService == NULL)
	{
		hService = CreateService(
			hManager,
			"dxgkrnl_hook",
			"dxgkrnl_hook",
			SERVICE_ALL_ACCESS,
			SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_IGNORE,
			driverFileName,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		);
	}

	CloseServiceHandle(hManager);

	if (hService == NULL)
	{

		return GetLastError();
	}

	SERVICE_STATUS status;

	if (QueryServiceStatus(hService, &status) == TRUE && status.dwCurrentState != SERVICE_RUNNING)
	{
		// START SERVICE
		if (StartService(hService, NULL, NULL) == FALSE)
		{
			CloseServiceHandle(hService);
			return GetLastError();
		}
	}
	CloseServiceHandle(hService);

	return NO_ERROR;
}