/// \brief Used to fill the missing infos of a MIP level by using the next one.
/// During light injection only the most detailed MIP levels are filled with useful informations,
/// this leaves each MIP with cube (gVoxelResolution\2)^3 empty on the center of that MIP.
/// Downscales the next MIP level to fill that area up to the base of the pyramid.

#define N 8
#define TOTAL_THREADS (N * N * N)

cbuffer SHFilter {

	int3 gSrcOffset;					// Offset of the surface used as source of the filtering.
	int gSrcStride;						// Horizontal stride for each source SH band coefficient.

	int3 gDstOffset;					// Offset of the surface used as destination of the filtering.	
	int gDstStride;						// Horizontal stride for each destination SH band coefficient.

	int3 gMIPOffset;					// Offset to apply within a single filtered MIP.
	int padding;						
		
};

RWTexture3D<int> gSHRed;				// Red spherical harmonics contribution to filter.
RWTexture3D<int> gSHGreen;				// Green spherical harmonics contribution to filter.
RWTexture3D<int> gSHBlue;				// Blue spherical harmonics contribution to filter.

groupshared int samples[N][N][N];		// Store the samples of the source texture.

void Filter(RWTexture3D<int> surface, int3 thread, int3 group_thread) {

	// Sample everything and store in group shared memory

	samples[group_thread.x][group_thread.y][group_thread.z] = surface[thread + gSrcOffset];

	GroupMemoryBarrierWithGroupSync();

	// Filter (downscale)

	if (group_thread.x % 2 == 0 &&
		group_thread.y % 2 == 0 &&
		group_thread.z % 2 == 0) {

		int sh_coefficient_index = thread.x / gSrcStride;

		thread.x %= gSrcStride;
		
		int3 dst_offset = gDstOffset + gMIPOffset;
		
		dst_offset.x += gDstStride * sh_coefficient_index;

		surface[thread / 2 + dst_offset] =   samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 0]
										   + samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 1]
										   + samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 0]
										   + samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 1]
										   + samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 0]
										   + samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 1]
										   + samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 0]
										   + samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 1];

	}

}

[numthreads(N, N, N)]
void CSMain(uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID) {

	// Red
	Filter(gSHRed, thread_id, group_thread_id);

	GroupMemoryBarrierWithGroupSync();

	// Green
	Filter(gSHGreen, thread_id, group_thread_id);

	GroupMemoryBarrierWithGroupSync();

	// Blue
	Filter(gSHBlue, thread_id, group_thread_id);
		
}