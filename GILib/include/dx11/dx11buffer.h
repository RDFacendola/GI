/// \file dx11buffer.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#pragma once

#include "buffer.h"

#include "dx11/dx11.h"
#include "dx11/dx11resources.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using windows::unique_com;

		/// \brief DirectX11 strongly-typed vector.
		/// \author Raffaele D. Facendola
		class DX11DynamicBuffer : public IDynamicBuffer{

		public:

			/// \brief Create a new DirectX11 material from shader code.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to load the material.
			DX11DynamicBuffer(const FromDescription& args);

			/// \brief Default destructor.
			~DX11DynamicBuffer();

			virtual size_t GetSize() const override;

			virtual size_t GetElementCount() const override;

			virtual size_t GetElementSize() const override;

			virtual ObjectPtr<IResourceView> GetView() override;

			virtual void Unlock() override;

			/// \brief Map the vector to the system memory.
			/// \param context Context used to map the buffer.
			/// \return Returns a pointer to the first element of the vector.
			template <typename TElement>
			TElement* Map(ID3D11DeviceContext& context);

			/// \brief Unmap the vector from the system memory and commits it back to the video memory.
			/// \param context Context used to unmap the buffer.
			void Unmap(ID3D11DeviceContext& context);

			/// \brief Write the uncommitted state to the structured buffer.
			/// \param context Context used to commit the buffer.
			void Commit(ID3D11DeviceContext& context);

		private:

			virtual void* LockDiscard() override;

			size_t element_count_;										///< \brief Number of elements inside the vector.

			size_t element_size_;										///< \brief Size of each element.

			void* data_;												///< \brief Pointer to the raw buffer in system memory.

			mutable bool dirty_;										///< \brief Whether the buffer contains dirty data that needs to be committed.

			unique_com<ID3D11Buffer> buffer_;							///< \brief Pointer to the structured buffer.

			unique_com<ID3D11ShaderResourceView> shader_view_;			///< \brief Pointer to the shader resource view of the vector.

		};
		
	}

}


////////////////////////////// DX11 STRUCTURED VECTOR //////////////////////////////////

inline size_t gi_lib::dx11::DX11DynamicBuffer::GetSize() const{

	return element_count_ * element_size_;

}

inline size_t gi_lib::dx11::DX11DynamicBuffer::GetElementCount() const{

	return element_count_;

}

inline size_t gi_lib::dx11::DX11DynamicBuffer::GetElementSize() const{

	return element_size_;

}

inline gi_lib::ObjectPtr<gi_lib::IResourceView> gi_lib::dx11::DX11DynamicBuffer::GetView(){

	return new DX11ResourceViewTemplate<const DX11DynamicBuffer>(this,
																	shader_view_.get(),
																	nullptr);
	
}

template <typename TElement>
inline TElement* gi_lib::dx11::DX11DynamicBuffer::Map(ID3D11DeviceContext& context){

	dirty_ = true;

	D3D11_MAPPED_SUBRESOURCE subresource;

	context.Map(buffer_.get(),
				0,								// Map everything
				D3D11_MAP_WRITE_DISCARD,		// Discard the previous content.
				0,
				&subresource);

	return static_cast<TElement*>(subresource.pData);

}
