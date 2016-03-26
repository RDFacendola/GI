#include "projection_def.hlsl"

cbuffer Parameters {

	float3 gCenter;								// Center of the voxelization. It is always a corner shared among 8 different voxels.

	float gVoxelSize;							// Size of each voxel in world units for each dimension.

	unsigned int gVoxelResolution;				// Resolution of each cascade in voxels for each dimension.

	unsigned int gCascades;						// Number of additional cascades inside the clipmap.
			
};

struct VoxelInfo {

	float3 center;			// Center of the voxel, in world space
	float size;				// Size of the voxel in world units

};

StructuredBuffer<VoxelInfo> gVoxelAppendBuffer;		//<T>: VoxelInfo

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

	output.color = lerp(float4(0, 0, 0.75f, 1), float4(0.75f, 0, 0, 1), factor);

	return output;

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float4 PSMain(VSOut input) : SV_Target0{

	return input.color;

}
