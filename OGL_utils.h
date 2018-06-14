#pragma once

//#ifdef __USE_JGT_MEMORY__
//#define GL_GLEXT_PROTOTYPES 1
//#define GL3_PROTOTYPES 1

//#include <GL/gl.h>              /* Header File For The OpenGL Library */


#include "OGL_defines.h"

//#include 	<GL/gl.h>
//#define GL_GLEXT_PROTOTYPES
//#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

// to include glttf with wstring support
#define HAVE_WSTRING 1
#include "glttf.h"

using namespace std;

// All ogl memory...

//gluBuild2DMipmaps(GL_TEXTURE_2D, pImage->format->BytesPerPixel, pImage->w, pImage->h, format, GL_UNSIGNED_BYTE, pImage->pixels))
//glTexImage2D(GL_TEXTURE_2D, 0, pImage->format->BytesPerPixel, pImage->w, pImage->h, 0, format, GL_UNSIGNED_BYTE, pImage->pixels);
// OGL texture handler...
// tGLTexture .../*
typedef CBuffer<GLuint> 	GLuintBuffer;

bool OGL_getError(string & sCause);
void OGL_initMemoryLeaker(void);
void OGL_deInitMemoryLeaker(void);

void OGL_perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar );
bool 	OGL_createFBO(FBOInfo & fboInfo, int width, int height);
void OGL_destroyFBO(FBOInfo & fboInfo);
/*
tGLTexture *OGL_LoadImage(const char *name, SDL_Surface *srf);


tGLTexture      *SDLSurface2OpenGlImage(SDL_Surface *Image);*/
//GLuint 			getGLFormatPixelFromSDLSurface(SDL_Surface *srf);
//GLuint 			getGLTypePixelFromSDLSurface(SDL_Surface *srf);

void __cmf__glClear( GLbitfield mask );

void __cmf__glCallList( GLuint list );

void __cmf__glTexImage1D(
                    GLenum target, GLint level, GLint internalFormat,
                    GLsizei width, GLint border, GLenum format,
                    GLenum type, const GLvoid *pixels);



void __cmf__glTexImage2D(
                   GLenum target, GLint level, GLint internalFormat,
                   GLsizei width, GLsizei height, GLint border,
                   GLenum format, GLenum type,
                   const GLvoid *pixels);

void __cmf__glBegin(GLenum mode);


void __cmf__glPopAttrib();
/*void __cmf__glTexCoord2f(GLfloat x, GLfloat y);

void __cmf__glVertex2i(GLint x, GLint y);



int _gluBuild1DMipmaps(const char *module, const char *fucntion, int line,
                       GLenum target, GLint components, GLint width,GLenum format, 
                       GLenum type, void *data);



int _gluBuild2DMipmaps(const char *module, const char *fucntion, int line,
                       GLenum target, GLint components, GLint width,GLint height, 
                       GLenum format, GLenum type,void *data);*/

void __cmf__glGenTextures(const char *module, const char *fucntion, int line,
						int num, GLuint *id);

void __cmf__glDeleteTextures(const char *module, const char *fucntion, int line,
						int num, GLuint *id);


// override own functions...

#define glGenTextures(num, text)\
		__cmf__glGenTextures(__FILE__, __FUNCTION__, __LINE__,num, text)

#define   glDeleteTextures(num, text)\
		__cmf__glDeleteTextures(__FILE__, __FUNCTION__, __LINE__,num, text)


#define  glTexImage1D(target, level, internalFormat, width,border, format,type, pixels)\
		__cmf__glTexImage1D(target, level, internalFormat, width, border, format,type, pixels)

#define  glTexImage2D(target, level, internalFormat, width,height, border, format,type, pixels)\
		__cmf__glTexImage2D(target, level, internalFormat, width, height,border, format,type, pixels)


#define glBegin(mode) __cmf__glBegin(mode)

//#define  glVertex2i(x,y) __cmf__glVertex2i(x, y )

//#define  glTexCoord2f(x,y) __cmf__glTexCoord2f(x, y )



#define  glClear(flags) __cmf__glClear(flags )
#define glCallList(list) __cmf__glCallList( list )
#define  glPopAttrib() __cmf__glPopAttrib( )
//#define glTexCoord2f(x,y) __cmf__glTexCoord2f(x,y)
//#define glVertex2i(x,y) __cmf__glVertex2i(x,y)


//#endif


bool isVBOSupported();
bool isPBOSupported();
bool isShadingSupported();

/*
PFNGLGENBUFFERSARBPROC glGenBuffersARB;
PFNGLBINDBUFFERARBPROC glBindBufferARB;
PFNGLBUFFERDATAARBPROC glBufferDataARB;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;*/
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLBINDBUFFERPROC glBindBuffer;

/*PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB = NULL; // return various parameters of VBO
PFNGLMAPBUFFERARBPROC glMapBufferARB = NULL;                       // map VBO procedure
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = NULL;                   // unmap VBO procedure*/


extern PFNGLGENBUFFERSPROC 			glGenBuffers;
extern PFNGLDELETEVERTEXARRAYSPROC 	glDeleteVertexArrays;
extern PFNGLDRAWARRAYSINSTANCEDPROC 	glDrawArraysInstanced;
extern PFNGLDELETEBUFFERSPROC 			glDeleteBuffers;
extern PFNGLVERTEXATTRIBDIVISORPROC 	glVertexAttribDivisor;
extern PFNGLVERTEXATTRIBPOINTERPROC 	glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray;
extern PFNGLBUFFERSUBDATAPROC 			glBufferSubData;
extern PFNGLBUFFERDATAPROC 			glBufferData;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLVERTEXATTRIB1FNVPROC glVertexAttrib1f;
extern PFNGLVERTEXATTRIB1FVNVPROC glVertexAttrib1fv;

extern PFNGLGENVERTEXARRAYSPROC		glGenVertexArrays;

  /*PFNGLCREATEPROGRAMOBJECTARBPROC     glCreateProgramObjectARB = NULL;
  PFNGLDELETEOBJECTARBPROC            glDeleteObjectARB = NULL;
  PFNGLCREATESHADEROBJECTARBPROC      glCreateShaderObjectARB = NULL;
  PFNGLSHADERSOURCEARBPROC            glShaderSourceARB = NULL;
  PFNGLCOMPILESHADERARBPROC           glCompileShaderARB = NULL;
  PFNGLGETOBJECTPARAMETERIVARBPROC    glGetObjectParameterivARB = NULL;
  PFNGLATTACHOBJECTARBPROC            glAttachObjectARB = NULL;
  PFNGLGETINFOLOGARBPROC              glGetInfoLogARB = NULL;
  PFNGLLINKPROGRAMARBPROC             glLinkProgramARB = NULL;*/
extern PFNGLUSEPROGRAMPROC        glUseProgram;
  /*PFNGLGETUNIFORMLOCATIONARBPROC      glGetUniformLocationARB = NULL;
    PFNGLUNIFORM1FARBPROC               glUniform1fARB = NULL;
  PFNGLUNIFORM1IARBPROC               glUniform1iARB = NULL;
  PFNGLGETATTRIBLOCATIONARBPROC      glGetUniformAtttibARB = NULL;
  PFNGLGETSHADERIVPROC				  glGetShaderivARB;*/
extern PFNGLGETSHADERINFOLOGPROC 	glGetShaderInfoLog;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLCREATEPROGRAMPROC		glCreateProgram;
extern PFNGLCREATESHADERPROC		glCreateShader;
extern PFNGLDELETEPROGRAMPROC		glDeleteProgram;
extern PFNGLDELETESHADERPROC		glDeleteShader;
extern PFNGLATTACHSHADERPROC		glAttachShader;
extern PFNGLVALIDATEPROGRAMPROC		glValidateProgram;
extern PFNGLSHADERSOURCEPROC		glShaderSource;
extern PFNGLCOMPILESHADERPROC		glCompileShader;
extern PFNGLLINKPROGRAMPROC			glLinkProgram;
extern PFNGLGETSHADERIVPROC			glGetShaderiv;
extern PFNGLGETPROGRAMINFOLOGPROC              glGetProgramInfoLog;
extern PFNGLPOINTPARAMETERFVPROC	 glPointParameterfv;
extern PFNGLPOINTPARAMETERFPROC	 glPointParameterf;
extern PFNGLGETUNIFORMLOCATIONPROC	 glGetUniformLocation;
extern PFNGLGETBUFFERPARAMETERIVPROC	 glGetBufferParameteriv;
extern PFNGLMAPBUFFERPROC	 glMapBuffer;
extern PFNGLUNMAPBUFFERPROC	 glUnmapBuffer;

extern PFNGLUNIFORM2FPROC		 glUniform2f;
extern PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;               // VBO Sub Data Loading Procedure
extern PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB; // return various parameters of VBO
extern PFNGLMAPBUFFERARBPROC glMapBufferARB;                       // map VBO procedure
extern PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB; // unmap VBO procedure



 // FrameBuffer functions.
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;


 //"GL_EXT_framebuffer_object"
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;


// mingw
#ifdef __MINGW32__
extern	PFNGLACTIVETEXTUREPROC glActiveTexture;
/*extern	PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;*/
extern	PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;
#endif

void OGL_ViewStatus();




