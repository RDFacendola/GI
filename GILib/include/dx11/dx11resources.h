/// \file resources.h
/// \brief Declare classes and interfaces used to manage directx11 graphical resources.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <d3d11.h>
#include <memory>
#include <type_traits>

#include "resources.h"

using ::std::shared_ptr;

namespace gi_lib{

	namespace dx11{

		/// \brief Resource manager interface for DirectX11.
		/// \author Raffaele D. Facendola.
		class DX11Resources: public Resources{

		public:

			/// \brief No copy constructor.
			DX11Resources(const DX11Resources &) = delete;

			/// \brief No assignment operator.
			DX11Resources & operator=(const DX11Resources &) = delete;

			/// \brief Create a new instance of DirectX11 resource manager.
			/// \param device The device used to create the actual resources.
			DX11Resources(ID3D11Device & device) :
				device_(device){}

		protected:

			virtual shared_ptr<Resource> Load(const wstring & path, const std::type_index & type_index);

		private:

			ID3D11Device & device_;

		};

	}

}

#else

#error "Unsupported platform"

#endif