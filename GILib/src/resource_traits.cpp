#include "..\include\resource_traits.h"

#include "..\external\fnv1.h"

using namespace gi_lib;
using namespace std;

size_t LoadSettings< Texture2D, Texture2D::LoadMode::kFromDDS >::GetCacheKey() const{

	// The tag of this load setting is just the hash of the texture's path

	std::string wfile(file_name.begin(), file_name.end());

	return ::hash::fnv_1{}(wfile);

}

size_t LoadSettings< Material, Material::LoadMode::kFromShader >::GetCacheKey() const{

	// The tag of this load setting is just the hash of the texture's path

	std::string wfile(file_name.begin(), file_name.end());

	return ::hash::fnv_1{}(wfile);

}