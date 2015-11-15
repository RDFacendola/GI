/// \file dx11gpgpu.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include <memory>

#include "gpgpu.h"

#include "dx11/dx11.h"
#include "dx11/dx11shader_state.h"
#include "dx11/dx11texture.h"
#include "dx11/dx11sampler.h"
#include "dx11/dx11gpgpu.h"
#include "dx11/dx11buffer.h"

#include "windows/win_os.h"
#include "instance_builder.h"

namespace gi_lib{

	namespace dx11{

		using windows::COMPtr;
		
		using std::unique_ptr;

		/// \brief Encapsulate a compute shader.
		/// \author Raffaele D. Facendola
		class DX11Computation : public IComputation{

		public:

			using IComputation::SetInput;
			using IComputation::SetOutput;

			DX11Computation(const CompileFromFile& arguments);

			/// \brief No copy-constructor.
			DX11Computation(const DX11Computation&) = delete;

			virtual ~DX11Computation();
			
			/// \brief No assignment operator.
			DX11Computation& operator=(const DX11Computation&) = delete;

			virtual size_t GetSize() const override;
						
			/// \brief Execute the computation on the GPU.
			/// \param x Threads to dispatch along the X-axis.
			/// \param y Threads to dispatch along the Y-axis.
			/// \param z Threads to dispatch along the Z-axis.
			/// \remarks The total amount of dispatched threads is x*y*z.
			virtual void Dispatch(ID3D11DeviceContext& context, unsigned int x, unsigned int y, unsigned int z);

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2DArray>& texture_2D_array) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array) override;

			virtual bool SetOutput(const Tag& tag, const ObjectPtr<IGPTexture2D>& gp_texture_2D) override;

			virtual bool SetOutput(const Tag& tag, const ObjectPtr<IGPTexture2DArray>& gp_texture_2D_array) override;
			
			unique_ptr<ShaderStateComposite> shader_composite_;						///< \brief Collection of shaders. This class holds just 1 compute shader.
			
			Vector3i group_size_;													///< \brief Size of each thread group, as defined inside the compute shader.

		};
		
		///////////////////////////////// DX11 COMPUTATION ///////////////////////////////////

		INSTANTIABLE(IComputation, DX11Computation, IComputation::CompileFromFile);

		inline size_t DX11Computation::GetSize() const
		{

			return 0;

		}

		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D){

			return shader_composite_->SetShaderResource(tag,
														resource_cast(texture_2D));

		}

		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<ITexture2DArray>& texture_2D_array){

			return shader_composite_->SetShaderResource(tag,
														resource_cast(texture_2D_array));

		}

		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state){

			return shader_composite_->SetSampler(tag,
												 resource_cast(sampler_state));

		}

		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer){
	
			return shader_composite_->SetConstantBuffer(tag,
														resource_cast(structured_buffer));
	
		}
		
		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array){

			return shader_composite_->SetShaderResource(tag,
														resource_cast(structured_array));

		}

		inline bool DX11Computation::SetOutput(const Tag& tag, const ObjectPtr<IGPTexture2D>& gp_texture_2D){

			return shader_composite_->SetUnorderedAccess(tag,
														 resource_cast(gp_texture_2D));

		}

		inline bool DX11Computation::SetOutput(const Tag& tag, const ObjectPtr<IGPTexture2DArray>& gp_texture_2D_array){

			return shader_composite_->SetUnorderedAccess(tag,
														 resource_cast(gp_texture_2D_array));

		}

	}

}



#endif