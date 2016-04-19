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

			static const Tag kVoxelSHTag;			///< \brief Tag associated to the 3D texture containing informations about the spherical harmonics stored for each voxel.

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
			/// \param bool xray Whether to enable or disable the depth test
			ObjectPtr<ITexture2D> DrawVoxels(const ObjectPtr<ITexture2D>& image, bool xray);

			/// \brief Draw the SH data. Debug function.
			/// \param output Surface the SH will be drawn onto.
			/// \param bool xray Whether to enable or disable the depth test
			ObjectPtr<ITexture2D> DrawSH(const ObjectPtr<ITexture2D>& image, bool xray);

			/// \brief Get the structure containing the pointers to the actual voxel informations.
			ObjectPtr<IGPStructuredArray> GetVoxelAddressTable() const;

			/// \brief Get the texture 3D containing the informations about the spherical harmonics stored for each voxel.
			ObjectPtr<IGPTexture3D> GetVoxelSH() const;

			/// \brief Get the constant buffer containing the voxelization parameters.
			ObjectPtr<IStructuredBuffer> GetVoxelizationParams() const;

			/// \brief Get the total grid size.
			float GetGridSize() const;

			/// \brief Get the amount of voxel along each axis for each cascade.
			/// It is a power of two
			unsigned int GetVoxelResolution() const;

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

			ObjectPtr<DX11GPTexture3D> voxel_sh_;								///< \brief Contains the spherical harmonics for each voxel.
																				///			The coefficients increases along the X axis.
																				///			The cascades increase along the Y axis.
																				///			The channels increase along the Z axis.

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
		
		inline ObjectPtr<IGPTexture3D> DX11Voxelization::GetVoxelSH() const {

			return ObjectPtr<IGPTexture3D>(voxel_sh_);

		}

		inline ObjectPtr<IGPStructuredArray> DX11Voxelization::GetVoxelAddressTable() const {

			return ObjectPtr<IGPStructuredArray>(voxel_address_table_);

		}

		inline ObjectPtr<IStructuredBuffer> DX11Voxelization::GetVoxelizationParams() const {

			return ObjectPtr<IStructuredBuffer>(cb_voxelization_);

		}

		inline unsigned int DX11Voxelization::GetVoxelResolution() const {

			return voxel_resolution_;

		}

	}

}