/// \file dx11deferred_renderer.h
/// \brief Deferred rendering classes for DirectX11.
///
/// \author Raffaele D. Facendola

#pragma once

#include "..\dx11graphics.h"
#include "..\..\..\include\renderers\deferred_renderer.h"

namespace gi_lib{

	/// \brief Deferrend renderer with tiled lighting computation for DirectX11.
	/// \author Raffaele D. Facendola
	class DX11TiledDeferredRenderer : public TiledDeferredRenderer{

	public:

		/// \brief Virtual destructor.
		virtual ~DX11TiledDeferredRenderer();

		virtual Scene& GetScene() override;

		virtual const Scene& GetScene() const override;

	};


}
