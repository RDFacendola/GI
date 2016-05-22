/// \brief Computes the histogram of the log luminance

#include "common/color.hlsl"

#define N 16
#define TOTAL_THREADS (N * N)

Texture2D<float> gSource;						///< \brief Source image.
RWStructuredBuffer<uint> gHistogram;			///< \brief buffer containing the result of the histogram.

cbuffer Parameters {

	float gLogMinimum;							///< \brief Minimum log luminance.
	float gLogMaximum;							///< \brief Maximum log luminance.

	float2 reserved;

};

[numthreads(N, N,1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	uint bins;
	uint stride;

	gHistogram.GetDimensions(bins, stride);

	uint width, height;
	float dummy;

	gSource.GetDimensions(0, width, height, dummy);

	if (dispatch_thread_id.x < width && dispatch_thread_id.y < height) {

		// Simple log luminance of the current pixel

		float luminance = log(Luminance(gSource[dispatch_thread_id.xy]));
		
		// Discard any sample that falls outside the histogram boundaries

		uint bin_index = min(floor((luminance - gLogMinimum) / (gLogMaximum - gLogMinimum) * bins), bins - 1);

		InterlockedAdd(gHistogram[bin_index], 1);


	}

	// TODO: optimize using groupshared histograms and merge the result at the end

}