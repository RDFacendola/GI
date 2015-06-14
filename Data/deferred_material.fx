
cbuffer PerObjectVS{

	float4x4 gWorldViewProj;
	float4x4 gWorldView;
	float4x4 gWorld;
	float4x4 gView;

};

cbuffer Lights{

	float4 gLightView;

}

struct Light{

	float4 position;

};

Texture2D ps_map;

StructuredBuffer<Light> gLights;

SamplerState tex_sampler;

struct VSInput{

	float4 position : SV_Position;
	float4 normal : NORMAL;
	float2 uv: TEXCOORD;

};

struct VSOutput{

	float4 proj_position : SV_Position;
	float3 view_position : ViewPosition;
	float3 view_normal : NORMAL;
	float2 uv : TEXCOORD;

};

struct PSOutput{

	float4 color : SV_Target;

};

VSOutput VSMain(VSInput input){

	VSOutput o;

	o.proj_position = mul(gWorldViewProj, input.position);
	
	o.view_position = mul(gWorldView, input.position);

	o.view_normal = normalize(mul((float3x3)gWorldView, (float3)input.normal));

	o.uv = float2(input.uv.x,
				  1.0 - input.uv.y);	// V coordinate is flipped because we are using ogl convention.

	return o;

}

PSOutput PSMain(VSOutput input){

	PSOutput o;

	// Diffuse

	float3 surface_position = mul(gWorldView, gLights[0].position).xyz; //input.view_position.xyz;

	float3 light_direction = normalize(surface_position - gLightView.xyz);	// From the light to the surface

	float3 view_direction = normalize(surface_position - float3(0, 0, 0));	// From the viewer to the surface

	// Diffuse	

	float diffuse_intensity = saturate(dot(-light_direction, input.view_normal));

	// Specular

	float3 reflection_direction = reflect(-light_direction, input.view_normal);

	float specular_intensity = saturate(dot(reflection_direction, view_direction));

	// Accumulation

	float4 diffuse_color = ps_map.Sample(tex_sampler, input.uv);

	float4 specular_color = float4(1,1,1,1);	// White

	float4 light_color = float4(1,1,1,1);		// White

	float shininess = 60;

	o.color = diffuse_color * light_color * diffuse_intensity + specular_color * pow(specular_intensity, shininess);

	//o.color = float4(abs(input.view_normal), 1);

	o.color.a = 1;

	return o;

}
