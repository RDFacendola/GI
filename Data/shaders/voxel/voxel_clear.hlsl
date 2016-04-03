
#define N 256
#define TOTAL_THREADS (N)

RWStructuredBuffer<uint> gVoxelAddressTable;

[numthreads(N, 1, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	gVoxelAddressTable[dispatch_thread_id.x] = 0;

}