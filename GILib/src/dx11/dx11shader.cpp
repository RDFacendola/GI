#include "dx11/dx11shader.h"

#pragma comment(lib, "d3dcompiler.lib")

#include <d3dcompiler.h>

#include "scope_guard.h"

#include "windows/win_os.h"

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

			return ShaderResourceType::BUFFER;

		default:

			return ShaderResourceType::UNKNOWN;

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
		buffer_desc.slot = input_desc.BindPoint;

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

	/// \brief Reflect a shader resource view.
	/// \param input_desc Description of the shader input.
	/// \return Return the description of the reflected resource.
	inline ShaderSRVDesc ReflectSRV(const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

		return ShaderSRVDesc{ input_desc.Name,
							  SRVDimensionToShaderResourceType(input_desc.Dimension),
							  input_desc.BindCount,
							  input_desc.BindPoint };

	}

	/// \brief Reflect a unordered access view.
	/// \param input_desc Description of the shader input.
	/// \return Return the description of the reflected unordered access.
	inline ShaderUAVDesc ReflectUAV(const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

		return ShaderUAVDesc{ input_desc.Name,
							  SRVDimensionToShaderResourceType(input_desc.Dimension),
							  input_desc.BindPoint };

	}
	
	/// \brief Reflect a sampler.
	/// \param reflector Reflector used to perform the reflection.
	/// \param input_desc Description of the shader input.
	/// \return Return the description of the reflected sampler.
	ShaderSamplerDesc ReflectSampler(const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

		return ShaderSamplerDesc{ input_desc.Name,
								  input_desc.BindPoint };

	}

	/// \brief Reflect a shader description.
	template <typename TShader>
	void ReflectShader(ID3D11ShaderReflection&, const D3D11_SHADER_DESC&, ShaderReflection&){}
	
	/// \brief Reflect a pixel shader description.
	template<>
	void ReflectShader<ID3D11PixelShader>(ID3D11ShaderReflection&, const D3D11_SHADER_DESC& desc, ShaderReflection& reflection) {

		reflection.pixel_shader.render_targets_ = desc.OutputParameters;

	}

	/// \brief Reflect a compute shader description.
	template<>
	void ReflectShader<ID3D11ComputeShader>(ID3D11ShaderReflection& reflector, const D3D11_SHADER_DESC&, ShaderReflection& reflection){

		reflector.GetThreadGroupSize(&reflection.compute_shader.thread_group_x,
									 &reflection.compute_shader.thread_group_y,
									 &reflection.compute_shader.thread_group_z);
		
	}

	/// \brief Reflect a shader resources via reflector.
	/// \param reflector Reflector used to access the description of the shader.
	/// \param reflection Actual shader reflection to fill. Out.
	template <typename TShader>
	void ReflectShaderResources(ID3D11ShaderReflection& reflector, const D3D11_SHADER_DESC& shader_desc, ShaderReflection& reflection){

		D3D11_SHADER_INPUT_BIND_DESC resource_desc;
		
		for (unsigned int resource_index = 0; resource_index < shader_desc.BoundResources; ++resource_index){

			reflector.GetResourceBindingDesc(resource_index, &resource_desc);

			switch (resource_desc.Type){

				case D3D_SIT_CBUFFER:
				case D3D_SIT_TBUFFER:
				{

					// Constant or Texture buffer
					reflection.buffers.push_back(ReflectCBuffer(reflector,
																resource_desc));

					break;

				}
				case D3D_SIT_TEXTURE:
				case D3D_SIT_STRUCTURED:
				case D3D_SIT_BYTEADDRESS:
				{

					// Shader resource view

					reflection.shader_resource_views.push_back(ReflectSRV(resource_desc));

					break;

				}
				case D3D_SIT_SAMPLER:
				{

					// Samplers
					reflection.samplers.push_back(ReflectSampler(resource_desc));

					break;

				}

				case D3D_SIT_UAV_RWTYPED:
				case D3D_SIT_UAV_RWSTRUCTURED:
				case D3D_SIT_UAV_RWBYTEADDRESS:
				case D3D_SIT_UAV_APPEND_STRUCTURED:
				case D3D_SIT_UAV_CONSUME_STRUCTURED:
				case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				{

					// Unordered access view
					reflection.unordered_access_views.push_back(ReflectUAV(resource_desc));

					break;

				}

			}

		}
				
	}

	/// \brief Reflect a shader from bytecode.
	/// \param bytecode Bytecode used to perform reflection.
	/// \param reflection Holds shared information about various shaders as well as detailed information about resources.
	template <typename TShader>
	HRESULT ReflectShader(ID3DBlob& bytecode, ShaderReflection& reflection){

		ID3D11ShaderReflection* reflector = nullptr;

		RETURN_ON_FAIL(D3DReflect(bytecode.GetBufferPointer(),
								  bytecode.GetBufferSize(),
								  IID_ID3D11ShaderReflection,
								  (void**)&reflector));

		COM_GUARD(reflector);

		D3D11_SHADER_DESC shader_desc;

		reflector->GetDesc(&shader_desc);


		reflection.shader_type = ShaderTraits<TShader>::flag;

		ReflectShader<TShader>(*reflector,
							   shader_desc,
							   reflection);
		
		ReflectShaderResources<TShader>(*reflector,
										shader_desc,
										reflection);

		return S_OK;

	}

	template <typename TShader, typename TCreateShader>
	HRESULT MakeShader(const string& HLSL, const string& source_file, TCreateShader CreateShader, TShader** shader, ShaderReflection* reflection, wstring* error_string){

		ID3DBlob* bytecode = nullptr;

		RETURN_ON_FAIL(CompileHLSL<TShader>(HLSL, 
											source_file, 
											&bytecode, 
											error_string));

		COM_GUARD(bytecode);

		TShader* shader_ptr = nullptr;

		auto cleanup = make_scope_guard([&](){

			if (shader_ptr)	shader_ptr->Release();

		});

		if (shader){

			RETURN_ON_FAIL(CreateShader(*bytecode, 
										&shader_ptr));

		}

		if (reflection){

			RETURN_ON_FAIL(::ReflectShader<TShader>(*bytecode,
													*reflection));

		}

		if (shader){

			*shader = shader_ptr;

		}

		cleanup.Dismiss();

		return S_OK;
		
	}
	
}

/////////////////// COMPILE HLSL ///////////////////////////

HRESULT gi_lib::dx11::CompileHLSL(const string& HLSL, const string& source_file, const string& entry_point, const string& profile, ID3DBlob** bytecode, wstring* error_string){

#ifdef _DEBUG

	UINT compilation_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_SKIP_OPTIMIZATION;

	D3D_SHADER_MACRO shader_macros[2] = { { "_DEBUG", "" }, { nullptr, nullptr } };

#else

	UINT compilation_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3;

	const D3D_SHADER_MACRO* shader_macros = nullptr;

#endif

	ID3DBlob * errors;

	auto hr = D3DCompile(HLSL.c_str(),
							HLSL.length(),
							source_file.c_str(),
							shader_macros,
							D3D_COMPILE_STANDARD_FILE_INCLUDE,
							entry_point.c_str(),
							profile.c_str(),
							compilation_flags,
							0,
							bytecode,
							&errors);

	if (FAILED(hr) &&
		error_string){

		COM_GUARD(errors);

		string err_string = static_cast<char*>(errors->GetBufferPointer());

		*error_string = wstring(err_string.begin(), err_string.end());

	}

	return hr;

}

/////////////////// SHADER TRAITS - VERTEX SHADER ///////////////////////////

const ShaderType ShaderTraits < ID3D11VertexShader >::flag = ShaderType::VERTEX_SHADER;

const char* ShaderTraits < ID3D11VertexShader >::entry_point = "VSMain";

const char* ShaderTraits < ID3D11VertexShader >::profile = "vs_5_0";

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

const char* ShaderTraits < ID3D11HullShader >::entry_point = "HSMain";

const char* ShaderTraits < ID3D11HullShader >::profile = "hs_5_0";

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

const char* ShaderTraits < ID3D11DomainShader >::entry_point = "DSMain";

const char* ShaderTraits < ID3D11DomainShader >::profile = "ds_5_0";

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

const char* ShaderTraits < ID3D11GeometryShader >::entry_point = "GSMain";

const char* ShaderTraits < ID3D11GeometryShader >::profile = "gs_5_0";

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

const char* ShaderTraits < ID3D11PixelShader >::entry_point = "PSMain";

const char* ShaderTraits < ID3D11PixelShader >::profile = "ps_5_0";

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

/////////////////// SHADER TRAITS - COMPUTE SHADER //////////////////////////

const ShaderType ShaderTraits < ID3D11ComputeShader >::flag = ShaderType::COMPUTE_SHADER;

const char* ShaderTraits < ID3D11ComputeShader >::entry_point = "CSMain";

const char* ShaderTraits < ID3D11ComputeShader >::profile = "cs_5_0";

HRESULT ShaderTraits < ID3D11ComputeShader >::MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11ComputeShader** shader, ShaderReflection* reflection, wstring* errors){

	auto make_compute_shader = [&device](ID3DBlob& bytecode, ID3D11ComputeShader** shader_ptr)
	{

		return device.CreateComputeShader(bytecode.GetBufferPointer(),
										  bytecode.GetBufferSize(),
										  nullptr,
										  shader_ptr);

	};

	return ::MakeShader<ID3D11ComputeShader>(HLSL,
											 source_file,
											 make_compute_shader,
											 shader,
											 reflection,
											 errors);
	
}
