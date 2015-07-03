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

		/// \brief Represents a generic buffer.
		/// \author Raffaele D. Facendola
		class DX11Buffer : IHardwareBuffer{

		public:

			

			virtual void* Lock() override;

			virtual void Unlock() override;

			virtual size_t GetSize() const override;
			
			/// \brief Lock the buffer, mapping it to the system memory.
			/// Use this method to map the hardware buffer directly.
			/// \return Returns a pointer to the mapped buffer.
			/// \remarks The buffer is intended for write-only purposes. Reading from it results in undefined behavior.
			void* Lock(ID3D11DeviceContext& context);

			/// \brief Unlock the buffer, committing it back to the video memory.
			/// Use this method to unmap the hardware buffer directly.
			/// \remarks This method will invalidate the pointer returned by "Lock". Do not use that pointer after this method.
			void Unlock(ID3D11DeviceContext& context);

			/// \brief Commit the constant buffer back to the GPU memory.
			/// If the buffer contains no new data, this method does nothing.
			void Commit(ID3D11DeviceContext& context);

		private:

			size_t size_;								///< \brief Size of the structure, in bytes.

			void* data_;								///< \brief Pointer to the raw buffer in system memory.

			mutable bool dirty_;						///< \brief Whether the buffer contains dirty data that needs to be committed.
			
			unique_com<ID3D11Buffer> buffer_;			///< \brief Pointer to the buffer.

		};

		/// \brief Represents a low-level buffer that behaves like a strongly-typed structure under DirectX 11.
		/// This class is used to handle a constant buffer.
		/// \author Raffaele D. Facendola
		class DX11StructuredBuffer : public IStructuredBuffer{

		public:

			virtual ~DX11StructuredBuffer();

			virtual void* Lock() override;

			virtual void Unlock() override;

			virtual size_t GetSize() const override;

			/// \brief Lock the buffer, mapping it to the system memory.
			/// Use this method to map the hardware buffer directly.
			/// \return Returns a pointer to the mapped buffer.
			/// \remarks The buffer is intended for write-only purposes. Reading from it results in undefined behavior.
			void* Lock(ID3D11DeviceContext& context);

			/// \brief Unlock the buffer, committing it back to the video memory.
			/// Use this method to unmap the hardware buffer directly.
			/// \remarks This method will invalidate the pointer returned by "Lock". Do not use that pointer after this method.
			void Unlock(ID3D11DeviceContext& context);

			/// \brief Commit the constant buffer back to the GPU memory.
			/// If the buffer contains no new data, this method does nothing.
			void Commit(ID3D11DeviceContext& context);

		private:

			ObjectPtr<DX11Buffer> buffer_;				///< \brief Pointer to the generic buffer.

		};

		/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements under DirectX 11.
		/// This array can be written by a CPU and read by a GPU.
		/// \author Raffaele D. Facendola
		class DX11StructuredArray : public IStructuredArray{

		public:

			virtual void* Lock() override;

			virtual void Unlock() override;

			virtual size_t GetCount() override;
			
			virtual size_t GetElementSize() override;

			virtual size_t GetSize() const override;

			/// \brief Lock the buffer, mapping it to the system memory.
			/// Use this method to map the hardware buffer directly.
			/// \return Returns a pointer to the mapped buffer.
			/// \remarks The buffer is intended for write-only purposes. Reading from it results in undefined behavior.
			void* Lock(ID3D11DeviceContext& context);

			/// \brief Unlock the buffer, committing it back to the video memory.
			/// Use this method to unmap the hardware buffer directly.
			/// \remarks This method will invalidate the pointer returned by "Lock". Do not use that pointer after this method.
			void Unlock(ID3D11DeviceContext& context);

			/// \brief Commit the constant buffer back to the GPU memory.
			/// If the buffer contains no new data, this method does nothing.
			void Commit(ID3D11DeviceContext& context);

		private:

			size_t element_count_;										///< \brief Array elements count.

			size_t element_size_;										///< \brief Size of each element, in bytes.

			ObjectPtr<DX11Buffer> buffer_;								///< \brief Pointer to the generic buffer.

			unique_com<ID3D11ShaderResourceView> shader_view_;			///< \brief Pointer to the shader resource view of the vector.

		};
	
	}

}

//////////////////////////////// DIRECTX11 BUFFER //////////////////////////////// 

void* gi_lib::dx11::DX11Buffer::Lock(){
	
	dirty_ = false;

	return data_;

}

void gi_lib::dx11::DX11Buffer::Unlock(){

	dirty_ = true;

}

size_t gi_lib::dx11::DX11Buffer::GetSize() const{

	return size_;

}

inline void* gi_lib::dx11::DX11Buffer::Lock(ID3D11DeviceContext& context){

	D3D11_MAPPED_SUBRESOURCE subresource;

	context.Map(buffer_.get(),
				0,                                // Map everything
				D3D11_MAP_WRITE_DISCARD,		  // Discard the previous content.
				0,
				&subresource);

	return subresource.pData;

}

inline void gi_lib::dx11::DX11Buffer::Unlock(ID3D11DeviceContext& context){

	context.Unmap(buffer_.get(),
				  0);

}

inline void gi_lib::dx11::DX11Buffer::Commit(ID3D11DeviceContext& context){

	if (dirty_){

		dirty_ = false;

		memcpy_s(Lock(context),
				 size_,
				 data_,
				 size_);

		Unlock(context);

	}

}

//////////////////////////////// DIRECTX11 STRUCTURED BUFFER //////////////////////////////// 

inline void* gi_lib::dx11::DX11StructuredBuffer::Lock(){

	return buffer_->Lock();

}

inline void gi_lib::dx11::DX11StructuredBuffer::Unlock(){

	buffer_->Unlock();

}

inline size_t gi_lib::dx11::DX11StructuredBuffer::GetSize() const{

	return buffer_->GetSize();

}

inline void* gi_lib::dx11::DX11StructuredBuffer::Lock(ID3D11DeviceContext& context){

	return buffer_->Lock(context);

}

inline void gi_lib::dx11::DX11StructuredBuffer::Unlock(ID3D11DeviceContext& context){

	buffer_->Unlock(context);

}

inline void gi_lib::dx11::DX11StructuredBuffer::Commit(ID3D11DeviceContext& context){

	buffer_->Commit(context);

}

//////////////////////////////// DIRECTX11 STRUCTURED ARRAY //////////////////////////////// 

inline void* gi_lib::dx11::DX11StructuredArray::Lock(){

	return buffer_->Lock();

}

inline void gi_lib::dx11::DX11StructuredArray::Unlock(){

	buffer_->Unlock();

}

inline size_t gi_lib::dx11::DX11StructuredArray::GetCount(){

	return element_count_;

}

inline size_t gi_lib::dx11::DX11StructuredArray::GetElementSize(){

	return element_size_;

}

inline size_t gi_lib::dx11::DX11StructuredArray::GetSize() const{

	return buffer_->GetSize();

}

inline void* gi_lib::dx11::DX11StructuredArray::Lock(ID3D11DeviceContext& context){

	return buffer_->Lock(context);

}

inline void gi_lib::dx11::DX11StructuredArray::Unlock(ID3D11DeviceContext& context){

	buffer_->Unlock(context);

}

inline void gi_lib::dx11::DX11StructuredArray::Commit(ID3D11DeviceContext& context){

	buffer_->Commit(context);

}