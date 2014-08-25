/// \file resource.h
/// \brief Generic graphical resource interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

namespace gi_lib{
	
	/// \brief Describe the priority of the resources.
	enum class ResourcePriority{

		MINIMUM,		///< Lowest priority. These resources will be the first one to be freed when the system will run out of memory.
		LOW,			///< Low priority.
		NORMAL,			///< Normal priority. Default value.
		HIGH,			///< High priority.
		CRITICAL		///< Highest priority. These resources will be kept in memory at any cost.

	};

	/// \brief Base interface for graphical resources.
	/// \author Raffaele D. Facendola.
	class Resource{

	public:

		virtual ~Resource(){}

		/// \brief Get the memory footprint of this resource.
		/// \return Returns the size of the resource, in bytes.
		virtual size_t GetSize() const = 0;

		/// \brief Get the priority of the resource.
		/// \return Returns the resource priority.
		virtual ResourcePriority GetPriority() const = 0;

		/// \brief Set the priority of the resource.
		/// \param priority The new priority.
		virtual void SetPriority(ResourcePriority priority) = 0;

	};

}