/// \file dx11gpgpu.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#pragma once

#include "gpgpu.h"

#include "dx11.h"

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

			unique_com<ID3D11ComputeShader> compute_shader_;		///< \brief Compute shader

		};

	}

}

///////////////////////////////// DX11 COMPUTATION ///////////////////////////////////

inline size_t gi_lib::dx11::DX11Computation::GetSize() const
{

	return 0;

}