// Tonemapping used by UE4: GammaColor = LinearColor / (LinearColor + 0.187) * 1.035;


// Parameters
cbuffer TonemapParams{

	float gVignette;
	float gExposureMul;
	float gExposureAdd;

};

RWTexture2D<float4> gUnexposed;			// Input - Light accumulation buffer
RWTexture2D<float4> gExposed;				// Output - Processed result

// Calculate the vignette factor.
// coordinates - Coordinates of the point
// size - Size of the surfaces
// vignette - Vignette factor
float Vignette(uint2 coordinates, uint2 size, float vignette){

	coordinates -= size;

	return pow(1.0 - dot(coordinates, coordinates), vignette);

}

// Calculate the exposed value of the image.
// unexposed - Unexposed color channel.
// exposure_mul - Multiplicative scaling factor of the exposure.
// exposure_add - Additive scaling factor of the exposure.
float ExposeChannel(float unexposed, float exposure_mul, float exposure_add) {

	return unexposed / ((unexposed + exposure_add) * exposure_mul);

}

// Calculate the exposed value of the image.
// unexposed - Unexposed color.
// exposure_mul - Multiplicative scaling factor of the exposure.
// exposure_add - Additive scaling factor of the exposure.
float4 Expose(float4 unexposed, float exposure_mul, float exposure_add){

	return float4(ExposeChannel(unexposed.r, exposure_mul, exposure_add),
				  ExposeChannel(unexposed.g, exposure_mul, exposure_add),
				  ExposeChannel(unexposed.b, exposure_mul, exposure_add),
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

	float4 exposed = Expose(unexposed, gExposureMul, gExposureAdd);

	// Output

	gExposed[thread_id.xy] = exposed;
		
}
