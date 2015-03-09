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
#include "..\..\include\scope_guard.h"

#include "..\..\include\windows\os_windows.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::windows;

using namespace DirectX;
using namespace Eigen;

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

		THROW(L"Unrecognized priority level.");

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

		THROW(L"Unrecognized priority level.");

	}
	
	/// \brief Convert an Eigen Vector3f to an XMFLOAT3.
	XMFLOAT3 EigenVector3fToXMFLOAT3(const Eigen::Vector3f & vector){

		return XMFLOAT3(vector.x(), 
						vector.y(), 
						vector.z());

	}

	/// \brief Convert an Eigen Vector2f to an XMFLOAT2.
	XMFLOAT2 EigenVector2fToXMFLOAT2(const Eigen::Vector2f & vector){

		return XMFLOAT2(vector.x(),
						vector.y());

	}

	/// \brief Create a depth stencil suitable for the provided target.

	/// The resource must be manually released!
	ID3D11Texture2D * MakeDepthStencil(ID3D11Device & device, ID3D11Texture2D & target){

		ID3D11Texture2D * depth_stencil;

		// Create the depth buffer for use with the depth/stencil view.
		D3D11_TEXTURE2D_DESC desc;

		target.GetDesc(&desc);

		auto width = desc.Width;
		auto height = desc.Height;

		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;

		THROW_ON_FAIL(device.CreateTexture2D(&desc, nullptr, &depth_stencil));

		return depth_stencil;

	}

	/// \brief Create a vertex buffer
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
	
	template <typename TVertexFormat>
	Bounds VerticesToBounds(const std::vector<TVertexFormat> & vertices){

		if (vertices.size() == 0){

			return Bounds{ Vector3f::Zero(), Vector3f::Zero() };

		}

		Vector3f min_corner;
		Vector3f max_corner;

		min_corner = vertices[0].position;
		max_corner = vertices[0].position;

		for (auto & vertex : vertices){

			// Find maximum and minimum coordinates for each axis independently

			for (int coordinate = 0; coordinate < 3; ++coordinate){

				if (min_corner(coordinate) > vertex.position(coordinate)){

					min_corner(coordinate) = vertex.position(coordinate);

				}
				else if (max_corner(coordinate) < vertex.position(coordinate)){

					max_corner(coordinate) = vertex.position(coordinate);

				}

			}

		}

		return Bounds{ 0.5f * (max_corner + min_corner),
			max_corner - min_corner };
					  
	}

	/// \brief Info about shader types.
	template <typename TShaderType>
	struct ShaderTypeInfo;

	/// \brief Info about vertex shader.
	template <> struct ShaderTypeInfo < ID3D11VertexShader > {

		static const char * kEntryPoint;		///< Entry point for the vertex shader.

		static const char * kShaderProfile;		///< Shader profile.

		static const bool kCompulsory;			///< Whether the presence of a vertex shader is compulsory or not.

	};

	/// \brief Info about geometry shader.
	template <> struct ShaderTypeInfo < ID3D11GeometryShader > {

		static const char * kEntryPoint;		///< Entry point for the geometry shader.

		static const char * kShaderProfile;		///< Shader profile.

		static const bool kCompulsory;			///< Whether the presence of a geometry shader is compulsory or not.

	};
	
	/// \brief Info about pixel shader.
	template <> struct ShaderTypeInfo < ID3D11PixelShader > {

		static const char * kEntryPoint;		///< Entry point for the pixel shader.

		static const char * kShaderProfile;		///< Shader profile.

		static const bool kCompulsory;			///< Whether the presence of a pixel shader is compulsory or not.

	};
	
	// \brief Compile a shader to bytecode.
	template <typename TShaderType>
	unique_ptr<ID3DBlob, COMDeleter> Compile(const string& source_file, const string& code){

		ID3DBlob * bytecode = nullptr;
		ID3DBlob * errors = nullptr;

		#ifdef _DEBUG

		UINT compilation_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_SKIP_OPTIMIZATION;

		#else

		UINT compilation_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3;

		#endif

		HRESULT hr = D3DCompile(&code[0],
								code.size(),
								source_file.c_str(),
								nullptr,
								D3D_COMPILE_STANDARD_FILE_INCLUDE,
								ShaderTypeInfo<TShaderType>::kEntryPoint,
								ShaderTypeInfo<TShaderType>::kShaderProfile,
								compilation_flags,
								0,
								&bytecode,
								&errors);

		COM_GUARD(errors);

		if (FAILED(hr)){

			if (ShaderTypeInfo<TShaderType>::kCompulsory){

				wstringstream stream;

				stream << std::to_wstring(hr) << L" - " << errors->GetBufferPointer();

				THROW(stream.str());

			}
			else{

				return nullptr;

			}

		}
		
		return unique_ptr<ID3DBlob, COMDeleter>(bytecode, COMDeleter{});

	}
	
	/// \brief Get the slot index of a constant buffer by name. If the name couldn't be found, the buffer is added at the end.
	/// \return Returns the slot index associated to the specified constant buffer.
	unsigned int CBufferSlot(const string& cbuffer_name, map<const string, unsigned int>& cbuffer_map){

		auto it = cbuffer_map.find(cbuffer_name);

		if (it != cbuffer_map.cend()){

			return it->second;

		}

		cbuffer_map[cbuffer_name] = cbuffer_map.size();

		return cbuffer_map.size() - 1;

	}

	// \brief Perform a shader reflection.
	void Reflect(ID3DBlob * bytecode, map<const string, unsigned int>& cbuffer_map){
		
		if (bytecode == nullptr){

			return;

		}

		ID3D11ShaderReflection * reflector = nullptr;

		THROW_ON_FAIL(D3DReflect(bytecode->GetBufferPointer(),
								 bytecode->GetBufferSize(),
								 IID_ID3D11ShaderReflection,
								 (void**)&reflector));

		COM_GUARD(reflector)

		D3D11_SHADER_DESC shader_desc;
		D3D11_SHADER_BUFFER_DESC buffer_desc;
		D3D11_SHADER_VARIABLE_DESC variable_desc;

		reflector->GetDesc(&shader_desc);

		for (int cbuffer_index = 0; cbuffer_index < shader_desc.ConstantBuffers; ++cbuffer_index){

			auto buffer = reflector->GetConstantBufferByIndex(cbuffer_index);

			buffer->GetDesc(&buffer_desc);

			auto cbuffer_slot = CBufferSlot(buffer_desc.Name, cbuffer_map);

			for (int variable_index = 0; variable_index < buffer_desc.Variables; ++variable_index){

				auto variable = buffer->GetVariableByIndex(variable_index);

				variable->GetDesc(&variable_desc);


			}

		}

	}


	//

	const char * ShaderTypeInfo<ID3D11VertexShader>::kEntryPoint = "VSMain";
	const char * ShaderTypeInfo<ID3D11VertexShader>::kShaderProfile = "vs_5_0";
	const bool   ShaderTypeInfo<ID3D11VertexShader>::kCompulsory = true;

	const char * ShaderTypeInfo<ID3D11GeometryShader>::kEntryPoint = "GSMain";
	const char * ShaderTypeInfo<ID3D11GeometryShader>::kShaderProfile = "gs_5_0";
	const bool   ShaderTypeInfo<ID3D11GeometryShader>::kCompulsory = false;

	const char * ShaderTypeInfo<ID3D11PixelShader>::kEntryPoint = "PSMain";
	const char * ShaderTypeInfo<ID3D11PixelShader>::kShaderProfile = "ps_5_0";
	const bool   ShaderTypeInfo<ID3D11PixelShader>::kCompulsory = true;

}

////////////////////////////// TEXTURE 2D //////////////////////////////////////////

DX11Texture2D::DX11Texture2D(ID3D11Device & device, const LoadFromFile& bundle){
	
	DDS_ALPHA_MODE alpha_mode;
	ID3D11Resource * resource;
	ID3D11ShaderResourceView * shader_view;

	wstringstream file_name;

	file_name << Application::GetInstance().GetDirectory() << bundle.file_name;

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

DX11Texture2D::DX11Texture2D(ID3D11Texture2D & texture, DXGI_FORMAT format){

	ID3D11Device * device;

	texture.GetDevice(&device);

	unique_ptr<ID3D11Device, COMDeleter> guard(device, COMDeleter{});	// Will release the device

	ID3D11ShaderResourceView * shader_view;

	D3D11_TEXTURE2D_DESC texture_desc;

	texture.GetDesc(&texture_desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc;

	view_desc.Format = format == DXGI_FORMAT_UNKNOWN ? texture_desc.Format : format;
	view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MostDetailedMip = 0;
	view_desc.Texture2D.MipLevels = texture_desc.MipLevels;

	THROW_ON_FAIL(device->CreateShaderResourceView(reinterpret_cast<ID3D11Resource *>(&texture),
		&view_desc,
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

void DX11Texture2D::UpdateDescription(){
	
	D3D11_TEXTURE2D_DESC description;

	texture_->GetDesc(&description);

	width_ = description.Width;
	height_ = description.Height;
	mip_levels_ = description.MipLevels;
	bits_per_pixel_ = static_cast<unsigned int>(BitsPerPixel(description.Format));

}

///////////////////////////// RENDER TARGET ///////////////////////////////////////

DX11RenderTarget::DX11RenderTarget(ID3D11Texture2D & target){

	SetBuffers({ &target });

}

void DX11RenderTarget::SetBuffers(std::initializer_list<ID3D11Texture2D*> targets){

	/// The render target view format and the shader resource view format for the render targets are the same of the textures they are generated from (DXGI_FORMAT_UNKNOWN).
	/// The depth stencil texture is created with a 24bit channel for the depth and a 8bit channel for the stencil, both without a type (DXGI_FORMAT_R24G8_TYPELESS).
	/// The depth stencil view format of the depth stencil texture is 24bit uniform for the depth and 8bit unsigned int for the stencil (DXGI_FORMAT_D24_UNORM_S8_UINT).
	/// The shader resource view of the depth stencil texture is 24bit uniform for the depth. The stencil cannot be sampled inside the shader (DXGI_FORMAT_R24_UNORM_X8_TYPELESS).
	
	ResetBuffers();

	ID3D11Device * device;
	ID3D11Texture2D * zstencil;
	ID3D11RenderTargetView * render_target_view;
	ID3D11DepthStencilView * zstencil_view;

	// Rollback guard ensures that the state of the render target is cleared on error
	// (ie: if one buffer causes an exception, the entire operation is rollback'd)

	auto rollback = make_scope_guard([this](){
	
		textures_.clear();
		target_views_.clear();

		zstencil_ = nullptr;
		zstencil_view_ = nullptr;
	
	});

	(*targets.begin())->GetDevice(&device);

	unique_ptr<ID3D11Device, COMDeleter> guard(device, COMDeleter{});	// Will release the device

	for (auto target : targets){
		
		THROW_ON_FAIL(device->CreateRenderTargetView(reinterpret_cast<ID3D11Resource *>(target),
			nullptr,
			&render_target_view));

		textures_.push_back(make_shared<DX11Texture2D>(*target, DXGI_FORMAT_UNKNOWN));
		target_views_.push_back(std::move(unique_ptr<ID3D11RenderTargetView, COMDeleter>(render_target_view, COMDeleter{})));

	}

	// Create the z-stencil and the z-stencil view

	zstencil = MakeDepthStencil(*device, **targets.begin());

	D3D11_DEPTH_STENCIL_VIEW_DESC view_desc;

	ZeroMemory(&view_desc, sizeof(view_desc));

	view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	
	THROW_ON_FAIL(device->CreateDepthStencilView(reinterpret_cast<ID3D11Resource *>(zstencil),
		&view_desc,
		&zstencil_view));

	zstencil_ = make_shared<DX11Texture2D>(*zstencil, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);				// This is the only format compatible with R24G8_TYPELESS used to create the depth buffer resource
	zstencil_view_ = unique_ptr<ID3D11DepthStencilView, COMDeleter>(zstencil_view, COMDeleter{});

	// Everything went as it should have...
	rollback.Dismiss();

}

void DX11RenderTarget::ResetBuffers(){

	textures_.clear();
	target_views_.clear();
	zstencil_ = nullptr;
	zstencil_view_ = nullptr;

}

void DX11RenderTarget::Bind(ID3D11DeviceContext & context){

	// Actual array of render target views.

	vector<ID3D11RenderTargetView *> target_view_array(target_views_.size());

	std::transform(target_views_.begin(),
		target_views_.end(),
		target_view_array.begin(),
		[](unique_ptr<ID3D11RenderTargetView, COMDeleter> & target_view){

			return target_view.get();

		});

	context.OMSetRenderTargets(static_cast<unsigned int>(target_view_array.size()),
		&target_view_array[0],
		zstencil_view_.get());

}

void DX11RenderTarget::ClearDepthStencil(ID3D11DeviceContext & context, unsigned int clear_flags, float depth, unsigned char stencil){

	context.ClearDepthStencilView(zstencil_view_.get(), clear_flags, depth, stencil);
	
}

void DX11RenderTarget::ClearTargets(ID3D11DeviceContext & context, Color color){

	// The color is ARGB, however the method ClearRenderTargetView needs an RGBA.

	float rgba_color[4];

	rgba_color[0] = color.color.red;
	rgba_color[1] = color.color.green;
	rgba_color[2] = color.color.blue;
	rgba_color[3] = color.color.alpha;

	for (auto & rt_view : target_views_){

		context.ClearRenderTargetView(rt_view.get(), rgba_color);

	}

}

///////////////////////////// MESH ////////////////////////////////////////////////

DX11Mesh::DX11Mesh(ID3D11Device & device, const BuildIndexedNormalTextured& bundle){

	// Normal, textured mesh.

	size_t vb_size = 0;
	size_t ib_size = 0;
	
	vertex_buffer_.reset(MakeVertexBuffer(device, bundle.vertices, vb_size));

	if (bundle.indices.size() > 0){

		index_buffer_.reset(MakeIndexBuffer(device, bundle.indices, ib_size));
	
		polygon_count_ = bundle.indices.size();

	}
	else{

		polygon_count_ = bundle.vertices.size() / 3;

	}

	vertex_count_ = bundle.vertices.size();
	LOD_count_ = 1;
	size_ = vb_size + ib_size;

	bounds_ = VerticesToBounds(bundle.vertices);

}

////////////////////////////// MATERIAL //////////////////////////////////////////////

DX11Material::DX11Material(ID3D11Device& device, const CompileFromFile& bundle){

	// TODO: Load the shader, perform reflection and stuffs

	string code = IO::ReadFile(bundle.file_name);

	string file_name = string(bundle.file_name.begin(), bundle.file_name.end());

	map<const string, unsigned int> cbuffer_map;

	auto vs = Compile<ID3D11VertexShader>(file_name, code);
	auto gs = Compile<ID3D11GeometryShader>(file_name, code);
	auto ps = Compile<ID3D11PixelShader>(file_name, code);

	shared_ptr<vector<ParameterInfo>> parameters_;

	// \brief Buffer status.
	vector<CBufferInfo> buffers_;


	Reflect(vs.get(), cbuffer_map);
	Reflect(gs.get(), cbuffer_map);
	Reflect(ps.get(), cbuffer_map);

}

DX11Material::DX11Material(ID3D11Device& device, const InstantiateFromMaterial& bundle){

	// TODO: Instantiate etc.

}

unsigned int DX11Material::GetParameterIndex(const string& name) const{

	THROW(L"Invalid shader parameter name.");

}

unsigned int DX11Material::GetTextureIndex(const string& name) const{

	return -1;

}

bool DX11Material::SetTexture(unsigned int index, shared_ptr<Texture2D> texture){

	return false;

}

bool DX11Material::SetParameter(unsigned int index, const void* buffer, size_t size){

	return false;

}