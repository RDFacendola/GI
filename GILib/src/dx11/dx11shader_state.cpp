#include "dx11/dx11shader_state.h"

#include "dx11/dx11buffer.h"
#include "dx11/dx11texture.h"

using namespace gi_lib;
using namespace gi_lib::dx11;

//////////////////////////////// SHADER MASTER TABLE //////////////////////////////////////

bool ShaderStateComposite::SetConstantBuffer(const Tag& tag, const ObjectPtr<IStructuredBuffer>& constant_buffer){

	auto it = cbuffer_table_.find(tag);

	if (it != cbuffer_table_.end()){

		// TODO: Check if the size of the structured buffer is legit.

		it->second(static_cast<DX11StructuredBuffer&>(*constant_buffer).GetConstantBuffer());

		return true;

	}

	return false;

}

bool ShaderStateComposite::SetShaderResource(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D){

	auto it = srv_table_.find(tag);

	if (it != srv_table_.end()){

		// TODO: Check if the specified resource is an actual Texture2D

		it->second(static_cast<DX11Texture2D&>(*texture_2D).GetShaderResourceView());

		return true;

	}

	return false;

}

bool ShaderStateComposite::SetShaderResource(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array){

	auto it = srv_table_.find(tag);

	if (it != srv_table_.end()){

		// TODO: Check if the specified resource is an actual structured buffer

		it->second(static_cast<DX11StructuredArray&>(*structured_array).GetShaderResourceView());

		return true;

	}

	return false;

}

bool ShaderStateComposite::SetUnorderedAccess(const Tag& tag, const ObjectPtr<IGPTexture2D>& gp_texture_2D){

	auto it = uav_table_.find(tag);

	if (it != uav_table_.end()){

		// TODO: Check if the specified resource is an actual

		

		return true;

	}

	return false;

}

void ShaderStateComposite::AddShaderBindings(const BaseShaderState& shader, const ShaderReflection& reflection){



}