#include "..\include\resources.h"

#include "..\external\fnv1.h"

#include "..\include\gilib.h"

using namespace gi_lib;
using namespace std;

size_t Texture2D::FromFile::GetCacheKey() const{

	return ::hash::fnv_1{}( to_string(file_name) );

}

size_t Material::CompileFromFile::GetCacheKey() const{

	return ::hash::fnv_1{}( to_string(file_name) );

}