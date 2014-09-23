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

	/// \brief Check whether the mesh defines normals.
	bool HasNormals(const FbxMesh & mesh){

		auto layer = mesh.GetLayer(0);

		return layer != nullptr &&
			layer->GetNormals() != nullptr &&
			layer->GetNormals()->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint;

	}

	/// \brief Write some attributes inside the provided vertex buffer.
	/// \tparam TVertexFormat Type of vertex.
	/// \tparam TAttributeFormat Type of attributes.
	/// \tparam TWriter Type of the writer.
	/// \param attributes Buffer containing the attributes to write.
	/// \param vertices Vertex buffer the attributes will be written to.
	/// \param Writer used to write the attribute inside the vertex
	template <typename TVertexFormat, typename TAttributeFormat, typename TWriter>
	void WriteAttribute(const TAttributeFormat * attributes, vector<TVertexFormat> & vertices, TWriter writer){

		for (auto & vertex : vertices){

			writer(vertex, *attributes);

			++attributes;

		}

	}

	/// \brief Write some attributes inside the provided vertex buffer.
	/// \tparam TVertexFormat Type of vertex.
	/// \tparam TAttributeFormat Type of attributes.
	/// \tparam TWriter Type of the writer.
	/// \param attributes Layer element containing the attributes to write.
	/// \param vertices Vertex buffer the attributes will be written to.
	/// \param Writer used to write the attribute inside the vertex
	template <typename TVertexFormat, typename TAttributeFormat, typename TWriter>
	void WriteAttribute(const FbxLayerElementTemplate<TAttributeFormat> & attributes, vector<TVertexFormat> & vertices, TWriter writer){

		auto & direct_array = attributes.GetDirectArray();
		auto & index_array = attributes.GetIndexArray();

		switch (attributes.GetReferenceMode()){

			case FbxLayerElement::EReferenceMode::eIndex:
			case FbxLayerElement::EReferenceMode::eIndexToDirect:
			{

				auto direct_buffer = static_cast<TAttributeFormat*>(direct_array.GetLocked(FbxLayerElementArray::eReadLock));
				auto index_buffer = static_cast<int*>(index_array.GetLocked(FbxLayerElementArray::eReadLock));

				for (auto & vertex : vertices){

					writer(vertex, direct_buffer[*index_buffer]);

					++index_buffer;

				}

				direct_array.ReadUnlock();
				index_array.ReadUnlock();

				break;

			}

			case FbxLayerElement::EReferenceMode::eDirect:
			{

				auto direct_buffer = static_cast<TAttributeFormat*>(direct_array.GetLocked(FbxLayerElementArray::eReadLock));

				for (auto & vertex : vertices){

					writer(vertex, *direct_buffer);

					++direct_buffer;

				}

				direct_array.ReadUnlock();
				
				break;

			}

		}

	}

	/// \brief Read the vertices from a fbx mesh.
	template <typename TFormat>
	vector<TFormat> ReadVertices(const FbxMesh & mesh);

	/// \brief Read normal texture vertex from a fbx mesh.
	template <> vector<VertexFormatNormalTextured> ReadVertices<VertexFormatNormalTextured>(const FbxMesh & mesh){

		// Position + Texture Coordinates

		vector<VertexFormatNormalTextured> vertices;

		vertices.resize(mesh.GetControlPointsCount());

		WriteAttribute(&mesh.GetControlPoints()[0], vertices, [](VertexFormatNormalTextured & vertex, const FbxVector4 position){ vertex.position = FbxVector4ToEigenVector3f(position); });
		WriteAttribute(*mesh.GetLayer(0)->GetNormals(), vertices, [](VertexFormatNormalTextured & vertex, const FbxVector4 normal){ vertex.normal = FbxVector4ToEigenVector3f(normal); });
		WriteAttribute(*mesh.GetLayer(0)->GetUVs(), vertices, [](VertexFormatNormalTextured & vertex, const FbxVector2 tex_coord){ vertex.tex_coord = FbxVector2ToEigenVector2f(tex_coord); });
			
		return vertices;

	}

	/// \brief Read the indices of a fbx mesh.
	vector<unsigned int> ReadIndices(const FbxMesh & mesh){
	
		auto index_buffer = mesh.GetPolygonVertices();

		// Resize the vertex buffer
		vector<unsigned int> indices;

		// Check whether the index buffer is trivial (ie the i-th position contains i as value, typical of unindexed mesh)

		bool trivial = true;

		for (int i = 0; i < mesh.GetPolygonVertexCount(); ++i){

			if (index_buffer[i] != i){

				trivial = false;		//The index buffer is not "trivial"

				break;
				
			}

		}

		if (trivial){

			return indices;

		}

		// Copy the index buffer elsewhere

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
	template<> void BuildMesh<VertexFormatNormalTextured>(const FbxMesh & mesh, SceneNode & node, Manager & resources){
		
		BuildSettings<Mesh, Mesh::BuildMode::kNormalTextured> settings;

		settings.indices = ReadIndices(mesh);

		settings.vertices = ReadVertices<VertexFormatNormalTextured>(mesh);
		
		node.Add<StaticGeometry>(resources.Build<Mesh, Mesh::BuildMode::kNormalTextured>(settings));

	}

	/// \brief Build material method template. Used to dispatch different build methods.
	template<typename TVertexFormat>
	void BuildMaterial(const FbxMesh & mesh, SceneNode & node, Manager & resources);

	/// \brief Builds a material with proper shader and textures.
	template<> void BuildMaterial<VertexFormatNormalTextured>(const FbxMesh & mesh, SceneNode & node, Manager & resources){

		// Phong shader
		auto shader = resources.Load<Shader, Shader::LoadMode::kCompileFromFile>({ Manager::kPhongShaderFile });

		if (!shader){

			throw RuntimeException(L"Could not find built-in Phong shader");

		}

		// Material
		auto material = resources.Build<Material, Material::BuildMode::kFromShader>({ shader });

		

	}
	
	/// \brief Attempts to load a mesh inside a scene.

	/// The methods load the mesh from fbx and loads it inside the scene. If the original mesh is not supported this method does nothing.
	void BuildObject(const FbxMesh & mesh, SceneNode & node, Manager & resources){

		if (!mesh.IsTriangleMesh()){

			// No triangle mesh, no party!
			return;

		}

		//Assumptions: byControlPoint is the only mapping mode supported (other are just ignored).

		if (HasTextureCoordinates(mesh)  &&
			HasNormals(mesh)){

			BuildMesh<VertexFormatNormalTextured>(mesh, node, resources);
			BuildMaterial<VertexFormatNormalTextured>(mesh, node, resources);
			
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

				// Model object
				BuildObject(*static_cast<FbxMesh*>(attribute), scene_node, resources);
				
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
