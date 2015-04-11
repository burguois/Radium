float4 swirl(float4 uv)
{
  float4 color = tex2D( s_image, uv );

  color *= 3;
  color = floor(color);
  color /= 3;
  
  return color;
    
}