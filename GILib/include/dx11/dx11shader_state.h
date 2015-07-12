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

#include "windows/win_os.h"

#ifdef _WIN32

namespace gi_lib{

	class IStructuredBuffer;
	class IStructuredArray;
	class ITexture2D;
	class IGPTexture2D;

	namespace dx11{

		using windows::COMPtr;

		/// \brief Base class for a shader state.
		/// \author Raffaele D. Facendola
		class BaseShaderState{

		public:

			/// \brief Create a new shader state.
			/// \param srv_count Number of shader resource views required by the shader.
			/// \param uav_count Number of unordered access views required by the shader.
			/// \param buffer_count Number of constant buffers required by the shader.
			/// \param sampler_count Number of samplers required by the shader.
			BaseShaderState(size_t srv_count, size_t uav_count, size_t buffer_count, size_t sampler_count);

			/// \brief Virtual destructor.
			virtual ~BaseShaderState(){}

			/// \brief Set a shader resource view for this shader.
			/// \param slot Index of the slot where the view will be bound.
			/// \param shader_resource_view The view to bind.
			void SetShaderResourceView(unsigned int slot, const COMPtr<ID3D11ShaderResourceView>& shader_resource_view);

			/// \brief Set an unordered access view for this shader.
			/// \param slot Index of the slot where the view will be bound.
			/// \param unordered_access_view The view to bind.
			void SetUnorderedAccessView(unsigned int slot, const COMPtr<ID3D11UnorderedAccessView>& unordered_access_view);

			/// \brief Set a constant buffer for this shader.
			/// \param slot Index of the slot where the buffer will be bound.
			/// \param constant_buffer The buffer to bind.
			void SetConstantBuffer(unsigned int slot, const COMPtr<ID3D11Buffer>& constant_buffer);

			/// \brief Set a sampler for this shader.
			/// \param slot Index of the slot where the sampler will be bound.
			/// \param sampler_state The sampler to bind.
			void SetSampler(unsigned int slot, const COMPtr<ID3D11SamplerState>& sampler_state);

			/// \brief Bind the shader to the given device context.
			virtual void Bind(ID3D11DeviceContext& context) = 0;

			/// \brief Unbind the shader from the given device context.
			virtual void Unbind(ID3D11DeviceContext& context) = 0;

		protected:

			std::vector<COMPtr<ID3D11ShaderResourceView>> shader_resource_views_;		///< \brief List of shader resource views.

			std::vector<COMPtr<ID3D11UnorderedAccessView>> unordered_access_views_;		///< \brief List of unordered access views.

			std::vector<COMPtr<ID3D11Buffer>> constant_buffers_;						///< \brief List of constant buffers.

			std::vector<COMPtr<ID3D11SamplerState>> samplers_;							///< \brief List of sampler states.

		};

		/// \brief Concrete shader state.
		/// \tparam TShader Type of shader this state refers to.
		/// \author Raffaele D. Facendola.
		template <typename TShader>
		class ShaderState : public BaseShaderState{

		public:

			ShaderState(const COMPtr<TShader>& shader, const ShaderReflection& reflection);

			virtual void Bind(ID3D11DeviceContext& context) override;

			virtual void Unbind(ID3D11DeviceContext& context) override;

		private:

			COMPtr<TShader> shader_;		/// \brief Pointer to the concrete shader.

		};

		/// \brief Functor used to set a shader resource view to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class SRVSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = COMPtr<ID3D11ShaderResourceView>;

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
			using TValue = COMPtr<ID3D11UnorderedAccessView>;

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
			using TValue = COMPtr<ID3D11Buffer>;

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
			using TValue = COMPtr<ID3D11SamplerState>;

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

		/// \brief Composite collection of setters of the same type.
		/// \author Raffaele D. Facendola.
		template <typename TSetter>
		class CompositeSetter{

		public:

			/// \brief Add a new setter to the collection.
			/// \param shader_state Shader state bound to the setter.
			/// \param slot Slot index where the setter will write.
			void AddSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set the same value for each setter stored so far.
			/// \param value Value to set.
			void operator()(const typename TSetter::TValue& value);

		private:

			std::vector<TSetter> setters_;			///< \brief List of setters.

		};

		/// \brief Manages a collection of shaders and their state.
		/// \author Raffaele D. Facendola.
		class ShaderStateComposite{

		public:

			
			template <typename TShader>
			bool AddShader(const std::string hlsl, const std::string file_name);

			/// \brief Destroy all the shader states stored inside this instance.
			void Clear();

			/// \brief Bind the shaders to the given device context.
			virtual void Bind(ID3D11DeviceContext& context);

			/// \brief Unbind the shaders from the given device context.
			virtual void Unbind(ID3D11DeviceContext& context);

			bool SetConstantBuffer(const Tag& tag, const ObjectPtr<IStructuredBuffer>& constant_buffer);

			bool SetShaderResource(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D);

			bool SetShaderResource(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array);

			bool SetUnorderedAccess(const Tag& tag, const ObjectPtr<IGPTexture2D>& gp_texture_2D);

			//bool SetSampler(const Tag& tag, const ObjectPtr<ISampler>& sampler);

		private:

			void AddShaderBindings(BaseShaderState& shader, const ShaderReflection& reflection);
			
			std::vector<std::unique_ptr<BaseShaderState>> shaders_;							///< \brief Shader collection.

			std::unordered_map<size_t, CompositeSetter<CBufferSetter>> cbuffer_table_;		///< \brief Table of constant buffers.

			std::unordered_map<size_t, CompositeSetter<SRVSetter>> srv_table_;				///< \brief Table of shader resource views.

			std::unordered_map<size_t, CompositeSetter<UAVSetter>> uav_table_;				///< \brief Table of unordered access views.

			std::unordered_map<size_t, CompositeSetter<SamplerSetter>> sampler_table_;		///< \brief Table of samplers.

		};

		/// \brief Bind a shader to a render context.
		/// \param context Context the shader will be bound to.
		/// \param shader Shader to bound.
		template <typename TShader>
		void SetShader(ID3D11DeviceContext& context, const COMPtr<TShader>& shader);

		/// \brief Bind some constant buffers to a render context.
		/// \param start_slot Slot where the first buffer will be bound to.
		/// \param context Context the constant buffers will be bound to.
		/// \param buffers Array containing the constant buffers.
		/// \param count Number of buffers.
		template <typename TShader>
		void SetConstantBuffers(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count = 0);
		
		/// \brief Bind some shader resources to a render context.
		/// \param start_slot Slot where the first resource will be bound to.
		/// \param context Context the resources will be bound to.
		/// \param buffers Array containing the shader resource views.
		/// \param count Number of resources.
		template <typename TShader>
		void SetShaderResources(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count = 0);

		/// \brief Bind some samplers to a render context.
		/// \param start_slot Slot where the first sampler will be bound to.
		/// \param context Context the samplers will be bound to.
		/// \param buffers Array containing the sampler states.
		/// \param count Number of samplers.
		template <typename TShader>
		void SetSamplers(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count = 0);

		/// \brief Bind some unordered access views to a render context.
		/// \param start_slot Slot where the first UAV will be bound to.
		/// \param context Context the UAVs will be bound to.
		/// \param UAVs Array containing the unordered access views.
		/// \param count Number of resources.
		template <typename TShader>
		void SetUnorderedAccess(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11UnorderedAccessView>>& UAVs, size_t count = 0);
				
		//////////////////////////////// BASE SHADER STATE ///////////////////////////////////////

		inline BaseShaderState::BaseShaderState(size_t srv_count, size_t uav_count, size_t buffer_count, size_t sampler_count){

			shader_resource_views_.resize(srv_count);
			unordered_access_views_.resize(uav_count);
			constant_buffers_.resize(buffer_count);
			samplers_.resize(sampler_count);

		}

		inline void BaseShaderState::SetShaderResourceView(unsigned int slot, const COMPtr<ID3D11ShaderResourceView>& shader_resource_view){

			shader_resource_views_[slot] = shader_resource_view;

		}

		inline void BaseShaderState::SetUnorderedAccessView(unsigned int slot, const COMPtr<ID3D11UnorderedAccessView>& unordered_access_view){

			unordered_access_views_[slot] = unordered_access_view;

		}

		inline void BaseShaderState::SetConstantBuffer(unsigned int slot, const COMPtr<ID3D11Buffer>& constant_buffer){

			constant_buffers_[slot] = constant_buffer;

		}

		inline void BaseShaderState::SetSampler(unsigned int slot, const COMPtr<ID3D11SamplerState>& sampler_state){

			samplers_[slot] = sampler_state;

		}

		//////////////////////////////// SHADER STATE ///////////////////////////////////////

		template <typename TShader>
		inline ShaderState<TShader>::ShaderState(const COMPtr<TShader>& shader, const ShaderReflection& reflection) :
		BaseShaderState(reflection.shader_resource_views.size(),
						reflection.unordered_access_views.size(),
						reflection.buffers.size(),
						reflection.samplers.size()),
		shader_(shader){}

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

			shader_state_->SetConstantBuffer(slot_,
											 constant_buffer);

		}

		//////////////////////////////// SAMPLER SETTER ///////////////////////////////////////

		inline SamplerSetter::SamplerSetter(BaseShaderState& shader_state, unsigned int slot) :
		shader_state_(&shader_state),
		slot_(slot){}

		inline void SamplerSetter::operator()(const TValue& sampler_state){

			shader_state_->SetSampler(slot_,
									  sampler_state);

		}

		//////////////////////////////// COMPOSITE SETTER ///////////////////////////////////////

		template <typename TSetter>
		void CompositeSetter<TSetter>::AddSetter(BaseShaderState& shader_state, unsigned int slot){

			setters_.emplace_back(TSetter{ shader_state, 
										   slot });

		}

		template <typename TSetter>
		inline void CompositeSetter<TSetter>::operator()(const typename TSetter::TValue& value){

			for (auto&& setter : setters_){

				setter(value);

			}
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

			AddShaderBindings(*shaders_.back(),
							  reflection);

			return true;

		}

		inline void ShaderStateComposite::Clear(){

			shaders_.clear();

		}

		inline void ShaderStateComposite::Bind(ID3D11DeviceContext& context){

			for (auto&& shader : shaders_){

				shader->Bind(context);

			}

		}

		inline void ShaderStateComposite::Unbind(ID3D11DeviceContext& context){

			for (auto&& shader : shaders_){

				shader->Unbind(context);

			}

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

		template <>
		inline void SetConstantBuffers<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count == 0){

				count = buffers.size();

			}

			context.VSSetConstantBuffers(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

		}

		template <>
		inline void SetConstantBuffers<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count == 0){

				count = buffers.size();

			}

			context.HSSetConstantBuffers(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

		}

		template <>
		inline void SetConstantBuffers<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count == 0){

				count = buffers.size();

			}

			context.DSSetConstantBuffers(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

		}

		template <>
		inline void SetConstantBuffers<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count == 0){

				count = buffers.size();

			}

			context.GSSetConstantBuffers(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

		}

		template <>
		inline void SetConstantBuffers<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count == 0){

				count = buffers.size();

			}

			context.PSSetConstantBuffers(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

		}

		template <>
		inline void SetConstantBuffers<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11Buffer>>& buffers, size_t count){

			if (count == 0){

				count = buffers.size();

			}

			context.CSSetConstantBuffers(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11Buffer* const*>(std::addressof(buffers[0])));

		}

		//////////////////////////////// SET SHADER RESOURCES ///////////////////////////////////////

		template <>
		inline void SetShaderResources<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count == 0){

				count = resources.size();

			}

			context.VSSetShaderResources(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

		}

		template <>
		inline void SetShaderResources<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count == 0){

				count = resources.size();

			}

			context.HSSetShaderResources(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

		}

		template <>
		inline void SetShaderResources<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count == 0){

				count = resources.size();

			}

			context.DSSetShaderResources(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

		}

		template <>
		inline void SetShaderResources<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count == 0){

				count = resources.size();

			}

			context.GSSetShaderResources(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

		}

		template <>
		inline void SetShaderResources<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count == 0){

				count = resources.size();

			}

			context.PSSetShaderResources(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

		}

		template <>
		inline void SetShaderResources<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11ShaderResourceView>>& resources, size_t count){

			if (count == 0){

				count = resources.size();

			}

			context.CSSetShaderResources(static_cast<UINT>(start_slot),
										 static_cast<UINT>(count),
										 reinterpret_cast<ID3D11ShaderResourceView* const*>(std::addressof(resources[0])));

		}

		//////////////////////////////// SET SHADER SAMPLERS ///////////////////////////////////////

		template <>
		inline void SetSamplers<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){
			
			if (count == 0){

				count = samplers.size();

			}

			context.VSSetSamplers(static_cast<UINT>(start_slot),
								  static_cast<UINT>(count),
								  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

		}

		template <>
		inline void SetSamplers<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count == 0){

				count = samplers.size();

			}

			context.HSSetSamplers(static_cast<UINT>(start_slot),
								  static_cast<UINT>(count),
								  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

		}

		template <>
		inline void SetSamplers<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count == 0){

				count = samplers.size();

			}

			context.DSSetSamplers(static_cast<UINT>(start_slot),
								  static_cast<UINT>(count),
								  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

		}

		template <>
		inline void SetSamplers<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count == 0){

				count = samplers.size();

			}

			context.GSSetSamplers(static_cast<UINT>(start_slot),
								  static_cast<UINT>(count),
								  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

		}

		template <>
		inline void SetSamplers<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count == 0){

				count = samplers.size();

			}

			context.PSSetSamplers(static_cast<UINT>(start_slot),
								  static_cast<UINT>(count),
								  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

		}
		
		template <>
		inline void SetSamplers<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11SamplerState>>& samplers, size_t count){

			if (count == 0){

				count = samplers.size();

			}

			context.CSSetSamplers(static_cast<UINT>(start_slot),
								  static_cast<UINT>(count),
								  reinterpret_cast<ID3D11SamplerState* const*>(std::addressof(samplers[0])));

		}

		//////////////////////////////// SET SHADER UAV ///////////////////////////////////////

		template <typename TShader>
		inline void gi_lib::dx11::SetUnorderedAccess<TShader>(ID3D11DeviceContext&, size_t, const std::vector<COMPtr<ID3D11UnorderedAccessView>>&, size_t){

			// Do nothing, for now

		}

		template <>
		inline void gi_lib::dx11::SetUnorderedAccess<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, const std::vector<COMPtr<ID3D11UnorderedAccessView>>& UAVs, size_t count){

			if (count == 0){

				count = UAVs.size();

			}

			context.CSSetUnorderedAccessViews(static_cast<UINT>(start_slot),
											  static_cast<UINT>(count),
											  reinterpret_cast<ID3D11UnorderedAccessView* const*>(std::addressof(UAVs[0])),
											  nullptr);

		}

	}

}

#endif