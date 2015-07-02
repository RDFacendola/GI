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

		/// \brief Abstract destructor.
		virtual ~StructuredBuffer() = 0 {}

		/// \brief Access the structure, granting write permission.
		/// This method will LOCK the buffer. Remember to unlock it afterwards!
		/// \return Returns a reference to the actual structure.
		/// \remarks Reading from the returned reference results in undefined behavior.
		TType& operator*();
		
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

		/// \brief Abstract destructor.
		virtual ~StructuredArray() = 0 {}

		/// \brief Access an element of the array, granting write permission.
		/// This method will LOCK the buffer. Remember to unlock it afterwards!
		/// \param index Index of the element to access.
		/// \return Returns a reference to the element at the specified location.
		/// \remarks Reading from the returned reference results in undefined behavior.
		TElement& operator[](size_t index);

	};

}

/////////////////////////// STRUCTURED BUFFER //////////////////////////

template <typename TType>
TType& gi_lib::StructuredBuffer::operator *(){

	return *static_cast<TType*>(Lock());
	
}

/////////////////////////// STRUCTURED ARRAY //////////////////////////

template <typename TElement*>
TType& gi_lib::StructuredArray::operator[](size_t index){

	return static_cast<TElement*>(Lock())[index];

}