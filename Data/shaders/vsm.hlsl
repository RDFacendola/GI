#include "projection_def.hlsl"

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

struct VSIn {

	float4 position : SV_Position;				// Vertex position.

};

struct VSOut {

	float4 position_ps: SV_Position;
	float depth : TEXCOORD;

};

cbuffer PerObject{

	float4x4 gWorldLight;						// World * Light-view matrix.

};

cbuffer PerLight {

	float gNearPlane;							// Near clipping plane

	float gFarPlane;							// Far clipping plane

	float gFrontFactor;							// Used to recycle the shader for the rear paraboloid.

};

void VSMain(VSIn input, out VSOut output){

	float4 position_ls = mul(gWorldLight, input.position);

	position_ls.z *= gFrontFactor;

	output.position_ps = ProjectToParaboloidSpace(position_ls.xyz,			// The projection guarantees that no geometry is clipped at this point, even the one behind!
												  gNearPlane,
												  gFarPlane);

	output.depth = position_ls.z;											// Clip at pixel level

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float2 PSMain(VSOut input) : SV_Target0{

	clip(input.depth);

	return float2(input.position_ps.z,
				  input.position_ps.z * input.position_ps.z);

}
