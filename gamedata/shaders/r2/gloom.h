float4 AdjustSaturation(float4 color, float saturation)
{
float grey = dot(color, float3(0.3, 0.59, 0.11));
return lerp(grey, color, saturation);
}

float4 gloom(float2 uv, float GloomIntensity, float BaseIntensity, float GloomSaturation, float BaseSaturation)
{
float GloomThreshold = 0.25f;
float4 base = 1.0 - tex2D(s_image, uv);
float4 gloom = saturate((base - GloomThreshold) / (1 - GloomThreshold));
gloom = AdjustSaturation(gloom, GloomSaturation) * GloomIntensity;
base = AdjustSaturation(base, BaseSaturation) * BaseIntensity;
base *= (1 - saturate(gloom));
return 1.0 - (base + gloom);
}