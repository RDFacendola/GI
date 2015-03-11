/// \file dx11shader.h
/// \brief Interfaces for DirectX11 shaders.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

#include <string>
#include <memory>
#include <vector>

#include "..\..\include\windows\os_windows.h"

using std::string;
using std::vector;
using std::unique_ptr;
using gi_lib::windows::COMDeleter;

namespace gi_lib{

	namespace dx11{

		/// \brief Info about shader types.
		template <typename TShaderType>
		struct ShaderTypeInfo;

		/// \brief Info about vertex shader.
		template <> struct ShaderTypeInfo < ID3D11VertexShader > {

			static const char * kEntryPoint;		///< Entry point for the vertex shader.

			static const char * kShaderProfile;		///< Shader profile.

		};

		/// \brief Info about hull shader.
		template <> struct ShaderTypeInfo < ID3D11HullShader > {

			static const char * kEntryPoint;		///< Entry point for the hull shader.

			static const char * kShaderProfile;		///< Shader profile.

		};

		/// \brief Info about domain shader.
		template <> struct ShaderTypeInfo < ID3D11DomainShader > {

			static const char * kEntryPoint;		///< Entry point for the domain shader.

			static const char * kShaderProfile;		///< Shader profile.

		};

		/// \brief Info about geometry shader.
		template <> struct ShaderTypeInfo < ID3D11GeometryShader > {

			static const char * kEntryPoint;		///< Entry point for the geometry shader.

			static const char * kShaderProfile;		///< Shader profile.

		};

		/// \brief Info about pixel shader.
		template <> struct ShaderTypeInfo < ID3D11PixelShader > {

			static const char * kEntryPoint;		///< Entry point for the pixel shader.

			static const char * kShaderProfile;		///< Shader profile.

		};

		/// \brief Shader variable reflection.
		struct ShaderVariableReflection{

			string variable_name;		///< \brief Variable name.

			size_t size;				///< \brief Size of the variable.

			size_t offset;				///< \brief Offset of the variable.

		};

		/// \brief Shader buffer reflection.
		struct BufferReflection{

			string buffer_name;								///< \brief Constant buffer name.

			size_t size;									///< \brief Constant buffer total size.

			vector<ShaderVariableReflection> variables;		///< \brief Variable reflections.

		};

		/// \brief Shader reflection.
		struct ShaderReflection{

			vector<BufferReflection> buffers;				///< \brief Buffer reflections.

		};

		/// \brief Helper class for shader management.
		class ShaderHelper{

		public:

			/// \brief Create a constant buffer.
			/// \param device Device used to create the constant buffer
			/// \param
			static ID3D11Buffer * MakeConstantBufferOrDie(ID3D11Device & device, size_t size);

			/// \brief Compile the specified shader code.
			/// \param code HLSL code to compile.
			/// \param source_file Name of the file the code was read from. This parameter is used to resolve #include directives.
			/// \return Returns a pointer to the compiled code if the compilation was successful, returns null otherwise.
			template <typename TShaderType>
			static unique_ptr<ID3DBlob, COMDeleter> Compile(const string& code, const string& source_file);

			/// \brief Compile the specified shader code.
			/// \param code HLSL code to compile.
			/// \param source_file Name of the file the code was read from. This parameter is used to resolve #include directives.
			/// \return Returns a pointer to the compiled code if the compilation was successful, throws an error otherwise.
			template <typename TShaderType>
			static unique_ptr<ID3DBlob, COMDeleter> CompileOrDie(const string& code, const string& source_file);
			
			/// \brief Reflect a shader from its bytecode.
			/// Reflection info are ignored if already present in the specified reflection object.
			/// \param bytecode Compiled shader code.
			/// \param reflection Output of the reflection.
			/// \return Returns a vector containing the indices of the constant buffers ordered according to the bytecode.
			static void ReflectMoreOrDie(ID3DBlob& bytecode, ShaderReflection& reflection);

		private:

			/// \brief Compile the specified shader code.
			/// \param code HLSL code to compile.
			/// \param source_file Name of the file the code was read from. This parameter is used to resolve #include directives.
			/// \param entry_point Name of the entry point of the shader.
			/// \param shader_profile Shader profile.
			/// \param compulsory Whether is not acceptable a compilation failure or not.
			/// \return Returns a pointer to the compiled code if the compilation was successful, otherwise, depending on "compulsory" the method will throw an exception (true) or return null (false).
			static unique_ptr<ID3DBlob, COMDeleter> Compile(const string& code, const string& source_file, const char * entry_point, const char * shader_profile, bool compulsory);

		};


		// ShaderHelper

		template <typename TShaderType>
		unique_ptr<ID3DBlob, COMDeleter> ShaderHelper::Compile(const string& code, const string& source_file){

			return Compile(code,
						   source_file,
						   ShaderTypeInfo<TShaderType>::kEntryPoint,
						   ShaderTypeInfo<TShaderType>::kShaderProfile,
						   false);
			
		}

		template <typename TShaderType>
		unique_ptr<ID3DBlob, COMDeleter> ShaderHelper::CompileOrDie(const string& code, const string& source_file){
			
			return Compile(code,
						   source_file,
						   ShaderTypeInfo<TShaderType>::kEntryPoint,
						   ShaderTypeInfo<TShaderType>::kShaderProfile,
						   true);
			
		}

	}

}
