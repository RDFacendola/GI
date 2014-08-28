#include "test.h"

#include "fbx/fbx.h"

#include "resources.h"
#include "core.h"

using namespace gi_lib;

void gi_lib::fbxImport(const wstring & path){

	FBX::GetInstance().Import(Application::GetInstance().GetDirectory() + L"Data" + kPathSeparator + path);

}