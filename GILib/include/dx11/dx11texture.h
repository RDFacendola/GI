/// \file dx11texture.h
/// \brief Classes and methods for DirectX11 texture management.
///
/// \author Raffaele D. Facendola

#pragma once

#include "texture.h"

namespace gi_lib{

	namespace dx11{

		/// \brief DirectX11 plain texture.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D : public Texture2D{

		public:

			virtual size_t GetSize();

			virtual ResourcePriority GetPriority();

			virtual void SetPriority(ResourcePriority priority);

			virtual unsigned int GetWidth();

			virtual unsigned int GetHeight() ;

			virtual unsigned int GetMipMapCount();

			virtual WrapMode GetWrapMode();

			virtual void SetWrapMode(WrapMode wrap_mode);

			virtual unsigned int GetMaxAnisotropy();

			virtual void SetMaxAnisotropy(unsigned int max_anisotropy);

			virtual unsigned int GetMaxLOD();

			virtual void SetMaxLOD(unsigned int max_LOD);

		private:



		};

	}

}