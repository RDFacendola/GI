#include "..\include\graphics.h"

#include <numeric>

#include "..\include\exceptions.h"
#include "..\include\core.h"
#include "..\include\resources.h"

#include "dx11\dx11graphics.h"

using namespace std;
using namespace gi_lib;

//////////////////////// MANAGER //////////////////////////////

#ifdef _WIN32

wchar_t * Manager::kPhongShaderFile = L"Data\\built-in\\phong.fx";

#endif

Manager::Manager(){}

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

//////////////// MANAGER::LOADKEY ////////////////////////////////

Manager::LoadKey::LoadKey() :
key(std::type_index(typeid(void)))
{

	// Clear the tag in case the load settings don't need to fill it entirely
	memset(tag, 0, sizeof(tag));

}

bool Manager::LoadKey::operator<(const LoadKey & other) const{

	//Strict order by key first and by tag after.

	return key < other.key ||
		key == other.key &&
		memcmp(tag, other.tag, sizeof(tag)) < 0;

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