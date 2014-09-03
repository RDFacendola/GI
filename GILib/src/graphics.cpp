#include "..\include\graphics.h"

#include <numeric>

#include "..\include\exceptions.h"
#include "..\include\core.h"
#include "..\include\resources.h"

#include "dx11\dx11graphics.h"

using namespace std;
using namespace gi_lib;

//////////////////////// MANAGER //////////////////////////////

Manager::Manager(){}

size_t Manager::GetSize(){

	// Runs trough every resource and sum its memory footprint.

	return accumulate(resources_.begin(),
		resources_.end(),
		static_cast<size_t>(0),
		[](size_t accumulator, const ResourceMap::value_type & it){

			if (auto resource = std::get<1>(it.second).lock()){

				accumulator += resource->GetSize();

			}

			return accumulator;

		});

}

////////////////////// GRAPHICS ////////////////////////////////

Graphics & Graphics::GetAPI(API api){

	switch (api){

		case API::DIRECTX_11:

			return dx11::DX11Graphics::GetInstance();

			break;

		default:

			throw RuntimeException(L"Specified API is not supported.");

	}

}