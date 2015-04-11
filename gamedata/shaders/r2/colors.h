float4 colors(float2 uv, float colorshift_offset, float red_offset, float green_offset, float blue_offset) 
{ 
    float4 color = tex2D(s_image, uv);
	
    color.r += red_offset * colorshift_offset;
    if (color.r > 1.0 || color.r < 0.0)
	color.r = color.r - 1.0;
    color.g += green_offset * colorshift_offset;
    if (color.g > 1.0 || color.g < 0.0)
	color.g = color.g - 1.0;
    color.b += blue_offset * colorshift_offset;
    if (color.b > 1.0 || color.b < 0.0)
	color.b = color.b - 1.0;
 
    return color;
}