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

		static const unsigned int flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.
		

	};

	/// \brief hull shader type traits.
	template<> struct ShaderTraits < ID3D11HullShader > {

		static const unsigned int flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	/// \brief domain shader type traits.
	template<> struct ShaderTraits < ID3D11DomainShader > {

		static const unsigned int flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	/// \brief geomtry shader type traits.
	template<> struct ShaderTraits < ID3D11GeometryShader > {

		static const unsigned int flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile. 

	};

	/// \brief pixel shader type traits.
	template<> struct ShaderTraits < ID3D11PixelShader > {

		static const unsigned int flag;		///< \brief Flag used to identify the shader type.
		static const char * entry_point;	///< \brief Entry point.
		static const char * profile;		///< \brief Shader profile.

	};

	const unsigned int ShaderTraits < ID3D11VertexShader >::flag = ShaderHelper::kVertexShader;
	const char * ShaderTraits < ID3D11VertexShader >::entry_point = "VSMain";
	const char * ShaderTraits < ID3D11VertexShader >::profile = "vs_5_0";

	const unsigned int ShaderTraits < ID3D11HullShader >::flag = ShaderHelper::kHullShader;
	const char * ShaderTraits < ID3D11HullShader >::entry_point = "HSMain";
	const char * ShaderTraits < ID3D11HullShader >::profile = "hs_5_0";

	const unsigned int ShaderTraits < ID3D11DomainShader >::flag = ShaderHelper::kDomainShader;
	const char * ShaderTraits < ID3D11DomainShader >::entry_point = "DSMain";
	const char * ShaderTraits < ID3D11DomainShader >::profile = "ds_5_0";
	
	const unsigned int ShaderTraits < ID3D11GeometryShader >::flag = ShaderHelper::kGeometryShader;
	const char * ShaderTraits < ID3D11GeometryShader >::entry_point = "GSMain";
	const char * ShaderTraits < ID3D11GeometryShader >::profile = "gs_5_0";

	const unsigned int ShaderTraits < ID3D11PixelShader >::flag = ShaderHelper::kPixelShader;
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
	template <typename TType, typename TReflector>
	void Reflect(ID3D11ShaderReflection& reflector, const D3D11_SHADER_INPUT_BIND_DESC& input_desc, TReflector ResourceReflector, vector<TType>& resources, vector<unsigned int>& order){

		auto it = std::find_if(resources.begin(),
							   resources.end(),
							   [&input_desc](const TType& desc){

									return desc.name == input_desc.Name;

							   });

		if (it == resources.end()){

			resources.push_back(ResourceReflector(reflector, input_desc));

			order[input_desc.BindPoint] = static_cast<unsigned int>(resources.size() - 1);

		}
		else{

			order[input_desc.BindPoint] = static_cast<unsigned int>(std::distance(resources.begin(), it));

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
	ShaderResourceDesc ReflectTexture(ID3D11ShaderReflection&, const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

		return ShaderResourceDesc{ input_desc.Name,
								   SRVDimensionToShaderResourceType(input_desc.Dimension),
								   input_desc.BindCount };

	}

	/// \brief Reflect a sampler.
	/// \param reflector Reflector used to perform the reflection.
	/// \param input_desc Description of the shader input.
	/// \return Return the description of the reflected sampler.
	ShaderSamplerDesc ReflectSampler(ID3D11ShaderReflection&, const D3D11_SHADER_INPUT_BIND_DESC& input_desc){

		return ShaderSamplerDesc{ input_desc.Name };

	}

	/// \brief Reflect a shader from bytecode.
	/// \param binding Holds information about the shader and the binding order of the resources.
	/// \param reflection Holds shared information about various shaders as well as detailed information about resources.
	void Reflect(ShaderBinding& binding, ShaderReflection& reflection){

		ID3D11ShaderReflection * reflector = nullptr;

		THROW_ON_FAIL(D3DReflect(binding.bytecode->GetBufferPointer(),
								 binding.bytecode->GetBufferSize(),
								 IID_ID3D11ShaderReflection,
								 (void**)&reflector));

		D3D11_SHADER_DESC shader_desc;

		D3D11_SHADER_INPUT_BIND_DESC resource_desc;

		reflector->GetDesc(&shader_desc);

		// Resize the arrays
		binding.buffer_order.resize(shader_desc.ConstantBuffers);									// The size is known.
		binding.resources_order.resize(shader_desc.BoundResources - shader_desc.ConstantBuffers);	// Worst case scenario, the size is unknown at this stage.
		binding.samplers_order.resize(shader_desc.BoundResources - shader_desc.ConstantBuffers);	// Worst case scenario, the size is unknown at this stage.

		int max_resource_index = -1;
		int max_sampler_index = -1;

		for (unsigned int resource_index = 0; resource_index < shader_desc.BoundResources; ++resource_index){

			reflector->GetResourceBindingDesc(resource_index, &resource_desc);

			switch (resource_desc.Type){

				case D3D_SIT_CBUFFER:
				case D3D_SIT_TBUFFER:
				{

					// Constant or Texture buffer

					Reflect(*reflector,
							resource_desc,
							ReflectCBuffer,
							reflection.buffers,
							binding.buffer_order);

					break;

				}
				case D3D_SIT_TEXTURE:
				{

					// Textures 

					max_resource_index = (std::max)(max_resource_index, static_cast<int>(resource_desc.BindPoint));

					Reflect(*reflector,
							resource_desc,
							ReflectTexture,
							reflection.resources,
							binding.resources_order);

					break;

				}
				case D3D_SIT_SAMPLER:
				{

					// Samplers

					max_sampler_index = (std::max)(max_sampler_index, static_cast<int>(resource_desc.BindPoint));

					Reflect(*reflector,
							resource_desc,
							ReflectSampler,
							reflection.samplers,
							binding.samplers_order);

					break;

				}

			}

		}

		// Resize the resource and the sampler vectors to the right size
		binding.resources_order.resize(max_resource_index + 1);
		binding.samplers_order.resize(max_sampler_index + 1);

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
	template <typename TShader>
	void CompileShader(const char* code, size_t size, const char* source_file, unsigned int shaders, unsigned int compulsory_shaders, ShaderBinding& binding, ShaderReflection& reflection){

		if ((shaders & ShaderTraits<TShader>::flag) > 0){

			binding.bytecode = shared_ptr<ID3DBlob>(Compile<TShader>(code,
													size,
													source_file,
													(compulsory_shaders & ShaderTraits<TShader>::flag) > 0),
													COMDeleter{});

			if (binding.bytecode){

				Reflect(binding,
					reflection);

			}

		}
				
	}

}

//////////////////// ShaderBinding /////////////////////////

ShaderBinding::ShaderBinding():
bytecode(nullptr){}

ShaderBinding::ShaderBinding(const ShaderBinding& other){

	bytecode = other.bytecode;
	buffer_order = other.buffer_order;
	resources_order = other.resources_order;
	samplers_order = other.samplers_order;

}

ShaderBinding::ShaderBinding(ShaderBinding&& other){

	bytecode = std::move(other.bytecode);
	buffer_order = std::move(other.buffer_order);
	resources_order = std::move(other.resources_order);
	samplers_order = std::move(other.samplers_order);

}

ShaderBinding& ShaderBinding::operator=(ShaderBinding other){

	other.Swap(*this);

	return *this;

}

void ShaderBinding::Swap(ShaderBinding& other){

	std::swap(bytecode, other.bytecode);
	std::swap(buffer_order, other.buffer_order);
	std::swap(resources_order, other.resources_order);
	std::swap(samplers_order, other.samplers_order);

}

//////////////////// ShaderCombo ///////////////////////////

ShaderCombo::ShaderCombo(){}

ShaderCombo::ShaderCombo(const ShaderCombo& other){

	vertex_shader = other.vertex_shader;
	hull_shader = other.hull_shader;
	domain_shader = other.domain_shader;
	geometry_shader = other.geometry_shader;
	pixel_shader = other.pixel_shader;
	reflection = other.reflection;

}

ShaderCombo::ShaderCombo(ShaderCombo&& other){

	vertex_shader = std::move(other.vertex_shader);
	hull_shader = std::move(other.hull_shader);
	domain_shader = std::move(other.domain_shader);
	geometry_shader = std::move(other.geometry_shader);
	pixel_shader = std::move(other.pixel_shader);
	reflection = other.reflection;

}

ShaderCombo& ShaderCombo::operator=(ShaderCombo other){

	other.Swap(*this);

	return *this;

}

void ShaderCombo::Swap(ShaderCombo& other){

	std::swap(vertex_shader, other.vertex_shader);
	std::swap(hull_shader, other.hull_shader);
	std::swap(domain_shader, other.domain_shader);
	std::swap(geometry_shader, other.geometry_shader);
	std::swap(pixel_shader, other.pixel_shader);
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

ShaderCombo ShaderHelper::CompileShadersOrDie(const char* code, size_t size, const char* source_file, unsigned int shaders, unsigned int compulsory_shaders){

	ShaderCombo shader_combo;
	
	CompileShader<ID3D11VertexShader>(code,
										size,
										source_file,
										shaders,
										compulsory_shaders,
										shader_combo.vertex_shader,
										shader_combo.reflection);

	CompileShader<ID3D11HullShader>(code,
									size,
									source_file,
									shaders,
									compulsory_shaders,
									shader_combo.hull_shader,
									shader_combo.reflection);


	CompileShader<ID3D11DomainShader>(code,
									  size,
									  source_file,
									  shaders,
									  compulsory_shaders,
									  shader_combo.domain_shader,
									  shader_combo.reflection);

	CompileShader<ID3D11GeometryShader>(code,
										size,
										source_file,
										shaders,
										compulsory_shaders,
										shader_combo.geometry_shader,
										shader_combo.reflection);

	CompileShader<ID3D11PixelShader>(code,
									 size,
									 source_file,
									 shaders,
									 compulsory_shaders,
									 shader_combo.pixel_shader,
									 shader_combo.reflection);

	return shader_combo;

}

