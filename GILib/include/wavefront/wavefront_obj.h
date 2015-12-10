/// \file wavefront_obj.h
/// \brief Classes and methods to import and convert Wavefront OBJ files.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <memory>

#include "gimath.h"

using ::std::wstring;
using ::std::string;
using ::std::unique_ptr;

namespace gi_lib {

	class Resources;
	class TransformComponent;
	class MeshComponent;

	namespace wavefront {

		/// \brief A single wavefront material property.
		class IMtlProperty {

		public:

			/// \brief Get the property name.
			/// \return Returns the property name.
			virtual string GetName() const = 0;

			/// \brief Reads the property as a float.
			/// \param out Contains the float property if the method succeeds.
			/// \return Returns true if the method succeeds, returns false otherwise.
			/// \remarks The method fails if the property cannot be interpreted as a float.
			virtual bool Read(float& out) const = 0;

			/// \brief Reads the property as a vector.
			/// \param out Contains the float property if the method succeeds.
			/// \return Returns true if the method succeeds, returns false otherwise.
			/// \remarks The method fails if the property cannot be interpreted as a float.
			virtual bool Read(Vector3f& out) const = 0;
		
			/// \brief Reads the property as a string.
			/// \param out Contains the float property if the method succeeds.
			/// \return Returns true if the method succeeds, returns false otherwise.
			/// \remarks The method fails if the property cannot be interpreted as a float.
			virtual bool Read(string& out) const = 0;

		};

		/// \brief Base interface for the wavefront mtl material definitions.
		class IMtlMaterial {

		public:
			
			/// \brief Get the material name.
			virtual string GetName() const = 0;

			/// \brief Get a property by name.
			/// \param property_name The property name
			virtual unique_ptr<IMtlProperty> operator[](const string& property_name) const = 0;

		};

		/// \brief Interface used to import a concrete wavefront mtl material.
		class IMtlMaterialImporter {

		public:

			/// \brief Called when a new material has been imported.
			/// Use this interface to create the actual material component and setup the renderer components.
			/// \param base_directory Directory of the file being imported.
			/// \param material The material being imported
			/// \param mesh The mesh whose materials have been imported.
			virtual void OnImportMaterial(const wstring& base_directory, const IMtlMaterial& material, MeshComponent& mesh) = 0;

		};

		/// \brief Class used to import a .obj scene.
		/// \author Raffaele D. Facendola
		class ObjImporter {

		public:

			ObjImporter(IMtlMaterialImporter& importer, Resources& resources);

			/// \brief Import an OBJ scene.

			/// The scene will load various scene nodes and the appropriate components.
			/// All the nodes will keep their structure but will be attached to the provided root.
			/// \param file_name Name of the OBJ file to import.
			/// \param root The node where all the imported nodes will be attached hierarchically.
			/// \param resources Used to load resources during the import process.
			bool ImportScene(const wstring& file_name, TransformComponent& root) const;

		private:

			/// \brief No assignment operator.
			ObjImporter& operator=(const ObjImporter&) = delete;

			IMtlMaterialImporter& material_importer_;	///< \brief Object used to import the materials.

			Resources& resources_;						///< \brief Used to load the resources.

		};

	}

}