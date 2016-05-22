/// \brief Reduces an image via average operator.
/// Each pass reduces the image by N for each direction

#define N 16
#define TOTAL_THREADS (N * N)

Texture2D<float> gSource;			///< \brief Source image.
RWTexture2D<float> gReduced;		///< \brief Reduced image.

groupshared float samples[TOTAL_THREADS];

/// \remarks You should dispatch half the threads for each dimension!

[numthreads(SIZE, SIZE,1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID, uint group_index: SV_GroupIndex, uint3 group_id : SV_GroupID) {

	// Build the sample cache

	// Each thread samples a 2x2 region

	const uint2 sample_location = dispatch_thread_id * 2;

	samples[group_index] = gSource[sample_location + uint2(0,0)] +
						   gSource[sample_location + uint2(0,1)] +
						   gSource[sample_location + uint2(1,0)] +
						   gSource[sample_location + uint2(1,1)];

	GroupMemoryBarrierWithGroupSync();

	// Performs the reduction

	[unroll(TOTAL_THREADS)]
	for (uint offset = TOTAL_THREADS / 2; offset > 0; offset /= 2) {

		if (group_index < offset) {

			samples[group_index] += samples[group_index + offset];
			GroupMemoryBarrierWithGroupSync();

		}

	}

	// The last thread writes out the result

	if (group_thread_index == 0) {

		gReduced[group_id.xy] = dot(samples[0], 0.25f) / TOTAL_THREADS;

	}


}