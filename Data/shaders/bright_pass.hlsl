#include "quad.hlsl"
#include "color.hlsl"

cbuffer Parameters {

	float gThreshold;

};

Texture2D gSource;

SamplerState gSourceSampler;

float4 PSMain(VSOut input) : SV_Target0{

	// Performs a high-pass filter.

	float4 color = gSource.Sample(gSourceSampler, input.uv);

	float luminance = RelativeLuminance(color.rgb);

	return color * saturate(1.0f - gThreshold / luminance);		// Color whose luminance is equal to the threshold and below will cause a black output
		
}