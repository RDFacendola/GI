/// \file dx11buffer.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#pragma once

#include "buffer.h"
#include "debug.h"

#include "dx11/dx11.h"
#include "dx11/dx11commitable.h"

#include "windows/win_os.h"
#include "instance_builder.h"


namespace gi_lib{

	namespace dx11{

		using windows::COMPtr;

		/// \brief Represents a generic buffer.
		/// \author Raffaele D. Facendola
		class DX11Buffer : public IHardwareBuffer{

		public:
			
			using IHardwareBuffer::Lock;

			/// \brief Create a new generic buffer.
			/// \param size Size of the buffer, in bytes.
			/// \param Pointer to the directx11 buffer. Compulsory.
			/// \param Pointer to the shader resource view used to bind the buffer to the pipeline. May be nullptr.
			DX11Buffer(size_t size, COMPtr<ID3D11Buffer> buffer, COMPtr<ID3D11ShaderResourceView> shader_resource_view);

			/// \brief No copy constructor.
			DX11Buffer(const DX11Buffer&) = delete;

			/// \brief Virtual destructor.
			virtual ~DX11Buffer();

			/// \brief No assignment operator.
			DX11Buffer& operator=(const DX11Buffer&) = delete;

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

			/// \brief Get the underlying buffer.
			/// \return Returns the underlying hardware buffer.
			ConstantBufferView GetBuffer();

			/// \brief Get the shader resource view used to bind this buffer to the pipeline.
			ShaderResourceView GetShaderResourceView();

		private:

			size_t size_;								///< \brief Size of the structure, in bytes.

			void* data_;								///< \brief Pointer to the raw buffer in system memory.

			mutable bool dirty_;						///< \brief Whether the buffer contains dirty data that needs to be committed.
			
			COMPtr<ID3D11Buffer> buffer_;				///< \brief Pointer to the buffer.

			COMPtr<ID3D11ShaderResourceView> srv_;		///< \brief Shader resource view. May be null.
			
		};

		/// \brief Represents a low-level buffer that behaves like a strongly-typed structure under DirectX 11.
		/// This class is used to handle a constant buffer.
		/// \author Raffaele D. Facendola
		class DX11StructuredBuffer : public IStructuredBuffer{

		public:

			using IHardwareBuffer::Lock;

			/// \brief Create a new constant buffer.
			/// \param size Size of the constant buffer.
			DX11StructuredBuffer(const IStructuredBuffer::FromSize& args);

			/// \brief Create a new constant buffer.
			/// \param size Size of the constant buffer.
			DX11StructuredBuffer(size_t size);

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

			ObjectPtr<ICommitter> GetCommitter();

			/// \brief Get the underlying constant buffer.
			ConstantBufferView GetConstantBuffer();

		private:

			ObjectPtr<DX11Buffer> buffer_;				///< \brief Pointer to the generic buffer.

		};

		/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements under DirectX 11.
		/// This array can be written by a CPU and read by a GPU.
		/// \author Raffaele D. Facendola
		class DX11StructuredArray : public IStructuredArray{

		public:

			using IHardwareBuffer::Lock;

			/// \brief Create a new structured array.
			/// \param element_count Number of the elemnts stored inside the array.
			/// \param element_size Size of each stored element.
			DX11StructuredArray(size_t element_count, size_t element_size);

			virtual void* Lock() override;

			virtual void Unlock() override;

			virtual size_t GetCount() const override;
			
			virtual size_t GetElementSize() const override;

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

			ObjectPtr<ICommitter> GetCommitter();

			/// \brief Get the shader resource view used to bind this buffer to the pipeline.
			ShaderResourceView GetShaderResourceView();

		private:

			size_t element_count_;										///< \brief Array elements count.

			size_t element_size_;										///< \brief Size of each element, in bytes.

			ObjectPtr<DX11Buffer> buffer_;								///< \brief Pointer to the generic buffer.

		};
		
		/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements under DirectX11.
		/// This array can be written by the GPU and read by the CPU.
		/// \author Raffaele D. Facendola
		class DX11ScratchStructuredArray : public IScratchStructuredArray {

		public:

			DX11ScratchStructuredArray(const FromElementSize& arguments);

			virtual size_t GetSize() const override;

			virtual size_t GetCount() const override;

			virtual size_t GetElementSize() const override;

			/// \brief Get the shader resource view used to bind this buffer to the pipeline (read only).
			ShaderResourceView GetShaderResourceView();

			/// \brief Get the unordered access view used to bind this texture to the pipeline (read/write).
			UnorderedAccessView GetUnorderedAccessView();

			/// \brief Refresh the content of the buffer.
			/// This method causes any unwritten GPU value to be written back to the system memory.
			void Refresh(ID3D11DeviceContext& context);

		protected:

			virtual void Read(size_t index, void* destination, size_t destination_size) const override;

		private:

			COMPtr<ID3D11UnorderedAccessView> unordered_access_view_;		///< \brief Pointer to the unordered access view of the structured array.

			COMPtr<ID3D11ShaderResourceView> shader_resource_view_;			///< \brief Pointer to the shader resource view of the array.

			COMPtr<ID3D11Buffer> buffer_;									///< \brief Buffer containing the computation result.

			COMPtr<ID3D11Buffer> readback_buffer_;							///< \brief Buffer used to read back the computation to the system memory.
						
			size_t element_size_;											///< \brief Size of each element in bytes.

			size_t element_count_;											///< \brief Number of elements inside the array.

			void* raw_buffer_;												///< \brief Raw representation of the buffer in system memory

		};

		/// \brief Downcasts an IStructuredBuffer to the proper concrete type.
		ObjectPtr<DX11StructuredBuffer> resource_cast(const ObjectPtr<IStructuredBuffer>& resource);

		/// \brief Downcasts an IStructuredArray to the proper concrete type.
		ObjectPtr<DX11StructuredArray> resource_cast(const ObjectPtr<IStructuredArray>& resource);

		/// \brief Downcasts an IScratchStructuredArray to the proper concrete type.
		ObjectPtr<DX11ScratchStructuredArray> resource_cast(const ObjectPtr<IScratchStructuredArray>& resource);
		
		//////////////////////////////// DIRECTX11 BUFFER //////////////////////////////// 

		inline void* DX11Buffer::Lock(){

			dirty_ = false;

			return data_;

		}

		inline void DX11Buffer::Unlock(){

			dirty_ = true;

		}

		inline size_t DX11Buffer::GetSize() const{

			return size_;

		}

		inline void* DX11Buffer::Lock(ID3D11DeviceContext& context){

			D3D11_MAPPED_SUBRESOURCE subresource;

			context.Map(buffer_.Get(),
						0,                                // Map everything
						D3D11_MAP_WRITE_DISCARD,		  // Discard the previous content.
						0,
						&subresource);

			return subresource.pData;

		}

		inline void DX11Buffer::Unlock(ID3D11DeviceContext& context){

			context.Unmap(buffer_.Get(),
						  0);

		}

		inline void DX11Buffer::Commit(ID3D11DeviceContext& context){

			if (dirty_){

				dirty_ = false;

				memcpy_s(Lock(context),
						 size_,
						 data_,
						 size_);

				Unlock(context);

			}

		}

		inline ConstantBufferView DX11Buffer::GetBuffer(){

			return ConstantBufferView(this,
									  buffer_);

		}

		inline ShaderResourceView DX11Buffer::GetShaderResourceView(){

			return ShaderResourceView(this, 
									  srv_);

		}

		//////////////////////////////// DIRECTX11 STRUCTURED BUFFER //////////////////////////////// 

		INSTANTIABLE(IStructuredBuffer, DX11StructuredBuffer, IStructuredBuffer::FromSize);

		inline void* DX11StructuredBuffer::Lock(){

			return buffer_->Lock();

		}

		inline void DX11StructuredBuffer::Unlock(){

			buffer_->Unlock();

		}

		inline size_t DX11StructuredBuffer::GetSize() const{

			return buffer_->GetSize();

		}

		inline void* DX11StructuredBuffer::Lock(ID3D11DeviceContext& context){

			return buffer_->Lock(context);

		}

		inline void DX11StructuredBuffer::Unlock(ID3D11DeviceContext& context){

			buffer_->Unlock(context);

		}

		inline void DX11StructuredBuffer::Commit(ID3D11DeviceContext& context){

			buffer_->Commit(context);

		}

		inline ObjectPtr<ICommitter> DX11StructuredBuffer::GetCommitter(){

			return new Committer<DX11StructuredBuffer>(ObjectPtr<DX11StructuredBuffer>(this));

		}

		inline ConstantBufferView DX11StructuredBuffer::GetConstantBuffer(){

			return buffer_->GetBuffer();

		}
		
		//////////////////////////////// DIRECTX11 STRUCTURED ARRAY //////////////////////////////// 
		
		inline void* DX11StructuredArray::Lock(){

			return buffer_->Lock();

		}

		inline void DX11StructuredArray::Unlock(){

			buffer_->Unlock();

		}

		inline size_t DX11StructuredArray::GetCount() const{

			return element_count_;

		}

		inline size_t DX11StructuredArray::GetElementSize() const{

			return element_size_;

		}

		inline size_t DX11StructuredArray::GetSize() const{

			return buffer_->GetSize();

		}

		inline void* DX11StructuredArray::Lock(ID3D11DeviceContext& context){

			return buffer_->Lock(context);

		}

		inline void DX11StructuredArray::Unlock(ID3D11DeviceContext& context){

			buffer_->Unlock(context);

		}

		inline void DX11StructuredArray::Commit(ID3D11DeviceContext& context){

			buffer_->Commit(context);

		}
	
		inline ObjectPtr<ICommitter> DX11StructuredArray::GetCommitter(){

			return new Committer<DX11StructuredArray>(ObjectPtr<DX11StructuredArray>(this));

		}

		inline ShaderResourceView DX11StructuredArray::GetShaderResourceView(){

			return buffer_->GetShaderResourceView();

		}

		//////////////////////////////// DIRECTX11 SCRATCH STRUCTURED ARRAY //////////////////////////
		
		INSTANTIABLE(IScratchStructuredArray, DX11ScratchStructuredArray, IScratchStructuredArray::FromElementSize);

		inline void DX11ScratchStructuredArray::Read(size_t index, void* destination, size_t destination_size) const {

			assert(index < element_count_);
			assert(destination_size == element_size_);
				
			char* buffer = reinterpret_cast<char*>(raw_buffer_);
			
			buffer += index * destination_size;

			memcpy_s(destination, 
					 destination_size, 
					 buffer, 
					 element_size_);

		}

		inline size_t DX11ScratchStructuredArray::GetSize() const{

			return (element_count_ * element_size_) * 3;			// GPU buffer, Staging buffer and system memory buffer

		}

		inline size_t DX11ScratchStructuredArray::GetCount() const {

			return element_count_;

		}

		inline size_t DX11ScratchStructuredArray::GetElementSize() const {

			return element_size_;

		}
		
		inline ShaderResourceView DX11ScratchStructuredArray::GetShaderResourceView() {

			return ShaderResourceView(this,
									  shader_resource_view_);

		}

		inline UnorderedAccessView DX11ScratchStructuredArray::GetUnorderedAccessView() {

			return UnorderedAccessView(this,
									   unordered_access_view_);

		}

		////////////////////////////////// RESOURCE CAST ///////////////////////////////////////

		inline ObjectPtr<DX11StructuredBuffer> resource_cast(const ObjectPtr<IStructuredBuffer>& resource){

			return ObjectPtr<DX11StructuredBuffer>(resource.Get());

		}

		inline ObjectPtr<DX11StructuredArray> resource_cast(const ObjectPtr<IStructuredArray>& resource){

			return ObjectPtr<DX11StructuredArray>(resource.Get());

		}

		inline ObjectPtr<DX11ScratchStructuredArray> resource_cast(const ObjectPtr<IScratchStructuredArray>& resource) {

			return ObjectPtr<DX11ScratchStructuredArray>(resource.Get());

		}

	}

}