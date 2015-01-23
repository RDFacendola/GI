#include "..\include\graphics.h"

#include <numeric>

#include "..\include\exceptions.h"
#include "..\include\core.h"
#include "..\include\resources.h"

#include "dx11\dx11graphics.h"

using namespace std;
using namespace gi_lib;

//////////////////////// RESOURCES //////////////////////////////

#ifdef _WIN32

wchar_t * Resources::kPhongShaderFile = L"Data\\built-in\\phong.fx";

#endif

Resources::Resources(){}

size_t Resources::GetSize(){

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

//////////////// MANAGER::LOADKEY ////////////////////////////////

Resources::ResourceMapKey::ResourceMapKey() :
resource_type_id(std::type_index(typeid(void))),
bundle_type_id(resource_type_id)
{}

bool Resources::ResourceMapKey::operator<(const ResourceMapKey & other) const{

	return memcmp(this, &other, sizeof(ResourceMapKey)) < 0;

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