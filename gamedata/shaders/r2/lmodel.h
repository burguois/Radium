#ifndef	LMODEL_H
#define LMODEL_H

#include "common.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Lighting formulas			// 
half4 	plight_infinity		(half m, half3 point, half3 normal, half3 light_direction)       				{
  half3 N		= normal;							// normal 
  half3 V 		= -normalize	(point);					// vector2eye
  half3 L 		= -light_direction;						// vector2light
  half3 H		= normalize	(L+V);						// half-angle-vector 
  return tex3D 		(s_material,	half3( dot(L,N), dot(H,N), m ) );		// sample material
}

half4 	specular_infinity		(half m, half3 point, half3 normal, half3 light_direction)       				{
  half3 N		= normal;							// normal 
  half3 V 		= -normalize	(point);					// vector2eye
  half3 L 		= -light_direction;						// vector2light
  half3 H		= normalize	(L+V);						// half-angle-vector
  half4 light = tex3D  (s_material, half3( dot(L,N), dot(H,N), m ) );  // sample material  
  float sum_light = saturate(0.6*light.x + light.w*1.4);
return float4(sum_light,sum_light,sum_light,sum_light);
}

half4 	plight_local		(half m, half3 point, half3 normal, half3 light_position, half light_range_rsq, out float rsqr)  {
  half3 N		= normal;							// normal 
  half3 L2P 	= point-light_position;                         		// light2point 
  half3 V 		= -normalize	(point);					// vector2eye
  half3 L 		= -normalize	((half3)L2P);					// vector2light
  half3 H		= normalize	(L+V);						// half-angle-vector
		rsqr	= dot		(L2P,L2P);					// distance 2 light (squared)
  half  att 	= saturate	(1 - rsqr*light_range_rsq);			// q-linear attenuate
  half4 light	= tex3D		(s_material, half3( dot(L,N), dot(H,N), m ) ); 	// sample material
  return att*light;
}

half4 	specular_local		(half m, half3 point, half3 normal, half3 light_position, half light_range_rsq, out float rsqr)  {
  half3 N		= normal;							// normal 
  half3 L2P 	= point-light_position;                         		// light2point 
  half3 V 		= -normalize	(point);					// vector2eye
  half3 L 		= -normalize	((half3)L2P);					// vector2light
  half3 H		= normalize	(L+V);						// half-angle-vector
		rsqr	= dot		(L2P,L2P);					// distance 2 light (squared)
  half  att 	= saturate	(1 - rsqr*light_range_rsq);			// q-linear attenuate
  half4 light	= tex3D		(s_material, half3( dot(L,N), dot(H,N), m ) ); 	// sample material
  float sum_light = saturate(0.6*light.x + light.w*1.4);
return att*float4(sum_light,sum_light,sum_light,sum_light);
}

half4	blendp	(half4	value, float4 	tcp)    		{
	#ifndef FP16_BLEND  
		value 	+= (half4)tex2Dproj 	(s_accumulator, tcp); 	// emulate blend
	#endif
	return 	value	;
}
half4	blend	(half4	value, float2 	tc)			{
	#ifndef FP16_BLEND  
		value 	+= (half4)tex2D 	(s_accumulator, tc); 	// emulate blend
	#endif
	return 	value	;
}

#endif
