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
#include "..\..\include\gimath.h"
#include "..\..\include\windows\win_os.h"

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

			ALL = 31u,					///< \brief All shaders (bitwise-or of the above)

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

			ShaderType shaders;							///< \brief Shader presence.

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

		/// \brief geometry shader type traits.
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

		/// \brief Compile an HLSL code returning a bytecode.
		/// \param HLSL HLSL code.
		/// \param source_file File containing the HLSL code, used to resolve the #include directives.
		/// \param bytecode If the method succeeds, it will contain the compiled bytecode.
		/// \param error_string If the method fails, it will contain the error string. Optional.
		/// \return Returns the com
		template <typename TShader>
		HRESULT Compile(const string& HLSL, const string& source_file, ID3DBlob** bytecode, wstring* error_string);

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

		/// \brief Bind a shader to a render context.
		/// \param context Context the shader will be bound to.
		/// \param shader Shader to bound.
		template <typename TShader>
		void SetShader(ID3D11DeviceContext& context, TShader* shader);

		/// \brief Bind some constant buffers to a render context.
		/// \param start_slot Slot where the first buffer will be bound to.
		/// \param context Context the constant buffers will be bound to.
		/// \param buffers Array containing the constant buffers.
		/// \param count Number of buffers.
		template <typename TShader>
		void SetConstantBuffers(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count);

		/// \brief Bind some shader resources to a render context.
		/// \param start_slot Slot where the first resource will be bound to.
		/// \param context Context the resources will be bound to.
		/// \param buffers Array containing the shader resource views.
		/// \param count Number of resources.
		template <typename TShader>
		void SetShaderResources(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count);

		/// \brief Bind some samplers to a render context.
		/// \param start_slot Slot where the first sampler will be bound to.
		/// \param context Context the samplers will be bound to.
		/// \param buffers Array containing the sampler states.
		/// \param count Number of samplers.
		template <typename TShader>
		void SetShaderSamplers(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count);

		/// \brief Compute the left-handed perspective projection matrix.
		/// \param field_of_view Field of view, in radians.
		/// \param aspect_ratio Width-to-height aspect ratio.
		/// \param near_plane Distance of the near clipping plane.
		/// \param far_plane Distance of the far clipping plane.
		Matrix4f ComputePerspectiveProjectionLH(float field_of_view, float aspect_ratio, float near_plane, float far_plane);

	}

}

////////////////////////////// INLINE IMPLEMENTATION /////////////////////////////////

template <typename TShader>
HRESULT gi_lib::dx11::Compile(const string& HLSL, const string& source_file, ID3DBlob** bytecode, wstring* error_string){

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
						 ShaderTraits<TShader>::entry_point,
						 ShaderTraits<TShader>::profile,
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

template <typename TShader>
inline HRESULT gi_lib::dx11::MakeShader(ID3D11Device& device, const string& HLSL, const string& source_file, TShader** shader, ShaderReflection* reflection, wstring* errors){

	return ShaderTraits<TShader>::MakeShader(device, HLSL, source_file, shader, reflection, errors);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11VertexShader>(ID3D11DeviceContext& context, ID3D11VertexShader* shader){

	context.VSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11HullShader>(ID3D11DeviceContext& context, ID3D11HullShader* shader){
	
	context.HSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11DomainShader>(ID3D11DeviceContext& context, ID3D11DomainShader* shader){

	context.DSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11GeometryShader>(ID3D11DeviceContext& context, ID3D11GeometryShader* shader){

	context.GSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11PixelShader>(ID3D11DeviceContext& context, ID3D11PixelShader* shader){

	context.PSSetShader(shader,
						nullptr,
						0);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.VSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.HSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.DSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.GSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.PSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.VSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){
	
	context.HSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.DSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.GSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.PSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

		context.VSSetSamplers(static_cast<UINT>(start_slot),
							  static_cast<UINT>(count),
							  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){
	
		context.HSSetSamplers(static_cast<UINT>(start_slot),
							  static_cast<UINT>(count),
							  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

		context.DSSetSamplers(static_cast<UINT>(start_slot),
							  static_cast<UINT>(count),
							  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

		context.GSSetSamplers(static_cast<UINT>(start_slot),
							  static_cast<UINT>(count),
							  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

		context.PSSetSamplers(static_cast<UINT>(start_slot),
							  static_cast<UINT>(count),
							  samplers);

}