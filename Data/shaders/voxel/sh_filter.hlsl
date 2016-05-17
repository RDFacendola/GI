/// \brief Used to fill the missing infos of a MIP level by using the next one.
/// During light injection only the most detailed MIP levels are filled with useful informations,
/// this leaves each MIP with cube (gVoxelResolution\2)^3 empty on the center of that MIP.
/// Downscales the next MIP level to fill that area up to the base of the pyramid.


/// \remarks Difficult to implement as parallel reduction since each MIP in the stack (and the pyramid) have the same dimensions.

#include "voxel_def.hlsl"

#define N 8
#define TOTAL_THREADS (N * N * N)

cbuffer SHFilter {

	uint gSrcVoxelResolution;						// Resolution of the source. The resolution of the destination is always half that resolution.

	uint gDstVoxelResolution;						// Resolution of the destination. Either the same dimension of the source, or half that.

	uint gDstOffset;								// Offset applied to the destination surface.

	uint gSrcCascade;								// Cascade of the source.								

	uint gDstCascade;								// Cascade of the destination.
		
};

#ifdef SH_STACK

RWTexture3D<int> gUnfilteredSHStack;				// Unfiltered SH stack texture. Source \ Destination.

#define gSource gUnfilteredSHStack
#define gDestination gUnfilteredSHStack

#endif

#ifdef SH_PYRAMID

RWTexture3D<int> gUnfilteredSHPyramid;				// Unfiltered SH pyramid texture. Destination.

Texture3D<int> gUnfilteredSHStack;					// Unfiltered SH pyramid texture. Source.

#define gSource gUnfilteredSHStack
#define gDestination gUnfilteredSHPyramid

#endif

groupshared int samples[N][N][N];					// Store the samples of the source texture.

[numthreads(N, N, N)]
void CSMain(uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID) {

	uint3 dst_offset;
	uint3 src_offset;
	uint band_count;

	[unroll]
	for (int channel_index = 0; channel_index < 3; ++channel_index) {

		// We cannot reconstruct bands that do not exists in both cascades of course.
		band_count = min(GetSHBandCount(gDstCascade), GetSHBandCount(gSrcCascade));

		for (uint coefficient_index = 0; coefficient_index < band_count * band_count; ++coefficient_index) {

			// Sample everything and store in groupshared memory

			src_offset = uint3(coefficient_index, gSrcCascade, channel_index) * gSrcVoxelResolution;

			samples[group_thread_id.x][group_thread_id.y][group_thread_id.z] = gSource[thread_id + src_offset];

			GroupMemoryBarrierWithGroupSync();

			// Store inside the destination surface

			if (group_thread_id.x % 2 == 0 &&
				group_thread_id.y % 2 == 0 &&
				group_thread_id.z % 2 == 0) {

				dst_offset = gDstOffset + uint3(coefficient_index, gDstCascade, channel_index) * gDstVoxelResolution;

				gDestination[thread_id / 2 + dst_offset] =   samples[group_thread_id.x + 0][group_thread_id.y + 0][group_thread_id.z + 0]
														   + samples[group_thread_id.x + 0][group_thread_id.y + 0][group_thread_id.z + 1]
														   + samples[group_thread_id.x + 0][group_thread_id.y + 1][group_thread_id.z + 0]
														   + samples[group_thread_id.x + 0][group_thread_id.y + 1][group_thread_id.z + 1]
														   + samples[group_thread_id.x + 1][group_thread_id.y + 0][group_thread_id.z + 0]
														   + samples[group_thread_id.x + 1][group_thread_id.y + 0][group_thread_id.z + 1]
														   + samples[group_thread_id.x + 1][group_thread_id.y + 1][group_thread_id.z + 0]
														   + samples[group_thread_id.x + 1][group_thread_id.y + 1][group_thread_id.z + 1];
				;

			}

			GroupMemoryBarrierWithGroupSync();

		}

	}
		
}