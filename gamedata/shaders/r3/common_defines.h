#ifndef	common_defines_h_included
#define	common_defines_h_included

//////////////////////////////////////////////////////////////////////////////////////////
// Defines                                		//
#define def_gloss       float(2.f /255.f)
#define def_aref        float(200.f/255.f)
#define def_dbumph      float(0.333f)
#define def_virtualh    float(0.05f)              // 5cm
#define def_distort     float(0.05f)             // we get -0.5 .. 0.5 range, this is -512 .. 512 for 1024, so scale it
#define def_hdr         float(9.h)         		// hight luminance range float(3.h)
#define def_hdr_clip	float(0.75h)        		//

#define	LUMINANCE_VECTOR	float3(0.3f, 0.38f, 0.22f)

//////////////////////////////////////////////////////////////////////////////////////////
#ifndef SMAP_size
#define SMAP_size        1024
#endif
#define PARALLAX_H 0.02
#define parallax float2(PARALLAX_H, -PARALLAX_H/2)
//////////////////////////////////////////////////////////////////////////////////////////

#define USE_GRASS_WAVE								// включить "волны" от ветра по траве
#ifdef USE_TRIP
#define GRASS_WAVE_FREQ 	float(1.5)				// частота появления волн
#define GRASS_WAVE_POWER 	float(3)
#else
#define GRASS_WAVE_FREQ 	float(0.35)				// частота появления волн
#define GRASS_WAVE_POWER 	float(1)				// "яркость", заметность волн
#endif

#define USE_SUNMASK
//#define USE_TRIPPY

#endif	//	common_defines_h_included
