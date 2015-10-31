/// \file dx11shader_state.h
/// \brief This file contains classes used to manage DirectX11 shader states.
///
/// \author Raffaele D. Facendola

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "tag.h"
#include "object.h"

#include "dx11/dx11.h"
#include "dx11/dx11shader.h"
#include "dx11/dx11commitable.h"

#include "windows/win_os.h"

#ifdef _WIN32

namespace gi_lib{

	namespace dx11{

		class DX11StructuredBuffer;
		class DX11StructuredArray;
		class DX11Texture2D;
		class DX11Texture2DArray;
		class DX11GPTexture2D;
		class DX11Sampler;
		
		using windows::COMPtr;

		/// \brief Base class for a shader state.
		/// \author Raffaele D. Facendola
		class BaseShaderState{

		public:

			/// \brief Create a new shader state.
			/// \brief Pointer to the shader reflection.
			BaseShaderState(const std::shared_ptr<ShaderReflection>& reflection);

			/// \brief Create a new shader state.
			/// \brief Shader reflection.
			BaseShaderState(const ShaderReflection& reflection);

			/// \brief Copy constructor.
			BaseShaderState(const BaseShaderState& other);

			/// \brief Virtual destructor.
			virtual ~BaseShaderState(){}

			/// \brief Get the shader reflection.
			/// \return Returns a reference to the shader reflection.
			const ShaderReflection& GetReflection() const;

			/// \brief Set a shader resource view for this shader.
			/// \param slot Index of the slot where the view will be bound.
			/// \param shader_resource_view The view to bind.
			void SetShaderResourceView(unsigned int slot, const ShaderResourceView& shader_resource_view);

			/// \brief Set an unordered access view for this shader.
			/// \param slot Index of the slot where the view will be bound.
			/// \param unordered_access_view The view to bind.
			void SetUnorderedAccessView(unsigned int slot, const UnorderedAccessView& unordered_access_view);

			/// \brief Set a constant buffer for this shader.
			/// \param slot Index of the slot where the buffer will be bound.
			/// \param constant_buffer The buffer to bind.
			void SetConstantBufferView(unsigned int slot, const ConstantBufferView& constant_buffer);

			/// \brief Set a sampler for this shader.
			/// \param slot Index of the slot where the sampler will be bound.
			/// \param sampler_state The sampler to bind.
			void SetSamplerStateView(unsigned int slot, const SamplerStateView& sampler_state);

			/// \brief Bind the shader to the given device context.
			virtual void Bind(ID3D11DeviceContext& context) = 0;

			/// \brief Unbind the shader from the given device context.
			virtual void Unbind(ID3D11DeviceContext& context) = 0;

			/// \brief Instantiate a copy of the shader state.
			virtual BaseShaderState* Instantiate() const = 0;

		protected:
			
			std::vector<COMPtr<ID3D11ShaderResourceView>> shader_resource_views_;		///< \brief List of shader resource views.

			std::vector<COMPtr<ID3D11UnorderedAccessView>> unordered_access_views_;		///< \brief List of unordered access views.

			std::vector<COMPtr<ID3D11Buffer>> constant_buffers_;						///< \brief List of constant buffers.

			std::vector<COMPtr<ID3D11SamplerState>> samplers_;							///< \brief List of sampler states.

		private:

			std::vector<ShaderResourceView> srv_resources_;								///< \brief Needed to keep alive the resources while their shader resource view is bound to this instance.

			std::vector<UnorderedAccessView> uav_resources_;							///< \brief Needed to keep alive the resources while their unordered access view is bound to this instance.

			std::vector<ConstantBufferView> buffer_resources_;							///< \brief Needed to keep alive the resources while their buffer view is bound to this instance.

			std::vector<SamplerStateView> sampler_resources_;							///< \brief Needed to keep alive the resources while their sampler view is bound to this instance.

			std::shared_ptr<ShaderReflection> reflection_;								///< \brief Shader reflection.

		};

		/// \brief Concrete shader state.
		/// \tparam TShader Type of shader this state refers to.
		/// \author Raffaele D. Facendola.
		template <typename TShader>
		class ShaderState : public BaseShaderState{

		public:

			ShaderState(const COMPtr<TShader>& shader, const ShaderReflection& reflection);

			/// \brief Copy constructor.
			/// \param other Other instance to copy.
			ShaderState(const ShaderState<TShader>& other);

			virtual void Bind(ID3D11DeviceContext& context) override;

			virtual void Unbind(ID3D11DeviceContext& context) override;

			virtual BaseShaderState* Instantiate() const override;

		private:

			COMPtr<TShader> shader_;		/// \brief Pointer to the concrete shader.

		};

		/// \brief Functor used to set a shader resource view to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class SRVSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = ShaderResourceView;

			/// \brief Set a shader resource view for this shader.
			/// \param shader_state Shader state where the view will be bound.
			/// \param slot Index of the slot where the view will be bound.
			SRVSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set a shader resource view to the bound shader state.
			/// \param shader_resource_view The view to bind.
			void operator()(const TValue& shader_resource_view);

		private:

			BaseShaderState* shader_state_;

			unsigned int slot_;

		};

		/// \brief Functor used to set an unordered access view to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class UAVSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = UnorderedAccessView;

			/// \brief Create a new unordered access view setter.
			/// \param shader_state Shader state where the view will be bound.
			/// \param slot Index of the slot where the view will be bound.
			UAVSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set an unordered access view for this shader.
			/// \param unordered_access_view The view to bind.
			void operator()(const TValue& unordered_access_view);

		private:

			BaseShaderState* shader_state_;

			unsigned int slot_;

		};

		/// \brief Functor used to set a constant buffer to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class CBufferSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = ConstantBufferView;

			/// \brief Create a new constant buffer setter.
			/// \param shader_state Shader state where the buffer will be bound.
			/// \param slot Index of the slot where the buffer will be bound.
			CBufferSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set a constant buffer for this shader.
			/// \param constant_buffer The buffer to bind.
			void operator()(const TValue& constant_buffer);

		private:

			BaseShaderState* shader_state_;

			unsigned int slot_;

		};

		/// \brief Functor used to set an unordered access view to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class SamplerSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = SamplerStateView;

			/// \brief Create a new sampler setter.
			/// \param shader_state Shader state where the sampler will be bound.
			/// \param slot Index of the slot where the sampler will be bound.
			SamplerSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set a sampler for this shader.
			/// \param sampler_state The sampler to bind.
			void operator()(const TValue& sampler_state);

		private:

			BaseShaderState* shader_state_;

			unsigned int slot_;

		};

		/// \brief Manages a collection of shaders and their state.
		/// \author Raffaele D. Facendola.
		class ShaderStateComposite{

		public:

			/// \brief Default constructor.
			ShaderStateComposite();

			/// \brief Copy constructor.
			ShaderStateComposite(const ShaderStateComposite& other);

			template <typename TShader>
			bool AddShader(const std::string hlsl, const std::string file_name);

			/// \brief Get the amount of shader inside the composite.
			size_t GetShaderCount() const;

			/// \brief Get a reference to a particular shader inside the composite
			const BaseShaderState& GetShaderState(size_t index) const;

			/// \brief Destroy all the shader states stored inside this instance.
			void Clear();

			/// \brief Bind the shaders to the given device context.
			virtual void Bind(ID3D11DeviceContext& context);

			/// \brief Unbind the shaders from the given device context.
			virtual void Unbind(ID3D11DeviceContext& context);

			/// \brief Set the value of a named constant buffer.
			/// \return Returns true if a constant buffer matching the specified tag was found, returns false otherwise.
			bool SetConstantBuffer(const Tag& tag, const ObjectPtr<DX11StructuredBuffer>& constant_buffer);

			/// \brief Set the value of a named shader resource view.
			/// \return Returns true if a shader resource view matching the specified tag was found, returns false otherwise.
			bool SetShaderResource(const Tag& tag, const ObjectPtr<DX11Texture2D>& texture_2D);

			/// \brief Set the value of a named shader resource view.
			/// \return Returns true if a shader resource view matching the specified tag was found, returns false otherwise.
			bool SetShaderResource(const Tag& tag, const ObjectPtr<DX11Texture2DArray>& texture_2D_array);

			/// \brief Set the value of a named shader resource view.
			/// \return Returns true if a shader resource view matching the specified tag was found, returns false otherwise.
			bool SetShaderResource(const Tag& tag, const ObjectPtr<DX11StructuredArray>& structured_array);
			
			/// \brief Set the value of a named unordered access view.
			/// \return Returns true if an unordered access view matching the specified tag was found, returns false otherwise.
			bool SetUnorderedAccess(const Tag& tag, const ObjectPtr<DX11GPTexture2D>& gp_texture_2D);

			/// \brief Set the value of a named sampler state.
			/// \return Returns true if a sampler state matching the specified tag was found, returns false otherwise.
			bool SetSampler(const Tag& tag, const ObjectPtr<DX11Sampler>& sampler);

		private:

			void AddShaderBindings(BaseShaderState& shader);
			
			std::vector<std::unique_ptr<BaseShaderState>> shaders_;						///< \brief Shader collection.

			std::unordered_map<size_t, ObjectPtr<ICommitter>> committer_table_;			///< \brief Used to track the resources that needs to be committed while binding the composite to the pipeline.

			std::unordered_multimap<size_t, CBufferSetter> cbuffer_table_;				///< \brief Table of constant buffers.

			std::unordered_multimap<size_t, SRVSetter> srv_table_;						///< \brief Table of shader resource views.

			std::unordered_multimap<size_t, UAVSetter> uav_table_;						///< \brief Table of unordered access views.

			std::unordered_multimap<size_t, SamplerSetter> sampler_table_;				///< \brief Table of samplers.

		};

		/// \brief Bind a shader to a render context.
		/// \param context Context the shader will be bound to.
		/// \param shader Shader to bound.
		template <typename TShader>
		void SetShader(ID3D11DeviceContext& context, const COMPtr<TShader>& shader);

		/// \brief Bind all the provided constant buffers to a render context.
		/// \param start_slot Slot where the first buffer will be bound to.
		/// \param context Context the constant buffers will be bound to.
		/// \param buffers Array containing the constant buffers.
		template <typename TShader>
		void SetConstantBuffers(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers);

		/// \brief Bind some constant buffers to a render context.
		/// \param start_slot Slot where the first buffer will be bound to.
		/// \param context Context the constant buffers will be bound to.
		/// \param buffers Array containing the constant buffers.
		/// \param count Number of buffers.
		template <typename TShader>
		void SetConstantBuffers(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count);

		/// \brief Bind all the provided shader resources to a render context.
		/// \param start_slot Slot where the first resource will be bound to.
		/// \param context Context the resources will be bound to.
		/// \param buffers Array containing the shader resource views.
		template <typename TShader>
		void SetShaderResources(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources);

		/// \brief Bind some shader resources to a render context.
		/// \param start_slot Slot where the first resource will be bound to.
		/// \param context Context the resources will be bound to.
		/// \param buffers Array containing the shader resource views.
		/// \param count Number of resources.
		template <typename TShader>
		void SetShaderResources(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count);

		/// \brief Bind all the provided samplers to a render context.
		/// \param start_slot Slot where the first sampler will be bound to.
		/// \param context Context the samplers will be bound to.
		/// \param buffers Array containing the sampler states.
		template <typename TShader>
		void SetSamplers(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers);

		/// \brief Bind some samplers to a render context.
		/// \param start_slot Slot where the first sampler will be bound to.
		/// \param context Context the samplers will be bound to.
		/// \param buffers Array containing the sampler states.
		/// \param count Number of samplers.
		template <typename TShader>
		void SetSamplers(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count);

		/// \brief Bind all the provided unordered access views to a render context.
		/// \param start_slot Slot where the first UAV will be bound to.
		/// \param context Context the UAVs will be bound to.
		/// \param UAVs Array containing the unordered access views.
		template <typename TShader>
		void SetUnorderedAccess(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11UnorderedAccessView>>& UAVs);

		/// \brief Bind some unordered access views to a render context.
		/// \param start_slot Slot where the first UAV will be bound to.
		/// \param context Context the UAVs will be bound to.
		/// \param UAVs Array containing the unordered access views.
		/// \param count Number of resources.
		template <typename TShader>
		void SetUnorderedAccess(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11UnorderedAccessView>>& UAVs, size_t count);
				
		//////////////////////////////// BASE SHADER STATE ///////////////////////////////////////

		inline BaseShaderState::BaseShaderState(const std::shared_ptr<ShaderReflection>& reflection) :
			reflection_(reflection){

			shader_resource_views_.resize(reflection->shader_resource_views.size());
			unordered_access_views_.resize(reflection->unordered_access_views.size());
			constant_buffers_.resize(reflection->buffers.size());
			samplers_.resize(reflection->samplers.size());

			srv_resources_.resize(shader_resource_views_.size());
			uav_resources_.resize(unordered_access_views_.size());
			buffer_resources_.resize(constant_buffers_.size());
			sampler_resources_.resize(samplers_.size());

		}

		inline BaseShaderState::BaseShaderState(const ShaderReflection& reflection) :
			BaseShaderState(std::make_shared<ShaderReflection>(reflection)){}
		
		inline BaseShaderState::BaseShaderState(const BaseShaderState& other) :
			BaseShaderState(other.reflection_){}

		inline const ShaderReflection& BaseShaderState::GetReflection() const{

			return *reflection_;

		}

		inline void BaseShaderState::SetShaderResourceView(unsigned int slot, const ShaderResourceView& shader_resource_view){

			shader_resource_views_[slot] = shader_resource_view.GetShaderResourceView();

			srv_resources_[slot] = shader_resource_view;

		}

		inline void BaseShaderState::SetUnorderedAccessView(unsigned int slot, const UnorderedAccessView& unordered_access_view){

			unordered_access_views_[slot] = unordered_access_view.GetUnorderedAccessView();

			uav_resources_[slot] = unordered_access_view;

		}

		inline void BaseShaderState::SetConstantBufferView(unsigned int slot, const ConstantBufferView& constant_buffer){

			constant_buffers_[slot] = constant_buffer.GetConstantBuffer();

			buffer_resources_[slot] = constant_buffer;

		}

		inline void BaseShaderState::SetSamplerStateView(unsigned int slot, const SamplerStateView& sampler_state){

			samplers_[slot] = sampler_state.GetSamplerState();

			sampler_resources_[slot] = sampler_state;

		}

		//////////////////////////////// SHADER STATE ///////////////////////////////////////

		template <typename TShader>
		inline ShaderState<TShader>::ShaderState(const COMPtr<TShader>& shader, const ShaderReflection& reflection) :
		BaseShaderState(reflection),
		shader_(shader){}

		template <typename TShader>
		inline ShaderState<TShader>::ShaderState(const ShaderState<TShader>& other) :
		BaseShaderState(other),
		shader_(other.shader_){}

		template <typename TShader>
		void ShaderState<TShader>::Bind(ID3D11DeviceContext& context){

			SetShader<TShader>(context,
							   shader_);
	
			SetShaderResources<TShader>(context,
										0,
										shader_resource_views_);

			SetUnorderedAccess<TShader>(context,
										0,
										unordered_access_views_);

			SetConstantBuffers<TShader>(context,
										0,
										constant_buffers_);

			SetSamplers<TShader>(context,
								 0,
								 samplers_);

		}

		template <typename TShader>
		void ShaderState<TShader>::Unbind(ID3D11DeviceContext& context){

			static vector<COMPtr<ID3D11ShaderResourceView>> null_srv(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
			static vector<COMPtr<ID3D11UnorderedAccessView>> null_uav(D3D11_1_UAV_SLOT_COUNT);
			static vector<COMPtr<ID3D11Buffer>> null_buffers(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
			static vector<COMPtr<ID3D11SamplerState>> null_samplers(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);

			SetShader<TShader>(context,
							   nullptr);
	
			SetShaderResources<TShader>(context,
										0,
										null_srv,
										shader_resource_views_.size());

			SetUnorderedAccess<TShader>(context,
										0,
										null_uav,
										unordered_access_views_.size());

			SetConstantBuffers<TShader>(context,
										0,
										null_buffers,
										constant_buffers_.size());

			SetSamplers<TShader>(context,
								 0,
								 null_samplers,
								 samplers_.size());

		}

		template <typename TShader>
		inline BaseShaderState* ShaderState<TShader>::Instantiate() const{

			return new ShaderState<TShader>(*this);

		}

		//////////////////////////////// SRV SETTER ///////////////////////////////////////

		inline SRVSetter::SRVSetter(BaseShaderState& shader_state, unsigned int slot) :
		shader_state_(&shader_state),
		slot_(slot){}

		inline void SRVSetter::operator()(const TValue& shader_resource_view){

			shader_state_->SetShaderResourceView(slot_,
												 shader_resource_view);

		}

		//////////////////////////////// UAV SETTER ///////////////////////////////////////

		inline UAVSetter::UAVSetter(BaseShaderState& shader_state, unsigned int slot) :
		shader_state_(&shader_state),
		slot_(slot){}

		inline void UAVSetter::operator()(const TValue& unordered_access_view){

			shader_state_->SetUnorderedAccessView(slot_,
												  unordered_access_view);

		}

		//////////////////////////////// CBUFFER SETTER ///////////////////////////////////////

		inline CBufferSetter::CBufferSetter(BaseShaderState& shader_state, unsigned int slot) :
		shader_state_(&shader_state),
		slot_(slot){}

		inline void CBufferSetter::operator()(const TValue& constant_buffer){

			shader_state_->SetConstantBufferView(slot_,
											 constant_buffer);

		}

		//////////////////////////////// SAMPLER SETTER ///////////////////////////////////////

		inline SamplerSetter::SamplerSetter(BaseShaderState& shader_state, unsigned int slot) :
		shader_state_(&shader_state),
		slot_(slot){}

		inline void SamplerSetter::operator()(const TValue& sampler_state){

			shader_state_->SetSamplerStateView(slot_,
									  sampler_state);

		}

		//////////////////////////////// SHADER STATE COMPOSITE ////////////////////////////////////

		template <typename TShader>
		bool ShaderStateComposite::AddShader(const std::string hlsl, const std::string file_name){
			
			const std::wstring entry_point_exception_code = L"X3501";	// Error code returned if the entry point couldn't be found.

			ShaderReflection reflection;

			TShader* shader;

			wstring errors;

			if (FAILED(::MakeShader(*DX11Graphics::GetInstance().GetDevice(),
									hlsl,
									file_name,
									&shader,
									&reflection,
									&errors))){

				if (errors.find(entry_point_exception_code) != wstring::npos){

					return false;		// The entry point couldn't be found.

				}
				else{

					THROW(errors);		// The shader code compilation failed.

				}

			}

			// Add the shader to the composite collection and update the bindings

			shaders_.push_back(std::make_unique<ShaderState<TShader>>(COMMove(&shader),
																	  reflection));

			AddShaderBindings(*shaders_.back());

			return true;

		}

		inline void ShaderStateComposite::Clear(){

			shaders_.clear();

		}

		inline void ShaderStateComposite::Bind(ID3D11DeviceContext& context){

			// Commit pending constant buffers and structured buffers

			for (auto&& committer : committer_table_){

				(*committer.second)(context);

			}
			
			// Bind the shaders to the graphic pipeline

			for (auto&& shader : shaders_){

				shader->Bind(context);

			}

		}

		inline void ShaderStateComposite::Unbind(ID3D11DeviceContext& context){

			for (auto&& shader : shaders_){

				shader->Unbind(context);

			}

		}

		inline size_t ShaderStateComposite::GetShaderCount() const{

			return shaders_.size();

		}

		inline const BaseShaderState& ShaderStateComposite::GetShaderState(size_t index) const{

			return *shaders_[index];

		}

		//////////////////////////////// SET SHADER ///////////////////////////////////////

		template<>
		inline void SetShader<ID3D11VertexShader>(ID3D11DeviceContext& context, const COMPtr<ID3D11VertexShader>& shader){

			context.VSSetShader(shader.Get(),
								nullptr,
								0);

		}

		template<>
		inline void SetShader<ID3D11HullShader>(ID3D11DeviceContext& context, const COMPtr<ID3D11HullShader>& shader){
	
			context.HSSetShader(shader.Get(),
								nullptr,
								0);

		}

		template<>
		inline void SetShader<ID3D11DomainShader>(ID3D11DeviceContext& context, const COMPtr<ID3D11DomainShader>& shader){

			context.DSSetShader(shader.Get(),
								nullptr,
								0);

		}

		template<>
		inline void SetShader<ID3D11GeometryShader>(ID3D11DeviceContext& context, const COMPtr<ID3D11GeometryShader>& shader){

			context.GSSetShader(shader.Get(),
								nullptr,
								0);

		}

		template<>
		inline void SetShader<ID3D11PixelShader>(ID3D11DeviceContext& context, const COMPtr<ID3D11PixelShader>& shader){

			context.PSSetShader(shader.Get(),
								nullptr,
								0);

		}

		template<>
		inline void SetShader<ID3D11ComputeShader>(ID3D11DeviceContext& context, const COMPtr<ID3D11ComputeShader>& shader){

			context.CSSetShader(shader.Get(),
								nullptr,
								0);

		}

		//////////////////////////////// SET CONSTANT BUFFERS ///////////////////////////////////////

		template <typename TShader>
		inline void SetConstantBuffers(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers){

			SetConstantBuffers<TShader>(context,
										start_slot,
										buffers,
										buffers.size());

		}

		template <>
		inline void SetConstantBuffers<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count > 0){

				context.VSSetConstantBuffers(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

			}
						
		}

		template <>
		inline void SetConstantBuffers<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count > 0){

				context.HSSetConstantBuffers(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));
				
			}
						
		}

		template <>
		inline void SetConstantBuffers<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count > 0){

				context.DSSetConstantBuffers(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

			}
			
		}

		template <>
		inline void SetConstantBuffers<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count > 0){

				context.GSSetConstantBuffers(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));
			
			}

		}

		template <>
		inline void SetConstantBuffers<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count > 0){
	
				context.PSSetConstantBuffers(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

			}

		}

		template <>
		inline void SetConstantBuffers<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count > 0){

				context.CSSetConstantBuffers(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

			}

		}

		//////////////////////////////// SET SHADER RESOURCES ///////////////////////////////////////

		template <typename TShader>
		inline void SetShaderResources(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources){

			SetShaderResources<TShader>(context,
									    start_slot,
									    resources,
									    resources.size());

		}

		template <>
		inline void SetShaderResources<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count > 0){

				context.VSSetShaderResources(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

			}
			
		}

		template <>
		inline void SetShaderResources<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count > 0){

				context.HSSetShaderResources(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

			}
			
		}

		template <>
		inline void SetShaderResources<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count == 0){

				context.DSSetShaderResources(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

			}
			
		}

		template <>
		inline void SetShaderResources<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count > 0){

				context.GSSetShaderResources(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));		
			
			}
			
		}

		template <>
		inline void SetShaderResources<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count > 0){

				context.PSSetShaderResources(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

			}

		}

		template <>
		inline void SetShaderResources<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count > 0){

				context.CSSetShaderResources(static_cast<UINT>(start_slot),
											 static_cast<UINT>(count),
											 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

			}

		}

		//////////////////////////////// SET SHADER SAMPLERS ///////////////////////////////////////

		template <typename TShader>
		inline void SetSamplers(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers){

			SetSamplers<TShader>(context,
								 start_slot,
								 samplers,
								 samplers.size());

		}

		template <>
		inline void SetSamplers<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){
			
			if (count > 0){

				context.VSSetSamplers(static_cast<UINT>(start_slot),
									  static_cast<UINT>(count),
									  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

			}

		}

		template <>
		inline void SetSamplers<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count > 0){

				context.HSSetSamplers(static_cast<UINT>(start_slot),
									  static_cast<UINT>(count),
									  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

			}

		}

		template <>
		inline void SetSamplers<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count > 0){

				context.DSSetSamplers(static_cast<UINT>(start_slot),
									  static_cast<UINT>(count),
									  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

			}

		}

		template <>
		inline void SetSamplers<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count > 0){

				context.GSSetSamplers(static_cast<UINT>(start_slot),
									  static_cast<UINT>(count),
									  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

			}

		}

		template <>
		inline void SetSamplers<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count > 0){

				context.PSSetSamplers(static_cast<UINT>(start_slot),
									  static_cast<UINT>(count),
									  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

			}
			
		}
		
		template <>
		inline void SetSamplers<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count > 0){

				context.CSSetSamplers(static_cast<UINT>(start_slot),
									  static_cast<UINT>(count),
									  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

			}
			
		}

		//////////////////////////////// SET SHADER UAV ///////////////////////////////////////

		template <typename TShader>
		inline void SetUnorderedAccess(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11UnorderedAccessView>>& UAVs){

			SetUnorderedAccess<TShader>(context,
										start_slot,
										UAVs,
										UAVs.size());

		}

		template <typename TShader>
		inline void SetUnorderedAccess<TShader>(ID3D11DeviceContext&, size_t, const std::vector<COMPtr<ID3D11UnorderedAccessView>>&, size_t){

			// Do nothing, for now

		}

		template <>
		inline void SetUnorderedAccess<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11UnorderedAccessView>>& UAVs, size_t count){

			if (count > 0){

				context.CSSetUnorderedAccessViews(static_cast<UINT>(start_slot),
												  static_cast<UINT>(count),
												  reinterpret_cast<ID3D11UnorderedAccessView* const*>(std::addressof(UAVs[0])),
												  nullptr);

			}
			
		}

	}

}

#endif