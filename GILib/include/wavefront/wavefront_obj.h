/// \file wavefront_obj.h
/// \brief Classes and methods to import and convert Wavefront OBJ files.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <memory>

#include "gimath.h"
#include "object.h"
#include "mesh.h"

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
		
		/// \brief Defines a collection of mtl materials.
		using MtlMaterialCollection = vector<const IMtlMaterial*>;

		/// \brief Interface used to import a concrete wavefront mtl material.
		class IMtlMaterialImporter {

		public:

			/// \brief Called when a new material has been imported.
			/// Use this interface to create the actual material component and setup the renderer components.
			/// \param base_directory Directory of the file being imported.
			/// \param material_collection The material collection being imported.
			/// \param mesh The mesh whose materials have been imported.
			virtual void OnImportMaterial(const wstring& base_directory, const MtlMaterialCollection& material_collection, MeshComponent& mesh) = 0;

		};
		/// \brief Class used to import a .obj scene.
		/// \author Raffaele D. Facendola
		class ObjImporter {

		public:
			
			/// \brief Create a new wavefront obj importer.
			/// \param resources Object used to import various resources.
			ObjImporter(Resources& resources);

			/// \brief Import an OBJ scene.
			/// The scene will load various scene nodes and the appropriate components.
			/// All the nodes will keep their structure but will be attached to the provided root.
			/// \param file_name Name of the OBJ file to import.
			/// \param root The node where all the imported nodes will be attached hierarchically.
			/// \param resources Used to load resources during the import process.
			bool ImportScene(const wstring& file_name, TransformComponent& root, IMtlMaterialImporter& material_importer) const;

			/// \brief Import a mesh from an OBJ file.
			/// \param file_name Name of the file to parse.
			/// \param mesh_name Name of the mesh to import.
			/// \param material_name If the method succeeds, this parameter contains the name of the material associated to the returned mesh. Optional.
			/// \return Returns a pointer to the imported mesh, if any. Returns nullptr otherwise.
			ObjectPtr<IStaticMesh> ImportMesh(const wstring& file_name, const string& mesh_name) const;

		private:

			/// \brief No assignment operator.
			ObjImporter& operator=(const ObjImporter&) = delete;

			Resources& resources_;			///< \brief Used to import the various resources.

		};

	}

}