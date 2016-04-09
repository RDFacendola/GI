
#include "voxel_def.hlsl"
#include "..\common\color.hlsl"

StructuredBuffer<VoxelInfo> gVoxelAppendBuffer;				// Append buffer containing the list of voxels in the current frame. (Read Only)

Texture3D<int> gVoxelSH;									// Contains SH infos

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
	float4 color : Color;

};

VSOut VSMain(VSIn input){

	VoxelInfo voxel_info = gVoxelAppendBuffer[input.instance_id];
	
	VSOut output;

	output.color = SampleVoxelColor(gVoxelSH, voxel_info, normalize(input.position.xyz));		// Sampled color of the SH

	// The debug draw is applied after the tonemap. Apply Reinhard here

	output.color.rgb = output.color.rgb * rcp(1.f + output.color.rgb);

	output.color.rgb = pow(output.color.rgb, 1.f / 2.2f);

	// Deformation of the SH mesh

	float magnitude = Luminance(output.color.rgb);
	
	float3 position = (input.position.xyz * voxel_info.size * magnitude) + voxel_info.center;
	
	output.position_ps = mul(gViewProjection, float4(position, 1));

	return output;

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float4 PSMain(VSOut input) : SV_Target0{

	return input.color;

}
