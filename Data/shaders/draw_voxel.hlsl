#include "common/hlsl_def.hlsl"
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

float4 VSMain(VSIn input) : SV_Position{
	
	VoxelInfo info = gVoxelAppendBuffer[input.instance_id];

	return mul(gViewProjection, float4((input.position.xyz * info.size) + info.center, 1));
	
}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float4 PSMain(float4 input: SV_Position) : SV_Target0{

	float grid_size = (gVoxelSize * gVoxelResolution) * (1 << gCascades);

	return float4(1,0,0,0);		// Plain red voxels

}
