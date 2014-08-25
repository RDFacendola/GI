#include "dx11/dx11resources.h"

#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include <dx11\dx11texture.h>

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;

namespace{

	template <typename TResource>
	unique_ptr<Resource> LoadResource(ID3D11Device &, const wstring &, const void * extras);

	template <typename Texture2D>
	unique_ptr<Resource> LoadResource(ID3D11Device & device, const wstring & path, const void *) {

		return make_unique<DX11Texture2D>(device, path);

	};

	// Loader class. Maps every resource with their respective loader.
	class Loader{

		using LoaderFunction = unique_ptr<Resource>(*)(ID3D11Device &, const wstring &, const void * extras);
		using LoaderMap = unordered_map < std::type_index, LoaderFunction >;

	public:

		static Loader GetInstance(){

			static Loader loader;

			return loader;

		}

		/// \brief Load a resource.
		/// \param type_index Type of the resource to load.
		/// \param device Device used to create the resource.
		/// \param path Path of the resource.
		/// \return Returns a shared pointer to the loaded resource
		unique_ptr<Resource> Load(const std::type_index & type_index, ID3D11Device & device, const wstring & path, const void * extras){

			auto it = loader_map_.find(type_index);

			if (it == loader_map_.end()){

				return unique_ptr<Resource>();	// Empty pointer

			}
				
			return it->second(device, path, extras);
			
		}

	private:

		Loader(){

			loader_map_.insert(MakeSupport<Texture2D>());

		}

		template <typename TResource>
		LoaderMap::value_type MakeSupport(){

			// Declare the support for the resources...

			return LoaderMap::value_type(type_index(typeid(TResource)), LoadResource<TResource>);

		}

		LoaderMap loader_map_;

	};

}

unique_ptr<Resource> DX11ResourceManager::LoadDirect(const ResourceKey & key, const void * extras){

	return Loader::GetInstance().Load(key.first, device_, key.second, extras);

}