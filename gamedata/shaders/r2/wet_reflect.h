/* */
/* wet_relection.h - Wet reflection shader (C)2011-2013 by Meltac */
/* */

#include "refraction.h"
//#ifdef WET_REFBLUR
//#include "lightdetect.h"
//#include "blur.h"
//#endif

float4 decolor(float4 Color) : COLOR
{	
	Color.r *= 5;	
	float sat = 0.7;
	//float sun = COLOR_SAT_SUN_COEF;
	float sun = 0.6;	
	float gray=dot(Color.xyz,float3(0.3,0.59,0.11));
	Color.xyz=lerp(float3(gray,gray,gray),Color.xyz,sat+(dot(Ldynamic_color.xyz,float3(1,1,1))/3)*sun);	
	return Color;
}


float4 checkwet(float2 uv, bool what) : COLOR
{     	
	float4 Color;
	Color = tex2Dlod( s_image , float4(uv.xy,0,0));
	#ifdef WET_MAXDIST
	float3 viewspace_P = tex2Dlod(s_position, float4(uv,0,0)).xyz; // float3(uv.x, uv.y, -uv.y);
	#endif
	float4 check_col = tex2Dlod(s_bloom, float4(uv,0,0));
	float lum = (Color.r + Color.g + Color.b) / 3;
	
	if (what)
		check_col /= L_hemi_color;
	else		
		check_col = (((check_col / L_hemi_color) / Ldynamic_color) / float4(L_sun_color, 1))  / 3;
	
	float m = 0.08;   // e_barrier.x * 10; //0.3;    
    float g = 0.30;	  // e_barrier.y * 10; //0.99;
    float r = 0.86;	  // e_barrier.z * 10; //1.0;
    float col_ratio = check_col.b / ( (check_col.r + check_col.g + check_col.b) / 3);

    if (check_col.b > m && check_col.r < g && check_col.g < g && col_ratio > r
	#ifdef WET_MAXDIST
	&& viewspace_P.z < WET_MAXDIST
	#endif
	&& lum > WET_MINLUM)
    {	
		float amount = saturate(((check_col.b-m)+0.01) * ((g-check_col.r)+0.01) * ((g-check_col.g)+0.01) * ((col_ratio-r)+0.01) * (0.5-0.4) * 100000);		
		Color.a = 0.1;
	}
	else	
		Color.a = 1.0;
	
	return Color;
}


float4 decolorize(float2 uv, bool what) : COLOR
{ 
	float4 Color = checkwet(uv, what);
	if (Color.a == 0.1)
		Color = decolor(Color);
	return Color;
}


float4 normalize_blue(float4 Color) : COLOR
{
 return float4(Color.r / Color.b, Color.g / Color.b, 1, Color.a);
}

#ifdef WET_OBJREFLECT
float4 objreflect(float4 Color, float4 P, int mat, bool indoor_wet, bool indoor_ref, float2 uv : TEXCOORD, float dist_factor, float wind, float colAmount, float refAmount, float rain_density, float water_height) : COLOR
{
	//int WET_OBJSAMPLES = 40; // (int)e_weights.x*100;
	//int WET_OBJSTEP = (int)e_weights.y*100;
	//int WET_OBJFADE = (int)e_weights.z*100;
	//float WET_OBJDEPTH = e_weights.z;
	
	float yMax = 0.5;  	
	bool obj = false;
	float lastDepth = P.z;
	float expectedDepth = 0;
	float maxObjDepth = 0;
	float objDepth = 0;
	float4 objPos = 0;
	int objMat = mat;
	int sample = 0;
		
	if ( (mat == 2 || mat == 1 && (indoor_wet || indoor_ref) ) && P.z < WET_OBJMAXDISTCOL) // restrict object reflections on terrain
	{		
		float yF   = 1.f / 768.f;
		float yFac = WET_OBJSTEP / 768.f;		
		
		for (int i=1; i <= WET_OBJSAMPLES; i++)
		{
			if (!obj)
			{
				float testY = uv.y - yFac * i;
				float4 testPos = tex2Dlod( s_position , float4(uv.x, testY, 0, 0) );	
				
				bool above_water = false;
				
				if (water_height != 0)
				{
					float3 testPos_world     = mul(float4(testPos.xyz,1),(m_WV)).xyz+eye_position.xyz;	
					above_water = testPos_world.y > water_height;
					// float3 testPos_water     = float3(testPos_world.x, water_height, testPos_world.z);				
					// float4 testPos_projected = mul(m_VP, float4(testPos_water, 1));					
					// float4 testPos_screen    = convert_to_screen_space(testPos_projected)/abs(testPos_projected.w);	
					// testPos = tex2Dlod( s_position , float4(testPos_screen.x, testPos_screen.y, 0, 0) );			
				}				
				
				objMat = mcls(testPos.a);
				
				if (testPos.z < WET_OBJMAXDISTREF)
				{				
					expectedDepth = (lastDepth + WET_OBJDEPTH); // * (lastDepth/e_barrier.z*100);
					maxObjDepth = P.z; // - 2*WET_OBJDEPTH;
					lastDepth = testPos.z;
					
					// Primary check: Change of material - Secondary: Depth
					//if ( water_height != 0 && above_water  ||  objMat != mat ) // || (testPos.z < expectedDepth  && testPos.z > maxObjDepth) )
					if (testPos.z < expectedDepth  && testPos.z > maxObjDepth)
					{						
						yMax = testY;
						obj = true;
						objDepth = testPos.z;	
						objPos = testPos;
						sample = i;
					}
				}				
			}
		}
		
		int testMat = objMat;
		
		// reflection point found, so refine it
		if (obj) // && e_barrier.z > 0.1)
		{
			for (int i=0; i <= (WET_OBJSTEP+5); i++)
			{
				if (obj)
				{
					float testY = yMax + yF * i;
					float4 testPos = tex2Dlod( s_position , float4(uv.x, testY, 0, 0) );
					
					bool below_water = false;
					
					if (water_height != 0)
					{
						float3 testPos_world     = mul(float4(testPos.xyz,1),(m_WV)).xyz+eye_position.xyz;	
						below_water = testPos_world.y < water_height;
						// float3 testPos_water     = float3(testPos_world.x, water_height, testPos_world.z);				
						// float4 testPos_projected = mul(m_VP, float4(testPos_water, 1));					
						// float4 testPos_screen    = convert_to_screen_space(testPos_projected)/abs(testPos_projected.w);	
						// testPos = tex2Dlod( s_position , float4(testPos_screen.x, testPos_screen.y, 0, 0) );			
					}
				
					testMat = mcls(testPos.a);
					
					expectedDepth = (lastDepth - WET_OBJDEPTH/WET_OBJSTEP) * 1; //e_barrier.z*100;					
					lastDepth = testPos.z;

					//if (abs(testPos.z - objDepth) < e_barrier.z*100)
					//if ( water_height != 0 && below_water  ||  objMat != mat && testMat == mat ) // || testPos.z >= expectedDepth || testMat != objMat)
					if (testPos.z >= expectedDepth)
					{										
						yMax = testY;						
						objDepth = testPos.z;
						objPos = testPos;
					}
					else obj = false;
				}
			}
			obj = true;
		}
		
		float yOld = yMax;
		
		// NEW: Redefine Y according to depth		
		if (objDepth > 0)
		{
			
			// calc ground world pos
			
			// float3 pos_world = eye_position;
			// float4 pos_projected = mul(m_VP, float4(pos_world, 1));	
			// float4 pos_screen = convert_to_screen_space(pos_projected)/abs(pos_projected.w);		
			// float  pos_diff = pos_world.y - pos_screen.y;
			
			//float  pos_diff = P.y - normalize(eye_direction).y*WET_YCORR; // P.y;
			objPos.xyz = mul(objPos.xyz,(float3x3)m_WV).xyz;			
			//objDepth = objPos.z;
			float d4 = distance(float2(0,0), objPos.xz);
			
			float pos_diff = mul(P,(float3x3)m_WV).y;
			
			
			float WET_YCORR2 = 0.8;  // (e_barrier.z*10);
			float WET_YCORR3 = 4.0f;			
			//float WET_YCORR4 = (e_barrier.y-0.5)*100 + (e_barrier.z-0.5)*100 * eye_position.y + pos_diff; //0.15f
			//float WET_YCORR4 = (e_barrier.y-0.5)*100 + (e_barrier.z-0.5)*100 * (pos_diff); //;+ (e_barrier.z-0.5)*100*pos_diff; //0.15f
			float WET_YCORR4 = (0.49005-0.5)*100 + (0.4925-0.5)*100 * (pos_diff);
			//float WET_YCORR4 = (0.4915-0.5)*100 + (0.5075-0.5)*100 * eye_position.y; //0.15f
			yMax = (WET_YCORR4/objDepth) + 0.5f + 0.5f*saturate(0.5f/pow(objDepth,1))*WET_YCORR3;// ;
			
			float ny = normalize(eye_direction).y;
			
			if (ny <= -0.6)
				WET_YCORR2 = 0.88;	// 0.90
			else if (ny > -0.6 && ny <= -0.55)
				WET_YCORR2 = 0.85;	// 0.87
			else if (ny > -0.55 && ny <= -0.5)
				WET_YCORR2 = 0.83;	// 0.84
			else if (ny > -0.5 && ny <= -0.4)
				WET_YCORR2 = 0.80;  // 0.82;
			else if (ny > -0.4 && ny <= -0.2)
				WET_YCORR2 = 0.79; //0.80;
			else
				WET_YCORR2 = 0.78;			
			
			yMax = yMax + ny*WET_YCORR2;
			
			if (ny < 0)
			{
				//yMax = (yOld + yMax) * 0.5f;
				float oldFact = abs(ny)*1.5;
				//yMax = ( oldFact * yOld + (1-oldFact)*yMax ) * 0.5f;
				yMax = lerp(yMax,yOld,oldFact);
			}
			
			
			//yMax = yMax - (pos_diff-P.y);
			//yMax = mul(float3(P.x, yMax, P.z),(float3x3)m_WV).y;
			
			//if (normalize(eye_direction).y > e_barrier.z*10)
			//	yMax = yOld;
			
			
			//float4 pos4 = tex2Dlod(s_position,float4(center.xy,0,0));
			//float3 wp4 = mul(objPos.xyz,(float3x3)m_WV).xyz;
			//float d4 = distance(float2(0,0), wp4.xz);
			
		}
		
		//float wave = e_barrier.z;
		float wave = WET_OBJWAVE; //WET_WAVE;

		//float newY = -(uv.y-yMax)+yMax + (sin(uv.y*100 / pow(dist_factor, 2) )*0.03); 
		float newY = -(uv.y-yMax)+yMax + (sin(P.y*100 * wind * wave / pow(dist_factor, 2) )*0.03); 
		
		// ANTI-TEST 1: Do not reflect onto reflected object (requires NOT USING ABS() above)
		if (newY > yMax || newY < 0)
			obj = false;

		if (obj) 
		{
			float4 ref_col = 0;
			
			// refract
			//rain_density = 1;
			//rain_density *= 100;
			//float WET_DISAMP2 = e_barrier.x * 100;
			//float WET_DISFRQ2 = e_barrier.y * 100;
			//float WET_DISPER2 = e_barrier.z * 100;
			float ref_amp = rain_density * WET_DISAMP; // / 2.f;
			float ref_frq = rain_density * WET_DISFRQ;
			float ref_per = rain_density * WET_DISPER; 
			float2 refraction = xrefract(float2(uv.x, newY), ref_amp, ref_frq, ref_per, pow(dist_factor, 0.5), 0);
			refraction = xrefract(refraction, ref_amp, ref_frq, ref_per, pow(dist_factor, 0.5), timers.x);			
			uv.x = refraction.x;
			newY = refraction.y;
			
			float4 ref_bloom = tex2Dlod( s_bloom , float4(uv.x, newY, 0, 0 ));
			
			if (mat != 1 || indoor_wet)	// buildings only when defined as wet
			{
				ref_col = tex2Dlod( s_image , float4(uv.x, newY, 0, 0 ));
				// mix between bloom and clean image according to wet amount and rain density
				//float WET_BLOOM = e_barrier.z;
				ref_col	  = lerp(ref_bloom, ref_col, saturate((1-WET_BLOOM)*refAmount - WET_RAINBLOOM*rain_density) );
			}
			else // dry but reflective buildings
			{
				ref_col = ref_bloom;		// s_bloom does not look so good here, so use the same for the moment ???????
			}
									
			float4 ref_pos = tex2Dlod( s_position , float4(uv.x, newY, 0, 0 ));
			int ref_mat = mcls(ref_pos.a);			
			
			// ANTI-TEST 2: Do not reflect beyond reflected object			
			expectedDepth = lastDepth + WET_OBJDSTCORR;
			if (ref_pos.z > expectedDepth)
				obj = false;    

			if (obj && ref_mat != 3 && ref_mat != 4 && ref_col.a < WET_MAXALPHA)	 // don't reflect trees and weapons
			{      
				//float ref_dist = pow(yMax/uv.y,WET_OBJFADE);   
				float ref_dist = pow(saturate(1-(float)sample/(float)WET_OBJSAMPLES),WET_OBJFADE)*10;   
				//ref_col.g = pow(ref_col.r,2) * 20; // debug
				
				refAmount  = saturate(10*refAmount * dist_factor * ref_dist);
				if (refAmount > WET_OBJMAXREF) refAmount = WET_OBJMAXREF;
				colAmount *= saturate(1-refAmount) * 1.8;			
				
				Color = colAmount * Color + refAmount * ref_col;
								
				//Color = colAmount * Color + refAmount * ref_col * dist_factor * ref_dist;
				
			}
		}
	}
		
	// debug
	//if (obj && e_barrier.z > 0.1 && abs(uv.y-yMax)<0.02)
	//	Color = float4(1,0,0,1);

	return Color;
}
#endif

float4 wetreflect(float4 Color, float2 uv, float wet_amount, float rain_density, float water_height, bool indoor_wet, bool indoor_ref) : COLOR
{	
	float4 OrigColor = Color;
	
	#ifdef WET_BUILDINGREF	
	indoor_ref = true;	
	#endif	
						
	float4 test_Color = tex2Dlod( s_image , float4(uv.xy,0,0));
	float4 viewspace_P = tex2Dlod(s_position, float4(uv,0,0));
	
	int mat = mcls(viewspace_P.a);
	
	// Freeze wet amount in reflective buildings (use constant value independently from rain)
	if (mat == 1 && indoor_ref) wet_amount = WET_REF_BUILD;
	
	float amount = 0;
	float ref_amount = 0;	
	//bool is_indoor_wet = (mat == 1 && indoor_wet);
	
	if (WET_DEBUGVAL > 0.0 || (test_Color.a < WET_MAXALPHA && wet_amount > 0) ) // alpha 0 = exclude sky! (plus particles, fences) -> avoid shader ghosting!
	{					
		float dist_factor = saturate(1/viewspace_P.z) * 1.0f; // wet amount = 1.0	
		
		//float wind = pow(wet_amount*10,3) * sin(timers.x * e_barrier.y * 1000) * pow(dist_factor, 3); // USE 0.0001 or lower // / dist_factor; // 1; //  ((e_kernel.x-0.4) * 100);		
		float wind = 1;		
		
		if (WET_DEBUGVAL > 0.1 || (mat == 2 || mat == 5 || mat == 6 || (mat == 1 && (indoor_wet || indoor_ref) ) ) ) // is_indoor_wet
		{	
			float calc_y        = uv.y - normalize(eye_direction).y*WET_YCORR;
			
			// compute normality / verticality			
			float3 P = float3(uv.x, calc_y , viewspace_P.z);	  /// UNPACK NORMALS ?????????????????????????????????????					

			float3 viewspace_N = tex2Dlod(s_normal, float4(uv,0,0)).xyz;
			float3 N = normalize(mul(viewspace_N,(float3x3)m_WV));

			float vert = pow( (1-N.r) * (1-N.b) * N.g, 2 ); // the more green the more vertical is the surface	
			
			// check color
			float4 check_col = lerp(tex2Dlod(s_image, float4(uv,0,0)), tex2Dlod(s_bloom, float4(uv,0,0)), WET_BLOOM_MIX);				// WAS only s_bloom before, looked better but did not render everything properly as wet (false negatives)			
			float lum = (test_Color.r + test_Color.g + test_Color.b) / 3;
			float check_lum = (check_col.r + check_col.g + check_col.b) / 3;
			
			//float col_factor = 1 ; e_barrier.z*10;
						
			//check_col = (((check_col / (L_hemi_color*col_factor)) / (Ldynamic_color*col_factor) ) / (float4(L_sun_color, 1)*col_factor))  / 3;	
			//check_col = check_col / (L_hemi_color*col_factor);
			
			float m = 0.08; //e_barrier.z;
			float s = 0.35; //e_barrier.y;
			float g = 0.35; //e_weights.z;
			float r = 0.46; //e_weights.y;
			
			float min_sun = 0.01;
			if (L_sun_color.r < min_sun && L_sun_color.g < min_sun && L_sun_color.b < min_sun)
			{
				//check_col = normalize(2 * L_hemi_color * check_col);
				check_col = (2 * L_hemi_color * check_col) * 9;
			}
			else
			{
				//check_col = normalize(2 * L_hemi_color * check_col / (e_barrier.z*10 * float4(L_sun_color, 1)) );
				//check_col = ( (2* check_col / (L_hemi_color)) / pow(float4(L_sun_color, 1), 0.5) );
				check_col = (2 * L_hemi_color * check_col) * 9 / pow(float4(L_sun_color, 1), 0.5) ;
				
				float hemi_lum = (L_hemi_color.r + L_hemi_color.g + L_hemi_color.b)/3;
				if (hemi_lum > WET_HEMILUM)
					r = 0.86;
				
				#define WET_COLOR_SATURATION float(0.8)  //Level of Grey. 0 is all grey (Black and White), 1 is no Grey (Why have this enabled?)					
				float gray=dot(check_col.xyz,float3(0.3,0.59,0.11));
				check_col.xyz=lerp(float3(gray,gray,gray),check_col.xyz,WET_COLOR_SATURATION+(dot(Ldynamic_color.xyz,float3(1,1,1))/3)*0);
			}
			
			//check_col.b *= e_barrier.z * 1000;
			check_col = normalize(normalize_blue(check_col))/2;
			
			float col_ratio = check_col.b / ( (check_col.r + check_col.g + check_col.b) / 3);
			
			float maxcof = (mat == 2) ? WET_MAXCOF : ( (mat == 1) ? WET_MAXCOF_BUILD : ( (mat == 5) ? WET_MAXCOF_VEHIC : WET_MAXCOF_METAL) );
			
			float amount_base = 1000 * maxcof * wet_amount; // TODO: Make configurable
			amount = saturate(((check_col.b-m)+0.01) * ((s-check_col.r)+0.01) * ((g-check_col.g)+0.01) * ((col_ratio-r)+0.01) * amount_base);
			
			float blue_amount = check_col.b * col_ratio * (1-check_col.r) * (1-check_col.g);				
			
			//float WET_NORMAL = 0;			
			//float WET_NORMALIN = e_barrier.z * 1000;
			//is_indoor_wet
			
			float min_vert = (mat == 1) ? WET_NORMALIN : WET_NORMAL;
			
			// add contrast	if enabled
			#ifdef WET_CONTRAST
			if (vert > min_vert && wet_amount > 0)
			{
				float3 highcontrast=Color.xyz*Color.xyz;
				Color.xyz=lerp(Color.xyz,highcontrast, WET_CONTRAST * maxcof * wet_amount * pow(blue_amount, WET_CONTRASTBLUE)); // * e_barrier.y * 1000 );
			}			
			#endif
			
			float sunColLum = (L_sun_color.r+L_sun_color.g+L_sun_color.b)/3;
						
			// add reflections if no sun OR color is matching OR disabled
			#ifdef WET_COLDETECT
			bool colordetection = true;
			#else
			bool colordetection = false;
			#endif
			
			if (WET_DEBUGVAL > 0.2 || (!colordetection || sunColLum < WET_SUNLUM || (check_col.b > m && check_col.r < s && check_col.g < g && col_ratio > r && viewspace_P.z < WET_MAXDIST && lum > WET_MINLUM)	) )
			{					
				// screen area check				
				//if (check_y > 0.4)
				if (true)
				{
				
					if (e_kernel.x == 0.7)
						Color = decolor(Color); 					
						
					//if (e_barrier.z > 0)
					if (WET_DEBUGVAL > 0.3 || (vert > min_vert) )  // was: min_vert
					{
						float newX = uv.x;						
						float newY = (rain_density < WET_RAINTHR) ?
							   (-(P.y-0.5)+0.5 + (sin(P.y*100 * wind * WET_WAVE / pow(dist_factor, 2) )*0.03)) : 
							abs(-(P.y-0.5)+0.5 + (sin(P.y*100 * wind * WET_WAVE / pow(dist_factor, 2) )*0.03)) ; 
						if (newY < 0)	// Correction for negative reflection coordinates
						{
							newY = abs(newY + (sin(P.y*100 * pow(abs(newY*100),2) * WET_WAVENEG / pow(dist_factor, 2) )*0.03));
							amount -= pow(abs(newY),0.5);
						}
						if (newY > 1)
							newY = 2 - newY;
							
						// add refraction ONLY ON TERRAIN (exclude indoor stuff such as metallic tables, weapons etc.)
						if (mat == 2)
						{
							//float WET_DISAMP = e_barrier.x * 100;
							//float WET_DISFRQ = e_barrier.y * 100;
							//float WET_DISPER = e_barrier.z * 100;
							float ref_amp = rain_density * WET_DISAMP;
							float ref_frq = rain_density * WET_DISFRQ;
							float ref_per = rain_density * WET_DISPER; 
							float2 refraction = xrefract(float2(newX, newY), ref_amp, ref_frq, ref_per, pow(dist_factor, 0.5), timers.x);
							//wind = newY-refraction.y;		// used for object reflections later														
							//Color = tex2D(s_image, float2(uv.x+(refraction.x-newX), uv.y+(refraction.y-newY))); 
							//Color = tex2D(s_image, float2(refraction.x, refraction.y)); 
							newX = refraction.x;
							newY = refraction.y;
						}
						
						//float4 bloom_col = tex2Dlod(s_bloom, float4(uv,0,0));
						float4 ref_bloom = tex2Dlod(s_bloom, float4(newX, newY, 0, 0 )); 
						float4 ref_col = 0;
												
						if (mat != 1 || indoor_wet)	// buildings only when defined as wet
						{
							// sample reflection color for wet outdoor and wet indoor
							ref_col   = tex2Dlod(s_image, float4(newX, newY, 0, 0 )); 
							
							// mix between bloom and clean image according to wet amount and rain density
							//float WET_BLOOM = e_barrier.z;
							ref_col	  = lerp(ref_bloom, ref_col, saturate((1-WET_BLOOM)*wet_amount - WET_RAINBLOOM*rain_density) );
						}
						else // mat == 1 && indoor_ref
						{
							// sample reflection color for dry indoor - bloom only
							ref_col   = ref_bloom;
						}						

						float ref_lum = (ref_col.r + ref_col.g + ref_col.b) * ref_col.a / 3;	
						
						float ref_vert = vert;

						if (mat != 1 || indoor_wet || indoor_ref)	// buildings only when defined as wet or reflective
						{
							// Set/add water color
							float3 water = Color.rgb * saturate(1-dist_factor * WET_BASE * 2); // What the hell did I intend with this??? --> Seems to darken ground the more wet it is  
							float3 water_color = amount * water + (1-amount) * Color.rgb;   
							Color.rgb = water_color;   

							// Make different use of verticality							
							if (wet_amount > WET_NORMALFLOOD && ref_vert < wet_amount) ref_vert = wet_amount;     // WHAT THE HELL??? LOOK AT THAT WEIRD LINE OF CODE!!!							
						}
						
						// Amplify reflected color according to wet amount, luminance, and verticality
						ref_amount = amount * WET_REF * saturate(pow(WET_LUM*check_lum, WET_LUMCOF)) * ref_vert; // vert;												
						
						// Apply amount to reflected color
						ref_col *= ref_amount * dist_factor;

						// add bloom if required (disabled by default)
						//float bloom = ((e_kernel.x-0.4) * 100); 
						if (vert > min_vert)  
							ref_col = (ref_col + WET_BLOOM_COL * ref_bloom + WET_BLOOM_REF * ref_bloom);
							
						// DEBUG - REMOVE THE FOLLOWING LINE AFTER TESTING OBJECT REFLECTIONS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						//ref_col = 0;

						#ifdef WET_COLFILTER
						if (mat == 1 && indoor_wet)	// buildings only when defined as wet
						{
							float gray=dot(Color.xyz,float3(0.3,0.59,0.11));							
							Color = 1 * float4(lerp(float3(gray,gray,gray),Color.xyz,0.1) * float3(0.1,0.1,0),1) +  ref_col * WET_COLFILTER;
						}
						else
							Color = 1 * Color  +  ref_col;
						#else
						Color = 1 * Color  +  ref_col;
						#endif
						
						// Add object reflections
						#ifdef WET_OBJREFLECT
						Color = objreflect(Color, viewspace_P, mat, indoor_wet, indoor_ref, uv, dist_factor, wind, WET_OBJMAXCOL, ref_amount, rain_density, water_height);
						#endif
						
						// Add light reflections
						//Color += light_reflection(float2(newX, newY), ref_amount, ref_amp, ref_frq, ref_per, dist_factor);
						
					}										
					
					Color.a = 0.1;  				
					
					 
				}
				else	// not inside affected screen area
				{
					Color.a = 1.0;			
				}		
			}
			else	// color doesn't match
			{
				Color.a = 1.0;
				//Color.r *= 5;
			}			
		}	
		else	// not affected material
		{			
			Color.a = 1.0; 
		}		
		
	}
	
	/* BLUE FILTER (POST PROCESS FOR WET SURFACES ASPHALT BLUE */
	if (ref_amount == 0)
	{
		if (test_Color.a < WET_MAXALPHA && mat == 2) // Apply to terrain only and not to sky
		{
			float blue_diff = test_Color.b*2 - test_Color.r - test_Color.g;
			if (blue_diff > WET_BLUEDIFF)
			{
				//Color.b = (test_Color.r + test_Color.g) / 2 + blue_diff * (10*(e_barrier.z-0.5));
				
				//if (ref_amount == 0)
					Color.b *= WET_BLUECORR;
				//else
					
			}
		}
	}
	else
	{
		// If wet, decrease saturation accordingly
		float gray=dot(Color.xyz,float3(0.3,0.59,0.11));
		//Color.xyz=lerp(float3(gray,gray,gray),Color.xyz,ref_amount*e_barrier.z*10+(dot(Ldynamic_color.xyz,float3(1,1,1))/3)*COLOR_SAT_SUN_COEF);
		Color.xyz=lerp(float3(gray,gray,gray),Color.xyz,WET_SAT);  // e_barrier.z*10
	}
	
	return Color;
}