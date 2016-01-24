#include "dx11/dx11shader_state.h"

#include "dx11/dx11buffer.h"
#include "dx11/dx11gpgpu.h"
#include "dx11/dx11texture.h"
#include "dx11/dx11sampler.h"
#include "dx11/dx11render_target.h"

using namespace gi_lib;
using namespace gi_lib::dx11;

namespace{

	/// \brief Reflect a collection of shader members into an unordered multimap.
	/// \param shader_state Shader state associated to the setters.
	/// \param collection Collection of shader members to enumerate.
	/// \param table Destination unordered map containing the setters.
	/// \param offset Additional offset to the resource slot index.
	template <typename TSetter, typename TCollection>
	void AddShaderBindings(BaseShaderState& shader_state, const TCollection& collection, std::unordered_multimap<size_t, TSetter>& table, int offset = 0){

		for (auto&& item : collection){

			table.insert(std::make_pair(Tag(item.name),
										TSetter(shader_state,
												item.slot + offset)));
			
		}

	}

	/// \brief Calls the setter functions of all the shader members with a particular name.
	/// \param tag Tag associated to the shader members to set.
	/// \param value Actual value to pass to the setter.
	/// \param table Map containing the definition of the shader setters.
	template <typename TSetter, typename TValue>
	bool SetShaderMember(const Tag& tag, TValue&& value, std::unordered_multimap < size_t, TSetter >& table){

		auto&& range = table.equal_range(tag);

		auto it = range.first;

		while (it != range.second){

			it->second(std::forward<TValue>(value));

			++it;

		}

		return range.first != range.second;

	}

}

//////////////////////////////// BASE SHADER STATE //////////////////////////////////////////

vector<ID3D11UnorderedAccessView*> BaseShaderState::GetUnorderedAccessViews() const {

	vector<ID3D11UnorderedAccessView*> uav_list(unordered_access_views_.size());

	std::transform(unordered_access_views_.begin(),
				   unordered_access_views_.end(),
				   uav_list.begin(),
				   [](const COMPtr<ID3D11UnorderedAccessView> uav_ptr) {

						return uav_ptr.Get();
	
				   });
	
	return uav_list;
	
}

//////////////////////////////// SHADER STATE COMPOSITE //////////////////////////////////////

ShaderStateComposite::ShaderStateComposite(){}

ShaderStateComposite::ShaderStateComposite(const ShaderStateComposite& other){

	shaders_.reserve(other.shaders_.size());

	for (auto&& shader : other.shaders_){

		auto shader_instance = shader->Instantiate();

		shaders_.push_back(std::unique_ptr<BaseShaderState>(shader_instance));

		AddShaderBindings(*shader_instance);

	}

}

bool ShaderStateComposite::SetConstantBuffer(const Tag& tag, const ObjectPtr<DX11StructuredBuffer>& constant_buffer){
	
	auto result = SetShaderMember(tag,
								  constant_buffer->GetConstantBuffer(),
								  cbuffer_table_);

	if (result){

		committer_table_[tag] = constant_buffer->GetCommitter();

	}

	return result;

}

bool ShaderStateComposite::SetShaderResource(const Tag& tag, const ObjectPtr<DX11Texture2D>& texture_2D){
	
	return SetShaderMember(tag,
						   texture_2D->GetShaderResourceView(),
						   srv_table_);

}

bool ShaderStateComposite::SetShaderResource(const Tag& tag, const ObjectPtr<DX11Texture2DArray>& texture_2D_array){
	
	return SetShaderMember(tag,
						   texture_2D_array->GetShaderResourceView(),
						   srv_table_);

}

bool ShaderStateComposite::SetShaderResource(const Tag& tag, const ObjectPtr<DX11GPStructuredArray>& gp_structured_array) {
	
	return SetShaderMember(tag,
						   gp_structured_array->GetShaderResourceView(),
						   srv_table_);

}

bool ShaderStateComposite::SetSampler(const Tag& tag, const ObjectPtr<DX11Sampler>& sampler){
	
	return SetShaderMember(tag,
						   sampler->GetSamplerStateView(),
						   sampler_table_);

}

bool ShaderStateComposite::SetShaderResource(const Tag& tag, const ObjectPtr<DX11StructuredArray>& structured_array){

	auto result = SetShaderMember(tag,
								  structured_array->GetShaderResourceView(),
								  srv_table_);

	if (result){

		committer_table_[tag] = structured_array->GetCommitter();

	}

	return result;

}

bool ShaderStateComposite::SetUnorderedAccess(const Tag& tag, const ObjectPtr<DX11GPTexture2D>& gp_texture_2D){
	
	return SetShaderMember(tag,
						   gp_texture_2D->GetUnorderedAccessView(),
						   uav_table_);

}

bool ShaderStateComposite::SetUnorderedAccess(const Tag& tag, const ObjectPtr<DX11GPTexture2DArray>& gp_texture_2D_array){
	
	return SetShaderMember(tag,
						   gp_texture_2D_array->GetUnorderedAccessView(),
						   uav_table_);

}

bool ShaderStateComposite::SetUnorderedAccess(const Tag& tag, const ObjectPtr<DX11GPStructuredArray>& gp_structured_array) {

	return SetShaderMember(tag,
						   gp_structured_array->GetUnorderedAccessView(),
						   uav_table_);
	
}

bool ShaderStateComposite::SetUnorderedAccess(const Tag& tag, const ObjectPtr<DX11ScratchStructuredArray>& scratch_structured_array) {

	return SetShaderMember(tag,
						   scratch_structured_array->GetUnorderedAccessView(),
						   uav_table_);
	
}

void ShaderStateComposite::AddShaderBindings(BaseShaderState& shader_state){

	auto&& reflection = shader_state.GetReflection();

	::AddShaderBindings<CBufferSetter>(shader_state,
									   reflection.buffers,
									   cbuffer_table_);

	::AddShaderBindings<SRVSetter>(shader_state,
								   reflection.shader_resource_views,
								   srv_table_);
	
	::AddShaderBindings<SamplerSetter>(shader_state,
									   reflection.samplers,
									   sampler_table_);

	// If the shader composite is used to bind UAVs inside a pixel shader, their slot must have an offset in order to compensate for the additional render targets
	// being bound as output of the pixel shader.
	int render_targets = 0;

	if (reflection.shader_type == ShaderType::PIXEL_SHADER) {

		render_targets = reflection.pixel_shader.render_targets_;

	}

	::AddShaderBindings<UAVSetter>(shader_state,
								   reflection.unordered_access_views,
								   uav_table_,
								   -render_targets);

}

void ShaderStateComposite::Clear() {

	shaders_.clear();

}

void ShaderStateComposite::Bind(ID3D11DeviceContext& context) {

	Commit(context);

	// Bind the shaders to the graphic pipeline

	for (auto&& shader : shaders_) {

		shader->Bind(context);

	}

}

void ShaderStateComposite::Commit(ID3D11DeviceContext& context) {

	// Commit pending constant buffers and structured buffers

	for (auto&& committer : committer_table_) {

		(*committer.second)(context);

	}


}

void ShaderStateComposite::Unbind(ID3D11DeviceContext& context) {

	for (auto&& shader : shaders_) {

		shader->Unbind(context);

	}

}

void ShaderStateComposite::Bind(ID3D11DeviceContext& context, const ObjectPtr<DX11RenderTarget>& render_target) {

	Bind(context);

	// Only pixel shader's UAV are supported

	auto it = std::find_if(shaders_.begin(),
						   shaders_.end(),
						   [](const unique_ptr<BaseShaderState>& shader_state) {

								return shader_state->GetReflection().shader_type == ShaderType::PIXEL_SHADER;

						   });

	if (it != shaders_.end()) {

		// Bind the render target as well as the provided UAVs

		render_target->Bind(context,
							(*it)->GetUnorderedAccessViews());

	}
	else {

		// No UAV to set, bind the render target to avoid unexpected behaviors client-side

		render_target->Bind(context);

	}

}

void ShaderStateComposite::Unbind(ID3D11DeviceContext& context, const ObjectPtr<DX11RenderTarget>& render_target) {

	Unbind(context);

	// Only pixel shader's UAV are supported

	auto it = std::find_if(shaders_.begin(),
						   shaders_.end(),
						   [](const unique_ptr<BaseShaderState>& shader_state) {

								return shader_state->GetReflection().shader_type == ShaderType::PIXEL_SHADER;

						   });

	if (it != shaders_.end()) {

		// Unbind the render target as well as the provided UAVs

		render_target->Unbind(context,
							  (*it)->GetUnorderedAccessViews());

	}
	else {

		// No UAV to remove, unbind the render target to avoid unexpected behaviors client-side

		render_target->Unbind(context);

	}

}