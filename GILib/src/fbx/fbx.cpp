#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#include <vector>
#include <algorithm>
#include <fbxsdk.h>

#include "..\..\include\fbx\fbx.h"
#include "..\..\include\gimath.h"
#include "..\..\include\exceptions.h"
#include "..\..\include\graphics.h"
#include "..\..\include\resources.h"
#include "..\..\include\resource_traits.h"
#include "..\..\include\scene.h"
#include "..\..\include\components.h"

using namespace std;
using namespace gi_lib;
using namespace Eigen;

namespace{

	/// \brief Map a 4-element fbx vector to a 3-element Eigen one.
	/// \param src The source vector to convert.
	/// \return Returns a 3-element vector. The last element of the source is discarded.
	inline Vector3f FbxVector4ToEigenVector3f(const FbxVector4 & src){

		return Vector3f(static_cast<float>(src.mData[0]),
			static_cast<float>(src.mData[1]),
			static_cast<float>(src.mData[2]));

	}

	/// \brief Map a 2-element fbx vector to a 2-element Eigen one.
	/// \param src The source vector to convert.
	/// \return Returns a 2-element vector.
	inline Vector2f FbxVector2ToEigenVector2f(const FbxVector2 & src){

		return Vector2f(static_cast<float>(src.mData[0]),
			static_cast<float>(src.mData[1]));

	}

	/// \brief Map a fbx matrix to an Eigen 4x4 affine transformation.
	inline Affine3f FbxMatrixToEigenAffine3f(const FbxMatrix & matrix){

		Affine3f affine;

		auto data = affine.data();

		for (int column_index = 0; column_index < 4; ++column_index){

			for (int row_index = 0; row_index < 4; ++row_index){

				*data = static_cast<float>(matrix.Get(row_index, column_index));
				
				++data;

			}
			
		}
		
		return affine;

	}

	/// \brief Check whether the mesh defines UV coordinates.
	bool HasTextureCoordinates(const FbxMesh & mesh){

		auto layer = mesh.GetLayer(0);

		return layer != nullptr &&
			layer->GetUVs() != nullptr &&
			layer->GetUVs()->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint;

	}

	/// \brief Read the vertices from a fbx mesh.
	template <typename TFormat>
	vector<TFormat> ReadVertices(const FbxMesh & mesh);

	/// \brief Read texture vertex from a fbx mesh.
	template <> vector<VertexFormatTextured> ReadVertices<VertexFormatTextured>(const FbxMesh & mesh){

		// Position + Texture Coordinates

		auto position_buffer = &mesh.GetControlPoints()[0];

		auto & uv_array = mesh.GetLayer(0)->GetUVs()->GetDirectArray();
		auto & uv_index_array = mesh.GetLayer(0)->GetUVs()->GetIndexArray();

		// Resize the vertex buffer
		vector<VertexFormatTextured> vertices;

		vertices.resize(mesh.GetControlPointsCount());

		// Copy everything inside the vertex buffer

		switch (mesh.GetLayer(0)->GetUVs()->GetReferenceMode()){

			case FbxLayerElement::EReferenceMode::eIndex:
			case FbxLayerElement::EReferenceMode::eIndexToDirect:
			{

				auto uv_buffer = static_cast<FbxVector2 *>(uv_array.GetLocked(FbxLayerElementArray::eReadLock));
				auto uv_index_buffer = static_cast<int *>(uv_index_array.GetLocked(FbxLayerElementArray::eReadLock));

				for (auto & it : vertices){

					it.position = FbxVector4ToEigenVector3f(*position_buffer);
					it.tex_coord = FbxVector2ToEigenVector2f(uv_buffer[*uv_index_buffer]);

					++position_buffer;
					++uv_index_buffer;

				}

				uv_array.ReadUnlock();
				uv_index_array.ReadUnlock();

				break;


			}

			default:
			{

				auto uv_buffer = static_cast<FbxVector2 *>(uv_array.GetLocked(FbxLayerElementArray::eReadLock));

				for (auto & it : vertices){

					it.position = FbxVector4ToEigenVector3f(*position_buffer);
					it.tex_coord = FbxVector2ToEigenVector2f(*uv_buffer);

					++position_buffer;
					++uv_buffer;

				}

				uv_array.ReadUnlock();

				break;

			}

		}

		return vertices;

	}

	/// \brief Read texture vertex from a fbx mesh.
	template <> vector<VertexFormatPosition> ReadVertices<VertexFormatPosition>(const FbxMesh & mesh){

		// Position + Texture Coordinates

		auto position_buffer = &mesh.GetControlPoints()[0];

		// Resize the vertex buffer
		vector<VertexFormatPosition> vertices;

		vertices.resize(mesh.GetControlPointsCount());

		std::transform(&position_buffer[0],
			&position_buffer[0] + vertices.size(),
			vertices.begin(),
			[](const FbxVector4 & position){ return VertexFormatPosition{ FbxVector4ToEigenVector3f(position) }; });

		return vertices;

	}

	/// \brief Read the indices of a fbx mesh.
	vector<unsigned int> ReadIndices(const FbxMesh & mesh){
	
		auto index_buffer = mesh.GetPolygonVertices();

		// Resize the vertex buffer
		vector<unsigned int> indices;

		indices.resize(mesh.GetPolygonVertexCount());

		std::copy(&index_buffer[0],
			&index_buffer[0] + indices.size(),
			indices.begin());

		return indices;

	}

	/// \brief Build mesh method template. Used to dispatch different build methods.
	template<typename TVertexFormat>
	void BuildMesh(const FbxMesh & mesh, SceneNode & node, Manager & resources);

	/// \brief Builds an indexed mesh with texture coordinates.
	template<> void BuildMesh<VertexFormatPosition>(const FbxMesh & mesh, SceneNode & node, Manager & resources){
		
		BuildSettings<Mesh, Mesh::BuildMode::kPosition> settings;

		settings.indices = ReadIndices(mesh);

		settings.vertices = ReadVertices<VertexFormatPosition>(mesh);
		
		node.Add<StaticGeometry>(resources.Build<Mesh, Mesh::BuildMode::kPosition>(settings));

	}

	/// \brief Builds an indexed mesh with texture coordinates.
	template<> void BuildMesh<VertexFormatTextured>(const FbxMesh & mesh, SceneNode & node, Manager & resources){

		BuildSettings<Mesh, Mesh::BuildMode::kTextured> settings;

		settings.indices = ReadIndices(mesh);

		settings.vertices = ReadVertices<VertexFormatTextured>(mesh);

		node.Add<StaticGeometry>(resources.Build<Mesh, Mesh::BuildMode::kTextured>(settings));

	}


	void BuildMesh(const FbxMesh & mesh, SceneNode & scene_root, Manager & resources){

		if (!mesh.IsTriangleMesh()){

			throw RuntimeException(L"Polygons other than triangles are not supported!");

		}

		//Assumptions: byControlPoint is the only mapping mode supported (other are just ignored).

		if (HasTextureCoordinates(mesh)){

			BuildMesh<VertexFormatTextured>(mesh, scene_root, resources);

		}
		else{

			BuildMesh<VertexFormatPosition>(mesh, scene_root, resources);

		}

	}

	/// \brief Walk the fbx scene and imports the nodes to a scene.
	/// \param fbx_node Current fbx scene node.
	/// \param scene_root Scene root where to import the nodes to.
	/// \param resources Manager used to load various resources.
	void WalkFbxScene(FbxNode * fbx_node, SceneNode & scene_root, Manager & resources){

		// Node data
		string name = fbx_node->GetName();
		wstring wname = wstring(name.begin(), name.end());

		//Create a new node and attack it to the scene
		auto & scene_node = scene_root.GetScene().CreateNode(wname, FbxMatrixToEigenAffine3f(fbx_node->EvaluateLocalTransform()));

		scene_node.SetParent(scene_root);

		FbxNodeAttribute * attribute;

		// Attributes
		for (int attribute_index = 0; attribute_index < fbx_node->GetNodeAttributeCount(); ++attribute_index){

			attribute = fbx_node->GetNodeAttributeByIndex(attribute_index);

			if (attribute->GetAttributeType() == FbxNodeAttribute::EType::eMesh){

				//Builds the mesh found
				BuildMesh(*static_cast<FbxMesh*>(attribute), scene_node, resources);
				
			}
				
		}

		// Recursion - Depth-first

		for (int child_index = 0; child_index < fbx_node->GetChildCount(); ++child_index){

			// The instantiated scene node becomes the root of the next level.
			WalkFbxScene(fbx_node->GetChild(child_index), scene_node, resources);

		}

	}

}

/////////////////////// FBX IMPORTER ///////////////////////

/// \brief Deleter used by COM IUnknown interface.

struct FBXImporter::FbxSDK{

	FbxManager * manager;

	FbxIOSettings * settings;

	FbxGeometryConverter * converter;

	~FbxSDK(){

		if (converter){

			delete converter;

		}

		if (settings){

			settings->Destroy();

		}

		if (manager){

			manager->Destroy();

		}

	}

};

FBXImporter::FBXImporter(){

	// Pimpl FBX objects

	fbx_sdk_ = new FbxSDK();

	fbx_sdk_->manager = FbxManager::Create();

	fbx_sdk_->settings = FbxIOSettings::Create(fbx_sdk_->manager, IOSROOT);

	fbx_sdk_->converter = new FbxGeometryConverter(fbx_sdk_->manager);

}

FBXImporter::~FBXImporter(){

	delete fbx_sdk_;

}

void FBXImporter::ImportScene(const wstring & file_name, SceneNode & scene_root, Manager & resources){

	// Create the importer
	string fbx_path = string(file_name.begin(), file_name.end());
	
	auto fbx_importer = FbxImporter::Create(fbx_sdk_->manager, "");

	if (!fbx_importer->Initialize(fbx_path.c_str(), -1, fbx_sdk_->manager->GetIOSettings())) {

		string error = fbx_importer->GetStatus().GetErrorString();

		wstring werror = wstring(error.begin(), error.end());

		throw RuntimeException(L"FbxImporter::Initialize() failed.\n" + werror);

	}

	// Populate a new scene object.
	auto fbx_scene = FbxScene::Create(fbx_sdk_->manager, "");

	fbx_importer->Import(fbx_scene);

	fbx_importer->Destroy();

	// Triangulate the scene (Better to do this offline, as it is heavily time-consuming)
	if (!fbx_sdk_->converter->Triangulate(fbx_scene, true)){

		throw RuntimeException(L"FbxGeometryConverter::Triangulate() failed.\n");

	}

	// Walk the hierarchy
	auto root_node = fbx_scene->GetRootNode();

	if (root_node){

		for (int child_index = 0; child_index < root_node->GetChildCount(); ++child_index){

			WalkFbxScene(root_node->GetChild(child_index), scene_root, resources);

		}

	}

	fbx_scene->Destroy();

}
