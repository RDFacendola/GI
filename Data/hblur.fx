cbuffer PerFrame{

	float4 gBlurKernel[3];

	uint gWidth;
	uint gHeight;

}

struct VSOutput{

	float4 position_ps : SV_Position;
	float3 pix_position : TexCoord;

};

struct PSOutput{

	float4 color : SV_Target;

};

//////////////////////////////////// VERTEX SHADER ////////////////////////////////////////

VSOutput VSMain(uint vertex_id: SV_VertexID){
	
	VSOutput output;

	float2 uv = float2((vertex_id << 1) & 2, 
					   vertex_id & 2 );

	output.position_ps = float4( uv * float2( 2.0, -2.0 ) + float2( -1.0f, 1.0f), 
								 0.0f, 
								 1.0f );
	
	output.pix_position.x = uv.x * (float)gWidth;
	output.pix_position.y = uv.y * (float)gHeight;
	output.pix_position.z = 0;

	return output;

}

////////////////////////////////// PIXEL SHADER /////////////////////////////////////////

Texture2D gSource;

SamplerState blur_sampler;

float KernelSample(int index){

	return ((float[4])(gBlurKernel[index/4]))[index%4];

}

PSOutput PSMain(VSOutput input){

	PSOutput output;

	float4 color = 0;

	for(int sample_index = -4; sample_index < 5; ++sample_index){

		color += gSource.Load(int3(input.pix_position) + int3(sample_index, 0, 0)) * KernelSample(sample_index + 4);

	}

	output.color = color;

	output.color.a = 1.0;

	return output;

}

////////////////////////////////// PIXEL SHADER /////////////////////////////////////////
