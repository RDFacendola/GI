/// \brief Used to fill the missing infos of a MIP level by using the next one.
/// During light injection only the most detailed MIP levels are filled with useful informations,
/// this leaves each MIP with cube (gVoxelResolution\2)^3 empty on the center of that MIP.
/// Downscales the next MIP level to fill that area up to the base of the pyramid.


/// \remarks Difficult to implement as parallel reduction since each MIP in the stack (and the pyramid) have the same dimensions.

#include "voxel_def.hlsl"

#define N 8
#define TOTAL_THREADS (N * N * N)

cbuffer SHFilter {

	uint gDestinationCascade;						// Cascade of the destination surface where the data will be stored.
	
	uint gSourceCascade;							// Cascade of the source surface hwere the data will be read from.

	uint2 gReserved;								// Padding
	
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
void CSMain(uint3 thread_id : SV_DispatchThreadID) {

	int half_resolution = gVoxelResolution >> 1;
	int quarter_resolution = gVoxelResolution >> 2;

	uint3 dst_offset;
	uint3 src_offset;
	uint band_count;

	[unroll]
	for (int channel_index = 0; channel_index < 3; ++channel_index) {

		band_count = min(GetSHBandCount(gDestinationCascade),
						 GetSHBandCount(gSourceCascade));			// We cannot reconstruct bands that do not exists in both cascades of course.

		for (uint coefficient_index = 0; coefficient_index < band_count * band_count; ++coefficient_index) {

			// Sample everything and store in groupshared memory

			src_offset = uint3(coefficient_index, gSourceCascade, channel_index) * gVoxelResolution;

			samples[thread_id.x][thread_id.y][thread_id.z] = gSource[thread_id + src_offset];

			GroupMemoryBarrierWithGroupSync();

			// Store inside the destination surface

			if (thread_id.x % 2 == 0 &&
				thread_id.y % 2 == 0 &&
				thread_id.z % 2 == 0) {

				dst_offset = quarter_resolution + uint3(coefficient_index, gDestinationCascade, channel_index) * gVoxelResolution;
				
				gDestination[thread_id / 2 + dst_offset] =   samples[thread_id.x + 0][thread_id.y + 0][thread_id.z + 0]
														   + samples[thread_id.x + 0][thread_id.y + 0][thread_id.z + 1]
														   + samples[thread_id.x + 0][thread_id.y + 1][thread_id.z + 0]
														   + samples[thread_id.x + 0][thread_id.y + 1][thread_id.z + 1]
														   + samples[thread_id.x + 1][thread_id.y + 0][thread_id.z + 0]
														   + samples[thread_id.x + 1][thread_id.y + 0][thread_id.z + 1]
														   + samples[thread_id.x + 1][thread_id.y + 1][thread_id.z + 0]
														   + samples[thread_id.x + 1][thread_id.y + 1][thread_id.z + 1];

			}

			GroupMemoryBarrierWithGroupSync();

		}

	}
		
}