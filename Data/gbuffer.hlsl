

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

struct VSIn{

	float4 position : SV_Position;
	float4 normal : NORMAL;
	float2 uv: TEXCOORD;

};

struct VSOut
{

	float4 position_ps : SV_Position;			// Position in projection space.
	float3 normal_ws : Normal;					// Normal in world space.
	float2 uv : TexCoord;						// Texture coordinates.
		
};

cbuffer PerObject{

	float4x4 gWorldViewProj;					// World * View * Projection matrix.
	float4x4 gWorld;							// World matrix.

};

void VSMain(VSIn input, out VSOut output){

	output.position_ps = mul(gWorldViewProj, input.position);	

	output.normal_ws = mul((float3x3)gWorld, (float3)input.normal);

	output.uv = float2(input.uv.x,
					   1.0 - input.uv.y);	// V coordinate is flipped because we are using ogl convention.

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

struct GBuffer{

	float4 albedo : SV_Target0;					// Diffuse.R | Diffuse.G | Diffuse.B | Diffuse.A
	float4 normal_shininess : SV_Target1;		// NormalWS.X | NormalWS.Y | NormalWS.Z | Shininess

};

Texture2D gDiffuseMap;

SamplerState gDiffuseSampler;

void PSMain(VSOut input, out GBuffer output){

	output.albedo = gDiffuseMap.Sample(gDiffuseSampler, input.uv);

	output.normal_shininess.xyz = input.normal_ws;

	output.normal_shininess.w = 25.0f;		// Dummy shininess
	
}