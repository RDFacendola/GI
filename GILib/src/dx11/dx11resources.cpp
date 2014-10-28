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
#include "..\..\include\scope_guard.h"

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
									std::string error_string(errors != nullptr ? static_cast<char *>(errors->GetBufferPointer()) : ""); \
									std::wstring werror_string(error_string.begin(), error_string.end()); \
									if(blob != nullptr) blob->Release(); \
									throw RuntimeException(stream.str(), {{L"error_code", std::to_wstring(hr)}, {L"compiler error", werror_string}}); \
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

	UpdateDescription();
	
}

DX11Texture2D::DX11Texture2D(ID3D11Texture2D & texture){

	ID3D11Device * device;

	texture.GetDevice(&device);

	unique_ptr<ID3D11Device, COMDeleter> guard(device, COMDeleter{});	// Will release the device

	ID3D11ShaderResourceView * shader_view;

	THROW_ON_FAIL(device->CreateShaderResourceView(reinterpret_cast<ID3D11Resource *>(&texture),
		nullptr,
		&shader_view));

	texture_.reset(&texture);
	shader_view_.reset(shader_view);

	UpdateDescription();

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

void DX11Texture2D::UpdateDescription(){
	
	D3D11_TEXTURE2D_DESC description;

	texture_->GetDesc(&description);

	width_ = description.Width;
	height_ = description.Height;
	mip_levels_ = description.MipLevels;
	bits_per_pixel_ = static_cast<unsigned int>(BitsPerPixel(description.Format));

}

///////////////////////////// RENDER TARGET ///////////////////////////////////////

DX11RenderTarget::DX11RenderTarget(ID3D11Texture2D & buffer){

	SetBuffers({ &buffer });

}

void DX11RenderTarget::SetBuffers(initializer_list<ID3D11Texture2D*> buffers){

	ResetBuffers();

	ID3D11Device * device;
	ID3D11RenderTargetView * render_target_view;

	// Rollback guard ensures that the state of the render target is cleared upon error
	// (ie: if one buffer causes an exception, the entire operation is rollback'd)

	auto rollback = make_scope_guard([this](){
	
		textures_.clear();
		target_views_.clear();
	
	});

	for (auto buffer : buffers){

		buffer->GetDevice(&device);

		unique_ptr<ID3D11Device, COMDeleter> guard(device, COMDeleter{});	// Will release the device

		THROW_ON_FAIL(device->CreateRenderTargetView(reinterpret_cast<ID3D11Resource *>(buffer),
			nullptr,
			&render_target_view));

		textures_.push_back(make_shared<DX11Texture2D>(*buffer));
		target_views_.push_back(std::move(unique_ptr<ID3D11RenderTargetView, COMDeleter>(render_target_view, COMDeleter{})));

	}

	// Everything went as it should have...
	rollback.Dismiss();

}

void DX11RenderTarget::ResetBuffers(){

	textures_.clear();
	target_views_.clear();

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

		return static_pointer_cast<MaterialParameter>(make_shared<DX11MaterialParameter>(variable, *this));

	}
	else{

		return nullptr;

	}

}

shared_ptr<MaterialParameter> DX11Material::GetParameterBySemantic(const string & semantic){

	auto variable = shared_ptr<ID3DX11EffectVariable>(effect_->GetVariableBySemantic(semantic.c_str()), COMDeleter{});

	if (variable->IsValid()){

		return static_pointer_cast<MaterialParameter>(make_shared<DX11MaterialParameter>(variable, *this));

	}
	else{

		return nullptr;

	}

}

//////////////////////////// MATERIAL PARAMETER /////////////////////////////////////////

/// \brief Create a new material parameter.
/// \param variable The variable accessed by this parameter
DX11MaterialParameter::DX11MaterialParameter(shared_ptr<ID3DX11EffectVariable> variable, DX11Material & material) :
	variable_(variable),
	material_(material){

	variable_->GetDesc(&metadata_);

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

	auto vector = variable_->AsVector();

	float data[2];

	if (vector->IsValid() &&
		!FAILED(vector->GetFloatVector(data))){

		memcpy_s(out.data(), out.size() * sizeof(float), data, sizeof(data));

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Vector3f & out){

	auto vector = variable_->AsVector();

	float data[3];

	if (vector->IsValid() &&
		!FAILED(vector->GetFloatVector(data))){

		memcpy_s(out.data(), out.size() * sizeof(float), data, sizeof(data));

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Vector4f & out){

	auto vector = variable_->AsVector();

	float data[4];

	if (vector->IsValid() &&
		!FAILED(vector->GetFloatVector(data))){

		memcpy_s(out.data(), out.size() * sizeof(float), data, sizeof(data));

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Affine3f & out){

	auto matrix = variable_->AsMatrix();

	float data[16];

	if (matrix->IsValid() &&
		!FAILED(matrix->GetMatrix(data))){

		//The affine matrix assumes that the last column is [0 0 0 1], so last 4 elements are ignored.
		memcpy_s(out.data(), 12 * sizeof(float), data, 12 * sizeof(float));	

		matrix->Release();

		return true;

	}
	else{

		matrix->Release();

		return false;

	}

}

bool DX11MaterialParameter::Read(Projective3f & out){

	auto matrix = variable_->AsMatrix();

	float data[16];

	if (matrix->IsValid() &&
		!FAILED(matrix->GetMatrix(data))){

		memcpy_s(out.data(), sizeof(data), data, sizeof(data));

		matrix->Release();

		return true;

	}
	else{

		matrix->Release();

		return false;

	}
	
}

bool DX11MaterialParameter::Read(shared_ptr<Texture2D> & out){

	auto texture = variable_->AsShaderResource();

	if (texture->IsValid()){

		auto & resources = material_.resources_;

		auto it = resources.find(metadata_.Name);
				
		if (it != resources.end()){

			// If the texture was set before then we just find the previous entry
			out.reset(static_cast<Texture2D*>(it->second.get()));


		}
		else{

			// Resource was not set, so it is null.
			out.reset();

		}

		texture->Release();
		return true;

	}
	else{

		texture->Release();
		return false;

	}

}

bool DX11MaterialParameter::Read(void **){

	throw RuntimeException(L"Not implemented!");

}

bool DX11MaterialParameter::Write(const bool & in){

	auto scalar = variable_->AsScalar();

	if (scalar->IsValid() &&
		!FAILED(scalar->SetBool(in))){

		scalar->Release();

		return true;

	}
	else{

		scalar->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const float & in){

	auto scalar = variable_->AsScalar();

	if (scalar->IsValid() &&
		!FAILED(scalar->SetFloat(in))){

		scalar->Release();

		return true;

	}
	else{

		scalar->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const int & in){

	auto scalar = variable_->AsScalar();

	if (scalar->IsValid() &&
		!FAILED(scalar->SetInt(in))){

		scalar->Release();

		return true;

	}
	else{

		scalar->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const Vector2f & in){

	auto vector = variable_->AsVector();

	float data[4];

	data[2] = 0.0f;
	data[3] = 0.0f;
		
	memcpy_s(data, sizeof(data), in.data(), in.size() * sizeof(float));

	if (vector->IsValid() &&
		!FAILED(vector->SetFloatVector(data))){

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const Vector3f & in){

	auto vector = variable_->AsVector();

	float data[4];

	data[3] = 0.0f;

	memcpy_s(data, sizeof(data), in.data(), in.size() * sizeof(float));

	if (vector->IsValid() &&
		!FAILED(vector->SetFloatVector(data))){

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const Vector4f & in){

	auto vector = variable_->AsVector();

	if (vector->IsValid() &&
		!FAILED(vector->SetFloatVector(in.data()))){

		vector->Release();

		return true;

	}
	else{

		vector->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const Affine3f & in){

	static const size_t size = sizeof(float) * 12;	//Last column is [0 0 0 1]. Eigen stores in column major by default, so the last 4 values are not needed.

	auto matrix = variable_->AsMatrix();

	float data[16];

	data[12] = 0.0f;
	data[13] = 0.0f;
	data[14] = 0.0f;
	data[15] = 1.0f;

	memcpy_s(data, sizeof(data), in.data(), size);

	if (matrix->IsValid() &&
		!FAILED(matrix->SetMatrix(data))){

		matrix->Release();

		return true;

	}
	else{

		matrix->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const Projective3f & in){

	auto matrix = variable_->AsMatrix();

	if (matrix->IsValid() &&
		!FAILED(matrix->SetMatrix(in.data()))){

		matrix->Release();

		return true;

	}
	else{

		matrix->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(const shared_ptr<Texture2D> in){

	auto texture = variable_->AsShaderResource();

	auto srv = &(resource_cast(in).GetShaderResourceView());

	if (texture->IsValid() &&
		!FAILED(texture->SetResource(srv))){

		texture->Release();

		material_.resources_[metadata_.Name] = in;

		return true;

	}
	else{

		texture->Release();

		return false;

	}

}

bool DX11MaterialParameter::Write(void **){

	throw RuntimeException(L"Not implemented!");

}