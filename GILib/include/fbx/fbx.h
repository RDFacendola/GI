/// \file fbx.h
/// \brief Classes and methods to import and convert Autodesk FBX files.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <memory>
#include <vector>

#include "gimath.h"

using ::std::wstring;
using ::std::string;
using ::std::shared_ptr;
using ::std::unique_ptr;
using ::std::vector;

namespace gi_lib{

	class Resources;
	class TransformComponent;
	class MeshComponent;

	namespace fbx{
		
		class IFbxMaterialImporter;
		class IFbxProperty;
		class IFbxMaterial;

		/// \brief Material collection for a single imported node.
		using FbxMaterialCollection = vector < unique_ptr< IFbxMaterial > >;

		/// \brief Interface used to import a concrete material
		class IFbxMaterialImporter{

		public:

			/// \brief Called when a material collection has been imported.
			/// Use this interface to create the actual material component and setup the renderer components.
			/// \param base_directory Directory of the file being imported.
			/// \param materials The collection of imported materials.
			/// \param mesh The mesh whose materials have been imported.
			virtual void OnImportMaterial(const wstring& base_directory, FbxMaterialCollection& materials, MeshComponent& mesh) = 0;

		};

		/// \brief A single material property.
		class IFbxProperty{

		public:

			/// \brief Get the property name.
			/// \return Returns the property name.
			virtual string GetName() const = 0;

			/// \brief Read a float value out of the property.
			/// If the property cannot be read as float the behavior is undefined.
			/// \return Returns the read value.
			virtual float ReadFloat() const = 0;

			/// \brief Read a 3-elements vector out of the property.
			/// If the property cannot be read as 3-elements vector the behavior is undefined.
			/// \return Returns the read value.
			virtual Vector3f ReadVector3() const = 0;

			/// \brief Enumerate the textures associated to this property.
			/// \return Returns the names of the textures associated to this property.
			virtual vector<string> EnumerateTextures() const = 0;

			/// \brief Get a subproperty by name.
			/// You may use the pipe character "|" to access subproperties directly (e.g.: "prop|subprop|subsubprop").
			/// \param property_name The subproperty name
			virtual unique_ptr<IFbxProperty> operator[](const string& subproperty_name) const = 0;

		};

		/// \brief A material description.
		class IFbxMaterial{

		public:

			/// \brief Get the material name.
			virtual string GetName() const = 0;

			/// \brief Get a property by name.
			/// You may use the pipe character "|" to access subproperties directly (e.g.: "prop|subprop|subsubprop").
			/// \param property_name The property name
			virtual unique_ptr<IFbxProperty> operator[](const string& property_name) const = 0;

		};

		/// \brief Fbx file importer
		/// \author Raffaele D. Facendola
		class FbxImporter{

		public:
			
			/// \brief Constructor.
			FbxImporter(IFbxMaterialImporter& material_importer, Resources& resources);

			/// \brief Destructor.
			~FbxImporter();

			/// \brief Import a FBX scene.

			/// The scene will load various scene nodes and the appropriate components.
			/// All the nodes will keep their structure but will be attached to the provided root.
			/// \param file_name Name of the FBX file to import.
			/// \param root The node where all the imported nodes will be attached hierarchically.
			/// \param resources Used to load resources during the import process.
			void ImportScene(const string& file_name, TransformComponent& root);

		private:

			struct FbxSDK;

			unique_ptr<FbxSDK> fbx_sdk_;				///< \brief SDK object

			IFbxMaterialImporter& material_importer_;		///< \brief Used to create the concrete material components and renderer components.

			Resources& resources_;						///< \brief Used to load the resources.

		};

	}

	
}