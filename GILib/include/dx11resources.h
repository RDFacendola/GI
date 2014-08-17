/// \file resources.h
/// \brief Declare classes and interfaces used to manage directx11 graphical resources.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <d3d11.h>
#include <memory>

#include "resources.h"
#include "handle.h"

using ::std::shared_ptr;

namespace gi_lib{

	namespace dx11{

		class DX11Texture2D;
		class DX11Texture3D;
		class DX11Mesh;

		template <typename TResource>
		struct resource_traits;

		template <>
		struct resource_traits < Texture2D > {

			using type = DX11Texture2D;

		};

		template <>
		struct resource_traits < Texture3D > {

			using type = DX11Texture3D;

		};

		template <>
		struct resource_traits < Mesh > {

			using type = DX11Mesh;

		};

		/// \brief Resource manager interface for DirectX11.
		/// \author Raffaele D. Facendola.
		class DX11Resources: public Resources{

		public:

			/// \brief Create a new instance of DirectX11 resource manager.
			/// \param device The device used to create the actual resources.
			DX11Resources(ID3D11Device & device) :
				device_(device){}

			/// \brief Get a resource by handle.
			/// \tparam Type of resource to get.
			/// \param handle The handle of the resource to get.
			/// \return Returns a pointer to the given resource if any. Returns null if the specified handle was invalid.
			template <typename TResource>
			typename shared_ptr<typename resource_traits<TResource>::type> Get(const Handle<TResource> & handle){

				return nullptr;

			}

		private:

			ID3D11Device & device_;

		};


		/// \brief DirectX 11 plain texture interface.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D: public Texture2D{

		public:

		};

		/// \brief DirectX 11 volumetric texture interface.
		/// \author Raffaele D. Facendola.
		class DX11Texture3D: public Texture3D{

		public:

		};

		/// \brief DirectX11 3D model interface.
		/// \author Raffaele D. Facendola.
		class DX11Mesh: public Mesh{

		public:

		};

	}

}

#else

static_assert(false, "DirectX is not supported");

#endif