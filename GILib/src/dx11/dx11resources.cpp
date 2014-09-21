#pragma comment(lib,"DirectXTK")
#pragma comment(lib,"DirectXTex")
#pragma comment(lib,"Effects11")

#include "dx11resources.h"

#include <DDSTextureLoader.h>
#include <DirectXTex.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <Eigen/Core>

#include <math.h>

#include "..\..\include\core.h"
#include "..\..\include\exceptions.h"
#include "..\..\include\functional.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace DirectX;

/// Throws if the compilation miserably failed...
#define THROW_ON_COMPILE_FAIL(expr, blob) do{ \
								HRESULT hr = expr; \
								if(FAILED(hr)) { \
									std::wstringstream stream; \
									stream << L"\"" << #expr << "\" failed with 0x" << std::hex << hr << std::dec << std::endl \
										   << __FILE__ << std::endl \
										   << __FUNCTION__ << L" @ " << __LINE__ << std::endl; \
									std::wstring error_string(blob != nullptr ? static_cast<wchar_t *>(blob->GetBufferPointer()) : L""); \
									if(blob != nullptr) blob->Release(); \
									throw RuntimeException(stream.str(), {{L"error_code", std::to_wstring(hr)}, {L"compiler error", error_string}}); \
								} \
							}WHILE0

namespace{

	/// \brief Ratio between a Bit and a Byte size.
	const float kBitOverByte = 1.0f / 8.0f;

	/// \brief Size ration between two consecutive MIP levels of a texture 2D.
	const float kMIPRatio2D = 1.0f / 4.0f;
	
	/// \brief Convert a resource priority to an eviction priority
	unsigned int ResourcePriorityToEvictionPriority(ResourcePriority priority){

		switch (priority){

		case ResourcePriority::MINIMUM:			return DXGI_RESOURCE_PRIORITY_MINIMUM;
		case ResourcePriority::LOW:				return DXGI_RESOURCE_PRIORITY_LOW;
		case ResourcePriority::NORMAL:			return DXGI_RESOURCE_PRIORITY_NORMAL;
		case ResourcePriority::HIGH:			return DXGI_RESOURCE_PRIORITY_HIGH;
		case ResourcePriority::CRITICAL:		return DXGI_RESOURCE_PRIORITY_MAXIMUM;

		}

		throw RuntimeException(L"Unrecognized priority level.");

	}

	/// \brief Convert a resource priority to an eviction priority (DirectX11)
	ResourcePriority EvictionPriorityToResourcePriority(unsigned int priority){

		switch (priority){

		case DXGI_RESOURCE_PRIORITY_MINIMUM:	return ResourcePriority::MINIMUM;
		case DXGI_RESOURCE_PRIORITY_LOW:		return ResourcePriority::LOW;
		case DXGI_RESOURCE_PRIORITY_NORMAL:		return ResourcePriority::NORMAL;
		case DXGI_RESOURCE_PRIORITY_HIGH:		return ResourcePriority::HIGH;
		case DXGI_RESOURCE_PRIORITY_MAXIMUM:	return ResourcePriority::CRITICAL;

		}

		throw RuntimeException(L"Unrecognized priority level.");

	}
	
	/// \brief Convert an Eigen Vector3f to an XMFLOAT3
	XMFLOAT3 EigenVector3fToXMFLOAT3(const Eigen::Vector3f & vector){

		return XMFLOAT3(vector.x(), 
						vector.y(), 
						vector.z());

	}

	/// \brief Convert an Eigen Vector2f to an XMFLOAT2
	XMFLOAT2 EigenVector2fToXMFLOAT2(const Eigen::Vector2f & vector){

		return XMFLOAT2(vector.x(),
						vector.y());

	}

	/// \brief Create an index buffer
	template <typename TVertexFormat>
	ID3D11Buffer * MakeVertexBuffer(ID3D11Device & device, const vector<TVertexFormat> & vertices, size_t & size){

		ID3D11Buffer * vertex_buffer = nullptr;

		// Fill in a buffer description.
		D3D11_BUFFER_DESC buffer_desc;

		buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
		buffer_desc.ByteWidth = static_cast<UINT>(sizeof(TVertexFormat) * vertices.size());
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA init_data;

		init_data.pSysMem = &vertices[0];
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;

		THROW_ON_FAIL(device.CreateBuffer(&buffer_desc, &init_data, &vertex_buffer));

		size = buffer_desc.ByteWidth;

		return vertex_buffer;

	}

	/// \brief Create an index buffer
	ID3D11Buffer * MakeIndexBuffer(ID3D11Device & device, const vector<unsigned int> & indices, size_t & size){

		ID3D11Buffer * index_buffer = nullptr;

		// Fill in a buffer description.
		D3D11_BUFFER_DESC buffer_desc;

		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = static_cast<UINT>( sizeof(unsigned int) * indices.size() );
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;

		// Define the resource data.
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = &indices[0];
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;

		// Create the buffer with the device.
		THROW_ON_FAIL(device.CreateBuffer(&buffer_desc, &init_data, &index_buffer));

		size = buffer_desc.ByteWidth;

		return index_buffer;

	}
	
}

////////////////////////////// TEXTURE 2D //////////////////////////////////////////

DX11Texture2D::DX11Texture2D(ID3D11Device & device, const LoadSettings<Texture2D, Texture2D::LoadMode::kFromDDS> & settings){
	
	DDS_ALPHA_MODE alpha_mode;
	ID3D11Resource * resource;
	ID3D11ShaderResourceView * shader_view;

	wstringstream file_name;

	file_name << Application::GetInstance().GetDirectory() << settings.file_name;

	THROW_ON_FAIL( CreateDDSTextureFromFileEx(&device, 
											  file_name.str().c_str(), 
											  0,									// Load everything.
											  D3D11_USAGE_IMMUTABLE, 
											  D3D11_BIND_SHADER_RESOURCE, 
											  0,									// No CPU access.
											  0,
											  false,								// No forced sRGB
											  &resource,
											  &shader_view, 
											  &alpha_mode) );						//Alpha informations

	texture_.reset(static_cast<ID3D11Texture2D*>(resource));	
	shader_view_.reset(shader_view);

	D3D11_TEXTURE2D_DESC description;

	texture_->GetDesc(&description);

	width_ = description.Width;
	height_ = description.Height;
	mip_levels_ = description.MipLevels;
	bits_per_pixel_ = BitsPerPixel(description.Format);
	alpha_ = alpha_mode != DDS_ALPHA_MODE_OPAQUE;									//If it is not opaque, it should have an alpha channel

}

size_t DX11Texture2D::GetSize() const{

	auto level_size = width_ * height_ * bits_per_pixel_ * kBitOverByte;	//Size of the most detailed level.

	// MIP map footprint -> Sum of a geometrical serie...

	return static_cast<size_t>( level_size * ((1.0f - std::powf(kMIPRatio2D, static_cast<float>(mip_levels_))) / (1.0f - kMIPRatio2D)) );

}

ResourcePriority DX11Texture2D::GetPriority() const{

	return EvictionPriorityToResourcePriority(texture_->GetEvictionPriority());

}

void DX11Texture2D::SetPriority(ResourcePriority priority){

	texture_->SetEvictionPriority(ResourcePriorityToEvictionPriority(priority));

}

///////////////////////////// MESH ////////////////////////////////////////////////

DX11Mesh::DX11Mesh(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kNormalTextured> & settings){

	// Normal, textured mesh.

	size_t vb_size = 0;
	size_t ib_size = 0;

	vertex_buffer_.reset(MakeVertexBuffer(device, settings.vertices, vb_size));

	if (settings.indices.size() > 0){

		index_buffer_.reset(MakeIndexBuffer(device, settings.indices, ib_size));
	
		polygon_count_ = settings.indices.size();

	}
	else{

		polygon_count_ = settings.vertices.size() / 3;

	}

	vertex_count_ = settings.vertices.size();
	LOD_count_ = 1;
	size_ = vb_size + ib_size;

}

ResourcePriority DX11Mesh::GetPriority() const{

	return EvictionPriorityToResourcePriority(vertex_buffer_->GetEvictionPriority());

}

void DX11Mesh::SetPriority(ResourcePriority priority){

	vertex_buffer_->SetEvictionPriority(ResourcePriorityToEvictionPriority(priority));

	if (index_buffer_){

		index_buffer_->SetEvictionPriority(ResourcePriorityToEvictionPriority(priority));

	}

}

///////////////////////////// SHADER ///////////////////////////////////////////////

DX11Shader::DX11Shader(ID3D11Device & device, const LoadSettings<Shader, Shader::LoadMode::kCompileFromFile> & settings){

	ID3DX11Effect * effect;
	ID3DBlob * errors = nullptr;

	wstringstream file_name;

	file_name << Application::GetInstance().GetDirectory() << settings.file_name;

#ifdef _DEBUG

	THROW_ON_COMPILE_FAIL(D3DX11CompileEffectFromFile(file_name.str().c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		D3DCOMPILE_DEBUG | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_SKIP_VALIDATION,
		0,
		&device,
		&effect,
		&errors),
		errors);

#else

	THROW_ON_COMPILE_FAIL(D3DX11CompileEffectFromFile(file_name.str().c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3,
		0,
		&device,
		&effect,
		&errors),
		errors);

#endif

	effect_.reset(effect);
	
	priority_ = ResourcePriority::NORMAL;

}

void DX11Shader::CloneEffect(ID3DX11Effect ** effect) const{

	THROW_ON_FAIL(effect_->CloneEffect(D3DX11_EFFECT_CLONE_FORCE_NONSINGLE, effect));

}

////////////////////////////// MATERIAL //////////////////////////////////////////////

DX11Material::DX11Material(ID3D11Device &, const BuildSettings<Material, Material::BuildMode::kFromShader> & settings){

	// Clone the effect of the shader
	auto & shader = resource_cast(settings.shader);

	ID3DX11Effect * effect;

	shader.CloneEffect(&effect);

	effect_.reset(effect);

}

shared_ptr<MaterialParameter> DX11Material::GetParameterByName(const string & name){

	auto variable = shared_ptr<ID3DX11EffectVariable>(effect_->GetVariableByName(name.c_str()), COMDeleter{});

	if (variable->IsValid()){

		return static_pointer_cast<MaterialParameter>(make_shared<DX11MaterialParameter>(variable));

	}
	else{

		return nullptr;

	}

}

shared_ptr<MaterialParameter> DX11Material::GetParameterBySemantic(const string & semantic){

	auto variable = shared_ptr<ID3DX11EffectVariable>(effect_->GetVariableBySemantic(semantic.c_str()), COMDeleter{});

	if (variable->IsValid()){

		return static_pointer_cast<MaterialParameter>(make_shared<DX11MaterialParameter>(variable));

	}
	else{

		return nullptr;

	}

}

//////////////////////////// MATERIAL PARAMETER /////////////////////////////////////////

/// \brief Create a new material parameter.
/// \param variable The variable accessed by this parameter
DX11MaterialParameter::DX11MaterialParameter(shared_ptr<ID3DX11EffectVariable> variable) :
	variable_(variable){

}

DX11MaterialParameter::~DX11MaterialParameter(){}

bool DX11MaterialParameter::Read(bool & out){

	auto scalar = variable_->AsScalar();

	if (scalar->IsValid() &&
		!FAILED(scalar->GetBool(&out))){
	
		scalar->Release();
		
		return true;
		
	}
	else{

		scalar->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(float & out){

	auto scalar = variable_->AsScalar();

	if (scalar->IsValid() &&
		!FAILED(scalar->GetFloat(&out))){

		scalar->Release();

		return true;

	}
	else{

		scalar->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(int & out){

	auto scalar = variable_->AsScalar();

	if (scalar->IsValid() &&
		!FAILED(scalar->GetInt(&out))){

		scalar->Release();

		return true;

	}
	else{

		scalar->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Vector2f & out){

	static const size_t size = sizeof(float) * 2;

	auto vector = variable_->AsVector();

	float * data;

	if (vector->IsValid() &&
		!FAILED(vector->GetFloatVector(data))){

		memcpy_s(out.data(), size, data, size);

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Vector3f & out){

	static const size_t size = sizeof(float) * 3;

	auto vector = variable_->AsVector();

	float * data;

	if (vector->IsValid() &&
		!FAILED(vector->GetFloatVector(data))){

		memcpy_s(out.data(), size, data, size);

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Vector4f & out){

	static const size_t size = sizeof(float) * 4;

	auto vector = variable_->AsVector();

	float * data;

	if (vector->IsValid() &&
		!FAILED(vector->GetFloatVector(data))){

		memcpy_s(out.data(), size, data, size);

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Affine3f & out){

	static const size_t size = sizeof(float) * 12;	//Last column is [0 0 0 1]. Eigen stores in column major by default, so the last 4 values are not needed.

	auto matrix = variable_->AsMatrix();

	float * data;

	if (matrix->IsValid() &&
		!FAILED(matrix->GetMatrix(data))){

		memcpy_s(out.data(), size, data, size);

		matrix->Release();

		return true;

	}
	else{

		matrix->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Projective3f & out){

	static const size_t size = sizeof(float) * 16;

	auto matrix = variable_->AsMatrix();

	float * data;

	if (matrix->IsValid() &&
		!FAILED(matrix->GetMatrix(data))){

		memcpy_s(out.data(), size, data, size);

		matrix->Release();

		return true;

	}
	else{

		matrix->Release();

		return false;

	}
	
}

bool DX11MaterialParameter::Read(shared_ptr<Texture2D> & out){



}

bool DX11MaterialParameter::Read(void ** out){

	static const size_t size = sizeof(float) * 16;

	auto constant_buffer = variable_->AsConstantBuffer();

	ID3D11Buffer * buffer;

	if (constant_buffer->IsValid() &&
		!FAILED(constant_buffer->GetConstantBuffer(&buffer))){

		//Map the constant buffer


		constant_buffer->Release();

		return true;

	}
	else{

		constant_buffer->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const bool & in){

}

bool DX11MaterialParameter::Write(const float & in){

}

bool DX11MaterialParameter::Write(const int & in){

}

bool DX11MaterialParameter::Write(const Vector2f & in){

}

bool DX11MaterialParameter::Write(const Vector3f & in){

}

bool DX11MaterialParameter::Write(const Vector4f & in){

}

bool DX11MaterialParameter::Write(const Affine3f & in){

}

bool DX11MaterialParameter::Write(const Projective3f & in){

}

bool DX11MaterialParameter::Write(const shared_ptr<Texture2D> in){

}

bool DX11MaterialParameter::Write(void ** in){

}