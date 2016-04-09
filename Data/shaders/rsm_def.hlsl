/// \file rsm_def.hlsl
/// \brief This file contains the definition of the reflective shadow maps structure.
/// \author Raffaele D. Facendola

#include "render_def.hlsl"

#ifndef RSM_DEF_HLSL_
#define RSM_DEF_HLSL_

/// Reflective variance shadow map definition
struct RSMBuffer {

	float2 moments : SV_Target0;				// Average depth | Depth variance
	float4 albedo_normal : SV_Target1;			// Albedo.R | Albedo.G | Albedo.B | Normal

};

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

struct VSIn {

	float3 position : SV_Position;
	float3 normal : NORMAL;
	float2 uv: TEXCOORD;

};

struct VSOut {

	float4 position_ps : SV_Position;			// Position in light projection space.
	float3 normal_ws : Normal;					// Normal in world space.
	float2 uv : TexCoord;						// Texture coordinates.

};

cbuffer PerObject {

	float4x4 gWorldLightProj;					// World * Light-view * Light-proj matrix.
	float4x4 gWorld;							// World matrix.

};

void VSMain(VSIn input, out VSOut output) {

	output.position_ps = mul(gWorldLightProj, float4(input.position, 1));
	output.normal_ws = mul((float3x3)gWorld, (float3)input.normal);

	output.uv = float2(input.uv.x,
					   1.0 - input.uv.y);	// V coordinate is flipped because we are using ogl convention.

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

#define PSIn VSOut

Texture2D gDiffuseMap;

SamplerState gDiffuseSampler;

void PSMain(PSIn input, out RSMBuffer output) {

	// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html

	// Variance shadow mapping - Compute the expected value and its variance

	float dx = ddx(input.position_ps.z);
	float dy = ddy(input.position_ps.z);

	output.moments = float2(input.position_ps.z,
							input.position_ps.z * input.position_ps.z + 0.25f * (dx * dx + dy * dy));

	// Albedo

	output.albedo_normal.xyz = gDiffuseMap.Sample(gDiffuseSampler, input.uv).xyz;

	// Normal - Encoded as 2x4-bit values, we don't need good precision anyway

	output.albedo_normal.w = EncodeNormalsCoarse(input.normal_ws);

}

#endif