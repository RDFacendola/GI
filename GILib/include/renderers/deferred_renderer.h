/// \file deferred_renderer.h
/// \brief Deferred rendering classes.
///
/// \author Raffaele D. Facendola

#pragma once

#include "..\graphics.h"
#include "..\resources.h"

namespace gi_lib{

	struct Blah{

	};

	/// \brief Deferrend renderer with tiled lighting computation.
	/// \author Raffaele D. Facendola
	class TiledDeferredRenderer : public IRenderer{

	public:

		/// \brief Virtual destructor.
		virtual ~TiledDeferredRenderer();

	};



}



