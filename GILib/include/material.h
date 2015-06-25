/// \file material.h
/// \brief This file defines the interfaces needed to handle materials.
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"
#include "object.h"

#include "gilib.h"
#include "fnv1.h"

namespace gi_lib{

	class IMaterialParameter;
	class IMaterialResource;

	/// \brief Base interface for materials.
	/// \author Raffaele D. Facendola
	class IMaterial : public IResource{

	public:

		/// \brief Structure used to compile a material from a file.
		struct CompileFromFile{

			USE_CACHE;

			wstring file_name;			///< \brief Name of the file containing the material code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Structure used to instantiate an existing material.
		struct Instantiate{

			NO_CACHE;

			ObjectPtr<IMaterial> base;	///< \brief Material to instantiate.

		};

		/// \brief Virtual destructor.
		virtual ~IMaterial() = 0 {};

		/// \brief Get a pointer to a material parameter.
		/// \param name Name of the parameter to get.
		/// \return Returns a pointer to the material parameter if any, returns nullptr otherwise.
		virtual ObjectPtr<IMaterialParameter> GetParameter(const string& name) = 0;

		/// \brief Get a pointer to a material resource.
		/// \param name Name of the resource to get.
		/// \return Returns a pointer to the material resource if any, returns nullptr otherwise.
		/// \remarks The resource is accessed in read-only mode.
		virtual ObjectPtr<IMaterialResource> GetResource(const string& name) = 0;
		
	};

	/// \brief Base interface for material parameters.
	/// The class is used to change the value of a material parameter.
	/// \author Raffaele D. Facendola
	class IMaterialParameter : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IMaterialParameter() = 0 {};

		/// \brief Set a new value for the material parameter.
		/// \tparam TParameter Type of the parameter to write.
		/// \param value Value to write.
		template <typename TParameter>
		void Set(const TParameter& value);

		/// \brief Set a new value for the material parameter.
		/// \param value_ptr Pointer to the value to write.
		/// \param size Size of the buffer containing the value to write.
		virtual void Set(const void* value_ptr, size_t size) = 0;
		
	};

	/// \brief Base interface for material resources.
	/// The class is used to bind a material resource.
	/// \author Raffaele D. Facendola
	class IMaterialResource : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IMaterialResource() = 0 {};

		/// \brief Bind a new resource to the material.
		/// \param resource Read-only view of the resource to bind to the material.
		virtual void Set(ObjectPtr<IResourceView> resource) = 0;

	};

	/////////////////////////////////// IMATERIAL PARAMETER ///////////////////////////////////

	template <typename TParameter>
	inline void IMaterialParameter::Set(const TParameter& value){

		Set(std::addressof(value),
			sizeof(TParameter));

	}

	////////////////////////////// MATERIAL :: COMPILE FROM FILE ///////////////////////////////

	inline size_t IMaterial::CompileFromFile::GetCacheKey() const{

		return ::hash::fnv_1{}(to_string(file_name));

	}



}