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
			
			/// \brief Default destructor.
			virtual ~DX11Material();

			virtual size_t GetSize() const override;

			/// \brief Bind the material to the pipeline.
			void Bind(ID3D11DeviceContext& context);

			/// \brief Bind both the material and render targets to the pipeline.
			void Bind(ID3D11DeviceContext& context, const ObjectPtr<DX11RenderTarget>& render_target);

			/// \brief Unbind the material from the pipeline.
			void Unbind(ID3D11DeviceContext& context);
			
			/// \brief Unbind the material from the pipeline.
			void Unbind(ID3D11DeviceContext& context, const ObjectPtr<DX11RenderTarget>& render_target);

			/// \brief Commit the pending resources to the shader.
			void Commit(ID3D11DeviceContext& context);

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture3D>& texture_3D) override;
			
			virtual bool GetInput(const Tag& tag, ObjectPtr<ITexture2D>& texture_2D) const override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2DArray>& texture_2D_array) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array) override;

			virtual bool SetInput(const Tag& tag, const ObjectPtr<IGPStructuredArray>& gp_structured_array) override;

			virtual bool SetOutput(const Tag& tag, const ObjectPtr<IGPStructuredArray>& gp_structured_array, bool keep_initial_count = true) override;

			virtual bool SetOutput(const Tag& tag, const ObjectPtr<IGPTexture3D>& gp_texture_3D) override;

			virtual ObjectPtr<IMaterial> Instantiate() override;

		protected:

			DX11Material(unique_ptr<ShaderStateComposite> shader_composite, const COMPtr<ID3D11InputLayout> input_layout);

		private:

			unique_ptr<ShaderStateComposite> shader_composite_;			///< \brief Collection of shaders. Vertex and pixel shaders are compulsory.

			std::map<Tag, ObjectPtr<ITexture2D>> texture_2D_inputs_;	///< \brief Collection of input 2D Textures.

			COMPtr<ID3D11InputLayout> input_layout_;					///< \brief Vertex input layout, defined per material.

		};
		
		/// \brief Downcasts an IMaterial to the proper concrete type.
		ObjectPtr<DX11Material> resource_cast(const ObjectPtr<IMaterial>& resource);

		///////////////////////////////// DX11 MATERIAL ///////////////////////////////////

		INSTANTIABLE(IMaterial, DX11Material, IMaterial::CompileFromFile);

		inline size_t DX11Material::GetSize() const
		{

			return 0;

		}

		inline void DX11Material::Bind(ID3D11DeviceContext& context){

			shader_composite_->Bind(context);

			context.IASetInputLayout(input_layout_.Get());

		}

		inline void DX11Material::Bind(ID3D11DeviceContext& context, const ObjectPtr<DX11RenderTarget>& render_target) {

			shader_composite_->Bind(context, render_target);

			context.IASetInputLayout(input_layout_.Get());

		}

		inline void DX11Material::Unbind(ID3D11DeviceContext& context){

			shader_composite_->Unbind(context);

			context.IASetInputLayout(nullptr);

		}

		inline void DX11Material::Unbind(ID3D11DeviceContext& context, const ObjectPtr<DX11RenderTarget>& render_target) {

			shader_composite_->Unbind(context, render_target);

			context.IASetInputLayout(nullptr);

		}

		inline void DX11Material::Commit(ID3D11DeviceContext& context) {

			shader_composite_->Commit(context);

		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D){

			if(shader_composite_->SetShaderResource(tag,
													resource_cast(texture_2D))){

				texture_2D_inputs_[tag] = texture_2D;

				return true;

			}

			return false;

		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<ITexture3D>& texture_3D) {

			return shader_composite_->SetShaderResource(tag,
														resource_cast(texture_3D));

		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<ITexture2DArray>& texture_2D_array){

			return shader_composite_->SetShaderResource(tag,
														resource_cast(texture_2D_array));
			
		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state){

			return shader_composite_->SetSampler(tag,
												 resource_cast(sampler_state));

		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer){

			return shader_composite_->SetConstantBuffer(tag,
														resource_cast(structured_buffer));

		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array){

			return shader_composite_->SetShaderResource(tag,
														resource_cast(structured_array));

		}

		inline bool DX11Material::GetInput(const Tag& tag, ObjectPtr<ITexture2D>& texture_2D) const {

			auto it = texture_2D_inputs_.find(tag);

			if (it != texture_2D_inputs_.end()) {

				texture_2D = it->second;

				return true;

			}

			return false;

		}

		inline bool DX11Material::SetInput(const Tag& tag, const ObjectPtr<IGPStructuredArray>& gp_structured_array) {
			
			return shader_composite_->SetShaderResource(tag,
														resource_cast(gp_structured_array));

		}

		inline bool DX11Material::SetOutput(const Tag& tag, const ObjectPtr<IGPStructuredArray>& gp_structured_array, bool keep_initial_count) {
			
			return shader_composite_->SetUnorderedAccess(tag,
														 resource_cast(gp_structured_array),
														 keep_initial_count);

		}

		inline bool DX11Material::SetOutput(const Tag& tag, const ObjectPtr<IGPTexture3D>& gp_texture_3D){

			return shader_composite_->SetUnorderedAccess(tag,
														 resource_cast(gp_texture_3D));

		}
		
		///////////////////////////// RESOURCE CAST ////////////////////////////////

		inline ObjectPtr<DX11Material> resource_cast(const ObjectPtr<IMaterial>& resource){

			return ObjectPtr<DX11Material>(resource.Get());

		}
		
	}

}