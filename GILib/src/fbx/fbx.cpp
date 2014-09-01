#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#include <vector>
#include <algorithm>
#include <Eigen\Core>

#include "fbx.h"

#include "..\..\include\exceptions.h"
#include "..\..\include\resources.h"

using namespace std;
using namespace gi_lib;
using namespace Eigen;

namespace{

	/// \brief Map a 4-element fbx vector to a 3-element eigen one.
	/// \param src The source vector to convert.
	/// \return Returns a 3-element vector. The last element of the source is discarded.
	inline Vector3f FbxVector4ToEigenVector3f(const FbxVector4 & src){

		return Vector3f(static_cast<float>(src.mData[0]),
			static_cast<float>(src.mData[1]),
			static_cast<float>(src.mData[2]));

	}

	/// \brief Map a 2-element fbx vector to a 2-element eigen one.
	/// \param src The source vector to convert.
	/// \return Returns a 2-element vector.
	inline Vector2f FbxVector2ToEigenVector2f(const FbxVector2 & src){

		return Vector2f(static_cast<float>(src.mData[0]),
			static_cast<float>(src.mData[1]));

	}

	/// \brief Map a fbx mapping mode to a mesh attribute mapping mode
	/// \param mapping_mode The mapping mode to convert.
	/// \return Returns the mapped mapping mode.
	inline AttributeMappingMode FbxMappingModeToAttributeMappingNode(FbxLayerElement::EMappingMode mapping_mode){

		switch (mapping_mode)
		{
		case fbxsdk_2015_1::FbxLayerElement::eByControlPoint:
			
			return AttributeMappingMode::BY_VERTEX;
			break;

		case fbxsdk_2015_1::FbxLayerElement::eByPolygonVertex:

			return AttributeMappingMode::BY_INDEX;
			break;

		default:

			return AttributeMappingMode::UNKNOWN;
			break;

		}
		
	}

	/// \brief Map a layer element to a plain vector.
	/// \tparam TSource Type of the elements inside the layer element.
	/// \tparam TDestination Tyype of the elements inside the destination vector.
	/// \tparam TMap Type of the mapping function that will be used to convert the elements.
	/// \param source The layer element to convert.
	/// \param destination The vector where the mapped data will be stored.
	/// \param map The mapping function.
	/// \return Returns the mapping mode assiciated to the layer element.
	template <typename TSource, typename TDestination, typename TMap>
	AttributeMappingMode MapFbxLayerElement(const FbxLayerElementTemplate<TSource> * source, vector<TDestination> & destination, TMap map){
		
		destination.clear();

		if (!source){

			return AttributeMappingMode::UNKNOWN;

		}

		// Fills the buffer attribute

		FbxLayerElementArrayTemplate<TSource> & direct_array = source->GetDirectArray();
		FbxLayerElementArrayTemplate<int> & index_array = source->GetIndexArray();

		switch (source->GetReferenceMode()){

		case FbxLayerElement::EReferenceMode::eDirect:
		{

			// Direct mapping, each element inside the direct array is mapped 1-to-1 to the destination array.
			destination.resize(direct_array.GetCount());

			TSource * direct_buffer = static_cast<TSource *>(direct_array.GetLocked(FbxLayerElementArray::eReadLock));

			std::transform(&direct_buffer[0],
				&direct_buffer[0] + direct_array.GetCount(),
				destination.begin(),
				map);

			direct_array.ReadUnlock();

			break;

		}
		case FbxLayerElement::EReferenceMode::eIndex:
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{

			// Indirect mapping, each element inside the index array points to the actual vertex to be mapped inside the destination array.

			destination.resize(index_array.GetCount());

			TSource * direct_buffer = static_cast<TSource *>(direct_array.GetLocked(FbxLayerElementArray::eReadLock));
			int * index_buffer = static_cast<int *>(index_array.GetLocked(FbxLayerElementArray::eReadLock));

			std::transform(&index_buffer[0],
				&index_buffer[0] + index_array.GetCount(),
				destination.begin(),
				[&](int index){

					return map(direct_buffer[index]);

				});

			direct_array.ReadUnlock();
			index_array.ReadUnlock();

			break;

		}

		}

		return FbxMappingModeToAttributeMappingNode(source->GetMappingMode());
		
	}

	void Visit(FbxNode * node, FbxGeometryConverter & converter){

		// Node data
		string name = node->GetName();

		auto transform = node->EvaluateLocalTransform();

		FbxNodeAttribute * attribute;

		// Attributes
		for (int attribute_index = 0; attribute_index < node->GetNodeAttributeCount(); ++attribute_index){

			attribute = node->GetNodeAttributeByIndex(attribute_index);

				if (attribute->GetAttributeType() == FbxNodeAttribute::EType::eMesh){

					//Parse mesh
					FBX::GetInstance().Parse(*static_cast<FbxMesh*>(attribute));
					
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

	// Triangulate the scene (Better to do this offline, as it is heavily time-consuming)
	if (!converter_->Triangulate(fbx_scene, true)){

		throw RuntimeException(L"FbxGeometryConverter::Triangulate() failed.\n");

	}

	// Walk the hierarchy
	auto root_node = fbx_scene->GetRootNode();

	if (root_node){

		for (int child_index = 0; child_index < root_node->GetChildCount(); ++child_index){

			Visit(root_node->GetChild(child_index), *converter_);

		}

	}

}

Mesh::CreationSettings FBX::Parse(const FbxMesh & mesh){

	if (!mesh.IsTriangleMesh()){

		throw RuntimeException(L"Polygons other than triangles are not supported!");

	}

	Mesh::CreationSettings settings;

	// Vertices aka Control Points

	auto control_points = mesh.GetControlPoints();
	auto vertex_count = mesh.GetControlPointsCount();

	settings.positions.resize(vertex_count);

	std::transform(&control_points[0],
		&control_points[0] + vertex_count,
		settings.positions.begin(),
		FbxVector4ToEigenVector3f);

	//Indices aka Polygon Vertices

	auto polygon_vertices = mesh.GetPolygonVertices();
	auto index_count = mesh.GetPolygonVertexCount();

	settings.indices.resize(index_count);

	std::copy(&polygon_vertices[0],
		&polygon_vertices[0] + index_count,
		settings.indices.begin());

	//First layer of the mesh
	if (mesh.GetLayerCount() > 0){

		auto layer = mesh.GetLayer(0);

		// Normals, Binormals, Tangents and UVs.
		settings.normal_mapping = MapFbxLayerElement(layer->GetNormals(), settings.normals, FbxVector4ToEigenVector3f);
		settings.binormal_mapping = MapFbxLayerElement(layer->GetBinormals(), settings.binormals, FbxVector4ToEigenVector3f);
		settings.tangent_mapping = MapFbxLayerElement(layer->GetTangents(), settings.tangents, FbxVector4ToEigenVector3f);
		settings.UV_mapping = MapFbxLayerElement(layer->GetUVs(), settings.UVs, FbxVector2ToEigenVector2f);

	}

	return settings;

}
