float4 sharpen(float2 uv, float Amount, float Width)
{
float4 color = s_image.Sample(s_image, uv);
color.rgb += s_image.Sample(s_image, uv - Width) * Amount;
color.rgb -= s_image.Sample(s_image, uv + Width) * Amount;
return color;
} 