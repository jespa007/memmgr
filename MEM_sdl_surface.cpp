//      #include                    "MEM__cmf__SDL_.h"
#include                    <stdio.h>
//#include                    <SDL2/SDL_ttf.h>

#include                    	"system/MEM_ram.h"

#include   <SDL2/SDL.h>
#include   <SDL2/SDL_image.h>
#include   <SDL2/SDL2_rotozoom.h>
#include   <SDL2/SDL2_gfxPrimitives.h>

void SDL_html2rgb(Uint32 html, SDL_Color & c){


}

//circle_color( $surface, $x, $y, $r, $color );
void SDL_flipY(SDL_Surface *surface){
	int size_scanline = surface->w*surface->format->BytesPerPixel;
	Uint8 *src_up   = (Uint8 *)surface->pixels;
	Uint8 *src_down = ((Uint8 *)surface->pixels+(surface->h-1)*size_scanline);
	Uint8 * aux_scanline = (Uint8 *)malloc(size_scanline);
	for(int y = 0; y < surface->h/2; y++){

		memcpy(aux_scanline, src_up	  		,size_scanline);
		memcpy(src_up		, src_down		,size_scanline);
		memcpy(src_down		, aux_scanline	,size_scanline);

		src_up	+=size_scanline;
		src_down-=size_scanline;

	}

	free(aux_scanline);
}

#if defined(__DEBUG__) && defined(__MEMMANAGER__)



#define          MAX__cmf__SDL__IMAGES        1000

//----------------------------------------------------------------------------------------------------
//#define  MAX_REGISTERED_GRAPH_RESOURCES  1000

/*enum{

        TYPE__cmf__SDL__SURFACE=0,
        TYPE__cmf__SDL__TTF,
        TYPE__cmf__SDL__SOUND,
        MAX_TYPE_RESOURCES
};*/

	#define  DEFAULT_TWIN_SCREENS  	false


SDL_Surface	*MovieSurface = NULL;


#define  MAX__cmf__SDL__IMAGES    1000

typedef  struct{

        struct{
                char    name[150];
                SDL_Surface  *resource;
                
        }Registered__cmf__SDL__Image[MAX__cmf__SDL__IMAGES+1];

        int  NRegistered__cmf__SDL__Images;

}tListSDL_Images;

tListSDL_Images        List__cmf__SDL__Images;//[MAX_TYPE_RESOURCES];



//----------------------------------------------------------------------------------------------------------------------------
void	 __cmf__SDL__FreeSurface(const  char  *module,const  char  *function,  int  line,
								    void  *surface)
{

    
		if(JGT_PreAllocatedPointerExist(surface, module, function, line)) // Because some images shares the pointer may be a deallocated surface ...
		{
	//#ifdef  __TEST_POINTERS__
				
				JGT_DeleteReservedPointer(surface, module, function, line);

                if((((SDL_Surface *)surface)->flags & SDL_PREALLOC) == SDL_PREALLOC)
                {
                            free(((SDL_Surface *)surface)->pixels);
                }


				SDL_FreeSurface((SDL_Surface *)surface);
			}

}


SDL_Surface * __cmf__SDL__GetMovieSurface(const  char  *module,const  char  *function,  int  line)
{
	return MovieSurface;
}

void __cmf__SDL__SetMovieSurface(const  char  *module,const  char  *function,  int  line, SDL_Surface *srf)
{
	srf = MovieSurface;
}

//------------------------------------------------------------------------------------------------------
unsigned  MaxAllocatedSDL_Surfaces  =  0;

void  MEMSDL__DeAllocateSurfaces()
{
//        int  type  =  GetTypeResource(name);
        unsigned  bytes_reserved  =  0;
        SDL_Surface  *_srf;


                for(int  i  =  0;  i  <  MAX__cmf__SDL__IMAGES;i++)
                {

                        
                        _srf  =  List__cmf__SDL__Images.Registered__cmf__SDL__Image[i].resource;

#ifdef  __DEBUG__

						//if(AGL_Options.TestAllocatedImages)
                        	printf("\nDe  allocating  %s......                             ",List__cmf__SDL__Images.Registered__cmf__SDL__Image[i].name);
#endif

                        if(_srf  !=  NULL)
                        {
                        

                                if((short)_srf->w  <  0  ||  (short)_srf->h  <  0  ||  (short)_srf->clip_rect.w  <  0  ||  (short)_srf->clip_rect.h  <  0)  //  Huele  mal!
                                {
                                        fprintf(stderr,"\n%s is a bad surface! Cannot be deallocated!",List__cmf__SDL__Images.Registered__cmf__SDL__Image[i].name);
                                }
                                else
                                {

                                                        bytes_reserved  +=  (_srf->w*_srf->h*_srf->format->BytesPerPixel);
                                                        __cmf__SDL__FreeSurface(0,0,0,_srf);
                                                        List__cmf__SDL__Images.Registered__cmf__SDL__Image[i].resource = NULL;

                                }
                        }

                }

        printf("\nMax  allocated  surfaces  %i  MB.  Total  allocated  surfaces  %i  MB  !\n\n",  MaxAllocatedSDL_Surfaces/(1024*1024),bytes_reserved/(1024*1024));
    
}
//------------------------------------------------------------------------------------------------------
SDL_Surface                        	*__cmf__SDL__NewSurface(const  char  *module,const  char  *function,  int  line,int  width,  int  height);
SDL_Surface                        	*__cmf__SDL__NewSurfaceBPP(const  char  *module,const  char  *function,  int  line,int  width,  int  height, int bpp);
SDL_Surface		*__cmf__SDL__CreateRGBSurface(const  char  *module,const  char  *function,  int  line,
									      Uint32  flags,  int  width,  int  height,  int  depth,  Uint32  Rmask,  Uint32  Gmask,  Uint32  Bmask,  Uint32  Amask);

SDL_Surface  *video_aux_surface  =  NULL;
//----------------------------------------------------------------------------------------------------------------------------
void  MEMSDL_SURFACE_Init()
{

        /*  SDL  interprets  each  pixel  as  a  32-bit  number,  so  our  masks  must  depend
              on  the  endianness  (byte  order)  of  the  machine  */

        
        memset(&List__cmf__SDL__Images,0,sizeof(List__cmf__SDL__Images));


        printf("\nSDL  Surface  INIT  (OK)");
}

//----------------------------------------------------------------------------------------------------------------------------

int
__cmf__SDL_Init(
		const  char  *module,const  char  *function,  int  line,
		Uint32 flags){
	int ret;
	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	JEME_MEM_DISABLE_REGISTER_MEMORYLEAKS;

	ret = SDL_Init(flags);

	JEME_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v
	return ret;
}

int
__cmf__SDL_InitSubSystem(
		const  char  *module,const  char  *function,  int  line,
		Uint32 flags){
	int ret;

	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	JEME_MEM_DISABLE_REGISTER_MEMORYLEAKS;

	ret = SDL_InitSubSystem(flags);

	JEME_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v

	return ret;
}

//----------------------------------------------------------------------------------------------------------------------------
//SDL_CreateRGBSurface
SDL_Surface		*__cmf__SDL__CreateRGBSurface(const  char  *module,const  char  *function,  int  line,
									      Uint32  flags,  int  width,  int  height,  int  depth,  Uint32  Rmask,  Uint32  Gmask,  Uint32  Bmask,  Uint32  Amask)
{
        SDL_Surface  *aux  =  NULL;
        if((aux  =  SDL_CreateRGBSurface(flags,  width,  height,  depth,  Rmask,Gmask,  Bmask,  Amask)))
        {
//#ifdef  __TEST_POINTERS__
                JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
//#endif
        }
        
        return  aux;
}
//----------------------------------------------------------------------------------------------------------------------------
SDL_Surface		*__cmf__SDL__CreateRGBSurfaceFrom(const  char  *module,const  char  *function,  int  line,
										      void  *pixels,  int  width,  int  height,  int  depth,  int  pitch,  Uint32  Rmask,  Uint32  Gmask,  Uint32  Bmask,  Uint32  Amask)
{
        SDL_Surface  *aux  =  NULL;
        if((aux  =  SDL_CreateRGBSurfaceFrom(pixels,  width,  height,  depth,  pitch,  Rmask,  Gmask,  Bmask,  Amask)))
        {
//#ifdef  __TEST_POINTERS__
                        JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
//#endif

        }

        return  aux;

        
}
//----------------------------------------------------------------------------------------------------------------------------
/*SDL_Surface		*__cmf__SDL__LoadBMP(const  char  *module,const  char  *function,  int  line,
							    const  char  *file)
{
      SDL_Surface *aux = SDL_LoadBMP(file);


      if(aux != NULL)
      	JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);

	return aux;


}*/
//----------------------------------------------------------------------------------------------------------------------------
SDL_Surface		*__cmf__SDL__ConvertSurface(const  char  *module,const  char  *function,  int  line,
									  SDL_Surface  *src,  SDL_PixelFormat  *fmt,  Uint32  flags)
{
        SDL_Surface  *aux  =  NULL;
        if((aux  =  SDL_ConvertSurface(src,  fmt,  flags)))
        {
                        JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
        }

        return  aux;

}          
//----------------------------------------------------------------------------------------------------------------------------
int  __cmf__SDL__BlitSurface(SDL_Surface  *src,  SDL_Rect  *_srcrect,  SDL_Surface  *dst,  SDL_Rect  *_dstrect)
{
		if(!dst || !src) return 0;

		SDL_BlitSurface(src,_srcrect,dst,_dstrect);
		
		return 0;
}

/*
//-------------------------------------------------------------------------------------------------------------------------
SDL_Surface  *__cmf__SDL__NewSurfaceFromSurface(const  char  *module,const  char  *function,int  line,int  width,  int  height,SDL_Surface  *src)
{
		SDL_Surface  *aux  =  NULL;

		if(src)
		{
		
			 if( (aux  =  __cmf__SDL__CreateRGBSurface(
                                module,  
                                function,  
                                line,  
                                0,
                                width,  
                                height,  
                                src->format->BitsPerPixel,  
                                src->format->Rmask,  
                                src->format->Gmask,	
                                src->format->Bmask,
		                src->format->Amask
                        )))
					SDL_BlitSurface(src,NULL,aux,NULL);
                                  
		}

		return  aux;
		
}*/

//----------------------------------------------------------------------------------------------------------------------------
SDL_Surface		*__cmf__SDL__CopySurface(const  char  *module,const  char  *function,  int  line,
							SDL_Surface *src, SDL_Rect *src_rect)
	//						void  *pixels,  int  width,  int  height,  int  depth,  int  pitch,  Uint32  Rmask,  Uint32  Gmask,  Uint32  Bmask,  Uint32  Amask)
{
        SDL_Surface  *aux  =  NULL;
		Uint32 bytes_per_pixel = src->format->BytesPerPixel;

        if(!src_rect || !src) return NULL;

        //return NULL;

	
	
		Uint8 *pixels_src,*pixels_dst;
//		Uint8 *pixels_end   = ((Uint8 *)src->pixels) +bytes_per_pixel*(src_rect->y+src_rect->h)*src->w+(src_rect->x+src_rect->w)*bytes_per_pixel;
        Uint8 *pixels = NULL;//(Uint8 *)malloc(src_rect->w*src_rect->h*bytes_per_pixel);
        int pitch = src_rect->w*src->format->BytesPerPixel;


		if(src->pixels) // not null
        {
        pixels = (Uint8 *)malloc(src_rect->w*src_rect->h*bytes_per_pixel);
        for(int y = 0; y < src_rect->h; y++)
        {
            pixels_src = ((Uint8 *)src->pixels) +bytes_per_pixel*(src_rect->y+   y       )*src->w     +(src_rect->x    +   0   )*bytes_per_pixel;
            pixels_dst = (              pixels) +bytes_per_pixel*(   0       +   y       )*src_rect->w+(     0         +   0   )*bytes_per_pixel;

            memcpy(pixels_dst,pixels_src, pitch);
        }
        
		
		

        if((aux  =  SDL_CreateRGBSurfaceFrom(pixels,  // ---> must be freed
											  	src_rect->w,  
												src_rect->h,  
												src->format->BitsPerPixel,
												pitch,  
												src->format->Rmask,  
												src->format->Gmask,  
												src->format->Bmask,  
												src->format->Amask)))
        {
                        JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
                        


        }
        }

       

        return  aux;

        
}

//----------------------------------------------------------------------------------------------------------------------------
void  __cmf__SDL__UpdateRects(SDL_Surface  *screen,  int  numrects,  SDL_Rect  *rects)
{

       // SDL_UpdateRects(screen,  numrects,  rects);
}

//----------------------------------------------------------------------------------------------------------------------------
//  SDL_image

SDL_Surface		*__cmf__IMG_Load(const  char  *module,const  char  *function,  int  line,  const  char  *file)
{
        //return    MM__cmf__SDL__IMAGE_Load(file);


		SDL_Surface *aux =  IMG_Load(file);

        if(aux != NULL)
        {
        	JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
        }

        return aux;


}
//----------------------------------------------------------------------------------------------------------------------------
SDL_Surface		*__cmf__IMG_Load_RW(const  char  *module,const  char  *function,  int  line,  SDL_RWops  *src,  int  freesrc)
{

        SDL_Surface *aux = IMG_Load_RW(src,  freesrc);

        if(aux != NULL)
        {
        	JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
        }

        return aux;
}

//  SDL_gfx..
			  //_rotozoomSurface
SDL_Surface		*__cmf__rotozoomSurface    (const  char  *module,const  char  *function,  int  line,
									SDL_Surface  *  src,  double  angle,  double  zoom  ,                              int  smooth)
{
	
        SDL_Surface *aux = NULL;

        if((aux  =rotozoomSurface(src,  angle,  zoom  ,smooth)) != NULL)
        {
            JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
        }


        return  aux;
}
//----------------------------------------------------------------------------------------------------------------------------
			  
SDL_Surface		*__cmf__rotozoomSurfaceXY(const  char  *module,const  char  *function,  int  line,
									SDL_Surface  *  src,  double  angle,  double  zoomx,  double  zoomy,  int  smooth)
{


                SDL_Surface *aux = NULL;

        if((aux  =rotozoomSurfaceXY(src,  angle,  zoomx,  zoomy  ,smooth)) != NULL)
        {
            JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
        }


        return  aux;
}
//----------------------------------------------------------------------------------------------------------------------------
SDL_Surface		*__cmf__zoomSurface            (const  char  *module,const  char  *function,  int  line,
									SDL_Surface  *  src,  double  zoomx,  double  zoomy,                              int  smooth)
{

                SDL_Surface *aux = NULL;

        if((aux  =zoomSurface(src,  zoomx,  zoomy  ,smooth)) != NULL)
        {
            JGT_AddReservedPointer(aux, __cmf__SDL__FreeSurface,module,function,line);
        }


        return  aux;
}


#endif
