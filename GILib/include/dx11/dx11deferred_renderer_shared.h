/// \file dx11deferred_renderer_shared.h
/// \brief This file contains shared classes and methods for deferred rendering under DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

#include "gimath.h"

namespace gi_lib {

	class Scene;
	class CameraComponent;

	namespace dx11 {

		/// \brief Information about the frame being rendered.
		struct FrameInfo {

			Scene* scene;

			CameraComponent* camera;

			Matrix4f view_matrix;

			Matrix4f view_proj_matrix;

			unsigned int width;

			unsigned int height;

			float aspect_ratio;

			float time_delta;					///< \brief Time passed since the last frame, in seconds.

			bool enable_global_illumination;	///< \brief Whether the global illumination is active or not.
		};


	}

}
