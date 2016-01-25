/// \file dx11voxelization.h
/// \brief Voxelization support for DirectX11.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <memory>

#include "object.h"
#include "tag.h"

#include "dx11\dx11.h"
#include "dx11\dx11deferred_renderer_shared.h"

#include "windows\win_os.h"

namespace gi_lib {
	
	namespace dx11 {

		class DX11Computation;
		class DX11Material;
		class DX11GPStructuredArray;
		class DX11RenderTarget;
		class DX11StructuredBuffer;

		class DX11Voxelization {

		public:
			
			/// \brief Create a new voxel processor.
			/// \param voxel_size Size of each voxel in world units.
			/// \param voxel_resolution Amount of voxels along each axis for each cascade. Will be approximated to the next power of 2.
			/// \param cascades Number of cascades inside the voxel clipmap 3D.
			DX11Voxelization(float voxel_size, unsigned int voxel_resolution, unsigned int cascades);

			/// \brief Set the voxel grid resolution.
			/// \param voxel_resolution Amount of voxels along each axis for each cascade. Will be approximated to the next power of 2.
			/// \param cascades Number of cascades inside the voxel clipmap 3D.
			void SetVoxelResolution(unsigned int voxel_resolution, unsigned int cascades);

			/// \brief Set the voxel grid size.
			/// \param voxel_size Size of each voxel in world units.
			void SetVoxelSize(float voxel_size);

			/// \brief Update the voxel structure.
			void Update(const FrameInfo& frame_info);

			/// \brief Get the total grid size.
			float GetGridSize() const;

		private:

			ObjectPtr<DX11Computation> voxel_shader_;							///< \brief Shader performing the dynamic voxelization.

			ObjectPtr<DX11GPStructuredArray> voxel_address_table_;				///< \brief This structure contains the address of each voxel inside the 3D texture. 
																				///			An address equal to 0 means that the voxel is not present at the specified location.
			
			ObjectPtr<DX11StructuredBuffer> per_object_;						///< \brief Per-object constant buffer.

			ObjectPtr<DX11StructuredBuffer> voxel_parameters_;					///< \brief Constant parameters for voxelization.

			ObjectPtr<DX11Material> voxel_material_;							///< \brief Material used to voxelize the scene.

			ObjectPtr<DX11RenderTarget> voxel_render_target_;					///< \brief Render target used during the voxelization. This technically is not needed at all.

			float voxel_size_;													///< \brief Size of each voxel in world units.

			unsigned int voxel_resolution_;										///< \brief Amount of voxels along each axis for each cascade. Must be a power of 2.

			unsigned int cascades_;												///< \brief Number of cascades inside the voxel clipmap 3D.

		};

	}

}