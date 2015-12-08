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

	// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html

	float2 moments;

	float dx = ddx(input.position_ps.z);
	float dy = ddy(input.position_ps.z);

	return float2(input.position_ps.z,
				  input.position_ps.z * input.position_ps.z + 0.25f * (dx * dx + dy * dy));

}
