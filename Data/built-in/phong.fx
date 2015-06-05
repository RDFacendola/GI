
float4 diffuse;

float4x4 gWorldViewProj;

Texture2D diffuse_map;
Texture2D specular_map;
Texture2D bump_map;

struct VSInput{

	float4 position : SV_Position;
	float4 normal : NORMAL;
	float2 uv: TEXCOORD;

};

struct VSOutput{

	float4 position : SV_Position;
	float4 color : COLOR;

};

struct PSOutput{

	float4 color : SV_Target;

};

VSOutput VSMain(VSInput input){

	VSOutput o;

	o.position = mul(input.position, gWorldViewProj);
	o.color = diffuse;

	return o;

}

PSOutput PSMain(VSOutput input){

	PSOutput o;

	o.color = input.color;

	return o;

}
