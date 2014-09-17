#include "..\include\shell_utils.h"
#include <codecvt>

#include <sstream>

using namespace ::std;
using namespace ::gi_lib;

//Executes a command and return the value
string ShellUtils::Execute(const string & command){

#if defined _WIN32

	//Windows stuffs
	FILE* pipe = _popen(command.c_str(), "r");

#else

	//Unix stuffs
	FILE* pipe = popen(command.c_str(), "r");

#endif


	if (!pipe){

		return "";

	}

	char buffer[128];

	stringstream result;

	while (!feof(pipe)) {

		if (fgets(buffer, 128, pipe) != NULL){

			result << buffer;

		}

	}

#if defined _WIN32

	_pclose(pipe);

#else

	//Unix stuffs
	pclose(pipe);

#endif

	return result.str();

}