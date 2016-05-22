#include "common/quad.hlsl"

Texture2D gDownscaled;
Texture2D gUpscaled;

SamplerState gSampler;

float4 PSMain(VSOut input) : SV_Target0{

	return gDownscaled.Sample(gSampler, input.uv) +
		   gUpscaled.Sample(gSampler, input.uv);

}