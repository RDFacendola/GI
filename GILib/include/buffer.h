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

		/// \brief Abstract destructor.
		virtual ~IStructuredBuffer() = 0 {}
				
	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed structure.
	/// The buffer can be written by the CPU and read by the GPU.
	/// \tparam TType Concrete type of the buffer.
	/// \author Raffaele D. Facendola
	template <typename TType>
	class StructuredBuffer : public IStructuredBuffer{

	public:

		/// \brief Create a new structured buffer.
		/// \param array Raw array to decorate.
		StructuredBuffer(ObjectPtr<IStructuredArray> raw_array);

		/// \brief Abstract destructor.
		virtual ~StructuredBuffer() = 0 {}

		/// \brief Access the structure, granting write permission.
		/// This method will LOCK the buffer. Remember to unlock it afterwards!
		/// \return Returns a reference to the actual structure.
		/// \remarks Reading from the returned reference results in undefined behavior.
		TType& operator*();

		virtual void* Lock() override;

		virtual void Unlock() override;

		virtual size_t GetSize() const override;

	private:

		ObjectPtr<IStructuredArray> raw_array_;		///< \brief Raw array.

	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements.
	/// This array can be written by a CPU and read by a GPU.
	/// \author Raffaele D. Facendola
	class IStructuredArray : public IHardwareBuffer{

	public:

		/// \brief Virtual destructor.
		virtual ~IStructuredArray(){}

		/// \brief Get the number of elements in the array.
		virtual size_t GetCount() = 0;

	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements.
	/// This array can be written by a CPU and read by a GPU.
	/// \tparam TElement Type of the elements stored within the array.
	/// \author Raffaele D. Facendola
	template <typename TElement>
	class StructuredArray : public IStructuredArray{

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

		virtual size_t GetCount() override;

		virtual void* Lock() override;

		virtual void Unlock() override;

		virtual size_t GetSize() const override;

	private:

		ObjectPtr<IStructuredArray> raw_array_;		///< \brief Raw array.

	};

}

/////////////////////////// STRUCTURED BUFFER //////////////////////////

template <typename TType>
gi_lib::StructuredBuffer::StructuredBuffer(ObjectPtr<IStructuredArray> raw_array) :
raw_array_(raw_array){}

template <typename TType>
inline TType& gi_lib::StructuredBuffer::operator*(){
	
	return *static_cast<TType*>(Lock());

}

template <typename TType>
inline void* gi_lib::StructuredBuffer::Lock(){

	return raw_array_->Lock();

}

template <typename TType>
inline void gi_lib::StructuredBuffer::Unlock(){

	raw_array_->Unlock();

}

template <typename TType>
inline size_t gi_lib::StructuredBuffer::GetSize() const{

	return raw_array_->GetSize();

}

/////////////////////////// STRUCTURED ARRAY //////////////////////////

template <typename TElement*>
gi_lib::StructuredArray::StructuredArray(ObjectPtr<IStructuredArray> raw_array) :
raw_array_(raw_array){}

template <typename TElement*>
inline TElement& gi_lib::StructuredArray::operator[](size_t index){

	return static_cast<TElement*>(Lock())[index];

}

template <typename TElement*>
inline size_t gi_lib::StructuredArray::GetCount(){

	return raw_array_->GetCount();

}

template <typename TElement*>
inline void* gi_lib::StructuredArray::Lock(){

	return raw_array_->Lock();

}

template <typename TElement*>
inline void gi_lib::StructuredArray::Unlock(){

	raw_array_->Unlock();

}

template <typename TElement*>
inline size_t gi_lib::StructuredArray::GetSize() const{

	return raw_array_->GetSize();

}
