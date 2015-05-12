/// \file dx11deferred_renderer.h
/// \brief Deferred rendering classes for DirectX11.
///
/// \author Raffaele D. Facendola

#pragma once

#include "..\..\..\include\renderers\deferred_renderer.h"

#include "..\dx11graphics.h"

namespace gi_lib{

	/// \brief Deferrend renderer with tiled lighting computation for DirectX11.
	/// \author Raffaele D. Facendola
	class DX11TiledDeferredRenderer : public TiledDeferredRenderer{

	public:

		/// \brief No copy constructor.
		DX11TiledDeferredRenderer(const DX11TiledDeferredRenderer&) = delete;

		/// \brief Virtual destructor.
		virtual ~DX11TiledDeferredRenderer();

		/// \brief No assignment operator.
		DX11TiledDeferredRenderer& operator=(DX11TiledDeferredRenderer&) = delete;

		virtual void Draw(IOutput& output) override;

	};


}
