/// \file dx11resource_manager.h
/// \brief Declare classes and interfaces used to manage directx11 graphical resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <memory>

#include "resource_manager.h"

using ::std::weak_ptr;
using ::std::shared_ptr;

struct ID3D11Device;

namespace gi_lib{

	enum class ResourcePriority;

	namespace dx11{

		/// \brief Resource manager interface for DirectX11.
		/// \author Raffaele D. Facendola.
		class DX11ResourceManager : public ResourceManager{

		public:

			/// \brief No copy constructor.
			DX11ResourceManager(const DX11ResourceManager &) = delete;

			/// \brief No assignment operator.
			DX11ResourceManager & operator=(const DX11ResourceManager &) = delete;

			/// \brief Create a new instance of DirectX11 resource manager.
			/// \param device The device used to create the actual resources.
			DX11ResourceManager(ID3D11Device & device) :
				device_(device){}

		protected:

			virtual unique_ptr<Resource> LoadDirect(const ResourceKey & key, const void * extras);

		private:

			ID3D11Device & device_;

		};

	}

}