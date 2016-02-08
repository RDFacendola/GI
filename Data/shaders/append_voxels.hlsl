/// \brief Appends the voxel stored inside a voxel address table inside an append buffer.

#define N 256
#define TOTAL_THREADS (N)

struct VoxelInfo {

	float3 center;			// Center of the voxel, in world space
	float size;				// Size of the voxel in world units

};

StructuredBuffer<uint> gVoxelAddressTable;
RWBuffer<uint> gIndirectArguments;
AppendStructuredBuffer<VoxelInfo> gVoxelAppendBuffer;		//<T>: VoxelInfo

cbuffer Parameters {

	float3 gCenter;						// Center of the voxelization. It is always a corner shared among 8 different voxels.

	float gVoxelSize;					// Size of each voxel in world units for each dimension.

	uint gVoxelResolution;				// Resolution of each cascade in voxels for each dimension.

	uint gCascades;						// Number of additional cascades inside the clipmap.

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

	VoxelInfo voxel_info;

	static const uint gSize = 8;

	if (dispatch_thread_id.x < (gSize * gSize * gSize)) {

		uint dummy;

		InterlockedAdd(gIndirectArguments[1], 1, dummy);	// Update the instances count

		voxel_info.size = gVoxelSize;

		voxel_info.center.x = dispatch_thread_id.x % gSize;
		voxel_info.center.y = (dispatch_thread_id.x / gSize) % gSize;
		voxel_info.center.z = (dispatch_thread_id.x / (gSize * gSize)) % gSize;

		voxel_info.center -= 8;

		voxel_info.center = (voxel_info.center + 0.5f) * voxel_info.size;

		voxel_info.center += gCenter;

		gVoxelAppendBuffer.Append(voxel_info);
		
	}
	

}


/*


// whatever

if (gVoxelAddressTable[dispatch_thread_id.x] != 0){

uint dummy;

InterlockedAdd(gIndirectArguments[1], 1, dummy);	// Update the instances count

// Append the voxel info to the append buffer

// Layout of the VAT
// Pyramid | Cascade 0 | Cascade 1 | Cascade 2 | ...

uint vat_elements;

gVoxelAddressTable.GetDimensions(vat_elements, dummy);

uint cascade_size = gVoxelResolution * gVoxelResolution * gVoxelResolution;		// Size of each cascade, in voxels

uint pyramid_size = vat_elements - cascade_size * gCascades;

if (dispatch_thread_id.x >= pyramid_size - cascade_size) {

uint linear_coord = dispatch_thread_id.x - pyramid_size + cascade_size;

uint cascade_index = (uint) floor(linear_coord / cascade_size);

linear_coord -= cascade_index * cascade_size;

voxel_info.size = gVoxelSize * (1 << cascade_index);

voxel_info.center = 0;

voxel_info.center.x = linear_coord % gVoxelResolution;
voxel_info.center.y = (linear_coord / gVoxelResolution) % gVoxelResolution;
voxel_info.center.z = (linear_coord / (gVoxelResolution * gVoxelResolution)) % gVoxelResolution;

voxel_info.center -= gVoxelResolution / 2.0f;

voxel_info.center = (voxel_info.center + 0.5f) * voxel_info.size;

voxel_info.center += gCenter;

if (cascade_index == 0) {

gVoxelAppendBuffer.Append(voxel_info);

}

}

}

*/