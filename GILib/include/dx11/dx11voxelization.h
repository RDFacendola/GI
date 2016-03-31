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
	class IGPTexture3D;
	class IRenderTargetCache;

	namespace dx11 {

		class DX11Computation;
		class DX11Material;
		class DX11GPStructuredArray;
		class DX11RenderTarget;
		class DX11StructuredBuffer;
		class DX11Mesh;	
		class DX11GPTexture3D;

		class DX11FxScale;

		class DX11DeferredRenderer;

		class DX11Voxelization {

		public:
			
			static const Tag kRedSH01Tag;			///< \brief Tag associated to the 3D texture containing informations about the first and the second SH coefficients of the red channel.

			static const Tag kGreenSH01Tag;			///< \brief Tag associated to the 3D texture containing informations about the first and the second SH coefficients of the green channel.

			static const Tag kBlueSH01Tag;			///< \brief Tag associated to the 3D texture containing informations about the first and the second SH coefficients of the blue channel.

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

			/// \brief Clear the current content of the spherical harmonics structure.
			void ClearSH();

			/// \brief Get the structure containing the first and the second SH coefficients of the red channel.
			ObjectPtr<IGPTexture3D> GetRedSH() const;

			/// \brief Get the structure containing the first and the second SH coefficients of the green channel.
			ObjectPtr<IGPTexture3D> GetGreenSH() const;

			/// \brief Get the structure containing the first and the second SH coefficients of the blue channel.
			ObjectPtr<IGPTexture3D> GetBlueSH() const;

			/// \brief Get the total grid size.
			float GetGridSize() const;

		private:

			/// \brief Create the debug voxel mesh
			void BuildVoxelMesh();

			ObjectPtr<DX11Computation> voxel_shader_;							///< \brief Shader performing the dynamic voxelization.

			ObjectPtr<DX11GPStructuredArray> voxel_address_table_;				///< \brief This structure contains the address of each voxel inside the 3D texture of the spherical harmonics. 
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

			// Spherical harmonics

			ObjectPtr<DX11Computation> clear_sh_;								///< \brief Compute shader used to clear the spherical harmonics structure

			ObjectPtr<DX11GPTexture3D> voxel_red_sh_01_;						///< \brief Contains the first and the second SH for the red channel. The pyramid is not stored here. Consecutive cascades are stored contiguously along the Z axis.

			ObjectPtr<DX11GPTexture3D> voxel_green_sh_01_;						///< \brief Contains the first and the second SH for the green channel. The pyramid is not stored here. Consecutive cascades are stored contiguously along the Z axis.

			ObjectPtr<DX11GPTexture3D> voxel_blue_sh_01_;						///< \brief Contains the first and the second SH for the blue channel. The pyramid is not stored here. Consecutive cascades are stored contiguously along the Z axis.

			// Debug stuff for voxel drawing - We don't care about performances here, it's debug stuff :)

			DX11DeferredRenderer& renderer_;

			ObjectPtr<DX11RenderTarget> output_;								///< \brief Contains the result of the post processing.

			ObjectPtr<IRenderTargetCache> render_target_cache_;					///< \brief Cache of render-target textures.

			ObjectPtr<DX11Computation> clear_voxel_draw_indirect_args_;			///< \brief Compute shader used to clear the voxel draw indirect arguments.

			ObjectPtr<DX11GPStructuredArray> voxel_draw_indirect_args_;			///< \brief Buffer containing the argument buffer used to dispatch the DrawInstancedIndirect call
																				///			Used to dispatch a DrawIndexedInstancedIndirect

			ObjectPtr<DX11GPStructuredArray> voxel_append_buffer_;				///< \brief Append buffer containing the debug voxel info (namely their center and their size).

			ObjectPtr<DX11Computation> append_voxel_info_;						///< \brief Compute shader used to append voxel info inside the voxel append buffer.

			ObjectPtr<DX11StructuredBuffer> per_frame_;							///< \brief Per-frame constant buffer used during voxel draw.

			COMPtr<ID3D11RasterizerState> wireframe_depth_bias_enable_;

			COMPtr<ID3D11RasterizerState> wireframe_depth_bias_disable_;			

			COMPtr<ID3D11BlendState> wireframe_disable_color_;

			COMPtr<ID3D11BlendState> wireframe_enable_color_;

			COMPtr<ID3D11DepthStencilState> wireframe_zprepass_depth_state_;	///< \brief Depth-stencil state used to enable Z testing.

			COMPtr<ID3D11DepthStencilState> wireframe_zequal_depth_state_;		///< \brief Depth-stencil state used to enable Z testing.

			ObjectPtr<DX11Material> wireframe_voxel_material_;					///< \brief Material used to draw the wireframe voxels.

			std::unique_ptr<DX11FxScale> scaler_;								///< \brief Used to copy the input image.

			ObjectPtr<DX11Mesh> voxel_cube_;									///< \brief Voxel mesh.
			
			ObjectPtr<DX11Mesh> voxel_edges_;									///< \brief Edges of the voxel.

		};

		////////////////////////////////////////////// DX11 VOXELIZATION //////////////////////////////////////////////

		inline ObjectPtr<IGPTexture3D> DX11Voxelization::GetRedSH() const {

			return ObjectPtr<IGPTexture3D>(voxel_red_sh_01_);

		}
				
		inline ObjectPtr<IGPTexture3D> DX11Voxelization::GetGreenSH() const {

			return ObjectPtr<IGPTexture3D>(voxel_green_sh_01_);

		}

		inline ObjectPtr<IGPTexture3D> DX11Voxelization::GetBlueSH() const {

			return ObjectPtr<IGPTexture3D>(voxel_blue_sh_01_);

		}

	}

}