/// \file fx_scaler.h
/// \brief This file contains classes used to scale a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "object.h"

#include "texture.h"
#include "render_target.h"

namespace gi_lib {

	namespace fx {

		/// \brief This class is used to scale a texture using the GPU.
		/// You may use this class both to copy a texture or to scale it to another size. The destination texture must be a valid render target.
		class FxScaler {

		public:

			/// \brief Copy the given source texture on top of the destination.
			/// If the textures do not match in sizes, the source texture is resized accordingly.
			/// \param source The source texture to copy.
			/// \param destination The destination render target.
			/// \remarks The source image will be pasted only on the first surface of the render target.
			virtual void Copy(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) = 0;
			
		};


	}

}