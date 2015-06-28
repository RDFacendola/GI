/// \file dx11gpgpu.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include "gpgpu.h"

#include "dx11/dx11.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using windows::unique_com;

		/// \brief Encapsulate a compute shader.
		/// \author Raffaele D. Facendola
		class DX11Computation : public IComputation{

		public:

			DX11Computation(const CompileFromFile& arguments);

			virtual ~DX11Computation();
			
			virtual void Dispatch(unsigned int x, unsigned int y, unsigned int z) override;
						
			virtual size_t GetSize() const override;

		private:

			virtual ObjectPtr<Object> GetArgument(const string& name, const std::type_index& argument_type, GPUAccess access) override;

			unique_com<ID3D11ComputeShader> compute_shader_;		///< \brief Compute shader.

			ShaderReflection reflection_;							///< \brief Shader reflection.

		};

		/// \brief Base interface for DirectX11 computation arguments.
		template <typename TArgument, GPUAccess access, typename>
		class DX11ComputationArgument : public IComputationArgument<TArgument, access, void>{

		public:

			/// \brief Virtual destructor.
			virtual ~DX11ComputationArgument();

			virtual void Set(const TArgument& value) override;

		};

		/// \brief Computation argument specialization for DirectX11 resources (textures, buffers, ...).
		template <typename TArgument, GPUAccess access>
		class DX11ComputationArgument < TArgument, access, typename std::enable_if<std::is_base_of<IResource, TArgument>::value>::type > : public IComputationArgument<TArgument, access, void>{

		public:

			using ResourceView = IResourceView;

			/// \brief Virtual destructor.
			virtual ~DX11ComputationArgument();

			virtual void Set(ObjectPtr<ResourceView> resource_view) override;

		};

		/// \brief Computation argument specialization for DirectX11 scalars (integers, floats, ...).
		template <typename TArgument, GPUAccess access>
		class DX11ComputationArgument < TArgument, access, typename std::enable_if<std::is_arithmetic<TArgument>::value>::type > : public IComputationArgument<TArgument, access, void>{

		public:

			/// \brief Virtual destructor.
			virtual ~DX11ComputationArgument();

			virtual void Set(TArgument value) override;

		};

	}

}

///////////////////////////////// DX11 COMPUTATION ///////////////////////////////////

inline size_t gi_lib::dx11::DX11Computation::GetSize() const
{

	return 0;

}

///////////////////////////////// DX11 COMPUTATION ARGUMENT ///////////////////////////////////

#endif