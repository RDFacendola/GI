#include "wavefront/wavefront_obj.h"

#include <algorithm>
#include <fstream>

#include "eigen.h"
#include "scene.h"
#include "mesh.h"
#include "graphics.h"
#include "core.h"


using namespace gi_lib;
using namespace gi_lib::wavefront; 

using namespace std;
using namespace Eigen;

namespace {

	vector<string> Split(const string& source, char delimiter) {

		std::stringstream source_stream(source);
		
		std::string item;

		vector<string> elements;

		while (std::getline(source_stream, item, delimiter)) {
			
			elements.push_back(item);

		}

		return elements;

	}

	struct MtlParser {
		
		bool Parse(const wstring& file_name);

		void ParseLine(const string& line);

		void ParseMaterial(istringstream& line_stream);

		void FinalizeCurrentMaterial();

		string current_material_;										///< \brief Current material name.

	};

	struct ObjParser {

		struct VertexDefinition {

			size_t position_index_;

			size_t texture_coordinates_index_;

			size_t normals_index_;

		};

		ObjParser(const wstring& file_name, Resources& resources);

		ObjParser& operator=(const ObjParser&) = delete;
		
		bool Parse(TransformComponent& root);

		void ParseLine(const string& line, TransformComponent& root);

		void ParseVertexPosition(istringstream& line_stream);

		void ParseTextureCoordinates(istringstream& line_stream);

		void ParseVertexNormals(istringstream& line_stream);

		void ParsePolygon(istringstream& line_stream);

		void ParseGroup(istringstream& line_stream, TransformComponent& root);

		void ParseMaterial(istringstream& line_stream);

		void ParseMaterialLibrary(istringstream& line_stream);
				
		void AppendPolygon(const vector<VertexDefinition>& polygon);
		
		void FinalizeCurrentMesh(TransformComponent& root);

		Resources& resources_;

		MtlParser material_library_;

		wstring file_name_;												///< \brief Name of the file being parsed.

		vector<Vector3f> positions_;									///< \brief List of vertices positions.

		vector<Vector2f> texture_coordinates_;							///< \brief List of texture coordinates.

		vector<Vector3f> normals_;										///< \brief List of vertices normals.

		IStaticMesh::FromVertices<VertexFormatNormalTextured> mesh_;	///< \brief Current mesh

		string current_group_;											///< \brief Current group name (ie. mesh name).

		string current_material_;										///< \brief Current material name.

	};

	//////////////////////////////////// MTL PARSER ////////////////////////////////////////////////

	bool MtlParser::Parse(const wstring& file_name) {
		
		std::ifstream scene_file(file_name.c_str());

		if (scene_file.good()) {

			string line;

			while (getline(scene_file, line)) {

				ParseLine(line);

			}

			FinalizeCurrentMaterial();

			return true;

		}

		return false;

	}

	void MtlParser::ParseLine(const string& line) {

		istringstream line_stream(line);

		string token;

		line_stream >> token;

		if (token == "newmtl") {

			ParseMaterial(line_stream);

		}
		else if (token == "vt") {

			

		}

	}

	void MtlParser::ParseMaterial(istringstream& line_stream) {

		// Finalize the previous material

		FinalizeCurrentMaterial();

		// Initialize the new mesh

		line_stream >> current_material_;

	}

	void MtlParser::FinalizeCurrentMaterial() {


	}

	//////////////////////////////////// OBJ PARSER ////////////////////////////////////////////////

	ObjParser::ObjParser(const wstring& file_name, Resources& resources) :
		file_name_(file_name),
		resources_(resources){
	
	}
		
	void ObjParser::ParseVertexPosition(istringstream& line_stream) {

		// v x y z [w]

		Vector3f position;

		line_stream >> position(0);
		line_stream >> position(1);
		line_stream >> position(2);

		positions_.push_back(position);
			
	}

	void ObjParser::ParseTextureCoordinates(istringstream& line_stream) {

		// vt u v [w]

		Vector2f tex_coords;

		line_stream >> tex_coords(0);
		line_stream >> tex_coords(1);

		texture_coordinates_.push_back(tex_coords);

	}

	void ObjParser::ParseVertexNormals(istringstream& line_stream) {

		// vn x y z

		Vector3f normal;

		line_stream >> normal(0);
		line_stream >> normal(1);
		line_stream >> normal(2);

		normals_.push_back(normal);

	}

	void ObjParser::ParsePolygon(istringstream& line_stream) {
				
		vector<VertexDefinition> vertices;

		string vertex_definition;	// v/vt/vn <or> v//vn

		while (line_stream >> vertex_definition) {

			VertexDefinition vertex;

			// This is unsafe and ugly: deal with it

			size_t* destination = &vertex.position_index_;		

			for (auto&& item : Split(vertex_definition, '/')) {

				assert(item.size() > 0);					// Texture coordinates are optional but we do not support that, okay?

				istringstream(item) >> *destination;

				++destination;

			}

			vertices.push_back(vertex);

		}

		// Unrolls the vertex strip to produce only triangles
		
		for (size_t vertex_index = 0; vertex_index < vertices.size() - 2; ++vertex_index) {

			AppendPolygon(vector<VertexDefinition>(vertices.begin() + vertex_index, vertices.begin() + vertex_index + 3));

		}

	}

	void ObjParser::AppendPolygon(const vector<VertexDefinition>& polygon) {

		assert(polygon.size() == 3);		// It's a triangle, right?

		// TODO: calculate the actual tangent and binormal vector

		for (auto&& vertex : polygon) {

			mesh_.vertices.push_back(VertexFormatNormalTextured{ positions_[vertex.position_index_ - 1],								// Position
																 normals_[vertex.normals_index_ - 1],									// Normals
																 texture_coordinates_[vertex.texture_coordinates_index_ - 1],			// Texture coordinates
																 Vector3f::Zero(),														// Tangent
																 Vector3f::Zero() });													// Binormal

		}

	}

	void ObjParser::ParseGroup(istringstream& line_stream, TransformComponent& root) {

		// Finalize the previous mesh

		FinalizeCurrentMesh(root);
		
		// Initialize the new mesh

		line_stream >> current_group_;

		mesh_.indices.clear();
		mesh_.vertices.clear();
		mesh_.subsets.clear();

	}

	void ObjParser::ParseMaterial(istringstream& line_stream) {

		line_stream >> current_material_;

	}

	void ObjParser::ParseMaterialLibrary(istringstream& line_stream) {

		string library_name;

		line_stream >> library_name;

		auto& file_system = FileSystem::GetInstance();

		material_library_.Parse(file_system.GetDirectory(file_name_) + to_wstring(library_name));

	}

	void ObjParser::FinalizeCurrentMesh(TransformComponent& root) {
		
		if (current_group_.size() > 0) {

			// One huge subset

			mesh_.subsets.push_back(MeshSubset{ 0, mesh_.vertices.size() / 3 });

			auto mesh = resources_.Load<IStaticMesh, IStaticMesh::FromVertices<VertexFormatNormalTextured>>(mesh_);

			if (mesh) {

				root.AddComponent<MeshComponent>(mesh);

			}

			// TODO: Import the proper material

		}

	}

	bool ObjParser::Parse(TransformComponent& root) {

		std::ifstream scene_file(file_name_.c_str());

		if (scene_file.good()) {

			string line;

			while (getline(scene_file, line)) {

				ParseLine(line, root);

			}

			FinalizeCurrentMesh(root);

			return true;

		}

		return false;

	}

	void ObjParser::ParseLine(const string& line, TransformComponent& root) {

		istringstream line_stream(line);

		string token;

		line_stream >> token;

		if (token == "v") {

			ParseVertexPosition(line_stream);

		}
		else if (token == "vt") {

			ParseTextureCoordinates(line_stream);

		}
		else if (token == "vn") {

			ParseVertexNormals(line_stream);

		}
		else if (token == "f") {

			ParsePolygon(line_stream);

		}
		else if (token == "g") {

			ParseGroup(line_stream, root);

		}
		else if (token == "usemtl") {

			ParseMaterial(line_stream);

		}
		else if (token == "mtllib") {

			ParseMaterialLibrary(line_stream);

		}

	}

}

////////////////////////////////// OBJ IMPORTER /////////////////////////////////////

ObjImporter::ObjImporter(Resources& resources) :
	resources_(resources) {

}

bool ObjImporter::ImportScene(const wstring& file_name, TransformComponent& root) const{

	ObjParser parser(file_name, resources_);

	return parser.Parse(root);

}