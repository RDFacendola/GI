/// \file dx11.h
/// \brief Utility and wrapper methods for DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>

#include "eigen.h"

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
		/// \param shader_resource_view Pointer to the shader resource view. Set to nullptr if not needed.
		/// \param depth_stencil_view Pointer to the depth stencil view. Set to nullptr if not needed.
		HRESULT MakeDepthStencil(ID3D11Device& device, unsigned int width, unsigned int height, ID3D11ShaderResourceView** shader_resource_view, ID3D11DepthStencilView** depth_stencil_view);

		/// \brief Create a render target.
		/// \param device Device used to create the texture.
		/// \param width Width of the texture in pixels.
		/// \param height Height of the texture in pixels.
		/// \param format Format of the surface.
		/// \param render_target_view Pointer to the render target view. Optional.
		/// \param shader_resource_view Pointer to the shader resource view. Optional.
		/// \param mip_chain Whether to generate a full MIP-map chain or not.
		HRESULT MakeRenderTarget(ID3D11Device& device, unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11ShaderResourceView** shader_resource_view, ID3D11RenderTargetView** render_target_view, bool mip_chain = false);

		/// \brief Create a 2D texture that can be bound to a compute shader as unordered access.
		/// \param device Device used to create the texture.
		/// \param width Width of the texture in pixels.
		/// \param height Height of the texture in pixels.
		/// \param format Format of the surface.
		/// \param unordered_access_view Pointer to the unordered access view. Optional.
		/// \param shader_resource_view Pointer to the shader resource view. Optional.
		/// \param mip_chain Whether to generate a full MIP-map chain or not.
		HRESULT MakeUnorderedTexture(ID3D11Device& device, unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11UnorderedAccessView** unordered_access_view, ID3D11ShaderResourceView** shader_resource_view, bool mip_chain = false);

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

		/// \brief Create a depth stencil view for the specified resource.
		/// \param device Device used to create the depth stencil view.
		/// \param resource Resource the view will refer to.
		/// \param Out. Pointer containing the created depth stencil view.
		HRESULT MakeDepthStencilView(ID3D11Device& device, ID3D11Resource& resource, ID3D11DepthStencilView** depth_stencil_view);

		/// \brief Create a render target view for the specified resource.
		/// \param device Device used to create the render target view.
		/// \param resource Resource the view will refer to.
		/// \param Out. Pointer containing the created render target view.
		HRESULT MakeRenderTargetView(ID3D11Device& device, ID3D11Resource& resource, ID3D11RenderTargetView** render_target_view);

		/// \brief Create an unordered access view for the specified resource.
		/// \param device Device used to create the unordered access view.
		/// \param resource Resource the view will refer to.
		/// \param Out. Pointer containing the created unordered access view.
		HRESULT MakeUnorderedAccessView(ID3D11Device& device, ID3D11Resource& resource, ID3D11UnorderedAccessView** unordered_access_view);

		/// \brief Create a new viewport from explicit dimensions.
		/// \param width Width of the viewport in pixels.
		/// \param height Height of the viewport in pixels.
		/// \return Returns a viewport with the specified dimensions starting from [0;0] and with a depth range equal to [0;1].
		template <typename TDimensions>
		D3D11_VIEWPORT MakeViewport(TDimensions width, TDimensions height);

		/// \brief Compute the left-handed perspective projection matrix.
		/// \param field_of_view Field of view, in radians.
		/// \param aspect_ratio Width-to-height aspect ratio.
		/// \param near_plane Distance of the near clipping plane.
		/// \param far_plane Distance of the far clipping plane.
		Matrix4f ComputePerspectiveProjectionLH(float field_of_view, float aspect_ratio, float near_plane, float far_plane);
		
		////////////////////////////// MAKE VIEWPORT ///////////////////////////////////////
		
		template <typename TDimensions>
		inline D3D11_VIEWPORT MakeViewport<TDimensions>(TDimensions width, TDimensions height){

			D3D11_VIEWPORT viewport;

			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.Width = static_cast<float>(width);
			viewport.Height = static_cast<float>(height);

			return viewport;

		}

	}

}