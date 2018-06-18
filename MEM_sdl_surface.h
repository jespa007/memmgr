#pragma once

//  Handeled  SDL...





#include   <SDL2/SDL.h>
#include   <SDL2/SDL_image.h>
#include   <SDL2/SDL2_rotozoom.h>
#include   <SDL2/SDL2_gfxPrimitives.h>
#include 	<SDL2/SDL_net.h>

//------------------------------------------------------------

//#include                    <SDL2/SDL_ttf.h>
/*
 *  16bpp
 * 255,0,0 = 11111000 00000000
0,255,0 = 00000111 11100000
0,0,255 = 00000000 00011111
 */







#define IS__cmf__SDL_SURFACE_RGB(sdl_surface)\
(((sdl_surface)->format->Rmask)&0x000000ff)&&\
(((sdl_surface)->format->Gmask)&0x0000ff00)&&\
(((sdl_surface)->format->Bmask)&0x00ff0000)\

#define IS__cmf__SDL_SURFACE_BGR(sdl_surface)\
(((sdl_surface)->format->Bmask)&0x000000ff)&&\
(((sdl_surface)->format->Gmask)&0x0000ff00)&&\
(((sdl_surface)->format->Rmask)&0x00ff0000)\

void SDL_html2rgb(Uint32 html, SDL_Color & c);


//  All  sdl  memory/sdl_image/sdl_gfx/sdl_ttf  etc...



#if defined(__DEBUG__) && defined(__MEMMANAGER__)

void  MEMSDL_SURFACE_Init();
void  MEMSDL_SURFACE_DeInit();
void  MEMSDL_DeAllocateSurfaces();


int
__cmf__SDL_Init(
		const  char  *module,const  char  *function,  int  line,
		Uint32 flags);

int
__cmf__SDL_InitSubSystem(
		const  char  *module,const  char  *function,  int  line,
		Uint32 flags);

SDL_Surface  *MEMSDL_SDL__IMAGE_Load(const  char  *name,  bool  *is_new,  bool  by_reference  =  true,bool fast_hw = false);

//SDL_CreateRGBSurface
SDL_Surface		*__cmf__SDL__CreateRGBSurface(const  char  *module,const  char  *function,  int  line,
									      Uint32  flags,  int  width,  int  height,  int  depth,  Uint32  Rmask,  Uint32  Gmask,  Uint32  Bmask,  Uint32  Amask);

SDL_Surface		*__cmf__SDL__CreateRGBSurfaceFrom(const  char  *module,const  char  *function,  int  line,
										      void  *pixels,  int  width,  int  height,  int  depth,  int  pitch,  Uint32  Rmask,  Uint32  Gmask,  Uint32  Bmask,  Uint32  Amask);

SDL_Surface		*__cmf__SDL__CopySurface(const  char  *module,const  char  *function,  int  line,
							SDL_Surface *src, SDL_Rect *src_rect);

SDL_Surface		*__cmf__SDL__LoadBMP(const  char  *module,const  char  *function,  int  line,
							    const  char  *file);

SDL_Surface		*__cmf__SDL__ConvertSurface(const  char  *module,const  char  *function,  int  line,
                                                              SDL_Surface  *src,  SDL_PixelFormat  *fmt,  Uint32  flags);

int  __cmf__SDL__BlitSurface(SDL_Surface  *src,  SDL_Rect  *srcrect,  SDL_Surface  *dst,  SDL_Rect  *dstrect);
void  __cmf__SDL__UpdateRects(SDL_Surface  *screen,  int  numrects,  SDL_Rect  *rects);

void			  __cmf__SDL__FreeSurface(const  char  *module,const  char  *function,  int  line,
                                                                    void  *surface);


SDL_Surface * __cmf__SDL__GetMovieSurface(const  char  *module,const  char  *function,  int  line
                                                                    );

void __cmf__SDL__SetMovieSurface(const  char  *module,const  char  *function,  int  line,
                                                                    SDL_Surface  *surface);


SDL_Surface                        	*__cmf__SDL__NewSurface(const  char  *module,const  char  *function,  int  line,int  width,  int  height);
SDL_Surface                        	*__cmf__SDL__NewSurfaceBPP(const  char  *module,const  char  *function,  int  line,int  width,  int  height, int bpp);
SDL_Surface  		*__cmf__SDL__NewSurfaceFromSurface(const  char  *module,const  char  *function,  int  line,int  width,  int  height,  SDL_Surface  *src);

int     SDL_SaveJPG(SDL_Surface *pSurface, const char* sFileName, int iQuality=-1);
//  SDL_image



SDL_Surface		*__cmf__IMG_Load(const  char  *module,const  char  *function,  int  line,  const  char  *file);
SDL_Surface		*__cmf__IMG_Load_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src,  int  freesrc);
SDL_Surface		*__cmf__IMG_LoadTyped_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src,  int  freesrc,  char  *type);
SDL_Surface		*__cmf__IMG_LoadBMP_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadPNM_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadXPM_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadXCF_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadPCX_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadGIF_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadJPG_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadTIF_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadPNG_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadTGA_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_LoadLBM_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src);
SDL_Surface		*__cmf__IMG_ReadXPMFromArray(const  char  *module,const  char  *function,  int  line,  char  **xpm);


/*
SDL_Surface		*__cmf__TTF_RenderText_Solid		(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  char  *text,  SDL_Color  fg);

SDL_Surface		*__cmf__TTF_RenderUTF8_Solid		(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  char  *text,SDL_Color  fg);

SDL_Surface		*__cmf__TTF_RenderUNICODE_Solid	(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  Uint16  *text,SDL_Color  fg);

SDL_Surface		*__cmf__TTF_RenderGlyph_Solid		(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  Uint16  ch,  SDL_Color  fg);

SDL_Surface		*__cmf__TTF_RenderText_Shaded		(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  char  *text,SDL_Color  fg,  SDL_Color  bg);

SDL_Surface		*__cmf__TTF_RenderUTF8_Shaded		(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  char  *text,SDL_Color  fg,  SDL_Color  bg);

SDL_Surface		*__cmf__TTF_RenderUNICODE_Shaded	(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  Uint16  *text,SDL_Color  fg,  SDL_Color  bg);

SDL_Surface		*__cmf__TTF_RenderGlyph_Shaded	(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  Uint16  ch,  SDL_Color  fg,SDL_Color  bg);

SDL_Surface		*__cmf__TTF_RenderText_Blended	(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  char  *text,SDL_Color  fg);

SDL_Surface		*__cmf__TTF_RenderUTF8_Blended	(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  char  *text,SDL_Color  fg);

SDL_Surface		*__cmf__TTF_RenderUNICODE_Blended	(const  char  *module,const  char  *function,  int  line,
											  TTF_Font  *font,  const  Uint16  *text,SDL_Color  fg);

SDL_Surface		*__cmf__TTF_RenderGlyph_Blended	(const  char  *module,const  char  *function,  int  line,

TTF_Font  		*font,  Uint16  ch,  SDL_Color  fg);

										  
*/


//  SDL_gfx..
SDL_Surface		*__cmf__rotozoomSurface    (const  char  *module,const  char  *function,  int  line,
									SDL_Surface  *  src,  double  angle,  double  zoom  ,                              int  smooth);

SDL_Surface		*__cmf__rotozoomSurfaceXY(const  char  *module,const  char  *function,  int  line,
									SDL_Surface  *  src,  double  angle,  double  zoomx,  double  zoomy,  int  smooth);

SDL_Surface		*__cmf__zoomSurface            (const  char  *module,const  char  *function,  int  line,
									SDL_Surface  *  src,  double  zoomx,  double  zoomy,                              int  smooth);



//------------------------------------------------------------------------------------------------------------------------------------------------------------v


//SDL  redefinitions
#ifdef  SDL_CreateRGBSurface
#undef  SDL_CreateRGBSurface
#endif


#define SDL_Init(flags)\
		__cmf__SDL_Init(__FILE__,  __FUNCTION__,__LINE__, flags)

#define  SDL_InitSubSystem(flags)\
		__cmf__SDL_InitSubSystem(__FILE__,  __FUNCTION__,__LINE__, flags)


#define  SDL_CreateRGBSurface(flags,  width,  height,  depth,  Rmask,  Gmask,  Bmask,  Amask)\
__cmf__SDL__CreateRGBSurface(__FILE__,  __FUNCTION__,__LINE__,flags,  width,  height,  depth,  Rmask,  Gmask,  Bmask,  Amask)

#define SDL_GetMovieSurface\
__cmf__SDL__GetMovieSurface(__FILE__,  __FUNCTION__,__LINE__)


#define SDL_SetMovieSurface(srf)\
__cmf__SDL__SetMovieSurface(__FILE__,  __FUNCTION__,__LINE__,srf)


#define    SDL_CreateRGBSurfaceFrom(pixels,  width,  height,  depth,  pitch,  Rmask,  Gmask,  Bmask,  Amask)\
                __cmf__SDL__CreateRGBSurfaceFrom(__FILE__,  __FUNCTION__,__LINE__,  pixels,  width,  height,  depth,  pitch,  Rmask,  Gmask,  Bmask,  Amask)

#define SDL_CopySurface(src, rect_src )\
		__cmf__SDL__CopySurface(__FILE__,  __FUNCTION__,__LINE__,  src,  rect_src)

//  Undefine  SDL_LoadBMP
#ifdef  SDL_LoadBMP
#undef  SDL_LoadBMP
#endif      

#define    SDL_LoadBMP(file)\
                __cmf__SDL__LoadBMP(__FILE__,  __FUNCTION__,  __LINE__,  file)

#define    SDL_ConvertSurface(src,  fmt,  flags)\
                __cmf__SDL__ConvertSurface(__FILE__,  __FUNCTION__,  __LINE__,  src,  fmt,  flags)

#define  SDL_NewSurfaceBPP(width,height,bpp)\
                __cmf__SDL__NewSurfaceBPP(__FILE__,  __FUNCTION__,  __LINE__,width,  height,bpp)


#define  SDL_NewSurface(width,height)\
                __cmf__SDL__NewSurface(__FILE__,  __FUNCTION__,  __LINE__,width,  height)


#define  SDL_NewSurfaceFromSurface(width,  height,src)\
	__cmf__SDL__NewSurfaceFromSurface(__FILE__,  __FUNCTION__,  __LINE__,width,  height,  src)

#define    SDL_FreeSurface(sdl_surface)\
                __cmf__SDL__FreeSurface(__FILE__,  __FUNCTION__,  __LINE__,  sdl_surface)

//  Undefine  SDL_LoadBMP
#ifdef  SDL_BlitSurface
#undef  SDL_BlitSurface
#endif      

#define    SDL_BlitSurface(src,  srcrect,  dst,  dstrect)\
              __cmf__SDL__BlitSurface(src,  srcrect,  dst,  dstrect)

#define    SDL_UpdateRects(screen,  numrects,  rects)\
                __cmf__SDL__UpdateRects(screen,  numrects,  rects)

//  SDL_image

#define    IMG_Load(file)\
	        __cmf__IMG_Load(__FILE__,  __FUNCTION__,  __LINE__,file)

#define    IMG_Load_RW(src,  freesrc)\
                __cmf__IMG_Load_RW(__FILE__,  __FUNCTION__,  __LINE__,  src,  freesrc)


//  SDL_gfx..

#define      rotozoomSurface(src,  angle,  zoom,  smooth)\
		__cmf__rotozoomSurface(__FILE__,  __FUNCTION__,  __LINE__,  src,  angle,  zoom,  smooth  )

#define      rotozoomSurfaceXY(src,  angle,  zoomx,  zoomy,  smooth)\
		__cmf__rotozoomSurfaceXY(__FILE__,  __FUNCTION__,  __LINE__,  src,  angle,  zoomx,  zoomy,  smooth  )

#define      zoomSurface(src,  zoomx,  zoomy,smooth)\
		__cmf__zoomSurface(__FILE__,  __FUNCTION__,  __LINE__,  src,  zoomx,  zoomy,smooth  )





#endif
