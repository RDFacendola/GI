#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#include <exception>
#include <string>
#include <sstream>

#include "..\include\fbx.h"

using namespace ::gi_lib;
using namespace ::std;

FBX::FBX(){

	manager_ = FbxManager::Create();

	settings_ = FbxIOSettings::Create(manager_, IOSROOT);

	converter_ = new FbxGeometryConverter(manager_);

}

FBX::~FBX(){

	if (converter_){

		delete converter_;

	}

	if (settings_){

		settings_->Destroy();

	}

	if (manager_){

		manager_->Destroy();

	}

}

FbxScene * FBX::Import(const string & path){

	// Create the importer

	auto fbx_importer = FbxImporter::Create(manager_, "");

	if (!fbx_importer->Initialize(path.c_str(), -1, manager_->GetIOSettings())) {

		stringstream error;

		error << "Unable to initialize the importer" << std::endl
			<< fbx_importer->GetStatus().GetErrorString();

		fbx_importer->Destroy();

		throw runtime_error(error.str().c_str());

	}

	// Populate a new scene object.
	auto fbx_scene = FbxScene::Create(manager_, "");

	if (!fbx_importer->Import(fbx_scene)){

		stringstream error;

		error << "Unable to import the scene" << std::endl
			<< fbx_importer->GetStatus().GetErrorString();

		fbx_importer->Destroy();

		throw runtime_error(error.str().c_str());

	}
	
	//Return
	return fbx_scene;
	
}

void FBX::Triangulate(FbxScene & scene){

	if (!converter_->Triangulate(&scene, true)){

		throw runtime_error("Unable to triangulate the scene");

	}

}

void FBX::Remap(FbxScene & scene, FbxLayerElement::EMappingMode mapping_mode){



}

void FBX::Export(FbxScene & scene, const string & path, bool binary){

	auto fbx_exporter = FbxExporter::Create(manager_, "");

	auto settings = manager_->GetIOSettings();

	if (!fbx_exporter->Initialize(path.c_str(), -1, manager_->GetIOSettings())){

		stringstream error;

		error << "Unable to initialize the exporter" << std::endl
			  << fbx_exporter->GetStatus().GetErrorString();

		fbx_exporter->Destroy();

		throw runtime_error(error.str().c_str());

	}

	if (!fbx_exporter->Export(&scene)){

		stringstream error;

		error << "Unable to export the scene" << std::endl
			<< fbx_exporter->GetStatus().GetErrorString();

		fbx_exporter->Destroy();

		throw runtime_error(error.str().c_str());

	}

	fbx_exporter->Destroy();

}