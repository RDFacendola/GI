#pragma comment(lib, "d3dcompiler.lib")

#include "dx11shader.h"

#include <d3dcompiler.h>

#include <sstream>
#include <algorithm>

#include "..\..\include\exceptions.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::windows;

namespace{

#ifdef _DEBUG
		
	/// \brief Shader macros used during compilation.
	D3D_SHADER_MACRO shader_macros[2] = { { "_DEBUG", "" }, { nullptr, nullptr } };

#else

	/// \brief Shader macros used during compilation.
	const D3D_SHADER_MACRO* shader_macros = nullptr;

#endif

	/// \brief Reflect more shader buffers.
	void ReflectMoreBuffers(ID3D11ShaderReflection& reflector, vector<BufferReflection>& reflection){

		D3D11_SHADER_DESC shader_desc;
		D3D11_SHADER_BUFFER_DESC buffer_desc;
		D3D11_SHADER_VARIABLE_DESC variable_desc;

		reflector.GetDesc(&shader_desc);

		// Constant buffers and variables

		for (unsigned int cbuffer_index = 0; cbuffer_index < shader_desc.ConstantBuffers; ++cbuffer_index){

			auto buffer = reflector.GetConstantBufferByIndex(cbuffer_index);

			buffer->GetDesc(&buffer_desc);

			string cbuffer_name = buffer_desc.Name;

			auto it = std::find_if(reflection.begin(),
								   reflection.end(),
								   [&cbuffer_name](const BufferReflection& reflection){

										return reflection.buffer_name == cbuffer_name;

								   });

			if (it == reflection.end()){

				// Add a new constant buffer

				BufferReflection buffer_reflection;

				buffer_reflection.buffer_name = std::move(cbuffer_name);
				buffer_reflection.size = buffer_desc.Size;
				
				for (unsigned int variable_index = 0; variable_index < buffer_desc.Variables; ++variable_index){

					// Add a new variable

					auto variable = buffer->GetVariableByIndex(variable_index);

					variable->GetDesc(&variable_desc);

					buffer_reflection.variables.push_back({ variable_desc.Name,
															variable_desc.Size,
															variable_desc.StartOffset });

				}

				reflection.push_back(std::move(buffer_reflection));

			}

		}

	}

	void ReflectResources(ID3D11ShaderReflection& reflector, vector<string>& buffer_sequence){

		D3D11_SHADER_DESC shader_desc;

		D3D11_SHADER_INPUT_BIND_DESC resource_desc;

		reflector.GetDesc(&shader_desc);

		buffer_sequence.resize(shader_desc.ConstantBuffers);

		for (unsigned int resource_index = 0; resource_index < shader_desc.BoundResources; ++resource_index){

			reflector.GetResourceBindingDesc(resource_index, &resource_desc);

			switch (resource_desc.Type){

			case D3D_SIT_CBUFFER:
			case D3D_SIT_TBUFFER:

				// Constant or Texture buffer (they are basically the same thing, however tbuffers are optimized for random access, such as lookup tables)

				buffer_sequence[resource_desc.BindPoint] = resource_desc.Name;
				break;

			case D3D_SIT_TEXTURE:

				break;

			case D3D10_SIT_SAMPLER:

				break;

			}

		}

	}
	
}

// ShaderTypeInfo constants

const char * ShaderTypeInfo<ID3D11VertexShader>::kEntryPoint = "VSMain";
const char * ShaderTypeInfo<ID3D11VertexShader>::kShaderProfile = "vs_5_0";

const char * ShaderTypeInfo<ID3D11HullShader>::kEntryPoint = "HSMain";
const char * ShaderTypeInfo<ID3D11HullShader>::kShaderProfile = "hs_5_0";

const char * ShaderTypeInfo<ID3D11DomainShader>::kEntryPoint = "DSMain";
const char * ShaderTypeInfo<ID3D11DomainShader>::kShaderProfile = "ds_5_0";

const char * ShaderTypeInfo<ID3D11GeometryShader>::kEntryPoint = "GSMain";
const char * ShaderTypeInfo<ID3D11GeometryShader>::kShaderProfile = "gs_5_0";

const char * ShaderTypeInfo<ID3D11PixelShader>::kEntryPoint = "PSMain";
const char * ShaderTypeInfo<ID3D11PixelShader>::kShaderProfile = "ps_5_0";

// ShaderHelper

ID3D11Buffer * ShaderHelper::MakeConstantBufferOrDie(ID3D11Device & device, size_t size){

	ID3D11Buffer* cbuffer = nullptr;

	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.ByteWidth = static_cast<unsigned int>(size);
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	// Create the buffer with the device.
	THROW_ON_FAIL(device.CreateBuffer(&buffer_desc,
									  nullptr,
									  &cbuffer));

	return cbuffer;

}

unique_ptr<ID3DBlob, COMDeleter> ShaderHelper::Compile(const string& code, const string& source_file, const char * entry_point, const char * shader_profile, bool compulsory){

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
							shader_macros,
							D3D_COMPILE_STANDARD_FILE_INCLUDE,
							entry_point,
							shader_profile,
							compilation_flags,
							0,
							&bytecode,
							&errors);

	COM_GUARD(errors);

	if (FAILED(hr)){

		if (compulsory){

			wstringstream stream;

			string error_string = static_cast<char *>(errors->GetBufferPointer());

			stream << std::to_wstring(hr) << L" - " << wstring(error_string.begin(), error_string.end());

			THROW(stream.str());

		}
		else{

			return nullptr;

		}

	}

	return unique_ptr<ID3DBlob, COMDeleter>(bytecode, COMDeleter{});

}

void ShaderHelper::ReflectMoreOrDie(ID3DBlob& bytecode, ShaderReflection& reflection){

	ID3D11ShaderReflection * reflector = nullptr;

	THROW_ON_FAIL(D3DReflect(bytecode.GetBufferPointer(),
							 bytecode.GetBufferSize(),
							 IID_ID3D11ShaderReflection,
							 (void**)&reflector));
		
	ReflectMoreBuffers(*reflector, reflection.buffers);

}