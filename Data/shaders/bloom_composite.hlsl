#include "quad.h"

Texture2D gSource;
Texture2D gGlow;

SamplerState gSourceSampler;

float4 PSMain(VSOut input) : SV_Target0{

	return gSource.Sample(gSourceSampler, input.uv) +
		   gGlow.Sample(gSourceSampler, input.uv);

}