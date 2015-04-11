#ifndef        COMMON_H
#define        COMMON_H

// #define USE_SUPER_SPECULAR

#include "shared\common.h"
//////////////////////////////////////////////////////////////////////////////////////////
// *** options

// #define DBG_TEST_NMAP
// #define DBG_TEST_NMAP_SPEC
// #define DBG_TEST_SPEC
// #define DBG_TEST_LIGHT
// #define DBG_TEST_LIGHT_SPEC

// #define USE_GAMMA_22
// #define USE_SJITTER
// #define USE_SUNFILTER
// #define USE_FETCH4
// #define USE_MBLUR                	//- HW-options defined
// #define USE_HWSMAP                	//- HW-options defined

// #define USE_HWSMAP_PCF				//- nVidia GF3+, R600+

// #define USE_BRANCHING        		//- HW-options defined
// #define USE_VTF                		//- HW-options defined, VertexTextureFetch
// #define FP16_FILTER                	//- HW-options defined
// #define FP16_BLEND                	//- HW-options defined
//
// #define USE_PARALLAX                	//- shader defined
// #define USE_TDETAIL                	//- shader defined
// #define USE_LM_HEMI                	//- shader defined
// #define USE_DISTORT                	//- shader defined
#define USE_SUNMASK                		//- shader defined
// #define DBG_TMAPPING

#define USE_GRASS_WAVE								// включить "волны" от ветра по траве
#ifdef USE_TRIP
#define GRASS_WAVE_FREQ 	float(1.5)				// частота появления волн
#define GRASS_WAVE_POWER 	float(3)
#else
#define GRASS_WAVE_FREQ 	float(0.35)				// частота появления волн
#define GRASS_WAVE_POWER 	float(1)				// "яркость", заметность волн
#endif
//#define SS_DUST
//#define USE_TRIPPY
#define SKY_DEPTH float(10000.f)
#define SKY_EPS float(0.001)
//#define SWARTZ
half is_sky(float depth)		{return step(abs(depth - SKY_DEPTH), SKY_EPS);}
	float4	decode_normal			(float4 _N)
	{
		float2 enc = _N.xy;

		float2 fenc = enc*4-2;
		float f = dot(fenc,fenc);
		float g = sqrt(1-f/4);

		float4 N = _N;
   
		N.xy = fenc*g;
		N.z = 1-f/2;
		return N;
	}
	//float4	get_full_normal			(float2 tc)	{return decode_normal(tex2D(s_normal, tc));}
	//float3	get_xyz_normal_fast		(float2 tc)	{return tex2Dlod(s_normal, float4(tc,0,0)).xyz;}  
int mcls(float mcol)
{
	mcol = (mcol + 0.25) / 4.f;
	if      (mcol < 0.14)	// RGB 28 - Concrete walls, stone/brick buildings, NPCs, mutants, suits
		return 1;
	else if (mcol < 0.16)	// RGB 37 - Terrain, grass, water, metal cases, wood crates, signs, plastics, medkits, food, TVs, bulbs/lamps, some doors
		return 2;
	else if (mcol < 0.22)	// RGB 52 - Wood, bushes, trees, glass bottles
		return 3;
	else if (mcol < 0.29)	// RGB 70 - Weapons, binoc, ammo, ammo boxes
		return 4;
	else if (mcol < 0.33)	// RGB 82 - Vehicles, steel boxes, gas cans, fuel cans, barrels
		return 5;
	else if (mcol < 0.36)	// RGB 87 - Metallic structures/objects
		return 6;
	else
		return -1;
}
//	float3	get_xyz_normal			(float2 tc)	
	//{
	//	float4 _N = get_full_normal(tc);
	//	return _N.xyz;
	//}
float4 proj_to_screen(float4 proj)
{
	float4 screen = proj;
	screen.x = (proj.x + proj.w);
	screen.y = (proj.w - proj.y);
	screen.xy *= 0.5;
	return screen;
}
float4 screen_to_proj(float2 screen, float z)
{
	float4 proj;
	proj.w = 1.0;
	proj.z = z;
	proj.x = screen.x*2 - proj.w;
	proj.y = -screen.y*2 + proj.w;
	return proj;
}
float is_in_range(float3 args)
{
	float mn = (args.x > args.y) ? 1: 0;
	float mx = (args.z > args.x) ? 1: 0;
	return mn*mx;
}

	#define SSDO_RADIUS float(0.04)				// радиус семплирования в игровых метрах. Чем выше, тем более заметен эффект, но тем больше нежелательных наложений теней
	#define SSDO_BLEND_FACTOR float(1.5)		// сила эффекта. 0 - отстутствует, чем больше значение, тем заметней эффект.
	#define SSDO_GRASS_TUNING float(1.0)		// коррекция затенения травы. Чем больше, тем меньше затеняется.
	#define SSDO_DISCARD_THRESHOLD float(1.0)	// максимальная разница в глубине пикселей, при которой пиксель еще учитывается в расчете затенения. Уменьшение убирает "шлейф" в некоторых случаях.
	#define SSDO_USE_INDIRECT_BOUNCES			// учет "светимости" семплов. Некая имитация вторичного освещения. Уменьшает производительность, зато тени не черные, а более реалистичные.
	#define SSDO_COLOR_BLEEDING float(15.0)		// сила цвета семплов. Дает более цветные тени, но уменьшает интенсивность эффекта в целом. Для компенсации увеличивайте SSDO_BLEND_FACTOR.
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef SMAP_size
#define SMAP_size        1024
#endif
#define PARALLAX_H 0.02
#define parallax float2(PARALLAX_H, -PARALLAX_H/2)

#ifdef        USE_R2_STATIC_SUN
#  define xmaterial half(1.0h/4.h)
#else
#  define xmaterial half(L_material.w)
#endif

//////////////////////////////////////////////////////////////////////////////////////////
uniform half4                hemi_cube_pos_faces;
uniform half4                hemi_cube_neg_faces;
uniform half4                L_material;                            // 0,0,0,mid
uniform half4                Ldynamic_color;                      // dynamic light color (rgb1)        - spot/point
uniform half4                Ldynamic_pos;                       // dynamic light pos+1/range(w) - spot/point
uniform half4                Ldynamic_dir;                        // dynamic light direction         - sun
uniform float4 c_screen;
uniform half4                J_direct        [6];
uniform half4                J_spot                [6];

half          calc_fogging               (half4 w_pos)      { return dot(w_pos,fog_plane);         }
half2         calc_detail                (half3 w_pos)      {
        float                 dtl        = distance                (w_pos,eye_position)*dt_params.w;
                              dtl        = min              (dtl*dtl, 1);
        half                  dt_mul     = 1  - dtl;        // dt*  [1 ..  0 ]
        half                  dt_add     = .5 * dtl;        // dt+  [0 .. 0.5]
        return                half2      (dt_mul,dt_add);
}
float3         calc_reflection     (float3 pos_w, float3 norm_w)
{
    return reflect(normalize(pos_w-eye_position), norm_w);
}

float3        calc_sun_r1                (float3 norm_w)    { return L_sun_color*saturate(dot((norm_w),-L_sun_dir_w));                 }
float3        calc_model_hemi_r1         (float3 norm_w)    { return max(0,norm_w.y)*L_hemi_color;                                         }
float3        calc_model_lq_lighting     (float3 norm_w)    { return L_material.x*calc_model_hemi_r1(norm_w) + L_ambient + L_material.y*calc_sun_r1(norm_w);         }

//////////////////////////////////////////////////////////////////////////////////////////
struct         v_static                {
        float4      P                	: POSITION;                        // (float,float,float,1)
        float4      Nh                	: NORMAL;                // (nx,ny,nz,hemi occlusion)
        float4      T                 	: TANGENT;                // tangent
        float4      B                 	: BINORMAL;                // binormal
        float2      tc                	: TEXCOORD0;        // (u,v)
        float2      lmh                	: TEXCOORD1;        // (lmu,lmv)
        float4      color               : COLOR0;                        // (r,g,b,dir-occlusion)
};

struct         v_tree               	{
        float4      P                	: POSITION;                // (float,float,float,1)
        float4      Nh                	: NORMAL;                // (nx,ny,nz)
        float3      T                 	: TANGENT;                // tangent
        float3      B                 	: BINORMAL;                // binormal
        float4      tc                	: TEXCOORD0;        // (u,v,frac,???)
};

struct         v_model                 	{
        float4      P                   : POSITION;                // (float,float,float,1)
        float3      N                	: NORMAL;                // (nx,ny,nz)
        float3      T                	: TANGENT;                // (nx,ny,nz)
        float3      B                	: BINORMAL;                // (nx,ny,nz)
        float2      tc                	: TEXCOORD0;        // (u,v)
};

struct        v_detail                    {
        float4      pos                : POSITION;                // (float,float,float,1)
        int4        misc        : TEXCOORD0;        // (u(Q),v(Q),frac,matrix-id)
};

#ifdef  USE_HWSMAP
struct         v_shadow_direct_aref
{
        float4      hpos:        POSITION;       // Clip-space position         (for rasterization)
        float2      tc0:        TEXCOORD1;       // Diffuse map for aref
};
struct         v_shadow_direct
{
        float4      hpos:        POSITION;       // Clip-space position         (for rasterization)
};
#else
struct         v_shadow_direct_aref
{
        float4      hpos:        POSITION;       // Clip-space position         (for rasterization)
        float       depth:         TEXCOORD0;     // Depth
        float2      tc0:        TEXCOORD1;       // Diffuse map for aref
};
struct         v_shadow_direct
{
        float4      hpos:        POSITION;       // Clip-space position         (for rasterization)
        float       depth:         TEXCOORD0;     // Depth
};


#endif

//////////////////////////////////////////////////////////////////////////////////////////
struct         p_bumped        {
        float4      hpos        : POSITION;
#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
        float4            tcdh        : TEXCOORD0;        // Texture coordinates,         w=sun_occlusion
#else
        float2            tcdh        : TEXCOORD0;        // Texture coordinates
#endif
        float4      position        : TEXCOORD1;        // position + hemi
        half3       M1                : TEXCOORD2;        // nmap 2 eye - 1
        half3       M2                : TEXCOORD3;        // nmap 2 eye - 2
        half3       M3                : TEXCOORD4;        // nmap 2 eye - 3
#ifdef USE_TDETAIL
        float2      tcdbump     	: TEXCOORD5;        // d-bump
    #ifdef USE_LM_HEMI
        float2      lmh             : TEXCOORD6;        // lm-hemi
    #endif
#else
    #ifdef USE_LM_HEMI
        float2      lmh             : TEXCOORD5;        // lm-hemi
    #endif
#endif
};
//////////////////////////////////////////////////////////////////////////////////////////
struct         p_flat                  {
        float4                 hpos        : POSITION;
#if ((defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)) || defined(USE_GRASS_WAVE))
    float4                    tcdh        : TEXCOORD0;        // Texture coordinates,         w=sun_occlusion
#else
    float2                    tcdh        : TEXCOORD0;        // Texture coordinates
#endif
        float4                position        : TEXCOORD1;        // position + hemi
        half3                N                : TEXCOORD2;        // Eye-space normal        (for lighting)
  #ifdef USE_TDETAIL
        float2                tcdbump                : TEXCOORD3;        // d-bump
    #ifdef USE_LM_HEMI
        float2         lmh      : TEXCOORD4;        // lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
        float2         lmh      : TEXCOORD3;        // lm-hemi
    #endif
  #endif
};

//////////////////////////////////////////////////////////////////////////////////////////
struct                  f_deffer        		{
        half4           position        		: COLOR0;        // px,py,pz, m-id
        half4           Ne                		: COLOR1;        // nx,ny,nz, hemi
        half4       	C                		: COLOR2;        // r, g, b,  gloss
};
//////////////////////////////////////////////////////////////////////////////////////////
struct  				p_screen                {
        float4          hpos               		: POSITION;
        float2          tc0                		: TEXCOORD0;        // Texture coordinates         (for sampling maps)
};
//////////////////////////////////////////////////////////////////////////////////////////
// Geometry phase / deferring               	//
uniform sampler2D       s_base;             	//
uniform sampler2D       s_bump;             	//
uniform sampler2D       s_bumpX;                //
uniform sampler2D       s_detail;               //
uniform sampler2D       s_detailBump;           //
uniform sampler2D       s_detailBumpX;          //	Error for bump detail
uniform sampler2D       s_bumpD;                //
uniform sampler2D       s_hemi;             	//

uniform sampler2D       s_mask;                	//

uniform sampler2D       s_dt_r;                	//
uniform sampler2D       s_dt_g;                	//
uniform sampler2D       s_dt_b;                	//
uniform sampler2D       s_dt_a;                	//

uniform sampler2D       s_dn_r;                	//
uniform sampler2D       s_dn_g;                	//
uniform sampler2D       s_dn_b;                	//
uniform sampler2D       s_dn_a;                	//

//////////////////////////////////////////////////////////////////////////////////////////
// Lighting/shadowing phase                     //
uniform sampler2D       s_depth;                //
uniform sampler2D       s_position;             //
uniform sampler2D       s_normal;               //
uniform sampler         s_lmap;             	// 2D/cube projector lightmap
uniform sampler3D       s_material;             //
uniform sampler1D       s_attenuate;        	//
//////////////////////////////////////////////////////////////////////////////////////////
// Combine phase                                //
uniform sampler2D       s_diffuse;              // rgb.a = diffuse.gloss
uniform sampler2D       s_accumulator;      	// rgb.a = diffuse.specular
uniform sampler2D       s_generic;              //
uniform sampler2D       s_bloom;                //
uniform sampler         s_image;                // used in various post-processing
uniform sampler2D       s_tonemap;              // actually MidleGray / exp(Lw + eps)
//////////////////////////////////////////////////////////////////////////////////////////
// Defines                                		//
#define def_gloss       half(2.f /255.f)
#define def_aref        half(200.f/255.f)
#define def_dbumph      half(0.333f)
#define def_virtualh    half(0.05f)              // 5cm
#define def_distort     half(0.05f)             // we get -0.5 .. 0.5 range, this is -512 .. 512 for 1024, so scale it
#define def_hdr         half(9.h)         		// hight luminance range half(3.h)
#define def_hdr_clip	half(0.75h)        		//

//////////////////////////////////////////////////////////////////////////////////////////
float	get_depth_fast			(float2 tc)	{return tex2Dlod(s_position, float4(tc,0,0)).z;}
	float4	get_full_normal			(float2 tc)	{return decode_normal(tex2D(s_normal, tc));}
	float4	get_full_normal_fast	(float2 tc)	{return decode_normal(tex2Dlod(s_normal, float4(tc,0,0)));}
	float4	get_full_normal_proj	(float4 tc)	{return decode_normal(tex2Dproj(s_normal, tc));}
	float3	get_xyz_normal			(float2 tc)	
	{
		float4 _N = get_full_normal(tc);
		return _N.xyz;
	}
	float3	get_xyz_normal_fast		(float2 tc)	
	{
		float4 _N = get_full_normal_fast(tc);
		return _N.xyz;
	}
	float3	get_xyz_normal_proj		(float4 tc)	
	{
		float4 _N = get_full_normal_proj(tc);
		return _N.xyz;
	}
#define	LUMINANCE_VECTOR                 half3(0.3f, 0.38f, 0.22f)
void        tonemap              (out half4 low, out half4 high, half3 rgb, half scale)
{
        rgb     =      	rgb*scale       ;

		const float fWhiteIntensity = 1.7;

		const float fWhiteIntensitySQR = fWhiteIntensity*fWhiteIntensity;
#ifdef	USE_BRANCHING		// ps_3_0
        //low		=       rgb.xyzz		;

		low		=	( (rgb*(1+rgb/fWhiteIntensitySQR)) / (rgb+1) ).xyzz;

        high	=		low/def_hdr		;        // 8x dynamic range
#else
        low		=       half4           ( ( (rgb*(1+rgb/fWhiteIntensitySQR)) / (rgb+1) ),           0 )	;
        high	=       half4       	(rgb/def_hdr,   0 )	;		// 8x dynamic range
#endif

/*
	rgb		=	rgb*scale;

	low		=	rgb.xyzz;
	high	=	low/def_hdr;	// 8x dynamic range
*/

//		low		= 	half4	(rgb, 0);
//		rgb		/=	def_hdr	;
//		high	= 	half4	(rgb, dot(rgb,0.333f)-def_hdr_clip)		;
}
half4		combine_bloom        (half3  low, half4 high)	{
        return        half4(low + high*high.a, 1.h);
}

float3	v_hemi        	(float3 n)                        	{        return L_hemi_color*(.5f + .5f*n.y);                   }
float3	v_hemi_wrap     (float3 n, float w)                	{        return L_hemi_color*(w + (1-w)*n.y);                   }
float3	v_sun           (float3 n)                        	{        return L_sun_color*dot(n,-L_sun_dir_w);                }
float3	v_sun_wrap      (float3 n, float w)                	{        return L_sun_color*(w+(1-w)*dot(n,-L_sun_dir_w));      }
half3   p_hemi          (float2 tc)                         {
//        half3        	t_lmh         = tex2D             	(s_hemi, tc);
//        return  dot     (t_lmh,1.h/4.h);
        half4        	t_lmh         = tex2D             	(s_hemi, tc);
        return t_lmh.a;
}

half   get_hemi( half4 lmh)
{
	return lmh.a;
}

half   get_sun( half4 lmh)
{
	return lmh.g;
}

//	contrast function
half Contrast(half Input, half ContrastPower)
{
     //piecewise contrast function
     bool IsAboveHalf = Input > 0.5 ;
     half ToRaise = saturate(2*(IsAboveHalf ? 1-Input : Input));
     half Output = 0.5*pow(ToRaise, ContrastPower);
     Output = IsAboveHalf ? 1-Output : Output;
     return Output;
}

#define FXPS technique _render{pass _code{PixelShader=compile ps_3_0 main();}}
#define FXVS technique _render{pass _code{VertexShader=compile vs_3_0 main();}}

#endif
