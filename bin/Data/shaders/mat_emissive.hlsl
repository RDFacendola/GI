#include "render_def.hlsl"

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

cbuffer PerObject{

	float4x4 gWorldViewProj;					// World * View * Projection matrix.
	float4x4 gWorld;							// World matrix.

};

float4 VSMain(float3 input : SV_Position) : SV_Position{

	return mul(gWorldViewProj, float4(input, 1));

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

cbuffer PerMaterial {

	float4 gColor;
	
};

void PSMain(float4 input : SV_Position, out GBuffer output){

	output.albedo_emissivity.xyz = gColor.xyz;
	output.albedo_emissivity.w = 1.f;
	output.normal_specular_shininess = 0.f;
	
}
