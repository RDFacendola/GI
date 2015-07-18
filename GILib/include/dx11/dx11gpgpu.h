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

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using windows::COMPtr;
		
		using std::unique_ptr;

		/// \brief Encapsulate a compute shader.
		/// \author Raffaele D. Facendola
		class DX11Computation : public IComputation{

		public:

			DX11Computation(const CompileFromFile& arguments);

			/// \brief No copy-constructor.
			DX11Computation(const DX11Computation&) = delete;

			virtual ~DX11Computation();
			
			/// \brief No assignment operator.
			DX11Computation& operator=(const DX11Computation&) = delete;

			virtual void Dispatch(unsigned int x, unsigned int y, unsigned int z) override;
						
			virtual size_t GetSize() const override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state) override;

			virtual bool SetOutput(const Tag& tag, const ObjectPtr<IGPTexture2D>& gp_texture_2D) override;

		private:

			virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array) override;

			unique_ptr<ShaderStateComposite> shader_composite_;						///< \brief Collection of shaders. This class holds just 1 compute shader.
			
		};
		
		///////////////////////////////// DX11 COMPUTATION ///////////////////////////////////

		inline size_t DX11Computation::GetSize() const
		{

			return 0;

		}

		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D){

			return shader_composite_->SetShaderResource(tag,
														texture_2D);

		}

		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state){

			return shader_composite_->SetSampler(tag,
												 sampler_state);

		}

		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array){

			return shader_composite_->SetShaderResource(tag,
														structured_array);

		}

		inline bool DX11Computation::SetInput(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer){
	
			return shader_composite_->SetConstantBuffer(tag,
														structured_buffer);
	
		}

		inline bool DX11Computation::SetOutput(const Tag& tag, const ObjectPtr<IGPTexture2D>& gp_texture_2D){

			return shader_composite_->SetUnorderedAccess(tag,
														 gp_texture_2D);

		}

	}

}



#endif