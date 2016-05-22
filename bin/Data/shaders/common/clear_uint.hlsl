/// \brief Clears a buffer of unsigned int

#include "color.hlsl"

#define N 256
#define TOTAL_THREADS (N)

RWStructuredBuffer<uint> gBuffer;			///< \brief Buffer to clear

[numthreads(N, 1, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	gBuffer[dispatch_thread_id.x] = 0;

}