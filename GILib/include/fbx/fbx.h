/// \file fbx.h
/// \brief Classes and methods to import and convert Autodesk FBX files.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <memory>

using ::std::wstring;
using ::std::unique_ptr;

namespace gi_lib{

	class TransformComponent;
	class Resources;

	/// \brief Fbx file importer
	/// \author Raffaele D. Facendola
	class FbxImporter{

	public:

		/// \brief Constructor.
		FbxImporter();

		/// \brief Destructor.
		~FbxImporter();

		/// \brief Import a FBX scene.

		/// The scene will load various scene nodes and the appropriate components.
		/// All the nodes will keep their structure but will be attached to the provided root.
		/// \param file_name Name of the FBX file to import.
		/// \param root The node where all the imported nodes will be attached hierarchically.
		/// \param resources Used to load resources during the import process.
		void ImportScene(const wstring& file_name, TransformComponent& root, Resources& resources);

	private:

		struct FbxSDK;

		unique_ptr<FbxSDK> fbx_sdk_;	///< \brief SDK object

	};

	
}