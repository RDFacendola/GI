/// \file dx11material.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#pragma once

#include <memory>

#include "material.h"

#include "debug.h"

#include "dx11/dx11.h"
#include "dx11/dx11shader_state.h"
#include "dx11/dx11buffer.h"
#include "dx11/dx11texture.h"
#include "dx11/dx11sampler.h"

namespace gi_lib{

	namespace dx11{

		using ::std::unique_ptr;
		using ::std::shared_ptr;

		/// \brief DirectX11 material.
		/// \author Raffaele D. Facendola
		class DX11Material : public IMaterial{

		public:

			using IMaterial::SetInput;

			/// \brief Create a new DirectX11 material from shader code.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to load the material.
			DX11Material(const CompileFromFile& args);

			/// \brief Instantiate a DirectX11 material from another one.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to instantiate the material.
			DX11Material(const Instantiate& args);

			/// \brief Default destructor.
			virtual ~DX11Material();

			virtual size_t GetSize() const override;

			/// \brief Bind the material to the pipeline.
			void Bind(ID3D11DeviceContext& context);

			/// \brief Unbind the material from the pipeline.
			void Unbind(ID3D11DeviceContext& context);

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state) override;

		private:

			virtual bool SetStructuredBuffer(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer) override;

			virtual bool SetStructuredArray(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array) override;

			unique_ptr<ShaderStateComposite> shader_composite_;		///< \brief Collection of shaders. Vertex and pixel shaders are compulsory.

			COMPtr<ID3D11InputLayout> input_layout_;				///< \brief Vertex input layout, defined per material.

		};
		
		/// \brief Downcasts an IMaterial to the proper concrete type.
		ObjectPtr<DX11Material> resource_cast(const ObjectPtr<IMaterial>& resource);

		///////////////////////////////// DX11 MATERIAL ///////////////////////////////////

		inline size_t DX11Material::GetSize() const
		{

			return 0;

		}

		inline void DX11Material::Bind(ID3D11DeviceContext& context){

			shader_composite_->Bind(context);

		}

		inline void DX11Material::Unbind(ID3D11DeviceContext& context){

			shader_composite_->Unbind(context);

		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D){

			return shader_composite_->SetShaderResource(tag,
														resource_cast(texture_2D));

		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state){

			return shader_composite_->SetSampler(tag,
												 resource_cast(sampler_state));

		}

		inline bool DX11Material::SetStructuredBuffer(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer){

			return shader_composite_->SetConstantBuffer(tag,
														resource_cast(structured_buffer));

		}

		inline bool DX11Material::SetStructuredArray(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array){

			return shader_composite_->SetShaderResource(tag,
														resource_cast(structured_array));

		}

		///////////////////////////// RESOURCE CAST ////////////////////////////////

		inline ObjectPtr<DX11Material> resource_cast(const ObjectPtr<IMaterial>& resource){

			return ObjectPtr<DX11Material>(resource.Get());

		}
		
	}

}