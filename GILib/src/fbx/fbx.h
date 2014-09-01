/// \file fbx.h
/// \brief Classes and methods to import and convert Autodesk FBX files.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>

#include <fbxsdk.h>

#include "..\..\include\resources.h"

using ::std::wstring;

namespace gi_lib{

	/// \brief Manager of FBX files.
	/// \author Raffaele D. Facendola
	class FBX{

	public:

		/// \brief Get the singleton instance.
		/// \return Returns the singleton instance.
		static FBX & GetInstance();

		/// \brief Default destructor.
		~FBX();

		void Import(const wstring & path);

		/// \brief Parse a FBX mesh.
		/// \param mesh Mesh to parse.
		/// \return Returns a description of the parsed mesh
		Mesh::CreationSettings Parse(const FbxMesh & mesh);

	private:

		FBX();

		FbxManager * manager_;

		FbxIOSettings * settings_;

		FbxGeometryConverter * converter_;

	};

	//

	inline FBX & FBX::GetInstance(){

		static FBX instance;

		return instance;

	}

}