/// \file dx11.h
/// \brief Utility and wrapper methods for DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <string>

#include <d3d11.h>


#include "enums.h"
#include "gimath.h"
#include "scope_guard.h"
#include "windows/win_os.h"

using ::std::vector;
using ::std::string;
using ::std::wstring;

namespace gi_lib{

	namespace dx11{

		/// \brief Texture mapping technique.
		enum class TextureMapping : unsigned int{

			WRAP,				///< \brief Repeat the texture for texture coordinates outside the boundary [0;1] every integer.
			CLAMP				///< \brief Texture coordinates below 0 or above 1 are set to 0 and 1 respectively instead.

		};

		/// \brief Create a depth stencil suitable for the provided target.
		/// \param device Device used to create the texture.
		/// \param width The width of the depth stencil texture in pixels.
		/// \param height The height of the depth stencil texture in pixels.
		/// \param depth_stencil Pointer to the created depth stencil. Set to nullptr if not needed.
		/// \param depth_stencil_view Pointer to the depth stencil view. Set to nullptr if not needed.
		HRESULT MakeDepthStencil(ID3D11Device& device, unsigned int width, unsigned int height, ID3D11Texture2D** depth_stencil, ID3D11DepthStencilView** depth_stencil_view);

		/// \brief Create a render target.
		/// \param device Device used to create the texture.
		/// \param width Width of the texture in pixels.
		/// \param height Height of the texture in pixels.
		/// \param format Format of the surface.
		/// \param texture Pointer to the created texture.
		/// \param render_target_view Pointer to the render target view. Optional.
		/// \param shader_resource_view Pointer to the shader resource view. Optional.
		/// \param unordered_access_view Pointer to the unordered access view. Optional.
		/// \param autogenerate_mips Whether to autogenerate mipmaps or not.
		/// \remarks The method expects both the texture, the render target view and the shader resource view to be not null.
		HRESULT MakeRenderTarget(ID3D11Device& device, unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11RenderTargetView** render_target_view, ID3D11ShaderResourceView** shader_resource_view, ID3D11UnorderedAccessView** unordered_access_view, bool autogenerate_mips = false);

		/// \brief Create a vertex buffer.
		/// \tparam TVertexFormat Format of the vertex.
		/// \param device Device used to create the vertex buffer.
		/// \param vertices Pointer to the firt vertex.
		/// \param size Total size of the vertex buffer in bytes.
		/// \param buffer Pointer to the object that will hold the buffer.
		HRESULT MakeVertexBuffer(ID3D11Device& device, const void* vertices, size_t size, ID3D11Buffer** buffer);

		/// \brief Create an index buffer.
		/// \param device Device used to create the index buffer.
		/// \param indices Pointer to the first index.
		/// \param size Total size of the index buffer in bytes.
		/// \param buffer Pointer to the object that will hold the buffer.
		HRESULT MakeIndexBuffer(ID3D11Device& device, const unsigned int* indices, size_t size, ID3D11Buffer** buffer);

		/// \brief Create a constant buffer.
		/// \param device Device used to create the constant buffer.
		/// \param size Size of the constant buffer in bytes.
		/// \param buffer Pointer to the object that will hold the buffer.
		HRESULT MakeConstantBuffer(ID3D11Device& device, size_t size, ID3D11Buffer** buffer);

		/// \brief Create a structured buffer.
		/// \tparam TElement Type of each element in the buffer.
		/// \param device Device used to create the structured buffer.
		/// \param elements Number of elements inside the buffer.
		/// \param dynamic Whether the buffer can be written by the CPU or not.
		/// \param buffer Pointer to the object that will hold the buffer.
		/// \param shader_resource_view Pointer to the shader resource view. If nullptr is specified, the buffer won't have any shader resource view associated to it.
		/// \param unordered_access_view Pointer to the unordered access view. If nullptr is specified, the buffer won't have any unordered access view associated to it.
		/// \remarks A dynamic buffer may not be written by the GPU, thus it must not define an unordered access view.
		HRESULT MakeStructuredBuffer(ID3D11Device& device, unsigned int element_count, unsigned int element_size, bool dynamic, ID3D11Buffer** buffer, ID3D11ShaderResourceView** shader_resource_view, ID3D11UnorderedAccessView** unordered_access_view);


		/// \brief Create a sampler state.
		/// \param device Device used to create the sampler.
		/// \param texture_mapping Texture mapping mode while sampling.
		/// \param anisotropy_level Maximum anisotropy level. Set to 0 to disable anisotropic filtering and enable trilinear filtering.
		/// \param sampler Pointer to the object that will hold the sampler if the method succeeds..
		HRESULT MakeSampler(ID3D11Device& device, TextureMapping texture_mapping, unsigned int anisotropy_level, ID3D11SamplerState** sampler);

		/// \brief Compute the left-handed perspective projection matrix.
		/// \param field_of_view Field of view, in radians.
		/// \param aspect_ratio Width-to-height aspect ratio.
		/// \param near_plane Distance of the near clipping plane.
		/// \param far_plane Distance of the far clipping plane.
		Matrix4f ComputePerspectiveProjectionLH(float field_of_view, float aspect_ratio, float near_plane, float far_plane);
		
	}

}

////////////////////////////// INLINE IMPLEMENTATION /////////////////////////////////
