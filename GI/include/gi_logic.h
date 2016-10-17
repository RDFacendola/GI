#pragma once

#include <memory>

#include "core.h"
#include "graphics.h"
#include "scene.h"
#include "postprocess.h"

using namespace gi_lib;
using namespace std;

namespace gi_lib{
	
	class DeferredRenderer;
	class TransformComponent;

}

namespace gi{

	///Application's logic
	class GILogic : public IWindowLogic{

	public:

		GILogic();

		virtual ~GILogic();

	protected:

		virtual void Initialize(Window& window) override;

		virtual void Update(const Time & time) override;
		
	private:

		void SetupLights(Scene& scene, ObjectPtr<IStaticMesh> point_light_mesh);

		std::vector<TransformComponent*> point_lights;

		std::vector<TransformComponent*> directional_lights;

		Graphics& graphics_;

		unique_ptr<IOutput> output_;

		unique_ptr<DeferredRenderer> deferred_renderer_;

		unique_ptr<Scene> scene_;

		unique_ptr<Postprocess> postprocess_;
		
		const IInput* input_;

		bool paused_;

		bool enable_postprocess_;				///< \brief Whether the post processing is enabled.

		bool enable_global_illumination_;		///< \brief Whether the global illumination is enabled.

		bool enable_voxel_draw_;				///< \brief Whether to draw voxels or not.

		bool enable_sh_draw_;					///< \brief Whether to draw spherical harmonics or not.

		bool lock_camera_;						///< \brief Whether the camera is locked or not.

	};

}

