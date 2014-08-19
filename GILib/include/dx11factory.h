/// \file dx11factory.h
/// \brief Abstract factory concrete implementation for DirectX11.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include "factory.h"

struct ID3D11Device;
struct IDXGIFactory;
struct IDXGIAdapter;
struct DXGI_MODE_DESC;

namespace gi_lib{

	namespace dx11{

		/// \brief DirectX11 factory class.
		/// \author Raffaele D. Facendola
		class DX11Factory : public Factory{

		public:

			/// \brief Get the DirectX11 factory singleton.
			/// \return Returns a reference to the DirectX11 factory singleton.
			static inline DX11Factory & GetInstance(){

				static DX11Factory factory;

				return factory;

			}

			/// \brief Default destructor
			~DX11Factory();

			virtual AdapterProfile GetAdapterProfile() const;

			virtual unique_ptr<Graphics> CreateGraphics(Window & window);

			virtual Resources & GetResources();

		private:

			/// \brief Hidden constructor.
			DX11Factory();

			ID3D11Device * device_;

			IDXGIFactory * factory_;

			IDXGIAdapter * adapter_;

		};

	}

}

#else

#error "Unsupported platform"

#endif