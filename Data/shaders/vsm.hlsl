#include "projection_def.hlsl"

struct VSIn {

	float4 position : SV_Position;				// Vertex position.

};

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

struct VSOut {

	float4 position_ps: SV_Position;			// Vertex position in light projection space.

};

cbuffer PerObject{

	float4x4 gWorldLightProj;					// World * Light-view * Light-proj matrix.

};

VSOut VSMain(VSIn input){

	VSOut output;

	output.position_ps = mul(gWorldLightProj, input.position);

	return output;

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float2 PSMain(VSOut input) : SV_Target0{

	return float2(input.position_ps.z,
				  input.position_ps.z * input.position_ps.z);

}
