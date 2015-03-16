#pragma comment(lib, "d3dcompiler.lib")

#include "dx11shader.h"

#include <d3dcompiler.h>

#include <sstream>
#include <algorithm>

#include "..\..\include\exceptions.h"
#include "..\..\include\enums.h"

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

		static const ShaderType flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.
		

	};

	/// \brief hull shader type traits.
	template<> struct ShaderTraits < ID3D11HullShader > {

		static const ShaderType flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	/// \brief domain shader type traits.
	template<> struct ShaderTraits < ID3D11DomainShader > {

		static const ShaderType flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	/// \brief geomtry shader type traits.
	template<> struct ShaderTraits < ID3D11GeometryShader > {

		static const ShaderType flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile. 

	};

	/// \brief pixel shader type traits.
	template<> struct ShaderTraits < ID3D11PixelShader > {

		static const ShaderType flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	const ShaderType ShaderTraits < ID3D11VertexShader >::flag = ShaderType::VERTEX_SHADER;
	const char * ShaderTraits < ID3D11VertexShader >::entry_point = "VSMain";
	const char * ShaderTraits < ID3D11VertexShader >::profile = "vs_5_0";

	const ShaderType ShaderTraits < ID3D11HullShader >::flag = ShaderType::HULL_SHADER;
	const char * ShaderTraits < ID3D11HullShader >::entry_point = "HSMain";
	const char * ShaderTraits < ID3D11HullShader >::profile = "hs_5_0";

	const ShaderType ShaderTraits < ID3D11DomainShader >::flag = ShaderType::DOMAIN_SHADER;
	const char * ShaderTraits < ID3D11DomainShader >::entry_point = "DSMain";
	const char * ShaderTraits < ID3D11DomainShader >::profile = "ds_5_0";
	
	const ShaderType ShaderTraits < ID3D11GeometryShader >::flag = ShaderType::GEOMETRY_SHADER;
	const char * ShaderTraits < ID3D11GeometryShader >::entry_point = "GSMain";
	const char * ShaderTraits < ID3D11GeometryShader >::profile = "gs_5_0";

	const ShaderType ShaderTraits < ID3D11PixelShader >::flag = ShaderType::PIXEL_SHADER;
	const char * ShaderTraits < ID3D11PixelShader >::entry_point = "PSMain";
	const char * ShaderTraits < ID3D11PixelShader >::profile = "ps_5_0";

#ifdef _DEBUG
		
	/// \brief Shader macros used during compilation.
	D3D_SHADER_MACRO shader_macros[2] = { { "_DEBUG", "" }, 
										  { nullptr, nullptr } };

#else

	/// \brief Shader macros used during compilation.
	const D3D_SHADER_MACRO* shader_macros = nullptr;

#endif

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
		
			default:

				return ShaderResourceType::UNKNOWN;

		}

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
	ShaderResourceDesc ReflectTexture(const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

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
	void Reflect(ID3DBlob& bytecode, ShaderReflection& reflection){

		ID3D11ShaderReflection * reflector = nullptr;

		THROW_ON_FAIL(D3DReflect(bytecode.GetBufferPointer(),
								 bytecode.GetBufferSize(),
								 IID_ID3D11ShaderReflection,
								 (void**)&reflector));

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
				{

					// Textures 

					Reflect(resource_desc,
							ReflectTexture,
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

	}

	/// \brief Compile an HLSL code.
	/// \tparam TShader Type of the shader to compile.
	/// \param code Pointer to the HLSL code buffer.
	/// \param size Size of the code buffer.
	/// \param source_file Null terminated string representing the source file. Used to resolve #include directives.
	/// \param compulsory Whether the presence of the given shader is compulsory or not.
	/// \return Returns a pointer to the compiled shader code if the method succeeds, otherwise the method will throw if the shader presence was compulsory or nullptr otherwise.
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

	/// \brief Compile and reflect a shader.
	/// \tparam TShader Type of the shader to handle.
	/// \param code Pointer to the HLSL code buffer.
	/// \param size Size of the code buffer.
	/// \param source_file Null terminated string representing the source file. Used to resolve #include directives.
	/// \param shaders Presence flags used to determine whether to compile a given shader or not.
	/// \param compulsory_shaders Flags used to determine whether the presence of the specified shader is compulsory or not. If a compulsory shader is not found the method will throw.
	/// \param binding Holds information about the shader and the binding order of the resources.
	/// \param reflection Holds shared information about various shaders as well as detailed information about resources.
	/// \return Returns the compiled bytecode if the method succeeds, returns nullptr otherwise.
	template <typename TShader>
	shared_ptr<ID3DBlob> CompileShader(const char* code, size_t size, const char* source_file, ShaderType shaders, ShaderType compulsory, ShaderReflection& reflection){

		if (shaders && ShaderTraits<TShader>::flag){

			auto bytecode = Compile<TShader>(code,
											 size,
											 source_file,
											 compulsory && ShaderTraits<TShader>::flag);

			if (bytecode){

				Reflect<TShader>(*bytecode, reflection);

				return shared_ptr<ID3DBlob>(bytecode, COMDeleter{});

			}

		}

		return nullptr;
				
	}

}

//////////////////// ShaderCombo ///////////////////////////

ShaderCombo::ShaderCombo(){}

ShaderCombo::ShaderCombo(const ShaderCombo& other){

	vs_bytecode = other.vs_bytecode;
	hs_bytecode = other.hs_bytecode;
	ds_bytecode = other.ds_bytecode;
	gs_bytecode = other.gs_bytecode;
	ps_bytecode = other.ps_bytecode;
	reflection = other.reflection;

}

ShaderCombo::ShaderCombo(ShaderCombo&& other){

	vs_bytecode = std::move(other.vs_bytecode);
	hs_bytecode = std::move(other.hs_bytecode);
	ds_bytecode = std::move(other.ds_bytecode);
	gs_bytecode = std::move(other.gs_bytecode);
	ps_bytecode = std::move(other.ps_bytecode);
	reflection = other.reflection;

}

ShaderCombo& ShaderCombo::operator=(ShaderCombo other){

	other.Swap(*this);

	return *this;

}

void ShaderCombo::Swap(ShaderCombo& other){

	std::swap(vs_bytecode, other.vs_bytecode);
	std::swap(hs_bytecode, other.hs_bytecode);
	std::swap(ds_bytecode, other.ds_bytecode);
	std::swap(gs_bytecode, other.gs_bytecode);
	std::swap(ps_bytecode, other.ps_bytecode);
	std::swap(reflection, other.reflection);

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

ShaderCombo ShaderHelper::CompileShadersOrDie(const char* code, size_t size, const char* source_file, ShaderType shaders, ShaderType compulsory){

	ShaderCombo shader_combo;
	
	shader_combo.vs_bytecode = CompileShader<ID3D11VertexShader>(code,
																 size,
																 source_file,
																 shaders,
																 compulsory,
																 shader_combo.reflection);

	shader_combo.hs_bytecode = CompileShader<ID3D11HullShader>(code,
															   size,
															   source_file,
															   shaders,
															   compulsory,
															   shader_combo.reflection);

	shader_combo.ds_bytecode = CompileShader<ID3D11DomainShader>(code,
																 size,
																 source_file,
																 shaders,
																 compulsory,
																 shader_combo.reflection);

	shader_combo.gs_bytecode = CompileShader<ID3D11GeometryShader>(code,
																   size,
																   source_file,
																   shaders,
																   compulsory,
																   shader_combo.reflection);

	shader_combo.ps_bytecode = CompileShader<ID3D11PixelShader>(code,
																size,
																source_file,
																shaders,
																compulsory,
																shader_combo.reflection);

	return shader_combo;

}

