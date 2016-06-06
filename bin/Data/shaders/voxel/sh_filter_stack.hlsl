/// \brief Used to fill the missing infos of a MIP level by using the next one.
/// During light injection only the most detailed MIP levels are filled with useful informations,
/// this leaves each MIP with cube (gVoxelResolution\2)^3 empty on the center of that MIP.
/// Downscales the next MIP level to fill that area up to the base of the pyramid.

/// Stack version:
/// The destination is always downscaled by a factor of 2
/// Offset is provided to center the destination accordingly, since different MIP levels may not have the same center.

#include "voxel_def.hlsl"

#define N 8
#define TOTAL_THREADS (N * N * N)

cbuffer SHFilterStack {

	uint3 gDstOffset;								// Offset within the destination cascade.

	uint gSrcCascade;								// Index of the source cascade. Greater than 1.
	
};

RWTexture3D<int> gUnfilteredSHStack;				// Unfiltered SH stack texture. Both source and destination.

groupshared int samples[N][N][N];					// Store the samples of the source texture.

[numthreads(N, N, N)]
void CSMain(uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID) {

	//return;

	uint3 dst_offset;
	uint3 src_offset;
	uint band_count;

	[unroll]
	for (int channel_index = 0; channel_index < 3; ++channel_index) {

		// We cannot reconstruct bands that do not exists in both cascades of course.
		band_count = min(GetSHBandCount(gSrcCascade - 1), GetSHBandCount(gSrcCascade));

		for (uint coefficient_index = 0; coefficient_index < band_count * band_count; ++coefficient_index) {

			// Sample everything and store in groupshared memory

			src_offset = uint3(coefficient_index, gSrcCascade, channel_index) * gVoxelResolution;

			samples[group_thread_id.x][group_thread_id.y][group_thread_id.z] = gUnfilteredSHStack[thread_id + src_offset];

			GroupMemoryBarrierWithGroupSync();

			// Store inside the destination surface

			if (group_thread_id.x % 2 == 0 &&
				group_thread_id.y % 2 == 0 &&
				group_thread_id.z % 2 == 0) {

				dst_offset = gDstOffset + uint3(coefficient_index, gSrcCascade - 1, channel_index) * gVoxelResolution;

				gUnfilteredSHStack[thread_id / 2 + dst_offset] =   samples[group_thread_id.x + 0][group_thread_id.y + 0][group_thread_id.z + 0]
																 + samples[group_thread_id.x + 0][group_thread_id.y + 0][group_thread_id.z + 1]
																 + samples[group_thread_id.x + 0][group_thread_id.y + 1][group_thread_id.z + 0]
																 + samples[group_thread_id.x + 0][group_thread_id.y + 1][group_thread_id.z + 1]
																 + samples[group_thread_id.x + 1][group_thread_id.y + 0][group_thread_id.z + 0]
																 + samples[group_thread_id.x + 1][group_thread_id.y + 0][group_thread_id.z + 1]
																 + samples[group_thread_id.x + 1][group_thread_id.y + 1][group_thread_id.z + 0]
																 + samples[group_thread_id.x + 1][group_thread_id.y + 1][group_thread_id.z + 1];
				
			}

			GroupMemoryBarrierWithGroupSync();

		}

	}
		
}