#include "quad.hlsl"

cbuffer Parameters {

	float gThreshold;

};

Texture2D gSource;

SamplerState gSourceSampler;

float Brightness(float3 color) {

	// http://www.w3.org/TR/AERT#color-contrast - ((Red value X 299) + (Green value X 587) + (Blue value X 114)) / 1000

	//const float3 conversion_factor = float3(0.299f, 0.587f, 0.144f);
	
	// https://en.wikipedia.org/wiki/Relative_luminance

	const float3 conversion_factor = float3(0.2126f, 0.7152f, 0.0722f);

	return dot(color.rgb, conversion_factor);

}

float4 PSMain(VSOut input) : SV_Target0{

	// Performs a high-pass filter.

	float4 color = gSource.Sample(gSourceSampler, input.uv);

	if (Brightness(color.rgb) > gThreshold) {

		return color;

	}
	else {

		return float4(0, 0, 0, 0);
	}

}