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
	const char kFaceToken[] = "f";
	const char kGroupToken[] = "g";
	const char kObjectToken[] = "o";
	const char kUseMaterialToken[] = "usemtl";
	const char kMaterialLibraryToken[] = "mtllib";
	const char kNewMaterialToken[] = "newmtl";
	
	/// \brief Defines a single mesh subset.
	struct Subset {

		string subset_name_;									// Name of the subset.

		string material_name_;									// Name of the material associated to the subset.

		vector<VertexFormatNormalTextured> vertices_;			// List of vertices inside the subset.
		
	};

	/// \brief Defines a single mesh.
	struct Mesh {

		string name_;											// Name of the mesh.

		vector<Subset> subsets_;								// List of subsets inside the mesh.

	};

	/// \brief Defines a single property inside a mtl material.
	/// \author Raffaele D. Facendola
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

	/// \brief Defines a single material inside a mtl material library.
	/// \author Raffaele D. Facendola
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

	/// \brief This object is used to parse a Wavefront .mtl material library.
	/// \author Raffaele D. Facendola
	class MtlParser {
		
	public:

		bool Parse(const wstring& file_name);

		const MtlMaterial* GetMaterial(const string& material_name) const;

	private:

		void ParseLine(const string& line);

		void ParseMaterial(istringstream& line_stream);
		
		vector<unique_ptr<MtlMaterial>> materials_;				///< \brief List of the materials inside the library.

	};

	/// \brief This object is used to parse a Wavefront .obj scene.
	/// \author Raffaele D. Facendola
	class ObjParser {
		
	public:

		/// \brief Default constructor.
		ObjParser();

		/// \brief No assignment operator.
		ObjParser& operator=(const ObjParser&) = delete;
		
		/// \brief Default destructor.
		~ObjParser();

		/// \brief Parse the specified file.
		/// \param file_name The name of the file to parse.
		/// \return Returns true if the parsing succeeded, returns false otherwise.
		bool Parse(const wstring& file_name);

		/// \brief Get an object definition by name.
		/// \param object_name Name of the object to get.
		/// \param mesh If the method succeeds this parameter contains the imported mesh definition. Output.
		/// \return Returns true if the method succeeds, returns false otherwise.
		bool GetMesh(const string& object_name, Mesh& mesh) const;

		/// \brief Get an object definition by index.
		/// \param index Inde of the mesh to get.
		/// \return Return the index-th parsed mesh.
		bool GetMesh(size_t index, Mesh& mesh) const;
		
		/// \brief Get a material definition by name.
		/// \return Returns a pointer to the material definition matching the specified name if any, returns nullptr otherwise.
		const IMtlMaterial* GetMaterial(const string& material_name) const;

		/// \brief Get the number of parsed objects.
		/// \return Returns the number of parsed objects.
		size_t GetObjectCount() const;

	private:
		/// \brief Definition of a vertex.
		struct VertexDefinition {

			size_t position_index_;							///< \brief Index of the vertex position. 1 based.

			size_t texture_coordinates_index_;				///< \brief Index of the texture coordinates. 1 based.

			size_t normals_index_;							///< \brief Index of the normals. 1 based.

		};

		/// \brief Definition of a group.
		struct GroupDefinition {

			string group_name_;								///< \brief Group name.

			string material_name_;							///< \brief Name of the material associated to the group.

			vector<VertexDefinition> vertices_;				///< \brief List of vertices inside the group. Topology: triangle list.

		};

		/// \brief Definition of an object.
		struct ObjectDefinition {

			string object_name_;								///< \brief Mesh name.

			vector<GroupDefinition> groups_;					///< \brief List of groups (subsets) inside the mesh.
					
		};
			
		/// \brief Get an object definition by name.
		const ObjectDefinition* GetObject(const string& object_name) const;
			
		/// \brief Get a static mesh by object definition.
		void GetMesh(const ObjectDefinition& object_definition, Mesh& mesh) const;

		/// \brief Append a polygon to an existing subset.
		void AppendPolygon(const VertexDefinition& a, const VertexDefinition& b, const VertexDefinition& c, Subset& subset) const;

		/// \brief Clear the current status of the parser.
		void Clear();

		/// \brief Parse a line.
		void ParseLine(const string& line, const wstring& file_name);

		/// \brief Parse a vertex position element.
		void ParseVertexPosition(istringstream& line_stream);
		
		/// \brief Parse vertex texture coordinates element.
		void ParseTextureCoordinates(istringstream& line_stream);

		/// \brief Parse a vertex normal element.
		void ParseVertexNormals(istringstream& line_stream);

		/// \brief Parse a face element.
		void ParseFace(istringstream& line_stream);

		/// \brief Parse a group element.
		void ParseGroup(istringstream& line_stream);
	
		/// \brief Parse an object element.
		void ParseObject(istringstream& line_stream);

		/// \brief Parse a use-material directive
		void ParseUseMaterial(istringstream& line_stream);

		/// \brief Parse a material-library directive
		void ParseMaterialLibrary(istringstream& line_stream, const wstring& file_name);

		/// \brief Get a reference to the current group element.
		GroupDefinition& GetCurrentGroup();

		/// \brief Get a reference to the current object element.
		ObjectDefinition& GetCurrentObject();

		vector<unique_ptr<MtlParser>> material_libraries_;				///< \brief Material libraries used to resolve the object materials.

		vector<unique_ptr<ObjectDefinition>> objects_;					///< \brief List of objects.

		vector<Vector3f> positions_;									///< \brief List of vertices positions.

		vector<Vector2f> texture_coordinates_;							///< \brief List of texture coordinates.

		vector<Vector3f> normals_;										///< \brief List of vertices normals.
				
	};

	/// \brief Import an wavefront mesh definition as static mesh.
	/// \param mesh_definition The mesh definition to import.
	/// \param resources Object used to create the actual static mesh.
	ObjectPtr<IStaticMesh> ImportStaticMesh(const Mesh& mesh_definition, Resources& resources) {
	
		IStaticMesh::FromVertices<VertexFormatNormalTextured> bundle;

		bundle.indices.clear();		// Unindexed mesh
		
		for (auto&& subset : mesh_definition.subsets_) {

			bundle.subsets.push_back(MeshSubset{ bundle.vertices.size(), subset.vertices_.size() / 3 });

			bundle.vertices.insert(bundle.vertices.end(),
								   subset.vertices_.begin(),
								   subset.vertices_.end());
			
		}

		return resources.Load<IStaticMesh, IStaticMesh::FromVertices<VertexFormatNormalTextured>>(bundle);
				
	}

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

	const MtlMaterial* MtlParser::GetMaterial(const string& material_name) const{

		auto it = std::find_if(materials_.begin(), 
							   materials_.end(), 
							   [&material_name](const unique_ptr<MtlMaterial>& material) {

									return material->GetName() == material_name;

							   });

		return it != materials_.end() ?
			   it->get() :
			   nullptr;

	}

	//////////////////////////////////// OBJ PARSER ////////////////////////////////////////////////

	ObjParser::ObjParser() {}
		
	ObjParser::~ObjParser() {

		Clear();

	}

	void ObjParser::Clear() {

		material_libraries_.clear();
		objects_.clear();
		positions_.clear();
		texture_coordinates_.clear();
		normals_.clear();

	}

	bool ObjParser::Parse(const wstring& file_name) {

		Clear();

		std::ifstream file_content(file_name);

		if (file_content.good()) {

			string line;

			while (getline(file_content, line)) {

				ParseLine(line, file_name);

			}
			
			return true;

		}

		return false;

	}

	void ObjParser::ParseLine(const string& line, const wstring& file_name) {

		istringstream line_stream(line);

		string token;

		line_stream >> token;

		if (token == kVertexPositionToken) {				// v

			ParseVertexPosition(line_stream);

		}
		else if (token == kTextureCoordinatesToken) {		// vt

			ParseTextureCoordinates(line_stream);

		}
		else if (token == kVertexNormalsToken) {			// vn

			ParseVertexNormals(line_stream);

		}
		else if (token == kFaceToken) {						// f

			ParseFace(line_stream);

		}
		else if (token == kGroupToken){						// g

			ParseGroup(line_stream);

		}
		else if (token == kObjectToken) {					// o

			ParseObject(line_stream);

		}
		else if (token == kUseMaterialToken) {				// usemtl

			ParseUseMaterial(line_stream);

		}
		else if (token == kMaterialLibraryToken) {			// mtllib

			ParseMaterialLibrary(line_stream, file_name);

		}

	}
	
	ObjParser::GroupDefinition& ObjParser::GetCurrentGroup() {
				
		ObjectDefinition& object = GetCurrentObject();

		// Group

		if (object.groups_.size() == 0) {

			object.groups_.push_back(GroupDefinition{});			// Add a new empty group definition if none was defined.

		}

		return object.groups_.back();								// Returns the last group of the last object.

	}

	ObjParser::ObjectDefinition& ObjParser::GetCurrentObject() {

		// Object

		if (objects_.size() == 0) {

			objects_.push_back(std::make_unique<ObjectDefinition>());		// Add a new empty object definition if none was defined.

		}

		return *objects_.back();

	}

	void ObjParser::ParseVertexPosition(istringstream& line_stream) {

		Vector3f position;

		line_stream >> position(0);		// X
		line_stream >> position(1);		// Y
		line_stream >> position(2);		// Z

		// Ignore the W coordinate (which should always be 1)

		positions_.push_back(position);
			
	}

	void ObjParser::ParseTextureCoordinates(istringstream& line_stream) {

		Vector2f tex_coords;

		line_stream >> tex_coords(0);		// U coordinate
		line_stream >> tex_coords(1);		// V coordinate

		// The W coordinate is not currently supported.

		texture_coordinates_.push_back(tex_coords);

	}

	void ObjParser::ParseVertexNormals(istringstream& line_stream) {
		
		Vector3f normal;

		line_stream >> normal(0);		// X direction
		line_stream >> normal(1);		// Y direction
		line_stream >> normal(2);		// Z direction

		normals_.push_back(normal.normalized());

	}

	void ObjParser::ParseFace(istringstream& line_stream) {
						
		string vertex_definition;	// Vertex definition in the form "v/vt/vn" or "v//vn"

		vector<VertexDefinition> vertices;

		while (line_stream >> vertex_definition) {

			VertexDefinition vertex;

			// This is unsafe and ugly: deal with it

			size_t* destination = &vertex.position_index_;		

			for (auto&& item : Split(vertex_definition, '/')) {

				istringstream(item) >> *destination;

				++destination;

			}

			vertices.push_back(vertex);

		}

		// Unrolls the vertex strip to produce only triangles
		
		GroupDefinition& group = GetCurrentGroup();

		if (vertices.size() == 3) {

			// Triangle

			group.vertices_.push_back(vertices[0]);
			group.vertices_.push_back(vertices[1]);
			group.vertices_.push_back(vertices[2]);

		}
		else if (vertices.size() == 4) {

			// Quad

			group.vertices_.push_back(vertices[0]);
			group.vertices_.push_back(vertices[1]);
			group.vertices_.push_back(vertices[2]);

			group.vertices_.push_back(vertices[0]);
			group.vertices_.push_back(vertices[2]);
			group.vertices_.push_back(vertices[3]);

		}
		else {

			// Some other topology we don't really care about

			THROW(L"Unsupported polygon topology");

		}

	}
	
	void ObjParser::ParseGroup(istringstream& line_stream) {
				
		// Create a new group in the current object

		ObjectDefinition& object = GetCurrentObject();

		object.groups_.push_back(GroupDefinition{});

		line_stream >> object.groups_.back().group_name_;
		
	}

	void ObjParser::ParseObject(istringstream& line_stream) {

		// Create a new named object

		objects_.push_back(std::make_unique<ObjectDefinition>());

		auto& object = *objects_.back();

		line_stream >> object.object_name_;

	}

	void ObjParser::ParseUseMaterial(istringstream& line_stream) {

		auto& group = GetCurrentGroup();

		line_stream >> group.material_name_;

	}

	void ObjParser::ParseMaterialLibrary(istringstream& line_stream, const wstring& file_name) {

		string library_name;

		line_stream >> library_name;

		auto& file_system = FileSystem::GetInstance();

		material_libraries_.push_back(std::make_unique<MtlParser>());

		material_libraries_.back()->Parse(file_system.GetDirectory(file_name) + to_wstring(library_name));

	}
	
	bool ObjParser::GetMesh(const string& object_name, Mesh& mesh) const {

		auto object_definition = GetObject(object_name);

		if (!object_definition) {

			return false;

		}

		GetMesh(*object_definition, mesh);
		
		return true;

	}

	void ObjParser::AppendPolygon(const VertexDefinition& a, const VertexDefinition& b, const VertexDefinition& c, Subset& subset) const {

		VertexDefinition polygon[] = { a, b, c };

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

			subset.vertices_.push_back(VertexFormatNormalTextured{ positions_[vertex.position_index_ - 1],								// Position
																   normal,																// Normals
																   texture_coordinates_[vertex.texture_coordinates_index_ - 1],			// Texture coordinates
																   tangent,																// Tangent
																   bitangent });														// Bitangent

		}

	}

	const ObjParser::ObjectDefinition* ObjParser::GetObject(const string& object_name) const {

		auto it = std::find_if(objects_.begin(),
							   objects_.end(),
							   [&object_name](const unique_ptr<ObjectDefinition>& object) {

									return object->object_name_ == object_name;

							   });

		return it != objects_.end() ?
			   it->get() :
			   nullptr;

	}
	
	bool ObjParser::GetMesh(size_t index, Mesh& mesh) const {

		if (index < 0 ||
			index >= objects_.size()) {

			return false;

		}

		GetMesh(*objects_[index], mesh);

		return true;
		
	}

	void ObjParser::GetMesh(const ObjectDefinition& object_definition, Mesh& mesh) const {

		// Import each group as named subset

		mesh.name_ = object_definition.object_name_;
		mesh.subsets_.clear();

		Subset subset;
		VertexFormatNormalTextured subset_vertex;

		for (auto&& group : object_definition.groups_) {

			subset.subset_name_ = group.group_name_;
			subset.material_name_ = group.material_name_;

			// Resolve vertex definitions

			subset.vertices_.clear();

			for (size_t index = 0; index < group.vertices_.size(); index += 3) {

				AppendPolygon(group.vertices_[index + 0],
							  group.vertices_[index + 1],
							  group.vertices_[index + 2],
							  subset);

			}

			// Add the new subset

			mesh.subsets_.push_back(std::move(subset));

		}
		
	}

	size_t ObjParser::GetObjectCount() const {

		return objects_.size();

	}

	const IMtlMaterial* ObjParser::GetMaterial(const string& material_name) const{

		const IMtlMaterial* material;

		for (auto&& library : material_libraries_) {

			material = library->GetMaterial(material_name);

			if (material) {

				return material;

			}

		}

		return nullptr;

	}

}

////////////////////////////////// OBJ IMPORTER /////////////////////////////////////

ObjImporter::ObjImporter(Resources& resources) :
	resources_(resources) {}

bool ObjImporter::ImportScene(const wstring& file_name, TransformComponent& root, IMtlMaterialImporter& material_importer) const{

	ObjParser parser;

	if (!parser.Parse(file_name)) {

		return false;

	}

	// Create the actual meshes, materials and hierarchy
	
	auto base_directory = FileSystem::GetInstance().GetDirectory(file_name);

	auto& scene = root.GetComponent<NodeComponent>()->GetScene();
	
	Mesh mesh;

	MtlMaterialCollection material_collection;

	for (size_t index = 0; index < parser.GetObjectCount(); ++index) {

		parser.GetMesh(index, mesh);

		// Node definition and hierarchy

		auto node = scene.CreateNode(to_wstring(mesh.name_),
									 Translation3f(Vector3f::Zero()),
									 Quaternionf::Identity(),
									 AlignedScaling3f(Vector3f::Ones()));

		node->SetParent(&root);
		
		// Mesh import

		auto mesh_component = node->AddComponent<MeshComponent>(ImportStaticMesh(mesh, resources_));

		// Material collection import

		material_collection.reserve(mesh.subsets_.size());
		material_collection.clear();

		for (auto&& subset : mesh.subsets_) {

			material_collection.push_back(parser.GetMaterial(subset.material_name_));

		}

		material_importer.OnImportMaterial(base_directory, 
										   material_collection, 
										   *mesh_component);
		
	}

	return false;

}

ObjectPtr<IStaticMesh> ObjImporter::ImportMesh(const wstring& file_name, const string& mesh_name) const {

	ObjParser parser;

	if (!parser.Parse(file_name)) {

		return nullptr;

	}

	// Import just the static mesh

	Mesh mesh_definition;

	if (parser.GetMesh(mesh_name, mesh_definition)) {

		return ImportStaticMesh(mesh_definition, resources_);

	}
	else {

		return nullptr;

	}

}