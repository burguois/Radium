float4 sharpen(float2 uv, float Amount, float Width)
{
float4 color = tex2D(s_image, uv);
color.rgb += tex2D(s_image, uv - Width) * Amount;
color.rgb -= tex2D(s_image, uv + Width) * Amount;
return color;
} 