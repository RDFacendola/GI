/// \file dx11shader.h
/// \brief This file contains classes used to manage DirectX11 shader and shader reflection.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <d3d11.h>
#include <string>

#include "enums.h"
#include "eigen.h"

#ifdef _WIN32

namespace gi_lib{

	namespace dx11{

		/// \brief Shader type.
		enum class ShaderType : unsigned int{

			VERTEX_SHADER,		///< \brief Vertex shader.
			HULL_SHADER,		///< \brief Hull shader.
			DOMAIN_SHADER,		///< \brief Domain shader.
			GEOMETRY_SHADER,	///< \brief Geometry shader.
			PIXEL_SHADER,		///< \brief Pixel shader.
			COMPUTE_SHADER,		///< \brief Compute shader.

		};

		/// \brief Type of a shader resource.
		enum class ShaderResourceType : unsigned int{

			UNKNOWN,			///< \brief Unknown resource.
			TEXTURE_1D,			///< \brief 1D texture.
			TEXTURE_2D,			///< \brief 2D texture.
			TEXTURE_3D,			///< \brief 3D texture.
			TEXTURE_CUBE,		///< \brief Cube texture.
			BUFFER				///< \brief Generic buffer.

		};

		/// \brief Description of a shader variable.
		struct ShaderVariableDesc{

			std::string name;							///< \brief Name of the variable.

			size_t size;								///< \brief Size of the variable.

			size_t offset;								///< \brief Offset of the variable.

		};

		/// \brief Description of a shader buffer (tbuffer or cbuffer).
		struct ShaderBufferDesc{

			std::string name;								///< \brief Name of the buffer.

			size_t size;									///< \brief Size of the buffer.

			std::vector<ShaderVariableDesc> variables;		///< \brief Variables inside the buffer.

			unsigned int slot;								///< \brief Binding slot.

		};

		/// \brief Description of a shader resource view (textures, structured buffers, ...).
		struct ShaderSRVDesc{

			std::string name;							///< \brief Name of the resource.

			ShaderResourceType type;					///< \brief Resource type.

			unsigned int elements;						///< \brief Elements in case of a resource array.

			unsigned int slot;							///< \brief Initial binding slot.

		};

		/// \brief Description of a shader unordered access view (RWTextures, RWStructuredBuffers, ...).
		struct ShaderUAVDesc{

			std::string name;							///< \brief Name of the unordered resource.

			ShaderResourceType type;					///< \brief Resource type.

			unsigned int slot;							///< \brief Binding slot.

		};

		/// \brief Description of a shader sampler.
		struct ShaderSamplerDesc{

			std::string name;							///< \brief Name of the sampler.

			unsigned int slot;							///< \brief Binding slot.

		};

		/// \brief Represents a single element of a vertex.
		struct InputElementReflection {

			std::string semantic;			///< \brief Semantic of the input element.

			unsigned int offset;			///< \brief Offset of the element from the beginning of the vertex, in bytes.

			unsigned int index;				///< \brief Progressive indec, in case the semantic refers to an array of elements.
			
			DXGI_FORMAT format;							///< \brief Format of the element.

		};

		/// \brief Additional description of a pixel shader.
		struct PixelShaderReflection {

			unsigned int render_targets_;	///< \brief Expected number of output render targets.

		};

		/// \bief Additional description of a compute shader.
		struct ComputeShaderReflection{

			unsigned int thread_group_x;	///< \brief Number of threads along the X axis.
			unsigned int thread_group_y;	///< \brief Number of threads along the Y axis.
			unsigned int thread_group_z;	///< \brief Number of threads along the Z axis.

		};

		struct VertexShaderReflection {

			std::vector<InputElementReflection> vertex_input;		///< \brief Elements expected as input of the vertex shader.

		};

		/// \brief Description of a shader.
		struct ShaderReflection{

			ShaderType shader_type;									///< \brief Shader type this reflection refers to.

			std::vector<ShaderBufferDesc> buffers;					///< \brief List of buffer descriptions.

			std::vector<ShaderSRVDesc> shader_resource_views;		///< \brief List of SRV descriptions.

			std::vector<ShaderSamplerDesc> samplers;				///< \brief List of sampler descriptions.

			std::vector<ShaderUAVDesc> unordered_access_views;		///< \brief List of UAV descriptions.

			// Shader-specific reflection

			union{

				PixelShaderReflection pixel_shader;					///< \brief Pixel-shader specific reflection. Valid only if shader_type is "PIXEL_SHADER".

				ComputeShaderReflection compute_shader;				///< \brief Compute-shader specific reflection. Valid only if shader_type is "COMPUTE_SHADER".
				

			};
			
			VertexShaderReflection vertex_shader;				///< \brief Vertex-shader specific reflection. Valid only if shader_type is "VERTEX_SHADER".

		};

		/// \brief Shader type traits.
		template <typename TShader>
		struct ShaderTraits;

		/// \brief Vertex shader type traits.
		template<> struct ShaderTraits < ID3D11VertexShader > {

			static const ShaderType flag;		///< \brief Flag used to identify the shader type.

			static const char* entry_point;		///< \brief Entry point.

			static const char* profile;			///< \brief Shader profile.

			/// \brief Create a shader from HLSL code.
			/// \param device Device used to create the shader.
			/// \param HLSL HLSL code to compile.
			/// \param source_file Used to resolve #include directives.
			/// \param shader Pointer to the shader that will contain the result. Set to null to ignore the object.
			/// \param reflection Pointer to a pre-filled shader reflection. Set to null to ignore the reflection.
			/// \param errors Pointer to a string that will contain the compilation errors if the the method fails. Set to null to ignore.
			static HRESULT MakeShader(ID3D11Device& device, const std::string& HLSL, const std::string& source_file, ID3D11VertexShader** shader, ShaderReflection* reflection, std::wstring* errors);

		};

		/// \brief Hull shader type traits.
		template<> struct ShaderTraits < ID3D11HullShader > {

			static const ShaderType flag;		///< \brief Flag used to identify the shader type.

			static const char* entry_point;		///< \brief Entry point.

			static const char* profile;			///< \brief Shader profile.

			/// \brief Create a shader from HLSL code.
			/// \param device Device used to create the shader.
			/// \param HLSL HLSL code to compile.
			/// \param source_file Used to resolve #include directives.
			/// \param shader Pointer to the shader that will contain the result. Set to null to ignore the object.
			/// \param reflection Pointer to a pre-filled shader reflection. Set to null to ignore the reflection.
			/// \param errors Pointer to a string that will contain the compilation errors if the the method fails. Set to null to ignore.
			static HRESULT MakeShader(ID3D11Device& device, const std::string& HLSL, const std::string& source_file, ID3D11HullShader** shader, ShaderReflection* reflection, std::wstring* errors);

		};

		/// \brief Domain shader type traits.
		template<> struct ShaderTraits < ID3D11DomainShader > {

			static const ShaderType flag;		///< \brief Flag used to identify the shader type.

			static const char* entry_point;		///< \brief Entry point.

			static const char* profile;			///< \brief Shader profile.

			/// \brief Create a shader from HLSL code.
			/// \param device Device used to create the shader.
			/// \param HLSL HLSL code to compile.
			/// \param source_file Used to resolve #include directives.
			/// \param shader Pointer to the shader that will contain the result. Set to null to ignore the object.
			/// \param reflection Pointer to a pre-filled shader reflection. Set to null to ignore the reflection.
			/// \param errors Pointer to a string that will contain the compilation errors if the the method fails. Set to null to ignore.
			static HRESULT MakeShader(ID3D11Device& device, const std::string& HLSL, const std::string& source_file, ID3D11DomainShader** shader, ShaderReflection* reflection, std::wstring* errors);

		};

		/// \brief Geometry shader type traits.
		template<> struct ShaderTraits < ID3D11GeometryShader > {

			static const ShaderType flag;		///< \brief Flag used to identify the shader type.

			static const char* entry_point;		///< \brief Entry point.

			static const char* profile;			///< \brief Shader profile. 

			/// \brief Create a shader from HLSL code.
			/// \param device Device used to create the shader.
			/// \param HLSL HLSL code to compile.
			/// \param source_file Used to resolve #include directives.
			/// \param shader Pointer to the shader that will contain the result. Set to null to ignore the object.
			/// \param reflection Pointer to a pre-filled shader reflection. Set to null to ignore the reflection.
			/// \param errors Pointer to a string that will contain the compilation errors if the the method fails. Set to null to ignore.
			static HRESULT MakeShader(ID3D11Device& device, const std::string& HLSL, const std::string& source_file, ID3D11GeometryShader** shader, ShaderReflection* reflection, std::wstring* errors);

		};

		/// \brief Pixel shader type traits.
		template<> struct ShaderTraits < ID3D11PixelShader > {

			static const ShaderType flag;		///< \brief Flag used to identify the shader type.

			static const char* entry_point;		///< \brief Entry point.

			static const char* profile;			///< \brief Shader profile.

			/// \brief Create a shader from HLSL code.
			/// \param device Device used to create the shader.
			/// \param HLSL HLSL code to compile.
			/// \param source_file Used to resolve #include directives.
			/// \param shader Pointer to the shader that will contain the result. Set to null to ignore the object.
			/// \param reflection Pointer to a pre-filled shader reflection. Set to null to ignore the reflection.
			static HRESULT MakeShader(ID3D11Device& device, const std::string& HLSL, const std::string& source_file, ID3D11PixelShader** shader, ShaderReflection* reflection, std::wstring* errors);

		};

		/// \brief Compute shader type traits.
		template<> struct ShaderTraits < ID3D11ComputeShader > {

			static const ShaderType flag;		///< \brief Flag used to identify the shader type.

			static const char* entry_point;		///< \brief Entry point.

			static const char* profile;			///< \brief Shader profile.

			/// \brief Create a compute shader from HLSL code.
			/// \param device Device used to create the shader.
			/// \param HLSL HLSL code to compile.
			/// \param source_file Used to resolve #include directives.
			/// \param shader Pointer to the shader that will contain the result. Set to null to ignore the object.
			/// \param reflection Pointer to a shader reflection. Set to null to ignore the reflection.
			static HRESULT MakeShader(ID3D11Device& device, const std::string& HLSL, const std::string& source_file, ID3D11ComputeShader** shader, ShaderReflection* reflection, std::wstring* errors);

		};

		/// \brief Create a shader from HLSL code.
		/// \param device Device used to create the shader.
		/// \param HLSL HLSL code to compile.
		/// \param source_file Used to resolve #include directives.
		/// \param shader Pointer to the shader that will contain the result. Set to null to ignore the object.
		/// \param reflection Pointer to the object that will contain the reflection. Set to null to ignore the reflection.
		/// \param errors Pointer to a string that will contain the compilation errors if the the method fails. Set to null to ignore.
		template <typename TShader>
		HRESULT MakeShader(ID3D11Device& device, const std::string& HLSL, const std::string& source_file, TShader** shader, ShaderReflection* reflection = nullptr, std::wstring* errors = nullptr);
	
		/// \brief Compile an HLSL code returning a bytecode.
		/// \param HLSL HLSL code.
		/// \param source_file File containing the HLSL code, used to resolve the #include directives.
		/// \param bytecode If the method succeeds, it will contain the compiled bytecode.
		/// \param error_string If the method fails, it will contain the error string. Optional.
		template <typename TShader>
		HRESULT CompileHLSL(const std::string& HLSL, const std::string& source_file, ID3DBlob** bytecode, ShaderReflection* reflection = nullptr, std::wstring* error_string = nullptr);

		/// \brief Compile an HLSL code returning a bytecode.
		/// \param HLSL HLSL code.
		/// \param source_file File containing the HLSL code, used to resolve the #include directives.
		/// \param bytecode If the method succeeds, it will contain the compiled bytecode.
		/// \param reflection If the method succeeds, it will containg the reflection of the HLSL code.
		/// \param error_string If the method fails, it will contain the error string. Optional.
		HRESULT CompileHLSL(const std::string& HLSL, const std::string& source_file, const std::string& entry_point, const std::string& profile, ID3DBlob** bytecode, ShaderReflection* reflection = nullptr, std::wstring* error_string = nullptr);
				
	}

}

//////////////////////////////// COMPILE HLSL ///////////////////////////////////////////

template <typename TShader>
HRESULT gi_lib::dx11::CompileHLSL(const std::string& HLSL, const std::string& source_file, ID3DBlob** bytecode, ShaderReflection* reflection, std::wstring* error_string){

	return CompileHLSL(HLSL,
					   source_file,
					   ShaderTraits<TShader>::entry_point,
					   ShaderTraits<TShader>::profile,
					   bytecode,
					   reflection,
					   error_string);

}

//////////////////////////////// MAKE SHADER ///////////////////////////////////////

template <typename TShader>
inline HRESULT gi_lib::dx11::MakeShader(ID3D11Device& device, const std::string& HLSL, const std::string& source_file, TShader** shader, ShaderReflection* reflection, std::wstring* errors){

	return ShaderTraits<TShader>::MakeShader(device, 
											 HLSL, 
											 source_file, 
											 shader, 
											 reflection, 
											 errors);

}

#endif