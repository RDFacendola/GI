#include "common/quad.hlsl"

Texture2D gBase;
Texture2D gBloom;

cbuffer Parameters {

	float gBloomStrength;

};

SamplerState gSampler;

float4 PSMain(VSOut input) : SV_Target0{

	return gBase.Sample(gSampler, input.uv) + gBloom.Sample(gSampler, input.uv) * gBloomStrength;

}