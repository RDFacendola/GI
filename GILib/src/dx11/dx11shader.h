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
using std::shared_ptr;
using gi_lib::windows::COMDeleter;

namespace gi_lib{

	namespace dx11{

		/// \brief Type of a shader resource.
		enum class ShaderResourceType{

			UNKNOWN,			///< \brief Unknown resource.
			TEXTURE_1D,			///< \brief 1D texture.
			TEXTURE_2D,			///< \brief 2D texture.
			TEXTURE_3D,			///< \brief 3D texture.
			TEXTURE_CUBE		///< \brief Cube texture.

		};

		/// \brief Description of a shader and the binding order of the resources.
		struct ShaderBinding{

			shared_ptr<ID3DBlob> bytecode;				///< \brief Compiled shader code.

			vector<unsigned int> buffer_order;			///< \brief Binding order of the buffers, relative to the buffers declared in the reflection.

			vector<unsigned int> resources_order;		///< \brief Binding order of the resources, relative to the resources declared in the reflection.

			vector<unsigned int> samplers_order;		///< \brief Binding order of the samplers, relative to the samplers declared in the reflection.

			/// \brief Default constructor.
			ShaderBinding();

			/// \brief Copy constructor.
			ShaderBinding(const ShaderBinding& other);

			/// \brief Move constructor.
			ShaderBinding(ShaderBinding&& other);

			/// \brief Unified assignment operator.
			ShaderBinding& operator=(ShaderBinding other);

			/// \brief Swaps this instance with the provided one.
			void Swap(ShaderBinding& other);

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

			vector<ShaderVariableDesc> variables;		///< \brief Variables inside the buffer.

		};

		/// \brief Description of a shader resource (textures, structured buffers, uavs, ...).
		struct ShaderResourceDesc{

			string name;								///< \brief Name of the resource.

			ShaderResourceType type;					///< \brief Type of the resource.

			unsigned int elements;						///< \brief Elements in case of a resource array.

		};

		/// \brief Description of a shader sampler.
		struct ShaderSamplerDesc{

			string name;								///< \brief Name of the sampler.

		};

		/// \brief Description of a shader.
		struct ShaderReflection{

			vector<ShaderBufferDesc> buffers;			///< \brief Buffers.

			vector<ShaderResourceDesc> resources;		///< \brief Resources.

			vector<ShaderSamplerDesc> samplers;			///< \brief Samplers.

		};

		/// \brief Combination of shaders and their reflection.
		struct ShaderCombo{

			ShaderBinding vertex_shader;				///< \brief Vertex shader.

			ShaderBinding hull_shader;					///< \brief Hull shader.

			ShaderBinding domain_shader;				///< \brief Domain shader.

			ShaderBinding geometry_shader;				///< \brief Geometry shader.

			ShaderBinding pixel_shader;					///< \brief Pixel shader.

			ShaderReflection reflection;				///< \brief Combined reflection of the shaders.
			
			/// \brief Default constructor.
			ShaderCombo();

			/// \brief Copy constructor.
			ShaderCombo(const ShaderCombo& other);

			/// \brief Move constructor.
			ShaderCombo(ShaderCombo&& other);

			/// \brief Unified assignment operator.
			ShaderCombo& operator=(ShaderCombo other);

			/// \brief Swaps this instance with the provided one.
			void Swap(ShaderCombo& other);
			
		};

		/// \brief Swaps two shader bindings.
		inline void swap(ShaderBinding& left, ShaderBinding& right){

			left.Swap(right);

		}

		/// \brief Swaps two shader combos.
		inline void swap(ShaderCombo& left, ShaderCombo& right){

			left.Swap(right);

		}

		/// \brief Helper class for shader management.
		class ShaderHelper{

		public:


			static const unsigned int kVertexShader = 1;		///< \brief Identifies a vertex shader.
			static const unsigned int kHullShader = 2;			///< \brief Identifies a hull shader.
			static const unsigned int kDomainShader = 4;		///< \brief Identifies a domain shader.
			static const unsigned int kGeometryShader = 8;		///< \brief Identifies a geometry shader.
			static const unsigned int kPixelShader = 16;		///< \brief Identifies a pixel shader.
			
			/// \brief All shaders.
			static const unsigned int kAllShaders = kVertexShader | kHullShader | kDomainShader | kGeometryShader | kPixelShader;	

			/// \brief Create a constant buffer.
			/// \param device Device used to create the constant buffer.
			/// \param size Size of the buffer.
			static ID3D11Buffer * MakeConstantBufferOrDie(ID3D11Device & device, size_t size);

			/// \brief Compile a shader.
			/// \param code Pointer to a buffer containing the HLSL code.
			/// \param size Size of the code buffer in bytes.
			/// \param source_file Name of the source file. Used to resolve the #include directives inside the HLSL code.
			/// \param shaders Shaders to compile (for example: kVertexShader | kPixelShader).
			/// \param compulsory_shaders Shaders that are required. If at least one shader is missing the method throws.
			/// \return Returns a shader combo
			static ShaderCombo CompileShadersOrDie(const char* code, size_t size, const char* source_file, unsigned int shaders, unsigned int compulsory_shaders);

		private:


		};

	}

}
