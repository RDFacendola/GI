/// \brief Appends the voxel stored inside a voxel address table inside an append buffer.

#define N 256
#define TOTAL_THREADS (N)

struct VoxelInfo {

	float3 center;			// Center of the voxel, in world space
	float size;				// Size of the voxel in world units

};

RWBuffer<uint> gIndirectArguments;

StructuredBuffer<uint> gVoxelAddressTable;

AppendStructuredBuffer<VoxelInfo> gVoxelAppendBuffer;		//<T>: VoxelInfo

cbuffer Parameters {

	float3 gCenter;						// Center of the voxelization. It is always a corner shared among 8 different voxels.

	float gVoxelSize;					// Size of each voxel in world units for each dimension.

	uint gVoxelResolution;				// Resolution of each cascade in voxels for each dimension.

	uint gCascades;						// Number of additional cascades inside the clipmap.

};

void AppendVoxelInfo(uint linear_coordinates, uint cascade) {
		
	if (cascade != 0) {

		return;

	}

	// Update the instances count

	uint dummy;

	InterlockedAdd(gIndirectArguments[1], 1, dummy);	

	// Fill out the voxel info

	VoxelInfo voxel_info;

	voxel_info.size = gVoxelSize * (1 << cascade);

	// Voxel space

	voxel_info.center.x = linear_coordinates % gVoxelResolution;
	voxel_info.center.y = (linear_coordinates / gVoxelResolution) % gVoxelResolution;
	voxel_info.center.z = (linear_coordinates / (gVoxelResolution * gVoxelResolution)) % gVoxelResolution;

	voxel_info.center -= (gVoxelResolution >> 1);

	// Local space

	voxel_info.center = (voxel_info.center + 0.5f) * voxel_info.size;

	// World space
	
	voxel_info.center += gCenter;

	gVoxelAppendBuffer.Append(voxel_info);

}

[numthreads(N, 1, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	// If the address of the VAT is valid there's a voxel there!
		
	// Layout of the VAT
	// Pyramid + Cascade 0 | Cascade 1 | Cascade 2 | ...

	if (gVoxelAddressTable[dispatch_thread_id.x] != 0) {

		uint VAT_elements;
		uint dummy;

		gVoxelAddressTable.GetDimensions(VAT_elements, dummy);

		uint cascade_size = gVoxelResolution * gVoxelResolution * gVoxelResolution;		// Size of each cascade, in voxels

		uint pyramid_size = VAT_elements - cascade_size * (gCascades + 1);				// Size of the pyramid, without its last level. Elements inside the pyramid are always ignored.

		if (dispatch_thread_id.x >= pyramid_size) {

			uint linear_coordinate = dispatch_thread_id.x - pyramid_size;

			uint cascade_index = linear_coordinate / cascade_size;

			linear_coordinate %= cascade_size;

			AppendVoxelInfo(linear_coordinate, cascade_index);

		}

	}
	
}