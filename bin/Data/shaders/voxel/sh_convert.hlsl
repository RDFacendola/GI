/// \brief Used to convert the 3x int-encoded SH structure to a RGB float3-encoded one

#define N 8
#define TOTAL_THREADS (N * N * N)

Texture3D<int> gSHRed;				// Unfiltered red spherical harmonics contribution.
Texture3D<int> gSHGreen;			// Unfiltered green spherical harmonics contribution.
Texture3D<int> gSHBlue;				// Unfiltered blue spherical harmonics contribution.

RWTexture3D<float4> gSH;			// Filtered chromatic spherical harmonics contribution.

float4 ToFloatSH(int4 coefficients) {

	return coefficients * 0.001f;

}

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	float4 color = ToFloatSH(int4(gSHRed[dispatch_thread_id],
								  gSHGreen[dispatch_thread_id],
								  gSHBlue[dispatch_thread_id],
								  0));

	color.a = 1.0f;

	gSH[dispatch_thread_id] = color;
	
}