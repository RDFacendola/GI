cbuffer PerFrame{

	float gExposure;

}

struct VSOutput{

	float4 position_ps : SV_Position;
	float2 uv: TexCoord;

};

struct PSOutput{

	float4 color : SV_Target;

};

//////////////////////////////////// VERTEX SHADER ////////////////////////////////////////

VSOutput VSMain(uint vertex_id: SV_VertexID){
	
	VSOutput output;

	output.uv = float2((vertex_id << 1) & 2, 
					   vertex_id & 2 );

	output.position_ps = float4( output.uv * float2( 2.0, -2.0 ) + float2( -1.0f, 1.0f), 
								 0.0f, 
								 1.0f );
	
	return output;

}

////////////////////////////////// PIXEL SHADER /////////////////////////////////////////

Texture2D gHDR;

SamplerState hdr_sampler;

float Expose(float unexposed){

	return saturate(1.0 - pow(2.718, -unexposed * gExposure));

}

PSOutput PSMain(VSOutput input){

	PSOutput output;

	float2 texcoord = float2(input.uv - 0.5);

	float vignette = pow( 1.0 - dot(texcoord, texcoord), 2.0 );

	float4 unexposed = gHDR.Sample(hdr_sampler, input.uv) * vignette;

	output.color.r = Expose(unexposed.r);
	output.color.g = Expose(unexposed.g);
	output.color.b = Expose(unexposed.b);
	output.color.a = 1.0;

	return output;

}
