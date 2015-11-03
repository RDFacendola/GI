#include "projection_def.hlsl"

struct VSIn {

	float4 position : SV_Position;				// Vertex position.

};

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

struct VSOut {

	float4 position_ls: SV_Position;			// Vertex position in light space.

};

cbuffer PerObject{

	float4x4 gWorldLight;						// World * Light-view matrix.

};

VSOut VSMain(VSIn input){

	VSOut output;

	output.position_ls = mul(gWorldLight, input.position);

	return output;

}

/////////////////////////////////// HULL SHADER ////////////////////////////////////////

struct HSOut {

	float4 position_ls: SV_Position;			// Vertex position in light space.

};

struct TessOut {

	float edge_tessellation[3] : SV_TessFactor;
	float inside_tessellation : SV_InsideTessFactor;

};

TessOut TessFunction(InputPatch<VSOut, 3> patch, uint patch_id : SV_PrimitiveID) {

	TessOut output;

	output.edge_tessellation[0] = 8;
	output.edge_tessellation[1] = 8;
	output.edge_tessellation[2] = 8;

	output.inside_tessellation = 3;				

	return output;

}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("TessFunction")]
[maxtessfactor(64.0f)]
HSOut HSMain(InputPatch<VSOut, 3> input, uint i : SV_OutputControlPointID, uint patch_id : SV_PrimitiveID) {

	HSOut output;

	output.position_ls = input[i].position_ls;

	return output;

}

/////////////////////////////////// DOMAIN SHADER //////////////////////////////////////

cbuffer PerLight {

	float gNearPlane;							// Near clipping plane

	float gFarPlane;							// Far clipping plane

};

struct DSOut {

	float4 position_ps: SV_Position;			// Vertex position in paraboloid space.

	float z : TEXCOORD0;						// Z-coordinate of the vertex as seen from the paraboloid.

};

[domain("tri")]
DSOut DSMain(TessOut tessellation, float3 uvw : SV_DomainLocation, const OutputPatch<HSOut, 3> input) {

	float4 position_ls = input[0].position_ls * uvw.x + input[1].position_ls * uvw.y + input[2].position_ls * uvw.z;

	DSOut output;

	output.z = sign(position_ls.z);				// 1 for front paraboloid, -1 for rear paraboloid

	output.position_ps = ProjectToParaboloidSpace(position_ls.xyz,
												  gNearPlane,
												  gFarPlane);

	output.position_ps.x *= 0.5f;				// Squash the X coordinate horizontally in order to fit both the front and the rear paraboloid in the same texture

	return output;

}

/////////////////////////////////// GEOMETRY SHADER ////////////////////////////////////

[maxvertexcount(6)]
void GSMain(triangle DSOut input[3], inout TriangleStream<DSOut> output) {

	// Adjust the viewport position (front paraboloid on the left half, rear paraboloid on the right one)

	int i;

	if (abs(input[0].z + input[1].z + input[2].z) >= 3.0f) {

		// The primitive is fully in front or rear the light POV, output just one primitive.

		[unroll]
		for (i = 0; i < 3; ++i) {

			input[i].position_ps.x -= 0.5f * input[i].z;		// Output on the front or on the rear paraboloid, according to the z.

			input[i].z = 1.0f;									// Prevent clipping at pixel level.

			output.Append(input[i]);

		}

	}
	else {

		// The primitive spans both the front and the rear paraboloid, output one primitive per paraboloid.

		[unroll]
		for (i = 0; i < 3; ++i) {

			input[i].position_ps.x -= 0.5f;						// Front paraboloid

			output.Append(input[i]);

		}

		output.RestartStrip();

		// Rear paraboloid
		[unroll]
		for (i = 2; i >= 0; --i) {

			input[i].position_ps.x += 1.0f;						// Rear paraboloid

			input[i].z *= -1.0f;								// Flip the polygon

			output.Append(input[i]);

		}

	}

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float2 PSMain(DSOut input) : SV_Target0{

	clip(input.z);

	return float2(input.position_ps.z,
				  input.position_ps.z * input.position_ps.z);

}
