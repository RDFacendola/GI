//////////////////////////////////// VERTEX SHADER ////////////////////////////////////////

#include "quad.hlsl"

////////////////////////////////// PIXEL SHADER /////////////////////////////////////////

cbuffer PerFrame{

	float gExposure;
	float gVignette;

}

struct PSOut{

	float4 color : SV_Target;

};

Texture2D gHDR;

SamplerState gHDRSampler;

float3 Expose(float3 unexposed, float vignette){

	return saturate(1.0 - pow(2.718, -unexposed * gExposure * vignette));

}

void PSMain(VSOut input, out PSOut output){

	float2 texcoord = float2(input.uv - 0.5);

	float vignette = pow( 1.0 - dot(texcoord, texcoord), gVignette );

	float4 unexposed = gHDR.Sample(gHDRSampler, input.uv);

	output.color.rgb = Expose(unexposed.rgb, vignette);
	output.color.a = 1.0;

}
