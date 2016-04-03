/// \brief Appends the voxel stored inside a voxel address table inside an append buffer.

#include "voxel_def.hlsl"

#define N 256
#define TOTAL_THREADS (N)

RWBuffer<uint> gVoxelIndirectArguments;
RWBuffer<uint> gSHIndirectArguments;

StructuredBuffer<uint> gVoxelAddressTable;

AppendStructuredBuffer<VoxelInfo> gVoxelAppendBuffer;		// Append buffer containing the list of voxels in the current frame. (Read/Write)

float max3(float a, float b, float c) {

	return max(max(a, b), c);

}

void AppendVoxelInfo(uint linear_coordinates, uint cascade) {
		
	// Fill out the voxel info

	VoxelInfo voxel_info;

	// Voxel space

	voxel_info.center.x = linear_coordinates % gVoxelResolution;
	voxel_info.center.y = (linear_coordinates / gVoxelResolution) % gVoxelResolution;
	voxel_info.center.z = (linear_coordinates / (gVoxelResolution * gVoxelResolution)) % gVoxelResolution;

	voxel_info.center -= (gVoxelResolution >> 1);

	// Suppress the voxel if there's a more precise version of it

	if (cascade < gCascades &&
		(uint)(max3(abs(voxel_info.center.x), abs(voxel_info.center.y), abs(voxel_info.center.z))) <= (gVoxelResolution >> 2)){

		return;

	}

	// Local space

	voxel_info.size = gVoxelSize / (1 << cascade);

	voxel_info.center = (voxel_info.center + 0.5f) * voxel_info.size;

	// World space
	
	voxel_info.center += gCenter;

	// Update the instances count

	uint dummy;

	InterlockedAdd(gVoxelIndirectArguments[1], 1, dummy);
	InterlockedAdd(gSHIndirectArguments[1], 1, dummy);

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