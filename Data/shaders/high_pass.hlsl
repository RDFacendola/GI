#include "quad.h"

cbuffer Parameters {

	float gThreshold;

};

Texture2D gSource;

SamplerState gSourceSampler;

float Brightness(float3 color) {

	// http://www.w3.org/TR/AERT#color-contrast - ((Red value X 299) + (Green value X 587) + (Blue value X 114)) / 1000
	
	return color.r * 0.299 +
		   color.g * 0.587 +
		   color.b * 0.144;

}

float4 PSMain(VSOut input) : SV_Target0{

	// Performs a high-pass filter.

	auto color = gSource.Sample(gSourceSampler, input.uv);

	if (Brightness(input.rgb) > gThreshold) {

		return color;

	}
	else {

		return float4(0, 0, 0, 0);
	}

}