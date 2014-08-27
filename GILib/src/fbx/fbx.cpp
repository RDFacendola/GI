#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#include <vector>
#include <algorithm>
#include <Eigen/Dense>

#include "fbx/fbx.h"

#include "exceptions.h"

using namespace std;
using namespace gi_lib;
using namespace Eigen;

namespace{

	void Visit(FbxNode * node, FbxGeometryConverter & converter){

		// Node data
		auto name = node->GetName();

		auto transform = node->EvaluateLocalTransform();

		FbxNodeAttribute * attribute;

		// Attributes
		for (int attribute_index = 0; attribute_index < node->GetNodeAttributeCount(); ++attribute_index){

			attribute = node->GetNodeAttributeByIndex(attribute_index);

				if (attribute->GetAttributeType() == FbxNodeAttribute::EType::eMesh){

					FbxMesh * mesh = static_cast<FbxMesh*>(converter.Triangulate(attribute, true));

					// Vertices
					
					auto control_points = mesh->GetControlPoints();			
					auto vertex_count = mesh->GetControlPointsCount();
					auto vertices = vector<Vector3f>(vertex_count);

					std::transform(&control_points[0],
						&control_points[vertex_count],
						vertices.begin(),
						[](FbxVector4 v){ return Vector3f(static_cast<float>(v.mData[0]),
								 					     static_cast<float>(v.mData[1]),
														 static_cast<float>(v.mData[2])); });
							
					//Indices
					auto polygon_vertices = mesh->GetPolygonVertices();		
					auto index_count = mesh->GetPolygonVertexCount();
					auto indices = vector<unsigned int>(index_count);

					std::copy(&polygon_vertices[0],
						&polygon_vertices[index_count],
						indices.begin());

					int okay;
					okay = 10;

				}
				

		}

		// Recursion - Depth-first

		for (int child_index = 0; child_index < node->GetChildCount(); ++child_index){

			Visit(node->GetChild(child_index), converter);

		}

	}

}

/////////////////////// FBX IMPORTER ///////////////////////

FBX::FBX(){

	manager_ = FbxManager::Create();

	settings_ = FbxIOSettings::Create(manager_, IOSROOT);

}

FBX::~FBX(){

	settings_->Destroy();
	manager_->Destroy();

}

void FBX::Import(const wstring & path){

	// Create the importer
	string fbx_path = string(path.begin(), path.end());
	
	auto fbx_importer = FbxImporter::Create(manager_, "");

	if (!fbx_importer->Initialize(fbx_path.c_str(), -1, manager_->GetIOSettings())) {

		string error = fbx_importer->GetStatus().GetErrorString();

		wstring werror = wstring(error.begin(), error.end());

		throw RuntimeException(L"FbxImporter::Initialize() failed.\n" + werror);

	}

	// Populate a new scene object.
	auto fbx_scene = FbxScene::Create(manager_, "");

	fbx_importer->Import(fbx_scene);

	fbx_importer->Destroy();

	// Walk the hierarchy
	auto root_node = fbx_scene->GetRootNode();

	auto converter = FbxGeometryConverter(manager_);

	if (root_node){

		for (int child_index = 0; child_index < root_node->GetChildCount(); ++child_index){

			Visit(root_node->GetChild(child_index), converter);

		}

	}

}

