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

	class ITexture2D;
	class IRenderTargetCache;

	namespace dx11 {

		class DX11Computation;
		class DX11Material;
		class DX11GPStructuredArray;
		class DX11RenderTarget;
		class DX11StructuredBuffer;
		class DX11Mesh;

		class DX11FxScale;

		class DX11DeferredRenderer;

		class DX11Voxelization {

		public:
			
			/// \brief Create a new voxel processor.
			/// \param voxel_size Size of each voxel in world units.
			/// \param voxel_resolution Amount of voxels along each axis for each cascade. Will be approximated to the next power of 2.
			/// \param cascades Number of cascades inside the voxel clipmap 3D.
			DX11Voxelization(DX11DeferredRenderer& renderer, float voxel_size, unsigned int voxel_resolution, unsigned int cascades);

			/// \brief Set the voxel grid resolution.
			/// \param voxel_resolution Amount of voxels along each axis for each cascade. Will be approximated to the next power of 2.
			/// \param cascades Number of cascades inside the voxel clipmap 3D.
			void SetVoxelResolution(unsigned int voxel_resolution, unsigned int cascades);

			/// \brief Set the voxel grid size.
			/// \param voxel_size Size of each voxel in world units.
			void SetVoxelSize(float voxel_size);

			/// \brief Update the voxel structure.
			void Update(const FrameInfo& frame_info);

			/// \brief Draw the voxel structure. Debug function.
			/// \param output Surface the structure will be drawn onto.
			ObjectPtr<ITexture2D> DrawVoxels(const ObjectPtr<ITexture2D>& image);

			/// \brief Get the total grid size.
			float GetGridSize() const;

		private:

			/// \brief Create the debug voxel mesh
			void BuildVoxelMesh();

			ObjectPtr<DX11Computation> voxel_shader_;							///< \brief Shader performing the dynamic voxelization.

			ObjectPtr<DX11GPStructuredArray> voxel_address_table_;				///< \brief This structure contains the address of each voxel inside the 3D texture. 
																				///			An address equal to 0 means that the voxel is not present at the specified location.
			
			ObjectPtr<DX11Computation> clear_voxel_address_table_;				///< \brief Compute shader used to clear the voxel address table.

			ObjectPtr<DX11StructuredBuffer> per_object_;						///< \brief Per-object constant buffer.

			ObjectPtr<DX11StructuredBuffer> voxel_parameters_;					///< \brief Constant parameters for voxelization.

			ObjectPtr<DX11Material> voxel_material_;							///< \brief Material used to voxelize the scene.

			ObjectPtr<DX11RenderTarget> voxel_render_target_;					///< \brief Render target used during the voxelization. This technically is not needed at all.
		
			COMPtr<ID3D11DepthStencilState> depth_stencil_state_;				///< \brief Depth-stencil state used to disable Z testing.

			COMPtr<ID3D11RasterizerState> rasterizer_state_;					///< \brief Rasterizer state.

			float voxel_size_;													///< \brief Size of each voxel in world units.

			unsigned int voxel_resolution_;										///< \brief Amount of voxels along each axis for each cascade. Must be a power of 2.

			unsigned int cascades_;												///< \brief Number of cascades inside the voxel clipmap 3D.

			// Debug stuff for voxel drawing - We don't care about performances here, it's debug stuff :)

			DX11DeferredRenderer& renderer_;

			ObjectPtr<DX11RenderTarget> output_;								///< \brief Contains the result of the post processing.

			ObjectPtr<IRenderTargetCache> render_target_cache_;					///< \brief Cache of render-target textures.

			ObjectPtr<DX11GPStructuredArray> voxel_draw_indirect_args_;			///< \brief Buffer containing the argument buffer used to dispatch the DrawInstancedIndirect call
																				///			Used to dispatch a DrawIndexedInstancedIndirect

			ObjectPtr<DX11GPStructuredArray> voxel_append_buffer_;				///< \brief Append buffer containing the debug voxel info (namely their center and their size).

			ObjectPtr<DX11Computation> append_voxel_info_;						///< \brief Compute shader used to append voxel info inside the voxel append buffer.

			ObjectPtr<DX11StructuredBuffer> per_frame_;							///< \brief Per-frame constant buffer used during voxel draw.

			COMPtr<ID3D11RasterizerState> wireframe_rasterizer_state_;			///< \brief Rasterizer state for wireframe visualization of the voxels.

			ObjectPtr<DX11Material> wireframe_voxel_material_;					///< \brief Material used to draw the wireframe voxels.

			std::unique_ptr<DX11FxScale> scaler_;								///< \brief Used to copy the input image.

			ObjectPtr<DX11Mesh> voxel_mesh_;									///< \brief Voxel mesh.
			
		};

	}

}