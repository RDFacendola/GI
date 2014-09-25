#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#ifdef _WIN32

#include <Windows.h>

#endif

#include <exception>
#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include <process.h>

#include "..\include\fbx.h"
#include "..\include\shell_utils.h"

using namespace ::gi_lib;
using namespace ::std;

namespace{

	const double epsilon = 2.0 / 128.0;
	const double theta_epsilon = 3.141592653 / 6;

	struct LayerElements{

		vector<FbxVector4> normals;
		vector<FbxVector4> binormals;
		vector<FbxVector4> tangents;
		vector<FbxVector2> uvs;

	};

	template <typename T>
	bool Equals(const T & left, const T & right);

	template <> bool Equals<FbxVector4>(const FbxVector4 & left, const FbxVector4 & right){

		return abs(left[0] - right[0]) < epsilon &&
			abs(left[1] - right[1]) < epsilon &&
			abs(left[2] - right[2]) < epsilon &&
			abs(left[3] - right[3]) < epsilon;

	}

	template <> bool Equals<FbxVector2>(const FbxVector2 & left, const FbxVector2 & right){

		return abs(left[0] - right[0]) < epsilon &&
			abs(left[1] - right[1]) < epsilon;

	}

	/// \brief Get an element from a FbxLayerElementTemplate by index.
	template <typename TType>
	TType Get(const FbxLayerElementTemplate<TType> & element, int index){

		auto & direct = element.GetDirectArray();

		auto ref = element.GetReferenceMode();

		switch(ref){

		case FbxLayerElement::EReferenceMode::eDirect:

			return direct[index];

		case FbxLayerElement::EReferenceMode::eIndex:
		case FbxLayerElement::EReferenceMode::eIndexToDirect:

			return direct[element.GetIndexArray()[index]];

		default:

			// Should never happen though
			throw exception("Unexpected reference mode (supported modes: eDirect, eIndex or eIndexToDirect)");

		}

	}

	/// \brief Unroll a layer element to a plain vector.
	template <typename TType>
	void UnrollElement(const FbxMesh & mesh, FbxLayerElementTemplate<TType> * element_ptr, vector<TType> & destination){

		if (element_ptr == nullptr){

			return;		//Nothing to do here

		}

		FbxLayerElementTemplate<TType> & element = *element_ptr;

		destination.resize(mesh.GetPolygonVertexCount());

		switch (element.GetMappingMode()){

		case FbxLayerElement::EMappingMode::eByControlPoint:{

			// Unroll the array inside the destination
			auto polygon_vertices = mesh.GetPolygonVertices();

			for (int index = 0; index < destination.size(); ++index){

				destination[index] = Get(element, polygon_vertices[index]);

			}

			break;

		}

		case FbxLayerElement::EMappingMode::eByPolygonVertex:{

			// Copy the array
			
			for (int index = 0; index < destination.size(); ++index){

				destination[index] = Get(element, index);

			}
		
			break;

		}

		default:

			throw exception("Unexpected mapping mode (supported modes: eByControlPoint, eByPolygonVertex");

		}

	}

	/// \brief Attempts to roll a layer element to a plain vector.
	template <typename TType>
	bool RollElement(const FbxMesh & mesh, FbxLayerElementTemplate<TType> * element_ptr, vector<TType> & destination){

		if (element_ptr == nullptr){

			destination.resize(0);
			return true;

		}

		FbxLayerElementTemplate<TType> & element = *element_ptr;

		destination.resize(mesh.GetControlPointsCount());

		switch (element.GetMappingMode()){

		case FbxLayerElement::EMappingMode::eByControlPoint:{

			// Copy the array
			for (int i = 0; i < destination.size(); ++i){

				destination[i] = Get(element, i);

			}

			break;

		}

		case FbxLayerElement::EMappingMode::eByPolygonVertex:{

			// See whether the attributes are duplicated according to the index buffer

			auto polygon_vertices = mesh.GetPolygonVertices();

			vector<bool> set(destination.size());

			std::fill(set.begin(), set.end(), false);

			for (int i = 0; i < mesh.GetPolygonVertexCount(); ++i){

				auto vertex_index = polygon_vertices[i];

				if (!set[vertex_index]){

					destination[vertex_index] = Get(element, i);
					set[vertex_index] = true;

				}
				else if (!Equals(destination[vertex_index], Get(element, i))){

					// The same control point has different values for the same attribute. Rollback!
					return false;

				}

			}

			break;

		}

		default:

			throw exception("Unexpected mapping mode (supported modes: eByControlPoint, eByPolygonVertex");

		}

		return true;

	}

	size_t GetIndex(vector<FbxVector4> vertices, vector<LayerElements> layers, vector<FbxVector4> & indexed_vertices, vector<LayerElements> & indexed_layers, int index){

		// Check against every other indexed vertex
		for (int i = 0; i < indexed_vertices.size(); ++i){
				
			if (!Equals(vertices[index], indexed_vertices[i])){

				continue;

			}

			bool found = true;

			for (int l = 0; l < layers.size(); ++l){

				auto & layer = layers[l];
				auto & indexed_layer = indexed_layers[l];

				if (layer.normals.size() > 0	&& !Equals(layer.normals[index], indexed_layer.normals[i]) ||
					layer.binormals.size() > 0	&& !Equals(layer.binormals[index], indexed_layer.binormals[i]) ||
					layer.tangents.size() > 0	&& !Equals(layer.tangents[index], indexed_layer.tangents[i]) ||
					layer.uvs.size() > 0		&& !Equals(layer.uvs[index], indexed_layer.uvs[i])){

					found = false;
					break;

				}


			}

			if (found){

				// Return the found index;
				return i;

			}

		}

		//Not found, add a new vertex

		size_t new_index = indexed_vertices.size();

		indexed_vertices.push_back(vertices[index]);

		for (int l = 0; l < layers.size(); ++l){

			auto & layer = layers[l];
			auto & indexed_layer = indexed_layers[l];

			if (layer.normals.size() > 0)	indexed_layer.normals.push_back(layer.normals[new_index]);
			if (layer.binormals.size() > 0)	indexed_layer.binormals.push_back(layer.binormals[new_index]);
			if (layer.tangents.size() > 0)	indexed_layer.tangents.push_back(layer.tangents[new_index]);
			if (layer.uvs.size() > 0)		indexed_layer.uvs.push_back(layer.uvs[new_index]);

		}

		// Return the new index.
		return new_index;

	}

	template <typename TType>
	void CommitLayerElementRemap(vector<TType> source, FbxLayerElementTemplate<TType> * destination_ptr){

		if (destination_ptr == nullptr){

			return;	//Nothing to do here...

		}

		FbxLayerElementTemplate<TType> & destination = *destination_ptr;

		destination.Clear();

		destination.SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
		destination.SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);			//Doesn't really matter...

		// Write the source data inside the direct array.

		auto & destination_array = destination.GetDirectArray();

		destination_array.Resize(static_cast<int>(source.size()));

		for (int i = 0; i < source.size(); ++i){

			destination_array.SetAt(i, source[i]);

		}
				
	}

	void CommitRemap(FbxMesh & mesh, vector<FbxVector4> * vertices_ptr, vector<unsigned int> * indices_ptr, vector<LayerElements> & layers){

		// Copy the vertices
		if (vertices_ptr){

			auto & vertices = *vertices_ptr;

			mesh.InitControlPoints(static_cast<int>(vertices.size()));

			auto control_points = mesh.GetControlPoints();

			for (int i = 0; i < vertices.size(); ++i){

				control_points[i] = vertices[i];

			}
			
		}

		// Copy the indices
		if (indices_ptr){

			auto & indices = *indices_ptr;

			// The polygon vertex count never change during remapping

			auto polygon_vertices = mesh.GetPolygonVertices();

			for (int i = 0; i < indices.size(); ++i){

				polygon_vertices[i] = indices[i];

			}

		}

		// Copy the layer elements
		for (int l = 0; l < layers.size(); ++l){

			auto & src_layer = layers[l];

			auto & dst_layer = *mesh.GetLayer(l);

			CommitLayerElementRemap(src_layer.normals, dst_layer.GetNormals());
			CommitLayerElementRemap(src_layer.binormals, dst_layer.GetBinormals());
			CommitLayerElementRemap(src_layer.tangents, dst_layer.GetTangents());
			CommitLayerElementRemap(src_layer.uvs, dst_layer.GetUVs());

		}

	}

	/// \brief Remap the mesh attributes
	void RemapAttributes(FbxMesh & mesh){
		
		vector<LayerElements> layers(mesh.GetLayerCount());

		// Attempts layer elements roll. Vertex buffer is left untouched.
		cout << "Re-indexing...";

		bool roll = true;

		for (int l = 0; l < layers.size(); ++l){

			auto & layer = *mesh.GetLayer(l);

			if (!RollElement(mesh, layer.GetNormals(), layers[l].normals) ||
				!RollElement(mesh, layer.GetBinormals(), layers[l].binormals) ||
				!RollElement(mesh, layer.GetTangents(), layers[l].tangents) ||
				!RollElement(mesh, layer.GetUVs(), layers[l].uvs)){

				roll = false;
				break;
				
			}

		}

		if (roll){

			// Neither the vertex buffer, nor the index buffer change.

			CommitRemap(mesh, nullptr, nullptr, layers);

			cout << "success!" << std::endl;

			return;

		}

		cout << "\rUn-indexing...";

		vector<FbxVector4> vertices;
		vector<unsigned int> indices(mesh.GetPolygonVertexCount());
		auto polygon_vertices = mesh.GetPolygonVertices();

		// Vertex unroll
		auto control_points = mesh.GetControlPoints();

		vertices.resize(mesh.GetPolygonVertexCount());

		for (int vertex_index = 0; vertex_index < vertices.size(); ++vertex_index){

			vertices[vertex_index] = control_points[polygon_vertices[vertex_index]];

			indices[vertex_index] = vertex_index;	//The index buffer is the trivial one (0,1,2,3,...)

		}

		// Layer elements unroll
		
		for (int l = 0; l < layers.size(); ++l){

			auto & layer = *mesh.GetLayer(l);

			UnrollElement(mesh, layer.GetNormals(), layers[l].normals);
			UnrollElement(mesh, layer.GetBinormals(), layers[l].binormals);
			UnrollElement(mesh, layer.GetTangents(), layers[l].tangents);
			UnrollElement(mesh, layer.GetUVs(), layers[l].uvs);

		}
		
		CommitRemap(mesh, &vertices, &indices, layers);
					
		cout << "success!" << std::endl;

	}

	void ReplacePropertyExtension(FbxProperty property, const string & extension){

		int count = property.GetSrcObjectCount<FbxFileTexture>();

		for (int t = 0; t < count; ++t){

			auto & texture = *property.GetSrcObject<FbxFileTexture>(t);
			
			string texture_name = texture.GetFileName();

			cout << "Replacing " << texture_name << "'s extension" << std::endl;

#ifdef _WIN32

			char ext[_MAX_EXT];

			_splitpath_s(texture_name.c_str(), nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);

			texture_name.erase(texture_name.end() - strnlen_s(ext, _MAX_EXT), texture_name.end());
			
			texture_name.append(extension);

#else

			static_assert(false, "Not supported yet!");

#endif
			
			texture.SetFileName(texture_name.c_str());
								
		}

	}

	/// \brief Extension replacing functor.
	struct ReplaceExtension{

		string extension_;

		ReplaceExtension(string extension) :
			extension_(std::move(extension)){}
		
		/// \brief Replace textures' extensions.
		void operator()(FbxMesh & mesh){

			auto & parent = *mesh.GetNode();

			for (int m = 0; m < parent.GetSrcObjectCount<FbxSurfaceMaterial>(); ++m){

				auto &material = *parent.GetSrcObject<FbxSurfaceMaterial>(m);

				// Standard maps only...
				ReplacePropertyExtension(material.FindProperty(FbxSurfaceMaterial::sEmissive), extension_);
				ReplacePropertyExtension(material.FindProperty(FbxSurfaceMaterial::sAmbient), extension_);
				ReplacePropertyExtension(material.FindProperty(FbxSurfaceMaterial::sDiffuse), extension_);
				ReplacePropertyExtension(material.FindProperty(FbxSurfaceMaterial::sSpecular), extension_);
				ReplacePropertyExtension(material.FindProperty(FbxSurfaceMaterial::sShininess), extension_);
				ReplacePropertyExtension(material.FindProperty(FbxSurfaceMaterial::sBump), extension_);
				ReplacePropertyExtension(material.FindProperty(FbxSurfaceMaterial::sNormalMap), extension_);
				ReplacePropertyExtension(material.FindProperty(FbxSurfaceMaterial::sReflection), extension_);

			}

		}

	};

	// Filters

	template <typename TProcessor, typename... TExtras >
	class MeshFilter{

	public:

		MeshFilter(TProcessor processor) :
			processor_(move(processor)){}

		void operator()(FbxNodeAttribute & attribute, TExtras&&... extras){

			if (attribute.GetAttributeType() == FbxNodeAttribute::EType::eMesh){

				processor_(static_cast<FbxMesh&>(attribute), forward<TExtras>(extras)...);

			}

		}

	private:

		TProcessor processor_;

	};

	template <typename TProcessor, typename... TExtras>
	MeshFilter<TProcessor, TExtras...> filter_by_mesh(TProcessor&& arg, TExtras&&...){

		return MeshFilter<TProcessor, TExtras...>(forward<TProcessor>(arg));
	}

	// Walkers

	template <typename TProcessor, typename... TExtras>
	void ProcessAttributes(FbxNode & fbx_node, TProcessor processor, TExtras&&... extras){

		// Attributes
		for (int attribute_index = 0; attribute_index < fbx_node.GetNodeAttributeCount(); ++attribute_index){

			cout << "#" << attribute_index << ": " << fbx_node.GetName() << std::endl;

			processor(*(fbx_node.GetNodeAttributeByIndex(attribute_index)), forward<TExtras>(extras)...);

		}
		
		// Recursion - Depth-first

		for (int child_index = 0; child_index < fbx_node.GetChildCount(); ++child_index){

			// The instantiated scene node becomes the root of the next level.
			ProcessAttributes(*fbx_node.GetChild(child_index), processor, forward<TExtras>(extras)...);

		}

	}
	
}

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

void FBX::RemapAttributes(FbxScene & scene){

	ProcessAttributes(*scene.GetRootNode(), filter_by_mesh(::RemapAttributes));

}

void FBX::StripExtension(FbxScene & scene, const string & extension){
	
	ProcessAttributes(*scene.GetRootNode(), filter_by_mesh(::ReplaceExtension{ extension }));

}

void FBX::Export(FbxScene & scene, const string & path, bool){

	auto fbx_exporter = FbxExporter::Create(manager_, "");

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