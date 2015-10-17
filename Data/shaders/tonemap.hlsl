// Tonemapping used by UE4: GammaColor = LinearColor / (LinearColor + 0.187) * 1.035;


// Parameters
cbuffer TonemapParams{

	float gVignette;
	float gFactor;
	float gBias;

};

Texture2D gUnexposed;						// Input
RWTexture2D<float4> gExposed;				// Output

// Calculate the vignette factor.
// coordinates - Coordinates of the point
// size - Size of the surfaces
// vignette - Vignette factor
float Vignette(uint2 coordinates, uint2 size, float vignette){

	float2 uv_coordinates = coordinates;

	uv_coordinates /= size;		// Coordinates in the range [0;1]
	uv_coordinates -= 0.5f;		// Centered on the texture's center.
	
	float sqr_distance = dot(uv_coordinates, uv_coordinates);
	
	return pow(1.0f - sqr_distance, vignette);

}

// Calculate the exposed value of the image.
// unexposed - Unexposed color channel.
// factor - Multiplicative factor.
// bias - Bias factor.
float ExposeChannel(float unexposed, float factor, float bias) {

	return unexposed / ((unexposed + bias) * factor);

}

// Calculate the exposed value of the image.
// unexposed - Unexposed color channel.
// factor - Multiplicative factor.
// bias - Bias factor.
float4 Expose(float4 unexposed, float factor, float bias){

	return float4(ExposeChannel(unexposed.r, factor, bias),
				  ExposeChannel(unexposed.g, factor, bias),
				  ExposeChannel(unexposed.b, factor, bias),
				  1.0);

}

[numthreads(16,16,1)]
void CSMain(int3 thread_id : SV_DispatchThreadID){

	// Vignette

	uint2 dimensions;

	gUnexposed.GetDimensions(dimensions.x, dimensions.y);

	float vignette = Vignette(thread_id.xy, dimensions, gVignette);

	// Color grading - Unreal Engine 4 style

	float4 unexposed = gUnexposed[thread_id.xy];

	float4 exposed = Expose(unexposed, gFactor, gBias);

	// Output

	gExposed[thread_id.xy] = exposed * vignette;
		
}
