#include "quad.hlsl"

Texture2D gSource;
SamplerState gSampler;

float4 PSMain(VSOut input) : SV_Target0{

	// Simple pixel shader, take the input and renders it to the output. Useful for upscaling and downscaling.

	return gSource.Sample(gSampler, input.uv);

}