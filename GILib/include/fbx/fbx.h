/// \file fbx.h
/// \brief Classes and methods to import and convert Autodesk FBX files.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <memory>
#include <vector>

#include "..\gimath.h"

using ::std::wstring;
using ::std::shared_ptr;
using ::std::unique_ptr;
using ::std::vector;

namespace gi_lib{

	class Resources;
	class TransformComponent;
	class MeshComponent;

	namespace fbx{
		
		class IMaterialImporter;
		class IProperty;
		class IMaterial;

		/// \brief Material collection for a single imported node.
		using MaterialCollection = vector < unique_ptr< IMaterial > >;

		/// \brief Interface used to import a concrete material
		class IMaterialImporter{

		public:

			/// \brief Called when a material collection has been imported.
			/// Use this interface to create the actual material component and setup the renderer components.
			/// \param materials The collection of imported materials.
			/// \param mesh The mesh whose materials have been imported.
			virtual void OnImportMaterial(MaterialCollection& materials, MeshComponent& mesh) = 0;

		};

		/// \brief A single material property.
		class IProperty{

		public:

			/// \brief Get the property name.
			/// \return Returns the property name.
			virtual wstring GetName() const = 0;

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
			virtual vector<wstring> EnumerateTextures() const = 0;

		};

		/// \brief A material description.
		class IMaterial{

		public:

			/// \brief Get the material name.
			virtual wstring GetName() const = 0;

			/// \brief Get a property by name.
			/// \param property_name The property name
			virtual unique_ptr<IProperty> operator[](const wstring& property_name) const = 0;

		};

		/// \brief Fbx file importer
		/// \author Raffaele D. Facendola
		class FbxImporter{

		public:
			
			/// \brief Constructor.
			FbxImporter(IMaterialImporter& material_importer, Resources& resources);

			/// \brief Destructor.
			~FbxImporter();

			/// \brief Import a FBX scene.

			/// The scene will load various scene nodes and the appropriate components.
			/// All the nodes will keep their structure but will be attached to the provided root.
			/// \param file_name Name of the FBX file to import.
			/// \param root The node where all the imported nodes will be attached hierarchically.
			/// \param resources Used to load resources during the import process.
			void ImportScene(const wstring& file_name, TransformComponent& root);

		private:

			struct FbxSDK;

			unique_ptr<FbxSDK> fbx_sdk_;				///< \brief SDK object

			IMaterialImporter& material_importer_;		///< \brief Used to create the concrete material components and renderer components.

			Resources& resources_;						///< \brief Used to load the resources.

		};

	}

	
}