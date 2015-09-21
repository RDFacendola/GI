/// \file dx11texture.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include "texture.h"
#include "debug.h"

#include "dx11/dx11.h"

#include "windows/win_os.h"
#include "instance_builder.h"

namespace gi_lib{

	namespace dx11{

		using windows::COMPtr;

		/// \brief DirectX11 2D texture.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D : public ITexture2D{

		public:

			/// \brief Create a texture from a shader resource view.
			DX11Texture2D(const COMPtr<ID3D11ShaderResourceView>& shader_resource_view);

			/// \brief Create a new texture from DDS file.
			/// \param device The device used to create the texture.
			/// \param bundle The bundle used to load the texture.
			DX11Texture2D(const FromFile& args);

			virtual ~DX11Texture2D(){}

			virtual size_t GetSize() const override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual unsigned int GetMIPCount() const override;

			DXGI_FORMAT GetFormat() const;

			/// \brief Get the shader resource view used to bind this texture to the pipeline.
			ShaderResourceView GetShaderResourceView();

			/// \brief Get a pointer to the hardware texture
			COMPtr<ID3D11Texture2D> GetTexture();

		private:

			void UpdateDescription(const D3D11_TEXTURE2D_DESC& description);

			COMPtr<ID3D11ShaderResourceView> shader_resource_view_;		///< \brief Pointer to the shader resource view of the texture.

			unsigned int width_;										///< \brief Width of the texture, in pixels.

			unsigned int height_;										///< \brief Height of the texture, in pixels.
			
			unsigned int bits_per_pixel_;								///< \brief Bits per pixel.

			unsigned int mip_levels_;									///< \brief MIP levels.

			DXGI_FORMAT format_;										///< \brief Surface format.

		};

		/// \brief DirectX11 general-purpose 2D texture.
		/// This texture can be used as a regular texture but can also be bound as unordered access resource to a compute or pixel shader.
		class DX11GPTexture2D : public IGPTexture2D{

		public:

			DX11GPTexture2D(unsigned int width, unsigned int height, DXGI_FORMAT format, unsigned int mips = 1);

			virtual ObjectPtr<ITexture2D> GetTexture() override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual unsigned int GetMIPCount() const override;

			virtual size_t GetSize() const override;

			DXGI_FORMAT GetFormat() const;

			/// \brief Get the shader resource view used to bind this texture to the pipeline.
			ShaderResourceView GetShaderResourceView();

			/// \brief Get the unordered access view used to bind this texture to the pipeline.
			UnorderedAccessView GetUnorderedAccessView();

		private:

			COMPtr<ID3D11UnorderedAccessView> unordered_access_view_;		///< \brief Pointer to the unordered access view of the texture.

			ObjectPtr<DX11Texture2D> texture_;								///< \brief Underlying texture.

		};

		/// \brief Downcasts an ITexture2D to the proper concrete type.
		ObjectPtr<DX11Texture2D> resource_cast(const ObjectPtr<ITexture2D>& resource);

		/// \brief Downcasts an IGPTexture2D to the proper concrete type.
		ObjectPtr<DX11GPTexture2D> resource_cast(const ObjectPtr<IGPTexture2D>& resource);

		/////////////////////////////// DX11 TEXTURE2D ///////////////////////////////
		
		INSTANTIABLE(ITexture2D, DX11Texture2D, ITexture2D::FromFile);

		inline unsigned int DX11Texture2D::GetWidth() const{

			return width_;

		}

		inline unsigned int DX11Texture2D::GetHeight()const {

			return height_;

		}

		inline unsigned int DX11Texture2D::GetMIPCount() const{

			return mip_levels_;

		}

		inline DXGI_FORMAT DX11Texture2D::GetFormat() const{

			return format_;

		}

		inline ShaderResourceView DX11Texture2D::GetShaderResourceView(){

			return ShaderResourceView(this,
									  shader_resource_view_);

		}

		inline COMPtr<ID3D11Texture2D> DX11Texture2D::GetTexture(){

			ID3D11Resource* resource;

			shader_resource_view_->GetResource(&resource);

			ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(resource);

			return windows::COMMove(&texture);

		}

		///////////////////////////// DX11 GP TEXTURE2D //////////////////////////////
		
		//INSTANTIABLE(IGPTexture2D, DX11GPTexture2D, IGPTexture2D::FromDescription);

		inline ObjectPtr<ITexture2D> DX11GPTexture2D::GetTexture()
		{
			
			return ObjectPtr<ITexture2D>(texture_);

		}

		inline unsigned int DX11GPTexture2D::GetWidth() const
		{
			
			return texture_->GetWidth();

		}

		inline unsigned int DX11GPTexture2D::GetHeight() const
		{
			
			return texture_->GetHeight();

		}

		inline unsigned int DX11GPTexture2D::GetMIPCount() const
		{
			
			return texture_->GetMIPCount();

		}

		inline size_t DX11GPTexture2D::GetSize() const
		{
			
			return texture_->GetSize();

		}

		inline ShaderResourceView DX11GPTexture2D::GetShaderResourceView(){

			return texture_->GetShaderResourceView();

		}

		inline UnorderedAccessView DX11GPTexture2D::GetUnorderedAccessView(){

			return UnorderedAccessView(this,
									   unordered_access_view_);

		}
		
		///////////////////////////// RESOURCE CAST ////////////////////////////////

		inline ObjectPtr<DX11Texture2D> resource_cast(const ObjectPtr<ITexture2D>& resource){

			return ObjectPtr<DX11Texture2D>(resource.Get());

		}

		inline ObjectPtr<DX11GPTexture2D> resource_cast(const ObjectPtr<IGPTexture2D>& resource){

			return ObjectPtr<DX11GPTexture2D>(resource.Get());

		}

	}

}

#endif