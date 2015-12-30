
// Parameters
cbuffer TonemapParams{

	float gVignette;			// Vignette factor.
	float gKeyValue;			// 'Mood' of the final image.
	float gAvgLuminance;		// Average luminance of the image. Linear.

	float reserved;

};

Texture2D gUnexposed;						// Input
RWTexture2D<float4> gExposed;				// Output

/// \brief Performs an exposure adjustment of a given color.
/// \param unexposed_color The color to adjust.
float3 Expose(float3 unexposed_color, float average_luminance, float key_value, float threshold) {

	average_luminance = max(average_luminance, 0.001f);

	float exposure = key_value / average_luminance;						// Linear exposure

	exposure = log2(max(exposure, 0.0001f)) - threshold;
	
	return exp2(exposure) * unexposed_color;

}

/// \brief Helper function used by the Uncharted 2 tonemapping procedure.
float3 UnchartedTonemap(float3 linear_color) {

	static float A = 0.15;
	static float B = 0.50;
	static float C = 0.10;
	static float D = 0.20;
	static float E = 0.02;
	static float F = 0.30;

	return ((linear_color*(A*linear_color + C*B) + D*E) / (linear_color*(A*linear_color + B) + D*F)) - E / F;

}

/// \brief Calculate a tonemapped color.
/// \param linear_color Linear color to map.
float3 Tonemap(float3 linear_color){

// Enable one of the following:

//#define UE4_TONEMAP
//#define GAMMA
//#define REINHARD
#define HEJL_BURGESS_DAWSON
//#define UNCHARTED2

#ifdef UE4_TONEMAP

	// Unreal Engine 4 tonemapping (without LuTs)

	return linear_color * rcp((linear_color + 0.187f) * 1.035f);

#endif

#ifdef GAMMA

	// Simple gamma adjustment

	return pow(linear_color, 1.f / 2.2f);

#endif

#ifdef REINHARD

	// Reinhard's

	linear_color = linear_color * rcp(1.f + linear_color);

	return pow(linear_color, 1.f / 2.2f);

#endif

#ifdef HEJL_BURGESS_DAWSON

	// Jim Hejl and Richard Burgess-Dawson's

	linear_color = max(0, linear_color - 0.004f);
	
	linear_color = (linear_color * (6.2f * linear_color + 0.5f)) / (linear_color*(6.2f*linear_color + 1.7f) + 0.06f);

	return pow(linear_color, 2.2f);
	
#endif

#ifdef UNCHARTED2
	
	// Uncharted 2's
	
	static float kExposureBias = 2.0f;
	static float W = 11.2f;

	linear_color = UnchartedTonemap(linear_color * kExposureBias);

	linear_color *= rcp(UnchartedTonemap(W));

	return pow(linear_color, 1.f / 2.2f);

#endif

}

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

[numthreads(16,16,1)]
void CSMain(int3 thread_id : SV_DispatchThreadID){

	// Unexposed color

	float3 color = gUnexposed[thread_id.xy].xyz;

	// Exposure adjustment

	color = Expose(color, gAvgLuminance, gKeyValue, 0.0f);

	// Tone mapping

	color = Tonemap(color);

	// Vignette

	uint2 dimensions;

	gUnexposed.GetDimensions(dimensions.x, dimensions.y);

	float vignette = Vignette(thread_id.xy, dimensions, gVignette);

	// Output

	gExposed[thread_id.xy] = float4(color * vignette, 1);
		
}
