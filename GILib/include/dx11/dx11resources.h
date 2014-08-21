/// \file resources.h
/// \brief Declare classes and interfaces used to manage directx11 graphical resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <memory>

#include "resources.h"

using ::std::weak_ptr;
using ::std::shared_ptr;

struct ID3D11Device;

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

			virtual unique_ptr<Resource> LoadDirect(const ResourceKey & key);

		private:

			ID3D11Device & device_;

		};

	}

}