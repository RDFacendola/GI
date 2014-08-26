#include "graphics.h"

#include <numeric>

#include "core.h"
#include "resources.h"

using namespace std;
using namespace gi_lib;

const wstring Manager::kResourceFolder = L"Data";

Manager::Manager(){

	base_path_ = Application::GetInstance().GetDirectory() + kResourceFolder + kPathSeparator;

}

size_t Manager::GetSize(){

	// Runs trough every resource and sum its memory footprint.

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