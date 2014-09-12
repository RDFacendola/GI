#pragma comment(lib,"libfbxsdk-md.lib")

#ifdef _DEBUG

#pragma comment(lib, "wininet.lib")

#endif

#include <exception>
#include <string>
#include <sstream>
#include <utility>
#include <iostream>

#include "..\include\fbx.h"

using namespace ::gi_lib;
using namespace ::std;

namespace{

	const double epsilon = 2.0 / 128.0;
	const double theta_epsilon = 3.141592653 / 6;

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

	template <typename T>
	bool DotEquals(const T & left, const T & right);

	template <> bool DotEquals<FbxVector4>(const FbxVector4 & left, const FbxVector4 & right){

		auto left_len = sqrt(left[0] * left[0] + left[1] * left[1] + left[2] * left[2] + left[3] * left[3]);
		auto right_len = sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2] + right[3] * right[3]);

		auto dot = (left[0] * right[0] + left[1] * right[1] + left[2] * right[2] + left[3] * right[3]);
		auto len = left_len * right_len;

		auto norm_dot = dot / len;
		auto cos_theta = cos(theta_epsilon);

		return norm_dot > cos_theta;

	}

	/// \brief Get an element from a FbxLayerElementTemplate by index.
	template <typename TType>
	TType GetFbxArrayElement(const FbxLayerElementTemplate<TType> & elements, int index){

		auto & direct = elements.GetDirectArray();

		if (elements.GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect){

			return direct[index];

		}
		else{

			auto & indices = elements.GetIndexArray();

			return direct[indices[index]];

		}
		
	}

	/// \brief Rolls a layer element and change the mapping mode to "byControlPoint".

	/// If the conversion fails nothing is changed.
	/// \return Returns true if the roll succeeded, false otherwise.
	template <class TType, typename TComparer>
	bool Roll(FbxMesh & mesh, FbxLayerElementTemplate<TType> * elements_ptr, TComparer compare){

		if (elements_ptr == nullptr){

			return true;	//The element does not exist.

		}

		auto & elements = *elements_ptr;

		vector<TType> dst(mesh.GetControlPointsCount());

		vector<bool> flag(mesh.GetControlPointsCount());

		std::fill(flag.begin(), flag.end(), false);

		switch (elements.GetMappingMode()){

		case FbxLayerElement::EMappingMode::eByControlPoint:

			return true;

		case FbxLayerElement::EMappingMode::eByPolygonVertex:{

			auto count = mesh.GetPolygonVertexCount();

			int * index_buffer = mesh.GetPolygonVertices();

			int vertex_index;

			// One element for each index
			for (int index = 0; index < count; index++){

				vertex_index = index_buffer[index];

				if (!flag[vertex_index]){

					dst[vertex_index] = GetFbxArrayElement(elements, index);
					flag[vertex_index] = true;	//Written

				}
				else if (!compare(GetFbxArrayElement(elements, index), dst[vertex_index])){

					return false;	// The same vertex is associated to different attribute's value.

				}

			}

			// rolled contains the rolled informations. Resize and remap the elements array.

			elements.SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
			elements.SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);

			elements.GetIndexArray().Clear();									//Not needed anymore.

			elements.GetDirectArray().Resize(static_cast<int>(dst.size()));		//Resize the attribute buffer

			auto * buffer = static_cast<TType *>(elements.GetDirectArray().GetLocked(FbxLayerElementArray::ELockMode::eReadLock));

			std::copy(dst.begin(),
				dst.end(),
				&buffer[0]);

			elements.GetDirectArray().ReadUnlock();

			// Done!

			return true;
			
		}

		case FbxLayerElement::EMappingMode::eNone:

			return true;	//The attribute does not exist.
			
		default:

			return false;

		}
				
	}

	template <class TType>
	bool Unroll(FbxMesh & mesh, FbxLayerElementTemplate<TType> * elements_ptr){

		if (elements_ptr == nullptr){

			return true;	//The element does not exist.

		}

		auto & elements = *elements_ptr;

		vector<TType> dst(mesh.GetPolygonVertexCount());

		switch (elements.GetMappingMode()){

		case FbxLayerElement::EMappingMode::eByControlPoint:
		{

			//Unroll the attribute

			auto count = mesh.GetPolygonVertexCount();

			int * index_buffer = mesh.GetPolygonVertices();

			int vertex_index;

			for (int index = 0; index < count; index++){

				vertex_index = index_buffer[index];

				dst[index] = GetFbxArrayElement(elements, vertex_index);

			}

			// Resize and remap the elements array.

			elements.SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);

			elements.GetIndexArray().Clear();									//Not needed anymore.

			elements.GetDirectArray().Resize(static_cast<int>(dst.size()));		//Resize the attribute buffer

			auto * buffer = static_cast<TType *>(elements.GetDirectArray().GetLocked(FbxLayerElementArray::ELockMode::eReadLock));

			std::copy(dst.begin(),
				dst.end(),
				buffer);

			elements.GetDirectArray().ReadUnlock();

			// Done!

			return true;

		}
		case FbxLayerElement::EMappingMode::eByPolygonVertex:{

			//Change the mapping mode only

			elements.SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);

			return true;

		}

		case FbxLayerElement::EMappingMode::eNone:

			return true;	//The attribute does not exist.

		default:

			return false;

		}

	}

	void UnrollVertices(FbxMesh & mesh){

		vector<FbxVector4> vertices(mesh.GetPolygonVertexCount());

		auto control_points = mesh.GetControlPoints();
		auto polygon_vertices = mesh.GetPolygonVertices();

		// Unroll the vertex buffer

		for (int i = 0; i < mesh.GetPolygonVertexCount(); i++){

			vertices[i] = control_points[polygon_vertices[i]];

		}

		// Replace the old vertex buffer

		mesh.InitControlPoints(static_cast<int>(vertices.size()));

		control_points = mesh.GetControlPoints();

		std::copy(vertices.begin(),
			vertices.end(),
			control_points);

		// Delete the index buffer
		
		// Ok let's just put as 1-2-3-blah
		for (int i = 0; i < mesh.GetPolygonVertexCount(); i++){

			polygon_vertices[i] = i;

		}

	}

	// Processors
	void RollAttributes(FbxMesh & mesh){

		for (int layer_index = 0; layer_index < mesh.GetLayerCount(); ++layer_index){

			auto & layer = *mesh.GetLayer(layer_index);

			cout << "Layer " << layer_index << std::endl;

			cout << "Processing normals..." << (Roll(mesh, layer.GetNormals(), DotEquals<FbxVector4>) ? "done" : "fail") << std::endl;
			cout << "Processing binormals..." << (Roll(mesh, layer.GetBinormals(), DotEquals<FbxVector4>) ? "done" : "fail") << std::endl;
			cout << "Processing tangents..." << (Roll(mesh, layer.GetTangents(), DotEquals<FbxVector4>) ? "done" : "fail") << std::endl;
			cout << "Processing uvs..." << (Roll(mesh, layer.GetUVs(), Equals<FbxVector2>) ? "done" : "fail") << std::endl;
			
			cout << std::endl;

		}

	}

	void UnrollAttributes(FbxMesh & mesh){

		if (mesh.GetPolygonVertices() == nullptr){

			cout << "The mesh is already non-indexed" << std::endl;

			return;

		}

		for (int layer_index = 0; layer_index < mesh.GetLayerCount(); ++layer_index){

			auto & layer = *mesh.GetLayer(layer_index);

			cout << "Layer " << layer_index << std::endl;

			cout << "Processing normals..." << (Unroll(mesh, layer.GetNormals()) ? "done" : "fail") << std::endl;
			cout << "Processing binormals..." << (Unroll(mesh, layer.GetBinormals()) ? "done" : "fail") << std::endl;
			cout << "Processing tangents..." << (Unroll(mesh, layer.GetTangents()) ? "done" : "fail") << std::endl;
			cout << "Processing uvs..." << (Unroll(mesh, layer.GetUVs()) ? "done" : "fail") << std::endl;

			cout << std::endl;

		}

		//Discards the index buffer and unrolls the vertices.
		cout << "Unrolling vertices..." << std::endl;

		UnrollVertices(mesh);
		
	}

	// Filters

	template <typename TProcessor>
	class MeshFilter{

	public:

		MeshFilter(TProcessor processor) :
			processor_(move(processor)){}

		void operator()(FbxNodeAttribute & attribute){

			if (attribute.GetAttributeType() == FbxNodeAttribute::EType::eMesh){

				processor_(static_cast<FbxMesh&>(attribute));

			}

		}

	private:

		TProcessor processor_;

	};

	template <typename TProcessor>
	MeshFilter<TProcessor> filter_by_mesh(TProcessor&& arg){

		return MeshFilter<TProcessor>(forward<TProcessor>(arg));

	}

	// Walkers

	template <typename TProcessor>
	void ProcessAttributes(FbxNode & fbx_node, TProcessor processor){

		cout << "Processing " << fbx_node.GetName() << std::endl;

		// Attributes
		for (int attribute_index = 0; attribute_index < fbx_node.GetNodeAttributeCount(); ++attribute_index){

			cout << "Attribute " << attribute_index << std::endl;

			processor(*(fbx_node.GetNodeAttributeByIndex(attribute_index)));
			
			cout << std::endl;

		}

		cout << std::endl;

		// Recursion - Depth-first

		for (int child_index = 0; child_index < fbx_node.GetChildCount(); ++child_index){

			// The instantiated scene node becomes the root of the next level.
			ProcessAttributes(*fbx_node.GetChild(child_index), processor);

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

void FBX::RollAttributes(FbxScene & scene){

	ProcessAttributes(*scene.GetRootNode(), filter_by_mesh(::RollAttributes));

}

void FBX::UnrollAttributes(FbxScene & scene){

	ProcessAttributes(*scene.GetRootNode(), filter_by_mesh(::UnrollAttributes));

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