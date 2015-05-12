/// \file dx11renderer.h
/// \brief Shared classes for DirectX11 renderers
///
/// \author Raffaele D. Facendola

#pragma once

namespace gi_lib{

	class Scene;

	namespace dx11{

		/// \brief Default arguments used to create all the DirectX11 renderers.
		struct RendererConstructionArgs{

			/// \brief No default constructor.
			RendererConstructionArgs() = delete;

			/// \brief Explicit constructor.
			/// \param scene Scene that will be rendered.
			RendererConstructionArgs(Scene& scene);

			/// \brief Copy constructor.
			/// \param other Other instance to copy.
			RendererConstructionArgs(const RendererConstructionArgs& other);

			/// \brief No assignment operator.
			RendererConstructionArgs& operator=(const RendererConstructionArgs&) = delete;

			/// \brief Scene to render.
			Scene& scene;

		};

		inline RendererConstructionArgs::RendererConstructionArgs(Scene& scene) :
			scene(scene){}

		inline RendererConstructionArgs::RendererConstructionArgs(const RendererConstructionArgs& other) :
			scene(other.scene){}

	}

}