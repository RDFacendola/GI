/// \file buffer.h
/// \brief This file contains the interfaces of buffer resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"

namespace gi_lib{

	/// \brief Represents a strongly typed hardware array.
	/// The array can be written by the CPU and read by the GPU.
	/// The size of the array does not change.
	/// \author Raffaele D. Facendola
	class IDynamicBuffer : public IResource{

	public:

		/// \brief Structure used to build an empty structured vector from a description.
		struct FromDescription{

			NO_CACHE;

			size_t element_count;			///< \brief Elements inside the vector.

			size_t element_size;			///< \brief Size of each element.

		};

		/// \brief Virtual destructor.
		virtual ~IDynamicBuffer(){}

		/// \brief Get the element count of this vector.
		/// \return Returns the element count of this vector.
		virtual size_t GetElementCount() const = 0;

		/// \brief Get the size of each element in bytes.
		/// \return Returns the size of each element.
		virtual size_t GetElementSize() const = 0;

		/// \brief Get the view to this resource.
		/// Use this view to bind the vector to the graphic pipeline.
		/// \return Returns a pointer to the resource view.
		virtual ObjectPtr<IResourceView> GetView() = 0;

		/// \brief Maps the buffer to the system memory, granting CPU access.
		/// \tparam TElement Type of the elements to map. Make sure to match the type this buffer was created with!.
		/// \return Returns a pointer to the first element of the array.
		/// \remarks Be sure to unlock the buffer afterwards: the context will hang trying to access a locked resource.
		template <typename TElement>
		TElement* Lock();

		/// \brief Commit a mapped buffer back to the video memory, revoking CPU access.
		/// \remarks Unlocking the buffer will invalidate the system-memory pointer returned by the Lock method.
		virtual void Unlock() = 0;

	private:

		/// \brief Maps the buffer to the system memory, granting CPU access.
		/// The elements already present inside the vector are discarded.
		/// \return Returns a pointer to the first element of the array.
		/// \remarks Be sure to unlock the buffer afterwards: the context will hang trying to access a locked resource.
		virtual void* LockDiscard() = 0;

	};

	///////////////////////////////////////// STRUCTURED BUFFER /////////////////////////////////////////

	template <typename TElement>
	inline TElement* IDynamicBuffer::Lock(){

		return static_cast<TElement*>(LockDiscard());

	}


}