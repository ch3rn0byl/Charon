#include "Privileges.h"

bool Privileges::CheckAdminPrivileges()
{
	SID_IDENTIFIER_AUTHORITY NtAuthortiy = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup = nullptr;

	BOOL fRetVal = AllocateAndInitializeSid(
		&NtAuthortiy,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		NULL, NULL, NULL, NULL, NULL, NULL,
		&AdministratorsGroup
	);
	if (fRetVal)
	{
		//
		// Check to see if the callee process was ran privileged.
		// 
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &fRetVal))
		{
			fRetVal = false;
		}
		FreeSid(AdministratorsGroup);
	}

	return fRetVal;
}
