#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#include <vector>
#include <algorithm>
#include <fbxsdk.h>
#include <type_traits>

#include "..\..\include\gilib.h"
#include "..\..\include\gimath.h"
#include "..\..\include\range.h"
#include "..\..\include\fbx\fbx.h"
#include "..\..\include\core.h"
#include "..\..\include\exceptions.h"
#include "..\..\include\graphics.h"
#include "..\..\include\resources.h"
#include "..\..\include\bundles.h"
#include "..\..\include\scene.h"

using namespace std;
using namespace Eigen;

using gi_lib::Resources;
using gi_lib::TransformComponent;
using gi_lib::NodeComponent;
using gi_lib::to_wstring;

using gi_lib::IndexedIterator;
using gi_lib::make_indexed;

namespace{

	/// \brief Functor used to convert a fbx vertex attribute type to a native one.
	/// \tparam TVertexAttributeFormat Type of the vertex attribute.
	template <typename TVertexAttributeFormat>
	struct Converter;

	/// \brief Converts a 4-element vector to a native 3-element vector.
	template <> struct Converter<FbxVector4> {

		/// \brief Native type.
		using TConverted = Vector3f;

		/// \brief Converts a 4-element vector to a native 3-element vector.
		/// \param vector Attribute to convert.
		/// \return Returns the first 3 elements of the original vector.
		TConverted operator()(const FbxVector4& vector){

			return Vector3f(static_cast<float>(vector.mData[0]),
							static_cast<float>(vector.mData[1]),
							static_cast<float>(vector.mData[2]));

		}

	};

	/// \brief Converts a 4-element vector to a native 2-element vector.
	template <> struct Converter<FbxVector2> {

		/// \brief Native type.
		using TConverted = Vector2f;

		/// \brief Converts a 2-element vector to a native 2-element vector.
		/// \param vector Attribute to convert.
		/// \return Returns the same elements of the original vector.
		TConverted operator()(const FbxVector2& vector){

			return Vector2f(static_cast<float>(vector.mData[0]),
							static_cast<float>(vector.mData[1]));

		}

	};

	/// \brief Extract the position of a vertex.
	template <typename TVertexFormat>
	struct Position{

		/// \brief Extract the position of a vertex.
		Vector3f& operator()(TVertexFormat& vertex){

			return vertex.position;

		}
		
	};

	/// \brief Extract the normal of a vertex.
	template <typename TVertexFormat>
	struct Normal{

		/// \brief Extract the normal of a vertex.
		Vector3f& operator()(TVertexFormat& vertex){

			return vertex.normal;

		}

	};

	/// \brief Extract the texture coordinate of a vertex.
	template <typename TVertexFormat>
	struct TexCoord{

		/// \brief Extract the texture coordinate of a vertex.
		Vector2f& operator()(TVertexFormat& vertex){

			return vertex.tex_coord;

		}

	};

	/// \brief Extract the translation component of a matrix.
	/// \param transform The transformation matrix.
	/// \return Returns the translation component of the given transformation matrix.
	Translation3f GetTranslation(const FbxAMatrix& transform){

		auto translation = transform.GetT();

		return Translation3f(static_cast<float>(translation.mData[0]),
							 static_cast<float>(translation.mData[1]),
							 static_cast<float>(translation.mData[2]));

	}

	/// \brief Extract the rotation component of a matrix.
	/// \param transform The transformation matrix.
	/// \return Returns the rotation component of the given transformation matrix.
	Quaternionf GetRotation(const FbxAMatrix& transform){

		auto rotation = transform.GetQ();

		return Quaternionf(static_cast<float>(rotation.mData[0]),
						   static_cast<float>(rotation.mData[1]),
						   static_cast<float>(rotation.mData[2]),
						   static_cast<float>(rotation.mData[3]));

	}

	/// \brief Extract the scale component of a matrix.
	/// \param transform The transformation matrix.
	/// \return Returns the scale component of the given transformation matrix.
	AlignedScaling3f GetScale(const FbxAMatrix& transform){

		auto scale = transform.GetS();

		return AlignedScaling3f(static_cast<float>(scale.mData[0]),
								static_cast<float>(scale.mData[1]),
								static_cast<float>(scale.mData[2]));

	}

	/// \brief Check whether the mesh defines UV coordinates in the first layer.
	/// \param mesh Mesh to check.
	/// \return Returns true if the mesh defines UV coordinates in the first layer, returns false otherwise.
	bool HasTextureCoordinates(const FbxMesh& mesh){

		auto layer = mesh.GetLayer(0);

		return layer != nullptr &&
			   layer->GetUVs() != nullptr &&
			   layer->GetUVs()->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint;

	}

	/// \brief Check whether the mesh defines normals in the first layer.
	/// \param mesh Mesh to check.
	/// \return Returns true if the mesh defines normals in the first layer, returns false otherwise.
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
	template <typename TVertexFormat, typename TAttributeFormat, typename TAttributeMap>
	void WriteAttribute(TAttributeFormat attributes, vector<TVertexFormat>& vertices, TAttributeMap attribute_map){

		Converter<remove_reference<decltype(*attributes)>::type> converter;

		for (auto& vertex : vertices){

			attribute_map(vertex) = converter(*attributes);

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
	template <typename TVertexFormat, typename TAttributeFormat, typename TAttributeMap>
	void WriteLayerAttribute(const FbxLayerElementTemplate<TAttributeFormat>& attributes, vector<TVertexFormat>& vertices, TAttributeMap attribute_map){

		auto& direct_array = attributes.GetDirectArray();
		
		auto direct_buffer = static_cast<TAttributeFormat*>(direct_array.GetLocked(FbxLayerElementArray::eReadLock));

		switch (attributes.GetReferenceMode()){

		case FbxLayerElement::EReferenceMode::eIndex:
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{

			// Indirect mapping

			auto& index_array = attributes.GetIndexArray();

			auto index_buffer = static_cast<int*>(index_array.GetLocked(FbxLayerElementArray::eReadLock));

			WriteAttribute(make_indexed(direct_buffer, index_buffer), 
						   vertices, 
						   attribute_map);

			index_array.ReadUnlock();

			break;

		}

		case FbxLayerElement::EReferenceMode::eDirect:
		{

			// Direct mapping

			WriteAttribute(direct_buffer, 
						   vertices, 
						   attribute_map);

			break;

		}

		}

		direct_array.ReadUnlock();

	}

	/// \brief Import the vertices of a mesh.
	/// \param mesh Mesh whose vertices needs to be imported.
	/// \return Returns a vector with the imported vertices.
	template <typename TVertexFormat>
	vector<TVertexFormat> ImportMeshVertices(FbxMesh& mesh);

	/// \brief Import the vertices of a mesh.
	/// \param mesh Mesh whose vertices needs to be imported.
	/// \return Returns a vector with the imported vertices.
	template <> vector<gi_lib::VertexFormatNormalTextured> ImportMeshVertices<gi_lib::VertexFormatNormalTextured>(FbxMesh& mesh){

		vector<gi_lib::VertexFormatNormalTextured> vertices(mesh.GetControlPointsCount());

		WriteAttribute(&mesh.GetControlPoints()[0], vertices, Position<gi_lib::VertexFormatNormalTextured>());				// Position

		WriteLayerAttribute(*mesh.GetLayer(0)->GetNormals(), vertices, Normal<gi_lib::VertexFormatNormalTextured>());		// Normal, taken from the first layer.
		WriteLayerAttribute(*mesh.GetLayer(0)->GetUVs(), vertices, TexCoord<gi_lib::VertexFormatNormalTextured>());			// Texture coordinates, taken from the first layer.

		return vertices;

	}

	/// \brief Import the index buffer of a mesh.
	/// \param mesh The mesh containing the indices to read.
	/// \return Returns a vector containing the indices of the specified mesh.
	vector<unsigned int> ImportMeshIndices(FbxMesh& mesh){

		auto indices = mesh.GetPolygonVertices();

		return vector<unsigned int>(&indices[0],
									&indices[0] + mesh.GetPolygonVertexCount());
		
	}


	template <typename TVertexFormat>
	void ImportMesh(FbxMesh& mesh, TransformComponent& node, Resources& resources){

		gi_lib::BuildFromVertices<TVertexFormat> bundle;	

		bundle.indices = ImportMeshIndices(mesh);

		bundle.vertices = ImportMeshVertices<TVertexFormat>(mesh);

		// Create the mesh component

		node.AddComponent<gi_lib::MeshComponent>(resources.Load<gi_lib::Mesh, gi_lib::BuildFromVertices<TVertexFormat>>(bundle));

	}


	/// \brief Import a mesh as a component.
	/// \param mesh Mesh to import.
	/// \param node Node where the component will be attached.
	void ImportMesh(FbxMesh& mesh, TransformComponent& node, Resources& resources){

		// Dispatch the correct "ImportMesh" based on the attributes of the mesh.

		if (HasTextureCoordinates(mesh) &&
			HasNormals(mesh)){

			ImportMesh<gi_lib::VertexFormatNormalTextured>(mesh, 
														   node,
														   resources);

			// TODO: Read materials here

		}


	}

	/// \brief Import a new component described by a fbx attribute.
	/// \param attribute The attribute to import.
	/// \param node The node where the imported component will be attached to.
	void ImportAttribute(FbxNodeAttribute& attribute, TransformComponent& node, Resources& resources){

		switch (attribute.GetAttributeType()){

		case FbxNodeAttribute::EType::eMesh:

			ImportMesh(static_cast<FbxMesh&>(attribute),
					   node,
					   resources);

			break;

		default:

			// Do nothing, yay
			break;

		}

	}

	/// \brief Import a Fbx Node inside a scene and attach it to a parent node.
	/// \param fbx_node Node to import.
	/// \param parent Parent node where the imported node will be attached.
	/// \return Returns the imported node.
	TransformComponent* ImportNode(FbxNode& fbx_node, TransformComponent& parent){

		auto& scene = parent.GetComponent<NodeComponent>()->GetScene();

		auto transform = fbx_node.EvaluateLocalTransform();	// Local transformation of the node.

		auto imported_node = scene.CreateNode(to_wstring(fbx_node.GetName()),
											  GetTranslation(transform),
											  GetRotation(transform),
											  GetScale(transform));

		imported_node->SetParent(&parent);

		return imported_node;

	}

	/// \brief Recursively import a FbxScene.
	/// \param fbx_node Root of the scene.
	/// \param parent Node where the imported ones will be attached hierarchically.
	void ImportScene(FbxNode& fbx_root, TransformComponent& parent, Resources& resources){

		// Basic components, such as node component and transform component.

		auto node = ImportNode(fbx_root,
							   parent);

		// Other components, stored as fbx attributes.

		for (int attribute_index = 0; attribute_index < fbx_root.GetNodeAttributeCount(); ++attribute_index){

			ImportAttribute(*fbx_root.GetNodeAttributeByIndex(attribute_index),
							*node,
							resources);
				
		}

		// Recursion - depth-first to save memory

		for (int child_index = 0; child_index < fbx_root.GetChildCount(); ++child_index){

			ImportScene(*fbx_root.GetChild(child_index),
						*node,
						resources);
						

		}

	}
	
	/// \brief Recursively import a FbxScene.
	/// \param fbx_scene Scene to import.
	/// \param parent Node where the imported ones will be attached hierarchically.
	void ImportScene(FbxScene& fbx_scene, TransformComponent& root, Resources& resources){

		auto fbx_root = fbx_scene.GetRootNode();

		if (fbx_root){

			ImportScene(*fbx_root,
						root,
						resources);

		}

	}

}

/////////////////////// FBXSDK ///////////////////////

struct gi_lib::FbxImporter::FbxSDK{

	/// \brief Manager of the Fbx SDK.
	FbxManager* manager;

	/// \brief Settings used during the import or export.
	FbxIOSettings* settings;

	/// \brief Used to convert the imported geometry.
	FbxGeometryConverter* converter;
	
	/// \brief Create the Fbx SDK implementation object.
	FbxSDK();

	/// \brief Destructor.
	~FbxSDK();

	/// \brief Read a scene from file.
	/// \param file_name Name of the file containing the scene.
	/// \return Returns a pointer to the imported scene.
	FbxScene* ReadSceneOrDie(const wstring& file_name);
	
};

gi_lib::FbxImporter::FbxSDK::FbxSDK(){

	manager = FbxManager::Create();

	settings = FbxIOSettings::Create(manager,
									 IOSROOT);

	converter = new FbxGeometryConverter(manager);

}

gi_lib::FbxImporter::FbxSDK::~FbxSDK(){

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

FbxScene* gi_lib::FbxImporter::FbxSDK::ReadSceneOrDie(const wstring& file_name){

	// Create the importer
	
	auto fbx_importer = ::FbxImporter::Create(manager, "");
	
	if (!fbx_importer->Initialize(to_string(file_name).c_str(),
								  -1,
								  manager->GetIOSettings())) {

		THROW(L"FbxImporter::Initialize() failed.\n" + to_wstring(fbx_importer->GetStatus().GetErrorString()));

	}

	// Populate a scene object.

	auto fbx_scene = FbxScene::Create(manager, "");

	fbx_importer->Import(fbx_scene);

	fbx_importer->Destroy();

	// Triangulate the scene (Better to do this offline, as it is heavily time-consuming)

	if (!converter->Triangulate(fbx_scene, true)){

		THROW(L"FbxGeometryConverter::Triangulate() failed.\n");

	}

	return fbx_scene;

}

/////////////////////// FBX IMPORTER ///////////////////////

gi_lib::FbxImporter::FbxImporter() :
fbx_sdk_(make_unique<FbxSDK>()){}

gi_lib::FbxImporter::~FbxImporter(){}

void gi_lib::FbxImporter::ImportScene(const wstring& file_name, TransformComponent& root, Resources& resources){

	auto scene = fbx_sdk_->ReadSceneOrDie(file_name);	// Use RAII, ImportScene may throw...
	
	::ImportScene(*scene,
				  root,
				  resources);

	scene->Destroy();

}

/*

auto app_directory = Application::GetDirectory();

auto base_directory = Application::GetBaseDirectory(file_name.substr(app_directory.length(),
													file_name.length() - app_directory.length()));

*/