#include "common/hlsl_def.hlsl"
#include "projection_def.hlsl"

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

cbuffer PerObject{

	float4x4 gWorldLight;						// World * Light-view matrix.

};

float4 VSMain(float4 position : SV_Position) : SV_Position{

	return mul(gWorldLight, position);

}

/////////////////////////////////// GEOMETRY SHADER ////////////////////////////////////

cbuffer PerLight {

	float gNearPlane;							// Near clipping plane

	float gFarPlane;							// Far clipping plane

};

struct GSOut {

	float4 position_ps: SV_Position;			// Vertex position in paraboloid space.

};

/// \brief Project a vertex into octahedron space and append the result to the output stream.
/// \param vertex Vertex to project to octahedron space and append.
/// \param flip Whether clamping the projection to the rear pyramid (true) or the front one(false).
/// \param output_stream The output stream.
void ProjectAndAppend(float4 vertex, int flip, inout TriangleStream<GSOut> output_stream) {

	GSOut output_vertex;

	output_vertex.position_ps = ProjectToOctahedronSpace(vertex.xyz, gNearPlane, gFarPlane, flip);

	output_stream.Append(output_vertex);

}

/// \brief Slice a triangle along a plane and append the result to the output stream.
/// \param polygon The polygon to slice.
/// \param plane The plane the polygon will be sliced against.
/// \param flip Whether the result will be projected on the rear (true) pyramid or the front one (false).
/// \param output_stream The output stream.
void SliceAndAppend(float4 polygon[3], float4 plane, bool flip, inout TriangleStream<GSOut> output_stream) {

	float4 strip[5];

	if (SlicePolygon(polygon, plane, strip)) {

		ProjectAndAppend(strip[0], flip, output_stream);
		ProjectAndAppend(strip[1], flip, output_stream);
		ProjectAndAppend(strip[2], flip, output_stream);
		ProjectAndAppend(strip[3], flip, output_stream);
		ProjectAndAppend(strip[4], flip, output_stream);

	}
	else {

		ProjectAndAppend(strip[0], flip, output_stream);
		ProjectAndAppend(strip[3], flip, output_stream);
		ProjectAndAppend(strip[4], flip, output_stream);

	}

}

void GSMainXY(float4 input[3], inout TriangleStream<GSOut> output_stream, bool flip) {

	int2 sign_mask = int2( GetSignMask(input[0].x, input[1].x, input[2].x),
						   GetSignMask(input[0].y, input[1].y, input[2].y) );

	bool split_x = (sign_mask.x & 0x5) == 0x5;
	bool split_y = (sign_mask.y & 0x5) == 0x5;


	if (!split_x && !split_y) {

		// No split required - 3 vertices output

		ProjectAndAppend(input[0], flip, output_stream);
		ProjectAndAppend(input[1], flip, output_stream);
		ProjectAndAppend(input[2], flip, output_stream);

	}
	else if (split_x ^ split_y) {

		// Split along X or Y - 5 vertices output

		float4 strip[5];

		float4 plane = (split_x) ? float4(1, 0, 0, 0) : float4(0, 1, 0, 0);

		SlicePolygon(input, plane, strip);

		ProjectAndAppend(strip[0], flip, output_stream);
		ProjectAndAppend(strip[1], flip, output_stream);
		ProjectAndAppend(strip[2], flip, output_stream);
		ProjectAndAppend(strip[3], flip, output_stream);
		ProjectAndAppend(strip[4], flip, output_stream);

	}
	else {

		// Split along X and Y - Up to 15 vertices output + 4 for strip restart
		
		float4 strip[5];

		SlicePolygon(input, float4(1, 0, 0, 0), strip);

		// Unwind the triangle fan centered at strip[2]

		input[0] = strip[2];								

		input[1] = strip[0];
		input[2] = strip[1];

		SliceAndAppend(input, float4(0, 1, 0, 0), flip, output_stream);
		
		output_stream.RestartStrip();

		input[1] = strip[1];
		input[2] = strip[3];

		SliceAndAppend(input, float4(0, 1, 0, 0), flip, output_stream);

		output_stream.RestartStrip();

		input[1] = strip[3];
		input[2] = strip[4];

		SliceAndAppend(input, float4(0, 1, 0, 0), flip, output_stream);

	}

}

[maxvertexcount(40)]
void GSMain(triangle float4 input[3] : SV_Position, inout TriangleStream<GSOut> output_stream) {
	
	bool split_z = (GetSignMask(input[0].z, input[1].z, input[2].z) & 0x5) == 0x5;
	
	// Splitting along Z takes precedence since we cannot output a strip for the spanning triangles but a list

	if (!split_z) {

		GSMainXY(input, output_stream, input[0].z < 0.0f);

	}
	else {
				
		float4 strip[5];

		SlicePolygon(input, float4(0, 0, 1, 0), strip);		// This will return "true" for sure - 5 vertices

		bool flip = strip[0].z < 0.0f;

		// Unwind the triangle fan centered at strip[2]

		input[0] = strip[2];								

		input[1] = strip[0];
		input[2] = strip[1];

		GSMainXY(input, output_stream, flip);

		output_stream.RestartStrip();

		input[1] = strip[1];
		input[2] = strip[3];

		GSMainXY(input, output_stream, !flip);

		output_stream.RestartStrip();

		input[1] = strip[3];
		input[2] = strip[4];

		GSMainXY(input, output_stream, !flip);
		
	}
			
}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float2 PSMain(GSOut input) : SV_Target0{

	// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html
	
	float2 moments;

	float dx = ddx(input.position_ps.z);
	float dy = ddy(input.position_ps.z);

	return float2(input.position_ps.z,
				  input.position_ps.z * input.position_ps.z + 0.25f * (dx * dx + dy * dy));

}
