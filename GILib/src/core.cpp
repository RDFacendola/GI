#include "core.h"

#include "exceptions.h"
#include "timer.h"

#include "windows/win_core.h"

using namespace gi_lib;
using namespace std;

//////////////////////////////////// SYSTEM /////////////////////////////////////////

System& System::GetInstance(){

#ifdef _WIN32

	return windows::System::GetInstance();

#endif
	
}

////////////////////// FILESYSTEM ////////////////////////////////////////

FileSystem& FileSystem::GetInstance(){

#ifdef _WIN32

	return windows::FileSystem::GetInstance();

#endif

}

////////////////////// APPLICATION ///////////////////////////////

Application& Application::GetInstance(){

#ifdef _WIN32

	return windows::Application::GetInstance();

#endif

}