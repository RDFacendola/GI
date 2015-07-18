/// \file dx11.h
/// \brief Utility and wrapper methods for DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>

#include "eigen.h"
#include "object.h"
#include "resources.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using windows::COMPtr;

		/// \brief Wraps a constant buffer with a resource.
		/// \author Raffaele D. Facendola
		class ConstantBufferView{

		public:

			/// \brief Create an empty constant buffer view.
			ConstantBufferView();

			/// \brief Create a constant buffer view.
			/// \param resource Resource owning the constant buffer.
			/// \param constant_buffer Constant buffer.
			ConstantBufferView(const ObjectPtr<IResource>& resource, const COMPtr<ID3D11Buffer>& constant_buffer);

			/// \brief Get the wrapped constant buffer.
			/// \return Returns the resource's constant buffer.
			const COMPtr<ID3D11Buffer>& GetConstantBuffer() const;

		private:

			COMPtr<ID3D11Buffer> constant_buffer_;		///< \brief Constant buffer.

			ObjectPtr<IResource> resource_;				///< \brief Resource owning the constant buffer.
			
		};

		/// \brief Wraps a shader resource view with a resource.
		/// \author Raffaele D. Facendola.
		class ShaderResourceView{

		public:

			/// \brief Create an empty shader resource view.
			ShaderResourceView();

			/// \brief Create a shader resource view.
			/// \param resource Resource owning the shade resource view.
			/// \param shader_resource_view Shader resource view.
			ShaderResourceView(const ObjectPtr<IResource>& resource, const COMPtr<ID3D11ShaderResourceView>& shader_resource_view);

			/// \brief Get the wrapped shader resource view.
			/// \return Returns the resource's shader resource view.
			const COMPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const;

		private:

			COMPtr<ID3D11ShaderResourceView> shader_resource_view_;		///< \brief Shader resource view.

			ObjectPtr<IResource> resource_;								///< \brief Resource owning the shader resource view.
			
		};

		/// \brief Wraps an unordered access view with a resource.
		/// \author Raffaele D. Facendola.
		class UnorderedAccessView{

		public:

			/// \brief Create an empty unordered access view.
			UnorderedAccessView();

			/// \brief Create an unordered access view.
			/// \param resource Resource owning the unordered access view.
			/// \param unordered_access_view Unordered access view.
			UnorderedAccessView(const ObjectPtr<IResource>& resource, const COMPtr<ID3D11UnorderedAccessView>& unordered_access_view);

			/// \brief Get the wrapped unordered access view.
			/// \return Returns the resource's unordered access view.
			const COMPtr<ID3D11UnorderedAccessView>& GetUnorderedAccessView() const;

		private:

			COMPtr<ID3D11UnorderedAccessView> unordered_access_view_;	///< \brief Unordered access view.

			ObjectPtr<IResource> resource_;								///< \brief Resource owning the unordered access view.
			
		};

		/// \brief Wraps an samplers state with a resource.
		/// \author Raffaele D. Facendola.
		class SamplerStateView{

		public:

			/// \brief Create an empty sampler view.
			SamplerStateView();

			/// \brief Create a sampler view.
			/// \param resource Resource owning the sampler state.
			/// \param sampler_state Sample state.
			SamplerStateView(const ObjectPtr<IResource>& resource, const COMPtr<ID3D11SamplerState>& sampler_state);

			/// \brief Get the wrapped sampler state.
			/// \return Returns the sampler state.
			const COMPtr<ID3D11SamplerState>& GetSamplerState() const;

		private:

			COMPtr<ID3D11SamplerState> sampler_state_;			///< \brief Sampler state.

			ObjectPtr<IResource> resource_;						///< \brief Resource owning the sampler state.
			
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
		/// \param address_mode Texture mapping mode while sampling.
		/// \param anisotropy_level Maximum anisotropy level. Set to 0 to disable anisotropic filtering and enable trilinear filtering.
		/// \param border_color Border color to use when the address mode specified is D3D11_TEXTURE_ADDRESS_BORDER.
		/// \param sampler Pointer to the object that will hold the sampler if the method succeeds..
		HRESULT MakeSampler(ID3D11Device& device, D3D11_TEXTURE_ADDRESS_MODE address_mode, unsigned int anisotropy_level, Vector4f border_color, ID3D11SamplerState** sampler);

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
		
		////////////////////////////// CONSTANT BUFFER VIEW ///////////////////////////////////////

		inline ConstantBufferView::ConstantBufferView(){}

		inline ConstantBufferView::ConstantBufferView(const ObjectPtr<IResource>& resource, const COMPtr<ID3D11Buffer>& constant_buffer) :
		resource_(resource),
		constant_buffer_(constant_buffer){}

		inline const COMPtr<ID3D11Buffer>& ConstantBufferView::GetConstantBuffer() const{

			return constant_buffer_;

		}
		
		////////////////////////////// SHADER RESOURCE VIEW ///////////////////////////////////////

		inline ShaderResourceView::ShaderResourceView(){}

		inline ShaderResourceView::ShaderResourceView(const ObjectPtr<IResource>& resource, const COMPtr<ID3D11ShaderResourceView>& shader_resource_view) :
		resource_(resource),
		shader_resource_view_(shader_resource_view){}

		inline const COMPtr<ID3D11ShaderResourceView>& ShaderResourceView::GetShaderResourceView() const{

			return shader_resource_view_;

		}

		////////////////////////////// UNORDERED ACCESS VIEW ///////////////////////////////////////

		inline UnorderedAccessView::UnorderedAccessView(){}

		inline UnorderedAccessView::UnorderedAccessView(const ObjectPtr<IResource>& resource, const COMPtr<ID3D11UnorderedAccessView>& unordered_access_view) :
		resource_(resource),
		unordered_access_view_(unordered_access_view){}

		inline const COMPtr<ID3D11UnorderedAccessView>& UnorderedAccessView::GetUnorderedAccessView() const{

			return unordered_access_view_;

		}

		////////////////////////////// SAMPLER VIEW ///////////////////////////////////////

		inline SamplerStateView::SamplerStateView(){}

		inline SamplerStateView::SamplerStateView(const ObjectPtr<IResource>& resource, const COMPtr<ID3D11SamplerState>& sampler_state) :
		resource_(resource),
		sampler_state_(sampler_state){}

		inline const COMPtr<ID3D11SamplerState>& SamplerStateView::GetSamplerState() const{

			return sampler_state_;

		}

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