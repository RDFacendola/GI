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

	class SceneNode;
	class Manager;

	/// \brief FBX file importer
	/// \author Raffaele D. Facendola
	class FBXImporter{

	public:

		/// \brief Get the singleton instance of the importer.
		/// \return Returns the singleton instance.
		static FBXImporter & GetInstance();

		/// \brief Destructor
		~FBXImporter();

		/// \brief Import a FBX scene.

		/// The scene will load various scene nodes and the appropriate components.
		/// All the nodes will keep their structure but will be attached to the provided root.
		/// \param file_name Name of the FBX file to import.
		/// \param scene_root The node where all the imported scene will be attached.
		/// \param resources Used to load resources during the import process.
		void ImportScene(const wstring & file_name, SceneNode & scene_root, Manager & resources);

	private:

		FBXImporter();

		struct FbxSDK;

		FbxSDK * fbx_sdk_;

	};

	//

	inline FBXImporter & FBXImporter::GetInstance(){

		static FBXImporter instance;

		return instance;

	}

}