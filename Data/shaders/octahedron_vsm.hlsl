#include "projection_def.hlsl"
#include "rsm_def.hlsl"

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

// See rsm_def.hlsl

/////////////////////////////////// GEOMETRY SHADER ////////////////////////////////////

// VSIn -> VSOut = GSIn = GSOut -> PSIn

#define GSIn VSOut
#define GSOut VSOut

/// Get the sign of the specified value as bit mask (100: negative, 010: zero, 001: positive)
int GetSignMask(float value) {

	return (1 << (1 - sign(value))) & 0x7;

}

/// Get the composite sign mask of the specified values
/// 1XX: at least one value was negative
/// X1X: at least one value was zero
/// XX1: at least one value was positive
int GetSignMask(float a, float b, float c) {

	return GetSignMask(a) | GetSignMask(b) | GetSignMask(c);

}

GSOut GetIntersection(GSIn a, GSIn b, float dota, float dotb) {

	float lerp_factor = saturate(-dota / (dotb - dota));

	GSOut output;

	output.position_ps = lerp(a.position_ps, b.position_ps, lerp_factor);
	output.normal_ws = lerp(a.normal_ws, b.normal_ws, lerp_factor);
	output.uv = lerp(a.uv, b.uv, lerp_factor);

	return output;

}

/// \brief Slices a triangle given a splitting plane.
/// \param polygon Triangle to split.
/// \param plane The plane the triangle will be splitted against.
/// \param strip Resulting triangle strip.
/// \return Returns true if the polygon was splitted, returns false otherwise.
bool SlicePolygon(GSIn polygon[3], float4 plane, out GSIn strip[5]) {

	// TODO: Can we reduce the branchiness from here?

	float dots[3];

	dots[0] = dot(polygon[0].position_ps, plane);
	dots[1] = dot(polygon[1].position_ps, plane);
	dots[2] = dot(polygon[2].position_ps, plane);

	int dot01 = sign(dots[0] * dots[1]);
	int dot02 = sign(dots[0] * dots[2]);
	int dot12 = sign(dots[1] * dots[2]);

	bool sliced = true;

	int3 order;

	if (!(dot01 < 0 || dot02 < 0 || dot12 < 0)) {

		order = int3(0, 1, 2);		// Any order will do

		sliced = false;

	}
	else if (dot01 < 0 && dot12 >= 0) {

		order = int3(0, 1, 2);

	}
	else if (dot12 < 0 && dot02 >= 0) {

		order = int3(1, 2, 0);

	}
	else {

		order = int3(2, 0, 1);

	}

	strip[0] = polygon[order.x];
	strip[1] = GetIntersection(polygon[order.x], polygon[order.y], dots[order.x], dots[order.y]);
	strip[2] = GetIntersection(polygon[order.x], polygon[order.z], dots[order.x], dots[order.z]);
	strip[3] = polygon[order.y];
	strip[4] = polygon[order.z];

	return sliced;

}

/// \brief Project a vertex into octahedron space and append the result to the output stream.
/// \param vertex Vertex to project to octahedron space and append.
/// \param flip Whether clamping the projection to the rear pyramid (true) or the front one(false).
/// \param output_stream The output stream.
void ProjectAndAppend(GSIn vertex, int flip, inout TriangleStream<GSOut> output_stream) {

	GSOut output_vertex;

	output_vertex.position_ps = ProjectToOctahedronSpace(vertex.position_ps.xyz, gNearPlane, gFarPlane, flip);
	output_vertex.normal_ws = vertex.normal_ws;
	output_vertex.uv = vertex.uv;

	output_stream.Append(output_vertex);

}

/// \brief Slice a triangle along a plane and append the result to the output stream.
/// \param polygon The polygon to slice.
/// \param plane The plane the polygon will be sliced against.
/// \param flip Whether the result will be projected on the rear (true) pyramid or the front one (false).
/// \param output_stream The output stream.
void SliceAndAppend(GSIn polygon[3], float4 plane, bool flip, inout TriangleStream<GSOut> output_stream) {

	GSIn strip[5];

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

void GSMainXY(GSIn input[3], inout TriangleStream<GSOut> output_stream, bool flip) {

	GSIn strip[5];

	int2 sign_mask = int2( GetSignMask(input[0].position_ps.x, input[1].position_ps.x, input[2].position_ps.x),
						   GetSignMask(input[0].position_ps.y, input[1].position_ps.y, input[2].position_ps.y) );

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
void GSMain(triangle GSIn input[3], inout TriangleStream<GSOut> output_stream) {
	
	bool split_z = (GetSignMask(input[0].position_ps.z, input[1].position_ps.z, input[2].position_ps.z) & 0x5) == 0x5;
	
	// Splitting along Z takes precedence since we cannot output a strip for the spanning triangles but a list

	if (!split_z) {

		GSMainXY(input, output_stream, input[0].position_ps.z < 0.0f);

	}
	else {
				
		GSIn strip[5];

		SlicePolygon(input, float4(0, 0, 1, 0), strip);		// This will return "true" for sure - 5 vertices

		bool flip = strip[0].position_ps.z < 0.0f;

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

// See rsm_def.hlsl

