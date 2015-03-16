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

#include "..\..\include\macros.h"
#include "..\..\include\enums.h"
#include "..\..\include\windows\os_windows.h"

using std::string;
using std::vector;
using std::shared_ptr;
using gi_lib::windows::COMDeleter;

namespace gi_lib{

	namespace dx11{

		/// \brief Shader type.
		ENUM_FLAGS(ShaderType, unsigned int){

			NONE = 0u,

			VERTEX_SHADER = 1u,
			HULL_SHADER = 2u,
			DOMAIN_SHADER = 4u,
			GEOMETRY_SHADER = 8u,
			PIXEL_SHADER = 16u,

			ALL = 31u

		};

		/// \brief Type of a shader resource.
		enum class ShaderResourceType: unsigned int{

			UNKNOWN,			///< \brief Unknown resource.
			TEXTURE_1D,			///< \brief 1D texture.
			TEXTURE_2D,			///< \brief 2D texture.
			TEXTURE_3D,			///< \brief 3D texture.
			TEXTURE_CUBE		///< \brief Cube texture.

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

		/// \brief Combination of shaders and their reflection.
		struct ShaderCombo{

			shared_ptr<ID3DBlob> vs_bytecode;				///< \brief Vertex shader bytecode.

			shared_ptr<ID3DBlob> hs_bytecode;				///< \brief Hull shader bytecode.

			shared_ptr<ID3DBlob> ds_bytecode;				///< \brief Domain shader bytecode.

			shared_ptr<ID3DBlob> gs_bytecode;				///< \brief Geometry shader bytecode.

			shared_ptr<ID3DBlob> ps_bytecode;				///< \brief Pixel shader bytecode.

			ShaderReflection reflection;					///< \brief Combined reflection of the shaders.
			
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

		/// \brief Helper class for shader management.
		class ShaderHelper{

		public:

			/// \brief Compile a shader.
			/// \param code Pointer to a buffer containing the HLSL code.
			/// \param size Size of the code buffer in bytes.
			/// \param source_file Name of the source file. Used to resolve the #include directives inside the HLSL code.
			/// \param shaders Shaders to compile (for example: ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER).
			/// \param compulsory Shaders that are required. If at least one shader is missing the method throws.
			/// \return Returns a shader combo
			static ShaderCombo CompileShadersOrDie(const char* code, size_t size, const char* source_file, ShaderType shaders, ShaderType compulsory);

		private:


		};

		//

		/// \brief Swaps two shader combos.
		inline void swap(ShaderCombo& left, ShaderCombo& right){

			left.Swap(right);

		}


	}

}
