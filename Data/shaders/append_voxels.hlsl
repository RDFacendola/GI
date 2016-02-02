/// \brief Appends the voxel stored inside a voxel address table inside an append buffer.

#define N 256
#define TOTAL_THREADS (N)

struct VoxelInfo {

	float3 center;			// Center of the voxel
	float half_size;		// Half size of the voxel in world units

};

StructuredBuffer<uint> gVoxelAddressTable;
RWBuffer<uint> gIndirectArguments;
AppendStructuredBuffer<VoxelInfo> gVoxelAppendBuffer;		//<T>: VoxelInfo

cbuffer Parameters {

	float3 gCenter;								// Center of the voxelization. It is always a corner shared among 8 different voxels.

	float gVoxelSize;							// Size of each voxel in world units for each dimension.

	unsigned int gVoxelResolution;				// Resolution of each cascade in voxels for each dimension.

	unsigned int gCascades;						// Number of additional cascades inside the clipmap.

};

[numthreads(N, 1, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	// The very first thread will initialize the indirect arguments

	if (dispatch_thread_id.x == 0){

		// DrawIndexedInstanced call

		gIndirectArguments[0] = 36;				// IndexCountPerInstance
		gIndirectArguments[1] = 0;				// InstanceCount***
		gIndirectArguments[2] = 0;				// StartIndexLocation
		gIndirectArguments[3] = 0;				// BaseVertexLocation
		gIndirectArguments[4] = 0;				// StartInstanceLocation

	}

	GroupMemoryBarrier();

	// If the address of the VAT is valid there's a voxel there!

	if (gVoxelAddressTable[dispatch_thread_id.x] != 0){

		uint dummy;

		InterlockedAdd(gIndirectArguments[1], 1, dummy);	// Update the instances count

		// Append the voxel info to the append buffer

		VoxelInfo voxel_info;

		voxel_info.center = dispatch_thread_id;
		voxel_info.half_size = 10.0f;

		gVoxelAppendBuffer.Append(voxel_info);
		
	}

}