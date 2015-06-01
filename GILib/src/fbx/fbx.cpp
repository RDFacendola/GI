#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#include <vector>
#include <algorithm>
#include <fbxsdk.h>
#include <type_traits>
#include <sstream>

#include "..\..\include\gilib.h"
#include "..\..\include\gimath.h"
#include "..\..\include\range.h"
#include "..\..\include\fbx\fbx.h"
#include "..\..\include\core.h"
#include "..\..\include\exceptions.h"
#include "..\..\include\graphics.h"
#include "..\..\include\resources.h"
#include "..\..\include\scene.h"
#include "..\..\include\scope_guard.h"

using namespace std;
using namespace Eigen;
using namespace gi_lib;

using gi_lib::fbx::IFbxMaterial;
using gi_lib::fbx::IFbxMaterialImporter;
using gi_lib::fbx::IFbxProperty;
using gi_lib::fbx::FbxMaterialCollection;

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

	/// \brief Converts a 3-element double to a native 3-element vector.
	template <> struct Converter < FbxDouble3 > {

		/// \brief Native type.
		using TConverted = Vector3f;

		/// \brief Converts a 2-element vector to a native 2-element vector.
		/// \param vector Attribute to convert.
		/// \return Returns the same elements of the original vector.
		TConverted operator()(const FbxDouble3& vector){
			
			return Vector3f(static_cast<float>(vector.mData[0]),
							static_cast<float>(vector.mData[1]),
							static_cast<float>(vector.mData[2]));

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

	/// \brief Objects needed while importing.
	struct ImportContext{

		IFbxMaterialImporter* material_importer;

		Resources* resources;

		wstring base_directory;

	};

	/// \brief Find a subproperty by name.
	/// \param base The property where the search will begin.
	/// \param subproperty_name Name of the subproperty. You may use the pipe character "|" to access subproperties (e.g.: "prop|subprop|subsubprop").
	/// \return Returns the specified subproperty if such property exists, otherwise returns an invalid property.
	FbxProperty FindSubProperty(FbxProperty base, const string& subproperty_name){

		string::size_type base_separator;

		string rest = subproperty_name;

		FbxProperty property = base;

		for (;;){

			base_separator = rest.find_first_of("|");

			property = property.Find(rest.substr(0, base_separator).c_str());

			if (!property.IsValid() ||
				base_separator == string::npos){

				// Exit if one of the properties is not found or the last one was checked.
				break;

			}

			rest = rest.substr(base_separator + 1);
			
		}

		return property;

	}

	/// \brief Wrapper around the FbxProperty
	class Property : public IFbxProperty{

	public:

		/// \brief Create a fbx material property wrapper.
		/// \param property Property to wrap.
		Property(FbxProperty property);

		virtual string GetName() const override;

		virtual float ReadFloat() const override;

		virtual Vector3f ReadVector3() const override;

		virtual vector<string> EnumerateTextures() const override;

		virtual unique_ptr<IFbxProperty> operator[](const string& subproperty_name) const override;
		
	private:

		FbxProperty property_;				///< \brief Fbx property

	};

	/// \brief Wrapper around the FbxSurfaceMaterial.
	class Material : public IFbxMaterial{

	public:

		Material(FbxSurfaceMaterial* material);

		virtual string GetName() const override;

		virtual unique_ptr<IFbxProperty> operator[](const string& property_name) const override;

	private:

		FbxSurfaceMaterial* material_;		///< \brief Fbx material

	};

	///////////////////////////////////// PROPERTY ///////////////////////////////////////////

	Property::Property(FbxProperty property) : 
		property_(property){}

	string Property::GetName() const{

		return property_.GetNameAsCStr();

	}

	float Property::ReadFloat() const{

		return static_cast<float>(property_.Get<FbxDouble>());

	}

	Vector3f Property::ReadVector3() const{

		return Converter<FbxDouble3>()(property_.Get<FbxDouble3>());

	}

	vector<string> Property::EnumerateTextures() const{

		vector<string> textures;

		FbxFileTexture* texture;

		for (int texture_index = 0; texture_index < property_.GetSrcObjectCount<FbxFileTexture>(); ++texture_index){

			texture = property_.GetSrcObject<FbxFileTexture>(texture_index);

			textures.push_back(texture->GetFileName());

		}

		return textures;

	}

	unique_ptr<IFbxProperty> Property::operator[](const string& subproperty_name) const{

		auto property = FindSubProperty(property_, 
										subproperty_name);

		return property.IsValid() ?
			   make_unique<Property>(property) :
			   nullptr;

	}

	///////////////////////////////////// MATERIAL ///////////////////////////////////////////

	Material::Material(FbxSurfaceMaterial* material) :
		material_(material){}

	string Material::GetName() const{

		return material_->GetName();

	}

	unique_ptr<IFbxProperty> Material::operator[](const string& property_name) const{

		auto base_separator = property_name.find_first_of("|");

		auto property = material_->FindProperty(property_name.substr(0, base_separator).c_str());

		if (property.IsValid() &&
			base_separator != string::npos){

			// Check the subproperties
			property = FindSubProperty(property, property_name.substr(base_separator + 1));

		}

		return property.IsValid() ?
			   make_unique<Property>(property) :
			   nullptr;
		
	}

	///////////////////////////////////// METHODS ////////////////////////////////////////////

	/// \brief Extract the translation component of a matrix.
	/// \param transform The transformation matrix.
	/// \return Returns the translation component of the given transformation matrix.
	inline Translation3f GetTranslation(const FbxAMatrix& transform){

		auto translation = transform.GetT();

		return Translation3f(static_cast<float>(translation.mData[0]),
							 static_cast<float>(translation.mData[1]),
							 static_cast<float>(translation.mData[2]));

	}

	/// \brief Extract the rotation component of a matrix.
	/// \param transform The transformation matrix.
	/// \return Returns the rotation component of the given transformation matrix.
	inline Quaternionf GetRotation(const FbxAMatrix& transform){

		auto rotation = transform.GetQ();

		return  Quaternionf(static_cast<float>(rotation.mData[3]),
							static_cast<float>(rotation.mData[0]),
							static_cast<float>(rotation.mData[1]),
							static_cast<float>(rotation.mData[2]));

	}

	/// \brief Extract the scale component of a matrix.
	/// \param transform The transformation matrix.
	/// \return Returns the scale component of the given transformation matrix.
	inline AlignedScaling3f GetScale(const FbxAMatrix& transform){

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
	void ImportMeshVertices(FbxMesh& mesh, Mesh::FromVertices<TVertexFormat>& bundle);

	/// \brief Import the vertices of a mesh.
	/// \param mesh Mesh whose vertices needs to be imported.
	/// \return Returns a vector with the imported vertices.
	template <> void ImportMeshVertices<VertexFormatNormalTextured>(FbxMesh& mesh, Mesh::FromVertices<VertexFormatNormalTextured>& bundle){

		bundle.vertices.resize(mesh.GetControlPointsCount());

		WriteAttribute(&mesh.GetControlPoints()[0], bundle.vertices, Position<gi_lib::VertexFormatNormalTextured>());			// Position

		WriteLayerAttribute(*mesh.GetLayer(0)->GetNormals(), bundle.vertices, Normal<VertexFormatNormalTextured>());			// Normal, taken from the first layer.
		WriteLayerAttribute(*mesh.GetLayer(0)->GetUVs(), bundle.vertices, TexCoord<VertexFormatNormalTextured>());				// Texture coordinates, taken from the first layer.

	}

	/// \brief Import the mesh index buffer and subsets.
	/// The mesh <b>must<\b> be a triangle mesh, ie every polygon must have exactly 3 vertices.
	/// \param mesh Source mesh.
	/// \param bundle Store the imported index buffer and subset.
	template <typename TVertexFormat>
	void ImportMeshIndices(FbxMesh& mesh, Mesh::FromVertices<TVertexFormat>& bundle){

		int polygon_size = 3;											// Triangle

		auto polygon_vertices = mesh.GetPolygonVertices();				// Original index buffer
		auto polygon_vertex_count = mesh.GetPolygonVertexCount();		// Indices count

		FbxLayerElementArrayTemplate<int>* material_indices;			// Material indices per polygon

		if (mesh.GetMaterialIndices(&material_indices)){

			// Sort by material index using counting sort, O(material_count + polygon_count)

			auto material_count = mesh.GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();

			int material_index;
			int polygon_index;
			int vertex_index;

			vector<size_t> count(material_count);
			
			// Count material indices

			for (polygon_index = 0; polygon_index < mesh.GetPolygonCount(); ++polygon_index){

				material_index = material_indices->GetAt(polygon_index);

				++(count[material_index]);

			}

			// Starting index for each material index

			size_t total = 0;
			size_t aux_count;

			bundle.subsets.resize(material_count);

			for (material_index = 0; material_index < material_count; ++material_index){

				aux_count = count[material_index];

				// Setup the mesh subset relative to the material index

				bundle.subsets[material_index].start_index = total * polygon_size;
				bundle.subsets[material_index].count = aux_count * polygon_size;

				count[material_index] = total;

				total += aux_count;

			}

			// Sort the indices

			bundle.indices.resize(polygon_vertex_count);	// Output index buffer

			size_t base_input_index;
			size_t base_output_index;

			for (polygon_index = 0; polygon_index < mesh.GetPolygonCount(); ++polygon_index){
				
				material_index = material_indices->GetAt(polygon_index);
				
				// Source and destination index offset

				base_input_index = mesh.GetPolygonVertexIndex(polygon_index);

				base_output_index = polygon_size * count[material_index];

				// Copy the entire polygon

				for (vertex_index = 0; vertex_index < polygon_size; ++vertex_index){

					bundle.indices[base_output_index + vertex_index] = polygon_vertices[base_input_index + vertex_index];

				}
						
				// Next

				++(count[material_index]);

			}

		}
		else{

			// If no material index is found, just use the same material (0) for the entire mesh

			bundle.indices.assign(polygon_vertices,
								  polygon_vertices + polygon_vertex_count);

			bundle.subsets.push_back(MeshSubset{ 0, polygon_vertex_count });

		}

	}

	template <typename TVertexFormat>
	MeshComponent* ImportMesh(FbxMesh& fbx_mesh, TransformComponent& node, const ImportContext& context){

		// Create the bundle used to build the mesh

		Mesh::FromVertices<TVertexFormat> bundle;	

		ImportMeshIndices(fbx_mesh, bundle);
		ImportMeshVertices(fbx_mesh, bundle);

		// Create the mesh component

		auto mesh = context.resources->Load<Mesh, Mesh::FromVertices<TVertexFormat>>(bundle);

		if (mesh){

			return node.AddComponent<MeshComponent>(mesh);

		}
		else{

			return nullptr;


		}

	}
	
	/// \brief Import the mesh materials.
	/// \param mesh The mesh whose materials will be imported.
	void ImportMaterials(const FbxMesh& mesh, MeshComponent& mesh_component, const ImportContext& context){

		auto& fbx_node = *mesh.GetNode();

		FbxMaterialCollection materials;

		for (int material_index = 0; material_index < fbx_node.GetSrcObjectCount<FbxSurfaceMaterial>(); ++material_index){

			materials.push_back(make_unique<Material>(fbx_node.GetSrcObject<FbxSurfaceMaterial>(material_index)));
				
		}

		context.material_importer->OnImportMaterial(context.base_directory,
													materials,
													mesh_component);

	}

	/// \brief Import a mesh as a component.
	/// \param mesh Mesh to import.
	/// \param node Node where the component will be attached.
	void ImportMesh(FbxMesh& mesh, TransformComponent& node, const ImportContext& context){

		// Dispatch the correct "ImportMesh" based on the attributes of the mesh.

		if (HasTextureCoordinates(mesh) &&
			HasNormals(mesh)){

			auto mesh_component = ImportMesh<gi_lib::VertexFormatNormalTextured>(mesh, 
																				 node,
																				 context);

			ImportMaterials(mesh, 
							*mesh_component,
							context);

		}

	}

	/// \brief Import a new component described by a fbx attribute.
	/// \param attribute The attribute to import.
	/// \param node The node where the imported component will be attached to.
	void ImportAttribute(FbxNodeAttribute& attribute, TransformComponent& node, const ImportContext& context){

		switch (attribute.GetAttributeType()){

		case FbxNodeAttribute::EType::eMesh:

			ImportMesh(static_cast<FbxMesh&>(attribute),
					   node,
					   context);

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
	void ImportScene(FbxNode& fbx_root, TransformComponent& parent, const ImportContext& context){

		// Basic components, such as node component and transform component.

		auto node = ImportNode(fbx_root,
							   parent);

		// Other components, stored as fbx attributes.

		for (int attribute_index = 0; attribute_index < fbx_root.GetNodeAttributeCount(); ++attribute_index){

			ImportAttribute(*fbx_root.GetNodeAttributeByIndex(attribute_index),
							*node,
							context);
				
		}

		// Recursion - depth-first to save memory

		for (int child_index = 0; child_index < fbx_root.GetChildCount(); ++child_index){

			ImportScene(*fbx_root.GetChild(child_index),
						*node,
						context);
						

		}

	}
	
	/// \brief Recursively import a FbxScene.
	/// \param fbx_scene Scene to import.
	/// \param parent Node where the imported ones will be attached hierarchically.
	void ImportScene(FbxScene& fbx_scene, TransformComponent& root, const ImportContext& context){

		auto fbx_root = fbx_scene.GetRootNode();

		if (fbx_root){

			ImportScene(*fbx_root,
						root,
						context);

		}

	}

}

/////////////////////// FBXSDK ///////////////////////

struct fbx::FbxImporter::FbxSDK{

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
	FbxScene* ReadSceneOrDie(const string& file_name);
	
};

fbx::FbxImporter::FbxSDK::FbxSDK(){

	manager = FbxManager::Create();

	settings = FbxIOSettings::Create(manager,
									 IOSROOT);

	converter = new FbxGeometryConverter(manager);

}

fbx::FbxImporter::FbxSDK::~FbxSDK(){

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

FbxScene* fbx::FbxImporter::FbxSDK::ReadSceneOrDie(const string& file_name){

	// Create the importer
	
	auto fbx_importer = ::FbxImporter::Create(manager, "");
	
	if (!fbx_importer->Initialize(file_name.c_str(),
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

fbx::FbxImporter::FbxImporter(IFbxMaterialImporter& material_importer, Resources& resources) :
fbx_sdk_(make_unique<FbxSDK>()),
material_importer_(material_importer),
resources_(resources){}

fbx::FbxImporter::~FbxImporter(){}

void fbx::FbxImporter::ImportScene(const string& file_name, TransformComponent& root){

	auto scene = fbx_sdk_->ReadSceneOrDie(file_name);

	// Something may throw during the import, this is needed to destroy the scene.
	
	auto guard = make_scope_guard([scene](){

		scene->Destroy();

	});

	// Context setup
	
	ImportContext context{ &material_importer_, 
						   &resources_, 
						   Application::GetBaseDirectory(to_wstring(file_name)) };

	// Actual import

	::ImportScene(*scene,
				  root,
				  context);
	
}