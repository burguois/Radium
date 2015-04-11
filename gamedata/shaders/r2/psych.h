float4 psych(float2 uv)
{
uv.y = uv.y  + (sin(uv.y*100)*0.03);
float4 Color = tex2D( s_image , uv.xy);
return Color;
}