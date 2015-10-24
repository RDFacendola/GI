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

	if (RelativeLuminance(color.rgb) > gThreshold) {

		return color;

	}
	else {

		return float4(0, 0, 0, 0);
	}

}