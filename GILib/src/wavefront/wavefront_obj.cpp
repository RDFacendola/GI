#include "wavefront/wavefront_obj.h"

#include <algorithm>
#include <fstream>

#include "eigen.h"
#include "scene.h"
#include "mesh.h"
#include "graphics.h"
#include "core.h"
#include "gilib.h"

using namespace gi_lib;
using namespace gi_lib::wavefront; 

using namespace std;
using namespace Eigen;

namespace {

	const char kVertexPositionToken[] = "v";
	const char kTextureCoordinatesToken[] = "vt";
	const char kVertexNormalsToken[] = "vn";
	const char kPolygonToken[] = "f";
	const char kGroupToken[] = "g";
	const char kUseMaterialToken[] = "usemtl";
	const char kMaterialLibraryToken[] = "mtllib";
	const char kNewMaterialToken[] = "newmtl";


	/// \brief Objects needed while importing.
	struct ImportContext {

		IMtlMaterialImporter* material_importer;

		Resources* resources;

		wstring base_directory;

	};

	class MtlProperty : public IMtlProperty {

	public:

		MtlProperty(const string& name, const string& value);

		virtual string GetName() const override;

		virtual bool Read(float& out) const override;

		virtual bool Read(Vector3f& out) const override;

		virtual bool Read(string& out) const override;

	private:
		
		string name_;

		string value_;

	};

	class MtlMaterial : public IMtlMaterial {

	public:
		
		MtlMaterial(const string& name);

		void AddProperty(const string& property_name, const string& property_value);

		virtual string GetName() const override;

		virtual unique_ptr<IMtlProperty> operator [](const string& property_name) const override;

	private:

		string name_;
			
		std::map<string, string> properties_;

	};

	struct MtlParser {
		
		bool Parse(const wstring& file_name);

		void ParseLine(const string& line);

		void ParseMaterial(istringstream& line_stream);
		
		const MtlMaterial& GetMaterial(const string& material_name) const;

		vector<unique_ptr<MtlMaterial>> materials_;				///< \brief List of the materials inside the library.

	};

	struct ObjParser {

		struct VertexDefinition {

			size_t position_index_;

			size_t texture_coordinates_index_;

			size_t normals_index_;

		};

		ObjParser(const wstring& file_name, Resources& resources);

		ObjParser& operator=(const ObjParser&) = delete;
		
		bool Parse(TransformComponent& root, const ImportContext& context);

		void ParseLine(const string& line, TransformComponent& root, const ImportContext& context);

		void ParseVertexPosition(istringstream& line_stream);

		void ParseTextureCoordinates(istringstream& line_stream);

		void ParseVertexNormals(istringstream& line_stream);

		void ParsePolygon(istringstream& line_stream);

		void ParseGroup(istringstream& line_stream, TransformComponent& root, const ImportContext& context);

		void ParseMaterial(istringstream& line_stream);

		void ParseMaterialLibrary(istringstream& line_stream);
				
		void AppendPolygon(const vector<VertexDefinition>& polygon);
		
		void FinalizeCurrentMesh(TransformComponent& root, const ImportContext& context);

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

	//////////////////////////////////// MTL PROPERTY //////////////////////////////////////////////

	MtlProperty::MtlProperty(const string& name, const string& value) :
		name_(name),
		value_(value){}

	string MtlProperty::GetName() const{

		return name_;

	}

	bool MtlProperty::Read(float& out) const{

		istringstream value_stream(value_);
		
		return static_cast<bool>(value_stream >> out);

	}

	bool MtlProperty::Read(Vector3f& out) const{

		istringstream value_stream(value_);

		return (value_stream >> out(0)) &&
			   (value_stream >> out(1)) &&
			   (value_stream >> out(2));
	}

	bool MtlProperty::Read(string& out) const{

		istringstream value_stream(value_);

		return static_cast<bool>(value_stream >> out);

	}

	//////////////////////////////////// MTL MATERIAL //////////////////////////////////////////////

	MtlMaterial::MtlMaterial(const string& name):
	name_(name){}

	string MtlMaterial::GetName() const {

		return name_;

	}

	unique_ptr<IMtlProperty> MtlMaterial::operator [](const string& property_name) const {

		auto it = properties_.find(property_name);

		return it != properties_.end() ?
			   make_unique<MtlProperty>(property_name, it->second) :
			   nullptr;

	}

	void MtlMaterial::AddProperty(const string& property_name, const string& property_value) {

		properties_.insert(std::make_pair(property_name, property_value));

	}

	//////////////////////////////////// MTL PARSER ////////////////////////////////////////////////

	bool MtlParser::Parse(const wstring& file_name) {
		
		std::ifstream scene_file(file_name.c_str());

		if (scene_file.good()) {

			string line;

			while (getline(scene_file, line)) {

				ParseLine(line);

			}

			return true;

		}

		return false;

	}

	void MtlParser::ParseLine(const string& line) {

		istringstream line_stream(line);

		string token;
		string property_value;

		line_stream >> token;

		if (token == kNewMaterialToken) {

			ParseMaterial(line_stream);

		}
		else if(materials_.size() > 0){

			// Interprets the rest of the string as the property value
			line_stream >> property_value;

			materials_.back()->AddProperty(token, property_value);

		}

	}

	void MtlParser::ParseMaterial(istringstream& line_stream) {

		// Initialize a new material

		string material_name;

		line_stream >> material_name;

		materials_.push_back(make_unique<MtlMaterial>(material_name));

	}

	const MtlMaterial& MtlParser::GetMaterial(const string& material_name) const{

		auto it = std::find_if(materials_.begin(), 
							   materials_.end(), 
							   [&material_name](const unique_ptr<MtlMaterial>& material) {

									return material->GetName() == material_name;

							   });

		static const MtlMaterial kEmpty("Empty");

		return it != materials_.end() ?
			   *(it->get()) :
			   kEmpty;

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
		
		if (vertices.size() == 3) {

			AppendPolygon(vertices);		// 0 1 2
				
		}
		else if (vertices.size() == 4) {

			vector<VertexDefinition> polygon_vertices;

			polygon_vertices.push_back(vertices[0]);
			polygon_vertices.push_back(vertices[1]);
			polygon_vertices.push_back(vertices[2]);

			AppendPolygon(polygon_vertices);

			polygon_vertices.clear();

			polygon_vertices.push_back(vertices[0]);
			polygon_vertices.push_back(vertices[2]);
			polygon_vertices.push_back(vertices[3]);

			AppendPolygon(polygon_vertices);

		}
		else {

			THROW(L"Unsupported polygon topology");

		}

		for (size_t vertex_index = 0; vertex_index < vertices.size() - 2; ++vertex_index) {

			AppendPolygon(vector<VertexDefinition>(vertices.begin() + vertex_index, vertices.begin() + vertex_index + 3));

		}

	}

	void ObjParser::AppendPolygon(const vector<VertexDefinition>& polygon) {

		assert(polygon.size() == 3);		// It's a triangle, right?

		// Compute the tangent and the bitangent vector

		// See - http://www.terathon.com/code/tangent.html

		Vector3f v1 = positions_[polygon[0].position_index_ - 1];
		Vector3f v2 = positions_[polygon[1].position_index_ - 1];
		Vector3f v3 = positions_[polygon[2].position_index_ - 1];

		Vector2f uv1 = texture_coordinates_[polygon[0].texture_coordinates_index_ - 1];
		Vector2f uv2 = texture_coordinates_[polygon[1].texture_coordinates_index_ - 1];
		Vector2f uv3 = texture_coordinates_[polygon[2].texture_coordinates_index_ - 1];

		Vector3f v2v1 = v2 - v1;
		Vector3f v3v1 = v3 - v1;

		Vector2f uv2uv1 = uv2 - uv1;
		Vector2f uv3uv1 = uv3 - uv1;

		Vector3f base_tangent = Vector3f(uv3uv1(1) * v2v1(0) - uv2uv1(1) * v3v1(0), 
									     uv3uv1(1) * v2v1(1) - uv2uv1(1) * v3v1(1), 
									     uv3uv1(1) * v2v1(2) - uv2uv1(1) * v3v1(2));		// Polygon tangent vector aligned with texture's u coordinate.

		//base_tangent /= (uv2uv1(0) * uv3uv1(1) - uv3uv1(0) * uv2uv1(0));

		for (auto&& vertex : polygon) {

			Vector3f normal = normals_[vertex.normals_index_ - 1];

			Vector3f bitangent = normal.cross(base_tangent).normalized();
			Vector3f tangent = bitangent.cross(normal).normalized();

			mesh_.vertices.push_back(VertexFormatNormalTextured{ positions_[vertex.position_index_ - 1],								// Position
																 normal,																// Normals
																 texture_coordinates_[vertex.texture_coordinates_index_ - 1],			// Texture coordinates
																 tangent,																// Tangent
																 bitangent });															// Bitangent

		}

	}

	void ObjParser::ParseGroup(istringstream& line_stream, TransformComponent& root, const ImportContext& context) {

		// Finalize the previous mesh

		FinalizeCurrentMesh(root, context);
		
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

	void ObjParser::FinalizeCurrentMesh(TransformComponent& root, const ImportContext& context) {
		
		if (current_group_.size() > 0) {
			
			// Import a dummy node with identity local transform where the mesh component will be attached

			auto& scene = root.GetComponent<NodeComponent>()->GetScene();

			auto imported_node = scene.CreateNode(to_wstring(current_group_),
												  Translation3f(Vector3f::Zero()),
												  Quaternionf::Identity(),
												  AlignedScaling3f(Vector3f::Ones()));

			imported_node->SetParent(&root);

			// Import the actual mesh

			mesh_.subsets.push_back(MeshSubset{ 0, mesh_.vertices.size() / 3 });

			auto mesh = resources_.Load<IStaticMesh, IStaticMesh::FromVertices<VertexFormatNormalTextured>>(mesh_);

			if (mesh) {

				auto mesh_component = imported_node->AddComponent<MeshComponent>(mesh);

				// Import the material

				context.material_importer->OnImportMaterial(context.base_directory,
															material_library_.GetMaterial(current_material_),
															*mesh_component);

			}

		}

	}

	bool ObjParser::Parse(TransformComponent& root, const ImportContext& context) {

		std::ifstream scene_file(file_name_.c_str());

		if (scene_file.good()) {

			string line;

			while (getline(scene_file, line)) {

				ParseLine(line, root, context);

			}

			FinalizeCurrentMesh(root, context);

			return true;

		}

		return false;

	}

	void ObjParser::ParseLine(const string& line, TransformComponent& root, const ImportContext& context) {

		istringstream line_stream(line);

		string token;

		line_stream >> token;

		if (token == kVertexPositionToken) {

			ParseVertexPosition(line_stream);

		}
		else if (token == kTextureCoordinatesToken) {

			ParseTextureCoordinates(line_stream);

		}
		else if (token == kVertexNormalsToken) {

			ParseVertexNormals(line_stream);

		}
		else if (token == kPolygonToken) {

			ParsePolygon(line_stream);

		}
		else if (token == kGroupToken) {

			ParseGroup(line_stream, root, context);

		}
		else if (token == kUseMaterialToken) {

			ParseMaterial(line_stream);

		}
		else if (token == kMaterialLibraryToken) {

			ParseMaterialLibrary(line_stream);

		}

	}

}

////////////////////////////////// OBJ IMPORTER /////////////////////////////////////

ObjImporter::ObjImporter(IMtlMaterialImporter& importer, Resources& resources) :
	material_importer_(importer),
	resources_(resources) {

}

bool ObjImporter::ImportScene(const wstring& file_name, TransformComponent& root) const{

	ObjParser parser(file_name, resources_);

	// Context setup

	auto& file_system = FileSystem::GetInstance();
	
	ImportContext context{ &material_importer_, 
						   &resources_, 
						   file_system.GetDirectory(file_name) };

	return parser.Parse(root,
						context);

}