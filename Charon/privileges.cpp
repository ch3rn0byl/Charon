#include "privileges.h"

bool
privileges::RunningAsAdmin()
{
	SID_IDENTIFIER_AUTHORITY NtAuthortiy = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup = nullptr;

	BOOL bRetVal = AllocateAndInitializeSid(
		&NtAuthortiy,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		NULL, NULL, NULL, NULL, NULL, NULL,
		&AdministratorsGroup
	);
	if (bRetVal)
	{
		//
		// Check to see if the callee process was ran privileged.
		// 
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &bRetVal))
		{
			bRetVal = false;
		}
		FreeSid(AdministratorsGroup);
	}

	return bRetVal;
}


/// EOF