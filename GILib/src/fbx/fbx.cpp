#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#include <vector>
#include <algorithm>
#include <Eigen/Core>

#include "fbx/fbx.h"

#include "exceptions.h"
#include "resources.h"

using namespace std;
using namespace gi_lib;
using namespace Eigen;

namespace{

	Vector3f FbxVector4ToEigenVector3f(const FbxVector4 & src){

		return Vector3f(static_cast<float>(src.mData[0]),
			static_cast<float>(src.mData[1]),
			static_cast<float>(src.mData[2]));

	}

	Vector2f FbxVector2ToEigenVector2f(const FbxVector2 & src){

		return Vector2f(static_cast<float>(src.mData[0]),
			static_cast<float>(src.mData[1]));

	}

	template <typename TSource, typename TDestination, typename TMap>
	AttributeMappingMode MapFbxVector(FbxLayerElementTemplate<TSource> * source, vector<TDestination> & destination, TMap map){
		
		if (source){

			destination.clear();
		
			// Fills the buffer attribute

			FbxLayerElementArrayTemplate<TSource> & direct_array = source->GetDirectArray();
			FbxLayerElementArrayTemplate<int> & indirect_array = source->GetIndexArray();

			switch (source->GetReferenceMode()){

			case FbxLayerElement::EReferenceMode::eDirect:
			{

				destination.resize(direct_array.GetCount());

				TSource * array = static_cast<TSource *>(direct_array.GetLocked(FbxLayerElementArray::eReadLock));

				std::transform(&array[0],
					&array[0] + direct_array.GetCount(),
					destination.begin(),
					map);

				direct_array.ReadUnlock();

				break;

			}
			case FbxLayerElement::EReferenceMode::eIndex:
			case FbxLayerElement::EReferenceMode::eIndexToDirect:
			{

				destination.resize(indirect_array.GetCount());

				TSource * array = static_cast<TSource *>(direct_array.GetLocked(FbxLayerElementArray::eReadLock));
				int * indices = static_cast<int *>(indirect_array.GetLocked(FbxLayerElementArray::eReadLock));

				std::transform(&indices[0],
					&indices[0] + indirect_array.GetCount(),
					destination.begin(),
					[&](int index){

						return map(array[index]);

					});

				indirect_array.ReadUnlock();
				direct_array.ReadUnlock();

				break;

			}

			}

			return source->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint ?
				AttributeMappingMode::BY_VERTEX :
				source->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex ?
				AttributeMappingMode::BY_INDEX :
				AttributeMappingMode::UNKNOWN;


		}

		return AttributeMappingMode::UNKNOWN;
		
	}

	void Visit(FbxNode * node, FbxGeometryConverter & converter){

		// Node data
		auto name = node->GetName();

		auto transform = node->EvaluateLocalTransform();

		FbxNodeAttribute * attribute;

		// Attributes
		for (int attribute_index = 0; attribute_index < node->GetNodeAttributeCount(); ++attribute_index){

			attribute = node->GetNodeAttributeByIndex(attribute_index);

				if (attribute->GetAttributeType() == FbxNodeAttribute::EType::eMesh){

					auto mesh = static_cast<FbxMesh*>(converter.Triangulate(attribute, true));

					Mesh::CreationSettings settings;

					// Vertices
					
					auto control_points = mesh->GetControlPoints();			
					auto vertex_count = mesh->GetControlPointsCount();
					
					settings.positions.resize(vertex_count);

					std::transform(&control_points[0],
						&control_points[0] + vertex_count,
						settings.positions.begin(),
						[](FbxVector4 v){ return Vector3f(static_cast<float>(v.mData[0]),
								 					      static_cast<float>(v.mData[1]),
														  static_cast<float>(v.mData[2])); });
							
					//Indices
					auto polygon_vertices = mesh->GetPolygonVertices();		
					auto index_count = mesh->GetPolygonVertexCount();
					
					settings.indices.resize(index_count);

					std::copy(&polygon_vertices[0],
						&polygon_vertices[0] + index_count,
						settings.indices.begin());

					//First layer of the mesh
					if (mesh->GetLayerCount() > 0){

						auto layer = mesh->GetLayer(0);

						// Normals, binormals, tangents and uvs
						settings.normal_mapping = MapFbxVector(layer->GetNormals(), settings.normals, FbxVector4ToEigenVector3f);
						settings.binormal_mapping =  MapFbxVector(layer->GetBinormals(), settings.binormals, FbxVector4ToEigenVector3f);
						settings.tangent_mapping =  MapFbxVector(layer->GetTangents(), settings.tangents, FbxVector4ToEigenVector3f);
						settings.UV_mapping =  MapFbxVector(layer->GetUVs(), settings.UVs, FbxVector2ToEigenVector2f);

					}

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

