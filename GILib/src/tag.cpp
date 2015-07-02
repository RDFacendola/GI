#include "tag.h"

#include "fnv1.h"

using namespace ::gi_lib;

Tag::Tag(const std::string& string){

	tag_ = hash::fnv_1{}(string);

}


Tag::Tag(const char* string, size_t size){

	tag_ = hash::fnv_1{}(std::string(&string[0],
									 &string[size]));

}