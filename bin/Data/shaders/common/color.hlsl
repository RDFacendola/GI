/// \brief Contains functions for color manipulation.
/// \author Raffaele D. Facendola

/// \brief Gets the relative luminance of a color.

float Luminance(float3 color) {

	// http://www.scantips.com/lumin.html

	return max(dot(color, 
				   float3(0.3f, 0.59f, 0.11f)),
			   0.0001f);

}

/// \brief Performs an exposure adjustment of a given color.
/// \param unexposed_color The color to adjust.
float3 Expose(float3 unexposed_color, float average_luminance, float key_value, float bias) {

	average_luminance = max(average_luminance, 0.001f);

	float exposure = key_value / average_luminance;						// Linear exposure

	exposure = log2(max(exposure, 0.0001f)) + bias;

	return unexposed_color * exp2(exposure);
	
}
