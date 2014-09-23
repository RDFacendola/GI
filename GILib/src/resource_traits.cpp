#include "..\include\resource_traits.h"

#include "..\external\fnv1.h"

using namespace gi_lib;
using namespace std;

void LoadSettings< Texture2D, Texture2D::LoadMode::kFromDDS >::FillTag(void * buffer, size_t size) const{

	// The tag of this load setting is just the hash of the texture's path

	std::string wfile(file_name.begin(), file_name.end());

	size_t hash = ::hash::fnv_1{}(wfile);

	memcpy_s(buffer, size, &hash, sizeof(hash));

}

void LoadSettings< Shader, Shader::LoadMode::kCompileFromFile >::FillTag(void * buffer, size_t size) const{

	// The tag of this load setting is just the hash of the shader's path

	std::string wfile(file_name.begin(), file_name.end());

	size_t hash = ::hash::fnv_1{}(wfile);

	memcpy_s(buffer, size, &hash, sizeof(hash));

}
