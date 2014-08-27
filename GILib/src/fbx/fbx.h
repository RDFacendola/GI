/// \file fbx.h
/// \brief Classes and methods to import and convert Autodesk FBX files.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>

#include <fbxsdk.h>

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

	private:

		FBX();

		FbxManager * manager_;

		FbxIOSettings * settings_;

	};

	//

	inline FBX & FBX::GetInstance(){

		static FBX instance;

		return instance;

	}

}