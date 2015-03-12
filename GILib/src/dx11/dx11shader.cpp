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

	/// \brief Shader type traits.
	template <typename TShader> 
	struct ShaderTraits;

	/// \brief Vertex shader type traits.
	template<> struct ShaderTraits < ID3D11VertexShader > {

		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	/// \brief hull shader type traits.
	template<> struct ShaderTraits < ID3D11HullShader > {

		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	/// \brief domain shader type traits.
	template<> struct ShaderTraits < ID3D11DomainShader > {

		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	/// \brief geomtry shader type traits.
	template<> struct ShaderTraits < ID3D11GeometryShader > {

		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile. 

	};

	/// \brief pixel shader type traits.
	template<> struct ShaderTraits < ID3D11PixelShader > {

		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	const char * ShaderTraits < ID3D11VertexShader >::entry_point = "VSMain";
	const char * ShaderTraits < ID3D11VertexShader >::profile = "vs_5_0";

	const char * ShaderTraits < ID3D11HullShader >::entry_point = "HSMain";
	const char * ShaderTraits < ID3D11HullShader >::profile = "hs_5_0";

	const char * ShaderTraits < ID3D11DomainShader >::entry_point = "DSMain";
	const char * ShaderTraits < ID3D11DomainShader >::profile = "ds_5_0";
	
	const char * ShaderTraits < ID3D11GeometryShader >::entry_point = "GSMain";
	const char * ShaderTraits < ID3D11GeometryShader >::profile = "gs_5_0";

	const char * ShaderTraits < ID3D11PixelShader >::entry_point = "PSMain";
	const char * ShaderTraits < ID3D11PixelShader >::profile = "ps_5_0";

#ifdef _DEBUG
		
	/// \brief Shader macros used during compilation.
	D3D_SHADER_MACRO shader_macros[2] = { { "_DEBUG", "" }, { nullptr, nullptr } };

#else

	/// \brief Shader macros used during compilation.
	const D3D_SHADER_MACRO* shader_macros = nullptr;

#endif

	void ReflectBuffers(ID3D11ShaderReflection& reflector, ShaderReflection& reflection){

		D3D11_SHADER_DESC dx_shader_desc;
		D3D11_SHADER_BUFFER_DESC dx_buffer_desc;
		D3D11_SHADER_VARIABLE_DESC dx_variable_desc;

		auto& buffers = reflection.buffers;

		reflector.GetDesc(&dx_shader_desc);

		// Constant buffers and variables

		for (unsigned int cbuffer_index = 0; cbuffer_index < dx_shader_desc.ConstantBuffers; ++cbuffer_index){

			auto buffer = reflector.GetConstantBufferByIndex(cbuffer_index);

			buffer->GetDesc(&dx_buffer_desc);

			auto it = std::find_if(buffers.begin(),
								   buffers.end(),
								   [&dx_buffer_desc](const ShaderBufferDesc& desc){

										return desc.name == dx_buffer_desc.Name;

								   });

			if (it == buffers.end()){

				// Add a new constant buffer

				ShaderBufferDesc buffer_desc;

				buffer_desc.name = dx_buffer_desc.Name;
				buffer_desc.size = dx_buffer_desc.Size;

				for (unsigned int variable_index = 0; variable_index < dx_buffer_desc.Variables; ++variable_index){

					// Add a new variable

					auto variable = buffer->GetVariableByIndex(variable_index);

					variable->GetDesc(&dx_variable_desc);

					buffer_desc.variables.push_back({ dx_variable_desc.Name,
													  dx_variable_desc.Size,
													  dx_variable_desc.StartOffset });

				}

				buffers.push_back(std::move(buffer_desc));

			}

		}

	}

	void Reflect(ShaderBinding& binding, ShaderReflection& reflection){

		ID3D11ShaderReflection * reflector = nullptr;

		THROW_ON_FAIL(D3DReflect(binding.bytecode->GetBufferPointer(),
								 binding.bytecode->GetBufferSize(),
								 IID_ID3D11ShaderReflection,
								 (void**)&reflector));

		ReflectBuffers(*reflector, reflection);

	}

	template <typename TShader>
	ID3DBlob * Compile(const char* code, size_t size, const char* source_file, bool compulsory){

		ID3DBlob * bytecode = nullptr;
		ID3DBlob * errors = nullptr;

	#ifdef _DEBUG

		UINT compilation_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_SKIP_OPTIMIZATION;

	#else

		UINT compilation_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3;

	#endif

		HRESULT hr = D3DCompile(code,
								size,
								source_file,
								shader_macros,
								D3D_COMPILE_STANDARD_FILE_INCLUDE,
								ShaderTraits<TShader>::entry_point,
								ShaderTraits<TShader>::profile,
								compilation_flags,
								0,
								&bytecode,
								&errors);

		COM_GUARD(errors);

		if (FAILED(hr)){

			if (compulsory){

				wstringstream stream;

				string error_string = static_cast<char *>(errors->GetBufferPointer());

				stream << L"0x" << std::hex << std::to_wstring(hr) << std::dec << L" - " << wstring(error_string.begin(), error_string.end());

				THROW(stream.str());

			}
			else{

				return nullptr;

			}

		}
		else{

			return bytecode;

		}

	}

	template <typename TShader>
	void CompileShader(const char* code, size_t size, const char* source_file, bool compulsory, ShaderBinding& binding, ShaderReflection& reflection){

		
		binding.bytecode = shared_ptr<ID3DBlob>( Compile<TShader>(code,
																  size, 
																  source_file, 
																  compulsory),
												 COMDeleter{});

		if (binding.bytecode){

			Reflect(binding,
					reflection);

		}
		
	}

}

//////////////////// ShaderHelper //////////////////////////

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

ShaderCombo ShaderHelper::CompileShadersOrDie(const char* code, size_t size, const char* source_file, unsigned int shaders, unsigned int compulsory_shaders){

	ShaderCombo shader_combo;
	
	if ((shaders & kVertexShader) > 0){

		CompileShader<ID3D11VertexShader>(code,
										  size,
										  source_file,
										  (compulsory_shaders & kVertexShader) > 0,
										  shader_combo.vertex_shader,
										  shader_combo.reflection);

	}

	if ((shaders & kHullShader) > 0){

		CompileShader<ID3D11HullShader>(code,
										size,
										source_file,
										(compulsory_shaders & kHullShader) > 0,
										shader_combo.hull_shader,
										shader_combo.reflection);

	}

	if ((shaders & kDomainShader) > 0){

		CompileShader<ID3D11DomainShader>(code,
										  size,
										  source_file,
										  (compulsory_shaders & kDomainShader) > 0,
										  shader_combo.domain_shader,
										  shader_combo.reflection);

	}

	if ((shaders & kGeometryShader) > 0){

		CompileShader<ID3D11GeometryShader>(code,
											size,
											source_file,
											(compulsory_shaders & kGeometryShader) > 0,
											shader_combo.geometry_shader,
											shader_combo.reflection);

	}

	if ((shaders & kPixelShader) > 0){

		CompileShader<ID3D11PixelShader>(code,
										 size,
										 source_file,
										 (compulsory_shaders & kPixelShader) > 0,
										 shader_combo.pixel_shader,
										 shader_combo.reflection);

	}

	return shader_combo;

}

