// Convolution of an image with a given 1D kernel

// Those macros should be defined via code

// #define RADIUS 5		// Radius of the convolution kernel
// #define HORIZONTAL	// Horizontal convolution <or>
// #define VERTICAL		// Vertical convolution

#define N 256
#define SAMPLE_SIZE (256 + 2 * RADIUS)

cbuffer Parameters {

	int2 destination_offset;	// Top left corner of the destination buffer.

};

StructuredBuffer<float> gKernel;				// Blurring kernel

Texture2D gSource;								// Source image
RWTexture2D<float4> gDestination;				// Destination image

//SamplerState gSourceSampler;					// Sampler used to sample the source texture.

groupshared float4 samples[SAMPLE_SIZE];		// Stores texture samples

// Performs the 1D convolution of the image centered at the specified element.
float4 Convolute(uint element_index) {

	float4 value = 0;

	[flatten]
	for (int index = -RADIUS; index <= RADIUS; ++index) {

		value += gKernel[index + RADIUS] * samples[element_index + index + RADIUS];

	}

	return value;

}

#ifdef HORIZONTAL

[numthreads(N,1,1)]

#endif

#ifdef VERTICAL

[numthreads(1,N,1)]

#endif

void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID){

// Store the samples inside the "cache". Out-of-bounds threads will be clamped to the nearest pixel.
	
#ifdef VERTICAL

	if (group_thread_id.y < RADIUS) {

		samples[group_thread_id.y] = gSource[int2(dispatch_thread_id.x,
											 max(dispatch_thread_id.y - RADIUS, 0))];

	}
	else if (group_thread_id.y >= N - RADIUS) {

		samples[group_thread_id.y + 2 * RADIUS] = gSource[int2(dispatch_thread_id.x,
															   min(dispatch_thread_id.y + RADIUS, gSource.Length.y - 1))];

	}

	samples[group_thread_id.y + RADIUS] = gSource[min(dispatch_thread_id.xy, gSource.Length.xy - 1)];

#endif

#ifdef HORIZONTAL

	if (group_thread_id.x < RADIUS) {

		samples[group_thread_id.x] = gSource[int2(max(dispatch_thread_id.x - RADIUS, 0),
												  dispatch_thread_id.y)];

	}
	else if (group_thread_id.x >= N - RADIUS) {

		samples[group_thread_id.x + 2 * RADIUS] = gSource[int2(min(dispatch_thread_id.x + RADIUS, gSource.Length.x - 1),
															   dispatch_thread_id.y)];

	}

	samples[group_thread_id.x + RADIUS] = gSource[min(dispatch_thread_id.xy, gSource.Length.xy - 1)];

#endif

	// Sync

	GroupMemoryBarrierWithGroupSync();
	
	// Convolution

#ifdef VERTICAL

	gDestination[dispatch_thread_id.xy + destination_offset] = Convolute(group_thread_id.y);

#endif

#ifdef HORIZONTAL

	gDestination[dispatch_thread_id.xy + destination_offset] = Convolute(group_thread_id.x);

#endif

}
