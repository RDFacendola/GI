
cbuffer PerObjectVS{

	float4x4 gWorldViewProj;
	float4x4 gWorldView;
	float4x4 gWorld;
	float4x4 gView;
	float4 gEye;

};

struct Light{

	float4 position_ws;

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

	float4 position_ps : SV_Position;
	float4 position_ws : WorldPosition;
	float3 normal_ws : NORMAL;
	float2 uv : TEXCOORD;

};

struct PSOutput{

	float4 color : SV_Target;

};

VSOutput VSMain(VSInput input){

	VSOutput o;

	o.position_ps = mul(gWorldViewProj, input.position);								// Vertex position, in projection space
	
	o.position_ws = mul(gWorld, input.position);										// Vertex position, in world space

	o.normal_ws = normalize(mul((float3x3)gWorld, (float3)input.normal));				// Vertex normal, in world space

	o.uv = float2(input.uv.x,
				  1.0 - input.uv.y);	// V coordinate is flipped because we are using ogl convention.

	return o;

}

float4 AccumulateLight(VSOutput input, int light_index){

	float4 light_vector = gLights[light_index].position_ws - input.position_ws;		// From the surface to the light

	float attenuation = min(1, 1700000 / (4*3.14159*dot(light_vector.xyz, light_vector.xyz)));
	
	float4 light_direction = normalize(light_vector);

	float3 surface_normal = normalize(input.normal_ws);		// Normalize to compensate for interpolation

	// Diffuse contribution

	float4 albedo = ps_map.Sample(tex_sampler, input.uv);

	float diffuse_intensity = saturate(dot(light_direction.xyz, surface_normal));

	// Specular contribution

	float shininess = 60;

	float4 specular_color = 1;

	float3 eye_direction = normalize(gEye.xyz - input.position_ws.xyz);						// From the surface to the eye

	float3 reflected = reflect(light_direction.xyz, surface_normal);						// 

	float specular_intensity = pow(saturate(dot(reflected, -eye_direction)), shininess);

	// Preserve the alpha channel of the texture

	float4 final_color = (albedo * diffuse_intensity + specular_color * specular_intensity) * attenuation;

	final_color.a = albedo.a;

	return final_color;

}

PSOutput PSMain(VSOutput input){

	PSOutput o;

	o.color = 0;

	[unroll]
	for(int light_index = 0; light_index < 32; ++light_index){

		o.color += AccumulateLight(input, light_index);

	}

	return o;

}
