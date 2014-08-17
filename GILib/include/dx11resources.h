/// \file resources.h
/// \brief Declare classes and interfaces used to manage directx11 graphical resources.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <d3d11.h>

#include "resources.h"

namespace gi_lib{

	class DX11Texture2D;
	class DX11Texture3D;
	class DX11Mesh;

	/// \brief Resource manager interface for DirectX11.
	/// \author Raffaele D. Facendola.
	class DX11Resources: public IResources{

	public:

		template <typename TResource>
		struct resource_traits;

		template <>
		struct resource_traits < ITexture2D > {

			using type = DX11Texture2D;

		};

		template <>
		struct resource_traits < ITexture3D > {

			using type = DX11Texture3D;

		};

		template <>
		struct resource_traits < IMesh > {

			using type = DX11Mesh;

		};

		template <typename TResource>
		typename resource_traits<TResource>::type * Get(){

			return nullptr;

		}

	private:


	};


	/// \brief DirectX 11 plain texture interface.
	/// \author Raffaele D. Facendola.
	class DX11Texture2D: public ITexture2D{

	public:

	};

	/// \brief DirectX 11 volumetric texture interface.
	/// \author Raffaele D. Facendola.
	class DX11Texture3D: public ITexture3D{

	public:

	};

	/// \brief DirectX11 3D model interface.
	/// \author Raffaele D. Facendola.
	class DX11Mesh: public IMesh{

	public:

	};

}

#else

static_assert(false, "DirectX is not supported");

#endif