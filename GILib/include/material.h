/// \file material.h
/// \brief This file defines the interfaces needed to handle materials.
///
/// \author Raffaele D. Facendola

#pragma once

#include "gilib.h"
#include "fnv1.h"
#include "resources.h"

namespace gi_lib{

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
		virtual ~IMaterial(){}

		/// \brief Get a material variable by name.
		/// \param name The name of the variable.
		/// \return Returns a pointer to the variable matching the specified name if found, returns nullptr otherwise.
		virtual ObjectPtr<IVariable> GetVariable(const string& name) = 0;

		/// \brief Get a material resource by name.
		/// \param name The name of the resource.
		/// \return Returns a pointer to the resource matching the specified name if found, returns nullptr otherwise.
		virtual ObjectPtr<IResourceBLAH> GetResource(const string& name) = 0;

	};
	////////////////////////////// MATERIAL :: COMPILE FROM FILE ///////////////////////////////

	inline size_t IMaterial::CompileFromFile::GetCacheKey() const{

		return ::hash::fnv_1{}(to_string(file_name));

	}

}