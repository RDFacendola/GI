
#include "voxel_def.hlsl"
#include "..\common\color.hlsl"

StructuredBuffer<VoxelInfo> gVoxelAppendBuffer;				// Append buffer containing the list of voxels in the current frame. (Read Only)

Texture3D<float3> gFilteredSHPyramid;						// Pyramid part of the filtered SH 3D clipmap.
Texture3D<float3> gFilteredSHStack;							// Stack part of the filtered SH 3D clipmap.

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

cbuffer PerFrame {

	float4x4 gViewProjection;					// View-projection matrix

};

struct VSIn {

	float3 position : SV_Position;
	uint instance_id : SV_InstanceID;

};

struct VSOut {

	float4 position_ps : SV_Position;
	float3 color : Color;

};

VSOut VSMain(VSIn input){

	VoxelInfo voxel_info = gVoxelAppendBuffer[input.instance_id];
	
	VSOut output;

	output.color = SampleVoxelColor(gFilteredSHPyramid, 
									gFilteredSHStack, 
									voxel_info.center, 
									normalize(input.position.xyz));

	// The debug draw is applied after the tonemap so we have to manually tonemap the result. (Reinhard's)

	output.color.rgb = output.color.rgb * rcp(1.f + output.color.rgb);

	output.color.rgb = pow(output.color.rgb, 1.f / 2.2f);

	// Deformation of the SH mesh

	float magnitude = (max(output.color.r, max(output.color.g, output.color.b)) + min(output.color.r, min(output.color.g, output.color.b))) * 0.5f;
	
	float3 position = (input.position.xyz * voxel_info.size * magnitude) + voxel_info.center;
	
	output.position_ps = mul(gViewProjection, float4(position, 1));
	
	return output;

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float4 PSMain(VSOut input) : SV_Target0{

	return float4(input.color, 1.f);

}
