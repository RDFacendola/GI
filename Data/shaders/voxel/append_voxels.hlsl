/// \brief Appends the voxel stored inside a voxel address table inside an append buffer.

#include "voxel_def.hlsl"

#define N 256
#define TOTAL_THREADS (N)

RWBuffer<uint> gVoxelIndirectArguments;
RWBuffer<uint> gSHIndirectArguments;

StructuredBuffer<uint> gVoxelAddressTable;					// Contains the "pointers" to the actual voxel infos.

AppendStructuredBuffer<VoxelInfo> gVoxelAppendBuffer;		// Append buffer containing the list of voxels in the current frame. (Read/Write)

cbuffer PerFrame {

    float4x4 gViewProjection;				            	// View-projection matrix

};

[numthreads(N, 1, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	VoxelInfo voxel_info;

	uint dummy;

	if (GetVoxelInfo(gVoxelAddressTable, dispatch_thread_id.x, voxel_info) &&
		voxel_info.cascade >= 0) {

		float3 center = abs(voxel_info.center - gCenter);
		
		// Append only if the voxel is the most precise available

		if(voxel_info.cascade == (int)gCascades ||
		   max(center.x, max(center.y, center.z)) > (gVoxelResolution >> (voxel_info.cascade + 2)) * gVoxelSize){
			
			// Reject if the center of the voxel is not visible from the current camera

            float4 transformed_center = mul(gViewProjection, float4(voxel_info.center, 1));

            if (abs(transformed_center.x) < transformed_center.w &&
                abs(transformed_center.y) < transformed_center.w &&
                transformed_center.z < transformed_center.w &&
                transformed_center.z > 0){

			    InterlockedAdd(gVoxelIndirectArguments[1], 1, dummy);
			    InterlockedAdd(gSHIndirectArguments[1], 1, dummy);

			    gVoxelAppendBuffer.Append(voxel_info);

            }

		}
		
	}
	
}