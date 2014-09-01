#include "..\include\test.h"

#include "..\include\resources.h"
#include "..\include\core.h"

#include "fbx/fbx.h"

using namespace gi_lib;

void gi_lib::fbxImport(const wstring & path){

	FBX::GetInstance().Import(Application::GetInstance().GetDirectory() + L"Data" + kPathSeparator + path);

}