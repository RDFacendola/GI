#include "render_def.hlsl"

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

struct VSIn{

	float4 position : SV_Position;
	float3 normal : NORMAL;
	float2 uv: TEXCOORD;
	float3 tangent  : TANGENT;
	float3 binormal : BINORMAL;

};

struct VSOut
{

	float4 position_ps : SV_Position;			// Position in projection space.
	float3 normal_ws : Normal;					// Normal in world space.
	float2 uv : TexCoord;						// Texture coordinates.
	float3 tangent_ws: Tangent;					// Tangent in world space.
	float3 binormal_ws: Binormal;				// Bitnormal in world space.

};

cbuffer PerObject{

	float4x4 gWorldViewProj;					// World * View * Projection matrix.
	float4x4 gWorld;							// World matrix.

};

void VSMain(VSIn input, out VSOut output){

	output.position_ps = mul(gWorldViewProj, input.position);

	output.normal_ws = mul((float3x3)gWorld, (float3)input.normal);
	output.tangent_ws = mul((float3x3)gWorld, (float3)input.tangent);
	output.binormal_ws = mul((float3x3)gWorld, (float3)input.binormal);

	output.uv = float2(input.uv.x,
					   1.0 - input.uv.y);	// V coordinate is flipped because we are using ogl convention.

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

Texture2D gDiffuseMap;
Texture2D gNormalMap;
Texture2D gSpecularMap;

SamplerState gDiffuseSampler;

cbuffer PerMaterial {

	float gShininess;

	float3 reserved;

};

void PSMain(VSOut input, out GBuffer output){

	output.albedo = gDiffuseMap.Sample(gDiffuseSampler, input.uv);

	clip(output.albedo.a < 0.1f ? -1 : 1);
	
	float3 normals_ws = gNormalMap.Sample(gDiffuseSampler, input.uv).xyz;

	float3x3 tangent_matrix = float3x3(normalize(input.tangent_ws), 
									   normalize(input.binormal_ws), 
									   normalize(input.normal_ws));

	normals_ws = normalize(mul(normals_ws * 2.0f - 1.0f, tangent_matrix));

	output.normal_specular_shininess.xy = EncodeNormals(normals_ws);
	output.normal_specular_shininess.z = gSpecularMap.Sample(gDiffuseSampler, input.uv).r;
	output.normal_specular_shininess.w = gShininess;

}
