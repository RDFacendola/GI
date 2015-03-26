#include "..\include\bundles.h"

#include "..\external\fnv1.h"

using namespace gi_lib;
using namespace std;

size_t LoadFromFile::GetCacheKey() const{

	std::string wfile(file_name.begin(), file_name.end());

	return ::hash::fnv_1{}(wfile);

}

size_t CompileFromFile::GetCacheKey() const{

	std::string wfile(file_name.begin(), file_name.end());

	return ::hash::fnv_1{}(wfile);

}