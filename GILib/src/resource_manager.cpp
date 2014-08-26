#include "resource_manager.h"

#include "core.h"
#include "resource.h"

#include <numeric>

using namespace std;
using namespace gi_lib;

const wstring ResourceManager::kResourceFolder = L"Data";

ResourceManager::ResourceManager(){

	base_path_ = Application::GetInstance().GetDirectory() + kResourceFolder + kPathSeparator;

}

size_t ResourceManager::GetSize(){

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