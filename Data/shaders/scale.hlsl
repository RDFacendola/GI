//////////////////////////////////// VERTEX SHADER ////////////////////////////////////////

struct VSOut
{

	float4 position_ps : SV_Position;		// Position in projection space.
	float2 uv : TexCoord;					// Texture coordinates.

};

void VSMain(uint vertex_id: SV_VertexID, out VSOut output){
	
	output.uv = float2((vertex_id << 1) & 2, 
					   vertex_id & 2 );

	output.position_ps = float4( output.uv * float2(2.0, -2.0) + float2(-1.0f, 1.0f), 
								 0.0f, 
								 1.0f );
	
}


Texture2D gSource;

SamplerState gSourceSampler;

float4 PSMain(VSOut input){

	// Simple pixel shader, take the input and renders it to the output. Useful for upscaling and downscaling.

	return gSource.Sample(gSourceSampler, input.uv);

}