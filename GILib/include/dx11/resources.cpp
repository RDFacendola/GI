#include "resources.h"

#include <numeric>

using namespace std;
using namespace gi_lib;

size_t Resources::GetSize(){

	return accumulate(resources_.begin(),
		resources_.end(),
		static_cast<size_t>(0),
		[](size_t accumulator, const ResourceMap::value_type & it){

			if (auto resource = it.second.lock()){

				accumulator += resource->GetSize();

			}

			return accumulator;

		});

}