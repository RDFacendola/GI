#pragma comment(lib, "d3dcompiler.lib")

#include "dx11.h"

#include <sstream>
#include <algorithm>

#include "..\..\include\exceptions.h"
#include "..\..\include\enums.h"
#include "..\..\include\scope_guard.h"
#include "..\..\include\windows\win_os.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::windows;

namespace{

	/// \brief Convert a shader resource view dimension into a shader resource type.
	/// \param dimension Shader resource view dimension.
	/// \return Returns the converted shader resource view dimension.
	ShaderResourceType SRVDimensionToShaderResourceType(D3D_SRV_DIMENSION dimension){

		switch (dimension){

		case D3D_SRV_DIMENSION_TEXTURE1D:
		case D3D_SRV_DIMENSION_TEXTURE1DARRAY:

			return ShaderResourceType::TEXTURE_1D;

		case D3D_SRV_DIMENSION_TEXTURE2D:
		case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
		case D3D_SRV_DIMENSION_TEXTURE2DMS:
		case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:

			return ShaderResourceType::TEXTURE_2D;

		case D3D_SRV_DIMENSION_TEXTURE3D:

			return ShaderResourceType::TEXTURE_3D;

		case D3D_SRV_DIMENSION_TEXTURECUBE:
		case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:

			return ShaderResourceType::TEXTURE_CUBE;

		case D3D_SRV_DIMENSION_BUFFER:

			return ShaderResourceType::STRUCTURED_ARRAY;

		default:

			return ShaderResourceType::UNKNOWN;

		}

	}

	D3D11_FILTER AnisotropyLevelToFilter(unsigned int anisotropy_level){

		return anisotropy_level > 0 ?
			   D3D11_FILTER_ANISOTROPIC :		// Anisotropic filtering
			   D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// Trilinear filtering

	}

	D3D11_TEXTURE_ADDRESS_MODE TextureMappingToAddressMode(TextureMapping mapping){

		return mapping == TextureMapping::WRAP ?
			   D3D11_TEXTURE_ADDRESS_WRAP :
			   D3D11_TEXTURE_ADDRESS_CLAMP;

	}

	/// \brief Reflect a shader input inside the specified vector.
	/// The method won't reflect resources already inside the resource vector.
	/// \tparam TType Type of the reflected structure. The type must me expose a field "name" used to uniquely identify the reflected resource.
	/// \tparam TReflector Type of the reflector used to build the reflected data.
	/// \param reflector Used to reflect the shader.
	/// \param input_desc Decription of the shader input.
	/// \param ResourceReflector Functor used to build the reflected data.
	/// \param resource Vector that will contain the reflected data.
	/// \param order Vector containing the binding order.
	/// \return Returns the a reference to the reflected element
	template <typename TType, typename TReflector>
	TType& Reflect(const D3D11_SHADER_INPUT_BIND_DESC& input_desc, TReflector ResourceReflector, vector<TType>& resources){

		auto it = std::find_if(resources.begin(),
							   resources.end(),
							   [&input_desc](const TType& desc){

			return desc.name == input_desc.Name;

		});

		if (it == resources.end()){

			resources.push_back(ResourceReflector(input_desc));

			return resources.back();

		}
		else{

			return *it;

		}

	}

	/// \brief Reflect a constant buffer.
	/// \param reflector Reflector used to perform the reflection.
	/// \param input_desc Description of the shader input.
	/// \return Return the description of the reflected constant buffer.
	ShaderBufferDesc ReflectCBuffer(ID3D11ShaderReflection& reflector, const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

		D3D11_SHADER_BUFFER_DESC dx_buffer_desc;
		D3D11_SHADER_VARIABLE_DESC dx_variable_desc;

		auto buffer = reflector.GetConstantBufferByName(input_desc.Name);

		buffer->GetDesc(&dx_buffer_desc);

		ShaderBufferDesc buffer_desc;

		buffer_desc.name = dx_buffer_desc.Name;
		buffer_desc.size = dx_buffer_desc.Size;
		buffer_desc.shader_usage = ShaderType::NONE;

		for (unsigned int i = 0; i < dx_buffer_desc.Variables; ++i){

			// Variables

			auto variable = buffer->GetVariableByIndex(i);

			variable->GetDesc(&dx_variable_desc);

			buffer_desc.variables.push_back({ dx_variable_desc.Name,
											  dx_variable_desc.Size,
											  dx_variable_desc.StartOffset });

		}

		return buffer_desc;

	}

	/// \brief Reflect a texture.
	/// \param reflector Reflector used to perform the reflection.
	/// \param input_desc Description of the shader input.
	/// \return Return the description of the reflected texture.
	ShaderResourceDesc ReflectResource(const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

		return ShaderResourceDesc{ input_desc.Name,
								   SRVDimensionToShaderResourceType(input_desc.Dimension),
								   input_desc.BindCount,
								   ShaderType::NONE };

	}

	/// \brief Reflect a sampler.
	/// \param reflector Reflector used to perform the reflection.
	/// \param input_desc Description of the shader input.
	/// \return Return the description of the reflected sampler.
	ShaderSamplerDesc ReflectSampler(const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

		return ShaderSamplerDesc{ input_desc.Name, ShaderType::NONE };

	}

	/// \brief Reflect a shader from bytecode.
	/// \param bytecode Bytecode used to perform reflection.
	/// \param reflection Holds shared information about various shaders as well as detailed information about resources.
	template <typename TShader>
	HRESULT Reflect(ID3DBlob& bytecode, ShaderReflection& reflection){

		ID3D11ShaderReflection * reflector = nullptr;

		RETURN_ON_FAIL(D3DReflect(bytecode.GetBufferPointer(),
								  bytecode.GetBufferSize(),
								  IID_ID3D11ShaderReflection,
								  (void**)&reflector));

		COM_GUARD(reflector);

		D3D11_SHADER_DESC shader_desc;

		D3D11_SHADER_INPUT_BIND_DESC resource_desc;

		reflector->GetDesc(&shader_desc);

		for (unsigned int resource_index = 0; resource_index < shader_desc.BoundResources; ++resource_index){

			reflector->GetResourceBindingDesc(resource_index, &resource_desc);

			switch (resource_desc.Type){

				case D3D_SIT_CBUFFER:
				case D3D_SIT_TBUFFER:
				{

					// Constant or Texture buffer

					Reflect(resource_desc,
							[reflector](const D3D11_SHADER_INPUT_BIND_DESC& input_desc){ return ReflectCBuffer(*reflector, input_desc); },
							reflection.buffers).shader_usage |= ShaderTraits<TShader>::flag;

					break;

				}
				case D3D_SIT_TEXTURE:
				case D3D_SIT_STRUCTURED:
				{

					// Shader resources

					Reflect(resource_desc,
							ReflectResource,
							reflection.resources).shader_usage |= ShaderTraits<TShader>::flag;

					break;

				}
				case D3D_SIT_SAMPLER:
				{

					// Samplers

					Reflect(resource_desc,
							ReflectSampler,
							reflection.samplers).shader_usage |= ShaderTraits<TShader>::flag;

					break;

				}

			}

		}

		reflection.shaders |= ShaderTraits<TShader>::flag;

		return S_OK;

	}

	template <typename TShader, typename TCreateShader>
	HRESULT MakeShader(const string& HLSL, const string& source_file, TCreateShader CreateShader, TShader** shader, ShaderReflection* reflection, wstring* error_string){

		ID3DBlob * bytecode = nullptr;

		RETURN_ON_FAIL(::Compile<TShader>(HLSL, 
										  source_file, 
										  &bytecode, 
										  error_string));

		COM_GUARD(bytecode);

		TShader* shader_ptr = nullptr;

		auto cleanup = make_scope_guard([&](){

			if (shader_ptr) shader_ptr->Release();

		});

		if (shader){

			RETURN_ON_FAIL(CreateShader(*bytecode, 
										&shader_ptr));

		}

		if (reflection){

			RETURN_ON_FAIL(::Reflect<TShader>(*bytecode, 
											  *reflection));

		}

		if (shader){

			*shader = shader_ptr;

		}

		cleanup.Dismiss();

		return S_OK;
		
	}
	
}

/////////////////// SHADER TRAITS - VERTEX SHADER ///////////////////////////

const ShaderType ShaderTraits < ID3D11VertexShader >::flag = ShaderType::VERTEX_SHADER;

const char * ShaderTraits < ID3D11VertexShader >::entry_point = "VSMain";

const char * ShaderTraits < ID3D11VertexShader >::profile = "vs_5_0";

HRESULT ShaderTraits < ID3D11VertexShader >::MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11VertexShader** shader, ShaderReflection* reflection, wstring* errors){

	auto make_vertex_shader = [&device](ID3DBlob& bytecode, ID3D11VertexShader** shader_ptr)
	{
		
		return device.CreateVertexShader(bytecode.GetBufferPointer(),
										 bytecode.GetBufferSize(), 
										 nullptr, 
										 shader_ptr);
	
	};

	return ::MakeShader<ID3D11VertexShader>(HLSL,
											source_file,
											make_vertex_shader,
											shader,
											reflection,
											errors);

}

/////////////////// SHADER TRAITS - HULL SHADER ///////////////////////////

const ShaderType ShaderTraits < ID3D11HullShader >::flag = ShaderType::HULL_SHADER;

const char * ShaderTraits < ID3D11HullShader >::entry_point = "HSMain";

const char * ShaderTraits < ID3D11HullShader >::profile = "hs_5_0";

HRESULT ShaderTraits < ID3D11HullShader >::MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11HullShader** shader, ShaderReflection* reflection, wstring* errors){

	auto make_vertex_shader = [&device](ID3DBlob& bytecode, ID3D11HullShader** shader_ptr)
	{

		return device.CreateHullShader(bytecode.GetBufferPointer(),
									   bytecode.GetBufferSize(),
									   nullptr,
									   shader_ptr);

	};

	return ::MakeShader<ID3D11HullShader>(HLSL,
										  source_file,
										  make_vertex_shader,
										  shader,
										  reflection,
										  errors);

}

/////////////////// SHADER TRAITS - DOMAIN SHADER ///////////////////////////

const ShaderType ShaderTraits < ID3D11DomainShader >::flag = ShaderType::DOMAIN_SHADER;

const char * ShaderTraits < ID3D11DomainShader >::entry_point = "DSMain";

const char * ShaderTraits < ID3D11DomainShader >::profile = "ds_5_0";

HRESULT ShaderTraits < ID3D11DomainShader >::MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11DomainShader** shader, ShaderReflection* reflection, wstring* errors){

	auto make_vertex_shader = [&device](ID3DBlob& bytecode, ID3D11DomainShader** shader_ptr)
	{

		return device.CreateDomainShader(bytecode.GetBufferPointer(),
										 bytecode.GetBufferSize(),
										 nullptr,
										 shader_ptr);

	};

	return ::MakeShader<ID3D11DomainShader>(HLSL,
											source_file,
											make_vertex_shader,
											shader,
											reflection,
											errors);

}

/////////////////// SHADER TRAITS - GEOMETRY SHADER ///////////////////////////

const ShaderType ShaderTraits < ID3D11GeometryShader >::flag = ShaderType::GEOMETRY_SHADER;

const char * ShaderTraits < ID3D11GeometryShader >::entry_point = "GSMain";

const char * ShaderTraits < ID3D11GeometryShader >::profile = "gs_5_0";

HRESULT ShaderTraits < ID3D11GeometryShader >::MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11GeometryShader** shader, ShaderReflection* reflection, wstring* errors){

	auto make_vertex_shader = [&device](ID3DBlob& bytecode, ID3D11GeometryShader** shader_ptr)
	{

		return device.CreateGeometryShader(bytecode.GetBufferPointer(),
										   bytecode.GetBufferSize(),
										   nullptr,
										   shader_ptr);

	};

	return ::MakeShader<ID3D11GeometryShader>(HLSL,
											  source_file,
											  make_vertex_shader,
											  shader,
											  reflection,
											  errors);

}

/////////////////// SHADER TRAITS - PIXEL SHADER ///////////////////////////

const ShaderType ShaderTraits < ID3D11PixelShader >::flag = ShaderType::PIXEL_SHADER;

const char * ShaderTraits < ID3D11PixelShader >::entry_point = "PSMain";

const char * ShaderTraits < ID3D11PixelShader >::profile = "ps_5_0";

HRESULT ShaderTraits < ID3D11PixelShader >::MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11PixelShader** shader, ShaderReflection* reflection, wstring* errors){

	auto make_vertex_shader = [&device](ID3DBlob& bytecode, ID3D11PixelShader** shader_ptr)
	{

		return device.CreatePixelShader(bytecode.GetBufferPointer(),
									    bytecode.GetBufferSize(),
										nullptr,
										shader_ptr);

	};

	return ::MakeShader<ID3D11PixelShader>(HLSL,
										   source_file,
										   make_vertex_shader,
										   shader,
										   reflection,
										   errors);

}

/////////////////// METHODS ///////////////////////////

HRESULT gi_lib::dx11::MakeDepthStencil(ID3D11Device& device, unsigned int width, unsigned int height, ID3D11Texture2D** depth_stencil, ID3D11DepthStencilView** depth_stencil_view){

	ID3D11Texture2D * texture = nullptr;
	ID3D11DepthStencilView * view = nullptr;

	auto cleanup = make_scope_guard([&texture](){

		if (texture) texture->Release();
		
	});

	D3D11_TEXTURE2D_DESC desc;

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

	RETURN_ON_FAIL(device.CreateTexture2D(&desc, 
										  nullptr, 
										  &texture));

	if (depth_stencil_view){

		D3D11_DEPTH_STENCIL_VIEW_DESC view_desc;

		ZeroMemory(&view_desc, sizeof(view_desc));

		view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		RETURN_ON_FAIL(device.CreateDepthStencilView(reinterpret_cast<ID3D11Resource *>(texture),
													 &view_desc,
													 &view));

		*depth_stencil_view = view;

	}

	*depth_stencil = texture;

	cleanup.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeRenderTarget(ID3D11Device& device, unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11RenderTargetView** render_target_view, ID3D11ShaderResourceView** shader_resource_view, ID3D11UnorderedAccessView** unordered_access_view, bool autogenerate_mips){

	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	ID3D11UnorderedAccessView* uav = nullptr;

	auto cleanup = make_scope_guard([&texture, &rtv, &srv, &uav](){

		windows::release_com({*texture, rtv, srv, uav});
		
	});

	D3D11_TEXTURE2D_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.ArraySize = 1;
	
	desc.BindFlags = (render_target_view ? D3D11_BIND_RENDER_TARGET : 0 ) |
					 (shader_resource_view ? D3D11_BIND_SHADER_RESOURCE : 0) |
					 (unordered_access_view ? D3D11_BIND_UNORDERED_ACCESS : 0);

	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = autogenerate_mips ? 0 : 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = autogenerate_mips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	RETURN_ON_FAIL(device.CreateTexture2D(&desc,
										  nullptr,
										  texture));

	if (render_target_view){
		
		RETURN_ON_FAIL(device.CreateRenderTargetView(reinterpret_cast<ID3D11Resource*>(*texture),
													 nullptr,
													 &rtv));

	}
	
	if (shader_resource_view){

		RETURN_ON_FAIL(device.CreateShaderResourceView(reinterpret_cast<ID3D11Resource*>(*texture),
													   nullptr,
													   &srv));

	}
	
	if (unordered_access_view){
		
		RETURN_ON_FAIL(device.CreateUnorderedAccessView(reinterpret_cast<ID3D11Resource*>(*texture),
														nullptr,
														&uav));

	}
	
	if (render_target_view){

		*render_target_view = rtv;

	}
	
	if (shader_resource_view){

		*shader_resource_view = srv;

	}
	
	if (unordered_access_view){

		*unordered_access_view = uav;

	}

	cleanup.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeVertexBuffer(ID3D11Device& device, const void* vertices, size_t size, ID3D11Buffer** buffer){

	// Fill in a buffer description.
	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	buffer_desc.ByteWidth = static_cast<unsigned int>(size);
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA init_data;

	init_data.pSysMem = vertices;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	// Create the buffer
	return device.CreateBuffer(&buffer_desc, 
							   &init_data,
							   buffer);

}

HRESULT gi_lib::dx11::MakeIndexBuffer(ID3D11Device& device, const unsigned int* indices, size_t size, ID3D11Buffer** buffer){

	// Fill in a buffer description.
	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = static_cast<unsigned int>(size);
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;

	// Define the resource data.
	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = indices;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	// Create the buffer
	return device.CreateBuffer(&buffer_desc, 
							   &init_data, 
							   buffer);

}

HRESULT gi_lib::dx11::MakeConstantBuffer(ID3D11Device& device, size_t size, ID3D11Buffer** buffer){

	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.ByteWidth = static_cast<unsigned int>(size);
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	// Create the buffer
	return device.CreateBuffer(&buffer_desc, 
							   nullptr, 
							   buffer);

}

HRESULT gi_lib::dx11::MakeStructuredBuffer(ID3D11Device& device, unsigned int element_count, unsigned int element_size, bool dynamic, ID3D11Buffer** buffer, ID3D11ShaderResourceView** shader_resource_view, ID3D11UnorderedAccessView** unordered_access_view){

	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	buffer_desc.ByteWidth = element_size * element_count;

	buffer_desc.BindFlags = (shader_resource_view ? D3D11_BIND_SHADER_RESOURCE : 0) |
							(unordered_access_view ? D3D11_BIND_UNORDERED_ACCESS : 0);

	buffer_desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	buffer_desc.StructureByteStride = element_size;

	// Transaction: either all the resources are created, or none.

	ID3D11Buffer* structured = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	ID3D11UnorderedAccessView* uav = nullptr;

	auto guard = make_scope_guard([&structured, &srv, &uav]{

		release_com({ structured, srv, uav });

	});

	RETURN_ON_FAIL(device.CreateBuffer(&buffer_desc,
									   nullptr,
									   &structured));

	if (shader_resource_view){

		RETURN_ON_FAIL(device.CreateShaderResourceView(structured,
													   nullptr,
													   &srv));

	}

	if (unordered_access_view){

		RETURN_ON_FAIL(device.CreateUnorderedAccessView(structured,
														nullptr, 
														&uav));
		
	}

	// Commit

	*buffer = structured;

	if (shader_resource_view){

		*shader_resource_view = srv;

	}

	if (unordered_access_view){

		*unordered_access_view = uav;

	}

	guard.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeSampler(ID3D11Device& device, TextureMapping texture_mapping, unsigned int anisotropy_level, ID3D11SamplerState** sampler){

	auto address_mode = TextureMappingToAddressMode(texture_mapping); // Same for each coordinate.

	auto filter = AnisotropyLevelToFilter(anisotropy_level);

	D3D11_SAMPLER_DESC desc;
	
	desc.Filter = filter;
	desc.AddressU = address_mode;
	desc.AddressV = address_mode;
	desc.AddressW = address_mode;
	desc.MipLODBias = 0.0f;								// This could be used to reduce the texture quality, however it will waste VRAM. 
	desc.MaxAnisotropy = anisotropy_level;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//desc.BorderColor[0] = 1.0f;						// Whatever, not used.
	//desc.BorderColor[1] = 1.0f;
	//desc.BorderColor[2] = 1.0f;
	//desc.BorderColor[3] = 1.0f;
	desc.MinLOD = -FLT_MAX;
	desc.MaxLOD = FLT_MAX;
	
	// Create the sampler state
	return device.CreateSamplerState(&desc, 
									 sampler);

}

Matrix4f gi_lib::dx11::ComputePerspectiveProjectionLH(float field_of_view, float aspect_ratio, float near_plane, float far_plane){

	Matrix4f projection_matrix = Matrix4f::Identity();

	auto height = 1.0f / std::tanf(field_of_view * 0.5f);

	auto width = height / aspect_ratio;

	projection_matrix(0, 0) = width;

	projection_matrix(1, 1) = height;

	projection_matrix(2, 2) = far_plane / (far_plane - near_plane);

	projection_matrix(3, 3) = 0.f;

	projection_matrix(2, 3) = -(near_plane * far_plane) / (far_plane - near_plane);

	projection_matrix(3, 2) = 1.0f;

	return projection_matrix;

}