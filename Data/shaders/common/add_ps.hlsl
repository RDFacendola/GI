#include "quad.hlsl"

Texture2D gOperand1;
Texture2D gOperand2;

SamplerState gSampler;

float4 PSMain(VSOut input) : SV_Target0{

	return gOperand1.Sample(gSampler, input.uv) +
		   gOperand2.Sample(gSampler, input.uv);

}