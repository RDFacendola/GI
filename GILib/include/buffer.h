/// \file buffer.h
/// \brief This file contains the interfaces of buffer resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"

namespace gi_lib {

	/// \brief Represents a low-level buffer stored in GPU-memory.
	/// The buffer can be written by the CPU and read by the GPU.
	/// \author Raffaele D. Facendola.
	class IHardwareBuffer : public IResource {

	public:

		/// \brief Virtual destructor.
		virtual ~IHardwareBuffer() {}

		/// \brief Lock the buffer, mapping it to the system memory.
		/// \return Returns a pointer to the mapped buffer.
		/// \remarks The buffer is intended for write-only purposes. Reading from it results in undefined behavior.
		virtual void* Lock() = 0;

		/// \brief Unlock the buffer, committing it back to the video memory.
		/// \remarks This method will invalidate the pointer returned by "Lock". Do not use that pointer after this method.
		virtual void Unlock() = 0;

		/// \brief Lock the buffer, mapping it to the system memory.
		/// \return Returns a strongly typed pointer to the mapped buffer.
		/// \remarks The buffer is intended for write-only purposes. Reading from it results in undefined behavior.
		template <typename TLock>
		TLock* Lock();

	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed structure.
	/// The buffer can be written by the CPU and read by the GPU.
	/// \author Raffaele D. Facendola
	class IStructuredBuffer : public IHardwareBuffer {

	public:

		/// \brief Structure used to create a structured buffer from an explicit size.
		struct FromSize {

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
	class IStructuredArray : public IHardwareBuffer {

	public:

		/// \brief Virtual destructor.
		virtual ~IStructuredArray() {}

		/// \brief Get the number of elements in the array.
		virtual size_t GetCount() const = 0;

		/// \brief Get the size of each element in bytes.
		virtual size_t GetElementSize() const = 0;

	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements and can be used for general-purposes computations.
	/// This array can be written and read by a GPU only.
	/// \author Raffaele D. Facendola
	class IGPStructuredArray : public IResource {

	public:

		/// \brief Structure used to create a scratch structured array from an explicit element size.
		struct FromElementSize {

			NO_CACHE;

			size_t element_count;			///< \brief Number of element inside the buffer.

			size_t element_size;			///< \brief Size of each element in bytes.

		};

		/// \brief Virtual destructor.
		virtual ~IGPStructuredArray() {}

		/// \brief Get the number of elements in the array.
		virtual size_t GetCount() const = 0;

		/// \brief Get the size of each element in bytes.
		virtual size_t GetElementSize() const = 0;

	};

	/// \brief Represents a low-level buffer that behaves like a strongly-typed array of elements.
	/// This array can be written by the GPU and read by the CPU.
	/// \author Raffaele D. Facendola
	class IScratchStructuredArray : public IResource {

	public:

		/// \brief Structure used to create a scratch structured array from an explicit element size.
		struct FromElementSize {

			NO_CACHE;

			size_t element_count;			///< \brief Number of element inside the buffer.
			
			size_t element_size;			///< \brief Size of each element in bytes.

		};

		/// \brief Abstract destructor.
		virtual ~IScratchStructuredArray() = 0 {}
		
		/// \brief Read an element from the structured array.
		/// \tparam TType Type of the element to read.
		/// \param index Index of the element to read.
		/// \return Return the value of the element at the specified location.
		template <typename TType>
		TType ElementAt(size_t index) const;

		/// \brief Get the number of elements in the array.
		virtual size_t GetCount() const = 0;

		/// \brief Get the size of each element in bytes.
		virtual size_t GetElementSize() const = 0;

	protected:

		/// \brief Read an element from the structured array.
		/// \param index Index of the element to read.
		/// \param destination Pointer to the element that will contain the read result.
		/// \param destination_size Size of the destination buffer.
		virtual void Read(size_t index, void* destination, size_t destination_size) const = 0;

	};

	/////////////////////////// IHARDWARE BUFFER ///////////////////////////
	
	template <typename TLock>
	TLock* IHardwareBuffer::Lock() {

		return reinterpret_cast<TLock*>(Lock());

	}
	
	/////////////////////////// I SCRATCH STRUCTURED ARRAY /////////////////////////

	template <typename TType>
	TType IScratchStructuredArray::ElementAt(size_t index) const {

		TType destination;

		Read(index, static_cast<void*>(&destination), sizeof(TType));

		return destination;

	}

}

