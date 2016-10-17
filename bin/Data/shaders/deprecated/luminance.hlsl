/// \brief Calculate an image relative luminance

#include "color.hlsl"

#define SIZE 16
#define TOTAL_THREADS (SIZE * SIZE)

Texture2D gSource;					///< \brief Source image.
RWTexture2D<float4> gLuminance;		///< \brief Image containing the relative luminance for each source location.

[numthreads(SIZE, SIZE,1)]
void CSMain(int3 dispatch_thread_id : SV_DispatchThreadID) {

	gLuminance[dispatch_thread_id.xy] = Luminance(gSource[dispatch_thread_id.xy]);

}