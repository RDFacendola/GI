/// \file dx11.h
/// \brief Utility methods for DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <string>

#include <d3d11.h>
#include <d3dcompiler.h>

#include "..\..\include\enums.h"

using ::std::vector;
using ::std::string;
using ::std::wstring;

namespace gi_lib{

	namespace dx11{

		/// \brief Shader type. Flags can be combined with the | operator.
		ENUM_FLAGS(ShaderType, unsigned int){

			NONE = 0u,					///< \brief No shader.

			VERTEX_SHADER = 1u,			///< \brief Vertex shader.
			HULL_SHADER = 2u,			///< \brief Hull shader.
			DOMAIN_SHADER = 4u,			///< \brief Domain shader.
			GEOMETRY_SHADER = 8u,		///< \brief Geometry shader.
			PIXEL_SHADER = 16u,			///< \brief Pixel shader.

		};

		/// \brief Type of a shader resource.
		enum class ShaderResourceType : unsigned int{

			UNKNOWN,			///< \brief Unknown resource.
			TEXTURE_1D,			///< \brief 1D texture.
			TEXTURE_2D,			///< \brief 2D texture.
			TEXTURE_3D,			///< \brief 3D texture.
			TEXTURE_CUBE		///< \brief Cube texture.

		};

		/// \brief Texture mapping technique.
		enum class TextureMapping : unsigned int{

			WRAP,				///< \brief Repeat the texture for texture coordinates outside the boundary [0;1] every integer.
			CLAMP				///< \brief Texture coordinates below 0 or above 1 are set to 0 and 1 respectively instead.

		};

		/// \brief Description of a shader variable.
		struct ShaderVariableDesc{

			string name;								///< \brief Name of the variable.

			size_t size;								///< \brief Size of the variable.

			size_t offset;								///< \brief Offset of the variable.

		};

		/// \brief Description of a shader buffer (tbuffer or cbuffer).
		struct ShaderBufferDesc{

			string name;								///< \brief Name of the buffer.

			size_t size;								///< \brief Size of the buffer.

			ShaderType shader_usage;					///< \brief Shader using this constant buffer.

			vector<ShaderVariableDesc> variables;		///< \brief Variables inside the buffer.

		};

		/// \brief Description of a shader resource (textures, structured buffers, uavs, ...).
		struct ShaderResourceDesc{

			string name;								///< \brief Name of the resource.

			ShaderResourceType type;					///< \brief Type of the resource.

			unsigned int elements;						///< \brief Elements in case of a resource array.

			ShaderType shader_usage;					///< \brief Shader using this resource.

		};

		/// \brief Description of a shader sampler.
		struct ShaderSamplerDesc{

			string name;								///< \brief Name of the sampler.

			ShaderType shader_usage;					///< \brief Shader using this sampler.

		};

		/// \brief Description of a shader.
		struct ShaderReflection{

			vector<ShaderBufferDesc> buffers;			///< \brief Buffers.

			vector<ShaderResourceDesc> resources;		///< \brief Resources.

			vector<ShaderSamplerDesc> samplers;			///< \brief Samplers.

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
			static HRESULT MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11VertexShader** shader, ShaderReflection* reflection, wstring* errors);

		};

		/// \brief hull shader type traits.
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
			static HRESULT MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11HullShader** shader, ShaderReflection* reflection, wstring* errors);

		};

		/// \brief domain shader type traits.
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
			static HRESULT MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11DomainShader** shader, ShaderReflection* reflection, wstring* errors);

		};

		/// \brief geomtry shader type traits.
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
			static HRESULT MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11GeometryShader** shader, ShaderReflection* reflection, wstring* errors);

		};

		/// \brief pixel shader type traits.
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
			static HRESULT MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, ID3D11PixelShader** shader, ShaderReflection* reflection, wstring* errors);

		};

		/// \brief Create a depth stencil suitable for the provided target.
		/// \param device Device used to create the texture.
		/// \param width The width of the depth stencil texture in pixels.
		/// \param height The height of the depth stencil texture in pixels.
		/// \param depth_stencil Pointer to the created depth stencil. Set to nullptr if not needed.
		/// \param depth_stencil_view Pointer to the depth stencil view. Set to nullptr if not needed.
		HRESULT MakeDepthStencil(ID3D11Device& device, unsigned int width, unsigned int height, ID3D11Texture2D** depth_stencil, ID3D11DepthStencilView** depth_stencil_view);

		/// \brief Create a vertex buffer.
		/// \tparam TVertexFormat Format of the vertex.
		/// \param device Device used to create the vertex buffer.
		/// \param vertices Pointer to the firt vertex.
		/// \param size Total size of the vertex buffer in bytes.
		/// \param buffer Pointer to the object that will hold the buffer.
		HRESULT MakeVertexBuffer(ID3D11Device& device, const void* vertices, size_t size, ID3D11Buffer** buffer);

		/// \brief Create an index buffer.
		/// \param device Device used to create the index buffer.
		/// \param indices Pointer to the first index.
		/// \param size Total size of the index buffer in bytes.
		/// \param buffer Pointer to the object that will hold the buffer.
		HRESULT MakeIndexBuffer(ID3D11Device& device, const unsigned int* indices, size_t size, ID3D11Buffer** buffer);

		/// \brief Create a constant buffer.
		/// \param device Device used to create the constant buffer.
		/// \param size Size of the constant buffer in bytes.
		/// \param buffer Pointer to the object that will hold the buffer.
		HRESULT MakeConstantBuffer(ID3D11Device& device, size_t size, ID3D11Buffer** buffer);

		/// \brief Create a shader from HLSL code.
		/// \param device Device used to create the shader.
		/// \param HLSL HLSL code to compile.
		/// \param source_file Used to resolve #include directives.
		/// \param shader Pointer to the shader that will contain the result. Set to null to ignore the object.
		/// \param reflection Pointer to a pre-filled shader reflection. Set to null to ignore the reflection.
		/// \param errors Pointer to a string that will contain the compilation errors if the the method fails. Set to null to ignore.
		template <typename TShader>
		HRESULT MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, TShader** shader, ShaderReflection* reflection = nullptr, wstring* errors = nullptr);

		/// \brief Create a sampler state.
		/// \param device Device used to create the sampler.
		/// \param texture_mapping Texture mapping mode while sampling.
		/// \param anisotropy_level Maximum anisotropy level. Set to 0 to disable anisotropic filtering and enable trilinear filtering.
		/// \param sampler Pointer to the object that will hold the sampler if the method succeeds..
		HRESULT MakeSampler(ID3D11Device& device, TextureMapping texture_mapping, unsigned int anisotropy_level, ID3D11SamplerState** sampler);
	}

}

//

template <typename TShader>
inline HRESULT gi_lib::dx11::MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, TShader** shader, ShaderReflection* reflection, wstring* errors){

	return ShaderTraits<TShader>::MakeShader(device, HLSL, source_file, shader, reflection, errors);

}