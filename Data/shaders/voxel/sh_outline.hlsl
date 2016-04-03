
#include "voxel_def.hlsl"

StructuredBuffer<VoxelInfo> gVoxelAppendBuffer;				// Append buffer containing the list of voxels in the current frame. (Read Only)

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
	
	VoxelInfo info = gVoxelAppendBuffer[input.instance_id];
	
	VSOut output;

	float3 position = (input.position.xyz * info.size) + info.center;

	output.position_ps = mul(gViewProjection, float4(position, 1));

	float factor = log2(gVoxelSize / info.size) / (gCascades + 1);

	output.color = lerp(info.red_sh01, info.green_sh01, factor);

	return output;

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float4 PSMain(VSOut input) : SV_Target0{

	return float4(input.color.rgb, 0.75f);

}
