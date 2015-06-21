


cbuffer PerDispatch{

	float gVariable;

};

Texture2D texture_a;
Texture2D texture_b;

RWTexture2D<float4> uav;

[numthreads(16,16,1)]
void CSMain(int3 thread_id : SV_DispatchThreadID){

	uav[thread_id.xy] = texture_a[thread_id.xy] + texture_b[thread_id.xy];

}