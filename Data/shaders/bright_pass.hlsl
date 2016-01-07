#include "common/quad.hlsl"
#include "common/color.hlsl"

cbuffer Parameters {

	float gThreshold;					///< \brief Exposure threshold below of which the color is killed.
	float gKeyValue;
	float gAverageLuminance;			///< \brief Average luminance of the scene.
	
	float reserved;

};

Texture2D gSource;

SamplerState gSourceSampler;

float4 PSMain(VSOut input) : SV_Target0{

	//Performs a bright-pass filter.

	float4 color = gSource.Sample(gSourceSampler, input.uv);

	float3 exposed_color = Expose(color.rgb, gAverageLuminance, gKeyValue, gThreshold);

	return dot(exposed_color, 0.25f) < 0.001f ?
		   0.f :
		   float4(exposed_color, color.a);
			
}