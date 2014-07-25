#pragma once

#include <Windows.h>
#include <string>

using ::std::wstring;

/// Utility class for system services
class SystemServices{

public:

	static SystemServices & GetSingleton();

	/// Get the full application path
	wstring GetApplicationPath() const;

	/// Get the application name
	/** \param extension Set it to true if the name should include the extension, set it to false otherwise */
	wstring GetApplicationName(bool extension = true) const;

private:

	SystemServices(){}

};