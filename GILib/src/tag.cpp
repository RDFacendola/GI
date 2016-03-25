#include "tag.h"

#include "fnv1.h"
#include "gilib.h"

using namespace ::gi_lib;

Tag::Tag(const std::string& string){

	tag_ = hash::fnv_1{}(string);

	DEBUG_ONLY(name_ = string;)

}


Tag::Tag(const char* string){

	tag_ = hash::fnv_1{}(string);

	DEBUG_ONLY(name_ = string;)

}

Tag::Tag(const std::wstring& string) :
Tag(to_string(string)){}


Tag::Tag(const wchar_t* string) :
Tag(to_string(string)){}