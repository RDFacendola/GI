#include "system_services.h"

SystemServices & SystemServices::GetSingleton(){

	static SystemServices singleton;

	return singleton;

}

wstring SystemServices::GetApplicationPath() const{

	wchar_t path_buffer[MAX_PATH + 1];

	GetModuleFileName(0, path_buffer, sizeof(path_buffer));

	return wstring(path_buffer);

}

wstring SystemServices::GetApplicationName(bool extension) const{

	wstring path = GetApplicationPath();

	unsigned int dot = static_cast<unsigned int>(path.find_last_of(L"."));

	unsigned int slash = static_cast<unsigned int>(path.find_last_of(L"\\"));

	wstring application_name = path.substr(slash + 1, dot - slash - 1);

	return application_name;

}