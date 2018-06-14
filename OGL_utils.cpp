/*
#include <stdio.h>
#include <stdlib.h>

//#include "../system/defines.h"


#ifdef _WIN32
#include <windows.h>
#endif
*/
#include "utils/zg_utils.h"
#include "OGL_defines.h"
//#include "../util/CLog.h"

//#include <SDL2/SDL_opengl.h>
//#define GL_GLEXT_PROTOTYPES
#include 	<GL/gl.h>
#include 	<SDL2/SDL.h>
#include 	<SDL2/SDL_opengl.h>
#include 	<SDL2/SDL_opengl_glext.h>

#include 	"glttf/glttf.h"

//#include "OGL_utils.h"


using std::string;


enum ENUM_OGL_RegisterMethods{
	UNKNOWN_METHOD=0,
	GENTEXTURES_METHOD

};

#define OGL_MAX_IDS	4000



typedef struct{
	char filename[30];
	int 	line;
	ENUM_OGL_RegisterMethods	method;
	GLuint 	*m_pointerId;
	GLuint 	m_id;
	bool	isReserved;
}tLoadedIdOGL;

tLoadedIdOGL loadedIdOGL[OGL_MAX_IDS];

Uint32 m_reservedSlotIdOGL[OGL_MAX_IDS]; // To accelerate find free slot. This apoints a free slot (id must be initialized as GL_INVALID_VALUE)

static int m_numLoadedIdOGL=0;
//static int m_numreservedSlots=0;

//------------------------------------------------------------------------------------
bool g_fVBOSupported = false;
bool g_fPixelBOSupported = false;
bool g_shading_enabled = false;
bool g_fFBOSupported = false;
bool g_fRenderBufferSupported = false;
/*
PFNGLGENBUFFERSARBPROC glGenBuffersARB=NULL;
PFNGLBINDBUFFERARBPROC glBindBufferARB=NULL;
PFNGLBUFFERDATAARBPROC glBufferDataARB=NULL;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB=NULL;*/
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;

/*PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB = NULL; // return various parameters of VBO
PFNGLMAPBUFFERARBPROC glMapBufferARB = NULL;                       // map VBO procedure
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = NULL;                   // unmap VBO procedure*/


	PFNGLGENBUFFERSPROC 			glGenBuffers= NULL;
	PFNGLDELETEVERTEXARRAYSPROC 	glDeleteVertexArrays= NULL;
 PFNGLDRAWARRAYSINSTANCEDPROC 	glDrawArraysInstanced= NULL;
 PFNGLDELETEBUFFERSPROC 			glDeleteBuffers= NULL;
 PFNGLVERTEXATTRIBDIVISORPROC 	glVertexAttribDivisor= NULL;
 PFNGLVERTEXATTRIBPOINTERPROC 	glVertexAttribPointer= NULL;
 PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray= NULL;
 PFNGLBUFFERSUBDATAPROC 			glBufferSubData= NULL;
 PFNGLBUFFERDATAPROC 			glBufferData = NULL;
  PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
  PFNGLVERTEXATTRIB1FNVPROC glVertexAttrib1f=NULL;
  PFNGLVERTEXATTRIB1FVNVPROC glVertexAttrib1fv=NULL;

  PFNGLGENVERTEXARRAYSPROC		glGenVertexArrays=NULL;

  /*PFNGLCREATEPROGRAMOBJECTARBPROC     glCreateProgramObjectARB = NULL;
  PFNGLDELETEOBJECTARBPROC            glDeleteObjectARB = NULL;
  PFNGLCREATESHADEROBJECTARBPROC      glCreateShaderObjectARB = NULL;
  PFNGLSHADERSOURCEARBPROC            glShaderSourceARB = NULL;
  PFNGLCOMPILESHADERARBPROC           glCompileShaderARB = NULL;
  PFNGLGETOBJECTPARAMETERIVARBPROC    glGetObjectParameterivARB = NULL;
  PFNGLATTACHOBJECTARBPROC            glAttachObjectARB = NULL;
  PFNGLGETINFOLOGARBPROC              glGetInfoLogARB = NULL;
  PFNGLLINKPROGRAMARBPROC             glLinkProgramARB = NULL;*/
  PFNGLUSEPROGRAMPROC        glUseProgram = NULL;
  /*PFNGLGETUNIFORMLOCATIONARBPROC      glGetUniformLocationARB = NULL;
    PFNGLUNIFORM1FARBPROC               glUniform1fARB = NULL;
  PFNGLUNIFORM1IARBPROC               glUniform1iARB = NULL;
  PFNGLGETATTRIBLOCATIONARBPROC      glGetUniformAtttibARB = NULL;
  PFNGLGETSHADERIVPROC				  glGetShaderivARB=NULL;*/
  PFNGLGETSHADERINFOLOGPROC 	glGetShaderInfoLog=NULL;
  PFNGLGETPROGRAMIVPROC glGetProgramiv=NULL;
  PFNGLCREATEPROGRAMPROC		glCreateProgram=NULL;
  PFNGLCREATESHADERPROC		glCreateShader=NULL;
  PFNGLDELETEPROGRAMPROC		glDeleteProgram=NULL;
  PFNGLDELETESHADERPROC		glDeleteShader=NULL;
   PFNGLATTACHSHADERPROC		glAttachShader=NULL;
   PFNGLVALIDATEPROGRAMPROC		glValidateProgram=NULL;
   PFNGLSHADERSOURCEPROC		glShaderSource=NULL;
   PFNGLCOMPILESHADERPROC		glCompileShader=NULL;
   PFNGLLINKPROGRAMPROC			glLinkProgram=NULL;
   PFNGLGETSHADERIVPROC			glGetShaderiv=NULL;
   PFNGLGETPROGRAMINFOLOGPROC              glGetProgramInfoLog=NULL;
    PFNGLPOINTPARAMETERFVPROC	 glPointParameterfv=NULL;
    PFNGLPOINTPARAMETERFPROC	 glPointParameterf=NULL;
    PFNGLGETUNIFORMLOCATIONPROC	 glGetUniformLocation=NULL;
     PFNGLGETBUFFERPARAMETERIVPROC	 glGetBufferParameteriv=NULL;
     PFNGLMAPBUFFERPROC	 glMapBuffer=NULL;
     PFNGLUNMAPBUFFERPROC	 glUnmapBuffer=NULL;

     PFNGLUNIFORM2FPROC		 glUniform2f=NULL;
     PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = NULL;               // VBO Sub Data Loading Procedure
     PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB = NULL; // return various parameters of VBO
     PFNGLMAPBUFFERARBPROC glMapBufferARB = NULL;                       // map VBO procedure
     PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = NULL; // unmap VBO procedure



 // FrameBuffer functions.
 PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers=NULL;
 PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer=NULL;
 PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture=NULL;
 PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D=NULL;
 PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer=NULL;
 PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus=NULL;
 PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;


 //"GL_EXT_framebuffer_object"
 PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers=NULL;
 PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer=NULL;
 PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample=NULL;
 PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer=NULL;
 PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = NULL;
 PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = NULL;

// compatibily with mingw
#ifdef __MINGW32__
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
//PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = NULL;
#endif







// fovY     - Field of vision in degrees in the y direction
// aspect   - Aspect ratio of the viewport
// zNear    - The near clipping distance
// zFar     - The far clipping distance

void OGL_perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar ) {
	const GLdouble pi = 3.1415926535897932384626433832795;

	GLdouble fW, fH;

	fH = tan( fovY / 360 * pi ) * zNear;

	fW = fH * aspect;

	//printf("\n%f %f %f %f %f %f\n",-fW, fW, -fH, fH, zNear, zFar);


	glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}



int
isExtensionSupported(const char *extension)
{
  const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;

  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;
  extensions = glGetString(GL_EXTENSIONS);
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  start = extensions;

  if(start == NULL)
  {
  	CLog::print(0,0,CLog::LOG_ERROR	,true,"\nCannot get extensions OpenGL. Maybe opengl context is not initialized!\n");
  	return 0;//exit(-1);
  }

  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return 1;
    start = terminator;
  }
  return 0;
}



bool OGL_getError(string & sCause)
{

	GLenum m_ErrorDraw = glGetError();
	bool m_drawError = false;
	switch(m_ErrorDraw)
	{
	case GL_NO_ERROR:
		//CLog::print(0,0,CLog::LOG_ERROR	,"No error has been recorded. The value of this symbolic constant is guaranteed to be 0.");
		break;
	case GL_INVALID_ENUM:
		sCause = "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
		m_drawError = true;
		break;

	case GL_INVALID_VALUE:
		sCause = "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
		m_drawError = true;
		break;

	case GL_INVALID_OPERATION:
		sCause = "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
		m_drawError = true;
		break;
		//case GL_INVALID_FRAMEBUFFER_OPERATION:
		//	CLog::print(0,0,CLog::LOG_ERROR	,"The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.");
		//	break;
	case GL_OUT_OF_MEMORY:
		sCause =  "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
		m_drawError = true;
		break;
	case GL_STACK_UNDERFLOW:
		sCause = "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
		m_drawError = true;
		break;
	case GL_STACK_OVERFLOW:
		sCause = "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
		m_drawError = true;
		break;
	}

	return m_drawError;
}


void OGL_initMemoryLeaker(void)
{

    memset(&loadedIdOGL,0,sizeof(loadedIdOGL));

// compatibily with mingw
#ifdef __MINGW32__
	glActiveTexture = (PFNGLACTIVETEXTUREPROC) SDL_GL_GetProcAddress("glActiveTexture");
	
	if(glActiveTexture == NULL) {
		print_error_cr("Fatal error. Cannot get glActiveTexture function!");
		exit(-1);
	}


	glActiveTexture = (PFNGLACTIVETEXTUREPROC) SDL_GL_GetProcAddress("glActiveTexture");

	if(glActiveTexture == NULL) {
		print_error_cr("Fatal error. Cannot get glActiveTextureARB function!");
		exit(-1);
	}


	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC) SDL_GL_GetProcAddress("glClientActiveTexture");

	if(glClientActiveTexture == NULL) {
		print_error_cr("Fatal error. Cannot get glClientActiveTextureARB function!");
		exit(-1);
	}
#endif	

   // Check For VBOs Supported
#ifndef NO_VBOS
	g_fVBOSupported = isExtensionSupported( "GL_ARB_vertex_buffer_object" );
	if( g_fVBOSupported )
	{
		// Get Pointers To The GL Functions
		glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetBufferParameteriv");
		glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SDL_GL_GetProcAddress("glDisableVertexAttribArray");
		glBindBuffer = (PFNGLBINDBUFFERPROC) SDL_GL_GetProcAddress("glBindBuffer");
		glGenBuffers = (PFNGLGENBUFFERSPROC) SDL_GL_GetProcAddress("glGenBuffers");
		glBufferData = (PFNGLBUFFERDATAPROC) SDL_GL_GetProcAddress("glBufferData");
		glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SDL_GL_GetProcAddress("glBufferSubData");
		glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SDL_GL_GetProcAddress("glDeleteBuffers");
		glGenVertexArrays =(PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
		glBindVertexArray =(PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
		glEnableVertexAttribArray= (PFNGLENABLEVERTEXATTRIBARRAYPROC	)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    	glVertexAttribPointer= (PFNGLVERTEXATTRIBPOINTERPROC 	)SDL_GL_GetProcAddress("glVertexAttribPointer");

		g_fVBOSupported = glGetBufferParameteriv&&glDisableVertexAttribArray && glBindBuffer && glGenBuffers && glBufferData && glBufferSubData && glDeleteBuffers && glGenVertexArrays && glBindVertexArray && glEnableVertexAttribArray&&glVertexAttribPointer;

		// Load Vertex Data Into The Graphics Card Memory
	}

	if(!g_fVBOSupported)
		print_warning_cr("This OpenGL version doesn't supports GL_ARB_vertex_buffer_object extension");

    // check PBO is supported by your video card
    // check PBO is supported by your video card
    if(isExtensionSupported("GL_ARB_pixel_buffer_object"))
    {
        // get pointers to GL functions
        //glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
        //glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
        //glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
        glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)SDL_GL_GetProcAddress("glBufferSubDataARB");
        //glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
        glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetBufferParameterivARB");
        glMapBufferARB = (PFNGLMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glMapBufferARB");
        glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glUnmapBufferARB");

        // check once again PBO extension
        if( glBufferSubDataARB &&
           glMapBufferARB && glUnmapBufferARB && glGetBufferParameterivARB)
        {
        	g_fPixelBOSupported = true;
            //pboMode = 1;    // using 1 PBO
            //cout << "Movie card supports GL_ARB_pixel_buffer_object." << endl;
        }
        else
        {
        	g_fPixelBOSupported = false;
            //pboMode = 0;    // without PBO
            //cout << "Movie card does NOT support GL_ARB_pixel_buffer_object." << endl;
        }
    }

    if(isExtensionSupported("GL_ARB_framebuffer_object"))
    {
    	glGenFramebuffers= (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
    	glBindFramebuffer= (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
    	glFramebufferTexture= (PFNGLFRAMEBUFFERTEXTUREPROC)SDL_GL_GetProcAddress("glFramebufferTexture");
    	glFramebufferTexture2D= (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
    	glBlitFramebuffer= (PFNGLBLITFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBlitFramebuffer");
    	glCheckFramebufferStatus= (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
    	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");;

    	if(glGenFramebuffers &&
    	   glBindFramebuffer &&
		   glFramebufferTexture &&
		   glFramebufferTexture2D &&
		   glCheckFramebufferStatus &&
		   glDeleteFramebuffers &&
		   glBlitFramebuffer){
    		g_fFBOSupported = true;
    	}
    }
    else
    {
    	g_fFBOSupported = false;
        //pboMode = 0;    // without PBO
        //cout << "Movie card does NOT support GL_ARB_pixel_buffer_object." << endl;
    }


    //"GL_EXT_framebuffer_object"
    if(isExtensionSupported("GL_EXT_framebuffer_object"))
    {
    	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffers");
    	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbuffer");
    	glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)SDL_GL_GetProcAddress("glRenderbufferStorageMultisample");
    	glFramebufferRenderbuffer=(PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
    	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) SDL_GL_GetProcAddress ("glRenderbufferStorage");
    	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffers");



    	if(	glGenRenderbuffers &&
    		glBindRenderbuffer &&
			glRenderbufferStorageMultisample &&
			glFramebufferRenderbuffer &&
			glRenderbufferStorage &&
			glDeleteRenderbuffers
    	){
    		g_fRenderBufferSupported = true;
    	}
    }
    else
    {
    	g_fRenderBufferSupported = false;
        //pboMode = 0;    // without PBO
        //cout << "Movie card does NOT support GL_ARB_pixel_buffer_object." << endl;
    }



    g_shading_enabled=false;

    if( 
    	isExtensionSupported("GL_ARB_shading_language_100") &&
		isExtensionSupported("GL_ARB_vertex_shader") &&
		isExtensionSupported("GL_ARB_fragment_shader") )
    {
     /* glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)
		SDL_GL_GetProcAddress("glCreateProgramObjectARB");
      glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)
		SDL_GL_GetProcAddress("glDeleteObjectARB");
      glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)
		SDL_GL_GetProcAddress("glCreateShaderObjectARB");
      glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)
		SDL_GL_GetProcAddress("glShaderSourceARB");
      glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)
		SDL_GL_GetProcAddress("glCompileShaderARB");
      glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)
		SDL_GL_GetProcAddress("glGetObjectParameterivARB");
      glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)
		SDL_GL_GetProcAddress("glAttachObjectARB");
      glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)
		SDL_GL_GetProcAddress("glGetInfoLogARB");
      glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)
		SDL_GL_GetProcAddress("glLinkProgramARB");

      glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)
		SDL_GL_GetProcAddress("glGetUniformLocationARB");
      glUniform1fARB = (PFNGLUNIFORM1FARBPROC)
		SDL_GL_GetProcAddress("glUniform1fARB");
      glUniform1iARB = (PFNGLUNIFORM1IARBPROC)
		SDL_GL_GetProcAddress("glUniform1iARB");

      glGetUniformAtttibARB = (PFNGLGETATTRIBLOCATIONARBPROC)
				SDL_GL_GetProcAddress("glGetUniformAtttibARB");
      glGetShaderivARB= (PFNGLGETSHADERIVPROC)
				SDL_GL_GetProcAddress("glGetShaderiv");*/

      glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgramObjectARB");
      glGetShaderInfoLog= (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
      glGetProgramiv= (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");

      glCreateProgram= (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
      glCreateShader=(PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
      glDeleteProgram=(PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
      glDeleteShader=(PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");

      glAttachShader =(PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
      glValidateProgram   =(PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
      glShaderSource  =(PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
      glCompileShader  =(PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
      glLinkProgram  =(PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
      glGetShaderiv  =(PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
      glGetProgramInfoLog=(PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");

      glPointParameterfv=(PFNGLPOINTPARAMETERFVPROC)SDL_GL_GetProcAddress("glPointParameterfv");
      glPointParameterf=(PFNGLPOINTPARAMETERFPROC)SDL_GL_GetProcAddress("glPointParameterf");
      //glBufferSubData=(PFNGLBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glBufferSubData");
      glGetUniformLocation=(PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");

      glVertexAttrib1f= (PFNGLVERTEXATTRIB1FNVPROC)SDL_GL_GetProcAddress("glVertexAttrib1f");
      glVertexAttrib1fv= (PFNGLVERTEXATTRIB1FVNVPROC)SDL_GL_GetProcAddress("glVertexAttrib1fv");
      glUniform2f=(PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");




      if(!(false
       //|| glActiveTextureARB == NULL
       //|| glMultiTexCoord2fARB == NULL
       //|| glCreateProgramObjectARB == NULL
       //|| glDeleteObjectARB == NULL
       //|| glCreateShaderObjectARB == NULL
       //|| glShaderSourceARB == NULL
       //|| glCompileShaderARB == NULL
       //|| glGetObjectParameterivARB == NULL
       //|| glAttachObjectARB == NULL
       //|| glGetInfoLogARB == NULL
       //|| glLinkProgramARB == NULL
       || glUseProgram == NULL
       //|| glGetUniformLocationARB == NULL
       //|| glUniform1fARB == NULL
       //|| glUniform1iARB == NULL
	   //|| glGetUniformAtttibARB == NULL
	   //|| glGetShaderivARB == NULL
	   || glGetShaderInfoLog == NULL
	   || glGetProgramiv == NULL
	   || glCreateProgram == NULL
	   || glCreateShader == NULL
	   || glDeleteProgram == NULL
	   || glDeleteShader == NULL
	   || glAttachShader ==NULL
	   || glValidateProgram  ==NULL
	   || glShaderSource  ==NULL
	   || glCompileShader  ==NULL
	   || glLinkProgram  ==NULL
	   || glGetShaderiv  ==NULL
	   || glGetProgramInfoLog==NULL
	   || glPointParameterfv == NULL
	   || glPointParameterf == NULL
	   ||glBufferSubData==NULL
	   || glGetUniformLocation==NULL
	   || glVertexAttrib1fv==NULL
	   || glVertexAttrib1f==NULL
	   || glUniform2f==NULL
        )){

    	  g_shading_enabled=true;
      }


      }

    if(!g_shading_enabled){
       	print_warning_cr("This OpenGL version doesn't supports shading extension");
    }



 /*   if(isExtensionSupported("GL_ARB_vertex_array_object")){
    	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    	glDisableVertexAttribArray=	(PFNGLDISABLEVERTEXATTRIBARRAYPROC )SDL_GL_GetProcAddress("glDisableVertexAttribArray");

    	glGenBuffers=	(PFNGLGENBUFFERSPROC 			)SDL_GL_GetProcAddress("glGenBuffers");
    	glDeleteVertexArrays=	(PFNGLDELETEVERTEXARRAYSPROC 	)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    	glDrawArraysInstanced= (PFNGLDRAWARRAYSINSTANCEDPROC 	)SDL_GL_GetProcAddress("glDrawArraysInstanced");
    	glDeleteBuffers= (PFNGLDELETEBUFFERSPROC 			)SDL_GL_GetProcAddress("glDeleteBuffers");
    	glVertexAttribDivisor= (PFNGLVERTEXATTRIBDIVISORPROC 	)SDL_GL_GetProcAddress("glVertexAttribDivisor");


    	glBufferSubData= (PFNGLBUFFERSUBDATAPROC 			)SDL_GL_GetProcAddress("glBufferSubData");
    	glBufferData = (PFNGLBUFFERDATAPROC) 			SDL_GL_GetProcAddress("glBufferData");


    }else{
    	print_warning_cr("This OpenGL version doesn't supports %s GL_ARB_vertex_array_object extension");
    }*/

#else /* NO_VBOS */
	    g_fVBOSupported = false;
#endif

    print_info_cr("***** OGL initialized! *******");

}

//------------------------------------------------------------------------------------
int OGL_searchFreeSlot() {
    for(int i = 0; i < OGL_MAX_IDS; i++)
    {
             if(!loadedIdOGL[i].isReserved) {
                return i;
            }
    }
    return -1;
}
//------------------------------------------------------------------------------------
int OGL_searchId(GLuint * idToSearch) {

    for(int i = 0; i < OGL_MAX_IDS; i++) {
             //if(loadedIdOGL[i].m_pointerId == idToSearch) {
             	if(loadedIdOGL[i].m_id == (*idToSearch))
                	return i;
            //}
    }

    return -1;
}

bool OGL_registerNewIdCell(const char *filename, int line, ENUM_OGL_RegisterMethods method, GLuint *id) {
	if(m_numLoadedIdOGL < OGL_MAX_IDS) {

		int slot = OGL_searchFreeSlot();
		if(slot != -1) {

				STR_GetDirAndFileName(NULL, loadedIdOGL[slot].filename,  filename);

				loadedIdOGL[slot].line = line;
				loadedIdOGL[slot].method = method;
				//loadedIdOGL[slot].m_pointerId = id;
				loadedIdOGL[slot].m_id = *id;
				loadedIdOGL[slot].isReserved = true;
				m_numLoadedIdOGL++;

				return true;
		}
		else CLog::print(0,0,CLog::LOG_ERROR	,true,"MEMOG: Cannot find free slot!!\n");
	}
	else CLog::print(0,0,CLog::LOG_ERROR	,true, "MEMOG: Cannot register more ids. Reached maximum!!\n");

	return false;
}

void OGL_freeIdCell(Uint32 nIdCellToBeFreed, ENUM_OGL_RegisterMethods method) {
	if(m_numLoadedIdOGL > 0) {
		if(nIdCellToBeFreed < OGL_MAX_IDS) {
			if(!loadedIdOGL[nIdCellToBeFreed].isReserved)
				CLog::print(0,0,CLog::LOG_ERROR	,true,"OGL: cell is not allocated!\n");
			else { // valid slot to be freed

				if(method == loadedIdOGL[nIdCellToBeFreed].method) {
					memset(&loadedIdOGL[nIdCellToBeFreed],0,sizeof(loadedIdOGL[nIdCellToBeFreed]));
					m_numLoadedIdOGL--;
				}
				else {
					CLog::print(0,0,CLog::LOG_ERROR	,true, "OGL: invalid method to free!\n");
				}
			}

		}
		else CLog::print(0,0,CLog::LOG_ERROR	,true, "MEMOG: Invalid index cell!??\n");
	}
	else CLog::print(0,0,CLog::LOG_ERROR	,true, "MEMOG: No loaded any cells yet!!\n");
}
//------------------------------------------------------------------------------------
void __cmf__glClear( GLbitfield mask ){
	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS;

	glClear(mask);

	ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v
}

void __cmf__glCallList( GLuint list ){
	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS;

	glCallList(list);

	ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v
}

void __cmf__glGenTextures(
		const char *module, const char *function, int line,
		int num,
		GLuint *id)
{
	glGenTextures(num, id);

	if(*id == GL_INVALID_VALUE){
		print_error_cr("Invalid texture id ---> maybe openGL isn't initialized!");
		return;
	}

	OGL_registerNewIdCell(module, line, GENTEXTURES_METHOD, id);



}
//------------------------------------------------------------------------------------
void __cmf__glDeleteTextures(
		const char *module, const char *fucntion, int line, 
		int num, 
		GLuint *id)
{
		char  s[4096];

		if(*id == GL_INVALID_VALUE){ // invalid texture ---> ignore...
			return;
		}



		STR_GetDirAndFileName(NULL, s, module);

		int slot = OGL_searchId(id);

		if(slot == -1)
			CLog::print(0,0,CLog::LOG_ERROR	,"MEMOG[%s:%i]:Texture not registered\n", s, line);
		else {
			glDeleteTextures(num, id);
			OGL_freeIdCell(slot,GENTEXTURES_METHOD);
		}

		//OGL_DeleteIDTexture(*id);
}

void __cmf__glTexImage1D(
                    GLenum target, GLint level, GLint internalFormat,
                    GLsizei width, GLint border, GLenum format,
                    GLenum type, const GLvoid *pixels){

	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS;

	glTexImage1D(target, level, internalFormat, width, border, format,type, pixels);

	ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v
}



void __cmf__glTexImage2D(
                   GLenum target, GLint level, GLint internalFormat,
                   GLsizei width, GLsizei height, GLint border,
                   GLenum format, GLenum type,
                   const GLvoid *pixels){
	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS;

	glTexImage2D(target, level, internalFormat, width, height,border, format,type, pixels);

	ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v


}

void __cmf__glBegin(GLenum mode) {


	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS;

	glBegin( mode);
	ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v
}

void __cmf__glPopAttrib() {
	ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS;
	glPopAttrib();
	ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS;

}
/*
void __cmf__glTexCoord2f(GLfloat x, GLfloat y){

	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS;
	glTexCoord2f( x,y);
	ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v
}


void __cmf__glVertex2i(GLint x, GLint y){
	//-----------------------------------------------------------------v
	// due memory leaks in xlib, we disable memory pointer register...
	ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS;
	glVertex2i( x,y);
	ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS;
	//-----------------------------------------------------------------v
}
*/
//------------------------------------------------------------------------------------
bool IsExtensionSupported( char* szTargetExtension ){
    const unsigned char *pszExtensions = NULL;
    const unsigned char *pszStart;
    unsigned char *pszWhere, *pszTerminator;

    // Extension names should not have spaces
    pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
    if( pszWhere || *szTargetExtension == '\0' )
        return false;

    // Get Extensions String
    pszExtensions = glGetString( GL_EXTENSIONS );

    // Search The Extensions String For An Exact Copy
    pszStart = pszExtensions;
    for(;;)
    {
        pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
        if( !pszWhere )
            break;
        pszTerminator = pszWhere + strlen( szTargetExtension );
        if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
            if( *pszTerminator == ' ' || *pszTerminator == '\0' )
                return true;
        pszStart = pszTerminator;
    }
    return false;
}

bool isVBOSupported() {
	return g_fVBOSupported;
}

bool isPVBOSupported() {
	return g_fPixelBOSupported;
}

bool isFBOSupported(){
	return g_fFBOSupported;
}

bool isShadingSupported() {
	return g_shading_enabled;
}

bool isPBOSupported() {
	return g_fPixelBOSupported;
}

GLuint OGL_createTexture2D(const int w, const int h, GLint internalFormat, GLenum format, GLenum type)
{
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    //NOTE: You should use GL_NEAREST here. Other values can cause problems
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (GL_DEPTH_COMPONENT == format) {
        //sample like regular texture, value is in all channels
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}

bool OGL_createFBO(FBOInfo & fboInfo, int width, int height)
{
	GLuint result = GL_FRAMEBUFFER_UNDEFINED;


	if(isFBOSupported()){

		//generate textures for FBO usage. You could use other formats here, e.g. GL_RGBA8 for color
		fboInfo.color = OGL_createTexture2D(width, height, GL_RGBA16F, GL_RGB, GL_FLOAT);
		fboInfo.depth = OGL_createTexture2D(width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
		//generate and bind FBO
		glGenFramebuffers(1, &fboInfo.id);
		glBindFramebuffer(GL_FRAMEBUFFER, fboInfo.id);
		//bind color and depth texture to FBO you could also use glFramebufferTexture2D with GL_TEXTURE_2D
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fboInfo.color, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fboInfo.depth, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,fboInfo.color, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,fboInfo.depth, 0);
		//check if FBO was created ok
		if (((result = glCheckFramebufferStatus(GL_FRAMEBUFFER))==GL_FRAMEBUFFER_COMPLETE)) {
			print_info_cr("FBO %d set up successfully!", fboInfo.id);
		}
		else {
			print_error_cr("FBO %d NOT set up properly! (result:0x%X)", fboInfo.id,result);
		}
		//unbind FBO for now
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

    return result==GL_FRAMEBUFFER_COMPLETE;
}





void OGL_destroyFBO(FBOInfo & fboInfo)
{
	glDeleteFramebuffers(1,&fboInfo.id);
	glDeleteTextures(1,&fboInfo.color);
	glDeleteTextures(1,&fboInfo.depth);

}

void OGL_ViewStatus() {

	if(m_numLoadedIdOGL > 0) {

		CLog::print(0,0,CLog::LOG_ERROR	, true, "----------- MEM OGL: Mempointers to deallocate %i ---------------------",m_numLoadedIdOGL);

		// for all registers get if any is not freed...
		for(unsigned i = 0; i < OGL_MAX_IDS; i++) {
			if(loadedIdOGL[i].isReserved) {
				CLog::print(0,0,CLog::LOG_ERROR	, true, "Mem allocated at filename %s line %i with [id=%i] has not been freed\n",loadedIdOGL[i].filename, loadedIdOGL[i].line,loadedIdOGL[i].m_id);

			}
		}
	}
	else
		print_info_cr("OGL:ok\n");
}
