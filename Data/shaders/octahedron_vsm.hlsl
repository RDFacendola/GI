#include "hlsl_def.hlsl"
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

float4 GetIntersection(float4 a, float4 b, float dota, float dotb) {

	return lerp(a,
				b, 
				-dota / (dotb - dota));
	
}

// Slices a polygon given a splitting plane.
int SlicePolygon(float4 polygon[3], float4 plane, out float4 strip[5]) {

	float dots[3];

	dots[0] = dot(polygon[0], plane);
	dots[1] = dot(polygon[1], plane);
	dots[2] = dot(polygon[2], plane);
	
	int dot01 = sign(dots[0] * dots[1]);
	int dot02 = sign(dots[0] * dots[2]);
	int dot12 = sign(dots[1] * dots[2]);
	
	int vertices;

	if (dot01 + dot02 + dot12 >= 0) {

		// The polygon should not be split!
		
		strip[0] = polygon[0];
		strip[1] = polygon[1];
		strip[2] = polygon[2];
		strip[3] = polygon[1];		// Not really needed
		strip[4] = polygon[2];		// Not really needed

		vertices = 3;

	}else if(dot01 < 0 && dot12 >= 0){

		strip[0] = polygon[0];
		strip[1] = GetIntersection(polygon[0], polygon[1], dots[0], dots[1]);
		strip[2] = GetIntersection(polygon[0], polygon[2], dots[0], dots[2]);
		strip[3] = polygon[1];
		strip[4] = polygon[2];

		vertices = 5;
		
	}
	else if (dot12 < 0 && dot02 >= 0) {
		
		strip[0] = polygon[1];
		strip[1] = GetIntersection(polygon[1], polygon[2], dots[1], dots[2]);
		strip[2] = GetIntersection(polygon[1], polygon[0], dots[1], dots[0]);
		strip[3] = polygon[2];
		strip[4] = polygon[0];

		vertices = 5;

	}
	else {

		strip[0] = polygon[2];
		strip[1] = GetIntersection(polygon[2], polygon[0], dots[2], dots[0]);
		strip[2] = GetIntersection(polygon[2], polygon[1], dots[2], dots[1]);
		strip[3] = polygon[0];
		strip[4] = polygon[1];
		
		vertices = 5;

	}
	
	return vertices;

}

/// Output the given triangle strip.
/// side: -1 back side, 1 front side, 0 both
void OutputStrip(float4 strip[5], int vertices, inout TriangleStream<GSOut> output_stream, int sign_mask) {

	GSOut output;
	
	// Fit both the front and the rear shadowmap on the same viewport. The front one is on the left, while the back one is on the right.

	// At least one vertex is on the positive Z plane: append the strip as-is

	if (sign_mask & 0x1) {

		for (int index = 0; index < vertices; ++index) {

			output.position_ps = ProjectToOctahedronSpace(strip[index].xyz, gNearPlane, gFarPlane, false);
		
			output_stream.Append(output);
		
		}
	
		output_stream.RestartStrip();

	}
		
	// At least one vertex is on the negative Z plane: append the strip flipped on the x axis (this will transform cw polygons to ccw and viceversa)

	if (sign_mask & 0x4) {

		for (int index = 0; index < vertices; ++index) {

			output.position_ps = ProjectToOctahedronSpace(strip[index].xyz, gNearPlane, gFarPlane, true);
		
			output_stream.Append(output);
		
		}
	
		output_stream.RestartStrip();

	}
	
}

[maxvertexcount(30)]
void GSMain(triangle float4 input[3] : SV_Position, inout TriangleStream<GSOut> output_stream) {
	
	static const float4 kXPlane = float4(1, 0, 0, 0);
	static const float4 kYPlane = float4(0, 1, 0, 0);
	
	float4 strip[5];

	int3 sign_mask = int3( GetSignMask(input[0].x, input[1].x, input[2].x),
						   GetSignMask(input[0].y, input[1].y, input[2].y),
						   GetSignMask(input[0].z, input[1].z, input[2].z) );

	bool split_x = (sign_mask.x & 0x5) == 0x5;
	bool split_y = (sign_mask.y & 0x5) == 0x5;

	if (!split_x && !split_y) {

		// Don't split

		strip[0] = input[0];
		strip[1] = input[1];
		strip[2] = input[2];
		strip[3] = 0;			// Whatever
		strip[4] = 0;			// Whatever

		OutputStrip(strip, 3, output_stream, sign_mask.z);

	}else if (split_x ^ split_y) {

		// Split along X or Y

		int vertices = SlicePolygon(input, 
									split_x ? kXPlane : kYPlane, 
									strip);

		OutputStrip(strip, vertices, output_stream, sign_mask.z);

	}
	else{
		
		// Split along X and Y

		SlicePolygon(input, kXPlane, strip);									// Will output a triangle and a trapezoid (5 vertices total)

		input[0] = strip[0];
		input[1] = strip[1];
		input[2] = strip[2];

		float4 substrip[5];

		// First

		int vertices = SlicePolygon(input, kYPlane, substrip);					// Split the triangle

		OutputStrip(substrip, vertices, output_stream, sign_mask.z);
		
		// Second

		input[0] = strip[2];
		input[1] = strip[1];
		input[2] = strip[3];

		vertices = SlicePolygon(input, kYPlane, substrip);

		OutputStrip(substrip, vertices, output_stream, sign_mask.z);

		// Third

		input[0] = strip[2];
		input[1] = strip[3];
		input[2] = strip[4];

		vertices = SlicePolygon(input, kYPlane, substrip);
		
		OutputStrip(substrip, vertices, output_stream, sign_mask.z);

	}
	
}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

float2 PSMain(GSOut input) : SV_Target0{

	// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html
	
	clip(input.position_ps.z);

	float2 moments;

	float dx = ddx(input.position_ps.z);
	float dy = ddy(input.position_ps.z);

	return float2(input.position_ps.z,
				  input.position_ps.z * input.position_ps.z + 0.25f * (dx * dx + dy * dy));

}
