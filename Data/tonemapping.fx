//////////////////////////////////// VERTEX SHADER ////////////////////////////////////////

struct VSOut
{

	float4 position_ps : SV_Position;		// Position in projection space.
	float2 uv : TexCoord;					// Texture coordinates.

};

void VSMain(uint vertex_id: SV_VertexID, out VSOut output){
	
	output.uv = float2((vertex_id << 1) & 2, 
					   vertex_id & 2 );

	output.position_ps = float4( output.uv * float2( 2.0, -2.0 ) + float2( -1.0f, 1.0f), 
								 0.0f, 
								 1.0f );
	
}

////////////////////////////////// PIXEL SHADER /////////////////////////////////////////

cbuffer PerFrame{

	float gExposure;
	float gVignette;

}

struct PSOut{

	float4 color : SV_Target;

};

Texture2D gHDR;

SamplerState gHDRSampler;

float Expose(float unexposed){

	return saturate(1.0 - pow(2.718, -unexposed * gExposure));

}

void PSMain(VSOut input, out PSOut output){

	float2 texcoord = float2(input.uv - 0.5);

	float vignette = pow( 1.0 - dot(texcoord, texcoord), gVignette );

	float4 unexposed = gHDR.Sample(gHDRSampler, input.uv) * vignette;

	output.color.r = Expose(unexposed.r);
	output.color.g = Expose(unexposed.g);
	output.color.b = Expose(unexposed.b);
	output.color.a = 1.0;

}
