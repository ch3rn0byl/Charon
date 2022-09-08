#pragma once
#include <Windows.h>

namespace privileges
{
	/// <summary>
	/// Checks to see if the callee is running with Administrator privileges.
	/// </summary>
	/// <returns>True if running with admin rights</returns>
	/// <returns>False if running without admin rights</returns>
	bool RunningAsAdmin();
};


/// EOF