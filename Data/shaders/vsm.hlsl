#include "projection_def.hlsl"

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

struct VSIn {

	float4 position : SV_Position;				// Vertex position.
	
};

struct VSOut {

	float4 position_ps: SV_Position;			// Vertex position in paraboloid space.

	float z : TEXCOORD0;						// Z-coordinate of the vertex as seen from the paraboloid.
	
};

cbuffer PerObject{

	float4x4 gWorldLight;						// World * Light-view matrix.

};

cbuffer PerLight {

	float gNearPlane;							// Near clipping plane

	float gFarPlane;							// Far clipping plane

};

void VSMain(VSIn input, out VSOut output){

	float4 position_ls = mul(gWorldLight, input.position);

	output.z = sign(position_ls.z);				// 1 for front paraboloid, -1 for rear paraboloid

	output.position_ps = ProjectToParaboloidSpace(position_ls.xyz,
												  gNearPlane,
												  gFarPlane);

	output.position_ps.x *= 0.5f;				// Squash the X coordinate horizontally in order to fit both the front and the rear paraboloid in the same texture

}

/////////////////////////////////// GEOMETRY SHADER ////////////////////////////////////

[maxvertexcount(6)]
void GSMain(triangle VSOut input[3], inout TriangleStream<VSOut> output) {

	// Adjust the viewport position (front paraboloid on the left half, rear paraboloid on the right one)

	int i;

	if (abs(input[0].z + input[1].z + input[2].z) >= 2.0f) {

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

float2 PSMain(VSOut input) : SV_Target0{

	clip(input.z);

	return float2(input.position_ps.z,
				  input.position_ps.z * input.position_ps.z);

}
