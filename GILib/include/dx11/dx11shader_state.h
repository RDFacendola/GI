/// \file dx11shader_state.h
/// \brief This file contains classes used to manage DirectX11 shader states.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "dx11/dx11.h"
#include "dx11/dx11shader.h"

#include "windows/win_os.h"

#ifdef _WIN32

namespace gi_lib{

	namespace dx11{

		using windows::unique_com;

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
			void SetShaderResourceView(unsigned int slot, ID3D11ShaderResourceView* shader_resource_view);

			/// \brief Set an unordered access view for this shader.
			/// \param slot Index of the slot where the view will be bound.
			/// \param unordered_access_view The view to bind.
			void SetUnorderedAccessView(unsigned int slot, ID3D11UnorderedAccessView* unordered_access_view);

			/// \brief Set a constant buffer for this shader.
			/// \param slot Index of the slot where the buffer will be bound.
			/// \param constant_buffer The buffer to bind.
			void SetConstantBuffer(unsigned int slot, ID3D11Buffer* constant_buffer);

			/// \brief Set a sampler for this shader.
			/// \param slot Index of the slot where the sampler will be bound.
			/// \param sampler_state The sampler to bind.
			void SetSampler(unsigned int slot, ID3D11SamplerState* sampler_state);

			/// \brief Bind the shader to the given device context.
			virtual void Bind(ID3D11DeviceContext& context) = 0;

			/// \brief Unbind the shader from the given device context.
			virtual void Unbind(ID3D11DeviceContext& context) = 0;

		private:

			vector<ID3D11ShaderResourceView*> shader_resource_views_;		///< \brief List of shader resource views.

			vector<ID3D11UnorderedAccessView*> unordered_access_views_;		///< \brief List of unordered access views.

			vector<ID3D11Buffer*> constant_buffers_;						///< \brief List of constant buffers.

			vector<ID3D11SamplerState*> samplers_;							///< \brief List of sampler states.

		};

		/// \brief Concrete shader state.
		/// \tparam TShader Type of shader this state refers to.
		/// \author Raffaele D. Facendola.
		template <typename TShader>
		class ShaderState : public BaseShaderState{

		public:

			ShaderState(TShader& shader, const ShaderReflection& reflection);

			virtual void Bind(ID3D11DeviceContext& context) override;

			virtual void Unbind(ID3D11DeviceContext& context) override;

		private:

			unique_com<TShader> shader_;		/// \brief Pointer to the concrete shader.

		};

		/// \brief Functor used to set a shader resource view to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class SRVSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = ID3D11ShaderResourceView;

			/// \brief Set a shader resource view for this shader.
			/// \param shader_state Shader state where the view will be bound.
			/// \param slot Index of the slot where the view will be bound.
			SRVSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set a shader resource view to the bound shader state.
			/// \param shader_resource_view The view to bind.
			void operator()(TValue* shader_resource_view);

		private:

			BaseShaderState* shader_state_;

			unsigned int slot_;

		};

		/// \brief Functor used to set an unordered access view to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class UAVSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = ID3D11UnorderedAccessView;

			/// \brief Create a new unordered access view setter.
			/// \param shader_state Shader state where the view will be bound.
			/// \param slot Index of the slot where the view will be bound.
			UAVSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set an unordered access view for this shader.
			/// \param unordered_access_view The view to bind.
			void operator()(TValue* unordered_access_view);

		private:

			BaseShaderState* shader_state_;

			unsigned int slot_;

		};

		/// \brief Functor used to set a constant buffer to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class CBufferSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = ID3D11Buffer;

			/// \brief Create a new constant buffer setter.
			/// \param shader_state Shader state where the buffer will be bound.
			/// \param slot Index of the slot where the buffer will be bound.
			CBufferSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set a constant buffer for this shader.
			/// \param constant_buffer The buffer to bind.
			void operator()(TValue* constant_buffer);

		private:

			BaseShaderState* shader_state_;

			unsigned int slot_;

		};

		/// \brief Functor used to set an unordered access view to a shader state in a given slot.
		/// \author Raffaele D. Facendola.
		class SamplerSetter{

		public:

			/// \brief Type of value set by this setter.
			using TValue = ID3D11SamplerState;

			/// \brief Create a new sampler setter.
			/// \param shader_state Shader state where the sampler will be bound.
			/// \param slot Index of the slot where the sampler will be bound.
			SamplerSetter(BaseShaderState& shader_state, unsigned int slot);

			/// \brief Set a sampler for this shader.
			/// \param sampler_state The sampler to bind.
			void operator()(TValue* sampler_state);

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
			void operator()(typename TSetter::TValue* value);

		private:

			std::vector<TSetter> setters_;			///< \brief List of setters.

		};

		/// \brief Bind a shader to a render context.
		/// \param context Context the shader will be bound to.
		/// \param shader Shader to bound.
		template <typename TShader>
		void SetShader(ID3D11DeviceContext& context, TShader* shader);

		/// \brief Bind some constant buffers to a render context.
		/// \param start_slot Slot where the first buffer will be bound to.
		/// \param context Context the constant buffers will be bound to.
		/// \param buffers Array containing the constant buffers.
		/// \param count Number of buffers.
		template <typename TShader>
		void SetConstantBuffers(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const* buffers, size_t count);
		
		/// \brief Bind some shader resources to a render context.
		/// \param start_slot Slot where the first resource will be bound to.
		/// \param context Context the resources will be bound to.
		/// \param buffers Array containing the shader resource views.
		/// \param count Number of resources.
		template <typename TShader>
		void SetShaderResources(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const* resources, size_t count);

		/// \brief Bind some samplers to a render context.
		/// \param start_slot Slot where the first sampler will be bound to.
		/// \param context Context the samplers will be bound to.
		/// \param buffers Array containing the sampler states.
		/// \param count Number of samplers.
		template <typename TShader>
		void SetShaderSamplers(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const* samplers, size_t count);

		/// \brief Bind some unordered access views to a render context.
		/// \param start_slot Slot where the first UAV will be bound to.
		/// \param context Context the UAVs will be bound to.
		/// \param UAVs Array containing the unordered access views.
		/// \param count Number of resources.
		template <typename TShader>
		void SetShaderUAV(ID3D11DeviceContext& context, size_t start_slot, ID3D11UnorderedAccessView* const* UAVs, size_t count);
		
	}

}

//////////////////////////////// BASE SHADER STATE ///////////////////////////////////////

inline gi_lib::dx11::BaseShaderState::BaseShaderState(size_t srv_count, size_t uav_count, size_t buffer_count, size_t sampler_count){

	shader_resource_views_.resize(srv_count);
	unordered_access_views_.resize(uav_count);
	constant_buffers_.resize(buffer_count);
	samplers_.resize(sampler_count);

}

inline void gi_lib::dx11::BaseShaderState::SetShaderResourceView(unsigned int slot, ID3D11ShaderResourceView* shader_resource_view){

	shader_resource_views_[slot] = shader_resource_view;

}

inline void gi_lib::dx11::BaseShaderState::SetUnorderedAccessView(unsigned int slot, ID3D11UnorderedAccessView* unordered_access_view){

	unordered_access_views_[slot] = unordered_access_view;

}

inline void gi_lib::dx11::BaseShaderState::SetConstantBuffer(unsigned int slot, ID3D11Buffer* constant_buffer){

	constant_buffers_[slot] = constant_buffer;

}

inline void gi_lib::dx11::BaseShaderState::SetSampler(unsigned int slot, ID3D11SamplerState* sampler_state){

	samplers_[slot] = sampler_state;

}

//////////////////////////////// SHADER STATE ///////////////////////////////////////

template <typename TShader>
inline gi_lib::dx11::ShaderState<TShader>::ShaderState(TShader& shader, const ShaderReflection& reflection) :
BaseShaderState(reflection.shader_resource_views.size(),
				reflection.unordered_access_views.size(),
				reflection.buffers.size(),
				reflection.samplers.size()){}

template <typename TShader>
void gi_lib::dx11::ShaderState<TShader>::Bind(ID3D11DeviceContext& context){

	SetShader<TShader>(context,
					   shader_.get());
	
	SetShaderResources(context,
					   0,
					   &shader_resource_views_[0],
					   shader_resource_views_.size());

	SetShaderUAV(context,
				 0,
				 &unordered_access_views_[0],
				 unordered_access_views_.size());

	SetConstantBuffers(context,
					   0,
					   &constant_buffers_[0],
					   constant_buffers_.size());

	SetShaderSamplers(context,
					  0,
					  &samplers_[0],
					  samplers_.size());

}

template <typename TShader>
void gi_lib::dx11::ShaderState<TShader>::Unbind(ID3D11DeviceContext& context){

	static vector<ID3D11ShaderResourceView*> null_srv(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
	static vector<ID3D11UnorderedAccessView*> null_uav(D3D11_1_UAV_SLOT_COUNT);
	static vector<ID3D11Buffer*> null_buffers(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
	static vector<ID3D11SamplerState*> null_samplers(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);

	SetShader<TShader>(context,
					   nullptr);
	
	SetShaderResources(context,
					   0,
					   &null_srv[0],
					   shader_resource_views_.size());

	SetShaderUAV(context,
				 0,
				 &null_uav[0],
				 unordered_access_views_.size());

	SetConstantBuffers(context,
					   0,
					   &null_buffers[0],
					   constant_buffers_.size());

	SetShaderSamplers(context,
					  0,
					  &null_samplers[0],
					  samplers_.size());

}

//////////////////////////////// SRV SETTER ///////////////////////////////////////

inline gi_lib::dx11::SRVSetter::SRVSetter(BaseShaderState& shader_state, unsigned int slot) :
shader_state_(&shader_state),
slot_(slot){}

inline void gi_lib::dx11::SRVSetter::operator()(TValue* shader_resource_view){

	shader_state_->SetShaderResourceView(slot_,
										 shader_resource_view);

}

//////////////////////////////// UAV SETTER ///////////////////////////////////////

inline gi_lib::dx11::UAVSetter::UAVSetter(BaseShaderState& shader_state, unsigned int slot) :
shader_state_(&shader_state),
slot_(slot){}

inline void gi_lib::dx11::UAVSetter::operator()(TValue* unordered_access_view){

	shader_state_->SetUnorderedAccessView(slot_,
										  unordered_access_view);

}

//////////////////////////////// CBUFFER SETTER ///////////////////////////////////////

inline gi_lib::dx11::CBufferSetter::CBufferSetter(BaseShaderState& shader_state, unsigned int slot) :
shader_state_(&shader_state),
slot_(slot){}

inline void gi_lib::dx11::CBufferSetter::operator()(TValue* constant_buffer){

	shader_state_->SetConstantBuffer(slot_,
									 constant_buffer);

}

//////////////////////////////// SAMPLER SETTER ///////////////////////////////////////

inline gi_lib::dx11::SamplerSetter::SamplerSetter(BaseShaderState& shader_state, unsigned int slot) :
shader_state_(&shader_state),
slot_(slot){}

inline void gi_lib::dx11::SamplerSetter::operator()(TValue* sampler_state){

	shader_state_->SetSampler(slot_,
							  sampler_state);

}

//////////////////////////////// COMPOSITE SETTER ///////////////////////////////////////

template <typename TSetter>
void gi_lib::dx11::CompositeSetter<TSetter>::AddSetter(BaseShaderState& shader_state, unsigned int slot){

	setters_.emplace_back({ shader_state, 
							slot });

}

template <typename TSetter>
void gi_lib::dx11::CompositeSetter<TSetter>::operator()(typename TSetter::TValue* value){

	for (auto&& setter : setters_){

		setter(value);

	}
}

//////////////////////////////// SET SHADER ///////////////////////////////////////

template<>
inline void gi_lib::dx11::SetShader<ID3D11VertexShader>(ID3D11DeviceContext& context, ID3D11VertexShader* shader){

	context.VSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11HullShader>(ID3D11DeviceContext& context, ID3D11HullShader* shader){
	
	context.HSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11DomainShader>(ID3D11DeviceContext& context, ID3D11DomainShader* shader){

	context.DSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11GeometryShader>(ID3D11DeviceContext& context, ID3D11GeometryShader* shader){

	context.GSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11PixelShader>(ID3D11DeviceContext& context, ID3D11PixelShader* shader){

	context.PSSetShader(shader,
						nullptr,
						0);

}

template<>
inline void gi_lib::dx11::SetShader<ID3D11ComputeShader>(ID3D11DeviceContext& context, ID3D11ComputeShader* shader){

	context.CSSetShader(shader,
						nullptr,
						0);

}

//////////////////////////////// SET CONSTANT BUFFERS ///////////////////////////////////////

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.VSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.HSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.DSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.GSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.PSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}

template <>
inline void gi_lib::dx11::SetConstantBuffers<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11Buffer* const * buffers, size_t count){

	context.CSSetConstantBuffers(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 buffers);

}
	
//////////////////////////////// SET SHADER RESOURCES ///////////////////////////////////////

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.VSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){
	
	context.HSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.DSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.GSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.PSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

template <>
inline void gi_lib::dx11::SetShaderResources<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11ShaderResourceView* const * resources, size_t count){

	context.CSSetShaderResources(static_cast<UINT>(start_slot),
								 static_cast<UINT>(count),
								 resources);

}

//////////////////////////////// SET SHADER SAMPLERS ///////////////////////////////////////

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11VertexShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

	context.VSSetSamplers(static_cast<UINT>(start_slot),
						  static_cast<UINT>(count),
						  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11HullShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){
	
	context.HSSetSamplers(static_cast<UINT>(start_slot),
						  static_cast<UINT>(count),
						  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11DomainShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

	context.DSSetSamplers(static_cast<UINT>(start_slot),
						  static_cast<UINT>(count),
						  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11GeometryShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

	context.GSSetSamplers(static_cast<UINT>(start_slot),
						  static_cast<UINT>(count),
						  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11PixelShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

	context.PSSetSamplers(static_cast<UINT>(start_slot),
						  static_cast<UINT>(count),
						  samplers);

}

template <>
inline void gi_lib::dx11::SetShaderSamplers<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11SamplerState* const * samplers, size_t count){

	context.CSSetSamplers(static_cast<UINT>(start_slot),
						  static_cast<UINT>(count),
						  samplers);

}
	
//////////////////////////////// SET SHADER UAV ///////////////////////////////////////

template <typename TShader>
inline void gi_lib::dx11::SetShaderUAV<TShader>(ID3D11DeviceContext&, size_t, ID3D11UnorderedAccessView* const*, size_t){

	// Do nothing, for now

}

template <>
inline void gi_lib::dx11::SetShaderUAV<ID3D11ComputeShader>(ID3D11DeviceContext& context, size_t start_slot, ID3D11UnorderedAccessView* const* UAVs, size_t count){

	context.CSSetUnorderedAccessViews(static_cast<UINT>(start_slot),
									  static_cast<UINT>(count),
									  UAVs,
									  nullptr);

}

#endif