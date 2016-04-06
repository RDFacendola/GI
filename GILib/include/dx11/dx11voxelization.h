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
#include "dx11\dx11graphics.h"
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
			
			static const Tag kVoxelAddressTableTag;	///< \brief Tag associated to the structured buffer containing the voxel address table.

			static const Tag kVoxelizationTag;		///< \brief Tag associated to the constant buffer containing the voxelization constants.

			static const Tag kRedSH01Tag;			///< \brief Tag associated to the 3D texture containing informations about the first and the second SH coefficients of the red channel.

			static const Tag kGreenSH01Tag;			///< \brief Tag associated to the 3D texture containing informations about the first and the second SH coefficients of the green channel.

			static const Tag kBlueSH01Tag;			///< \brief Tag associated to the 3D texture containing informations about the first and the second SH coefficients of the blue channel.
			
			/// \brief Create a new voxel processor.
			/// \param voxel_size Size of each voxel in world units.
			/// \param voxel_resolution Amount of voxels along each axis for each cascade. Will be approximated to the next power of 2.
			/// \param cascades Number of cascades inside the voxel clipmap 3D.
			DX11Voxelization(DX11DeferredRenderer& renderer, float voxel_size, unsigned int voxel_resolution, unsigned int cascades);

			~DX11Voxelization();

			/// \brief Update the voxel structure.
			void Update(const FrameInfo& frame_info);

			/// \brief Get the total amount of voxels.
			unsigned int GetVoxelCount() const;

			/// \brief Draw the voxel structure. Debug function.
			/// \param output Surface the structure will be drawn onto.
			ObjectPtr<ITexture2D> DrawVoxels(const ObjectPtr<ITexture2D>& image);

			/// \brief Get the structure containing the first and the second SH coefficients of the specified channel.
			ObjectPtr<IGPTexture3D> GetSH(size_t channel_index) const;

			/// \brief Get the constant buffer containing the voxelization parameters.
			ObjectPtr<IStructuredBuffer> GetVoxelizationParams() const;

			/// \brief Get the total grid size.
			float GetGridSize() const;

		private:

			/// \brief Initialize the shader resources required for voxelization.
			void InitResources();

			/// \brief Initialize shaders required for voxelization.
			void InitShaders();

			/// \brief Clear voxel and spherical harmonics info.
			void Clear();

			/// \brief Class used to draw voxels and spherical harmonics
			class DebugDrawer;
			
			float voxel_size_;													///< \brief Size of each voxel in world units.

			unsigned int voxel_resolution_;										///< \brief Amount of voxels along each axis for each cascade. Must be a power of 2.

			unsigned int cascades_;												///< \brief Number of cascades inside the voxel clipmap 3D.

			DX11DeferredRenderer& renderer_;									///< \brief TODO: Remove this from here!!!

			// Shader resources

			ObjectPtr<DX11StructuredBuffer> cb_voxelization_;					///< \brief Constant parameters for voxelization.

			ObjectPtr<DX11StructuredBuffer> cb_object_;							///< \brief Per-object constant buffer.

			ObjectPtr<DX11GPStructuredArray> voxel_address_table_;				///< \brief This structure contains the address of each voxel inside the 3D texture of the spherical harmonics. 
																				///			An address equal to 0 means that the voxel is not present at the specified location.

			ObjectPtr<DX11GPTexture3D> voxel_sh_01_[3];							///< \brief Contains the first and the second SH for each channel. The pyramid is not stored here. Consecutive cascades are stored contiguously along the Z axis.

			ObjectPtr<DX11RenderTarget> voxel_render_target_;					///< \brief Render target used during the voxelization. This technically is not needed at all.
			
			DX11PipelineState voxelization_state_;								///< \brief Pipeline state for voxelization stage.
			
			// Shaders

			ObjectPtr<DX11Material> voxel_material_;							///< \brief Material used to voxelize the scene.

			ObjectPtr<DX11Computation> clear_voxel_;							///< \brief Compute shader used to clear the voxel address table.

			ObjectPtr<DX11Computation> clear_sh_;								///< \brief Compute shader used to clear the spherical harmonics structure
						
			// Debug stuffs

			std::unique_ptr<DebugDrawer> debug_drawer_;							///< \brief Object used to draw voxels and spherical harmonics

		};

		////////////////////////////////////////////// DX11 VOXELIZATION //////////////////////////////////////////////
		
		inline ObjectPtr<IGPTexture3D> DX11Voxelization::GetSH(size_t channel_index) const {

			return ObjectPtr<IGPTexture3D>(voxel_sh_01_[channel_index]);

		}

		inline ObjectPtr<IStructuredBuffer> DX11Voxelization::GetVoxelizationParams() const {

			return ObjectPtr<IStructuredBuffer>(cb_voxelization_);

		}

	}

}