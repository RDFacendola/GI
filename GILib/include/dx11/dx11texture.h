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
			COMPtr<ID3D11ShaderResourceView> GetShaderResourceView() const;

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

			DX11GPTexture2D(const FromDescription& args);

			virtual ObjectPtr<ITexture2D> GetTexture() override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual unsigned int GetMIPCount() const override;

			virtual size_t GetSize() const override;

			DXGI_FORMAT GetFormat() const;

			/// \brief Get the shader resource view used to bind this texture to the pipeline.
			COMPtr<ID3D11ShaderResourceView> GetShaderResourceView() const;

			/// \brief Get the unordered access view used to bind this texture to the pipeline.
			COMPtr<ID3D11UnorderedAccessView> GetUnorderedAccessView() const;

		private:

			COMPtr<ID3D11UnorderedAccessView> unordered_access_view_;		///< \brief Pointer to the unordered access view of the texture.

			ObjectPtr<DX11Texture2D> texture_;								///< \brief Underlying texture.

		};

		/// \brief Downcasts an ITexture2D to the proper concrete type.
		ObjectPtr<DX11Texture2D> resource_cast(const ObjectPtr<ITexture2D>& resource);

		/// \brief Downcasts an IGPTexture2D to the proper concrete type.
		ObjectPtr<DX11GPTexture2D> resource_cast(const ObjectPtr<IGPTexture2D>& resource);

		/////////////////////////////// DX11 TEXTURE2D ///////////////////////////////

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

		inline COMPtr<ID3D11ShaderResourceView> DX11Texture2D::GetShaderResourceView() const{

			return shader_resource_view_;

		}

		///////////////////////////// DX11 GP TEXTURE2D //////////////////////////////
		
		inline ObjectPtr<ITexture2D> DX11GPTexture2D::GetTexture()
		{
			
			return texture_;

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

		inline COMPtr<ID3D11ShaderResourceView> DX11GPTexture2D::GetShaderResourceView() const{

			return texture_->GetShaderResourceView();

		}

		inline COMPtr<ID3D11UnorderedAccessView> DX11GPTexture2D::GetUnorderedAccessView() const{

			return unordered_access_view_;

		}
		
		///////////////////////////// RESOURCE CAST ////////////////////////////////

		inline ObjectPtr<DX11Texture2D> resource_cast(const ObjectPtr<ITexture2D>& resource){

			return checked_cast<DX11Texture2D>(resource.Get());

		}

		inline ObjectPtr<DX11GPTexture2D> resource_cast(const ObjectPtr<IGPTexture2D>& resource){

			return checked_cast<DX11GPTexture2D>(resource.Get());

		}

	}

}

#endif