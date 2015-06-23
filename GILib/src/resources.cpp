#include "resources.h"

#include "fnv1.h"
#include "gilib.h"

using namespace gi_lib;
using namespace std;

size_t Material::CompileFromFile::GetCacheKey() const{

	return ::hash::fnv_1{}( to_string(file_name) );

}