/// \file buffer.h
/// \brief This file contains the interfaces of buffer resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"

namespace gi_lib{
	
	/// \brief Represents a low-level buffer stored in GPU-memory.
	/// The buffer can be written by the CPU and read by the GPU.
	/// \author Raffaele D. Facendola.
	class IHardwareBuffer : public IResource{

	public:

		/// \brief Virtual destructor.
		virtual ~IHardwareBuffer(){}

		/// \brief Lock the buffer, mapping it to the system memory.
		/// \return Returns a pointer to the mapped buffer.
		/// \remarks The buffer is intended for write-only purposes. Reading from it results in undefined behavior.
		virtual void* Lock() = 0;

		/// \brief Unlock the buffer, committing it back to the video memory.
		/// \remarks This method will invalidate the pointer returned by "Lock". Do not use that pointer after this method.
		virtual void Unlock() = 0;
		
	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed structure.
	/// The buffer can be written by the CPU and read by the GPU.
	/// \author Raffaele D. Facendola
	class IStructuredBuffer : public IHardwareBuffer{

	public:

		/// \brief Structure used to create a structured buffer from an explicit size.
		struct FromSize{
			
			NO_CACHE;

			size_t size;		///< \brief Size of the constant buffer to create.

			bool clear;			///< \brief Whether the buffer should be cleared beforehand or not.

		};

		/// \brief Abstract destructor.
		virtual ~IStructuredBuffer() = 0 {}
				
	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements.
	/// This array can be written by a CPU and read by a GPU.
	/// \author Raffaele D. Facendola
	class IStructuredArray : public IHardwareBuffer{

	public:

		/// \brief Virtual destructor.
		virtual ~IStructuredArray(){}

		/// \brief Get the number of elements in the array.
		virtual size_t GetCount() const = 0;

		/// \brief Get the size of each element in bytes.
		virtual size_t GetElementSize() const = 0;

	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed structure.
	/// The buffer can be written by the CPU and read by the GPU.
	/// \tparam TType Concrete type of the buffer.
	/// \author Raffaele D. Facendola
	template <typename TType>
	class StructuredBuffer : public IResource{

	public:

		/// \brief Create a new structured buffer.
		/// \param array Raw array to decorate.
		StructuredBuffer(ObjectPtr<IStructuredBuffer> raw_array);

		/// \brief Virtual destructor.
		virtual ~StructuredBuffer(){}

		operator const IStructuredBuffer&() const;

		operator IStructuredBuffer&();

		/// \brief Access the structure, granting write permission.
		/// This method will LOCK the buffer. Remember to unlock it afterwards!
		/// \return Returns a reference to the actual structure.
		/// \remarks Reading from the returned reference results in undefined behavior.
		TType& operator*();

		const ObjectPtr<IStructuredBuffer>& GetBuffer() const;

		ObjectPtr<IStructuredBuffer>& GetBuffer();

		virtual void* Lock();

		virtual void Unlock();

		virtual size_t GetSize() const override;

	private:

		ObjectPtr<IStructuredBuffer> raw_buffer_;		///< \brief Raw buffer.

	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements.
	/// This array can be written by a CPU and read by a GPU.
	/// \tparam TElement Type of the elements stored within the array.
	/// \author Raffaele D. Facendola
	template <typename TElement>
	class StructuredArray : public IResource{

	public:

		/// \brief Create a new structured array.
		/// \param array Raw array to decorate.
		StructuredArray(ObjectPtr<IStructuredArray> raw_array);

		/// \brief Virtual destructor.
		virtual ~StructuredArray(){}

		/// \brief Access an element of the array, granting write permission.
		/// This method will LOCK the buffer. Remember to unlock it afterwards!
		/// \param index Index of the element to access.
		/// \return Returns a reference to the element at the specified location.
		/// \remarks Reading from the returned reference results in undefined behavior.
		TElement& operator[](size_t index);

		const ObjectPtr<IStructuredArray>& GetBuffer() const;

		ObjectPtr<IStructuredArray>& GetBuffer();

		virtual size_t GetCount() const;

		virtual size_t GetElementSize() const;

		virtual void* Lock();

		virtual void Unlock();

		virtual size_t GetSize() const override;

	private:

		ObjectPtr<IStructuredArray> raw_array_;		///< \brief Raw array.

	};

	/////////////////////////// STRUCTURED BUFFER //////////////////////////

	template <typename TType>
	StructuredBuffer<TType>::StructuredBuffer(ObjectPtr<IStructuredBuffer> raw_array) :
	raw_buffer_(raw_array){}

	template <typename TType>
	inline TType& StructuredBuffer<TType>::operator*(){
	
		return *static_cast<TType*>(Lock());

	}

	template <typename TType>
	inline void* StructuredBuffer<TType>::Lock(){

		return raw_buffer_->Lock();

	}

	template <typename TType>
	inline void StructuredBuffer<TType>::Unlock(){

		raw_buffer_->Unlock();

	}

	template <typename TType>
	inline size_t StructuredBuffer<TType>::GetSize() const{

		return raw_buffer_->GetSize();

	}

	template <typename TType>
	ObjectPtr<IStructuredBuffer>& StructuredBuffer<TType>::GetBuffer(){

		return raw_buffer_;

	}

	template <typename TType>
	const ObjectPtr<IStructuredBuffer>& StructuredBuffer<TType>::GetBuffer() const{

		return raw_buffer_;

	}

	/////////////////////////// STRUCTURED ARRAY //////////////////////////

	template <typename TElement>
	StructuredArray<TElement>::StructuredArray(ObjectPtr<IStructuredArray> raw_array) :
	raw_array_(raw_array){

		if (sizeof(TElement) != raw_array->GetElementSize()){

			THROW(L"The size of the elements does not match!");

		}

	}

	template <typename TElement>
	inline TElement& StructuredArray<TElement>::operator[](size_t index){

		return static_cast<TElement*>(Lock())[index];

	}

	template <typename TElement>
	inline size_t StructuredArray<TElement>::GetCount() const{

		return raw_array_->GetCount();

	}

	template <typename TElement>
	inline size_t StructuredArray<TElement>::GetElementSize() const{

		return raw_array_->GetElementSize();

	}

	template <typename TElement>
	inline void* StructuredArray<TElement>::Lock(){

		return raw_array_->Lock();

	}

	template <typename TElement>
	inline void StructuredArray<TElement>::Unlock(){

		raw_array_->Unlock();

	}

	template <typename TElement>
	inline size_t StructuredArray<TElement>::GetSize() const{

		return raw_array_->GetSize();

	}

	template <typename TType>
	ObjectPtr<IStructuredArray>& StructuredArray<TType>::GetBuffer(){

		return *raw_array_;

	}

	template <typename TType>
	const ObjectPtr<IStructuredArray>& StructuredArray<TType>::GetBuffer() const{

		return *raw_array_;

	}

}

