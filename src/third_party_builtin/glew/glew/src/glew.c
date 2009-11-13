/*
** The OpenGL Extension Wrangler Library
** Copyright (C) 2002-2008, Milan Ikits <milan ikits[]ieee org>
** Copyright (C) 2002-2008, Marcelo E. Magallon <mmagallo[]debian org>
** Copyright (C) 2002, Lev Povalahev
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without 
** modification, are permitted provided that the following conditions are met:
** 
** * Redistributions of source code must retain the above copyright notice, 
**   this list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice, 
**   this list of conditions and the following disclaimer in the documentation 
**   and/or other materials provided with the distribution.
** * The name of the author may be used to endorse or promote products 
**   derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <GL/glew.h>
#if defined(_WIN32)
#  include <GL/wglew.h>
#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX)
#  include <GL/glxew.h>
#endif

/*
 * Define glewGetContext and related helper macros.
 */
#ifdef GLEW_MX
#  define glewGetContext() ctx
#  ifdef _WIN32
#    define GLEW_CONTEXT_ARG_DEF_INIT GLEWContext* ctx
#    define GLEW_CONTEXT_ARG_VAR_INIT ctx
#    define wglewGetContext() ctx
#    define WGLEW_CONTEXT_ARG_DEF_INIT WGLEWContext* ctx
#    define WGLEW_CONTEXT_ARG_DEF_LIST WGLEWContext* ctx
#  else /* _WIN32 */
#    define GLEW_CONTEXT_ARG_DEF_INIT void
#    define GLEW_CONTEXT_ARG_VAR_INIT
#    define glxewGetContext() ctx
#    define GLXEW_CONTEXT_ARG_DEF_INIT void
#    define GLXEW_CONTEXT_ARG_DEF_LIST GLXEWContext* ctx
#  endif /* _WIN32 */
#  define GLEW_CONTEXT_ARG_DEF_LIST GLEWContext* ctx
#else /* GLEW_MX */
#  define GLEW_CONTEXT_ARG_DEF_INIT void
#  define GLEW_CONTEXT_ARG_VAR_INIT
#  define GLEW_CONTEXT_ARG_DEF_LIST void
#  define WGLEW_CONTEXT_ARG_DEF_INIT void
#  define WGLEW_CONTEXT_ARG_DEF_LIST void
#  define GLXEW_CONTEXT_ARG_DEF_INIT void
#  define GLXEW_CONTEXT_ARG_DEF_LIST void
#endif /* GLEW_MX */

#ifdef _WIN32
typedef void*(WINAPI *GPA)(const GLubyte*);
typedef void*(WINAPI *_GPA)(const char*);
typedef  PROC  (WINAPI* WGLGPA)(LPCSTR);
static WGLGPA _wglewGetProcAddress = NULL;
static HDC (*_wglewGetCurrentDC)(void) = NULL;
#else
typedef void*(*GPA)(const GLubyte*);
typedef void*(*_GPA)(const char*);
#endif

static enum GL_Name_Convention glew_convention = GLEW_NAME_CONVENTION_GL;
#ifdef _WIN32
static const char *glew_gl_lib = "opengl32.dll";
#elif defined(__APPLE__)
static const char *glew_gl_lib = "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL";
#elif defined(__linux__)
static const char *glew_gl_lib = "/usr/lib/libGL.so";
#elif defined(_AIX)
static const char *glew_gl_lib = "/usr/lib/libGL.a";
#else
static const char *glew_gl_lib = "/usr/lib/libGL.so";
#endif

static GPA __glewXGetProcAddress = NULL;

#if defined(__APPLE__)
#include <stdlib.h>
#include <string.h>
#include <AvailabilityMacros.h>

#ifdef MAC_OS_X_VERSION_10_3

#include <dlfcn.h>

void* NSGLGetProcAddress (const GLubyte *name)
{
  static void* image = NULL;
  if (NULL == image)
  {
    image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
  }
  return image ? dlsym(image, (const char*)name) : NULL;
}

#endif /* MAC_OS_X_VERSION_10_3 */
#endif /* __APPLE__ */

#if defined(__sgi) || defined (__sun)
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void* dlGetProcAddress (const GLubyte* name)
{
  static void* h = NULL;
  static void* gpa;

  if (h == NULL)
  {
    if ((h = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL)) == NULL) return NULL;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != NULL)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun */

#if defined (_WIN32)
static HMODULE dso_handle = NULL;
#else
#include <dlfcn.h>
static void *dso_handle = NULL;
#endif

/* Give a couple debugging macros which allow us to report issues, but only if
 * a debugging define is given.  When defined, GLEW requires the standard C
 * library. */
#if defined(GLEW_DEBUG) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
# include <stdio.h>
# include <stdlib.h>
# define warning(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
# define error(...) do { fprintf(stderr, __VA_ARGS__); abort(); } while(0)
#else
#ifdef __xlC__
#include <stdio.h>
# define warning if(0)printf /* nothing */
# define error   if(0)printf /* nothing */
#else
# define warning(...) /* nothing */
# define error(...) /* nothing */
#endif
#endif

/* Function pointer for OSMesa function loader. */
static _GPA _glewOSMesaGetProcAddress = NULL;

/* The interfaces to GetProcAddress for glX and OSMesa differ in both return
 * type and argument type.  To get around this, we create two thunks which
 * forward to the appropriate function, casting back and forth. */
static void* _glew_glx_thunk_gpa(const char *name) {
  return (void*) __glewXGetProcAddress((const GLubyte*)name);
}
#ifdef __APPLE__
/* Apple doesn't use a function loader in its proprietary APIs: just load the
 * symbol directly from the object. */
static void* _glew_dlsym_thunk_gpa(const char *name) {
  return (void*) dlsym(dso_handle, name);
}
#endif
static void* _glew_osmesa_thunk_gpa(const char *name) {
  return _glewOSMesaGetProcAddress(name);
}
static void* (*_glewGetProcAddress)(const char*);

#ifdef _WIN32
static void* _glew_wgl_thunk_gpa(const char* name) {
  void* addr = _wglewGetProcAddress((LPCSTR)name);
  if (addr == NULL)
  {
    addr = GetProcAddress(dso_handle, (LPCSTR)name);
  }

  return addr;
}
#endif

/* Initialize our function loader.
 * This loads function pointers for .. functions that load function pointers.
 * This translates to glXGetProcAddress on X-based platforms, and OSMesa
 * otherwise. */
static void _glewInitFunctionLoader()
{
#if defined(_WIN32)
  const char wgl_gpa[] = "mwglGetProcAddress";
  DWORD err = ERROR_SUCCESS;
  if (NULL == dso_handle)
  {
    dso_handle = LoadLibrary(glew_gl_lib);
    if (dso_handle == NULL)
    {
      warning("Could not load library `%s': %d", glew_gl_lib, GetLastError());
    }
  }
  GetLastError();
#else
  const char *dl_error;
  const char glx_gpa[] = "mglXGetProcAddressARB";
  if(NULL == dso_handle) {
    dso_handle = dlopen(glew_gl_lib, RTLD_LAZY | RTLD_LOCAL);
    if(NULL == dso_handle) {
      warning("Could not load library `%s': %s", glew_gl_lib, dlerror());
      return;
    }
  }
  dlerror(); /* clear any previous error. */
#endif


  switch(glew_convention) {
  case GLEW_NAME_CONVENTION_MANGLED_MESA:
    __glewXGetProcAddress = NULL;
#ifdef _WIN32
    _glewOSMesaGetProcAddress = (_GPA)GetProcAddress(dso_handle, "OSMesaGetProcAddress");
#else
    _glewOSMesaGetProcAddress = (_GPA)dlsym(dso_handle, "OSMesaGetProcAddress");
#endif
    _glewGetProcAddress = _glew_osmesa_thunk_gpa;
    break;
  case GLEW_NAME_CONVENTION_GL: /* FALL THROUGH */
  default:
#ifdef _WIN32
    _wglewGetProcAddress = (WGLGPA)GetProcAddress(dso_handle, wgl_gpa+1);
    _glewGetProcAddress = _glew_wgl_thunk_gpa;
	_wglewGetCurrentDC = _glewGetProcAddress("wglGetCurrentDC");
#elif defined(__APPLE__)
    __glewXGetProcAddress = NULL;
    _glewGetProcAddress = _glew_dlsym_thunk_gpa;
#else
    __glewXGetProcAddress = (GPA)dlsym(dso_handle, glx_gpa+1 /* skip 'm' */);
    _glewGetProcAddress = _glew_glx_thunk_gpa;
#endif

    break;
  }

#ifdef _WIN32
  err = GetLastError();
  if (err != ERROR_SUCCESS)
  {
    error("Could not find dynamic loading function; %d", err);
  }
#else
  if((dl_error = dlerror()) != NULL) {
    error("Could not find dynamic loading function; %s", dl_error);
  }
#endif
}

#define fqn_from_convention(convention, fqn) \
    (convention == GLEW_NAME_CONVENTION_MANGLED_MESA ? "m"fqn : fqn)

/*
 * Define glewGetProcAddress.
 */
#if defined(_WIN32)
#  define glewGetProcAddress(name) (*_glewGetProcAddress)(name)
#  define wglGetCurrentDC _wglewGetCurrentDC
#else
#  if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_3) && !defined(MAC_OS_X_VERSION_10_4)
#    define glewGetProcAddress(name) NSGLGetProcAddress(name)
#  else
#    if defined(__sgi) || defined(__sun)
#      define glewGetProcAddress(name) dlGetProcAddress(name)
#    else /* __linux */
#      define glewGetProcAddress(name) (*_glewGetProcAddress)(name)
#    endif
#  endif
#endif

/*
 * Define GLboolean const cast.
 */
#define CONST_CAST(x) (*(GLboolean*)&x)

/*
 * GLEW, just like OpenGL or GLU, does not rely on the standard C library.
 * These functions implement the functionality required in this file.
 */
static GLuint _glewStrLen (const GLubyte* s)
{
  GLuint i=0;
  if (s == NULL) return 0;
  while (s[i] != '\0') i++;
  return i;
}

static GLuint _glewStrCLen (const GLubyte* s, GLubyte c)
{
  GLuint i=0;
  if (s == NULL) return 0;
  while (s[i] != '\0' && s[i] != c) i++;
  return (s[i] == '\0' || s[i] == c) ? i : 0;
}

static GLboolean _glewStrSame (const GLubyte* a, const GLubyte* b, GLuint n)
{
  GLuint i=0;
  if(a == NULL || b == NULL)
    return (a == NULL && b == NULL && n == 0) ? GL_TRUE : GL_FALSE;
  while (i < n && a[i] != '\0' && b[i] != '\0' && a[i] == b[i]) i++;
  return i == n ? GL_TRUE : GL_FALSE;
}

static GLboolean _glewStrSame1 (GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  while (*na > 0 && (**a == ' ' || **a == '\n' || **a == '\r' || **a == '\t'))
  {
    (*a)++;
    (*na)--;
  }
  if(*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)+i != NULL && b+i != NULL && (*a)[i] == b[i]) i++;
    if(i == nb)
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

static GLboolean _glewStrSame2 (GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  if(*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)+i != NULL && b+i != NULL && (*a)[i] == b[i]) i++;
    if(i == nb)
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

static GLboolean _glewStrSame3 (GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  if(*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)+i != NULL && b+i != NULL && (*a)[i] == b[i]) i++;
    if (i == nb && (*na == nb || (*a)[i] == ' ' || (*a)[i] == '\n' || (*a)[i] == '\r' || (*a)[i] == '\t'))
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

#if !defined(_WIN32) || !defined(GLEW_MX)

PFNGLACCUMPROC __glewAccum = NULL;
PFNGLALPHAFUNCPROC __glewAlphaFunc = NULL;
PFNGLARETEXTURESRESIDENTPROC __glewAreTexturesResident = NULL;
PFNGLARRAYELEMENTPROC __glewArrayElement = NULL;
PFNGLBEGINPROC __glewBegin = NULL;
PFNGLBINDTEXTUREPROC __glewBindTexture = NULL;
PFNGLBITMAPPROC __glewBitmap = NULL;
PFNGLBLENDFUNCPROC __glewBlendFunc = NULL;
PFNGLCALLLISTPROC __glewCallList = NULL;
PFNGLCALLLISTSPROC __glewCallLists = NULL;
PFNGLCLEARPROC __glewClear = NULL;
PFNGLCLEARACCUMPROC __glewClearAccum = NULL;
PFNGLCLEARCOLORPROC __glewClearColor = NULL;
PFNGLCLEARDEPTHPROC __glewClearDepth = NULL;
PFNGLCLEARINDEXPROC __glewClearIndex = NULL;
PFNGLCLEARSTENCILPROC __glewClearStencil = NULL;
PFNGLCLIPPLANEPROC __glewClipPlane = NULL;
PFNGLCOLOR3BPROC __glewColor3b = NULL;
PFNGLCOLOR3BVPROC __glewColor3bv = NULL;
PFNGLCOLOR3DPROC __glewColor3d = NULL;
PFNGLCOLOR3DVPROC __glewColor3dv = NULL;
PFNGLCOLOR3FPROC __glewColor3f = NULL;
PFNGLCOLOR3FVPROC __glewColor3fv = NULL;
PFNGLCOLOR3IPROC __glewColor3i = NULL;
PFNGLCOLOR3IVPROC __glewColor3iv = NULL;
PFNGLCOLOR3SPROC __glewColor3s = NULL;
PFNGLCOLOR3SVPROC __glewColor3sv = NULL;
PFNGLCOLOR3UBPROC __glewColor3ub = NULL;
PFNGLCOLOR3UBVPROC __glewColor3ubv = NULL;
PFNGLCOLOR3UIPROC __glewColor3ui = NULL;
PFNGLCOLOR3UIVPROC __glewColor3uiv = NULL;
PFNGLCOLOR3USPROC __glewColor3us = NULL;
PFNGLCOLOR3USVPROC __glewColor3usv = NULL;
PFNGLCOLOR4BPROC __glewColor4b = NULL;
PFNGLCOLOR4BVPROC __glewColor4bv = NULL;
PFNGLCOLOR4DPROC __glewColor4d = NULL;
PFNGLCOLOR4DVPROC __glewColor4dv = NULL;
PFNGLCOLOR4FPROC __glewColor4f = NULL;
PFNGLCOLOR4FVPROC __glewColor4fv = NULL;
PFNGLCOLOR4IPROC __glewColor4i = NULL;
PFNGLCOLOR4IVPROC __glewColor4iv = NULL;
PFNGLCOLOR4SPROC __glewColor4s = NULL;
PFNGLCOLOR4SVPROC __glewColor4sv = NULL;
PFNGLCOLOR4UBPROC __glewColor4ub = NULL;
PFNGLCOLOR4UBVPROC __glewColor4ubv = NULL;
PFNGLCOLOR4UIPROC __glewColor4ui = NULL;
PFNGLCOLOR4UIVPROC __glewColor4uiv = NULL;
PFNGLCOLOR4USPROC __glewColor4us = NULL;
PFNGLCOLOR4USVPROC __glewColor4usv = NULL;
PFNGLCOLORMASKPROC __glewColorMask = NULL;
PFNGLCOLORMATERIALPROC __glewColorMaterial = NULL;
PFNGLCOLORPOINTERPROC __glewColorPointer = NULL;
PFNGLCOPYPIXELSPROC __glewCopyPixels = NULL;
PFNGLCOPYTEXIMAGE1DPROC __glewCopyTexImage1D = NULL;
PFNGLCOPYTEXIMAGE2DPROC __glewCopyTexImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE1DPROC __glewCopyTexSubImage1D = NULL;
PFNGLCOPYTEXSUBIMAGE2DPROC __glewCopyTexSubImage2D = NULL;
PFNGLCULLFACEPROC __glewCullFace = NULL;
PFNGLDELETELISTSPROC __glewDeleteLists = NULL;
PFNGLDELETETEXTURESPROC __glewDeleteTextures = NULL;
PFNGLDEPTHFUNCPROC __glewDepthFunc = NULL;
PFNGLDEPTHMASKPROC __glewDepthMask = NULL;
PFNGLDEPTHRANGEPROC __glewDepthRange = NULL;
PFNGLDISABLEPROC __glewDisable = NULL;
PFNGLDISABLECLIENTSTATEPROC __glewDisableClientState = NULL;
PFNGLDRAWARRAYSPROC __glewDrawArrays = NULL;
PFNGLDRAWBUFFERPROC __glewDrawBuffer = NULL;
PFNGLDRAWELEMENTSPROC __glewDrawElements = NULL;
PFNGLDRAWPIXELSPROC __glewDrawPixels = NULL;
PFNGLEDGEFLAGPROC __glewEdgeFlag = NULL;
PFNGLEDGEFLAGPOINTERPROC __glewEdgeFlagPointer = NULL;
PFNGLEDGEFLAGVPROC __glewEdgeFlagv = NULL;
PFNGLENABLEPROC __glewEnable = NULL;
PFNGLENABLECLIENTSTATEPROC __glewEnableClientState = NULL;
PFNGLENDPROC __glewEnd = NULL;
PFNGLENDLISTPROC __glewEndList = NULL;
PFNGLEVALCOORD1DPROC __glewEvalCoord1d = NULL;
PFNGLEVALCOORD1DVPROC __glewEvalCoord1dv = NULL;
PFNGLEVALCOORD1FPROC __glewEvalCoord1f = NULL;
PFNGLEVALCOORD1FVPROC __glewEvalCoord1fv = NULL;
PFNGLEVALCOORD2DPROC __glewEvalCoord2d = NULL;
PFNGLEVALCOORD2DVPROC __glewEvalCoord2dv = NULL;
PFNGLEVALCOORD2FPROC __glewEvalCoord2f = NULL;
PFNGLEVALCOORD2FVPROC __glewEvalCoord2fv = NULL;
PFNGLEVALMESH1PROC __glewEvalMesh1 = NULL;
PFNGLEVALMESH2PROC __glewEvalMesh2 = NULL;
PFNGLEVALPOINT1PROC __glewEvalPoint1 = NULL;
PFNGLEVALPOINT2PROC __glewEvalPoint2 = NULL;
PFNGLFEEDBACKBUFFERPROC __glewFeedbackBuffer = NULL;
PFNGLFINISHPROC __glewFinish = NULL;
PFNGLFLUSHPROC __glewFlush = NULL;
PFNGLFOGFPROC __glewFogf = NULL;
PFNGLFOGFVPROC __glewFogfv = NULL;
PFNGLFOGIPROC __glewFogi = NULL;
PFNGLFOGIVPROC __glewFogiv = NULL;
PFNGLFRONTFACEPROC __glewFrontFace = NULL;
PFNGLFRUSTUMPROC __glewFrustum = NULL;
PFNGLGENLISTSPROC __glewGenLists = NULL;
PFNGLGENTEXTURESPROC __glewGenTextures = NULL;
PFNGLGETBOOLEANVPROC __glewGetBooleanv = NULL;
PFNGLGETCLIPPLANEPROC __glewGetClipPlane = NULL;
PFNGLGETDOUBLEVPROC __glewGetDoublev = NULL;
PFNGLGETERRORPROC __glewGetError = NULL;
PFNGLGETFLOATVPROC __glewGetFloatv = NULL;
PFNGLGETINTEGERVPROC __glewGetIntegerv = NULL;
PFNGLGETLIGHTFVPROC __glewGetLightfv = NULL;
PFNGLGETLIGHTIVPROC __glewGetLightiv = NULL;
PFNGLGETMAPDVPROC __glewGetMapdv = NULL;
PFNGLGETMAPFVPROC __glewGetMapfv = NULL;
PFNGLGETMAPIVPROC __glewGetMapiv = NULL;
PFNGLGETMATERIALFVPROC __glewGetMaterialfv = NULL;
PFNGLGETMATERIALIVPROC __glewGetMaterialiv = NULL;
PFNGLGETPIXELMAPFVPROC __glewGetPixelMapfv = NULL;
PFNGLGETPIXELMAPUIVPROC __glewGetPixelMapuiv = NULL;
PFNGLGETPIXELMAPUSVPROC __glewGetPixelMapusv = NULL;
PFNGLGETPOINTERVPROC __glewGetPointerv = NULL;
PFNGLGETPOLYGONSTIPPLEPROC __glewGetPolygonStipple = NULL;
PFNGLGETSTRINGPROC __glewGetString = NULL;
PFNGLGETTEXENVFVPROC __glewGetTexEnvfv = NULL;
PFNGLGETTEXENVIVPROC __glewGetTexEnviv = NULL;
PFNGLGETTEXGENDVPROC __glewGetTexGendv = NULL;
PFNGLGETTEXGENFVPROC __glewGetTexGenfv = NULL;
PFNGLGETTEXGENIVPROC __glewGetTexGeniv = NULL;
PFNGLGETTEXIMAGEPROC __glewGetTexImage = NULL;
PFNGLGETTEXLEVELPARAMETERFVPROC __glewGetTexLevelParameterfv = NULL;
PFNGLGETTEXLEVELPARAMETERIVPROC __glewGetTexLevelParameteriv = NULL;
PFNGLGETTEXPARAMETERFVPROC __glewGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC __glewGetTexParameteriv = NULL;
PFNGLHINTPROC __glewHint = NULL;
PFNGLINDEXMASKPROC __glewIndexMask = NULL;
PFNGLINDEXPOINTERPROC __glewIndexPointer = NULL;
PFNGLINDEXDPROC __glewIndexd = NULL;
PFNGLINDEXDVPROC __glewIndexdv = NULL;
PFNGLINDEXFPROC __glewIndexf = NULL;
PFNGLINDEXFVPROC __glewIndexfv = NULL;
PFNGLINDEXIPROC __glewIndexi = NULL;
PFNGLINDEXIVPROC __glewIndexiv = NULL;
PFNGLINDEXSPROC __glewIndexs = NULL;
PFNGLINDEXSVPROC __glewIndexsv = NULL;
PFNGLINDEXUBPROC __glewIndexub = NULL;
PFNGLINDEXUBVPROC __glewIndexubv = NULL;
PFNGLINITNAMESPROC __glewInitNames = NULL;
PFNGLINTERLEAVEDARRAYSPROC __glewInterleavedArrays = NULL;
PFNGLISENABLEDPROC __glewIsEnabled = NULL;
PFNGLISLISTPROC __glewIsList = NULL;
PFNGLISTEXTUREPROC __glewIsTexture = NULL;
PFNGLLIGHTMODELFPROC __glewLightModelf = NULL;
PFNGLLIGHTMODELFVPROC __glewLightModelfv = NULL;
PFNGLLIGHTMODELIPROC __glewLightModeli = NULL;
PFNGLLIGHTMODELIVPROC __glewLightModeliv = NULL;
PFNGLLIGHTFPROC __glewLightf = NULL;
PFNGLLIGHTFVPROC __glewLightfv = NULL;
PFNGLLIGHTIPROC __glewLighti = NULL;
PFNGLLIGHTIVPROC __glewLightiv = NULL;
PFNGLLINESTIPPLEPROC __glewLineStipple = NULL;
PFNGLLINEWIDTHPROC __glewLineWidth = NULL;
PFNGLLISTBASEPROC __glewListBase = NULL;
PFNGLLOADIDENTITYPROC __glewLoadIdentity = NULL;
PFNGLLOADMATRIXDPROC __glewLoadMatrixd = NULL;
PFNGLLOADMATRIXFPROC __glewLoadMatrixf = NULL;
PFNGLLOADNAMEPROC __glewLoadName = NULL;
PFNGLLOGICOPPROC __glewLogicOp = NULL;
PFNGLMAP1DPROC __glewMap1d = NULL;
PFNGLMAP1FPROC __glewMap1f = NULL;
PFNGLMAP2DPROC __glewMap2d = NULL;
PFNGLMAP2FPROC __glewMap2f = NULL;
PFNGLMAPGRID1DPROC __glewMapGrid1d = NULL;
PFNGLMAPGRID1FPROC __glewMapGrid1f = NULL;
PFNGLMAPGRID2DPROC __glewMapGrid2d = NULL;
PFNGLMAPGRID2FPROC __glewMapGrid2f = NULL;
PFNGLMATERIALFPROC __glewMaterialf = NULL;
PFNGLMATERIALFVPROC __glewMaterialfv = NULL;
PFNGLMATERIALIPROC __glewMateriali = NULL;
PFNGLMATERIALIVPROC __glewMaterialiv = NULL;
PFNGLMATRIXMODEPROC __glewMatrixMode = NULL;
PFNGLMULTMATRIXDPROC __glewMultMatrixd = NULL;
PFNGLMULTMATRIXFPROC __glewMultMatrixf = NULL;
PFNGLNEWLISTPROC __glewNewList = NULL;
PFNGLNORMAL3BPROC __glewNormal3b = NULL;
PFNGLNORMAL3BVPROC __glewNormal3bv = NULL;
PFNGLNORMAL3DPROC __glewNormal3d = NULL;
PFNGLNORMAL3DVPROC __glewNormal3dv = NULL;
PFNGLNORMAL3FPROC __glewNormal3f = NULL;
PFNGLNORMAL3FVPROC __glewNormal3fv = NULL;
PFNGLNORMAL3IPROC __glewNormal3i = NULL;
PFNGLNORMAL3IVPROC __glewNormal3iv = NULL;
PFNGLNORMAL3SPROC __glewNormal3s = NULL;
PFNGLNORMAL3SVPROC __glewNormal3sv = NULL;
PFNGLNORMALPOINTERPROC __glewNormalPointer = NULL;
PFNGLORTHOPROC __glewOrtho = NULL;
PFNGLPASSTHROUGHPROC __glewPassThrough = NULL;
PFNGLPIXELMAPFVPROC __glewPixelMapfv = NULL;
PFNGLPIXELMAPUIVPROC __glewPixelMapuiv = NULL;
PFNGLPIXELMAPUSVPROC __glewPixelMapusv = NULL;
PFNGLPIXELSTOREFPROC __glewPixelStoref = NULL;
PFNGLPIXELSTOREIPROC __glewPixelStorei = NULL;
PFNGLPIXELTRANSFERFPROC __glewPixelTransferf = NULL;
PFNGLPIXELTRANSFERIPROC __glewPixelTransferi = NULL;
PFNGLPIXELZOOMPROC __glewPixelZoom = NULL;
PFNGLPOINTSIZEPROC __glewPointSize = NULL;
PFNGLPOLYGONMODEPROC __glewPolygonMode = NULL;
PFNGLPOLYGONOFFSETPROC __glewPolygonOffset = NULL;
PFNGLPOLYGONSTIPPLEPROC __glewPolygonStipple = NULL;
PFNGLPOPATTRIBPROC __glewPopAttrib = NULL;
PFNGLPOPCLIENTATTRIBPROC __glewPopClientAttrib = NULL;
PFNGLPOPMATRIXPROC __glewPopMatrix = NULL;
PFNGLPOPNAMEPROC __glewPopName = NULL;
PFNGLPRIORITIZETEXTURESPROC __glewPrioritizeTextures = NULL;
PFNGLPUSHATTRIBPROC __glewPushAttrib = NULL;
PFNGLPUSHCLIENTATTRIBPROC __glewPushClientAttrib = NULL;
PFNGLPUSHMATRIXPROC __glewPushMatrix = NULL;
PFNGLPUSHNAMEPROC __glewPushName = NULL;
PFNGLRASTERPOS2DPROC __glewRasterPos2d = NULL;
PFNGLRASTERPOS2DVPROC __glewRasterPos2dv = NULL;
PFNGLRASTERPOS2FPROC __glewRasterPos2f = NULL;
PFNGLRASTERPOS2FVPROC __glewRasterPos2fv = NULL;
PFNGLRASTERPOS2IPROC __glewRasterPos2i = NULL;
PFNGLRASTERPOS2IVPROC __glewRasterPos2iv = NULL;
PFNGLRASTERPOS2SPROC __glewRasterPos2s = NULL;
PFNGLRASTERPOS2SVPROC __glewRasterPos2sv = NULL;
PFNGLRASTERPOS3DPROC __glewRasterPos3d = NULL;
PFNGLRASTERPOS3DVPROC __glewRasterPos3dv = NULL;
PFNGLRASTERPOS3FPROC __glewRasterPos3f = NULL;
PFNGLRASTERPOS3FVPROC __glewRasterPos3fv = NULL;
PFNGLRASTERPOS3IPROC __glewRasterPos3i = NULL;
PFNGLRASTERPOS3IVPROC __glewRasterPos3iv = NULL;
PFNGLRASTERPOS3SPROC __glewRasterPos3s = NULL;
PFNGLRASTERPOS3SVPROC __glewRasterPos3sv = NULL;
PFNGLRASTERPOS4DPROC __glewRasterPos4d = NULL;
PFNGLRASTERPOS4DVPROC __glewRasterPos4dv = NULL;
PFNGLRASTERPOS4FPROC __glewRasterPos4f = NULL;
PFNGLRASTERPOS4FVPROC __glewRasterPos4fv = NULL;
PFNGLRASTERPOS4IPROC __glewRasterPos4i = NULL;
PFNGLRASTERPOS4IVPROC __glewRasterPos4iv = NULL;
PFNGLRASTERPOS4SPROC __glewRasterPos4s = NULL;
PFNGLRASTERPOS4SVPROC __glewRasterPos4sv = NULL;
PFNGLREADBUFFERPROC __glewReadBuffer = NULL;
PFNGLREADPIXELSPROC __glewReadPixels = NULL;
PFNGLRECTDPROC __glewRectd = NULL;
PFNGLRECTDVPROC __glewRectdv = NULL;
PFNGLRECTFPROC __glewRectf = NULL;
PFNGLRECTFVPROC __glewRectfv = NULL;
PFNGLRECTIPROC __glewRecti = NULL;
PFNGLRECTIVPROC __glewRectiv = NULL;
PFNGLRECTSPROC __glewRects = NULL;
PFNGLRECTSVPROC __glewRectsv = NULL;
PFNGLRENDERMODEPROC __glewRenderMode = NULL;
PFNGLROTATEDPROC __glewRotated = NULL;
PFNGLROTATEFPROC __glewRotatef = NULL;
PFNGLSCALEDPROC __glewScaled = NULL;
PFNGLSCALEFPROC __glewScalef = NULL;
PFNGLSCISSORPROC __glewScissor = NULL;
PFNGLSELECTBUFFERPROC __glewSelectBuffer = NULL;
PFNGLSHADEMODELPROC __glewShadeModel = NULL;
PFNGLSTENCILFUNCPROC __glewStencilFunc = NULL;
PFNGLSTENCILMASKPROC __glewStencilMask = NULL;
PFNGLSTENCILOPPROC __glewStencilOp = NULL;
PFNGLTEXCOORD1DPROC __glewTexCoord1d = NULL;
PFNGLTEXCOORD1DVPROC __glewTexCoord1dv = NULL;
PFNGLTEXCOORD1FPROC __glewTexCoord1f = NULL;
PFNGLTEXCOORD1FVPROC __glewTexCoord1fv = NULL;
PFNGLTEXCOORD1IPROC __glewTexCoord1i = NULL;
PFNGLTEXCOORD1IVPROC __glewTexCoord1iv = NULL;
PFNGLTEXCOORD1SPROC __glewTexCoord1s = NULL;
PFNGLTEXCOORD1SVPROC __glewTexCoord1sv = NULL;
PFNGLTEXCOORD2DPROC __glewTexCoord2d = NULL;
PFNGLTEXCOORD2DVPROC __glewTexCoord2dv = NULL;
PFNGLTEXCOORD2FPROC __glewTexCoord2f = NULL;
PFNGLTEXCOORD2FVPROC __glewTexCoord2fv = NULL;
PFNGLTEXCOORD2IPROC __glewTexCoord2i = NULL;
PFNGLTEXCOORD2IVPROC __glewTexCoord2iv = NULL;
PFNGLTEXCOORD2SPROC __glewTexCoord2s = NULL;
PFNGLTEXCOORD2SVPROC __glewTexCoord2sv = NULL;
PFNGLTEXCOORD3DPROC __glewTexCoord3d = NULL;
PFNGLTEXCOORD3DVPROC __glewTexCoord3dv = NULL;
PFNGLTEXCOORD3FPROC __glewTexCoord3f = NULL;
PFNGLTEXCOORD3FVPROC __glewTexCoord3fv = NULL;
PFNGLTEXCOORD3IPROC __glewTexCoord3i = NULL;
PFNGLTEXCOORD3IVPROC __glewTexCoord3iv = NULL;
PFNGLTEXCOORD3SPROC __glewTexCoord3s = NULL;
PFNGLTEXCOORD3SVPROC __glewTexCoord3sv = NULL;
PFNGLTEXCOORD4DPROC __glewTexCoord4d = NULL;
PFNGLTEXCOORD4DVPROC __glewTexCoord4dv = NULL;
PFNGLTEXCOORD4FPROC __glewTexCoord4f = NULL;
PFNGLTEXCOORD4FVPROC __glewTexCoord4fv = NULL;
PFNGLTEXCOORD4IPROC __glewTexCoord4i = NULL;
PFNGLTEXCOORD4IVPROC __glewTexCoord4iv = NULL;
PFNGLTEXCOORD4SPROC __glewTexCoord4s = NULL;
PFNGLTEXCOORD4SVPROC __glewTexCoord4sv = NULL;
PFNGLTEXCOORDPOINTERPROC __glewTexCoordPointer = NULL;
PFNGLTEXENVFPROC __glewTexEnvf = NULL;
PFNGLTEXENVFVPROC __glewTexEnvfv = NULL;
PFNGLTEXENVIPROC __glewTexEnvi = NULL;
PFNGLTEXENVIVPROC __glewTexEnviv = NULL;
PFNGLTEXGENDPROC __glewTexGend = NULL;
PFNGLTEXGENDVPROC __glewTexGendv = NULL;
PFNGLTEXGENFPROC __glewTexGenf = NULL;
PFNGLTEXGENFVPROC __glewTexGenfv = NULL;
PFNGLTEXGENIPROC __glewTexGeni = NULL;
PFNGLTEXGENIVPROC __glewTexGeniv = NULL;
PFNGLTEXIMAGE1DPROC __glewTexImage1D = NULL;
PFNGLTEXIMAGE2DPROC __glewTexImage2D = NULL;
PFNGLTEXPARAMETERFPROC __glewTexParameterf = NULL;
PFNGLTEXPARAMETERFVPROC __glewTexParameterfv = NULL;
PFNGLTEXPARAMETERIPROC __glewTexParameteri = NULL;
PFNGLTEXPARAMETERIVPROC __glewTexParameteriv = NULL;
PFNGLTEXSUBIMAGE1DPROC __glewTexSubImage1D = NULL;
PFNGLTEXSUBIMAGE2DPROC __glewTexSubImage2D = NULL;
PFNGLTRANSLATEDPROC __glewTranslated = NULL;
PFNGLTRANSLATEFPROC __glewTranslatef = NULL;
PFNGLVERTEX2DPROC __glewVertex2d = NULL;
PFNGLVERTEX2DVPROC __glewVertex2dv = NULL;
PFNGLVERTEX2FPROC __glewVertex2f = NULL;
PFNGLVERTEX2FVPROC __glewVertex2fv = NULL;
PFNGLVERTEX2IPROC __glewVertex2i = NULL;
PFNGLVERTEX2IVPROC __glewVertex2iv = NULL;
PFNGLVERTEX2SPROC __glewVertex2s = NULL;
PFNGLVERTEX2SVPROC __glewVertex2sv = NULL;
PFNGLVERTEX3DPROC __glewVertex3d = NULL;
PFNGLVERTEX3DVPROC __glewVertex3dv = NULL;
PFNGLVERTEX3FPROC __glewVertex3f = NULL;
PFNGLVERTEX3FVPROC __glewVertex3fv = NULL;
PFNGLVERTEX3IPROC __glewVertex3i = NULL;
PFNGLVERTEX3IVPROC __glewVertex3iv = NULL;
PFNGLVERTEX3SPROC __glewVertex3s = NULL;
PFNGLVERTEX3SVPROC __glewVertex3sv = NULL;
PFNGLVERTEX4DPROC __glewVertex4d = NULL;
PFNGLVERTEX4DVPROC __glewVertex4dv = NULL;
PFNGLVERTEX4FPROC __glewVertex4f = NULL;
PFNGLVERTEX4FVPROC __glewVertex4fv = NULL;
PFNGLVERTEX4IPROC __glewVertex4i = NULL;
PFNGLVERTEX4IVPROC __glewVertex4iv = NULL;
PFNGLVERTEX4SPROC __glewVertex4s = NULL;
PFNGLVERTEX4SVPROC __glewVertex4sv = NULL;
PFNGLVERTEXPOINTERPROC __glewVertexPointer = NULL;
PFNGLVIEWPORTPROC __glewViewport = NULL;

PFNGLCOPYTEXSUBIMAGE3DPROC __glewCopyTexSubImage3D = NULL;
PFNGLDRAWRANGEELEMENTSPROC __glewDrawRangeElements = NULL;
PFNGLTEXIMAGE3DPROC __glewTexImage3D = NULL;
PFNGLTEXSUBIMAGE3DPROC __glewTexSubImage3D = NULL;

PFNGLACTIVETEXTUREPROC __glewActiveTexture = NULL;
PFNGLCLIENTACTIVETEXTUREPROC __glewClientActiveTexture = NULL;
PFNGLCOMPRESSEDTEXIMAGE1DPROC __glewCompressedTexImage1D = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC __glewCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DPROC __glewCompressedTexImage3D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC __glewCompressedTexSubImage1D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC __glewCompressedTexSubImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC __glewCompressedTexSubImage3D = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEPROC __glewGetCompressedTexImage = NULL;
PFNGLLOADTRANSPOSEMATRIXDPROC __glewLoadTransposeMatrixd = NULL;
PFNGLLOADTRANSPOSEMATRIXFPROC __glewLoadTransposeMatrixf = NULL;
PFNGLMULTTRANSPOSEMATRIXDPROC __glewMultTransposeMatrixd = NULL;
PFNGLMULTTRANSPOSEMATRIXFPROC __glewMultTransposeMatrixf = NULL;
PFNGLMULTITEXCOORD1DPROC __glewMultiTexCoord1d = NULL;
PFNGLMULTITEXCOORD1DVPROC __glewMultiTexCoord1dv = NULL;
PFNGLMULTITEXCOORD1FPROC __glewMultiTexCoord1f = NULL;
PFNGLMULTITEXCOORD1FVPROC __glewMultiTexCoord1fv = NULL;
PFNGLMULTITEXCOORD1IPROC __glewMultiTexCoord1i = NULL;
PFNGLMULTITEXCOORD1IVPROC __glewMultiTexCoord1iv = NULL;
PFNGLMULTITEXCOORD1SPROC __glewMultiTexCoord1s = NULL;
PFNGLMULTITEXCOORD1SVPROC __glewMultiTexCoord1sv = NULL;
PFNGLMULTITEXCOORD2DPROC __glewMultiTexCoord2d = NULL;
PFNGLMULTITEXCOORD2DVPROC __glewMultiTexCoord2dv = NULL;
PFNGLMULTITEXCOORD2FPROC __glewMultiTexCoord2f = NULL;
PFNGLMULTITEXCOORD2FVPROC __glewMultiTexCoord2fv = NULL;
PFNGLMULTITEXCOORD2IPROC __glewMultiTexCoord2i = NULL;
PFNGLMULTITEXCOORD2IVPROC __glewMultiTexCoord2iv = NULL;
PFNGLMULTITEXCOORD2SPROC __glewMultiTexCoord2s = NULL;
PFNGLMULTITEXCOORD2SVPROC __glewMultiTexCoord2sv = NULL;
PFNGLMULTITEXCOORD3DPROC __glewMultiTexCoord3d = NULL;
PFNGLMULTITEXCOORD3DVPROC __glewMultiTexCoord3dv = NULL;
PFNGLMULTITEXCOORD3FPROC __glewMultiTexCoord3f = NULL;
PFNGLMULTITEXCOORD3FVPROC __glewMultiTexCoord3fv = NULL;
PFNGLMULTITEXCOORD3IPROC __glewMultiTexCoord3i = NULL;
PFNGLMULTITEXCOORD3IVPROC __glewMultiTexCoord3iv = NULL;
PFNGLMULTITEXCOORD3SPROC __glewMultiTexCoord3s = NULL;
PFNGLMULTITEXCOORD3SVPROC __glewMultiTexCoord3sv = NULL;
PFNGLMULTITEXCOORD4DPROC __glewMultiTexCoord4d = NULL;
PFNGLMULTITEXCOORD4DVPROC __glewMultiTexCoord4dv = NULL;
PFNGLMULTITEXCOORD4FPROC __glewMultiTexCoord4f = NULL;
PFNGLMULTITEXCOORD4FVPROC __glewMultiTexCoord4fv = NULL;
PFNGLMULTITEXCOORD4IPROC __glewMultiTexCoord4i = NULL;
PFNGLMULTITEXCOORD4IVPROC __glewMultiTexCoord4iv = NULL;
PFNGLMULTITEXCOORD4SPROC __glewMultiTexCoord4s = NULL;
PFNGLMULTITEXCOORD4SVPROC __glewMultiTexCoord4sv = NULL;
PFNGLSAMPLECOVERAGEPROC __glewSampleCoverage = NULL;

PFNGLBLENDCOLORPROC __glewBlendColor = NULL;
PFNGLBLENDEQUATIONPROC __glewBlendEquation = NULL;
PFNGLBLENDFUNCSEPARATEPROC __glewBlendFuncSeparate = NULL;
PFNGLFOGCOORDPOINTERPROC __glewFogCoordPointer = NULL;
PFNGLFOGCOORDDPROC __glewFogCoordd = NULL;
PFNGLFOGCOORDDVPROC __glewFogCoorddv = NULL;
PFNGLFOGCOORDFPROC __glewFogCoordf = NULL;
PFNGLFOGCOORDFVPROC __glewFogCoordfv = NULL;
PFNGLMULTIDRAWARRAYSPROC __glewMultiDrawArrays = NULL;
PFNGLMULTIDRAWELEMENTSPROC __glewMultiDrawElements = NULL;
PFNGLPOINTPARAMETERFPROC __glewPointParameterf = NULL;
PFNGLPOINTPARAMETERFVPROC __glewPointParameterfv = NULL;
PFNGLPOINTPARAMETERIPROC __glewPointParameteri = NULL;
PFNGLPOINTPARAMETERIVPROC __glewPointParameteriv = NULL;
PFNGLSECONDARYCOLOR3BPROC __glewSecondaryColor3b = NULL;
PFNGLSECONDARYCOLOR3BVPROC __glewSecondaryColor3bv = NULL;
PFNGLSECONDARYCOLOR3DPROC __glewSecondaryColor3d = NULL;
PFNGLSECONDARYCOLOR3DVPROC __glewSecondaryColor3dv = NULL;
PFNGLSECONDARYCOLOR3FPROC __glewSecondaryColor3f = NULL;
PFNGLSECONDARYCOLOR3FVPROC __glewSecondaryColor3fv = NULL;
PFNGLSECONDARYCOLOR3IPROC __glewSecondaryColor3i = NULL;
PFNGLSECONDARYCOLOR3IVPROC __glewSecondaryColor3iv = NULL;
PFNGLSECONDARYCOLOR3SPROC __glewSecondaryColor3s = NULL;
PFNGLSECONDARYCOLOR3SVPROC __glewSecondaryColor3sv = NULL;
PFNGLSECONDARYCOLOR3UBPROC __glewSecondaryColor3ub = NULL;
PFNGLSECONDARYCOLOR3UBVPROC __glewSecondaryColor3ubv = NULL;
PFNGLSECONDARYCOLOR3UIPROC __glewSecondaryColor3ui = NULL;
PFNGLSECONDARYCOLOR3UIVPROC __glewSecondaryColor3uiv = NULL;
PFNGLSECONDARYCOLOR3USPROC __glewSecondaryColor3us = NULL;
PFNGLSECONDARYCOLOR3USVPROC __glewSecondaryColor3usv = NULL;
PFNGLSECONDARYCOLORPOINTERPROC __glewSecondaryColorPointer = NULL;
PFNGLWINDOWPOS2DPROC __glewWindowPos2d = NULL;
PFNGLWINDOWPOS2DVPROC __glewWindowPos2dv = NULL;
PFNGLWINDOWPOS2FPROC __glewWindowPos2f = NULL;
PFNGLWINDOWPOS2FVPROC __glewWindowPos2fv = NULL;
PFNGLWINDOWPOS2IPROC __glewWindowPos2i = NULL;
PFNGLWINDOWPOS2IVPROC __glewWindowPos2iv = NULL;
PFNGLWINDOWPOS2SPROC __glewWindowPos2s = NULL;
PFNGLWINDOWPOS2SVPROC __glewWindowPos2sv = NULL;
PFNGLWINDOWPOS3DPROC __glewWindowPos3d = NULL;
PFNGLWINDOWPOS3DVPROC __glewWindowPos3dv = NULL;
PFNGLWINDOWPOS3FPROC __glewWindowPos3f = NULL;
PFNGLWINDOWPOS3FVPROC __glewWindowPos3fv = NULL;
PFNGLWINDOWPOS3IPROC __glewWindowPos3i = NULL;
PFNGLWINDOWPOS3IVPROC __glewWindowPos3iv = NULL;
PFNGLWINDOWPOS3SPROC __glewWindowPos3s = NULL;
PFNGLWINDOWPOS3SVPROC __glewWindowPos3sv = NULL;

PFNGLBEGINQUERYPROC __glewBeginQuery = NULL;
PFNGLBINDBUFFERPROC __glewBindBuffer = NULL;
PFNGLBUFFERDATAPROC __glewBufferData = NULL;
PFNGLBUFFERSUBDATAPROC __glewBufferSubData = NULL;
PFNGLDELETEBUFFERSPROC __glewDeleteBuffers = NULL;
PFNGLDELETEQUERIESPROC __glewDeleteQueries = NULL;
PFNGLENDQUERYPROC __glewEndQuery = NULL;
PFNGLGENBUFFERSPROC __glewGenBuffers = NULL;
PFNGLGENQUERIESPROC __glewGenQueries = NULL;
PFNGLGETBUFFERPARAMETERIVPROC __glewGetBufferParameteriv = NULL;
PFNGLGETBUFFERPOINTERVPROC __glewGetBufferPointerv = NULL;
PFNGLGETBUFFERSUBDATAPROC __glewGetBufferSubData = NULL;
PFNGLGETQUERYOBJECTIVPROC __glewGetQueryObjectiv = NULL;
PFNGLGETQUERYOBJECTUIVPROC __glewGetQueryObjectuiv = NULL;
PFNGLGETQUERYIVPROC __glewGetQueryiv = NULL;
PFNGLISBUFFERPROC __glewIsBuffer = NULL;
PFNGLISQUERYPROC __glewIsQuery = NULL;
PFNGLMAPBUFFERPROC __glewMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC __glewUnmapBuffer = NULL;

PFNGLATTACHSHADERPROC __glewAttachShader = NULL;
PFNGLBINDATTRIBLOCATIONPROC __glewBindAttribLocation = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC __glewBlendEquationSeparate = NULL;
PFNGLCOMPILESHADERPROC __glewCompileShader = NULL;
PFNGLCREATEPROGRAMPROC __glewCreateProgram = NULL;
PFNGLCREATESHADERPROC __glewCreateShader = NULL;
PFNGLDELETEPROGRAMPROC __glewDeleteProgram = NULL;
PFNGLDELETESHADERPROC __glewDeleteShader = NULL;
PFNGLDETACHSHADERPROC __glewDetachShader = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC __glewDisableVertexAttribArray = NULL;
PFNGLDRAWBUFFERSPROC __glewDrawBuffers = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC __glewEnableVertexAttribArray = NULL;
PFNGLGETACTIVEATTRIBPROC __glewGetActiveAttrib = NULL;
PFNGLGETACTIVEUNIFORMPROC __glewGetActiveUniform = NULL;
PFNGLGETATTACHEDSHADERSPROC __glewGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATIONPROC __glewGetAttribLocation = NULL;
PFNGLGETPROGRAMINFOLOGPROC __glewGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVPROC __glewGetProgramiv = NULL;
PFNGLGETSHADERINFOLOGPROC __glewGetShaderInfoLog = NULL;
PFNGLGETSHADERSOURCEPROC __glewGetShaderSource = NULL;
PFNGLGETSHADERIVPROC __glewGetShaderiv = NULL;
PFNGLGETUNIFORMLOCATIONPROC __glewGetUniformLocation = NULL;
PFNGLGETUNIFORMFVPROC __glewGetUniformfv = NULL;
PFNGLGETUNIFORMIVPROC __glewGetUniformiv = NULL;
PFNGLGETVERTEXATTRIBPOINTERVPROC __glewGetVertexAttribPointerv = NULL;
PFNGLGETVERTEXATTRIBDVPROC __glewGetVertexAttribdv = NULL;
PFNGLGETVERTEXATTRIBFVPROC __glewGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIVPROC __glewGetVertexAttribiv = NULL;
PFNGLISPROGRAMPROC __glewIsProgram = NULL;
PFNGLISSHADERPROC __glewIsShader = NULL;
PFNGLLINKPROGRAMPROC __glewLinkProgram = NULL;
PFNGLSHADERSOURCEPROC __glewShaderSource = NULL;
PFNGLSTENCILFUNCSEPARATEPROC __glewStencilFuncSeparate = NULL;
PFNGLSTENCILMASKSEPARATEPROC __glewStencilMaskSeparate = NULL;
PFNGLSTENCILOPSEPARATEPROC __glewStencilOpSeparate = NULL;
PFNGLUNIFORM1FPROC __glewUniform1f = NULL;
PFNGLUNIFORM1FVPROC __glewUniform1fv = NULL;
PFNGLUNIFORM1IPROC __glewUniform1i = NULL;
PFNGLUNIFORM1IVPROC __glewUniform1iv = NULL;
PFNGLUNIFORM2FPROC __glewUniform2f = NULL;
PFNGLUNIFORM2FVPROC __glewUniform2fv = NULL;
PFNGLUNIFORM2IPROC __glewUniform2i = NULL;
PFNGLUNIFORM2IVPROC __glewUniform2iv = NULL;
PFNGLUNIFORM3FPROC __glewUniform3f = NULL;
PFNGLUNIFORM3FVPROC __glewUniform3fv = NULL;
PFNGLUNIFORM3IPROC __glewUniform3i = NULL;
PFNGLUNIFORM3IVPROC __glewUniform3iv = NULL;
PFNGLUNIFORM4FPROC __glewUniform4f = NULL;
PFNGLUNIFORM4FVPROC __glewUniform4fv = NULL;
PFNGLUNIFORM4IPROC __glewUniform4i = NULL;
PFNGLUNIFORM4IVPROC __glewUniform4iv = NULL;
PFNGLUNIFORMMATRIX2FVPROC __glewUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC __glewUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC __glewUniformMatrix4fv = NULL;
PFNGLUSEPROGRAMPROC __glewUseProgram = NULL;
PFNGLVALIDATEPROGRAMPROC __glewValidateProgram = NULL;
PFNGLVERTEXATTRIB1DPROC __glewVertexAttrib1d = NULL;
PFNGLVERTEXATTRIB1DVPROC __glewVertexAttrib1dv = NULL;
PFNGLVERTEXATTRIB1FPROC __glewVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FVPROC __glewVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB1SPROC __glewVertexAttrib1s = NULL;
PFNGLVERTEXATTRIB1SVPROC __glewVertexAttrib1sv = NULL;
PFNGLVERTEXATTRIB2DPROC __glewVertexAttrib2d = NULL;
PFNGLVERTEXATTRIB2DVPROC __glewVertexAttrib2dv = NULL;
PFNGLVERTEXATTRIB2FPROC __glewVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FVPROC __glewVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB2SPROC __glewVertexAttrib2s = NULL;
PFNGLVERTEXATTRIB2SVPROC __glewVertexAttrib2sv = NULL;
PFNGLVERTEXATTRIB3DPROC __glewVertexAttrib3d = NULL;
PFNGLVERTEXATTRIB3DVPROC __glewVertexAttrib3dv = NULL;
PFNGLVERTEXATTRIB3FPROC __glewVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FVPROC __glewVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB3SPROC __glewVertexAttrib3s = NULL;
PFNGLVERTEXATTRIB3SVPROC __glewVertexAttrib3sv = NULL;
PFNGLVERTEXATTRIB4NBVPROC __glewVertexAttrib4Nbv = NULL;
PFNGLVERTEXATTRIB4NIVPROC __glewVertexAttrib4Niv = NULL;
PFNGLVERTEXATTRIB4NSVPROC __glewVertexAttrib4Nsv = NULL;
PFNGLVERTEXATTRIB4NUBPROC __glewVertexAttrib4Nub = NULL;
PFNGLVERTEXATTRIB4NUBVPROC __glewVertexAttrib4Nubv = NULL;
PFNGLVERTEXATTRIB4NUIVPROC __glewVertexAttrib4Nuiv = NULL;
PFNGLVERTEXATTRIB4NUSVPROC __glewVertexAttrib4Nusv = NULL;
PFNGLVERTEXATTRIB4BVPROC __glewVertexAttrib4bv = NULL;
PFNGLVERTEXATTRIB4DPROC __glewVertexAttrib4d = NULL;
PFNGLVERTEXATTRIB4DVPROC __glewVertexAttrib4dv = NULL;
PFNGLVERTEXATTRIB4FPROC __glewVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FVPROC __glewVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIB4IVPROC __glewVertexAttrib4iv = NULL;
PFNGLVERTEXATTRIB4SPROC __glewVertexAttrib4s = NULL;
PFNGLVERTEXATTRIB4SVPROC __glewVertexAttrib4sv = NULL;
PFNGLVERTEXATTRIB4UBVPROC __glewVertexAttrib4ubv = NULL;
PFNGLVERTEXATTRIB4UIVPROC __glewVertexAttrib4uiv = NULL;
PFNGLVERTEXATTRIB4USVPROC __glewVertexAttrib4usv = NULL;
PFNGLVERTEXATTRIBPOINTERPROC __glewVertexAttribPointer = NULL;

PFNGLUNIFORMMATRIX2X3FVPROC __glewUniformMatrix2x3fv = NULL;
PFNGLUNIFORMMATRIX2X4FVPROC __glewUniformMatrix2x4fv = NULL;
PFNGLUNIFORMMATRIX3X2FVPROC __glewUniformMatrix3x2fv = NULL;
PFNGLUNIFORMMATRIX3X4FVPROC __glewUniformMatrix3x4fv = NULL;
PFNGLUNIFORMMATRIX4X2FVPROC __glewUniformMatrix4x2fv = NULL;
PFNGLUNIFORMMATRIX4X3FVPROC __glewUniformMatrix4x3fv = NULL;

PFNGLBEGINCONDITIONALRENDERPROC __glewBeginConditionalRender = NULL;
PFNGLBEGINTRANSFORMFEEDBACKPROC __glewBeginTransformFeedback = NULL;
PFNGLBINDBUFFERBASEPROC __glewBindBufferBase = NULL;
PFNGLBINDBUFFERRANGEPROC __glewBindBufferRange = NULL;
PFNGLBINDFRAGDATALOCATIONPROC __glewBindFragDataLocation = NULL;
PFNGLCLAMPCOLORPROC __glewClampColor = NULL;
PFNGLCLEARBUFFERFIPROC __glewClearBufferfi = NULL;
PFNGLCLEARBUFFERFVPROC __glewClearBufferfv = NULL;
PFNGLCLEARBUFFERIVPROC __glewClearBufferiv = NULL;
PFNGLCLEARBUFFERUIVPROC __glewClearBufferuiv = NULL;
PFNGLCOLORMASKIPROC __glewColorMaski = NULL;
PFNGLDISABLEIPROC __glewDisablei = NULL;
PFNGLENABLEIPROC __glewEnablei = NULL;
PFNGLENDCONDITIONALRENDERPROC __glewEndConditionalRender = NULL;
PFNGLENDTRANSFORMFEEDBACKPROC __glewEndTransformFeedback = NULL;
PFNGLGETBOOLEANI_VPROC __glewGetBooleani_v = NULL;
PFNGLGETFRAGDATALOCATIONPROC __glewGetFragDataLocation = NULL;
PFNGLGETINTEGERI_VPROC __glewGetIntegeri_v = NULL;
PFNGLGETSTRINGIPROC __glewGetStringi = NULL;
PFNGLGETTEXPARAMETERIIVPROC __glewGetTexParameterIiv = NULL;
PFNGLGETTEXPARAMETERIUIVPROC __glewGetTexParameterIuiv = NULL;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC __glewGetTransformFeedbackVarying = NULL;
PFNGLGETUNIFORMUIVPROC __glewGetUniformuiv = NULL;
PFNGLGETVERTEXATTRIBIIVPROC __glewGetVertexAttribIiv = NULL;
PFNGLGETVERTEXATTRIBIUIVPROC __glewGetVertexAttribIuiv = NULL;
PFNGLISENABLEDIPROC __glewIsEnabledi = NULL;
PFNGLTEXPARAMETERIIVPROC __glewTexParameterIiv = NULL;
PFNGLTEXPARAMETERIUIVPROC __glewTexParameterIuiv = NULL;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC __glewTransformFeedbackVaryings = NULL;
PFNGLUNIFORM1UIPROC __glewUniform1ui = NULL;
PFNGLUNIFORM1UIVPROC __glewUniform1uiv = NULL;
PFNGLUNIFORM2UIPROC __glewUniform2ui = NULL;
PFNGLUNIFORM2UIVPROC __glewUniform2uiv = NULL;
PFNGLUNIFORM3UIPROC __glewUniform3ui = NULL;
PFNGLUNIFORM3UIVPROC __glewUniform3uiv = NULL;
PFNGLUNIFORM4UIPROC __glewUniform4ui = NULL;
PFNGLUNIFORM4UIVPROC __glewUniform4uiv = NULL;
PFNGLVERTEXATTRIBI1IPROC __glewVertexAttribI1i = NULL;
PFNGLVERTEXATTRIBI1IVPROC __glewVertexAttribI1iv = NULL;
PFNGLVERTEXATTRIBI1UIPROC __glewVertexAttribI1ui = NULL;
PFNGLVERTEXATTRIBI1UIVPROC __glewVertexAttribI1uiv = NULL;
PFNGLVERTEXATTRIBI2IPROC __glewVertexAttribI2i = NULL;
PFNGLVERTEXATTRIBI2IVPROC __glewVertexAttribI2iv = NULL;
PFNGLVERTEXATTRIBI2UIPROC __glewVertexAttribI2ui = NULL;
PFNGLVERTEXATTRIBI2UIVPROC __glewVertexAttribI2uiv = NULL;
PFNGLVERTEXATTRIBI3IPROC __glewVertexAttribI3i = NULL;
PFNGLVERTEXATTRIBI3IVPROC __glewVertexAttribI3iv = NULL;
PFNGLVERTEXATTRIBI3UIPROC __glewVertexAttribI3ui = NULL;
PFNGLVERTEXATTRIBI3UIVPROC __glewVertexAttribI3uiv = NULL;
PFNGLVERTEXATTRIBI4BVPROC __glewVertexAttribI4bv = NULL;
PFNGLVERTEXATTRIBI4IPROC __glewVertexAttribI4i = NULL;
PFNGLVERTEXATTRIBI4IVPROC __glewVertexAttribI4iv = NULL;
PFNGLVERTEXATTRIBI4SVPROC __glewVertexAttribI4sv = NULL;
PFNGLVERTEXATTRIBI4UBVPROC __glewVertexAttribI4ubv = NULL;
PFNGLVERTEXATTRIBI4UIPROC __glewVertexAttribI4ui = NULL;
PFNGLVERTEXATTRIBI4UIVPROC __glewVertexAttribI4uiv = NULL;
PFNGLVERTEXATTRIBI4USVPROC __glewVertexAttribI4usv = NULL;
PFNGLVERTEXATTRIBIPOINTERPROC __glewVertexAttribIPointer = NULL;

PFNGLTBUFFERMASK3DFXPROC __glewTbufferMask3DFX = NULL;

PFNGLDRAWELEMENTARRAYAPPLEPROC __glewDrawElementArrayAPPLE = NULL;
PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC __glewDrawRangeElementArrayAPPLE = NULL;
PFNGLELEMENTPOINTERAPPLEPROC __glewElementPointerAPPLE = NULL;
PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC __glewMultiDrawElementArrayAPPLE = NULL;
PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC __glewMultiDrawRangeElementArrayAPPLE = NULL;

PFNGLDELETEFENCESAPPLEPROC __glewDeleteFencesAPPLE = NULL;
PFNGLFINISHFENCEAPPLEPROC __glewFinishFenceAPPLE = NULL;
PFNGLFINISHOBJECTAPPLEPROC __glewFinishObjectAPPLE = NULL;
PFNGLGENFENCESAPPLEPROC __glewGenFencesAPPLE = NULL;
PFNGLISFENCEAPPLEPROC __glewIsFenceAPPLE = NULL;
PFNGLSETFENCEAPPLEPROC __glewSetFenceAPPLE = NULL;
PFNGLTESTFENCEAPPLEPROC __glewTestFenceAPPLE = NULL;
PFNGLTESTOBJECTAPPLEPROC __glewTestObjectAPPLE = NULL;

PFNGLBUFFERPARAMETERIAPPLEPROC __glewBufferParameteriAPPLE = NULL;
PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC __glewFlushMappedBufferRangeAPPLE = NULL;

PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC __glewGetTexParameterPointervAPPLE = NULL;
PFNGLTEXTURERANGEAPPLEPROC __glewTextureRangeAPPLE = NULL;

PFNGLBINDVERTEXARRAYAPPLEPROC __glewBindVertexArrayAPPLE = NULL;
PFNGLDELETEVERTEXARRAYSAPPLEPROC __glewDeleteVertexArraysAPPLE = NULL;
PFNGLGENVERTEXARRAYSAPPLEPROC __glewGenVertexArraysAPPLE = NULL;
PFNGLISVERTEXARRAYAPPLEPROC __glewIsVertexArrayAPPLE = NULL;

PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC __glewFlushVertexArrayRangeAPPLE = NULL;
PFNGLVERTEXARRAYPARAMETERIAPPLEPROC __glewVertexArrayParameteriAPPLE = NULL;
PFNGLVERTEXARRAYRANGEAPPLEPROC __glewVertexArrayRangeAPPLE = NULL;

PFNGLCLAMPCOLORARBPROC __glewClampColorARB = NULL;

PFNGLDRAWBUFFERSARBPROC __glewDrawBuffersARB = NULL;

PFNGLDRAWARRAYSINSTANCEDARBPROC __glewDrawArraysInstancedARB = NULL;
PFNGLDRAWELEMENTSINSTANCEDARBPROC __glewDrawElementsInstancedARB = NULL;

PFNGLBINDFRAMEBUFFERPROC __glewBindFramebuffer = NULL;
PFNGLBINDRENDERBUFFERPROC __glewBindRenderbuffer = NULL;
PFNGLBLITFRAMEBUFFERPROC __glewBlitFramebuffer = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC __glewCheckFramebufferStatus = NULL;
PFNGLDELETEFRAMEBUFFERSPROC __glewDeleteFramebuffers = NULL;
PFNGLDELETERENDERBUFFERSPROC __glewDeleteRenderbuffers = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC __glewFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE1DPROC __glewFramebufferTexture1D = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC __glewFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERTEXTURE3DPROC __glewFramebufferTexture3D = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERPROC __glewFramebufferTextureLayer = NULL;
PFNGLGENFRAMEBUFFERSPROC __glewGenFramebuffers = NULL;
PFNGLGENRENDERBUFFERSPROC __glewGenRenderbuffers = NULL;
PFNGLGENERATEMIPMAPPROC __glewGenerateMipmap = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC __glewGetFramebufferAttachmentParameteriv = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVPROC __glewGetRenderbufferParameteriv = NULL;
PFNGLISFRAMEBUFFERPROC __glewIsFramebuffer = NULL;
PFNGLISRENDERBUFFERPROC __glewIsRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC __glewRenderbufferStorage = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC __glewRenderbufferStorageMultisample = NULL;

PFNGLFRAMEBUFFERTEXTUREARBPROC __glewFramebufferTextureARB = NULL;
PFNGLFRAMEBUFFERTEXTUREFACEARBPROC __glewFramebufferTextureFaceARB = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERARBPROC __glewFramebufferTextureLayerARB = NULL;
PFNGLPROGRAMPARAMETERIARBPROC __glewProgramParameteriARB = NULL;

PFNGLCOLORSUBTABLEPROC __glewColorSubTable = NULL;
PFNGLCOLORTABLEPROC __glewColorTable = NULL;
PFNGLCOLORTABLEPARAMETERFVPROC __glewColorTableParameterfv = NULL;
PFNGLCOLORTABLEPARAMETERIVPROC __glewColorTableParameteriv = NULL;
PFNGLCONVOLUTIONFILTER1DPROC __glewConvolutionFilter1D = NULL;
PFNGLCONVOLUTIONFILTER2DPROC __glewConvolutionFilter2D = NULL;
PFNGLCONVOLUTIONPARAMETERFPROC __glewConvolutionParameterf = NULL;
PFNGLCONVOLUTIONPARAMETERFVPROC __glewConvolutionParameterfv = NULL;
PFNGLCONVOLUTIONPARAMETERIPROC __glewConvolutionParameteri = NULL;
PFNGLCONVOLUTIONPARAMETERIVPROC __glewConvolutionParameteriv = NULL;
PFNGLCOPYCOLORSUBTABLEPROC __glewCopyColorSubTable = NULL;
PFNGLCOPYCOLORTABLEPROC __glewCopyColorTable = NULL;
PFNGLCOPYCONVOLUTIONFILTER1DPROC __glewCopyConvolutionFilter1D = NULL;
PFNGLCOPYCONVOLUTIONFILTER2DPROC __glewCopyConvolutionFilter2D = NULL;
PFNGLGETCOLORTABLEPROC __glewGetColorTable = NULL;
PFNGLGETCOLORTABLEPARAMETERFVPROC __glewGetColorTableParameterfv = NULL;
PFNGLGETCOLORTABLEPARAMETERIVPROC __glewGetColorTableParameteriv = NULL;
PFNGLGETCONVOLUTIONFILTERPROC __glewGetConvolutionFilter = NULL;
PFNGLGETCONVOLUTIONPARAMETERFVPROC __glewGetConvolutionParameterfv = NULL;
PFNGLGETCONVOLUTIONPARAMETERIVPROC __glewGetConvolutionParameteriv = NULL;
PFNGLGETHISTOGRAMPROC __glewGetHistogram = NULL;
PFNGLGETHISTOGRAMPARAMETERFVPROC __glewGetHistogramParameterfv = NULL;
PFNGLGETHISTOGRAMPARAMETERIVPROC __glewGetHistogramParameteriv = NULL;
PFNGLGETMINMAXPROC __glewGetMinmax = NULL;
PFNGLGETMINMAXPARAMETERFVPROC __glewGetMinmaxParameterfv = NULL;
PFNGLGETMINMAXPARAMETERIVPROC __glewGetMinmaxParameteriv = NULL;
PFNGLGETSEPARABLEFILTERPROC __glewGetSeparableFilter = NULL;
PFNGLHISTOGRAMPROC __glewHistogram = NULL;
PFNGLMINMAXPROC __glewMinmax = NULL;
PFNGLRESETHISTOGRAMPROC __glewResetHistogram = NULL;
PFNGLRESETMINMAXPROC __glewResetMinmax = NULL;
PFNGLSEPARABLEFILTER2DPROC __glewSeparableFilter2D = NULL;

PFNGLVERTEXATTRIBDIVISORARBPROC __glewVertexAttribDivisorARB = NULL;

PFNGLFLUSHMAPPEDBUFFERRANGEPROC __glewFlushMappedBufferRange = NULL;
PFNGLMAPBUFFERRANGEPROC __glewMapBufferRange = NULL;

PFNGLCURRENTPALETTEMATRIXARBPROC __glewCurrentPaletteMatrixARB = NULL;
PFNGLMATRIXINDEXPOINTERARBPROC __glewMatrixIndexPointerARB = NULL;
PFNGLMATRIXINDEXUBVARBPROC __glewMatrixIndexubvARB = NULL;
PFNGLMATRIXINDEXUIVARBPROC __glewMatrixIndexuivARB = NULL;
PFNGLMATRIXINDEXUSVARBPROC __glewMatrixIndexusvARB = NULL;

PFNGLSAMPLECOVERAGEARBPROC __glewSampleCoverageARB = NULL;

PFNGLACTIVETEXTUREARBPROC __glewActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC __glewClientActiveTextureARB = NULL;
PFNGLMULTITEXCOORD1DARBPROC __glewMultiTexCoord1dARB = NULL;
PFNGLMULTITEXCOORD1DVARBPROC __glewMultiTexCoord1dvARB = NULL;
PFNGLMULTITEXCOORD1FARBPROC __glewMultiTexCoord1fARB = NULL;
PFNGLMULTITEXCOORD1FVARBPROC __glewMultiTexCoord1fvARB = NULL;
PFNGLMULTITEXCOORD1IARBPROC __glewMultiTexCoord1iARB = NULL;
PFNGLMULTITEXCOORD1IVARBPROC __glewMultiTexCoord1ivARB = NULL;
PFNGLMULTITEXCOORD1SARBPROC __glewMultiTexCoord1sARB = NULL;
PFNGLMULTITEXCOORD1SVARBPROC __glewMultiTexCoord1svARB = NULL;
PFNGLMULTITEXCOORD2DARBPROC __glewMultiTexCoord2dARB = NULL;
PFNGLMULTITEXCOORD2DVARBPROC __glewMultiTexCoord2dvARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC __glewMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2FVARBPROC __glewMultiTexCoord2fvARB = NULL;
PFNGLMULTITEXCOORD2IARBPROC __glewMultiTexCoord2iARB = NULL;
PFNGLMULTITEXCOORD2IVARBPROC __glewMultiTexCoord2ivARB = NULL;
PFNGLMULTITEXCOORD2SARBPROC __glewMultiTexCoord2sARB = NULL;
PFNGLMULTITEXCOORD2SVARBPROC __glewMultiTexCoord2svARB = NULL;
PFNGLMULTITEXCOORD3DARBPROC __glewMultiTexCoord3dARB = NULL;
PFNGLMULTITEXCOORD3DVARBPROC __glewMultiTexCoord3dvARB = NULL;
PFNGLMULTITEXCOORD3FARBPROC __glewMultiTexCoord3fARB = NULL;
PFNGLMULTITEXCOORD3FVARBPROC __glewMultiTexCoord3fvARB = NULL;
PFNGLMULTITEXCOORD3IARBPROC __glewMultiTexCoord3iARB = NULL;
PFNGLMULTITEXCOORD3IVARBPROC __glewMultiTexCoord3ivARB = NULL;
PFNGLMULTITEXCOORD3SARBPROC __glewMultiTexCoord3sARB = NULL;
PFNGLMULTITEXCOORD3SVARBPROC __glewMultiTexCoord3svARB = NULL;
PFNGLMULTITEXCOORD4DARBPROC __glewMultiTexCoord4dARB = NULL;
PFNGLMULTITEXCOORD4DVARBPROC __glewMultiTexCoord4dvARB = NULL;
PFNGLMULTITEXCOORD4FARBPROC __glewMultiTexCoord4fARB = NULL;
PFNGLMULTITEXCOORD4FVARBPROC __glewMultiTexCoord4fvARB = NULL;
PFNGLMULTITEXCOORD4IARBPROC __glewMultiTexCoord4iARB = NULL;
PFNGLMULTITEXCOORD4IVARBPROC __glewMultiTexCoord4ivARB = NULL;
PFNGLMULTITEXCOORD4SARBPROC __glewMultiTexCoord4sARB = NULL;
PFNGLMULTITEXCOORD4SVARBPROC __glewMultiTexCoord4svARB = NULL;

PFNGLBEGINQUERYARBPROC __glewBeginQueryARB = NULL;
PFNGLDELETEQUERIESARBPROC __glewDeleteQueriesARB = NULL;
PFNGLENDQUERYARBPROC __glewEndQueryARB = NULL;
PFNGLGENQUERIESARBPROC __glewGenQueriesARB = NULL;
PFNGLGETQUERYOBJECTIVARBPROC __glewGetQueryObjectivARB = NULL;
PFNGLGETQUERYOBJECTUIVARBPROC __glewGetQueryObjectuivARB = NULL;
PFNGLGETQUERYIVARBPROC __glewGetQueryivARB = NULL;
PFNGLISQUERYARBPROC __glewIsQueryARB = NULL;

PFNGLPOINTPARAMETERFARBPROC __glewPointParameterfARB = NULL;
PFNGLPOINTPARAMETERFVARBPROC __glewPointParameterfvARB = NULL;

PFNGLATTACHOBJECTARBPROC __glewAttachObjectARB = NULL;
PFNGLCOMPILESHADERARBPROC __glewCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC __glewCreateProgramObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC __glewCreateShaderObjectARB = NULL;
PFNGLDELETEOBJECTARBPROC __glewDeleteObjectARB = NULL;
PFNGLDETACHOBJECTARBPROC __glewDetachObjectARB = NULL;
PFNGLGETACTIVEUNIFORMARBPROC __glewGetActiveUniformARB = NULL;
PFNGLGETATTACHEDOBJECTSARBPROC __glewGetAttachedObjectsARB = NULL;
PFNGLGETHANDLEARBPROC __glewGetHandleARB = NULL;
PFNGLGETINFOLOGARBPROC __glewGetInfoLogARB = NULL;
PFNGLGETOBJECTPARAMETERFVARBPROC __glewGetObjectParameterfvARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC __glewGetObjectParameterivARB = NULL;
PFNGLGETSHADERSOURCEARBPROC __glewGetShaderSourceARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC __glewGetUniformLocationARB = NULL;
PFNGLGETUNIFORMFVARBPROC __glewGetUniformfvARB = NULL;
PFNGLGETUNIFORMIVARBPROC __glewGetUniformivARB = NULL;
PFNGLLINKPROGRAMARBPROC __glewLinkProgramARB = NULL;
PFNGLSHADERSOURCEARBPROC __glewShaderSourceARB = NULL;
PFNGLUNIFORM1FARBPROC __glewUniform1fARB = NULL;
PFNGLUNIFORM1FVARBPROC __glewUniform1fvARB = NULL;
PFNGLUNIFORM1IARBPROC __glewUniform1iARB = NULL;
PFNGLUNIFORM1IVARBPROC __glewUniform1ivARB = NULL;
PFNGLUNIFORM2FARBPROC __glewUniform2fARB = NULL;
PFNGLUNIFORM2FVARBPROC __glewUniform2fvARB = NULL;
PFNGLUNIFORM2IARBPROC __glewUniform2iARB = NULL;
PFNGLUNIFORM2IVARBPROC __glewUniform2ivARB = NULL;
PFNGLUNIFORM3FARBPROC __glewUniform3fARB = NULL;
PFNGLUNIFORM3FVARBPROC __glewUniform3fvARB = NULL;
PFNGLUNIFORM3IARBPROC __glewUniform3iARB = NULL;
PFNGLUNIFORM3IVARBPROC __glewUniform3ivARB = NULL;
PFNGLUNIFORM4FARBPROC __glewUniform4fARB = NULL;
PFNGLUNIFORM4FVARBPROC __glewUniform4fvARB = NULL;
PFNGLUNIFORM4IARBPROC __glewUniform4iARB = NULL;
PFNGLUNIFORM4IVARBPROC __glewUniform4ivARB = NULL;
PFNGLUNIFORMMATRIX2FVARBPROC __glewUniformMatrix2fvARB = NULL;
PFNGLUNIFORMMATRIX3FVARBPROC __glewUniformMatrix3fvARB = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC __glewUniformMatrix4fvARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC __glewUseProgramObjectARB = NULL;
PFNGLVALIDATEPROGRAMARBPROC __glewValidateProgramARB = NULL;

PFNGLTEXBUFFERARBPROC __glewTexBufferARB = NULL;

PFNGLCOMPRESSEDTEXIMAGE1DARBPROC __glewCompressedTexImage1DARB = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC __glewCompressedTexImage2DARB = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC __glewCompressedTexImage3DARB = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC __glewCompressedTexSubImage1DARB = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC __glewCompressedTexSubImage2DARB = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC __glewCompressedTexSubImage3DARB = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC __glewGetCompressedTexImageARB = NULL;

PFNGLLOADTRANSPOSEMATRIXDARBPROC __glewLoadTransposeMatrixdARB = NULL;
PFNGLLOADTRANSPOSEMATRIXFARBPROC __glewLoadTransposeMatrixfARB = NULL;
PFNGLMULTTRANSPOSEMATRIXDARBPROC __glewMultTransposeMatrixdARB = NULL;
PFNGLMULTTRANSPOSEMATRIXFARBPROC __glewMultTransposeMatrixfARB = NULL;

PFNGLBINDVERTEXARRAYPROC __glewBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC __glewDeleteVertexArrays = NULL;
PFNGLGENVERTEXARRAYSPROC __glewGenVertexArrays = NULL;
PFNGLISVERTEXARRAYPROC __glewIsVertexArray = NULL;

PFNGLVERTEXBLENDARBPROC __glewVertexBlendARB = NULL;
PFNGLWEIGHTPOINTERARBPROC __glewWeightPointerARB = NULL;
PFNGLWEIGHTBVARBPROC __glewWeightbvARB = NULL;
PFNGLWEIGHTDVARBPROC __glewWeightdvARB = NULL;
PFNGLWEIGHTFVARBPROC __glewWeightfvARB = NULL;
PFNGLWEIGHTIVARBPROC __glewWeightivARB = NULL;
PFNGLWEIGHTSVARBPROC __glewWeightsvARB = NULL;
PFNGLWEIGHTUBVARBPROC __glewWeightubvARB = NULL;
PFNGLWEIGHTUIVARBPROC __glewWeightuivARB = NULL;
PFNGLWEIGHTUSVARBPROC __glewWeightusvARB = NULL;

PFNGLBINDBUFFERARBPROC __glewBindBufferARB = NULL;
PFNGLBUFFERDATAARBPROC __glewBufferDataARB = NULL;
PFNGLBUFFERSUBDATAARBPROC __glewBufferSubDataARB = NULL;
PFNGLDELETEBUFFERSARBPROC __glewDeleteBuffersARB = NULL;
PFNGLGENBUFFERSARBPROC __glewGenBuffersARB = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC __glewGetBufferParameterivARB = NULL;
PFNGLGETBUFFERPOINTERVARBPROC __glewGetBufferPointervARB = NULL;
PFNGLGETBUFFERSUBDATAARBPROC __glewGetBufferSubDataARB = NULL;
PFNGLISBUFFERARBPROC __glewIsBufferARB = NULL;
PFNGLMAPBUFFERARBPROC __glewMapBufferARB = NULL;
PFNGLUNMAPBUFFERARBPROC __glewUnmapBufferARB = NULL;

PFNGLBINDPROGRAMARBPROC __glewBindProgramARB = NULL;
PFNGLDELETEPROGRAMSARBPROC __glewDeleteProgramsARB = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC __glewDisableVertexAttribArrayARB = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC __glewEnableVertexAttribArrayARB = NULL;
PFNGLGENPROGRAMSARBPROC __glewGenProgramsARB = NULL;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC __glewGetProgramEnvParameterdvARB = NULL;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC __glewGetProgramEnvParameterfvARB = NULL;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC __glewGetProgramLocalParameterdvARB = NULL;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC __glewGetProgramLocalParameterfvARB = NULL;
PFNGLGETPROGRAMSTRINGARBPROC __glewGetProgramStringARB = NULL;
PFNGLGETPROGRAMIVARBPROC __glewGetProgramivARB = NULL;
PFNGLGETVERTEXATTRIBPOINTERVARBPROC __glewGetVertexAttribPointervARB = NULL;
PFNGLGETVERTEXATTRIBDVARBPROC __glewGetVertexAttribdvARB = NULL;
PFNGLGETVERTEXATTRIBFVARBPROC __glewGetVertexAttribfvARB = NULL;
PFNGLGETVERTEXATTRIBIVARBPROC __glewGetVertexAttribivARB = NULL;
PFNGLISPROGRAMARBPROC __glewIsProgramARB = NULL;
PFNGLPROGRAMENVPARAMETER4DARBPROC __glewProgramEnvParameter4dARB = NULL;
PFNGLPROGRAMENVPARAMETER4DVARBPROC __glewProgramEnvParameter4dvARB = NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC __glewProgramEnvParameter4fARB = NULL;
PFNGLPROGRAMENVPARAMETER4FVARBPROC __glewProgramEnvParameter4fvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC __glewProgramLocalParameter4dARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC __glewProgramLocalParameter4dvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC __glewProgramLocalParameter4fARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC __glewProgramLocalParameter4fvARB = NULL;
PFNGLPROGRAMSTRINGARBPROC __glewProgramStringARB = NULL;
PFNGLVERTEXATTRIB1DARBPROC __glewVertexAttrib1dARB = NULL;
PFNGLVERTEXATTRIB1DVARBPROC __glewVertexAttrib1dvARB = NULL;
PFNGLVERTEXATTRIB1FARBPROC __glewVertexAttrib1fARB = NULL;
PFNGLVERTEXATTRIB1FVARBPROC __glewVertexAttrib1fvARB = NULL;
PFNGLVERTEXATTRIB1SARBPROC __glewVertexAttrib1sARB = NULL;
PFNGLVERTEXATTRIB1SVARBPROC __glewVertexAttrib1svARB = NULL;
PFNGLVERTEXATTRIB2DARBPROC __glewVertexAttrib2dARB = NULL;
PFNGLVERTEXATTRIB2DVARBPROC __glewVertexAttrib2dvARB = NULL;
PFNGLVERTEXATTRIB2FARBPROC __glewVertexAttrib2fARB = NULL;
PFNGLVERTEXATTRIB2FVARBPROC __glewVertexAttrib2fvARB = NULL;
PFNGLVERTEXATTRIB2SARBPROC __glewVertexAttrib2sARB = NULL;
PFNGLVERTEXATTRIB2SVARBPROC __glewVertexAttrib2svARB = NULL;
PFNGLVERTEXATTRIB3DARBPROC __glewVertexAttrib3dARB = NULL;
PFNGLVERTEXATTRIB3DVARBPROC __glewVertexAttrib3dvARB = NULL;
PFNGLVERTEXATTRIB3FARBPROC __glewVertexAttrib3fARB = NULL;
PFNGLVERTEXATTRIB3FVARBPROC __glewVertexAttrib3fvARB = NULL;
PFNGLVERTEXATTRIB3SARBPROC __glewVertexAttrib3sARB = NULL;
PFNGLVERTEXATTRIB3SVARBPROC __glewVertexAttrib3svARB = NULL;
PFNGLVERTEXATTRIB4NBVARBPROC __glewVertexAttrib4NbvARB = NULL;
PFNGLVERTEXATTRIB4NIVARBPROC __glewVertexAttrib4NivARB = NULL;
PFNGLVERTEXATTRIB4NSVARBPROC __glewVertexAttrib4NsvARB = NULL;
PFNGLVERTEXATTRIB4NUBARBPROC __glewVertexAttrib4NubARB = NULL;
PFNGLVERTEXATTRIB4NUBVARBPROC __glewVertexAttrib4NubvARB = NULL;
PFNGLVERTEXATTRIB4NUIVARBPROC __glewVertexAttrib4NuivARB = NULL;
PFNGLVERTEXATTRIB4NUSVARBPROC __glewVertexAttrib4NusvARB = NULL;
PFNGLVERTEXATTRIB4BVARBPROC __glewVertexAttrib4bvARB = NULL;
PFNGLVERTEXATTRIB4DARBPROC __glewVertexAttrib4dARB = NULL;
PFNGLVERTEXATTRIB4DVARBPROC __glewVertexAttrib4dvARB = NULL;
PFNGLVERTEXATTRIB4FARBPROC __glewVertexAttrib4fARB = NULL;
PFNGLVERTEXATTRIB4FVARBPROC __glewVertexAttrib4fvARB = NULL;
PFNGLVERTEXATTRIB4IVARBPROC __glewVertexAttrib4ivARB = NULL;
PFNGLVERTEXATTRIB4SARBPROC __glewVertexAttrib4sARB = NULL;
PFNGLVERTEXATTRIB4SVARBPROC __glewVertexAttrib4svARB = NULL;
PFNGLVERTEXATTRIB4UBVARBPROC __glewVertexAttrib4ubvARB = NULL;
PFNGLVERTEXATTRIB4UIVARBPROC __glewVertexAttrib4uivARB = NULL;
PFNGLVERTEXATTRIB4USVARBPROC __glewVertexAttrib4usvARB = NULL;
PFNGLVERTEXATTRIBPOINTERARBPROC __glewVertexAttribPointerARB = NULL;

PFNGLBINDATTRIBLOCATIONARBPROC __glewBindAttribLocationARB = NULL;
PFNGLGETACTIVEATTRIBARBPROC __glewGetActiveAttribARB = NULL;
PFNGLGETATTRIBLOCATIONARBPROC __glewGetAttribLocationARB = NULL;

PFNGLWINDOWPOS2DARBPROC __glewWindowPos2dARB = NULL;
PFNGLWINDOWPOS2DVARBPROC __glewWindowPos2dvARB = NULL;
PFNGLWINDOWPOS2FARBPROC __glewWindowPos2fARB = NULL;
PFNGLWINDOWPOS2FVARBPROC __glewWindowPos2fvARB = NULL;
PFNGLWINDOWPOS2IARBPROC __glewWindowPos2iARB = NULL;
PFNGLWINDOWPOS2IVARBPROC __glewWindowPos2ivARB = NULL;
PFNGLWINDOWPOS2SARBPROC __glewWindowPos2sARB = NULL;
PFNGLWINDOWPOS2SVARBPROC __glewWindowPos2svARB = NULL;
PFNGLWINDOWPOS3DARBPROC __glewWindowPos3dARB = NULL;
PFNGLWINDOWPOS3DVARBPROC __glewWindowPos3dvARB = NULL;
PFNGLWINDOWPOS3FARBPROC __glewWindowPos3fARB = NULL;
PFNGLWINDOWPOS3FVARBPROC __glewWindowPos3fvARB = NULL;
PFNGLWINDOWPOS3IARBPROC __glewWindowPos3iARB = NULL;
PFNGLWINDOWPOS3IVARBPROC __glewWindowPos3ivARB = NULL;
PFNGLWINDOWPOS3SARBPROC __glewWindowPos3sARB = NULL;
PFNGLWINDOWPOS3SVARBPROC __glewWindowPos3svARB = NULL;

PFNGLDRAWBUFFERSATIPROC __glewDrawBuffersATI = NULL;

PFNGLDRAWELEMENTARRAYATIPROC __glewDrawElementArrayATI = NULL;
PFNGLDRAWRANGEELEMENTARRAYATIPROC __glewDrawRangeElementArrayATI = NULL;
PFNGLELEMENTPOINTERATIPROC __glewElementPointerATI = NULL;

PFNGLGETTEXBUMPPARAMETERFVATIPROC __glewGetTexBumpParameterfvATI = NULL;
PFNGLGETTEXBUMPPARAMETERIVATIPROC __glewGetTexBumpParameterivATI = NULL;
PFNGLTEXBUMPPARAMETERFVATIPROC __glewTexBumpParameterfvATI = NULL;
PFNGLTEXBUMPPARAMETERIVATIPROC __glewTexBumpParameterivATI = NULL;

PFNGLALPHAFRAGMENTOP1ATIPROC __glewAlphaFragmentOp1ATI = NULL;
PFNGLALPHAFRAGMENTOP2ATIPROC __glewAlphaFragmentOp2ATI = NULL;
PFNGLALPHAFRAGMENTOP3ATIPROC __glewAlphaFragmentOp3ATI = NULL;
PFNGLBEGINFRAGMENTSHADERATIPROC __glewBeginFragmentShaderATI = NULL;
PFNGLBINDFRAGMENTSHADERATIPROC __glewBindFragmentShaderATI = NULL;
PFNGLCOLORFRAGMENTOP1ATIPROC __glewColorFragmentOp1ATI = NULL;
PFNGLCOLORFRAGMENTOP2ATIPROC __glewColorFragmentOp2ATI = NULL;
PFNGLCOLORFRAGMENTOP3ATIPROC __glewColorFragmentOp3ATI = NULL;
PFNGLDELETEFRAGMENTSHADERATIPROC __glewDeleteFragmentShaderATI = NULL;
PFNGLENDFRAGMENTSHADERATIPROC __glewEndFragmentShaderATI = NULL;
PFNGLGENFRAGMENTSHADERSATIPROC __glewGenFragmentShadersATI = NULL;
PFNGLPASSTEXCOORDATIPROC __glewPassTexCoordATI = NULL;
PFNGLSAMPLEMAPATIPROC __glewSampleMapATI = NULL;
PFNGLSETFRAGMENTSHADERCONSTANTATIPROC __glewSetFragmentShaderConstantATI = NULL;

PFNGLMAPOBJECTBUFFERATIPROC __glewMapObjectBufferATI = NULL;
PFNGLUNMAPOBJECTBUFFERATIPROC __glewUnmapObjectBufferATI = NULL;

PFNGLPNTRIANGLESFATIPROC __glPNTrianglewesfATI = NULL;
PFNGLPNTRIANGLESIATIPROC __glPNTrianglewesiATI = NULL;

PFNGLSTENCILFUNCSEPARATEATIPROC __glewStencilFuncSeparateATI = NULL;
PFNGLSTENCILOPSEPARATEATIPROC __glewStencilOpSeparateATI = NULL;

PFNGLARRAYOBJECTATIPROC __glewArrayObjectATI = NULL;
PFNGLFREEOBJECTBUFFERATIPROC __glewFreeObjectBufferATI = NULL;
PFNGLGETARRAYOBJECTFVATIPROC __glewGetArrayObjectfvATI = NULL;
PFNGLGETARRAYOBJECTIVATIPROC __glewGetArrayObjectivATI = NULL;
PFNGLGETOBJECTBUFFERFVATIPROC __glewGetObjectBufferfvATI = NULL;
PFNGLGETOBJECTBUFFERIVATIPROC __glewGetObjectBufferivATI = NULL;
PFNGLGETVARIANTARRAYOBJECTFVATIPROC __glewGetVariantArrayObjectfvATI = NULL;
PFNGLGETVARIANTARRAYOBJECTIVATIPROC __glewGetVariantArrayObjectivATI = NULL;
PFNGLISOBJECTBUFFERATIPROC __glewIsObjectBufferATI = NULL;
PFNGLNEWOBJECTBUFFERATIPROC __glewNewObjectBufferATI = NULL;
PFNGLUPDATEOBJECTBUFFERATIPROC __glewUpdateObjectBufferATI = NULL;
PFNGLVARIANTARRAYOBJECTATIPROC __glewVariantArrayObjectATI = NULL;

PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC __glewGetVertexAttribArrayObjectfvATI = NULL;
PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC __glewGetVertexAttribArrayObjectivATI = NULL;
PFNGLVERTEXATTRIBARRAYOBJECTATIPROC __glewVertexAttribArrayObjectATI = NULL;

PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC __glewClientActiveVertexStreamATI = NULL;
PFNGLNORMALSTREAM3BATIPROC __glewNormalStream3bATI = NULL;
PFNGLNORMALSTREAM3BVATIPROC __glewNormalStream3bvATI = NULL;
PFNGLNORMALSTREAM3DATIPROC __glewNormalStream3dATI = NULL;
PFNGLNORMALSTREAM3DVATIPROC __glewNormalStream3dvATI = NULL;
PFNGLNORMALSTREAM3FATIPROC __glewNormalStream3fATI = NULL;
PFNGLNORMALSTREAM3FVATIPROC __glewNormalStream3fvATI = NULL;
PFNGLNORMALSTREAM3IATIPROC __glewNormalStream3iATI = NULL;
PFNGLNORMALSTREAM3IVATIPROC __glewNormalStream3ivATI = NULL;
PFNGLNORMALSTREAM3SATIPROC __glewNormalStream3sATI = NULL;
PFNGLNORMALSTREAM3SVATIPROC __glewNormalStream3svATI = NULL;
PFNGLVERTEXBLENDENVFATIPROC __glewVertexBlendEnvfATI = NULL;
PFNGLVERTEXBLENDENVIATIPROC __glewVertexBlendEnviATI = NULL;
PFNGLVERTEXSTREAM2DATIPROC __glewVertexStream2dATI = NULL;
PFNGLVERTEXSTREAM2DVATIPROC __glewVertexStream2dvATI = NULL;
PFNGLVERTEXSTREAM2FATIPROC __glewVertexStream2fATI = NULL;
PFNGLVERTEXSTREAM2FVATIPROC __glewVertexStream2fvATI = NULL;
PFNGLVERTEXSTREAM2IATIPROC __glewVertexStream2iATI = NULL;
PFNGLVERTEXSTREAM2IVATIPROC __glewVertexStream2ivATI = NULL;
PFNGLVERTEXSTREAM2SATIPROC __glewVertexStream2sATI = NULL;
PFNGLVERTEXSTREAM2SVATIPROC __glewVertexStream2svATI = NULL;
PFNGLVERTEXSTREAM3DATIPROC __glewVertexStream3dATI = NULL;
PFNGLVERTEXSTREAM3DVATIPROC __glewVertexStream3dvATI = NULL;
PFNGLVERTEXSTREAM3FATIPROC __glewVertexStream3fATI = NULL;
PFNGLVERTEXSTREAM3FVATIPROC __glewVertexStream3fvATI = NULL;
PFNGLVERTEXSTREAM3IATIPROC __glewVertexStream3iATI = NULL;
PFNGLVERTEXSTREAM3IVATIPROC __glewVertexStream3ivATI = NULL;
PFNGLVERTEXSTREAM3SATIPROC __glewVertexStream3sATI = NULL;
PFNGLVERTEXSTREAM3SVATIPROC __glewVertexStream3svATI = NULL;
PFNGLVERTEXSTREAM4DATIPROC __glewVertexStream4dATI = NULL;
PFNGLVERTEXSTREAM4DVATIPROC __glewVertexStream4dvATI = NULL;
PFNGLVERTEXSTREAM4FATIPROC __glewVertexStream4fATI = NULL;
PFNGLVERTEXSTREAM4FVATIPROC __glewVertexStream4fvATI = NULL;
PFNGLVERTEXSTREAM4IATIPROC __glewVertexStream4iATI = NULL;
PFNGLVERTEXSTREAM4IVATIPROC __glewVertexStream4ivATI = NULL;
PFNGLVERTEXSTREAM4SATIPROC __glewVertexStream4sATI = NULL;
PFNGLVERTEXSTREAM4SVATIPROC __glewVertexStream4svATI = NULL;

PFNGLGETUNIFORMBUFFERSIZEEXTPROC __glewGetUniformBufferSizeEXT = NULL;
PFNGLGETUNIFORMOFFSETEXTPROC __glewGetUniformOffsetEXT = NULL;
PFNGLUNIFORMBUFFEREXTPROC __glewUniformBufferEXT = NULL;

PFNGLBLENDCOLOREXTPROC __glewBlendColorEXT = NULL;

PFNGLBLENDEQUATIONSEPARATEEXTPROC __glewBlendEquationSeparateEXT = NULL;

PFNGLBLENDFUNCSEPARATEEXTPROC __glewBlendFuncSeparateEXT = NULL;

PFNGLBLENDEQUATIONEXTPROC __glewBlendEquationEXT = NULL;

PFNGLCOLORSUBTABLEEXTPROC __glewColorSubTableEXT = NULL;
PFNGLCOPYCOLORSUBTABLEEXTPROC __glewCopyColorSubTableEXT = NULL;

PFNGLLOCKARRAYSEXTPROC __glewLockArraysEXT = NULL;
PFNGLUNLOCKARRAYSEXTPROC __glewUnlockArraysEXT = NULL;

PFNGLCONVOLUTIONFILTER1DEXTPROC __glewConvolutionFilter1DEXT = NULL;
PFNGLCONVOLUTIONFILTER2DEXTPROC __glewConvolutionFilter2DEXT = NULL;
PFNGLCONVOLUTIONPARAMETERFEXTPROC __glewConvolutionParameterfEXT = NULL;
PFNGLCONVOLUTIONPARAMETERFVEXTPROC __glewConvolutionParameterfvEXT = NULL;
PFNGLCONVOLUTIONPARAMETERIEXTPROC __glewConvolutionParameteriEXT = NULL;
PFNGLCONVOLUTIONPARAMETERIVEXTPROC __glewConvolutionParameterivEXT = NULL;
PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC __glewCopyConvolutionFilter1DEXT = NULL;
PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC __glewCopyConvolutionFilter2DEXT = NULL;
PFNGLGETCONVOLUTIONFILTEREXTPROC __glewGetConvolutionFilterEXT = NULL;
PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC __glewGetConvolutionParameterfvEXT = NULL;
PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC __glewGetConvolutionParameterivEXT = NULL;
PFNGLGETSEPARABLEFILTEREXTPROC __glewGetSeparableFilterEXT = NULL;
PFNGLSEPARABLEFILTER2DEXTPROC __glewSeparableFilter2DEXT = NULL;

PFNGLBINORMALPOINTEREXTPROC __glewBinormalPointerEXT = NULL;
PFNGLTANGENTPOINTEREXTPROC __glewTangentPointerEXT = NULL;

PFNGLCOPYTEXIMAGE1DEXTPROC __glewCopyTexImage1DEXT = NULL;
PFNGLCOPYTEXIMAGE2DEXTPROC __glewCopyTexImage2DEXT = NULL;
PFNGLCOPYTEXSUBIMAGE1DEXTPROC __glewCopyTexSubImage1DEXT = NULL;
PFNGLCOPYTEXSUBIMAGE2DEXTPROC __glewCopyTexSubImage2DEXT = NULL;
PFNGLCOPYTEXSUBIMAGE3DEXTPROC __glewCopyTexSubImage3DEXT = NULL;

PFNGLCULLPARAMETERDVEXTPROC __glewCullParameterdvEXT = NULL;
PFNGLCULLPARAMETERFVEXTPROC __glewCullParameterfvEXT = NULL;

PFNGLDEPTHBOUNDSEXTPROC __glewDepthBoundsEXT = NULL;

PFNGLBINDMULTITEXTUREEXTPROC __glewBindMultiTextureEXT = NULL;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC __glewCheckNamedFramebufferStatusEXT = NULL;
PFNGLCLIENTATTRIBDEFAULTEXTPROC __glewClientAttribDefaultEXT = NULL;
PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC __glewCompressedMultiTexImage1DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC __glewCompressedMultiTexImage2DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC __glewCompressedMultiTexImage3DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC __glewCompressedMultiTexSubImage1DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC __glewCompressedMultiTexSubImage2DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC __glewCompressedMultiTexSubImage3DEXT = NULL;
PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC __glewCompressedTextureImage1DEXT = NULL;
PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC __glewCompressedTextureImage2DEXT = NULL;
PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC __glewCompressedTextureImage3DEXT = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC __glewCompressedTextureSubImage1DEXT = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC __glewCompressedTextureSubImage2DEXT = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC __glewCompressedTextureSubImage3DEXT = NULL;
PFNGLCOPYMULTITEXIMAGE1DEXTPROC __glewCopyMultiTexImage1DEXT = NULL;
PFNGLCOPYMULTITEXIMAGE2DEXTPROC __glewCopyMultiTexImage2DEXT = NULL;
PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC __glewCopyMultiTexSubImage1DEXT = NULL;
PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC __glewCopyMultiTexSubImage2DEXT = NULL;
PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC __glewCopyMultiTexSubImage3DEXT = NULL;
PFNGLCOPYTEXTUREIMAGE1DEXTPROC __glewCopyTextureImage1DEXT = NULL;
PFNGLCOPYTEXTUREIMAGE2DEXTPROC __glewCopyTextureImage2DEXT = NULL;
PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC __glewCopyTextureSubImage1DEXT = NULL;
PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC __glewCopyTextureSubImage2DEXT = NULL;
PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC __glewCopyTextureSubImage3DEXT = NULL;
PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC __glewDisableClientStateIndexedEXT = NULL;
PFNGLENABLECLIENTSTATEINDEXEDEXTPROC __glewEnableClientStateIndexedEXT = NULL;
PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC __glewFramebufferDrawBufferEXT = NULL;
PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC __glewFramebufferDrawBuffersEXT = NULL;
PFNGLFRAMEBUFFERREADBUFFEREXTPROC __glewFramebufferReadBufferEXT = NULL;
PFNGLGENERATEMULTITEXMIPMAPEXTPROC __glewGenerateMultiTexMipmapEXT = NULL;
PFNGLGENERATETEXTUREMIPMAPEXTPROC __glewGenerateTextureMipmapEXT = NULL;
PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC __glewGetCompressedMultiTexImageEXT = NULL;
PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC __glewGetCompressedTextureImageEXT = NULL;
PFNGLGETDOUBLEINDEXEDVEXTPROC __glewGetDoubleIndexedvEXT = NULL;
PFNGLGETFLOATINDEXEDVEXTPROC __glewGetFloatIndexedvEXT = NULL;
PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC __glewGetFramebufferParameterivEXT = NULL;
PFNGLGETMULTITEXENVFVEXTPROC __glewGetMultiTexEnvfvEXT = NULL;
PFNGLGETMULTITEXENVIVEXTPROC __glewGetMultiTexEnvivEXT = NULL;
PFNGLGETMULTITEXGENDVEXTPROC __glewGetMultiTexGendvEXT = NULL;
PFNGLGETMULTITEXGENFVEXTPROC __glewGetMultiTexGenfvEXT = NULL;
PFNGLGETMULTITEXGENIVEXTPROC __glewGetMultiTexGenivEXT = NULL;
PFNGLGETMULTITEXIMAGEEXTPROC __glewGetMultiTexImageEXT = NULL;
PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC __glewGetMultiTexLevelParameterfvEXT = NULL;
PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC __glewGetMultiTexLevelParameterivEXT = NULL;
PFNGLGETMULTITEXPARAMETERIIVEXTPROC __glewGetMultiTexParameterIivEXT = NULL;
PFNGLGETMULTITEXPARAMETERIUIVEXTPROC __glewGetMultiTexParameterIuivEXT = NULL;
PFNGLGETMULTITEXPARAMETERFVEXTPROC __glewGetMultiTexParameterfvEXT = NULL;
PFNGLGETMULTITEXPARAMETERIVEXTPROC __glewGetMultiTexParameterivEXT = NULL;
PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC __glewGetNamedBufferParameterivEXT = NULL;
PFNGLGETNAMEDBUFFERPOINTERVEXTPROC __glewGetNamedBufferPointervEXT = NULL;
PFNGLGETNAMEDBUFFERSUBDATAEXTPROC __glewGetNamedBufferSubDataEXT = NULL;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC __glewGetNamedFramebufferAttachmentParameterivEXT = NULL;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC __glewGetNamedProgramLocalParameterIivEXT = NULL;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC __glewGetNamedProgramLocalParameterIuivEXT = NULL;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC __glewGetNamedProgramLocalParameterdvEXT = NULL;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC __glewGetNamedProgramLocalParameterfvEXT = NULL;
PFNGLGETNAMEDPROGRAMSTRINGEXTPROC __glewGetNamedProgramStringEXT = NULL;
PFNGLGETNAMEDPROGRAMIVEXTPROC __glewGetNamedProgramivEXT = NULL;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC __glewGetNamedRenderbufferParameterivEXT = NULL;
PFNGLGETPOINTERINDEXEDVEXTPROC __glewGetPointerIndexedvEXT = NULL;
PFNGLGETTEXTUREIMAGEEXTPROC __glewGetTextureImageEXT = NULL;
PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC __glewGetTextureLevelParameterfvEXT = NULL;
PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC __glewGetTextureLevelParameterivEXT = NULL;
PFNGLGETTEXTUREPARAMETERIIVEXTPROC __glewGetTextureParameterIivEXT = NULL;
PFNGLGETTEXTUREPARAMETERIUIVEXTPROC __glewGetTextureParameterIuivEXT = NULL;
PFNGLGETTEXTUREPARAMETERFVEXTPROC __glewGetTextureParameterfvEXT = NULL;
PFNGLGETTEXTUREPARAMETERIVEXTPROC __glewGetTextureParameterivEXT = NULL;
PFNGLMAPNAMEDBUFFEREXTPROC __glewMapNamedBufferEXT = NULL;
PFNGLMATRIXFRUSTUMEXTPROC __glewMatrixFrustumEXT = NULL;
PFNGLMATRIXLOADIDENTITYEXTPROC __glewMatrixLoadIdentityEXT = NULL;
PFNGLMATRIXLOADTRANSPOSEDEXTPROC __glewMatrixLoadTransposedEXT = NULL;
PFNGLMATRIXLOADTRANSPOSEFEXTPROC __glewMatrixLoadTransposefEXT = NULL;
PFNGLMATRIXLOADDEXTPROC __glewMatrixLoaddEXT = NULL;
PFNGLMATRIXLOADFEXTPROC __glewMatrixLoadfEXT = NULL;
PFNGLMATRIXMULTTRANSPOSEDEXTPROC __glewMatrixMultTransposedEXT = NULL;
PFNGLMATRIXMULTTRANSPOSEFEXTPROC __glewMatrixMultTransposefEXT = NULL;
PFNGLMATRIXMULTDEXTPROC __glewMatrixMultdEXT = NULL;
PFNGLMATRIXMULTFEXTPROC __glewMatrixMultfEXT = NULL;
PFNGLMATRIXORTHOEXTPROC __glewMatrixOrthoEXT = NULL;
PFNGLMATRIXPOPEXTPROC __glewMatrixPopEXT = NULL;
PFNGLMATRIXPUSHEXTPROC __glewMatrixPushEXT = NULL;
PFNGLMATRIXROTATEDEXTPROC __glewMatrixRotatedEXT = NULL;
PFNGLMATRIXROTATEFEXTPROC __glewMatrixRotatefEXT = NULL;
PFNGLMATRIXSCALEDEXTPROC __glewMatrixScaledEXT = NULL;
PFNGLMATRIXSCALEFEXTPROC __glewMatrixScalefEXT = NULL;
PFNGLMATRIXTRANSLATEDEXTPROC __glewMatrixTranslatedEXT = NULL;
PFNGLMATRIXTRANSLATEFEXTPROC __glewMatrixTranslatefEXT = NULL;
PFNGLMULTITEXBUFFEREXTPROC __glewMultiTexBufferEXT = NULL;
PFNGLMULTITEXCOORDPOINTEREXTPROC __glewMultiTexCoordPointerEXT = NULL;
PFNGLMULTITEXENVFEXTPROC __glewMultiTexEnvfEXT = NULL;
PFNGLMULTITEXENVFVEXTPROC __glewMultiTexEnvfvEXT = NULL;
PFNGLMULTITEXENVIEXTPROC __glewMultiTexEnviEXT = NULL;
PFNGLMULTITEXENVIVEXTPROC __glewMultiTexEnvivEXT = NULL;
PFNGLMULTITEXGENDEXTPROC __glewMultiTexGendEXT = NULL;
PFNGLMULTITEXGENDVEXTPROC __glewMultiTexGendvEXT = NULL;
PFNGLMULTITEXGENFEXTPROC __glewMultiTexGenfEXT = NULL;
PFNGLMULTITEXGENFVEXTPROC __glewMultiTexGenfvEXT = NULL;
PFNGLMULTITEXGENIEXTPROC __glewMultiTexGeniEXT = NULL;
PFNGLMULTITEXGENIVEXTPROC __glewMultiTexGenivEXT = NULL;
PFNGLMULTITEXIMAGE1DEXTPROC __glewMultiTexImage1DEXT = NULL;
PFNGLMULTITEXIMAGE2DEXTPROC __glewMultiTexImage2DEXT = NULL;
PFNGLMULTITEXIMAGE3DEXTPROC __glewMultiTexImage3DEXT = NULL;
PFNGLMULTITEXPARAMETERIIVEXTPROC __glewMultiTexParameterIivEXT = NULL;
PFNGLMULTITEXPARAMETERIUIVEXTPROC __glewMultiTexParameterIuivEXT = NULL;
PFNGLMULTITEXPARAMETERFEXTPROC __glewMultiTexParameterfEXT = NULL;
PFNGLMULTITEXPARAMETERFVEXTPROC __glewMultiTexParameterfvEXT = NULL;
PFNGLMULTITEXPARAMETERIEXTPROC __glewMultiTexParameteriEXT = NULL;
PFNGLMULTITEXPARAMETERIVEXTPROC __glewMultiTexParameterivEXT = NULL;
PFNGLMULTITEXRENDERBUFFEREXTPROC __glewMultiTexRenderbufferEXT = NULL;
PFNGLMULTITEXSUBIMAGE1DEXTPROC __glewMultiTexSubImage1DEXT = NULL;
PFNGLMULTITEXSUBIMAGE2DEXTPROC __glewMultiTexSubImage2DEXT = NULL;
PFNGLMULTITEXSUBIMAGE3DEXTPROC __glewMultiTexSubImage3DEXT = NULL;
PFNGLNAMEDBUFFERDATAEXTPROC __glewNamedBufferDataEXT = NULL;
PFNGLNAMEDBUFFERSUBDATAEXTPROC __glewNamedBufferSubDataEXT = NULL;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC __glewNamedFramebufferRenderbufferEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC __glewNamedFramebufferTexture1DEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC __glewNamedFramebufferTexture2DEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC __glewNamedFramebufferTexture3DEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC __glewNamedFramebufferTextureEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC __glewNamedFramebufferTextureFaceEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC __glewNamedFramebufferTextureLayerEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC __glewNamedProgramLocalParameter4dEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC __glewNamedProgramLocalParameter4dvEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC __glewNamedProgramLocalParameter4fEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC __glewNamedProgramLocalParameter4fvEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC __glewNamedProgramLocalParameterI4iEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC __glewNamedProgramLocalParameterI4ivEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC __glewNamedProgramLocalParameterI4uiEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC __glewNamedProgramLocalParameterI4uivEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC __glewNamedProgramLocalParameters4fvEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC __glewNamedProgramLocalParametersI4ivEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC __glewNamedProgramLocalParametersI4uivEXT = NULL;
PFNGLNAMEDPROGRAMSTRINGEXTPROC __glewNamedProgramStringEXT = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC __glewNamedRenderbufferStorageEXT = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC __glewNamedRenderbufferStorageMultisampleCoverageEXT = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC __glewNamedRenderbufferStorageMultisampleEXT = NULL;
PFNGLPROGRAMUNIFORM1FEXTPROC __glewProgramUniform1fEXT = NULL;
PFNGLPROGRAMUNIFORM1FVEXTPROC __glewProgramUniform1fvEXT = NULL;
PFNGLPROGRAMUNIFORM1IEXTPROC __glewProgramUniform1iEXT = NULL;
PFNGLPROGRAMUNIFORM1IVEXTPROC __glewProgramUniform1ivEXT = NULL;
PFNGLPROGRAMUNIFORM1UIEXTPROC __glewProgramUniform1uiEXT = NULL;
PFNGLPROGRAMUNIFORM1UIVEXTPROC __glewProgramUniform1uivEXT = NULL;
PFNGLPROGRAMUNIFORM2FEXTPROC __glewProgramUniform2fEXT = NULL;
PFNGLPROGRAMUNIFORM2FVEXTPROC __glewProgramUniform2fvEXT = NULL;
PFNGLPROGRAMUNIFORM2IEXTPROC __glewProgramUniform2iEXT = NULL;
PFNGLPROGRAMUNIFORM2IVEXTPROC __glewProgramUniform2ivEXT = NULL;
PFNGLPROGRAMUNIFORM2UIEXTPROC __glewProgramUniform2uiEXT = NULL;
PFNGLPROGRAMUNIFORM2UIVEXTPROC __glewProgramUniform2uivEXT = NULL;
PFNGLPROGRAMUNIFORM3FEXTPROC __glewProgramUniform3fEXT = NULL;
PFNGLPROGRAMUNIFORM3FVEXTPROC __glewProgramUniform3fvEXT = NULL;
PFNGLPROGRAMUNIFORM3IEXTPROC __glewProgramUniform3iEXT = NULL;
PFNGLPROGRAMUNIFORM3IVEXTPROC __glewProgramUniform3ivEXT = NULL;
PFNGLPROGRAMUNIFORM3UIEXTPROC __glewProgramUniform3uiEXT = NULL;
PFNGLPROGRAMUNIFORM3UIVEXTPROC __glewProgramUniform3uivEXT = NULL;
PFNGLPROGRAMUNIFORM4FEXTPROC __glewProgramUniform4fEXT = NULL;
PFNGLPROGRAMUNIFORM4FVEXTPROC __glewProgramUniform4fvEXT = NULL;
PFNGLPROGRAMUNIFORM4IEXTPROC __glewProgramUniform4iEXT = NULL;
PFNGLPROGRAMUNIFORM4IVEXTPROC __glewProgramUniform4ivEXT = NULL;
PFNGLPROGRAMUNIFORM4UIEXTPROC __glewProgramUniform4uiEXT = NULL;
PFNGLPROGRAMUNIFORM4UIVEXTPROC __glewProgramUniform4uivEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC __glewProgramUniformMatrix2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC __glewProgramUniformMatrix2x3fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC __glewProgramUniformMatrix2x4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC __glewProgramUniformMatrix3fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC __glewProgramUniformMatrix3x2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC __glewProgramUniformMatrix3x4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC __glewProgramUniformMatrix4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC __glewProgramUniformMatrix4x2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC __glewProgramUniformMatrix4x3fvEXT = NULL;
PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC __glewPushClientAttribDefaultEXT = NULL;
PFNGLTEXTUREBUFFEREXTPROC __glewTextureBufferEXT = NULL;
PFNGLTEXTUREIMAGE1DEXTPROC __glewTextureImage1DEXT = NULL;
PFNGLTEXTUREIMAGE2DEXTPROC __glewTextureImage2DEXT = NULL;
PFNGLTEXTUREIMAGE3DEXTPROC __glewTextureImage3DEXT = NULL;
PFNGLTEXTUREPARAMETERIIVEXTPROC __glewTextureParameterIivEXT = NULL;
PFNGLTEXTUREPARAMETERIUIVEXTPROC __glewTextureParameterIuivEXT = NULL;
PFNGLTEXTUREPARAMETERFEXTPROC __glewTextureParameterfEXT = NULL;
PFNGLTEXTUREPARAMETERFVEXTPROC __glewTextureParameterfvEXT = NULL;
PFNGLTEXTUREPARAMETERIEXTPROC __glewTextureParameteriEXT = NULL;
PFNGLTEXTUREPARAMETERIVEXTPROC __glewTextureParameterivEXT = NULL;
PFNGLTEXTURERENDERBUFFEREXTPROC __glewTextureRenderbufferEXT = NULL;
PFNGLTEXTURESUBIMAGE1DEXTPROC __glewTextureSubImage1DEXT = NULL;
PFNGLTEXTURESUBIMAGE2DEXTPROC __glewTextureSubImage2DEXT = NULL;
PFNGLTEXTURESUBIMAGE3DEXTPROC __glewTextureSubImage3DEXT = NULL;
PFNGLUNMAPNAMEDBUFFEREXTPROC __glewUnmapNamedBufferEXT = NULL;

PFNGLCOLORMASKINDEXEDEXTPROC __glewColorMaskIndexedEXT = NULL;
PFNGLDISABLEINDEXEDEXTPROC __glewDisableIndexedEXT = NULL;
PFNGLENABLEINDEXEDEXTPROC __glewEnableIndexedEXT = NULL;
PFNGLGETBOOLEANINDEXEDVEXTPROC __glewGetBooleanIndexedvEXT = NULL;
PFNGLGETINTEGERINDEXEDVEXTPROC __glewGetIntegerIndexedvEXT = NULL;
PFNGLISENABLEDINDEXEDEXTPROC __glewIsEnabledIndexedEXT = NULL;

PFNGLDRAWARRAYSINSTANCEDEXTPROC __glewDrawArraysInstancedEXT = NULL;
PFNGLDRAWELEMENTSINSTANCEDEXTPROC __glewDrawElementsInstancedEXT = NULL;

PFNGLDRAWRANGEELEMENTSEXTPROC __glewDrawRangeElementsEXT = NULL;

PFNGLFOGCOORDPOINTEREXTPROC __glewFogCoordPointerEXT = NULL;
PFNGLFOGCOORDDEXTPROC __glewFogCoorddEXT = NULL;
PFNGLFOGCOORDDVEXTPROC __glewFogCoorddvEXT = NULL;
PFNGLFOGCOORDFEXTPROC __glewFogCoordfEXT = NULL;
PFNGLFOGCOORDFVEXTPROC __glewFogCoordfvEXT = NULL;

PFNGLFRAGMENTCOLORMATERIALEXTPROC __glewFragmentColorMaterialEXT = NULL;
PFNGLFRAGMENTLIGHTMODELFEXTPROC __glewFragmentLightModelfEXT = NULL;
PFNGLFRAGMENTLIGHTMODELFVEXTPROC __glewFragmentLightModelfvEXT = NULL;
PFNGLFRAGMENTLIGHTMODELIEXTPROC __glewFragmentLightModeliEXT = NULL;
PFNGLFRAGMENTLIGHTMODELIVEXTPROC __glewFragmentLightModelivEXT = NULL;
PFNGLFRAGMENTLIGHTFEXTPROC __glewFragmentLightfEXT = NULL;
PFNGLFRAGMENTLIGHTFVEXTPROC __glewFragmentLightfvEXT = NULL;
PFNGLFRAGMENTLIGHTIEXTPROC __glewFragmentLightiEXT = NULL;
PFNGLFRAGMENTLIGHTIVEXTPROC __glewFragmentLightivEXT = NULL;
PFNGLFRAGMENTMATERIALFEXTPROC __glewFragmentMaterialfEXT = NULL;
PFNGLFRAGMENTMATERIALFVEXTPROC __glewFragmentMaterialfvEXT = NULL;
PFNGLFRAGMENTMATERIALIEXTPROC __glewFragmentMaterialiEXT = NULL;
PFNGLFRAGMENTMATERIALIVEXTPROC __glewFragmentMaterialivEXT = NULL;
PFNGLGETFRAGMENTLIGHTFVEXTPROC __glewGetFragmentLightfvEXT = NULL;
PFNGLGETFRAGMENTLIGHTIVEXTPROC __glewGetFragmentLightivEXT = NULL;
PFNGLGETFRAGMENTMATERIALFVEXTPROC __glewGetFragmentMaterialfvEXT = NULL;
PFNGLGETFRAGMENTMATERIALIVEXTPROC __glewGetFragmentMaterialivEXT = NULL;
PFNGLLIGHTENVIEXTPROC __glewLightEnviEXT = NULL;

PFNGLBLITFRAMEBUFFEREXTPROC __glewBlitFramebufferEXT = NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC __glewRenderbufferStorageMultisampleEXT = NULL;

PFNGLBINDFRAMEBUFFEREXTPROC __glewBindFramebufferEXT = NULL;
PFNGLBINDRENDERBUFFEREXTPROC __glewBindRenderbufferEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC __glewCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC __glewDeleteFramebuffersEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC __glewDeleteRenderbuffersEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC __glewFramebufferRenderbufferEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC __glewFramebufferTexture1DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC __glewFramebufferTexture2DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC __glewFramebufferTexture3DEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC __glewGenFramebuffersEXT = NULL;
PFNGLGENRENDERBUFFERSEXTPROC __glewGenRenderbuffersEXT = NULL;
PFNGLGENERATEMIPMAPEXTPROC __glewGenerateMipmapEXT = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC __glewGetFramebufferAttachmentParameterivEXT = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC __glewGetRenderbufferParameterivEXT = NULL;
PFNGLISFRAMEBUFFEREXTPROC __glewIsFramebufferEXT = NULL;
PFNGLISRENDERBUFFEREXTPROC __glewIsRenderbufferEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC __glewRenderbufferStorageEXT = NULL;

PFNGLFRAMEBUFFERTEXTUREEXTPROC __glewFramebufferTextureEXT = NULL;
PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC __glewFramebufferTextureFaceEXT = NULL;
PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC __glewFramebufferTextureLayerEXT = NULL;
PFNGLPROGRAMPARAMETERIEXTPROC __glewProgramParameteriEXT = NULL;

PFNGLPROGRAMENVPARAMETERS4FVEXTPROC __glewProgramEnvParameters4fvEXT = NULL;
PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC __glewProgramLocalParameters4fvEXT = NULL;

PFNGLBINDFRAGDATALOCATIONEXTPROC __glewBindFragDataLocationEXT = NULL;
PFNGLGETFRAGDATALOCATIONEXTPROC __glewGetFragDataLocationEXT = NULL;
PFNGLGETUNIFORMUIVEXTPROC __glewGetUniformuivEXT = NULL;
PFNGLGETVERTEXATTRIBIIVEXTPROC __glewGetVertexAttribIivEXT = NULL;
PFNGLGETVERTEXATTRIBIUIVEXTPROC __glewGetVertexAttribIuivEXT = NULL;
PFNGLUNIFORM1UIEXTPROC __glewUniform1uiEXT = NULL;
PFNGLUNIFORM1UIVEXTPROC __glewUniform1uivEXT = NULL;
PFNGLUNIFORM2UIEXTPROC __glewUniform2uiEXT = NULL;
PFNGLUNIFORM2UIVEXTPROC __glewUniform2uivEXT = NULL;
PFNGLUNIFORM3UIEXTPROC __glewUniform3uiEXT = NULL;
PFNGLUNIFORM3UIVEXTPROC __glewUniform3uivEXT = NULL;
PFNGLUNIFORM4UIEXTPROC __glewUniform4uiEXT = NULL;
PFNGLUNIFORM4UIVEXTPROC __glewUniform4uivEXT = NULL;
PFNGLVERTEXATTRIBI1IEXTPROC __glewVertexAttribI1iEXT = NULL;
PFNGLVERTEXATTRIBI1IVEXTPROC __glewVertexAttribI1ivEXT = NULL;
PFNGLVERTEXATTRIBI1UIEXTPROC __glewVertexAttribI1uiEXT = NULL;
PFNGLVERTEXATTRIBI1UIVEXTPROC __glewVertexAttribI1uivEXT = NULL;
PFNGLVERTEXATTRIBI2IEXTPROC __glewVertexAttribI2iEXT = NULL;
PFNGLVERTEXATTRIBI2IVEXTPROC __glewVertexAttribI2ivEXT = NULL;
PFNGLVERTEXATTRIBI2UIEXTPROC __glewVertexAttribI2uiEXT = NULL;
PFNGLVERTEXATTRIBI2UIVEXTPROC __glewVertexAttribI2uivEXT = NULL;
PFNGLVERTEXATTRIBI3IEXTPROC __glewVertexAttribI3iEXT = NULL;
PFNGLVERTEXATTRIBI3IVEXTPROC __glewVertexAttribI3ivEXT = NULL;
PFNGLVERTEXATTRIBI3UIEXTPROC __glewVertexAttribI3uiEXT = NULL;
PFNGLVERTEXATTRIBI3UIVEXTPROC __glewVertexAttribI3uivEXT = NULL;
PFNGLVERTEXATTRIBI4BVEXTPROC __glewVertexAttribI4bvEXT = NULL;
PFNGLVERTEXATTRIBI4IEXTPROC __glewVertexAttribI4iEXT = NULL;
PFNGLVERTEXATTRIBI4IVEXTPROC __glewVertexAttribI4ivEXT = NULL;
PFNGLVERTEXATTRIBI4SVEXTPROC __glewVertexAttribI4svEXT = NULL;
PFNGLVERTEXATTRIBI4UBVEXTPROC __glewVertexAttribI4ubvEXT = NULL;
PFNGLVERTEXATTRIBI4UIEXTPROC __glewVertexAttribI4uiEXT = NULL;
PFNGLVERTEXATTRIBI4UIVEXTPROC __glewVertexAttribI4uivEXT = NULL;
PFNGLVERTEXATTRIBI4USVEXTPROC __glewVertexAttribI4usvEXT = NULL;
PFNGLVERTEXATTRIBIPOINTEREXTPROC __glewVertexAttribIPointerEXT = NULL;

PFNGLGETHISTOGRAMEXTPROC __glewGetHistogramEXT = NULL;
PFNGLGETHISTOGRAMPARAMETERFVEXTPROC __glewGetHistogramParameterfvEXT = NULL;
PFNGLGETHISTOGRAMPARAMETERIVEXTPROC __glewGetHistogramParameterivEXT = NULL;
PFNGLGETMINMAXEXTPROC __glewGetMinmaxEXT = NULL;
PFNGLGETMINMAXPARAMETERFVEXTPROC __glewGetMinmaxParameterfvEXT = NULL;
PFNGLGETMINMAXPARAMETERIVEXTPROC __glewGetMinmaxParameterivEXT = NULL;
PFNGLHISTOGRAMEXTPROC __glewHistogramEXT = NULL;
PFNGLMINMAXEXTPROC __glewMinmaxEXT = NULL;
PFNGLRESETHISTOGRAMEXTPROC __glewResetHistogramEXT = NULL;
PFNGLRESETMINMAXEXTPROC __glewResetMinmaxEXT = NULL;

PFNGLINDEXFUNCEXTPROC __glewIndexFuncEXT = NULL;

PFNGLINDEXMATERIALEXTPROC __glewIndexMaterialEXT = NULL;

PFNGLAPPLYTEXTUREEXTPROC __glewApplyTextureEXT = NULL;
PFNGLTEXTURELIGHTEXTPROC __glewTextureLightEXT = NULL;
PFNGLTEXTUREMATERIALEXTPROC __glewTextureMaterialEXT = NULL;

PFNGLMULTIDRAWARRAYSEXTPROC __glewMultiDrawArraysEXT = NULL;
PFNGLMULTIDRAWELEMENTSEXTPROC __glewMultiDrawElementsEXT = NULL;

PFNGLSAMPLEMASKEXTPROC __glewSampleMaskEXT = NULL;
PFNGLSAMPLEPATTERNEXTPROC __glewSamplePatternEXT = NULL;

PFNGLCOLORTABLEEXTPROC __glewColorTableEXT = NULL;
PFNGLGETCOLORTABLEEXTPROC __glewGetColorTableEXT = NULL;
PFNGLGETCOLORTABLEPARAMETERFVEXTPROC __glewGetColorTableParameterfvEXT = NULL;
PFNGLGETCOLORTABLEPARAMETERIVEXTPROC __glewGetColorTableParameterivEXT = NULL;

PFNGLGETPIXELTRANSFORMPARAMETERFVEXTPROC __glewGetPixelTransformParameterfvEXT = NULL;
PFNGLGETPIXELTRANSFORMPARAMETERIVEXTPROC __glewGetPixelTransformParameterivEXT = NULL;
PFNGLPIXELTRANSFORMPARAMETERFEXTPROC __glewPixelTransformParameterfEXT = NULL;
PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC __glewPixelTransformParameterfvEXT = NULL;
PFNGLPIXELTRANSFORMPARAMETERIEXTPROC __glewPixelTransformParameteriEXT = NULL;
PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC __glewPixelTransformParameterivEXT = NULL;

PFNGLPOINTPARAMETERFEXTPROC __glewPointParameterfEXT = NULL;
PFNGLPOINTPARAMETERFVEXTPROC __glewPointParameterfvEXT = NULL;

PFNGLPOLYGONOFFSETEXTPROC __glewPolygonOffsetEXT = NULL;

PFNGLBEGINSCENEEXTPROC __glewBeginSceneEXT = NULL;
PFNGLENDSCENEEXTPROC __glewEndSceneEXT = NULL;

PFNGLSECONDARYCOLOR3BEXTPROC __glewSecondaryColor3bEXT = NULL;
PFNGLSECONDARYCOLOR3BVEXTPROC __glewSecondaryColor3bvEXT = NULL;
PFNGLSECONDARYCOLOR3DEXTPROC __glewSecondaryColor3dEXT = NULL;
PFNGLSECONDARYCOLOR3DVEXTPROC __glewSecondaryColor3dvEXT = NULL;
PFNGLSECONDARYCOLOR3FEXTPROC __glewSecondaryColor3fEXT = NULL;
PFNGLSECONDARYCOLOR3FVEXTPROC __glewSecondaryColor3fvEXT = NULL;
PFNGLSECONDARYCOLOR3IEXTPROC __glewSecondaryColor3iEXT = NULL;
PFNGLSECONDARYCOLOR3IVEXTPROC __glewSecondaryColor3ivEXT = NULL;
PFNGLSECONDARYCOLOR3SEXTPROC __glewSecondaryColor3sEXT = NULL;
PFNGLSECONDARYCOLOR3SVEXTPROC __glewSecondaryColor3svEXT = NULL;
PFNGLSECONDARYCOLOR3UBEXTPROC __glewSecondaryColor3ubEXT = NULL;
PFNGLSECONDARYCOLOR3UBVEXTPROC __glewSecondaryColor3ubvEXT = NULL;
PFNGLSECONDARYCOLOR3UIEXTPROC __glewSecondaryColor3uiEXT = NULL;
PFNGLSECONDARYCOLOR3UIVEXTPROC __glewSecondaryColor3uivEXT = NULL;
PFNGLSECONDARYCOLOR3USEXTPROC __glewSecondaryColor3usEXT = NULL;
PFNGLSECONDARYCOLOR3USVEXTPROC __glewSecondaryColor3usvEXT = NULL;
PFNGLSECONDARYCOLORPOINTEREXTPROC __glewSecondaryColorPointerEXT = NULL;

PFNGLACTIVESTENCILFACEEXTPROC __glewActiveStencilFaceEXT = NULL;

PFNGLTEXSUBIMAGE1DEXTPROC __glewTexSubImage1DEXT = NULL;
PFNGLTEXSUBIMAGE2DEXTPROC __glewTexSubImage2DEXT = NULL;
PFNGLTEXSUBIMAGE3DEXTPROC __glewTexSubImage3DEXT = NULL;

PFNGLTEXIMAGE3DEXTPROC __glewTexImage3DEXT = NULL;

PFNGLTEXBUFFEREXTPROC __glewTexBufferEXT = NULL;

PFNGLCLEARCOLORIIEXTPROC __glewClearColorIiEXT = NULL;
PFNGLCLEARCOLORIUIEXTPROC __glewClearColorIuiEXT = NULL;
PFNGLGETTEXPARAMETERIIVEXTPROC __glewGetTexParameterIivEXT = NULL;
PFNGLGETTEXPARAMETERIUIVEXTPROC __glewGetTexParameterIuivEXT = NULL;
PFNGLTEXPARAMETERIIVEXTPROC __glewTexParameterIivEXT = NULL;
PFNGLTEXPARAMETERIUIVEXTPROC __glewTexParameterIuivEXT = NULL;

PFNGLARETEXTURESRESIDENTEXTPROC __glewAreTexturesResidentEXT = NULL;
PFNGLBINDTEXTUREEXTPROC __glewBindTextureEXT = NULL;
PFNGLDELETETEXTURESEXTPROC __glewDeleteTexturesEXT = NULL;
PFNGLGENTEXTURESEXTPROC __glewGenTexturesEXT = NULL;
PFNGLISTEXTUREEXTPROC __glewIsTextureEXT = NULL;
PFNGLPRIORITIZETEXTURESEXTPROC __glewPrioritizeTexturesEXT = NULL;

PFNGLTEXTURENORMALEXTPROC __glewTextureNormalEXT = NULL;

PFNGLGETQUERYOBJECTI64VEXTPROC __glewGetQueryObjecti64vEXT = NULL;
PFNGLGETQUERYOBJECTUI64VEXTPROC __glewGetQueryObjectui64vEXT = NULL;

PFNGLBEGINTRANSFORMFEEDBACKEXTPROC __glewBeginTransformFeedbackEXT = NULL;
PFNGLBINDBUFFERBASEEXTPROC __glewBindBufferBaseEXT = NULL;
PFNGLBINDBUFFEROFFSETEXTPROC __glewBindBufferOffsetEXT = NULL;
PFNGLBINDBUFFERRANGEEXTPROC __glewBindBufferRangeEXT = NULL;
PFNGLENDTRANSFORMFEEDBACKEXTPROC __glewEndTransformFeedbackEXT = NULL;
PFNGLGETTRANSFORMFEEDBACKVARYINGEXTPROC __glewGetTransformFeedbackVaryingEXT = NULL;
PFNGLTRANSFORMFEEDBACKVARYINGSEXTPROC __glewTransformFeedbackVaryingsEXT = NULL;

PFNGLARRAYELEMENTEXTPROC __glewArrayElementEXT = NULL;
PFNGLCOLORPOINTEREXTPROC __glewColorPointerEXT = NULL;
PFNGLDRAWARRAYSEXTPROC __glewDrawArraysEXT = NULL;
PFNGLEDGEFLAGPOINTEREXTPROC __glewEdgeFlagPointerEXT = NULL;
PFNGLGETPOINTERVEXTPROC __glewGetPointervEXT = NULL;
PFNGLINDEXPOINTEREXTPROC __glewIndexPointerEXT = NULL;
PFNGLNORMALPOINTEREXTPROC __glewNormalPointerEXT = NULL;
PFNGLTEXCOORDPOINTEREXTPROC __glewTexCoordPointerEXT = NULL;
PFNGLVERTEXPOINTEREXTPROC __glewVertexPointerEXT = NULL;

PFNGLBEGINVERTEXSHADEREXTPROC __glewBeginVertexShaderEXT = NULL;
PFNGLBINDLIGHTPARAMETEREXTPROC __glewBindLightParameterEXT = NULL;
PFNGLBINDMATERIALPARAMETEREXTPROC __glewBindMaterialParameterEXT = NULL;
PFNGLBINDPARAMETEREXTPROC __glewBindParameterEXT = NULL;
PFNGLBINDTEXGENPARAMETEREXTPROC __glewBindTexGenParameterEXT = NULL;
PFNGLBINDTEXTUREUNITPARAMETEREXTPROC __glewBindTextureUnitParameterEXT = NULL;
PFNGLBINDVERTEXSHADEREXTPROC __glewBindVertexShaderEXT = NULL;
PFNGLDELETEVERTEXSHADEREXTPROC __glewDeleteVertexShaderEXT = NULL;
PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC __glewDisableVariantClientStateEXT = NULL;
PFNGLENABLEVARIANTCLIENTSTATEEXTPROC __glewEnableVariantClientStateEXT = NULL;
PFNGLENDVERTEXSHADEREXTPROC __glewEndVertexShaderEXT = NULL;
PFNGLEXTRACTCOMPONENTEXTPROC __glewExtractComponentEXT = NULL;
PFNGLGENSYMBOLSEXTPROC __glewGenSymbolsEXT = NULL;
PFNGLGENVERTEXSHADERSEXTPROC __glewGenVertexShadersEXT = NULL;
PFNGLGETINVARIANTBOOLEANVEXTPROC __glewGetInvariantBooleanvEXT = NULL;
PFNGLGETINVARIANTFLOATVEXTPROC __glewGetInvariantFloatvEXT = NULL;
PFNGLGETINVARIANTINTEGERVEXTPROC __glewGetInvariantIntegervEXT = NULL;
PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC __glewGetLocalConstantBooleanvEXT = NULL;
PFNGLGETLOCALCONSTANTFLOATVEXTPROC __glewGetLocalConstantFloatvEXT = NULL;
PFNGLGETLOCALCONSTANTINTEGERVEXTPROC __glewGetLocalConstantIntegervEXT = NULL;
PFNGLGETVARIANTBOOLEANVEXTPROC __glewGetVariantBooleanvEXT = NULL;
PFNGLGETVARIANTFLOATVEXTPROC __glewGetVariantFloatvEXT = NULL;
PFNGLGETVARIANTINTEGERVEXTPROC __glewGetVariantIntegervEXT = NULL;
PFNGLGETVARIANTPOINTERVEXTPROC __glewGetVariantPointervEXT = NULL;
PFNGLINSERTCOMPONENTEXTPROC __glewInsertComponentEXT = NULL;
PFNGLISVARIANTENABLEDEXTPROC __glewIsVariantEnabledEXT = NULL;
PFNGLSETINVARIANTEXTPROC __glewSetInvariantEXT = NULL;
PFNGLSETLOCALCONSTANTEXTPROC __glewSetLocalConstantEXT = NULL;
PFNGLSHADEROP1EXTPROC __glewShaderOp1EXT = NULL;
PFNGLSHADEROP2EXTPROC __glewShaderOp2EXT = NULL;
PFNGLSHADEROP3EXTPROC __glewShaderOp3EXT = NULL;
PFNGLSWIZZLEEXTPROC __glewSwizzleEXT = NULL;
PFNGLVARIANTPOINTEREXTPROC __glewVariantPointerEXT = NULL;
PFNGLVARIANTBVEXTPROC __glewVariantbvEXT = NULL;
PFNGLVARIANTDVEXTPROC __glewVariantdvEXT = NULL;
PFNGLVARIANTFVEXTPROC __glewVariantfvEXT = NULL;
PFNGLVARIANTIVEXTPROC __glewVariantivEXT = NULL;
PFNGLVARIANTSVEXTPROC __glewVariantsvEXT = NULL;
PFNGLVARIANTUBVEXTPROC __glewVariantubvEXT = NULL;
PFNGLVARIANTUIVEXTPROC __glewVariantuivEXT = NULL;
PFNGLVARIANTUSVEXTPROC __glewVariantusvEXT = NULL;
PFNGLWRITEMASKEXTPROC __glewWriteMaskEXT = NULL;

PFNGLVERTEXWEIGHTPOINTEREXTPROC __glewVertexWeightPointerEXT = NULL;
PFNGLVERTEXWEIGHTFEXTPROC __glewVertexWeightfEXT = NULL;
PFNGLVERTEXWEIGHTFVEXTPROC __glewVertexWeightfvEXT = NULL;

PFNGLFRAMETERMINATORGREMEDYPROC __glewFrameTerminatorGREMEDY = NULL;

PFNGLSTRINGMARKERGREMEDYPROC __glewStringMarkerGREMEDY = NULL;

PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC __glewGetImageTransformParameterfvHP = NULL;
PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC __glewGetImageTransformParameterivHP = NULL;
PFNGLIMAGETRANSFORMPARAMETERFHPPROC __glewImageTransformParameterfHP = NULL;
PFNGLIMAGETRANSFORMPARAMETERFVHPPROC __glewImageTransformParameterfvHP = NULL;
PFNGLIMAGETRANSFORMPARAMETERIHPPROC __glewImageTransformParameteriHP = NULL;
PFNGLIMAGETRANSFORMPARAMETERIVHPPROC __glewImageTransformParameterivHP = NULL;

PFNGLMULTIMODEDRAWARRAYSIBMPROC __glewMultiModeDrawArraysIBM = NULL;
PFNGLMULTIMODEDRAWELEMENTSIBMPROC __glewMultiModeDrawElementsIBM = NULL;

PFNGLCOLORPOINTERLISTIBMPROC __glewColorPointerListIBM = NULL;
PFNGLEDGEFLAGPOINTERLISTIBMPROC __glewEdgeFlagPointerListIBM = NULL;
PFNGLFOGCOORDPOINTERLISTIBMPROC __glewFogCoordPointerListIBM = NULL;
PFNGLINDEXPOINTERLISTIBMPROC __glewIndexPointerListIBM = NULL;
PFNGLNORMALPOINTERLISTIBMPROC __glewNormalPointerListIBM = NULL;
PFNGLSECONDARYCOLORPOINTERLISTIBMPROC __glewSecondaryColorPointerListIBM = NULL;
PFNGLTEXCOORDPOINTERLISTIBMPROC __glewTexCoordPointerListIBM = NULL;
PFNGLVERTEXPOINTERLISTIBMPROC __glewVertexPointerListIBM = NULL;

PFNGLCOLORPOINTERVINTELPROC __glewColorPointervINTEL = NULL;
PFNGLNORMALPOINTERVINTELPROC __glewNormalPointervINTEL = NULL;
PFNGLTEXCOORDPOINTERVINTELPROC __glewTexCoordPointervINTEL = NULL;
PFNGLVERTEXPOINTERVINTELPROC __glewVertexPointervINTEL = NULL;

PFNGLTEXSCISSORFUNCINTELPROC __glewTexScissorFuncINTEL = NULL;
PFNGLTEXSCISSORINTELPROC __glewTexScissorINTEL = NULL;

PFNGLBUFFERREGIONENABLEDEXTPROC __glewBufferRegionEnabledEXT = NULL;
PFNGLDELETEBUFFERREGIONEXTPROC __glewDeleteBufferRegionEXT = NULL;
PFNGLDRAWBUFFERREGIONEXTPROC __glewDrawBufferRegionEXT = NULL;
PFNGLNEWBUFFERREGIONEXTPROC __glewNewBufferRegionEXT = NULL;
PFNGLREADBUFFERREGIONEXTPROC __glewReadBufferRegionEXT = NULL;

PFNGLRESIZEBUFFERSMESAPROC __glewResizeBuffersMESA = NULL;

PFNGLWINDOWPOS2DMESAPROC __glewWindowPos2dMESA = NULL;
PFNGLWINDOWPOS2DVMESAPROC __glewWindowPos2dvMESA = NULL;
PFNGLWINDOWPOS2FMESAPROC __glewWindowPos2fMESA = NULL;
PFNGLWINDOWPOS2FVMESAPROC __glewWindowPos2fvMESA = NULL;
PFNGLWINDOWPOS2IMESAPROC __glewWindowPos2iMESA = NULL;
PFNGLWINDOWPOS2IVMESAPROC __glewWindowPos2ivMESA = NULL;
PFNGLWINDOWPOS2SMESAPROC __glewWindowPos2sMESA = NULL;
PFNGLWINDOWPOS2SVMESAPROC __glewWindowPos2svMESA = NULL;
PFNGLWINDOWPOS3DMESAPROC __glewWindowPos3dMESA = NULL;
PFNGLWINDOWPOS3DVMESAPROC __glewWindowPos3dvMESA = NULL;
PFNGLWINDOWPOS3FMESAPROC __glewWindowPos3fMESA = NULL;
PFNGLWINDOWPOS3FVMESAPROC __glewWindowPos3fvMESA = NULL;
PFNGLWINDOWPOS3IMESAPROC __glewWindowPos3iMESA = NULL;
PFNGLWINDOWPOS3IVMESAPROC __glewWindowPos3ivMESA = NULL;
PFNGLWINDOWPOS3SMESAPROC __glewWindowPos3sMESA = NULL;
PFNGLWINDOWPOS3SVMESAPROC __glewWindowPos3svMESA = NULL;
PFNGLWINDOWPOS4DMESAPROC __glewWindowPos4dMESA = NULL;
PFNGLWINDOWPOS4DVMESAPROC __glewWindowPos4dvMESA = NULL;
PFNGLWINDOWPOS4FMESAPROC __glewWindowPos4fMESA = NULL;
PFNGLWINDOWPOS4FVMESAPROC __glewWindowPos4fvMESA = NULL;
PFNGLWINDOWPOS4IMESAPROC __glewWindowPos4iMESA = NULL;
PFNGLWINDOWPOS4IVMESAPROC __glewWindowPos4ivMESA = NULL;
PFNGLWINDOWPOS4SMESAPROC __glewWindowPos4sMESA = NULL;
PFNGLWINDOWPOS4SVMESAPROC __glewWindowPos4svMESA = NULL;

PFNGLBEGINCONDITIONALRENDERNVPROC __glewBeginConditionalRenderNV = NULL;
PFNGLENDCONDITIONALRENDERNVPROC __glewEndConditionalRenderNV = NULL;

PFNGLCLEARDEPTHDNVPROC __glewClearDepthdNV = NULL;
PFNGLDEPTHBOUNDSDNVPROC __glewDepthBoundsdNV = NULL;
PFNGLDEPTHRANGEDNVPROC __glewDepthRangedNV = NULL;

PFNGLEVALMAPSNVPROC __glewEvalMapsNV = NULL;
PFNGLGETMAPATTRIBPARAMETERFVNVPROC __glewGetMapAttribParameterfvNV = NULL;
PFNGLGETMAPATTRIBPARAMETERIVNVPROC __glewGetMapAttribParameterivNV = NULL;
PFNGLGETMAPCONTROLPOINTSNVPROC __glewGetMapControlPointsNV = NULL;
PFNGLGETMAPPARAMETERFVNVPROC __glewGetMapParameterfvNV = NULL;
PFNGLGETMAPPARAMETERIVNVPROC __glewGetMapParameterivNV = NULL;
PFNGLMAPCONTROLPOINTSNVPROC __glewMapControlPointsNV = NULL;
PFNGLMAPPARAMETERFVNVPROC __glewMapParameterfvNV = NULL;
PFNGLMAPPARAMETERIVNVPROC __glewMapParameterivNV = NULL;

PFNGLGETMULTISAMPLEFVNVPROC __glewGetMultisamplefvNV = NULL;
PFNGLSAMPLEMASKINDEXEDNVPROC __glewSampleMaskIndexedNV = NULL;
PFNGLTEXRENDERBUFFERNVPROC __glewTexRenderbufferNV = NULL;

PFNGLDELETEFENCESNVPROC __glewDeleteFencesNV = NULL;
PFNGLFINISHFENCENVPROC __glewFinishFenceNV = NULL;
PFNGLGENFENCESNVPROC __glewGenFencesNV = NULL;
PFNGLGETFENCEIVNVPROC __glewGetFenceivNV = NULL;
PFNGLISFENCENVPROC __glewIsFenceNV = NULL;
PFNGLSETFENCENVPROC __glewSetFenceNV = NULL;
PFNGLTESTFENCENVPROC __glewTestFenceNV = NULL;

PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC __glewGetProgramNamedParameterdvNV = NULL;
PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC __glewGetProgramNamedParameterfvNV = NULL;
PFNGLPROGRAMNAMEDPARAMETER4DNVPROC __glewProgramNamedParameter4dNV = NULL;
PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC __glewProgramNamedParameter4dvNV = NULL;
PFNGLPROGRAMNAMEDPARAMETER4FNVPROC __glewProgramNamedParameter4fNV = NULL;
PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC __glewProgramNamedParameter4fvNV = NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC __glewRenderbufferStorageMultisampleCoverageNV = NULL;

PFNGLPROGRAMVERTEXLIMITNVPROC __glewProgramVertexLimitNV = NULL;

PFNGLPROGRAMENVPARAMETERI4INVPROC __glewProgramEnvParameterI4iNV = NULL;
PFNGLPROGRAMENVPARAMETERI4IVNVPROC __glewProgramEnvParameterI4ivNV = NULL;
PFNGLPROGRAMENVPARAMETERI4UINVPROC __glewProgramEnvParameterI4uiNV = NULL;
PFNGLPROGRAMENVPARAMETERI4UIVNVPROC __glewProgramEnvParameterI4uivNV = NULL;
PFNGLPROGRAMENVPARAMETERSI4IVNVPROC __glewProgramEnvParametersI4ivNV = NULL;
PFNGLPROGRAMENVPARAMETERSI4UIVNVPROC __glewProgramEnvParametersI4uivNV = NULL;
PFNGLPROGRAMLOCALPARAMETERI4INVPROC __glewProgramLocalParameterI4iNV = NULL;
PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC __glewProgramLocalParameterI4ivNV = NULL;
PFNGLPROGRAMLOCALPARAMETERI4UINVPROC __glewProgramLocalParameterI4uiNV = NULL;
PFNGLPROGRAMLOCALPARAMETERI4UIVNVPROC __glewProgramLocalParameterI4uivNV = NULL;
PFNGLPROGRAMLOCALPARAMETERSI4IVNVPROC __glewProgramLocalParametersI4ivNV = NULL;
PFNGLPROGRAMLOCALPARAMETERSI4UIVNVPROC __glewProgramLocalParametersI4uivNV = NULL;

PFNGLCOLOR3HNVPROC __glewColor3hNV = NULL;
PFNGLCOLOR3HVNVPROC __glewColor3hvNV = NULL;
PFNGLCOLOR4HNVPROC __glewColor4hNV = NULL;
PFNGLCOLOR4HVNVPROC __glewColor4hvNV = NULL;
PFNGLFOGCOORDHNVPROC __glewFogCoordhNV = NULL;
PFNGLFOGCOORDHVNVPROC __glewFogCoordhvNV = NULL;
PFNGLMULTITEXCOORD1HNVPROC __glewMultiTexCoord1hNV = NULL;
PFNGLMULTITEXCOORD1HVNVPROC __glewMultiTexCoord1hvNV = NULL;
PFNGLMULTITEXCOORD2HNVPROC __glewMultiTexCoord2hNV = NULL;
PFNGLMULTITEXCOORD2HVNVPROC __glewMultiTexCoord2hvNV = NULL;
PFNGLMULTITEXCOORD3HNVPROC __glewMultiTexCoord3hNV = NULL;
PFNGLMULTITEXCOORD3HVNVPROC __glewMultiTexCoord3hvNV = NULL;
PFNGLMULTITEXCOORD4HNVPROC __glewMultiTexCoord4hNV = NULL;
PFNGLMULTITEXCOORD4HVNVPROC __glewMultiTexCoord4hvNV = NULL;
PFNGLNORMAL3HNVPROC __glewNormal3hNV = NULL;
PFNGLNORMAL3HVNVPROC __glewNormal3hvNV = NULL;
PFNGLSECONDARYCOLOR3HNVPROC __glewSecondaryColor3hNV = NULL;
PFNGLSECONDARYCOLOR3HVNVPROC __glewSecondaryColor3hvNV = NULL;
PFNGLTEXCOORD1HNVPROC __glewTexCoord1hNV = NULL;
PFNGLTEXCOORD1HVNVPROC __glewTexCoord1hvNV = NULL;
PFNGLTEXCOORD2HNVPROC __glewTexCoord2hNV = NULL;
PFNGLTEXCOORD2HVNVPROC __glewTexCoord2hvNV = NULL;
PFNGLTEXCOORD3HNVPROC __glewTexCoord3hNV = NULL;
PFNGLTEXCOORD3HVNVPROC __glewTexCoord3hvNV = NULL;
PFNGLTEXCOORD4HNVPROC __glewTexCoord4hNV = NULL;
PFNGLTEXCOORD4HVNVPROC __glewTexCoord4hvNV = NULL;
PFNGLVERTEX2HNVPROC __glewVertex2hNV = NULL;
PFNGLVERTEX2HVNVPROC __glewVertex2hvNV = NULL;
PFNGLVERTEX3HNVPROC __glewVertex3hNV = NULL;
PFNGLVERTEX3HVNVPROC __glewVertex3hvNV = NULL;
PFNGLVERTEX4HNVPROC __glewVertex4hNV = NULL;
PFNGLVERTEX4HVNVPROC __glewVertex4hvNV = NULL;
PFNGLVERTEXATTRIB1HNVPROC __glewVertexAttrib1hNV = NULL;
PFNGLVERTEXATTRIB1HVNVPROC __glewVertexAttrib1hvNV = NULL;
PFNGLVERTEXATTRIB2HNVPROC __glewVertexAttrib2hNV = NULL;
PFNGLVERTEXATTRIB2HVNVPROC __glewVertexAttrib2hvNV = NULL;
PFNGLVERTEXATTRIB3HNVPROC __glewVertexAttrib3hNV = NULL;
PFNGLVERTEXATTRIB3HVNVPROC __glewVertexAttrib3hvNV = NULL;
PFNGLVERTEXATTRIB4HNVPROC __glewVertexAttrib4hNV = NULL;
PFNGLVERTEXATTRIB4HVNVPROC __glewVertexAttrib4hvNV = NULL;
PFNGLVERTEXATTRIBS1HVNVPROC __glewVertexAttribs1hvNV = NULL;
PFNGLVERTEXATTRIBS2HVNVPROC __glewVertexAttribs2hvNV = NULL;
PFNGLVERTEXATTRIBS3HVNVPROC __glewVertexAttribs3hvNV = NULL;
PFNGLVERTEXATTRIBS4HVNVPROC __glewVertexAttribs4hvNV = NULL;
PFNGLVERTEXWEIGHTHNVPROC __glewVertexWeighthNV = NULL;
PFNGLVERTEXWEIGHTHVNVPROC __glewVertexWeighthvNV = NULL;

PFNGLBEGINOCCLUSIONQUERYNVPROC __glewBeginOcclusionQueryNV = NULL;
PFNGLDELETEOCCLUSIONQUERIESNVPROC __glewDeleteOcclusionQueriesNV = NULL;
PFNGLENDOCCLUSIONQUERYNVPROC __glewEndOcclusionQueryNV = NULL;
PFNGLGENOCCLUSIONQUERIESNVPROC __glewGenOcclusionQueriesNV = NULL;
PFNGLGETOCCLUSIONQUERYIVNVPROC __glewGetOcclusionQueryivNV = NULL;
PFNGLGETOCCLUSIONQUERYUIVNVPROC __glewGetOcclusionQueryuivNV = NULL;
PFNGLISOCCLUSIONQUERYNVPROC __glewIsOcclusionQueryNV = NULL;

PFNGLPROGRAMBUFFERPARAMETERSIIVNVPROC __glewProgramBufferParametersIivNV = NULL;
PFNGLPROGRAMBUFFERPARAMETERSIUIVNVPROC __glewProgramBufferParametersIuivNV = NULL;
PFNGLPROGRAMBUFFERPARAMETERSFVNVPROC __glewProgramBufferParametersfvNV = NULL;

PFNGLFLUSHPIXELDATARANGENVPROC __glewFlushPixelDataRangeNV = NULL;
PFNGLPIXELDATARANGENVPROC __glewPixelDataRangeNV = NULL;

PFNGLPOINTPARAMETERINVPROC __glewPointParameteriNV = NULL;
PFNGLPOINTPARAMETERIVNVPROC __glewPointParameterivNV = NULL;

PFNGLGETVIDEOI64VNVPROC __glewGetVideoi64vNV = NULL;
PFNGLGETVIDEOIVNVPROC __glewGetVideoivNV = NULL;
PFNGLGETVIDEOUI64VNVPROC __glewGetVideoui64vNV = NULL;
PFNGLGETVIDEOUIVNVPROC __glewGetVideouivNV = NULL;
PFNGLPRESENTFRAMEDUALFILLNVPROC __glewPresentFrameDualFillNV = NULL;
PFNGLPRESENTFRAMEKEYEDNVPROC __glewPresentFrameKeyedNV = NULL;
PFNGLVIDEOPARAMETERIVNVPROC __glewVideoParameterivNV = NULL;

PFNGLPRIMITIVERESTARTINDEXNVPROC __glewPrimitiveRestartIndexNV = NULL;
PFNGLPRIMITIVERESTARTNVPROC __glewPrimitiveRestartNV = NULL;

PFNGLCOMBINERINPUTNVPROC __glewCombinerInputNV = NULL;
PFNGLCOMBINEROUTPUTNVPROC __glewCombinerOutputNV = NULL;
PFNGLCOMBINERPARAMETERFNVPROC __glewCombinerParameterfNV = NULL;
PFNGLCOMBINERPARAMETERFVNVPROC __glewCombinerParameterfvNV = NULL;
PFNGLCOMBINERPARAMETERINVPROC __glewCombinerParameteriNV = NULL;
PFNGLCOMBINERPARAMETERIVNVPROC __glewCombinerParameterivNV = NULL;
PFNGLFINALCOMBINERINPUTNVPROC __glewFinalCombinerInputNV = NULL;
PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC __glewGetCombinerInputParameterfvNV = NULL;
PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC __glewGetCombinerInputParameterivNV = NULL;
PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC __glewGetCombinerOutputParameterfvNV = NULL;
PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC __glewGetCombinerOutputParameterivNV = NULL;
PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC __glewGetFinalCombinerInputParameterfvNV = NULL;
PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC __glewGetFinalCombinerInputParameterivNV = NULL;

PFNGLCOMBINERSTAGEPARAMETERFVNVPROC __glewCombinerStageParameterfvNV = NULL;
PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC __glewGetCombinerStageParameterfvNV = NULL;

PFNGLACTIVEVARYINGNVPROC __glewActiveVaryingNV = NULL;
PFNGLBEGINTRANSFORMFEEDBACKNVPROC __glewBeginTransformFeedbackNV = NULL;
PFNGLBINDBUFFERBASENVPROC __glewBindBufferBaseNV = NULL;
PFNGLBINDBUFFEROFFSETNVPROC __glewBindBufferOffsetNV = NULL;
PFNGLBINDBUFFERRANGENVPROC __glewBindBufferRangeNV = NULL;
PFNGLENDTRANSFORMFEEDBACKNVPROC __glewEndTransformFeedbackNV = NULL;
PFNGLGETACTIVEVARYINGNVPROC __glewGetActiveVaryingNV = NULL;
PFNGLGETTRANSFORMFEEDBACKVARYINGNVPROC __glewGetTransformFeedbackVaryingNV = NULL;
PFNGLGETVARYINGLOCATIONNVPROC __glewGetVaryingLocationNV = NULL;
PFNGLTRANSFORMFEEDBACKATTRIBSNVPROC __glewTransformFeedbackAttribsNV = NULL;
PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC __glewTransformFeedbackVaryingsNV = NULL;

PFNGLBINDTRANSFORMFEEDBACKNVPROC __glewBindTransformFeedbackNV = NULL;
PFNGLDELETETRANSFORMFEEDBACKSNVPROC __glewDeleteTransformFeedbacksNV = NULL;
PFNGLDRAWTRANSFORMFEEDBACKNVPROC __glewDrawTransformFeedbackNV = NULL;
PFNGLGENTRANSFORMFEEDBACKSNVPROC __glewGenTransformFeedbacksNV = NULL;
PFNGLISTRANSFORMFEEDBACKNVPROC __glewIsTransformFeedbackNV = NULL;
PFNGLPAUSETRANSFORMFEEDBACKNVPROC __glewPauseTransformFeedbackNV = NULL;
PFNGLRESUMETRANSFORMFEEDBACKNVPROC __glewResumeTransformFeedbackNV = NULL;

PFNGLFLUSHVERTEXARRAYRANGENVPROC __glewFlushVertexArrayRangeNV = NULL;
PFNGLVERTEXARRAYRANGENVPROC __glewVertexArrayRangeNV = NULL;

PFNGLAREPROGRAMSRESIDENTNVPROC __glewAreProgramsResidentNV = NULL;
PFNGLBINDPROGRAMNVPROC __glewBindProgramNV = NULL;
PFNGLDELETEPROGRAMSNVPROC __glewDeleteProgramsNV = NULL;
PFNGLEXECUTEPROGRAMNVPROC __glewExecuteProgramNV = NULL;
PFNGLGENPROGRAMSNVPROC __glewGenProgramsNV = NULL;
PFNGLGETPROGRAMPARAMETERDVNVPROC __glewGetProgramParameterdvNV = NULL;
PFNGLGETPROGRAMPARAMETERFVNVPROC __glewGetProgramParameterfvNV = NULL;
PFNGLGETPROGRAMSTRINGNVPROC __glewGetProgramStringNV = NULL;
PFNGLGETPROGRAMIVNVPROC __glewGetProgramivNV = NULL;
PFNGLGETTRACKMATRIXIVNVPROC __glewGetTrackMatrixivNV = NULL;
PFNGLGETVERTEXATTRIBPOINTERVNVPROC __glewGetVertexAttribPointervNV = NULL;
PFNGLGETVERTEXATTRIBDVNVPROC __glewGetVertexAttribdvNV = NULL;
PFNGLGETVERTEXATTRIBFVNVPROC __glewGetVertexAttribfvNV = NULL;
PFNGLGETVERTEXATTRIBIVNVPROC __glewGetVertexAttribivNV = NULL;
PFNGLISPROGRAMNVPROC __glewIsProgramNV = NULL;
PFNGLLOADPROGRAMNVPROC __glewLoadProgramNV = NULL;
PFNGLPROGRAMPARAMETER4DNVPROC __glewProgramParameter4dNV = NULL;
PFNGLPROGRAMPARAMETER4DVNVPROC __glewProgramParameter4dvNV = NULL;
PFNGLPROGRAMPARAMETER4FNVPROC __glewProgramParameter4fNV = NULL;
PFNGLPROGRAMPARAMETER4FVNVPROC __glewProgramParameter4fvNV = NULL;
PFNGLPROGRAMPARAMETERS4DVNVPROC __glewProgramParameters4dvNV = NULL;
PFNGLPROGRAMPARAMETERS4FVNVPROC __glewProgramParameters4fvNV = NULL;
PFNGLREQUESTRESIDENTPROGRAMSNVPROC __glewRequestResidentProgramsNV = NULL;
PFNGLTRACKMATRIXNVPROC __glewTrackMatrixNV = NULL;
PFNGLVERTEXATTRIB1DNVPROC __glewVertexAttrib1dNV = NULL;
PFNGLVERTEXATTRIB1DVNVPROC __glewVertexAttrib1dvNV = NULL;
PFNGLVERTEXATTRIB1FNVPROC __glewVertexAttrib1fNV = NULL;
PFNGLVERTEXATTRIB1FVNVPROC __glewVertexAttrib1fvNV = NULL;
PFNGLVERTEXATTRIB1SNVPROC __glewVertexAttrib1sNV = NULL;
PFNGLVERTEXATTRIB1SVNVPROC __glewVertexAttrib1svNV = NULL;
PFNGLVERTEXATTRIB2DNVPROC __glewVertexAttrib2dNV = NULL;
PFNGLVERTEXATTRIB2DVNVPROC __glewVertexAttrib2dvNV = NULL;
PFNGLVERTEXATTRIB2FNVPROC __glewVertexAttrib2fNV = NULL;
PFNGLVERTEXATTRIB2FVNVPROC __glewVertexAttrib2fvNV = NULL;
PFNGLVERTEXATTRIB2SNVPROC __glewVertexAttrib2sNV = NULL;
PFNGLVERTEXATTRIB2SVNVPROC __glewVertexAttrib2svNV = NULL;
PFNGLVERTEXATTRIB3DNVPROC __glewVertexAttrib3dNV = NULL;
PFNGLVERTEXATTRIB3DVNVPROC __glewVertexAttrib3dvNV = NULL;
PFNGLVERTEXATTRIB3FNVPROC __glewVertexAttrib3fNV = NULL;
PFNGLVERTEXATTRIB3FVNVPROC __glewVertexAttrib3fvNV = NULL;
PFNGLVERTEXATTRIB3SNVPROC __glewVertexAttrib3sNV = NULL;
PFNGLVERTEXATTRIB3SVNVPROC __glewVertexAttrib3svNV = NULL;
PFNGLVERTEXATTRIB4DNVPROC __glewVertexAttrib4dNV = NULL;
PFNGLVERTEXATTRIB4DVNVPROC __glewVertexAttrib4dvNV = NULL;
PFNGLVERTEXATTRIB4FNVPROC __glewVertexAttrib4fNV = NULL;
PFNGLVERTEXATTRIB4FVNVPROC __glewVertexAttrib4fvNV = NULL;
PFNGLVERTEXATTRIB4SNVPROC __glewVertexAttrib4sNV = NULL;
PFNGLVERTEXATTRIB4SVNVPROC __glewVertexAttrib4svNV = NULL;
PFNGLVERTEXATTRIB4UBNVPROC __glewVertexAttrib4ubNV = NULL;
PFNGLVERTEXATTRIB4UBVNVPROC __glewVertexAttrib4ubvNV = NULL;
PFNGLVERTEXATTRIBPOINTERNVPROC __glewVertexAttribPointerNV = NULL;
PFNGLVERTEXATTRIBS1DVNVPROC __glewVertexAttribs1dvNV = NULL;
PFNGLVERTEXATTRIBS1FVNVPROC __glewVertexAttribs1fvNV = NULL;
PFNGLVERTEXATTRIBS1SVNVPROC __glewVertexAttribs1svNV = NULL;
PFNGLVERTEXATTRIBS2DVNVPROC __glewVertexAttribs2dvNV = NULL;
PFNGLVERTEXATTRIBS2FVNVPROC __glewVertexAttribs2fvNV = NULL;
PFNGLVERTEXATTRIBS2SVNVPROC __glewVertexAttribs2svNV = NULL;
PFNGLVERTEXATTRIBS3DVNVPROC __glewVertexAttribs3dvNV = NULL;
PFNGLVERTEXATTRIBS3FVNVPROC __glewVertexAttribs3fvNV = NULL;
PFNGLVERTEXATTRIBS3SVNVPROC __glewVertexAttribs3svNV = NULL;
PFNGLVERTEXATTRIBS4DVNVPROC __glewVertexAttribs4dvNV = NULL;
PFNGLVERTEXATTRIBS4FVNVPROC __glewVertexAttribs4fvNV = NULL;
PFNGLVERTEXATTRIBS4SVNVPROC __glewVertexAttribs4svNV = NULL;
PFNGLVERTEXATTRIBS4UBVNVPROC __glewVertexAttribs4ubvNV = NULL;

PFNGLCLEARDEPTHFOESPROC __glewClearDepthfOES = NULL;
PFNGLCLIPPLANEFOESPROC __glewClipPlanefOES = NULL;
PFNGLDEPTHRANGEFOESPROC __glewDepthRangefOES = NULL;
PFNGLFRUSTUMFOESPROC __glewFrustumfOES = NULL;
PFNGLGETCLIPPLANEFOESPROC __glewGetClipPlanefOES = NULL;
PFNGLORTHOFOESPROC __glewOrthofOES = NULL;

PFNGLDETAILTEXFUNCSGISPROC __glewDetailTexFuncSGIS = NULL;
PFNGLGETDETAILTEXFUNCSGISPROC __glewGetDetailTexFuncSGIS = NULL;

PFNGLFOGFUNCSGISPROC __glewFogFuncSGIS = NULL;
PFNGLGETFOGFUNCSGISPROC __glewGetFogFuncSGIS = NULL;

PFNGLSAMPLEMASKSGISPROC __glewSampleMaskSGIS = NULL;
PFNGLSAMPLEPATTERNSGISPROC __glewSamplePatternSGIS = NULL;

PFNGLGETSHARPENTEXFUNCSGISPROC __glewGetSharpenTexFuncSGIS = NULL;
PFNGLSHARPENTEXFUNCSGISPROC __glewSharpenTexFuncSGIS = NULL;

PFNGLTEXIMAGE4DSGISPROC __glewTexImage4DSGIS = NULL;
PFNGLTEXSUBIMAGE4DSGISPROC __glewTexSubImage4DSGIS = NULL;

PFNGLGETTEXFILTERFUNCSGISPROC __glewGetTexFilterFuncSGIS = NULL;
PFNGLTEXFILTERFUNCSGISPROC __glewTexFilterFuncSGIS = NULL;

PFNGLASYNCMARKERSGIXPROC __glewAsyncMarkerSGIX = NULL;
PFNGLDELETEASYNCMARKERSSGIXPROC __glewDeleteAsyncMarkersSGIX = NULL;
PFNGLFINISHASYNCSGIXPROC __glewFinishAsyncSGIX = NULL;
PFNGLGENASYNCMARKERSSGIXPROC __glewGenAsyncMarkersSGIX = NULL;
PFNGLISASYNCMARKERSGIXPROC __glewIsAsyncMarkerSGIX = NULL;
PFNGLPOLLASYNCSGIXPROC __glewPollAsyncSGIX = NULL;

PFNGLFLUSHRASTERSGIXPROC __glewFlushRasterSGIX = NULL;

PFNGLTEXTUREFOGSGIXPROC __glewTextureFogSGIX = NULL;

PFNGLFRAGMENTCOLORMATERIALSGIXPROC __glewFragmentColorMaterialSGIX = NULL;
PFNGLFRAGMENTLIGHTMODELFSGIXPROC __glewFragmentLightModelfSGIX = NULL;
PFNGLFRAGMENTLIGHTMODELFVSGIXPROC __glewFragmentLightModelfvSGIX = NULL;
PFNGLFRAGMENTLIGHTMODELISGIXPROC __glewFragmentLightModeliSGIX = NULL;
PFNGLFRAGMENTLIGHTMODELIVSGIXPROC __glewFragmentLightModelivSGIX = NULL;
PFNGLFRAGMENTLIGHTFSGIXPROC __glewFragmentLightfSGIX = NULL;
PFNGLFRAGMENTLIGHTFVSGIXPROC __glewFragmentLightfvSGIX = NULL;
PFNGLFRAGMENTLIGHTISGIXPROC __glewFragmentLightiSGIX = NULL;
PFNGLFRAGMENTLIGHTIVSGIXPROC __glewFragmentLightivSGIX = NULL;
PFNGLFRAGMENTMATERIALFSGIXPROC __glewFragmentMaterialfSGIX = NULL;
PFNGLFRAGMENTMATERIALFVSGIXPROC __glewFragmentMaterialfvSGIX = NULL;
PFNGLFRAGMENTMATERIALISGIXPROC __glewFragmentMaterialiSGIX = NULL;
PFNGLFRAGMENTMATERIALIVSGIXPROC __glewFragmentMaterialivSGIX = NULL;
PFNGLGETFRAGMENTLIGHTFVSGIXPROC __glewGetFragmentLightfvSGIX = NULL;
PFNGLGETFRAGMENTLIGHTIVSGIXPROC __glewGetFragmentLightivSGIX = NULL;
PFNGLGETFRAGMENTMATERIALFVSGIXPROC __glewGetFragmentMaterialfvSGIX = NULL;
PFNGLGETFRAGMENTMATERIALIVSGIXPROC __glewGetFragmentMaterialivSGIX = NULL;

PFNGLFRAMEZOOMSGIXPROC __glewFrameZoomSGIX = NULL;

PFNGLPIXELTEXGENSGIXPROC __glewPixelTexGenSGIX = NULL;

PFNGLREFERENCEPLANESGIXPROC __glewReferencePlaneSGIX = NULL;

PFNGLSPRITEPARAMETERFSGIXPROC __glewSpriteParameterfSGIX = NULL;
PFNGLSPRITEPARAMETERFVSGIXPROC __glewSpriteParameterfvSGIX = NULL;
PFNGLSPRITEPARAMETERISGIXPROC __glewSpriteParameteriSGIX = NULL;
PFNGLSPRITEPARAMETERIVSGIXPROC __glewSpriteParameterivSGIX = NULL;

PFNGLTAGSAMPLEBUFFERSGIXPROC __glewTagSampleBufferSGIX = NULL;

PFNGLCOLORTABLEPARAMETERFVSGIPROC __glewColorTableParameterfvSGI = NULL;
PFNGLCOLORTABLEPARAMETERIVSGIPROC __glewColorTableParameterivSGI = NULL;
PFNGLCOLORTABLESGIPROC __glewColorTableSGI = NULL;
PFNGLCOPYCOLORTABLESGIPROC __glewCopyColorTableSGI = NULL;
PFNGLGETCOLORTABLEPARAMETERFVSGIPROC __glewGetColorTableParameterfvSGI = NULL;
PFNGLGETCOLORTABLEPARAMETERIVSGIPROC __glewGetColorTableParameterivSGI = NULL;
PFNGLGETCOLORTABLESGIPROC __glewGetColorTableSGI = NULL;

PFNGLFINISHTEXTURESUNXPROC __glewFinishTextureSUNX = NULL;

PFNGLGLOBALALPHAFACTORBSUNPROC __glewGlobalAlphaFactorbSUN = NULL;
PFNGLGLOBALALPHAFACTORDSUNPROC __glewGlobalAlphaFactordSUN = NULL;
PFNGLGLOBALALPHAFACTORFSUNPROC __glewGlobalAlphaFactorfSUN = NULL;
PFNGLGLOBALALPHAFACTORISUNPROC __glewGlobalAlphaFactoriSUN = NULL;
PFNGLGLOBALALPHAFACTORSSUNPROC __glewGlobalAlphaFactorsSUN = NULL;
PFNGLGLOBALALPHAFACTORUBSUNPROC __glewGlobalAlphaFactorubSUN = NULL;
PFNGLGLOBALALPHAFACTORUISUNPROC __glewGlobalAlphaFactoruiSUN = NULL;
PFNGLGLOBALALPHAFACTORUSSUNPROC __glewGlobalAlphaFactorusSUN = NULL;

PFNGLREADVIDEOPIXELSSUNPROC __glewReadVideoPixelsSUN = NULL;

PFNGLREPLACEMENTCODEPOINTERSUNPROC __glewReplacementCodePointerSUN = NULL;
PFNGLREPLACEMENTCODEUBSUNPROC __glewReplacementCodeubSUN = NULL;
PFNGLREPLACEMENTCODEUBVSUNPROC __glewReplacementCodeubvSUN = NULL;
PFNGLREPLACEMENTCODEUISUNPROC __glewReplacementCodeuiSUN = NULL;
PFNGLREPLACEMENTCODEUIVSUNPROC __glewReplacementCodeuivSUN = NULL;
PFNGLREPLACEMENTCODEUSSUNPROC __glewReplacementCodeusSUN = NULL;
PFNGLREPLACEMENTCODEUSVSUNPROC __glewReplacementCodeusvSUN = NULL;

PFNGLCOLOR3FVERTEX3FSUNPROC __glewColor3fVertex3fSUN = NULL;
PFNGLCOLOR3FVERTEX3FVSUNPROC __glewColor3fVertex3fvSUN = NULL;
PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewColor4fNormal3fVertex3fSUN = NULL;
PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewColor4fNormal3fVertex3fvSUN = NULL;
PFNGLCOLOR4UBVERTEX2FSUNPROC __glewColor4ubVertex2fSUN = NULL;
PFNGLCOLOR4UBVERTEX2FVSUNPROC __glewColor4ubVertex2fvSUN = NULL;
PFNGLCOLOR4UBVERTEX3FSUNPROC __glewColor4ubVertex3fSUN = NULL;
PFNGLCOLOR4UBVERTEX3FVSUNPROC __glewColor4ubVertex3fvSUN = NULL;
PFNGLNORMAL3FVERTEX3FSUNPROC __glewNormal3fVertex3fSUN = NULL;
PFNGLNORMAL3FVERTEX3FVSUNPROC __glewNormal3fVertex3fvSUN = NULL;
PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC __glewReplacementCodeuiColor3fVertex3fSUN = NULL;
PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC __glewReplacementCodeuiColor3fVertex3fvSUN = NULL;
PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiColor4fNormal3fVertex3fSUN = NULL;
PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiColor4fNormal3fVertex3fvSUN = NULL;
PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC __glewReplacementCodeuiColor4ubVertex3fSUN = NULL;
PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC __glewReplacementCodeuiColor4ubVertex3fvSUN = NULL;
PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiNormal3fVertex3fSUN = NULL;
PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiNormal3fVertex3fvSUN = NULL;
PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN = NULL;
PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN = NULL;
PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fNormal3fVertex3fSUN = NULL;
PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN = NULL;
PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fVertex3fSUN = NULL;
PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fVertex3fvSUN = NULL;
PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC __glewReplacementCodeuiVertex3fSUN = NULL;
PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC __glewReplacementCodeuiVertex3fvSUN = NULL;
PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC __glewTexCoord2fColor3fVertex3fSUN = NULL;
PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC __glewTexCoord2fColor3fVertex3fvSUN = NULL;
PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewTexCoord2fColor4fNormal3fVertex3fSUN = NULL;
PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewTexCoord2fColor4fNormal3fVertex3fvSUN = NULL;
PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC __glewTexCoord2fColor4ubVertex3fSUN = NULL;
PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC __glewTexCoord2fColor4ubVertex3fvSUN = NULL;
PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC __glewTexCoord2fNormal3fVertex3fSUN = NULL;
PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC __glewTexCoord2fNormal3fVertex3fvSUN = NULL;
PFNGLTEXCOORD2FVERTEX3FSUNPROC __glewTexCoord2fVertex3fSUN = NULL;
PFNGLTEXCOORD2FVERTEX3FVSUNPROC __glewTexCoord2fVertex3fvSUN = NULL;
PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC __glewTexCoord4fColor4fNormal3fVertex4fSUN = NULL;
PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC __glewTexCoord4fColor4fNormal3fVertex4fvSUN = NULL;
PFNGLTEXCOORD4FVERTEX4FSUNPROC __glewTexCoord4fVertex4fSUN = NULL;
PFNGLTEXCOORD4FVERTEX4FVSUNPROC __glewTexCoord4fVertex4fvSUN = NULL;

PFNGLADDSWAPHINTRECTWINPROC __glewAddSwapHintRectWIN = NULL;

#endif /* !WIN32 || !GLEW_MX */

#if !defined(GLEW_MX)
GLboolean __GLEW_VERSION_1_1 = GL_FALSE;
GLboolean __GLEW_VERSION_1_2 = GL_FALSE;
GLboolean __GLEW_VERSION_1_3 = GL_FALSE;
GLboolean __GLEW_VERSION_1_4 = GL_FALSE;
GLboolean __GLEW_VERSION_1_5 = GL_FALSE;
GLboolean __GLEW_VERSION_2_0 = GL_FALSE;
GLboolean __GLEW_VERSION_2_1 = GL_FALSE;
GLboolean __GLEW_VERSION_3_0 = GL_FALSE;
GLboolean __GLEW_3DFX_multisample = GL_FALSE;
GLboolean __GLEW_3DFX_tbuffer = GL_FALSE;
GLboolean __GLEW_3DFX_texture_compression_FXT1 = GL_FALSE;
GLboolean __GLEW_APPLE_client_storage = GL_FALSE;
GLboolean __GLEW_APPLE_element_array = GL_FALSE;
GLboolean __GLEW_APPLE_fence = GL_FALSE;
GLboolean __GLEW_APPLE_float_pixels = GL_FALSE;
GLboolean __GLEW_APPLE_flush_buffer_range = GL_FALSE;
GLboolean __GLEW_APPLE_pixel_buffer = GL_FALSE;
GLboolean __GLEW_APPLE_specular_vector = GL_FALSE;
GLboolean __GLEW_APPLE_texture_range = GL_FALSE;
GLboolean __GLEW_APPLE_transform_hint = GL_FALSE;
GLboolean __GLEW_APPLE_vertex_array_object = GL_FALSE;
GLboolean __GLEW_APPLE_vertex_array_range = GL_FALSE;
GLboolean __GLEW_APPLE_ycbcr_422 = GL_FALSE;
GLboolean __GLEW_ARB_color_buffer_float = GL_FALSE;
GLboolean __GLEW_ARB_depth_buffer_float = GL_FALSE;
GLboolean __GLEW_ARB_depth_texture = GL_FALSE;
GLboolean __GLEW_ARB_draw_buffers = GL_FALSE;
GLboolean __GLEW_ARB_draw_instanced = GL_FALSE;
GLboolean __GLEW_ARB_fragment_program = GL_FALSE;
GLboolean __GLEW_ARB_fragment_program_shadow = GL_FALSE;
GLboolean __GLEW_ARB_fragment_shader = GL_FALSE;
GLboolean __GLEW_ARB_framebuffer_object = GL_FALSE;
GLboolean __GLEW_ARB_framebuffer_sRGB = GL_FALSE;
GLboolean __GLEW_ARB_geometry_shader4 = GL_FALSE;
GLboolean __GLEW_ARB_half_float_pixel = GL_FALSE;
GLboolean __GLEW_ARB_half_float_vertex = GL_FALSE;
GLboolean __GLEW_ARB_imaging = GL_FALSE;
GLboolean __GLEW_ARB_instanced_arrays = GL_FALSE;
GLboolean __GLEW_ARB_map_buffer_range = GL_FALSE;
GLboolean __GLEW_ARB_matrix_palette = GL_FALSE;
GLboolean __GLEW_ARB_multisample = GL_FALSE;
GLboolean __GLEW_ARB_multitexture = GL_FALSE;
GLboolean __GLEW_ARB_occlusion_query = GL_FALSE;
GLboolean __GLEW_ARB_pixel_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_point_parameters = GL_FALSE;
GLboolean __GLEW_ARB_point_sprite = GL_FALSE;
GLboolean __GLEW_ARB_shader_objects = GL_FALSE;
GLboolean __GLEW_ARB_shading_language_100 = GL_FALSE;
GLboolean __GLEW_ARB_shadow = GL_FALSE;
GLboolean __GLEW_ARB_shadow_ambient = GL_FALSE;
GLboolean __GLEW_ARB_texture_border_clamp = GL_FALSE;
GLboolean __GLEW_ARB_texture_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_texture_compression = GL_FALSE;
GLboolean __GLEW_ARB_texture_compression_rgtc = GL_FALSE;
GLboolean __GLEW_ARB_texture_cube_map = GL_FALSE;
GLboolean __GLEW_ARB_texture_env_add = GL_FALSE;
GLboolean __GLEW_ARB_texture_env_combine = GL_FALSE;
GLboolean __GLEW_ARB_texture_env_crossbar = GL_FALSE;
GLboolean __GLEW_ARB_texture_env_dot3 = GL_FALSE;
GLboolean __GLEW_ARB_texture_float = GL_FALSE;
GLboolean __GLEW_ARB_texture_mirrored_repeat = GL_FALSE;
GLboolean __GLEW_ARB_texture_non_power_of_two = GL_FALSE;
GLboolean __GLEW_ARB_texture_rectangle = GL_FALSE;
GLboolean __GLEW_ARB_texture_rg = GL_FALSE;
GLboolean __GLEW_ARB_transpose_matrix = GL_FALSE;
GLboolean __GLEW_ARB_vertex_array_object = GL_FALSE;
GLboolean __GLEW_ARB_vertex_blend = GL_FALSE;
GLboolean __GLEW_ARB_vertex_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_vertex_program = GL_FALSE;
GLboolean __GLEW_ARB_vertex_shader = GL_FALSE;
GLboolean __GLEW_ARB_window_pos = GL_FALSE;
GLboolean __GLEW_ATIX_point_sprites = GL_FALSE;
GLboolean __GLEW_ATIX_texture_env_combine3 = GL_FALSE;
GLboolean __GLEW_ATIX_texture_env_route = GL_FALSE;
GLboolean __GLEW_ATIX_vertex_shader_output_point_size = GL_FALSE;
GLboolean __GLEW_ATI_draw_buffers = GL_FALSE;
GLboolean __GLEW_ATI_element_array = GL_FALSE;
GLboolean __GLEW_ATI_envmap_bumpmap = GL_FALSE;
GLboolean __GLEW_ATI_fragment_shader = GL_FALSE;
GLboolean __GLEW_ATI_map_object_buffer = GL_FALSE;
GLboolean __GLEW_ATI_pn_triangles = GL_FALSE;
GLboolean __GLEW_ATI_separate_stencil = GL_FALSE;
GLboolean __GLEW_ATI_shader_texture_lod = GL_FALSE;
GLboolean __GLEW_ATI_text_fragment_shader = GL_FALSE;
GLboolean __GLEW_ATI_texture_compression_3dc = GL_FALSE;
GLboolean __GLEW_ATI_texture_float = GL_FALSE;
GLboolean __GLEW_ATI_texture_mirror_once = GL_FALSE;
GLboolean __GLEW_ATI_vertex_array_object = GL_FALSE;
GLboolean __GLEW_ATI_vertex_attrib_array_object = GL_FALSE;
GLboolean __GLEW_ATI_vertex_streams = GL_FALSE;
GLboolean __GLEW_EXT_422_pixels = GL_FALSE;
GLboolean __GLEW_EXT_Cg_shader = GL_FALSE;
GLboolean __GLEW_EXT_abgr = GL_FALSE;
GLboolean __GLEW_EXT_bgra = GL_FALSE;
GLboolean __GLEW_EXT_bindable_uniform = GL_FALSE;
GLboolean __GLEW_EXT_blend_color = GL_FALSE;
GLboolean __GLEW_EXT_blend_equation_separate = GL_FALSE;
GLboolean __GLEW_EXT_blend_func_separate = GL_FALSE;
GLboolean __GLEW_EXT_blend_logic_op = GL_FALSE;
GLboolean __GLEW_EXT_blend_minmax = GL_FALSE;
GLboolean __GLEW_EXT_blend_subtract = GL_FALSE;
GLboolean __GLEW_EXT_clip_volume_hint = GL_FALSE;
GLboolean __GLEW_EXT_cmyka = GL_FALSE;
GLboolean __GLEW_EXT_color_subtable = GL_FALSE;
GLboolean __GLEW_EXT_compiled_vertex_array = GL_FALSE;
GLboolean __GLEW_EXT_convolution = GL_FALSE;
GLboolean __GLEW_EXT_coordinate_frame = GL_FALSE;
GLboolean __GLEW_EXT_copy_texture = GL_FALSE;
GLboolean __GLEW_EXT_cull_vertex = GL_FALSE;
GLboolean __GLEW_EXT_depth_bounds_test = GL_FALSE;
GLboolean __GLEW_EXT_direct_state_access = GL_FALSE;
GLboolean __GLEW_EXT_draw_buffers2 = GL_FALSE;
GLboolean __GLEW_EXT_draw_instanced = GL_FALSE;
GLboolean __GLEW_EXT_draw_range_elements = GL_FALSE;
GLboolean __GLEW_EXT_fog_coord = GL_FALSE;
GLboolean __GLEW_EXT_fragment_lighting = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_blit = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_multisample = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_object = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_sRGB = GL_FALSE;
GLboolean __GLEW_EXT_geometry_shader4 = GL_FALSE;
GLboolean __GLEW_EXT_gpu_program_parameters = GL_FALSE;
GLboolean __GLEW_EXT_gpu_shader4 = GL_FALSE;
GLboolean __GLEW_EXT_histogram = GL_FALSE;
GLboolean __GLEW_EXT_index_array_formats = GL_FALSE;
GLboolean __GLEW_EXT_index_func = GL_FALSE;
GLboolean __GLEW_EXT_index_material = GL_FALSE;
GLboolean __GLEW_EXT_index_texture = GL_FALSE;
GLboolean __GLEW_EXT_light_texture = GL_FALSE;
GLboolean __GLEW_EXT_misc_attribute = GL_FALSE;
GLboolean __GLEW_EXT_multi_draw_arrays = GL_FALSE;
GLboolean __GLEW_EXT_multisample = GL_FALSE;
GLboolean __GLEW_EXT_packed_depth_stencil = GL_FALSE;
GLboolean __GLEW_EXT_packed_float = GL_FALSE;
GLboolean __GLEW_EXT_packed_pixels = GL_FALSE;
GLboolean __GLEW_EXT_paletted_texture = GL_FALSE;
GLboolean __GLEW_EXT_pixel_buffer_object = GL_FALSE;
GLboolean __GLEW_EXT_pixel_transform = GL_FALSE;
GLboolean __GLEW_EXT_pixel_transform_color_table = GL_FALSE;
GLboolean __GLEW_EXT_point_parameters = GL_FALSE;
GLboolean __GLEW_EXT_polygon_offset = GL_FALSE;
GLboolean __GLEW_EXT_rescale_normal = GL_FALSE;
GLboolean __GLEW_EXT_scene_marker = GL_FALSE;
GLboolean __GLEW_EXT_secondary_color = GL_FALSE;
GLboolean __GLEW_EXT_separate_specular_color = GL_FALSE;
GLboolean __GLEW_EXT_shadow_funcs = GL_FALSE;
GLboolean __GLEW_EXT_shared_texture_palette = GL_FALSE;
GLboolean __GLEW_EXT_stencil_clear_tag = GL_FALSE;
GLboolean __GLEW_EXT_stencil_two_side = GL_FALSE;
GLboolean __GLEW_EXT_stencil_wrap = GL_FALSE;
GLboolean __GLEW_EXT_subtexture = GL_FALSE;
GLboolean __GLEW_EXT_texture = GL_FALSE;
GLboolean __GLEW_EXT_texture3D = GL_FALSE;
GLboolean __GLEW_EXT_texture_array = GL_FALSE;
GLboolean __GLEW_EXT_texture_buffer_object = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_dxt1 = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_latc = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_rgtc = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_s3tc = GL_FALSE;
GLboolean __GLEW_EXT_texture_cube_map = GL_FALSE;
GLboolean __GLEW_EXT_texture_edge_clamp = GL_FALSE;
GLboolean __GLEW_EXT_texture_env = GL_FALSE;
GLboolean __GLEW_EXT_texture_env_add = GL_FALSE;
GLboolean __GLEW_EXT_texture_env_combine = GL_FALSE;
GLboolean __GLEW_EXT_texture_env_dot3 = GL_FALSE;
GLboolean __GLEW_EXT_texture_filter_anisotropic = GL_FALSE;
GLboolean __GLEW_EXT_texture_integer = GL_FALSE;
GLboolean __GLEW_EXT_texture_lod_bias = GL_FALSE;
GLboolean __GLEW_EXT_texture_mirror_clamp = GL_FALSE;
GLboolean __GLEW_EXT_texture_object = GL_FALSE;
GLboolean __GLEW_EXT_texture_perturb_normal = GL_FALSE;
GLboolean __GLEW_EXT_texture_rectangle = GL_FALSE;
GLboolean __GLEW_EXT_texture_sRGB = GL_FALSE;
GLboolean __GLEW_EXT_texture_shared_exponent = GL_FALSE;
GLboolean __GLEW_EXT_texture_swizzle = GL_FALSE;
GLboolean __GLEW_EXT_timer_query = GL_FALSE;
GLboolean __GLEW_EXT_transform_feedback = GL_FALSE;
GLboolean __GLEW_EXT_vertex_array = GL_FALSE;
GLboolean __GLEW_EXT_vertex_array_bgra = GL_FALSE;
GLboolean __GLEW_EXT_vertex_shader = GL_FALSE;
GLboolean __GLEW_EXT_vertex_weighting = GL_FALSE;
GLboolean __GLEW_GREMEDY_frame_terminator = GL_FALSE;
GLboolean __GLEW_GREMEDY_string_marker = GL_FALSE;
GLboolean __GLEW_HP_convolution_border_modes = GL_FALSE;
GLboolean __GLEW_HP_image_transform = GL_FALSE;
GLboolean __GLEW_HP_occlusion_test = GL_FALSE;
GLboolean __GLEW_HP_texture_lighting = GL_FALSE;
GLboolean __GLEW_IBM_cull_vertex = GL_FALSE;
GLboolean __GLEW_IBM_multimode_draw_arrays = GL_FALSE;
GLboolean __GLEW_IBM_rasterpos_clip = GL_FALSE;
GLboolean __GLEW_IBM_static_data = GL_FALSE;
GLboolean __GLEW_IBM_texture_mirrored_repeat = GL_FALSE;
GLboolean __GLEW_IBM_vertex_array_lists = GL_FALSE;
GLboolean __GLEW_INGR_color_clamp = GL_FALSE;
GLboolean __GLEW_INGR_interlace_read = GL_FALSE;
GLboolean __GLEW_INTEL_parallel_arrays = GL_FALSE;
GLboolean __GLEW_INTEL_texture_scissor = GL_FALSE;
GLboolean __GLEW_KTX_buffer_region = GL_FALSE;
GLboolean __GLEW_MESAX_texture_stack = GL_FALSE;
GLboolean __GLEW_MESA_pack_invert = GL_FALSE;
GLboolean __GLEW_MESA_resize_buffers = GL_FALSE;
GLboolean __GLEW_MESA_window_pos = GL_FALSE;
GLboolean __GLEW_MESA_ycbcr_texture = GL_FALSE;
GLboolean __GLEW_NV_blend_square = GL_FALSE;
GLboolean __GLEW_NV_conditional_render = GL_FALSE;
GLboolean __GLEW_NV_copy_depth_to_color = GL_FALSE;
GLboolean __GLEW_NV_depth_buffer_float = GL_FALSE;
GLboolean __GLEW_NV_depth_clamp = GL_FALSE;
GLboolean __GLEW_NV_depth_range_unclamped = GL_FALSE;
GLboolean __GLEW_NV_evaluators = GL_FALSE;
GLboolean __GLEW_NV_explicit_multisample = GL_FALSE;
GLboolean __GLEW_NV_fence = GL_FALSE;
GLboolean __GLEW_NV_float_buffer = GL_FALSE;
GLboolean __GLEW_NV_fog_distance = GL_FALSE;
GLboolean __GLEW_NV_fragment_program = GL_FALSE;
GLboolean __GLEW_NV_fragment_program2 = GL_FALSE;
GLboolean __GLEW_NV_fragment_program4 = GL_FALSE;
GLboolean __GLEW_NV_fragment_program_option = GL_FALSE;
GLboolean __GLEW_NV_framebuffer_multisample_coverage = GL_FALSE;
GLboolean __GLEW_NV_geometry_program4 = GL_FALSE;
GLboolean __GLEW_NV_geometry_shader4 = GL_FALSE;
GLboolean __GLEW_NV_gpu_program4 = GL_FALSE;
GLboolean __GLEW_NV_half_float = GL_FALSE;
GLboolean __GLEW_NV_light_max_exponent = GL_FALSE;
GLboolean __GLEW_NV_multisample_filter_hint = GL_FALSE;
GLboolean __GLEW_NV_occlusion_query = GL_FALSE;
GLboolean __GLEW_NV_packed_depth_stencil = GL_FALSE;
GLboolean __GLEW_NV_parameter_buffer_object = GL_FALSE;
GLboolean __GLEW_NV_pixel_data_range = GL_FALSE;
GLboolean __GLEW_NV_point_sprite = GL_FALSE;
GLboolean __GLEW_NV_present_video = GL_FALSE;
GLboolean __GLEW_NV_primitive_restart = GL_FALSE;
GLboolean __GLEW_NV_register_combiners = GL_FALSE;
GLboolean __GLEW_NV_register_combiners2 = GL_FALSE;
GLboolean __GLEW_NV_texgen_emboss = GL_FALSE;
GLboolean __GLEW_NV_texgen_reflection = GL_FALSE;
GLboolean __GLEW_NV_texture_compression_vtc = GL_FALSE;
GLboolean __GLEW_NV_texture_env_combine4 = GL_FALSE;
GLboolean __GLEW_NV_texture_expand_normal = GL_FALSE;
GLboolean __GLEW_NV_texture_rectangle = GL_FALSE;
GLboolean __GLEW_NV_texture_shader = GL_FALSE;
GLboolean __GLEW_NV_texture_shader2 = GL_FALSE;
GLboolean __GLEW_NV_texture_shader3 = GL_FALSE;
GLboolean __GLEW_NV_transform_feedback = GL_FALSE;
GLboolean __GLEW_NV_transform_feedback2 = GL_FALSE;
GLboolean __GLEW_NV_vertex_array_range = GL_FALSE;
GLboolean __GLEW_NV_vertex_array_range2 = GL_FALSE;
GLboolean __GLEW_NV_vertex_program = GL_FALSE;
GLboolean __GLEW_NV_vertex_program1_1 = GL_FALSE;
GLboolean __GLEW_NV_vertex_program2 = GL_FALSE;
GLboolean __GLEW_NV_vertex_program2_option = GL_FALSE;
GLboolean __GLEW_NV_vertex_program3 = GL_FALSE;
GLboolean __GLEW_NV_vertex_program4 = GL_FALSE;
GLboolean __GLEW_OES_byte_coordinates = GL_FALSE;
GLboolean __GLEW_OES_compressed_paletted_texture = GL_FALSE;
GLboolean __GLEW_OES_read_format = GL_FALSE;
GLboolean __GLEW_OES_single_precision = GL_FALSE;
GLboolean __GLEW_OML_interlace = GL_FALSE;
GLboolean __GLEW_OML_resample = GL_FALSE;
GLboolean __GLEW_OML_subsample = GL_FALSE;
GLboolean __GLEW_PGI_misc_hints = GL_FALSE;
GLboolean __GLEW_PGI_vertex_hints = GL_FALSE;
GLboolean __GLEW_REND_screen_coordinates = GL_FALSE;
GLboolean __GLEW_S3_s3tc = GL_FALSE;
GLboolean __GLEW_SGIS_color_range = GL_FALSE;
GLboolean __GLEW_SGIS_detail_texture = GL_FALSE;
GLboolean __GLEW_SGIS_fog_function = GL_FALSE;
GLboolean __GLEW_SGIS_generate_mipmap = GL_FALSE;
GLboolean __GLEW_SGIS_multisample = GL_FALSE;
GLboolean __GLEW_SGIS_pixel_texture = GL_FALSE;
GLboolean __GLEW_SGIS_point_line_texgen = GL_FALSE;
GLboolean __GLEW_SGIS_sharpen_texture = GL_FALSE;
GLboolean __GLEW_SGIS_texture4D = GL_FALSE;
GLboolean __GLEW_SGIS_texture_border_clamp = GL_FALSE;
GLboolean __GLEW_SGIS_texture_edge_clamp = GL_FALSE;
GLboolean __GLEW_SGIS_texture_filter4 = GL_FALSE;
GLboolean __GLEW_SGIS_texture_lod = GL_FALSE;
GLboolean __GLEW_SGIS_texture_select = GL_FALSE;
GLboolean __GLEW_SGIX_async = GL_FALSE;
GLboolean __GLEW_SGIX_async_histogram = GL_FALSE;
GLboolean __GLEW_SGIX_async_pixel = GL_FALSE;
GLboolean __GLEW_SGIX_blend_alpha_minmax = GL_FALSE;
GLboolean __GLEW_SGIX_clipmap = GL_FALSE;
GLboolean __GLEW_SGIX_convolution_accuracy = GL_FALSE;
GLboolean __GLEW_SGIX_depth_texture = GL_FALSE;
GLboolean __GLEW_SGIX_flush_raster = GL_FALSE;
GLboolean __GLEW_SGIX_fog_offset = GL_FALSE;
GLboolean __GLEW_SGIX_fog_texture = GL_FALSE;
GLboolean __GLEW_SGIX_fragment_specular_lighting = GL_FALSE;
GLboolean __GLEW_SGIX_framezoom = GL_FALSE;
GLboolean __GLEW_SGIX_interlace = GL_FALSE;
GLboolean __GLEW_SGIX_ir_instrument1 = GL_FALSE;
GLboolean __GLEW_SGIX_list_priority = GL_FALSE;
GLboolean __GLEW_SGIX_pixel_texture = GL_FALSE;
GLboolean __GLEW_SGIX_pixel_texture_bits = GL_FALSE;
GLboolean __GLEW_SGIX_reference_plane = GL_FALSE;
GLboolean __GLEW_SGIX_resample = GL_FALSE;
GLboolean __GLEW_SGIX_shadow = GL_FALSE;
GLboolean __GLEW_SGIX_shadow_ambient = GL_FALSE;
GLboolean __GLEW_SGIX_sprite = GL_FALSE;
GLboolean __GLEW_SGIX_tag_sample_buffer = GL_FALSE;
GLboolean __GLEW_SGIX_texture_add_env = GL_FALSE;
GLboolean __GLEW_SGIX_texture_coordinate_clamp = GL_FALSE;
GLboolean __GLEW_SGIX_texture_lod_bias = GL_FALSE;
GLboolean __GLEW_SGIX_texture_multi_buffer = GL_FALSE;
GLboolean __GLEW_SGIX_texture_range = GL_FALSE;
GLboolean __GLEW_SGIX_texture_scale_bias = GL_FALSE;
GLboolean __GLEW_SGIX_vertex_preclip = GL_FALSE;
GLboolean __GLEW_SGIX_vertex_preclip_hint = GL_FALSE;
GLboolean __GLEW_SGIX_ycrcb = GL_FALSE;
GLboolean __GLEW_SGI_color_matrix = GL_FALSE;
GLboolean __GLEW_SGI_color_table = GL_FALSE;
GLboolean __GLEW_SGI_texture_color_table = GL_FALSE;
GLboolean __GLEW_SUNX_constant_data = GL_FALSE;
GLboolean __GLEW_SUN_convolution_border_modes = GL_FALSE;
GLboolean __GLEW_SUN_global_alpha = GL_FALSE;
GLboolean __GLEW_SUN_mesh_array = GL_FALSE;
GLboolean __GLEW_SUN_read_video_pixels = GL_FALSE;
GLboolean __GLEW_SUN_slice_accum = GL_FALSE;
GLboolean __GLEW_SUN_triangle_list = GL_FALSE;
GLboolean __GLEW_SUN_vertex = GL_FALSE;
GLboolean __GLEW_WIN_phong_shading = GL_FALSE;
GLboolean __GLEW_WIN_specular_fog = GL_FALSE;
GLboolean __GLEW_WIN_swap_hint = GL_FALSE;

#endif /* !GLEW_MX */

#ifdef GL_VERSION_1_1

static GLboolean _glewInit_GL_VERSION_1_1 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAccum = (PFNGLACCUMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAccum"))) == NULL) || r;
  r = ((glAlphaFunc = (PFNGLALPHAFUNCPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAlphaFunc"))) == NULL) || r;
  r = ((glAreTexturesResident = (PFNGLARETEXTURESRESIDENTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAreTexturesResident"))) == NULL) || r;
  r = ((glArrayElement = (PFNGLARRAYELEMENTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glArrayElement"))) == NULL) || r;
  r = ((glBegin = (PFNGLBEGINPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBegin"))) == NULL) || r;
  r = ((glBindTexture = (PFNGLBINDTEXTUREPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindTexture"))) == NULL) || r;
  r = ((glBitmap = (PFNGLBITMAPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBitmap"))) == NULL) || r;
  r = ((glBlendFunc = (PFNGLBLENDFUNCPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendFunc"))) == NULL) || r;
  r = ((glCallList = (PFNGLCALLLISTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCallList"))) == NULL) || r;
  r = ((glCallLists = (PFNGLCALLLISTSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCallLists"))) == NULL) || r;
  r = ((glClear = (PFNGLCLEARPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClear"))) == NULL) || r;
  r = ((glClearAccum = (PFNGLCLEARACCUMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearAccum"))) == NULL) || r;
  r = ((glClearColor = (PFNGLCLEARCOLORPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearColor"))) == NULL) || r;
  r = ((glClearDepth = (PFNGLCLEARDEPTHPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearDepth"))) == NULL) || r;
  r = ((glClearIndex = (PFNGLCLEARINDEXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearIndex"))) == NULL) || r;
  r = ((glClearStencil = (PFNGLCLEARSTENCILPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearStencil"))) == NULL) || r;
  r = ((glClipPlane = (PFNGLCLIPPLANEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClipPlane"))) == NULL) || r;
  r = ((glColor3b = (PFNGLCOLOR3BPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3b"))) == NULL) || r;
  r = ((glColor3bv = (PFNGLCOLOR3BVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3bv"))) == NULL) || r;
  r = ((glColor3d = (PFNGLCOLOR3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3d"))) == NULL) || r;
  r = ((glColor3dv = (PFNGLCOLOR3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3dv"))) == NULL) || r;
  r = ((glColor3f = (PFNGLCOLOR3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3f"))) == NULL) || r;
  r = ((glColor3fv = (PFNGLCOLOR3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3fv"))) == NULL) || r;
  r = ((glColor3i = (PFNGLCOLOR3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3i"))) == NULL) || r;
  r = ((glColor3iv = (PFNGLCOLOR3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3iv"))) == NULL) || r;
  r = ((glColor3s = (PFNGLCOLOR3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3s"))) == NULL) || r;
  r = ((glColor3sv = (PFNGLCOLOR3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3sv"))) == NULL) || r;
  r = ((glColor3ub = (PFNGLCOLOR3UBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3ub"))) == NULL) || r;
  r = ((glColor3ubv = (PFNGLCOLOR3UBVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3ubv"))) == NULL) || r;
  r = ((glColor3ui = (PFNGLCOLOR3UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3ui"))) == NULL) || r;
  r = ((glColor3uiv = (PFNGLCOLOR3UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3uiv"))) == NULL) || r;
  r = ((glColor3us = (PFNGLCOLOR3USPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3us"))) == NULL) || r;
  r = ((glColor3usv = (PFNGLCOLOR3USVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3usv"))) == NULL) || r;
  r = ((glColor4b = (PFNGLCOLOR4BPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4b"))) == NULL) || r;
  r = ((glColor4bv = (PFNGLCOLOR4BVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4bv"))) == NULL) || r;
  r = ((glColor4d = (PFNGLCOLOR4DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4d"))) == NULL) || r;
  r = ((glColor4dv = (PFNGLCOLOR4DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4dv"))) == NULL) || r;
  r = ((glColor4f = (PFNGLCOLOR4FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4f"))) == NULL) || r;
  r = ((glColor4fv = (PFNGLCOLOR4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4fv"))) == NULL) || r;
  r = ((glColor4i = (PFNGLCOLOR4IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4i"))) == NULL) || r;
  r = ((glColor4iv = (PFNGLCOLOR4IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4iv"))) == NULL) || r;
  r = ((glColor4s = (PFNGLCOLOR4SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4s"))) == NULL) || r;
  r = ((glColor4sv = (PFNGLCOLOR4SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4sv"))) == NULL) || r;
  r = ((glColor4ub = (PFNGLCOLOR4UBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4ub"))) == NULL) || r;
  r = ((glColor4ubv = (PFNGLCOLOR4UBVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4ubv"))) == NULL) || r;
  r = ((glColor4ui = (PFNGLCOLOR4UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4ui"))) == NULL) || r;
  r = ((glColor4uiv = (PFNGLCOLOR4UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4uiv"))) == NULL) || r;
  r = ((glColor4us = (PFNGLCOLOR4USPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4us"))) == NULL) || r;
  r = ((glColor4usv = (PFNGLCOLOR4USVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4usv"))) == NULL) || r;
  r = ((glColorMask = (PFNGLCOLORMASKPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorMask"))) == NULL) || r;
  r = ((glColorMaterial = (PFNGLCOLORMATERIALPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorMaterial"))) == NULL) || r;
  r = ((glColorPointer = (PFNGLCOLORPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorPointer"))) == NULL) || r;
  r = ((glCopyPixels = (PFNGLCOPYPIXELSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyPixels"))) == NULL) || r;
  r = ((glCopyTexImage1D = (PFNGLCOPYTEXIMAGE1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexImage1D"))) == NULL) || r;
  r = ((glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexImage2D"))) == NULL) || r;
  r = ((glCopyTexSubImage1D = (PFNGLCOPYTEXSUBIMAGE1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexSubImage1D"))) == NULL) || r;
  r = ((glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexSubImage2D"))) == NULL) || r;
  r = ((glCullFace = (PFNGLCULLFACEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCullFace"))) == NULL) || r;
  r = ((glDeleteLists = (PFNGLDELETELISTSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteLists"))) == NULL) || r;
  r = ((glDeleteTextures = (PFNGLDELETETEXTURESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteTextures"))) == NULL) || r;
  r = ((glDepthFunc = (PFNGLDEPTHFUNCPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDepthFunc"))) == NULL) || r;
  r = ((glDepthMask = (PFNGLDEPTHMASKPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDepthMask"))) == NULL) || r;
  r = ((glDepthRange = (PFNGLDEPTHRANGEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDepthRange"))) == NULL) || r;
  r = ((glDisable = (PFNGLDISABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDisable"))) == NULL) || r;
  r = ((glDisableClientState = (PFNGLDISABLECLIENTSTATEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDisableClientState"))) == NULL) || r;
  r = ((glDrawArrays = (PFNGLDRAWARRAYSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawArrays"))) == NULL) || r;
  r = ((glDrawBuffer = (PFNGLDRAWBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawBuffer"))) == NULL) || r;
  r = ((glDrawElements = (PFNGLDRAWELEMENTSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawElements"))) == NULL) || r;
  r = ((glDrawPixels = (PFNGLDRAWPIXELSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawPixels"))) == NULL) || r;
  r = ((glEdgeFlag = (PFNGLEDGEFLAGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEdgeFlag"))) == NULL) || r;
  r = ((glEdgeFlagPointer = (PFNGLEDGEFLAGPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEdgeFlagPointer"))) == NULL) || r;
  r = ((glEdgeFlagv = (PFNGLEDGEFLAGVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEdgeFlagv"))) == NULL) || r;
  r = ((glEnable = (PFNGLENABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnable"))) == NULL) || r;
  r = ((glEnableClientState = (PFNGLENABLECLIENTSTATEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnableClientState"))) == NULL) || r;
  r = ((glEnd = (PFNGLENDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnd"))) == NULL) || r;
  r = ((glEndList = (PFNGLENDLISTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndList"))) == NULL) || r;
  r = ((glEvalCoord1d = (PFNGLEVALCOORD1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalCoord1d"))) == NULL) || r;
  r = ((glEvalCoord1dv = (PFNGLEVALCOORD1DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalCoord1dv"))) == NULL) || r;
  r = ((glEvalCoord1f = (PFNGLEVALCOORD1FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalCoord1f"))) == NULL) || r;
  r = ((glEvalCoord1fv = (PFNGLEVALCOORD1FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalCoord1fv"))) == NULL) || r;
  r = ((glEvalCoord2d = (PFNGLEVALCOORD2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalCoord2d"))) == NULL) || r;
  r = ((glEvalCoord2dv = (PFNGLEVALCOORD2DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalCoord2dv"))) == NULL) || r;
  r = ((glEvalCoord2f = (PFNGLEVALCOORD2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalCoord2f"))) == NULL) || r;
  r = ((glEvalCoord2fv = (PFNGLEVALCOORD2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalCoord2fv"))) == NULL) || r;
  r = ((glEvalMesh1 = (PFNGLEVALMESH1PROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalMesh1"))) == NULL) || r;
  r = ((glEvalMesh2 = (PFNGLEVALMESH2PROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalMesh2"))) == NULL) || r;
  r = ((glEvalPoint1 = (PFNGLEVALPOINT1PROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalPoint1"))) == NULL) || r;
  r = ((glEvalPoint2 = (PFNGLEVALPOINT2PROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalPoint2"))) == NULL) || r;
  r = ((glFeedbackBuffer = (PFNGLFEEDBACKBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFeedbackBuffer"))) == NULL) || r;
  r = ((glFinish = (PFNGLFINISHPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFinish"))) == NULL) || r;
  r = ((glFlush = (PFNGLFLUSHPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFlush"))) == NULL) || r;
  r = ((glFogf = (PFNGLFOGFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogf"))) == NULL) || r;
  r = ((glFogfv = (PFNGLFOGFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogfv"))) == NULL) || r;
  r = ((glFogi = (PFNGLFOGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogi"))) == NULL) || r;
  r = ((glFogiv = (PFNGLFOGIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogiv"))) == NULL) || r;
  r = ((glFrontFace = (PFNGLFRONTFACEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFrontFace"))) == NULL) || r;
  r = ((glFrustum = (PFNGLFRUSTUMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFrustum"))) == NULL) || r;
  r = ((glGenLists = (PFNGLGENLISTSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenLists"))) == NULL) || r;
  r = ((glGenTextures = (PFNGLGENTEXTURESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenTextures"))) == NULL) || r;
  r = ((glGetBooleanv = (PFNGLGETBOOLEANVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBooleanv"))) == NULL) || r;
  r = ((glGetClipPlane = (PFNGLGETCLIPPLANEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetClipPlane"))) == NULL) || r;
  r = ((glGetDoublev = (PFNGLGETDOUBLEVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetDoublev"))) == NULL) || r;
  r = ((glGetError = (PFNGLGETERRORPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetError"))) == NULL) || r;
  r = ((glGetFloatv = (PFNGLGETFLOATVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFloatv"))) == NULL) || r;
  r = ((glGetIntegerv = (PFNGLGETINTEGERVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetIntegerv"))) == NULL) || r;
  r = ((glGetLightfv = (PFNGLGETLIGHTFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetLightfv"))) == NULL) || r;
  r = ((glGetLightiv = (PFNGLGETLIGHTIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetLightiv"))) == NULL) || r;
  r = ((glGetMapdv = (PFNGLGETMAPDVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMapdv"))) == NULL) || r;
  r = ((glGetMapfv = (PFNGLGETMAPFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMapfv"))) == NULL) || r;
  r = ((glGetMapiv = (PFNGLGETMAPIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMapiv"))) == NULL) || r;
  r = ((glGetMaterialfv = (PFNGLGETMATERIALFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMaterialfv"))) == NULL) || r;
  r = ((glGetMaterialiv = (PFNGLGETMATERIALIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMaterialiv"))) == NULL) || r;
  r = ((glGetPixelMapfv = (PFNGLGETPIXELMAPFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPixelMapfv"))) == NULL) || r;
  r = ((glGetPixelMapuiv = (PFNGLGETPIXELMAPUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPixelMapuiv"))) == NULL) || r;
  r = ((glGetPixelMapusv = (PFNGLGETPIXELMAPUSVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPixelMapusv"))) == NULL) || r;
  r = ((glGetPointerv = (PFNGLGETPOINTERVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPointerv"))) == NULL) || r;
  r = ((glGetPolygonStipple = (PFNGLGETPOLYGONSTIPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPolygonStipple"))) == NULL) || r;
  r = ((glGetString = (PFNGLGETSTRINGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetString"))) == NULL) || r;
  r = ((glGetTexEnvfv = (PFNGLGETTEXENVFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexEnvfv"))) == NULL) || r;
  r = ((glGetTexEnviv = (PFNGLGETTEXENVIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexEnviv"))) == NULL) || r;
  r = ((glGetTexGendv = (PFNGLGETTEXGENDVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexGendv"))) == NULL) || r;
  r = ((glGetTexGenfv = (PFNGLGETTEXGENFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexGenfv"))) == NULL) || r;
  r = ((glGetTexGeniv = (PFNGLGETTEXGENIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexGeniv"))) == NULL) || r;
  r = ((glGetTexImage = (PFNGLGETTEXIMAGEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexImage"))) == NULL) || r;
  r = ((glGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexLevelParameterfv"))) == NULL) || r;
  r = ((glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexLevelParameteriv"))) == NULL) || r;
  r = ((glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexParameterfv"))) == NULL) || r;
  r = ((glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexParameteriv"))) == NULL) || r;
  r = ((glHint = (PFNGLHINTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glHint"))) == NULL) || r;
  r = ((glIndexMask = (PFNGLINDEXMASKPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexMask"))) == NULL) || r;
  r = ((glIndexPointer = (PFNGLINDEXPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexPointer"))) == NULL) || r;
  r = ((glIndexd = (PFNGLINDEXDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexd"))) == NULL) || r;
  r = ((glIndexdv = (PFNGLINDEXDVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexdv"))) == NULL) || r;
  r = ((glIndexf = (PFNGLINDEXFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexf"))) == NULL) || r;
  r = ((glIndexfv = (PFNGLINDEXFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexfv"))) == NULL) || r;
  r = ((glIndexi = (PFNGLINDEXIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexi"))) == NULL) || r;
  r = ((glIndexiv = (PFNGLINDEXIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexiv"))) == NULL) || r;
  r = ((glIndexs = (PFNGLINDEXSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexs"))) == NULL) || r;
  r = ((glIndexsv = (PFNGLINDEXSVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexsv"))) == NULL) || r;
  r = ((glIndexub = (PFNGLINDEXUBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexub"))) == NULL) || r;
  r = ((glIndexubv = (PFNGLINDEXUBVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexubv"))) == NULL) || r;
  r = ((glInitNames = (PFNGLINITNAMESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glInitNames"))) == NULL) || r;
  r = ((glInterleavedArrays = (PFNGLINTERLEAVEDARRAYSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glInterleavedArrays"))) == NULL) || r;
  r = ((glIsEnabled = (PFNGLISENABLEDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsEnabled"))) == NULL) || r;
  r = ((glIsList = (PFNGLISLISTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsList"))) == NULL) || r;
  r = ((glIsTexture = (PFNGLISTEXTUREPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsTexture"))) == NULL) || r;
  r = ((glLightModelf = (PFNGLLIGHTMODELFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLightModelf"))) == NULL) || r;
  r = ((glLightModelfv = (PFNGLLIGHTMODELFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLightModelfv"))) == NULL) || r;
  r = ((glLightModeli = (PFNGLLIGHTMODELIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLightModeli"))) == NULL) || r;
  r = ((glLightModeliv = (PFNGLLIGHTMODELIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLightModeliv"))) == NULL) || r;
  r = ((glLightf = (PFNGLLIGHTFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLightf"))) == NULL) || r;
  r = ((glLightfv = (PFNGLLIGHTFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLightfv"))) == NULL) || r;
  r = ((glLighti = (PFNGLLIGHTIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLighti"))) == NULL) || r;
  r = ((glLightiv = (PFNGLLIGHTIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLightiv"))) == NULL) || r;
  r = ((glLineStipple = (PFNGLLINESTIPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLineStipple"))) == NULL) || r;
  r = ((glLineWidth = (PFNGLLINEWIDTHPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLineWidth"))) == NULL) || r;
  r = ((glListBase = (PFNGLLISTBASEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glListBase"))) == NULL) || r;
  r = ((glLoadIdentity = (PFNGLLOADIDENTITYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadIdentity"))) == NULL) || r;
  r = ((glLoadMatrixd = (PFNGLLOADMATRIXDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadMatrixd"))) == NULL) || r;
  r = ((glLoadMatrixf = (PFNGLLOADMATRIXFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadMatrixf"))) == NULL) || r;
  r = ((glLoadName = (PFNGLLOADNAMEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadName"))) == NULL) || r;
  r = ((glLogicOp = (PFNGLLOGICOPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLogicOp"))) == NULL) || r;
  r = ((glMap1d = (PFNGLMAP1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMap1d"))) == NULL) || r;
  r = ((glMap1f = (PFNGLMAP1FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMap1f"))) == NULL) || r;
  r = ((glMap2d = (PFNGLMAP2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMap2d"))) == NULL) || r;
  r = ((glMap2f = (PFNGLMAP2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMap2f"))) == NULL) || r;
  r = ((glMapGrid1d = (PFNGLMAPGRID1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapGrid1d"))) == NULL) || r;
  r = ((glMapGrid1f = (PFNGLMAPGRID1FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapGrid1f"))) == NULL) || r;
  r = ((glMapGrid2d = (PFNGLMAPGRID2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapGrid2d"))) == NULL) || r;
  r = ((glMapGrid2f = (PFNGLMAPGRID2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapGrid2f"))) == NULL) || r;
  r = ((glMaterialf = (PFNGLMATERIALFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMaterialf"))) == NULL) || r;
  r = ((glMaterialfv = (PFNGLMATERIALFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMaterialfv"))) == NULL) || r;
  r = ((glMateriali = (PFNGLMATERIALIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMateriali"))) == NULL) || r;
  r = ((glMaterialiv = (PFNGLMATERIALIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMaterialiv"))) == NULL) || r;
  r = ((glMatrixMode = (PFNGLMATRIXMODEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixMode"))) == NULL) || r;
  r = ((glMultMatrixd = (PFNGLMULTMATRIXDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultMatrixd"))) == NULL) || r;
  r = ((glMultMatrixf = (PFNGLMULTMATRIXFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultMatrixf"))) == NULL) || r;
  r = ((glNewList = (PFNGLNEWLISTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNewList"))) == NULL) || r;
  r = ((glNormal3b = (PFNGLNORMAL3BPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3b"))) == NULL) || r;
  r = ((glNormal3bv = (PFNGLNORMAL3BVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3bv"))) == NULL) || r;
  r = ((glNormal3d = (PFNGLNORMAL3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3d"))) == NULL) || r;
  r = ((glNormal3dv = (PFNGLNORMAL3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3dv"))) == NULL) || r;
  r = ((glNormal3f = (PFNGLNORMAL3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3f"))) == NULL) || r;
  r = ((glNormal3fv = (PFNGLNORMAL3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3fv"))) == NULL) || r;
  r = ((glNormal3i = (PFNGLNORMAL3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3i"))) == NULL) || r;
  r = ((glNormal3iv = (PFNGLNORMAL3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3iv"))) == NULL) || r;
  r = ((glNormal3s = (PFNGLNORMAL3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3s"))) == NULL) || r;
  r = ((glNormal3sv = (PFNGLNORMAL3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3sv"))) == NULL) || r;
  r = ((glNormalPointer = (PFNGLNORMALPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalPointer"))) == NULL) || r;
  r = ((glOrtho = (PFNGLORTHOPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glOrtho"))) == NULL) || r;
  r = ((glPassThrough = (PFNGLPASSTHROUGHPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPassThrough"))) == NULL) || r;
  r = ((glPixelMapfv = (PFNGLPIXELMAPFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelMapfv"))) == NULL) || r;
  r = ((glPixelMapuiv = (PFNGLPIXELMAPUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelMapuiv"))) == NULL) || r;
  r = ((glPixelMapusv = (PFNGLPIXELMAPUSVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelMapusv"))) == NULL) || r;
  r = ((glPixelStoref = (PFNGLPIXELSTOREFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelStoref"))) == NULL) || r;
  r = ((glPixelStorei = (PFNGLPIXELSTOREIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelStorei"))) == NULL) || r;
  r = ((glPixelTransferf = (PFNGLPIXELTRANSFERFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelTransferf"))) == NULL) || r;
  r = ((glPixelTransferi = (PFNGLPIXELTRANSFERIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelTransferi"))) == NULL) || r;
  r = ((glPixelZoom = (PFNGLPIXELZOOMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelZoom"))) == NULL) || r;
  r = ((glPointSize = (PFNGLPOINTSIZEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointSize"))) == NULL) || r;
  r = ((glPolygonMode = (PFNGLPOLYGONMODEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPolygonMode"))) == NULL) || r;
  r = ((glPolygonOffset = (PFNGLPOLYGONOFFSETPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPolygonOffset"))) == NULL) || r;
  r = ((glPolygonStipple = (PFNGLPOLYGONSTIPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPolygonStipple"))) == NULL) || r;
  r = ((glPopAttrib = (PFNGLPOPATTRIBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPopAttrib"))) == NULL) || r;
  r = ((glPopClientAttrib = (PFNGLPOPCLIENTATTRIBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPopClientAttrib"))) == NULL) || r;
  r = ((glPopMatrix = (PFNGLPOPMATRIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPopMatrix"))) == NULL) || r;
  r = ((glPopName = (PFNGLPOPNAMEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPopName"))) == NULL) || r;
  r = ((glPrioritizeTextures = (PFNGLPRIORITIZETEXTURESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPrioritizeTextures"))) == NULL) || r;
  r = ((glPushAttrib = (PFNGLPUSHATTRIBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPushAttrib"))) == NULL) || r;
  r = ((glPushClientAttrib = (PFNGLPUSHCLIENTATTRIBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPushClientAttrib"))) == NULL) || r;
  r = ((glPushMatrix = (PFNGLPUSHMATRIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPushMatrix"))) == NULL) || r;
  r = ((glPushName = (PFNGLPUSHNAMEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPushName"))) == NULL) || r;
  r = ((glRasterPos2d = (PFNGLRASTERPOS2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos2d"))) == NULL) || r;
  r = ((glRasterPos2dv = (PFNGLRASTERPOS2DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos2dv"))) == NULL) || r;
  r = ((glRasterPos2f = (PFNGLRASTERPOS2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos2f"))) == NULL) || r;
  r = ((glRasterPos2fv = (PFNGLRASTERPOS2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos2fv"))) == NULL) || r;
  r = ((glRasterPos2i = (PFNGLRASTERPOS2IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos2i"))) == NULL) || r;
  r = ((glRasterPos2iv = (PFNGLRASTERPOS2IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos2iv"))) == NULL) || r;
  r = ((glRasterPos2s = (PFNGLRASTERPOS2SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos2s"))) == NULL) || r;
  r = ((glRasterPos2sv = (PFNGLRASTERPOS2SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos2sv"))) == NULL) || r;
  r = ((glRasterPos3d = (PFNGLRASTERPOS3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos3d"))) == NULL) || r;
  r = ((glRasterPos3dv = (PFNGLRASTERPOS3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos3dv"))) == NULL) || r;
  r = ((glRasterPos3f = (PFNGLRASTERPOS3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos3f"))) == NULL) || r;
  r = ((glRasterPos3fv = (PFNGLRASTERPOS3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos3fv"))) == NULL) || r;
  r = ((glRasterPos3i = (PFNGLRASTERPOS3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos3i"))) == NULL) || r;
  r = ((glRasterPos3iv = (PFNGLRASTERPOS3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos3iv"))) == NULL) || r;
  r = ((glRasterPos3s = (PFNGLRASTERPOS3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos3s"))) == NULL) || r;
  r = ((glRasterPos3sv = (PFNGLRASTERPOS3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos3sv"))) == NULL) || r;
  r = ((glRasterPos4d = (PFNGLRASTERPOS4DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos4d"))) == NULL) || r;
  r = ((glRasterPos4dv = (PFNGLRASTERPOS4DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos4dv"))) == NULL) || r;
  r = ((glRasterPos4f = (PFNGLRASTERPOS4FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos4f"))) == NULL) || r;
  r = ((glRasterPos4fv = (PFNGLRASTERPOS4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos4fv"))) == NULL) || r;
  r = ((glRasterPos4i = (PFNGLRASTERPOS4IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos4i"))) == NULL) || r;
  r = ((glRasterPos4iv = (PFNGLRASTERPOS4IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos4iv"))) == NULL) || r;
  r = ((glRasterPos4s = (PFNGLRASTERPOS4SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos4s"))) == NULL) || r;
  r = ((glRasterPos4sv = (PFNGLRASTERPOS4SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRasterPos4sv"))) == NULL) || r;
  r = ((glReadBuffer = (PFNGLREADBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReadBuffer"))) == NULL) || r;
  r = ((glReadPixels = (PFNGLREADPIXELSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReadPixels"))) == NULL) || r;
  r = ((glRectd = (PFNGLRECTDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRectd"))) == NULL) || r;
  r = ((glRectdv = (PFNGLRECTDVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRectdv"))) == NULL) || r;
  r = ((glRectf = (PFNGLRECTFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRectf"))) == NULL) || r;
  r = ((glRectfv = (PFNGLRECTFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRectfv"))) == NULL) || r;
  r = ((glRecti = (PFNGLRECTIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRecti"))) == NULL) || r;
  r = ((glRectiv = (PFNGLRECTIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRectiv"))) == NULL) || r;
  r = ((glRects = (PFNGLRECTSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRects"))) == NULL) || r;
  r = ((glRectsv = (PFNGLRECTSVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRectsv"))) == NULL) || r;
  r = ((glRenderMode = (PFNGLRENDERMODEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRenderMode"))) == NULL) || r;
  r = ((glRotated = (PFNGLROTATEDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRotated"))) == NULL) || r;
  r = ((glRotatef = (PFNGLROTATEFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRotatef"))) == NULL) || r;
  r = ((glScaled = (PFNGLSCALEDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glScaled"))) == NULL) || r;
  r = ((glScalef = (PFNGLSCALEFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glScalef"))) == NULL) || r;
  r = ((glScissor = (PFNGLSCISSORPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glScissor"))) == NULL) || r;
  r = ((glSelectBuffer = (PFNGLSELECTBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSelectBuffer"))) == NULL) || r;
  r = ((glShadeModel = (PFNGLSHADEMODELPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glShadeModel"))) == NULL) || r;
  r = ((glStencilFunc = (PFNGLSTENCILFUNCPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStencilFunc"))) == NULL) || r;
  r = ((glStencilMask = (PFNGLSTENCILMASKPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStencilMask"))) == NULL) || r;
  r = ((glStencilOp = (PFNGLSTENCILOPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStencilOp"))) == NULL) || r;
  r = ((glTexCoord1d = (PFNGLTEXCOORD1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1d"))) == NULL) || r;
  r = ((glTexCoord1dv = (PFNGLTEXCOORD1DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1dv"))) == NULL) || r;
  r = ((glTexCoord1f = (PFNGLTEXCOORD1FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1f"))) == NULL) || r;
  r = ((glTexCoord1fv = (PFNGLTEXCOORD1FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1fv"))) == NULL) || r;
  r = ((glTexCoord1i = (PFNGLTEXCOORD1IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1i"))) == NULL) || r;
  r = ((glTexCoord1iv = (PFNGLTEXCOORD1IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1iv"))) == NULL) || r;
  r = ((glTexCoord1s = (PFNGLTEXCOORD1SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1s"))) == NULL) || r;
  r = ((glTexCoord1sv = (PFNGLTEXCOORD1SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1sv"))) == NULL) || r;
  r = ((glTexCoord2d = (PFNGLTEXCOORD2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2d"))) == NULL) || r;
  r = ((glTexCoord2dv = (PFNGLTEXCOORD2DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2dv"))) == NULL) || r;
  r = ((glTexCoord2f = (PFNGLTEXCOORD2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2f"))) == NULL) || r;
  r = ((glTexCoord2fv = (PFNGLTEXCOORD2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fv"))) == NULL) || r;
  r = ((glTexCoord2i = (PFNGLTEXCOORD2IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2i"))) == NULL) || r;
  r = ((glTexCoord2iv = (PFNGLTEXCOORD2IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2iv"))) == NULL) || r;
  r = ((glTexCoord2s = (PFNGLTEXCOORD2SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2s"))) == NULL) || r;
  r = ((glTexCoord2sv = (PFNGLTEXCOORD2SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2sv"))) == NULL) || r;
  r = ((glTexCoord3d = (PFNGLTEXCOORD3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3d"))) == NULL) || r;
  r = ((glTexCoord3dv = (PFNGLTEXCOORD3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3dv"))) == NULL) || r;
  r = ((glTexCoord3f = (PFNGLTEXCOORD3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3f"))) == NULL) || r;
  r = ((glTexCoord3fv = (PFNGLTEXCOORD3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3fv"))) == NULL) || r;
  r = ((glTexCoord3i = (PFNGLTEXCOORD3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3i"))) == NULL) || r;
  r = ((glTexCoord3iv = (PFNGLTEXCOORD3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3iv"))) == NULL) || r;
  r = ((glTexCoord3s = (PFNGLTEXCOORD3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3s"))) == NULL) || r;
  r = ((glTexCoord3sv = (PFNGLTEXCOORD3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3sv"))) == NULL) || r;
  r = ((glTexCoord4d = (PFNGLTEXCOORD4DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4d"))) == NULL) || r;
  r = ((glTexCoord4dv = (PFNGLTEXCOORD4DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4dv"))) == NULL) || r;
  r = ((glTexCoord4f = (PFNGLTEXCOORD4FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4f"))) == NULL) || r;
  r = ((glTexCoord4fv = (PFNGLTEXCOORD4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4fv"))) == NULL) || r;
  r = ((glTexCoord4i = (PFNGLTEXCOORD4IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4i"))) == NULL) || r;
  r = ((glTexCoord4iv = (PFNGLTEXCOORD4IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4iv"))) == NULL) || r;
  r = ((glTexCoord4s = (PFNGLTEXCOORD4SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4s"))) == NULL) || r;
  r = ((glTexCoord4sv = (PFNGLTEXCOORD4SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4sv"))) == NULL) || r;
  r = ((glTexCoordPointer = (PFNGLTEXCOORDPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoordPointer"))) == NULL) || r;
  r = ((glTexEnvf = (PFNGLTEXENVFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexEnvf"))) == NULL) || r;
  r = ((glTexEnvfv = (PFNGLTEXENVFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexEnvfv"))) == NULL) || r;
  r = ((glTexEnvi = (PFNGLTEXENVIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexEnvi"))) == NULL) || r;
  r = ((glTexEnviv = (PFNGLTEXENVIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexEnviv"))) == NULL) || r;
  r = ((glTexGend = (PFNGLTEXGENDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexGend"))) == NULL) || r;
  r = ((glTexGendv = (PFNGLTEXGENDVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexGendv"))) == NULL) || r;
  r = ((glTexGenf = (PFNGLTEXGENFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexGenf"))) == NULL) || r;
  r = ((glTexGenfv = (PFNGLTEXGENFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexGenfv"))) == NULL) || r;
  r = ((glTexGeni = (PFNGLTEXGENIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexGeni"))) == NULL) || r;
  r = ((glTexGeniv = (PFNGLTEXGENIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexGeniv"))) == NULL) || r;
  r = ((glTexImage1D = (PFNGLTEXIMAGE1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexImage1D"))) == NULL) || r;
  r = ((glTexImage2D = (PFNGLTEXIMAGE2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexImage2D"))) == NULL) || r;
  r = ((glTexParameterf = (PFNGLTEXPARAMETERFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexParameterf"))) == NULL) || r;
  r = ((glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexParameterfv"))) == NULL) || r;
  r = ((glTexParameteri = (PFNGLTEXPARAMETERIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexParameteri"))) == NULL) || r;
  r = ((glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexParameteriv"))) == NULL) || r;
  r = ((glTexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexSubImage1D"))) == NULL) || r;
  r = ((glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexSubImage2D"))) == NULL) || r;
  r = ((glTranslated = (PFNGLTRANSLATEDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTranslated"))) == NULL) || r;
  r = ((glTranslatef = (PFNGLTRANSLATEFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTranslatef"))) == NULL) || r;
  r = ((glVertex2d = (PFNGLVERTEX2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2d"))) == NULL) || r;
  r = ((glVertex2dv = (PFNGLVERTEX2DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2dv"))) == NULL) || r;
  r = ((glVertex2f = (PFNGLVERTEX2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2f"))) == NULL) || r;
  r = ((glVertex2fv = (PFNGLVERTEX2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2fv"))) == NULL) || r;
  r = ((glVertex2i = (PFNGLVERTEX2IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2i"))) == NULL) || r;
  r = ((glVertex2iv = (PFNGLVERTEX2IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2iv"))) == NULL) || r;
  r = ((glVertex2s = (PFNGLVERTEX2SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2s"))) == NULL) || r;
  r = ((glVertex2sv = (PFNGLVERTEX2SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2sv"))) == NULL) || r;
  r = ((glVertex3d = (PFNGLVERTEX3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3d"))) == NULL) || r;
  r = ((glVertex3dv = (PFNGLVERTEX3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3dv"))) == NULL) || r;
  r = ((glVertex3f = (PFNGLVERTEX3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3f"))) == NULL) || r;
  r = ((glVertex3fv = (PFNGLVERTEX3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3fv"))) == NULL) || r;
  r = ((glVertex3i = (PFNGLVERTEX3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3i"))) == NULL) || r;
  r = ((glVertex3iv = (PFNGLVERTEX3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3iv"))) == NULL) || r;
  r = ((glVertex3s = (PFNGLVERTEX3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3s"))) == NULL) || r;
  r = ((glVertex3sv = (PFNGLVERTEX3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3sv"))) == NULL) || r;
  r = ((glVertex4d = (PFNGLVERTEX4DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4d"))) == NULL) || r;
  r = ((glVertex4dv = (PFNGLVERTEX4DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4dv"))) == NULL) || r;
  r = ((glVertex4f = (PFNGLVERTEX4FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4f"))) == NULL) || r;
  r = ((glVertex4fv = (PFNGLVERTEX4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4fv"))) == NULL) || r;
  r = ((glVertex4i = (PFNGLVERTEX4IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4i"))) == NULL) || r;
  r = ((glVertex4iv = (PFNGLVERTEX4IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4iv"))) == NULL) || r;
  r = ((glVertex4s = (PFNGLVERTEX4SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4s"))) == NULL) || r;
  r = ((glVertex4sv = (PFNGLVERTEX4SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4sv"))) == NULL) || r;
  r = ((glVertexPointer = (PFNGLVERTEXPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexPointer"))) == NULL) || r;
  r = ((glViewport = (PFNGLVIEWPORTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glViewport"))) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_1_1 */

#ifdef GL_VERSION_1_2

static GLboolean _glewInit_GL_VERSION_1_2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexSubImage3D"))) == NULL) || r;
  r = ((glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawRangeElements"))) == NULL) || r;
  r = ((glTexImage3D = (PFNGLTEXIMAGE3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexImage3D"))) == NULL) || r;
  r = ((glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexSubImage3D"))) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_1_2 */

#ifdef GL_VERSION_1_3

static GLboolean _glewInit_GL_VERSION_1_3 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveTexture = (PFNGLACTIVETEXTUREPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glActiveTexture"))) == NULL) || r;
  r = ((glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClientActiveTexture"))) == NULL) || r;
  r = ((glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexImage1D"))) == NULL) || r;
  r = ((glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexImage2D"))) == NULL) || r;
  r = ((glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexImage3D"))) == NULL) || r;
  r = ((glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexSubImage1D"))) == NULL) || r;
  r = ((glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexSubImage2D"))) == NULL) || r;
  r = ((glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexSubImage3D"))) == NULL) || r;
  r = ((glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCompressedTexImage"))) == NULL) || r;
  r = ((glLoadTransposeMatrixd = (PFNGLLOADTRANSPOSEMATRIXDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadTransposeMatrixd"))) == NULL) || r;
  r = ((glLoadTransposeMatrixf = (PFNGLLOADTRANSPOSEMATRIXFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadTransposeMatrixf"))) == NULL) || r;
  r = ((glMultTransposeMatrixd = (PFNGLMULTTRANSPOSEMATRIXDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultTransposeMatrixd"))) == NULL) || r;
  r = ((glMultTransposeMatrixf = (PFNGLMULTTRANSPOSEMATRIXFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultTransposeMatrixf"))) == NULL) || r;
  r = ((glMultiTexCoord1d = (PFNGLMULTITEXCOORD1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1d"))) == NULL) || r;
  r = ((glMultiTexCoord1dv = (PFNGLMULTITEXCOORD1DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1dv"))) == NULL) || r;
  r = ((glMultiTexCoord1f = (PFNGLMULTITEXCOORD1FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1f"))) == NULL) || r;
  r = ((glMultiTexCoord1fv = (PFNGLMULTITEXCOORD1FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1fv"))) == NULL) || r;
  r = ((glMultiTexCoord1i = (PFNGLMULTITEXCOORD1IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1i"))) == NULL) || r;
  r = ((glMultiTexCoord1iv = (PFNGLMULTITEXCOORD1IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1iv"))) == NULL) || r;
  r = ((glMultiTexCoord1s = (PFNGLMULTITEXCOORD1SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1s"))) == NULL) || r;
  r = ((glMultiTexCoord1sv = (PFNGLMULTITEXCOORD1SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1sv"))) == NULL) || r;
  r = ((glMultiTexCoord2d = (PFNGLMULTITEXCOORD2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2d"))) == NULL) || r;
  r = ((glMultiTexCoord2dv = (PFNGLMULTITEXCOORD2DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2dv"))) == NULL) || r;
  r = ((glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2f"))) == NULL) || r;
  r = ((glMultiTexCoord2fv = (PFNGLMULTITEXCOORD2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2fv"))) == NULL) || r;
  r = ((glMultiTexCoord2i = (PFNGLMULTITEXCOORD2IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2i"))) == NULL) || r;
  r = ((glMultiTexCoord2iv = (PFNGLMULTITEXCOORD2IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2iv"))) == NULL) || r;
  r = ((glMultiTexCoord2s = (PFNGLMULTITEXCOORD2SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2s"))) == NULL) || r;
  r = ((glMultiTexCoord2sv = (PFNGLMULTITEXCOORD2SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2sv"))) == NULL) || r;
  r = ((glMultiTexCoord3d = (PFNGLMULTITEXCOORD3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3d"))) == NULL) || r;
  r = ((glMultiTexCoord3dv = (PFNGLMULTITEXCOORD3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3dv"))) == NULL) || r;
  r = ((glMultiTexCoord3f = (PFNGLMULTITEXCOORD3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3f"))) == NULL) || r;
  r = ((glMultiTexCoord3fv = (PFNGLMULTITEXCOORD3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3fv"))) == NULL) || r;
  r = ((glMultiTexCoord3i = (PFNGLMULTITEXCOORD3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3i"))) == NULL) || r;
  r = ((glMultiTexCoord3iv = (PFNGLMULTITEXCOORD3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3iv"))) == NULL) || r;
  r = ((glMultiTexCoord3s = (PFNGLMULTITEXCOORD3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3s"))) == NULL) || r;
  r = ((glMultiTexCoord3sv = (PFNGLMULTITEXCOORD3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3sv"))) == NULL) || r;
  r = ((glMultiTexCoord4d = (PFNGLMULTITEXCOORD4DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4d"))) == NULL) || r;
  r = ((glMultiTexCoord4dv = (PFNGLMULTITEXCOORD4DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4dv"))) == NULL) || r;
  r = ((glMultiTexCoord4f = (PFNGLMULTITEXCOORD4FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4f"))) == NULL) || r;
  r = ((glMultiTexCoord4fv = (PFNGLMULTITEXCOORD4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4fv"))) == NULL) || r;
  r = ((glMultiTexCoord4i = (PFNGLMULTITEXCOORD4IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4i"))) == NULL) || r;
  r = ((glMultiTexCoord4iv = (PFNGLMULTITEXCOORD4IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4iv"))) == NULL) || r;
  r = ((glMultiTexCoord4s = (PFNGLMULTITEXCOORD4SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4s"))) == NULL) || r;
  r = ((glMultiTexCoord4sv = (PFNGLMULTITEXCOORD4SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4sv"))) == NULL) || r;
  r = ((glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSampleCoverage"))) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_1_3 */

#ifdef GL_VERSION_1_4

static GLboolean _glewInit_GL_VERSION_1_4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendColor = (PFNGLBLENDCOLORPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendColor"))) == NULL) || r;
  r = ((glBlendEquation = (PFNGLBLENDEQUATIONPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendEquation"))) == NULL) || r;
  r = ((glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendFuncSeparate"))) == NULL) || r;
  r = ((glFogCoordPointer = (PFNGLFOGCOORDPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordPointer"))) == NULL) || r;
  r = ((glFogCoordd = (PFNGLFOGCOORDDPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordd"))) == NULL) || r;
  r = ((glFogCoorddv = (PFNGLFOGCOORDDVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoorddv"))) == NULL) || r;
  r = ((glFogCoordf = (PFNGLFOGCOORDFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordf"))) == NULL) || r;
  r = ((glFogCoordfv = (PFNGLFOGCOORDFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordfv"))) == NULL) || r;
  r = ((glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiDrawArrays"))) == NULL) || r;
  r = ((glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiDrawElements"))) == NULL) || r;
  r = ((glPointParameterf = (PFNGLPOINTPARAMETERFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameterf"))) == NULL) || r;
  r = ((glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameterfv"))) == NULL) || r;
  r = ((glPointParameteri = (PFNGLPOINTPARAMETERIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameteri"))) == NULL) || r;
  r = ((glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameteriv"))) == NULL) || r;
  r = ((glSecondaryColor3b = (PFNGLSECONDARYCOLOR3BPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3b"))) == NULL) || r;
  r = ((glSecondaryColor3bv = (PFNGLSECONDARYCOLOR3BVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3bv"))) == NULL) || r;
  r = ((glSecondaryColor3d = (PFNGLSECONDARYCOLOR3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3d"))) == NULL) || r;
  r = ((glSecondaryColor3dv = (PFNGLSECONDARYCOLOR3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3dv"))) == NULL) || r;
  r = ((glSecondaryColor3f = (PFNGLSECONDARYCOLOR3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3f"))) == NULL) || r;
  r = ((glSecondaryColor3fv = (PFNGLSECONDARYCOLOR3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3fv"))) == NULL) || r;
  r = ((glSecondaryColor3i = (PFNGLSECONDARYCOLOR3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3i"))) == NULL) || r;
  r = ((glSecondaryColor3iv = (PFNGLSECONDARYCOLOR3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3iv"))) == NULL) || r;
  r = ((glSecondaryColor3s = (PFNGLSECONDARYCOLOR3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3s"))) == NULL) || r;
  r = ((glSecondaryColor3sv = (PFNGLSECONDARYCOLOR3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3sv"))) == NULL) || r;
  r = ((glSecondaryColor3ub = (PFNGLSECONDARYCOLOR3UBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3ub"))) == NULL) || r;
  r = ((glSecondaryColor3ubv = (PFNGLSECONDARYCOLOR3UBVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3ubv"))) == NULL) || r;
  r = ((glSecondaryColor3ui = (PFNGLSECONDARYCOLOR3UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3ui"))) == NULL) || r;
  r = ((glSecondaryColor3uiv = (PFNGLSECONDARYCOLOR3UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3uiv"))) == NULL) || r;
  r = ((glSecondaryColor3us = (PFNGLSECONDARYCOLOR3USPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3us"))) == NULL) || r;
  r = ((glSecondaryColor3usv = (PFNGLSECONDARYCOLOR3USVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3usv"))) == NULL) || r;
  r = ((glSecondaryColorPointer = (PFNGLSECONDARYCOLORPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColorPointer"))) == NULL) || r;
  r = ((glWindowPos2d = (PFNGLWINDOWPOS2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2d"))) == NULL) || r;
  r = ((glWindowPos2dv = (PFNGLWINDOWPOS2DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2dv"))) == NULL) || r;
  r = ((glWindowPos2f = (PFNGLWINDOWPOS2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2f"))) == NULL) || r;
  r = ((glWindowPos2fv = (PFNGLWINDOWPOS2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2fv"))) == NULL) || r;
  r = ((glWindowPos2i = (PFNGLWINDOWPOS2IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2i"))) == NULL) || r;
  r = ((glWindowPos2iv = (PFNGLWINDOWPOS2IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2iv"))) == NULL) || r;
  r = ((glWindowPos2s = (PFNGLWINDOWPOS2SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2s"))) == NULL) || r;
  r = ((glWindowPos2sv = (PFNGLWINDOWPOS2SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2sv"))) == NULL) || r;
  r = ((glWindowPos3d = (PFNGLWINDOWPOS3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3d"))) == NULL) || r;
  r = ((glWindowPos3dv = (PFNGLWINDOWPOS3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3dv"))) == NULL) || r;
  r = ((glWindowPos3f = (PFNGLWINDOWPOS3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3f"))) == NULL) || r;
  r = ((glWindowPos3fv = (PFNGLWINDOWPOS3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3fv"))) == NULL) || r;
  r = ((glWindowPos3i = (PFNGLWINDOWPOS3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3i"))) == NULL) || r;
  r = ((glWindowPos3iv = (PFNGLWINDOWPOS3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3iv"))) == NULL) || r;
  r = ((glWindowPos3s = (PFNGLWINDOWPOS3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3s"))) == NULL) || r;
  r = ((glWindowPos3sv = (PFNGLWINDOWPOS3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3sv"))) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_1_4 */

#ifdef GL_VERSION_1_5

static GLboolean _glewInit_GL_VERSION_1_5 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginQuery = (PFNGLBEGINQUERYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginQuery"))) == NULL) || r;
  r = ((glBindBuffer = (PFNGLBINDBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBuffer"))) == NULL) || r;
  r = ((glBufferData = (PFNGLBUFFERDATAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBufferData"))) == NULL) || r;
  r = ((glBufferSubData = (PFNGLBUFFERSUBDATAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBufferSubData"))) == NULL) || r;
  r = ((glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteBuffers"))) == NULL) || r;
  r = ((glDeleteQueries = (PFNGLDELETEQUERIESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteQueries"))) == NULL) || r;
  r = ((glEndQuery = (PFNGLENDQUERYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndQuery"))) == NULL) || r;
  r = ((glGenBuffers = (PFNGLGENBUFFERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenBuffers"))) == NULL) || r;
  r = ((glGenQueries = (PFNGLGENQUERIESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenQueries"))) == NULL) || r;
  r = ((glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBufferParameteriv"))) == NULL) || r;
  r = ((glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBufferPointerv"))) == NULL) || r;
  r = ((glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBufferSubData"))) == NULL) || r;
  r = ((glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetQueryObjectiv"))) == NULL) || r;
  r = ((glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetQueryObjectuiv"))) == NULL) || r;
  r = ((glGetQueryiv = (PFNGLGETQUERYIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetQueryiv"))) == NULL) || r;
  r = ((glIsBuffer = (PFNGLISBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsBuffer"))) == NULL) || r;
  r = ((glIsQuery = (PFNGLISQUERYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsQuery"))) == NULL) || r;
  r = ((glMapBuffer = (PFNGLMAPBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapBuffer"))) == NULL) || r;
  r = ((glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUnmapBuffer"))) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_1_5 */

#ifdef GL_VERSION_2_0

static GLboolean _glewInit_GL_VERSION_2_0 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAttachShader = (PFNGLATTACHSHADERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAttachShader"))) == NULL) || r;
  r = ((glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindAttribLocation"))) == NULL) || r;
  r = ((glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendEquationSeparate"))) == NULL) || r;
  r = ((glCompileShader = (PFNGLCOMPILESHADERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompileShader"))) == NULL) || r;
  r = ((glCreateProgram = (PFNGLCREATEPROGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCreateProgram"))) == NULL) || r;
  r = ((glCreateShader = (PFNGLCREATESHADERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCreateShader"))) == NULL) || r;
  r = ((glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteProgram"))) == NULL) || r;
  r = ((glDeleteShader = (PFNGLDELETESHADERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteShader"))) == NULL) || r;
  r = ((glDetachShader = (PFNGLDETACHSHADERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDetachShader"))) == NULL) || r;
  r = ((glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDisableVertexAttribArray"))) == NULL) || r;
  r = ((glDrawBuffers = (PFNGLDRAWBUFFERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawBuffers"))) == NULL) || r;
  r = ((glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnableVertexAttribArray"))) == NULL) || r;
  r = ((glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetActiveAttrib"))) == NULL) || r;
  r = ((glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetActiveUniform"))) == NULL) || r;
  r = ((glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetAttachedShaders"))) == NULL) || r;
  r = ((glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetAttribLocation"))) == NULL) || r;
  r = ((glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramInfoLog"))) == NULL) || r;
  r = ((glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramiv"))) == NULL) || r;
  r = ((glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetShaderInfoLog"))) == NULL) || r;
  r = ((glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetShaderSource"))) == NULL) || r;
  r = ((glGetShaderiv = (PFNGLGETSHADERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetShaderiv"))) == NULL) || r;
  r = ((glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformLocation"))) == NULL) || r;
  r = ((glGetUniformfv = (PFNGLGETUNIFORMFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformfv"))) == NULL) || r;
  r = ((glGetUniformiv = (PFNGLGETUNIFORMIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformiv"))) == NULL) || r;
  r = ((glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribPointerv"))) == NULL) || r;
  r = ((glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribdv"))) == NULL) || r;
  r = ((glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribfv"))) == NULL) || r;
  r = ((glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribiv"))) == NULL) || r;
  r = ((glIsProgram = (PFNGLISPROGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsProgram"))) == NULL) || r;
  r = ((glIsShader = (PFNGLISSHADERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsShader"))) == NULL) || r;
  r = ((glLinkProgram = (PFNGLLINKPROGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLinkProgram"))) == NULL) || r;
  r = ((glShaderSource = (PFNGLSHADERSOURCEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glShaderSource"))) == NULL) || r;
  r = ((glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStencilFuncSeparate"))) == NULL) || r;
  r = ((glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStencilMaskSeparate"))) == NULL) || r;
  r = ((glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStencilOpSeparate"))) == NULL) || r;
  r = ((glUniform1f = (PFNGLUNIFORM1FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1f"))) == NULL) || r;
  r = ((glUniform1fv = (PFNGLUNIFORM1FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1fv"))) == NULL) || r;
  r = ((glUniform1i = (PFNGLUNIFORM1IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1i"))) == NULL) || r;
  r = ((glUniform1iv = (PFNGLUNIFORM1IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1iv"))) == NULL) || r;
  r = ((glUniform2f = (PFNGLUNIFORM2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2f"))) == NULL) || r;
  r = ((glUniform2fv = (PFNGLUNIFORM2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2fv"))) == NULL) || r;
  r = ((glUniform2i = (PFNGLUNIFORM2IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2i"))) == NULL) || r;
  r = ((glUniform2iv = (PFNGLUNIFORM2IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2iv"))) == NULL) || r;
  r = ((glUniform3f = (PFNGLUNIFORM3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3f"))) == NULL) || r;
  r = ((glUniform3fv = (PFNGLUNIFORM3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3fv"))) == NULL) || r;
  r = ((glUniform3i = (PFNGLUNIFORM3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3i"))) == NULL) || r;
  r = ((glUniform3iv = (PFNGLUNIFORM3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3iv"))) == NULL) || r;
  r = ((glUniform4f = (PFNGLUNIFORM4FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4f"))) == NULL) || r;
  r = ((glUniform4fv = (PFNGLUNIFORM4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4fv"))) == NULL) || r;
  r = ((glUniform4i = (PFNGLUNIFORM4IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4i"))) == NULL) || r;
  r = ((glUniform4iv = (PFNGLUNIFORM4IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4iv"))) == NULL) || r;
  r = ((glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix2fv"))) == NULL) || r;
  r = ((glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix3fv"))) == NULL) || r;
  r = ((glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix4fv"))) == NULL) || r;
  r = ((glUseProgram = (PFNGLUSEPROGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUseProgram"))) == NULL) || r;
  r = ((glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glValidateProgram"))) == NULL) || r;
  r = ((glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1d"))) == NULL) || r;
  r = ((glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1dv"))) == NULL) || r;
  r = ((glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1f"))) == NULL) || r;
  r = ((glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1fv"))) == NULL) || r;
  r = ((glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1s"))) == NULL) || r;
  r = ((glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1sv"))) == NULL) || r;
  r = ((glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2d"))) == NULL) || r;
  r = ((glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2dv"))) == NULL) || r;
  r = ((glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2f"))) == NULL) || r;
  r = ((glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2fv"))) == NULL) || r;
  r = ((glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2s"))) == NULL) || r;
  r = ((glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2sv"))) == NULL) || r;
  r = ((glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3d"))) == NULL) || r;
  r = ((glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3dv"))) == NULL) || r;
  r = ((glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3f"))) == NULL) || r;
  r = ((glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3fv"))) == NULL) || r;
  r = ((glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3s"))) == NULL) || r;
  r = ((glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3sv"))) == NULL) || r;
  r = ((glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4Nbv"))) == NULL) || r;
  r = ((glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4Niv"))) == NULL) || r;
  r = ((glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4Nsv"))) == NULL) || r;
  r = ((glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4Nub"))) == NULL) || r;
  r = ((glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4Nubv"))) == NULL) || r;
  r = ((glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4Nuiv"))) == NULL) || r;
  r = ((glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4Nusv"))) == NULL) || r;
  r = ((glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4bv"))) == NULL) || r;
  r = ((glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4d"))) == NULL) || r;
  r = ((glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4dv"))) == NULL) || r;
  r = ((glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4f"))) == NULL) || r;
  r = ((glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4fv"))) == NULL) || r;
  r = ((glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4iv"))) == NULL) || r;
  r = ((glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4s"))) == NULL) || r;
  r = ((glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4sv"))) == NULL) || r;
  r = ((glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4ubv"))) == NULL) || r;
  r = ((glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4uiv"))) == NULL) || r;
  r = ((glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4usv"))) == NULL) || r;
  r = ((glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribPointer"))) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_2_0 */

#ifdef GL_VERSION_2_1

static GLboolean _glewInit_GL_VERSION_2_1 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix2x3fv"))) == NULL) || r;
  r = ((glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix2x4fv"))) == NULL) || r;
  r = ((glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix3x2fv"))) == NULL) || r;
  r = ((glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix3x4fv"))) == NULL) || r;
  r = ((glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix4x2fv"))) == NULL) || r;
  r = ((glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix4x3fv"))) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_2_1 */

#ifdef GL_VERSION_3_0

static GLboolean _glewInit_GL_VERSION_3_0 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginConditionalRender"))) == NULL) || r;
  r = ((glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginTransformFeedback"))) == NULL) || r;
  r = ((glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferBase"))) == NULL) || r;
  r = ((glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferRange"))) == NULL) || r;
  r = ((glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindFragDataLocation"))) == NULL) || r;
  r = ((glClampColor = (PFNGLCLAMPCOLORPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClampColor"))) == NULL) || r;
  r = ((glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearBufferfi"))) == NULL) || r;
  r = ((glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearBufferfv"))) == NULL) || r;
  r = ((glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearBufferiv"))) == NULL) || r;
  r = ((glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearBufferuiv"))) == NULL) || r;
  r = ((glColorMaski = (PFNGLCOLORMASKIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorMaski"))) == NULL) || r;
  r = ((glDisablei = (PFNGLDISABLEIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDisablei"))) == NULL) || r;
  r = ((glEnablei = (PFNGLENABLEIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnablei"))) == NULL) || r;
  r = ((glEndConditionalRender = (PFNGLENDCONDITIONALRENDERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndConditionalRender"))) == NULL) || r;
  r = ((glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndTransformFeedback"))) == NULL) || r;
  r = ((glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBooleani_v"))) == NULL) || r;
  r = ((glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragDataLocation"))) == NULL) || r;
  r = ((glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetIntegeri_v"))) == NULL) || r;
  r = ((glGetStringi = (PFNGLGETSTRINGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetStringi"))) == NULL) || r;
  r = ((glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexParameterIiv"))) == NULL) || r;
  r = ((glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexParameterIuiv"))) == NULL) || r;
  r = ((glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTransformFeedbackVarying"))) == NULL) || r;
  r = ((glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformuiv"))) == NULL) || r;
  r = ((glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribIiv"))) == NULL) || r;
  r = ((glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribIuiv"))) == NULL) || r;
  r = ((glIsEnabledi = (PFNGLISENABLEDIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsEnabledi"))) == NULL) || r;
  r = ((glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexParameterIiv"))) == NULL) || r;
  r = ((glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexParameterIuiv"))) == NULL) || r;
  r = ((glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTransformFeedbackVaryings"))) == NULL) || r;
  r = ((glUniform1ui = (PFNGLUNIFORM1UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1ui"))) == NULL) || r;
  r = ((glUniform1uiv = (PFNGLUNIFORM1UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1uiv"))) == NULL) || r;
  r = ((glUniform2ui = (PFNGLUNIFORM2UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2ui"))) == NULL) || r;
  r = ((glUniform2uiv = (PFNGLUNIFORM2UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2uiv"))) == NULL) || r;
  r = ((glUniform3ui = (PFNGLUNIFORM3UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3ui"))) == NULL) || r;
  r = ((glUniform3uiv = (PFNGLUNIFORM3UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3uiv"))) == NULL) || r;
  r = ((glUniform4ui = (PFNGLUNIFORM4UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4ui"))) == NULL) || r;
  r = ((glUniform4uiv = (PFNGLUNIFORM4UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4uiv"))) == NULL) || r;
  r = ((glVertexAttribI1i = (PFNGLVERTEXATTRIBI1IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI1i"))) == NULL) || r;
  r = ((glVertexAttribI1iv = (PFNGLVERTEXATTRIBI1IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI1iv"))) == NULL) || r;
  r = ((glVertexAttribI1ui = (PFNGLVERTEXATTRIBI1UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI1ui"))) == NULL) || r;
  r = ((glVertexAttribI1uiv = (PFNGLVERTEXATTRIBI1UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI1uiv"))) == NULL) || r;
  r = ((glVertexAttribI2i = (PFNGLVERTEXATTRIBI2IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI2i"))) == NULL) || r;
  r = ((glVertexAttribI2iv = (PFNGLVERTEXATTRIBI2IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI2iv"))) == NULL) || r;
  r = ((glVertexAttribI2ui = (PFNGLVERTEXATTRIBI2UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI2ui"))) == NULL) || r;
  r = ((glVertexAttribI2uiv = (PFNGLVERTEXATTRIBI2UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI2uiv"))) == NULL) || r;
  r = ((glVertexAttribI3i = (PFNGLVERTEXATTRIBI3IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI3i"))) == NULL) || r;
  r = ((glVertexAttribI3iv = (PFNGLVERTEXATTRIBI3IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI3iv"))) == NULL) || r;
  r = ((glVertexAttribI3ui = (PFNGLVERTEXATTRIBI3UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI3ui"))) == NULL) || r;
  r = ((glVertexAttribI3uiv = (PFNGLVERTEXATTRIBI3UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI3uiv"))) == NULL) || r;
  r = ((glVertexAttribI4bv = (PFNGLVERTEXATTRIBI4BVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4bv"))) == NULL) || r;
  r = ((glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4i"))) == NULL) || r;
  r = ((glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4iv"))) == NULL) || r;
  r = ((glVertexAttribI4sv = (PFNGLVERTEXATTRIBI4SVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4sv"))) == NULL) || r;
  r = ((glVertexAttribI4ubv = (PFNGLVERTEXATTRIBI4UBVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4ubv"))) == NULL) || r;
  r = ((glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4ui"))) == NULL) || r;
  r = ((glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4uiv"))) == NULL) || r;
  r = ((glVertexAttribI4usv = (PFNGLVERTEXATTRIBI4USVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4usv"))) == NULL) || r;
  r = ((glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribIPointer"))) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_3_0 */

#ifdef GL_3DFX_multisample

#endif /* GL_3DFX_multisample */

#ifdef GL_3DFX_tbuffer

static GLboolean _glewInit_GL_3DFX_tbuffer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTbufferMask3DFX = (PFNGLTBUFFERMASK3DFXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTbufferMask3DFX"))) == NULL) || r;

  return r;
}

#endif /* GL_3DFX_tbuffer */

#ifdef GL_3DFX_texture_compression_FXT1

#endif /* GL_3DFX_texture_compression_FXT1 */

#ifdef GL_APPLE_client_storage

#endif /* GL_APPLE_client_storage */

#ifdef GL_APPLE_element_array

static GLboolean _glewInit_GL_APPLE_element_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawElementArrayAPPLE = (PFNGLDRAWELEMENTARRAYAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawElementArrayAPPLE"))) == NULL) || r;
  r = ((glDrawRangeElementArrayAPPLE = (PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawRangeElementArrayAPPLE"))) == NULL) || r;
  r = ((glElementPointerAPPLE = (PFNGLELEMENTPOINTERAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glElementPointerAPPLE"))) == NULL) || r;
  r = ((glMultiDrawElementArrayAPPLE = (PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiDrawElementArrayAPPLE"))) == NULL) || r;
  r = ((glMultiDrawRangeElementArrayAPPLE = (PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiDrawRangeElementArrayAPPLE"))) == NULL) || r;

  return r;
}

#endif /* GL_APPLE_element_array */

#ifdef GL_APPLE_fence

static GLboolean _glewInit_GL_APPLE_fence (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDeleteFencesAPPLE = (PFNGLDELETEFENCESAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteFencesAPPLE"))) == NULL) || r;
  r = ((glFinishFenceAPPLE = (PFNGLFINISHFENCEAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFinishFenceAPPLE"))) == NULL) || r;
  r = ((glFinishObjectAPPLE = (PFNGLFINISHOBJECTAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFinishObjectAPPLE"))) == NULL) || r;
  r = ((glGenFencesAPPLE = (PFNGLGENFENCESAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenFencesAPPLE"))) == NULL) || r;
  r = ((glIsFenceAPPLE = (PFNGLISFENCEAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsFenceAPPLE"))) == NULL) || r;
  r = ((glSetFenceAPPLE = (PFNGLSETFENCEAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSetFenceAPPLE"))) == NULL) || r;
  r = ((glTestFenceAPPLE = (PFNGLTESTFENCEAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTestFenceAPPLE"))) == NULL) || r;
  r = ((glTestObjectAPPLE = (PFNGLTESTOBJECTAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTestObjectAPPLE"))) == NULL) || r;

  return r;
}

#endif /* GL_APPLE_fence */

#ifdef GL_APPLE_float_pixels

#endif /* GL_APPLE_float_pixels */

#ifdef GL_APPLE_flush_buffer_range

static GLboolean _glewInit_GL_APPLE_flush_buffer_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBufferParameteriAPPLE = (PFNGLBUFFERPARAMETERIAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBufferParameteriAPPLE"))) == NULL) || r;
  r = ((glFlushMappedBufferRangeAPPLE = (PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFlushMappedBufferRangeAPPLE"))) == NULL) || r;

  return r;
}

#endif /* GL_APPLE_flush_buffer_range */

#ifdef GL_APPLE_pixel_buffer

#endif /* GL_APPLE_pixel_buffer */

#ifdef GL_APPLE_specular_vector

#endif /* GL_APPLE_specular_vector */

#ifdef GL_APPLE_texture_range

static GLboolean _glewInit_GL_APPLE_texture_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetTexParameterPointervAPPLE = (PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexParameterPointervAPPLE"))) == NULL) || r;
  r = ((glTextureRangeAPPLE = (PFNGLTEXTURERANGEAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureRangeAPPLE"))) == NULL) || r;

  return r;
}

#endif /* GL_APPLE_texture_range */

#ifdef GL_APPLE_transform_hint

#endif /* GL_APPLE_transform_hint */

#ifdef GL_APPLE_vertex_array_object

static GLboolean _glewInit_GL_APPLE_vertex_array_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindVertexArrayAPPLE = (PFNGLBINDVERTEXARRAYAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindVertexArrayAPPLE"))) == NULL) || r;
  r = ((glDeleteVertexArraysAPPLE = (PFNGLDELETEVERTEXARRAYSAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteVertexArraysAPPLE"))) == NULL) || r;
  r = ((glGenVertexArraysAPPLE = (PFNGLGENVERTEXARRAYSAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenVertexArraysAPPLE"))) == NULL) || r;
  r = ((glIsVertexArrayAPPLE = (PFNGLISVERTEXARRAYAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsVertexArrayAPPLE"))) == NULL) || r;

  return r;
}

#endif /* GL_APPLE_vertex_array_object */

#ifdef GL_APPLE_vertex_array_range

static GLboolean _glewInit_GL_APPLE_vertex_array_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushVertexArrayRangeAPPLE = (PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFlushVertexArrayRangeAPPLE"))) == NULL) || r;
  r = ((glVertexArrayParameteriAPPLE = (PFNGLVERTEXARRAYPARAMETERIAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexArrayParameteriAPPLE"))) == NULL) || r;
  r = ((glVertexArrayRangeAPPLE = (PFNGLVERTEXARRAYRANGEAPPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexArrayRangeAPPLE"))) == NULL) || r;

  return r;
}

#endif /* GL_APPLE_vertex_array_range */

#ifdef GL_APPLE_ycbcr_422

#endif /* GL_APPLE_ycbcr_422 */

#ifdef GL_ARB_color_buffer_float

static GLboolean _glewInit_GL_ARB_color_buffer_float (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClampColorARB = (PFNGLCLAMPCOLORARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClampColorARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_color_buffer_float */

#ifdef GL_ARB_depth_buffer_float

#endif /* GL_ARB_depth_buffer_float */

#ifdef GL_ARB_depth_texture

#endif /* GL_ARB_depth_texture */

#ifdef GL_ARB_draw_buffers

static GLboolean _glewInit_GL_ARB_draw_buffers (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawBuffersARB = (PFNGLDRAWBUFFERSARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawBuffersARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_draw_buffers */

#ifdef GL_ARB_draw_instanced

static GLboolean _glewInit_GL_ARB_draw_instanced (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawArraysInstancedARB = (PFNGLDRAWARRAYSINSTANCEDARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawArraysInstancedARB"))) == NULL) || r;
  r = ((glDrawElementsInstancedARB = (PFNGLDRAWELEMENTSINSTANCEDARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawElementsInstancedARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_draw_instanced */

#ifdef GL_ARB_fragment_program

#endif /* GL_ARB_fragment_program */

#ifdef GL_ARB_fragment_program_shadow

#endif /* GL_ARB_fragment_program_shadow */

#ifdef GL_ARB_fragment_shader

#endif /* GL_ARB_fragment_shader */

#ifdef GL_ARB_framebuffer_object

static GLboolean _glewInit_GL_ARB_framebuffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindFramebuffer"))) == NULL) || r;
  r = ((glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindRenderbuffer"))) == NULL) || r;
  r = ((glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlitFramebuffer"))) == NULL) || r;
  r = ((glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCheckFramebufferStatus"))) == NULL) || r;
  r = ((glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteFramebuffers"))) == NULL) || r;
  r = ((glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteRenderbuffers"))) == NULL) || r;
  r = ((glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferRenderbuffer"))) == NULL) || r;
  r = ((glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTexture1D"))) == NULL) || r;
  r = ((glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTexture2D"))) == NULL) || r;
  r = ((glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTexture3D"))) == NULL) || r;
  r = ((glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTextureLayer"))) == NULL) || r;
  r = ((glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenFramebuffers"))) == NULL) || r;
  r = ((glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenRenderbuffers"))) == NULL) || r;
  r = ((glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenerateMipmap"))) == NULL) || r;
  r = ((glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFramebufferAttachmentParameteriv"))) == NULL) || r;
  r = ((glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetRenderbufferParameteriv"))) == NULL) || r;
  r = ((glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsFramebuffer"))) == NULL) || r;
  r = ((glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsRenderbuffer"))) == NULL) || r;
  r = ((glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRenderbufferStorage"))) == NULL) || r;
  r = ((glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRenderbufferStorageMultisample"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_framebuffer_object */

#ifdef GL_ARB_framebuffer_sRGB

#endif /* GL_ARB_framebuffer_sRGB */

#ifdef GL_ARB_geometry_shader4

static GLboolean _glewInit_GL_ARB_geometry_shader4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFramebufferTextureARB = (PFNGLFRAMEBUFFERTEXTUREARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTextureARB"))) == NULL) || r;
  r = ((glFramebufferTextureFaceARB = (PFNGLFRAMEBUFFERTEXTUREFACEARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTextureFaceARB"))) == NULL) || r;
  r = ((glFramebufferTextureLayerARB = (PFNGLFRAMEBUFFERTEXTURELAYERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTextureLayerARB"))) == NULL) || r;
  r = ((glProgramParameteriARB = (PFNGLPROGRAMPARAMETERIARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramParameteriARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_geometry_shader4 */

#ifdef GL_ARB_half_float_pixel

#endif /* GL_ARB_half_float_pixel */

#ifdef GL_ARB_half_float_vertex

#endif /* GL_ARB_half_float_vertex */

#ifdef GL_ARB_imaging

static GLboolean _glewInit_GL_ARB_imaging (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquation = (PFNGLBLENDEQUATIONPROC)glewGetProcAddress("glBlendEquation")) == NULL) || r;
  r = ((glColorSubTable = (PFNGLCOLORSUBTABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorSubTable"))) == NULL) || r;
  r = ((glColorTable = (PFNGLCOLORTABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorTable"))) == NULL) || r;
  r = ((glColorTableParameterfv = (PFNGLCOLORTABLEPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorTableParameterfv"))) == NULL) || r;
  r = ((glColorTableParameteriv = (PFNGLCOLORTABLEPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorTableParameteriv"))) == NULL) || r;
  r = ((glConvolutionFilter1D = (PFNGLCONVOLUTIONFILTER1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionFilter1D"))) == NULL) || r;
  r = ((glConvolutionFilter2D = (PFNGLCONVOLUTIONFILTER2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionFilter2D"))) == NULL) || r;
  r = ((glConvolutionParameterf = (PFNGLCONVOLUTIONPARAMETERFPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionParameterf"))) == NULL) || r;
  r = ((glConvolutionParameterfv = (PFNGLCONVOLUTIONPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionParameterfv"))) == NULL) || r;
  r = ((glConvolutionParameteri = (PFNGLCONVOLUTIONPARAMETERIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionParameteri"))) == NULL) || r;
  r = ((glConvolutionParameteriv = (PFNGLCONVOLUTIONPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionParameteriv"))) == NULL) || r;
  r = ((glCopyColorSubTable = (PFNGLCOPYCOLORSUBTABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyColorSubTable"))) == NULL) || r;
  r = ((glCopyColorTable = (PFNGLCOPYCOLORTABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyColorTable"))) == NULL) || r;
  r = ((glCopyConvolutionFilter1D = (PFNGLCOPYCONVOLUTIONFILTER1DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyConvolutionFilter1D"))) == NULL) || r;
  r = ((glCopyConvolutionFilter2D = (PFNGLCOPYCONVOLUTIONFILTER2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyConvolutionFilter2D"))) == NULL) || r;
  r = ((glGetColorTable = (PFNGLGETCOLORTABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTable"))) == NULL) || r;
  r = ((glGetColorTableParameterfv = (PFNGLGETCOLORTABLEPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTableParameterfv"))) == NULL) || r;
  r = ((glGetColorTableParameteriv = (PFNGLGETCOLORTABLEPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTableParameteriv"))) == NULL) || r;
  r = ((glGetConvolutionFilter = (PFNGLGETCONVOLUTIONFILTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetConvolutionFilter"))) == NULL) || r;
  r = ((glGetConvolutionParameterfv = (PFNGLGETCONVOLUTIONPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetConvolutionParameterfv"))) == NULL) || r;
  r = ((glGetConvolutionParameteriv = (PFNGLGETCONVOLUTIONPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetConvolutionParameteriv"))) == NULL) || r;
  r = ((glGetHistogram = (PFNGLGETHISTOGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetHistogram"))) == NULL) || r;
  r = ((glGetHistogramParameterfv = (PFNGLGETHISTOGRAMPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetHistogramParameterfv"))) == NULL) || r;
  r = ((glGetHistogramParameteriv = (PFNGLGETHISTOGRAMPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetHistogramParameteriv"))) == NULL) || r;
  r = ((glGetMinmax = (PFNGLGETMINMAXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMinmax"))) == NULL) || r;
  r = ((glGetMinmaxParameterfv = (PFNGLGETMINMAXPARAMETERFVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMinmaxParameterfv"))) == NULL) || r;
  r = ((glGetMinmaxParameteriv = (PFNGLGETMINMAXPARAMETERIVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMinmaxParameteriv"))) == NULL) || r;
  r = ((glGetSeparableFilter = (PFNGLGETSEPARABLEFILTERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetSeparableFilter"))) == NULL) || r;
  r = ((glHistogram = (PFNGLHISTOGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glHistogram"))) == NULL) || r;
  r = ((glMinmax = (PFNGLMINMAXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMinmax"))) == NULL) || r;
  r = ((glResetHistogram = (PFNGLRESETHISTOGRAMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glResetHistogram"))) == NULL) || r;
  r = ((glResetMinmax = (PFNGLRESETMINMAXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glResetMinmax"))) == NULL) || r;
  r = ((glSeparableFilter2D = (PFNGLSEPARABLEFILTER2DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSeparableFilter2D"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_imaging */

#ifdef GL_ARB_instanced_arrays

static GLboolean _glewInit_GL_ARB_instanced_arrays (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glVertexAttribDivisorARB = (PFNGLVERTEXATTRIBDIVISORARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribDivisorARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_instanced_arrays */

#ifdef GL_ARB_map_buffer_range

static GLboolean _glewInit_GL_ARB_map_buffer_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFlushMappedBufferRange"))) == NULL) || r;
  r = ((glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapBufferRange"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_map_buffer_range */

#ifdef GL_ARB_matrix_palette

static GLboolean _glewInit_GL_ARB_matrix_palette (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCurrentPaletteMatrixARB = (PFNGLCURRENTPALETTEMATRIXARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCurrentPaletteMatrixARB"))) == NULL) || r;
  r = ((glMatrixIndexPointerARB = (PFNGLMATRIXINDEXPOINTERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixIndexPointerARB"))) == NULL) || r;
  r = ((glMatrixIndexubvARB = (PFNGLMATRIXINDEXUBVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixIndexubvARB"))) == NULL) || r;
  r = ((glMatrixIndexuivARB = (PFNGLMATRIXINDEXUIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixIndexuivARB"))) == NULL) || r;
  r = ((glMatrixIndexusvARB = (PFNGLMATRIXINDEXUSVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixIndexusvARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_matrix_palette */

#ifdef GL_ARB_multisample

static GLboolean _glewInit_GL_ARB_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSampleCoverageARB = (PFNGLSAMPLECOVERAGEARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSampleCoverageARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_multisample */

#ifdef GL_ARB_multitexture

static GLboolean _glewInit_GL_ARB_multitexture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glActiveTextureARB"))) == NULL) || r;
  r = ((glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClientActiveTextureARB"))) == NULL) || r;
  r = ((glMultiTexCoord1dARB = (PFNGLMULTITEXCOORD1DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1dARB"))) == NULL) || r;
  r = ((glMultiTexCoord1dvARB = (PFNGLMULTITEXCOORD1DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1dvARB"))) == NULL) || r;
  r = ((glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1fARB"))) == NULL) || r;
  r = ((glMultiTexCoord1fvARB = (PFNGLMULTITEXCOORD1FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1fvARB"))) == NULL) || r;
  r = ((glMultiTexCoord1iARB = (PFNGLMULTITEXCOORD1IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1iARB"))) == NULL) || r;
  r = ((glMultiTexCoord1ivARB = (PFNGLMULTITEXCOORD1IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1ivARB"))) == NULL) || r;
  r = ((glMultiTexCoord1sARB = (PFNGLMULTITEXCOORD1SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1sARB"))) == NULL) || r;
  r = ((glMultiTexCoord1svARB = (PFNGLMULTITEXCOORD1SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1svARB"))) == NULL) || r;
  r = ((glMultiTexCoord2dARB = (PFNGLMULTITEXCOORD2DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2dARB"))) == NULL) || r;
  r = ((glMultiTexCoord2dvARB = (PFNGLMULTITEXCOORD2DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2dvARB"))) == NULL) || r;
  r = ((glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2fARB"))) == NULL) || r;
  r = ((glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2fvARB"))) == NULL) || r;
  r = ((glMultiTexCoord2iARB = (PFNGLMULTITEXCOORD2IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2iARB"))) == NULL) || r;
  r = ((glMultiTexCoord2ivARB = (PFNGLMULTITEXCOORD2IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2ivARB"))) == NULL) || r;
  r = ((glMultiTexCoord2sARB = (PFNGLMULTITEXCOORD2SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2sARB"))) == NULL) || r;
  r = ((glMultiTexCoord2svARB = (PFNGLMULTITEXCOORD2SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2svARB"))) == NULL) || r;
  r = ((glMultiTexCoord3dARB = (PFNGLMULTITEXCOORD3DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3dARB"))) == NULL) || r;
  r = ((glMultiTexCoord3dvARB = (PFNGLMULTITEXCOORD3DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3dvARB"))) == NULL) || r;
  r = ((glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3fARB"))) == NULL) || r;
  r = ((glMultiTexCoord3fvARB = (PFNGLMULTITEXCOORD3FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3fvARB"))) == NULL) || r;
  r = ((glMultiTexCoord3iARB = (PFNGLMULTITEXCOORD3IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3iARB"))) == NULL) || r;
  r = ((glMultiTexCoord3ivARB = (PFNGLMULTITEXCOORD3IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3ivARB"))) == NULL) || r;
  r = ((glMultiTexCoord3sARB = (PFNGLMULTITEXCOORD3SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3sARB"))) == NULL) || r;
  r = ((glMultiTexCoord3svARB = (PFNGLMULTITEXCOORD3SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3svARB"))) == NULL) || r;
  r = ((glMultiTexCoord4dARB = (PFNGLMULTITEXCOORD4DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4dARB"))) == NULL) || r;
  r = ((glMultiTexCoord4dvARB = (PFNGLMULTITEXCOORD4DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4dvARB"))) == NULL) || r;
  r = ((glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4fARB"))) == NULL) || r;
  r = ((glMultiTexCoord4fvARB = (PFNGLMULTITEXCOORD4FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4fvARB"))) == NULL) || r;
  r = ((glMultiTexCoord4iARB = (PFNGLMULTITEXCOORD4IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4iARB"))) == NULL) || r;
  r = ((glMultiTexCoord4ivARB = (PFNGLMULTITEXCOORD4IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4ivARB"))) == NULL) || r;
  r = ((glMultiTexCoord4sARB = (PFNGLMULTITEXCOORD4SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4sARB"))) == NULL) || r;
  r = ((glMultiTexCoord4svARB = (PFNGLMULTITEXCOORD4SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4svARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_multitexture */

#ifdef GL_ARB_occlusion_query

static GLboolean _glewInit_GL_ARB_occlusion_query (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginQueryARB"))) == NULL) || r;
  r = ((glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteQueriesARB"))) == NULL) || r;
  r = ((glEndQueryARB = (PFNGLENDQUERYARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndQueryARB"))) == NULL) || r;
  r = ((glGenQueriesARB = (PFNGLGENQUERIESARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenQueriesARB"))) == NULL) || r;
  r = ((glGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetQueryObjectivARB"))) == NULL) || r;
  r = ((glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetQueryObjectuivARB"))) == NULL) || r;
  r = ((glGetQueryivARB = (PFNGLGETQUERYIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetQueryivARB"))) == NULL) || r;
  r = ((glIsQueryARB = (PFNGLISQUERYARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsQueryARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_occlusion_query */

#ifdef GL_ARB_pixel_buffer_object

#endif /* GL_ARB_pixel_buffer_object */

#ifdef GL_ARB_point_parameters

static GLboolean _glewInit_GL_ARB_point_parameters (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPointParameterfARB = (PFNGLPOINTPARAMETERFARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameterfARB"))) == NULL) || r;
  r = ((glPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameterfvARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_point_parameters */

#ifdef GL_ARB_point_sprite

#endif /* GL_ARB_point_sprite */

#ifdef GL_ARB_shader_objects

static GLboolean _glewInit_GL_ARB_shader_objects (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAttachObjectARB"))) == NULL) || r;
  r = ((glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompileShaderARB"))) == NULL) || r;
  r = ((glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCreateProgramObjectARB"))) == NULL) || r;
  r = ((glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCreateShaderObjectARB"))) == NULL) || r;
  r = ((glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteObjectARB"))) == NULL) || r;
  r = ((glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDetachObjectARB"))) == NULL) || r;
  r = ((glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetActiveUniformARB"))) == NULL) || r;
  r = ((glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetAttachedObjectsARB"))) == NULL) || r;
  r = ((glGetHandleARB = (PFNGLGETHANDLEARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetHandleARB"))) == NULL) || r;
  r = ((glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetInfoLogARB"))) == NULL) || r;
  r = ((glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetObjectParameterfvARB"))) == NULL) || r;
  r = ((glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetObjectParameterivARB"))) == NULL) || r;
  r = ((glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetShaderSourceARB"))) == NULL) || r;
  r = ((glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformLocationARB"))) == NULL) || r;
  r = ((glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformfvARB"))) == NULL) || r;
  r = ((glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformivARB"))) == NULL) || r;
  r = ((glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLinkProgramARB"))) == NULL) || r;
  r = ((glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glShaderSourceARB"))) == NULL) || r;
  r = ((glUniform1fARB = (PFNGLUNIFORM1FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1fARB"))) == NULL) || r;
  r = ((glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1fvARB"))) == NULL) || r;
  r = ((glUniform1iARB = (PFNGLUNIFORM1IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1iARB"))) == NULL) || r;
  r = ((glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1ivARB"))) == NULL) || r;
  r = ((glUniform2fARB = (PFNGLUNIFORM2FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2fARB"))) == NULL) || r;
  r = ((glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2fvARB"))) == NULL) || r;
  r = ((glUniform2iARB = (PFNGLUNIFORM2IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2iARB"))) == NULL) || r;
  r = ((glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2ivARB"))) == NULL) || r;
  r = ((glUniform3fARB = (PFNGLUNIFORM3FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3fARB"))) == NULL) || r;
  r = ((glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3fvARB"))) == NULL) || r;
  r = ((glUniform3iARB = (PFNGLUNIFORM3IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3iARB"))) == NULL) || r;
  r = ((glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3ivARB"))) == NULL) || r;
  r = ((glUniform4fARB = (PFNGLUNIFORM4FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4fARB"))) == NULL) || r;
  r = ((glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4fvARB"))) == NULL) || r;
  r = ((glUniform4iARB = (PFNGLUNIFORM4IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4iARB"))) == NULL) || r;
  r = ((glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4ivARB"))) == NULL) || r;
  r = ((glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix2fvARB"))) == NULL) || r;
  r = ((glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix3fvARB"))) == NULL) || r;
  r = ((glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformMatrix4fvARB"))) == NULL) || r;
  r = ((glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUseProgramObjectARB"))) == NULL) || r;
  r = ((glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glValidateProgramARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_shader_objects */

#ifdef GL_ARB_shading_language_100

#endif /* GL_ARB_shading_language_100 */

#ifdef GL_ARB_shadow

#endif /* GL_ARB_shadow */

#ifdef GL_ARB_shadow_ambient

#endif /* GL_ARB_shadow_ambient */

#ifdef GL_ARB_texture_border_clamp

#endif /* GL_ARB_texture_border_clamp */

#ifdef GL_ARB_texture_buffer_object

static GLboolean _glewInit_GL_ARB_texture_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexBufferARB = (PFNGLTEXBUFFERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexBufferARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_texture_buffer_object */

#ifdef GL_ARB_texture_compression

static GLboolean _glewInit_GL_ARB_texture_compression (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCompressedTexImage1DARB = (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexImage1DARB"))) == NULL) || r;
  r = ((glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexImage2DARB"))) == NULL) || r;
  r = ((glCompressedTexImage3DARB = (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexImage3DARB"))) == NULL) || r;
  r = ((glCompressedTexSubImage1DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexSubImage1DARB"))) == NULL) || r;
  r = ((glCompressedTexSubImage2DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexSubImage2DARB"))) == NULL) || r;
  r = ((glCompressedTexSubImage3DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTexSubImage3DARB"))) == NULL) || r;
  r = ((glGetCompressedTexImageARB = (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCompressedTexImageARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_texture_compression */

#ifdef GL_ARB_texture_compression_rgtc

#endif /* GL_ARB_texture_compression_rgtc */

#ifdef GL_ARB_texture_cube_map

#endif /* GL_ARB_texture_cube_map */

#ifdef GL_ARB_texture_env_add

#endif /* GL_ARB_texture_env_add */

#ifdef GL_ARB_texture_env_combine

#endif /* GL_ARB_texture_env_combine */

#ifdef GL_ARB_texture_env_crossbar

#endif /* GL_ARB_texture_env_crossbar */

#ifdef GL_ARB_texture_env_dot3

#endif /* GL_ARB_texture_env_dot3 */

#ifdef GL_ARB_texture_float

#endif /* GL_ARB_texture_float */

#ifdef GL_ARB_texture_mirrored_repeat

#endif /* GL_ARB_texture_mirrored_repeat */

#ifdef GL_ARB_texture_non_power_of_two

#endif /* GL_ARB_texture_non_power_of_two */

#ifdef GL_ARB_texture_rectangle

#endif /* GL_ARB_texture_rectangle */

#ifdef GL_ARB_texture_rg

#endif /* GL_ARB_texture_rg */

#ifdef GL_ARB_transpose_matrix

static GLboolean _glewInit_GL_ARB_transpose_matrix (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glLoadTransposeMatrixdARB = (PFNGLLOADTRANSPOSEMATRIXDARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadTransposeMatrixdARB"))) == NULL) || r;
  r = ((glLoadTransposeMatrixfARB = (PFNGLLOADTRANSPOSEMATRIXFARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadTransposeMatrixfARB"))) == NULL) || r;
  r = ((glMultTransposeMatrixdARB = (PFNGLMULTTRANSPOSEMATRIXDARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultTransposeMatrixdARB"))) == NULL) || r;
  r = ((glMultTransposeMatrixfARB = (PFNGLMULTTRANSPOSEMATRIXFARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultTransposeMatrixfARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_transpose_matrix */

#ifdef GL_ARB_vertex_array_object

static GLboolean _glewInit_GL_ARB_vertex_array_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindVertexArray"))) == NULL) || r;
  r = ((glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteVertexArrays"))) == NULL) || r;
  r = ((glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenVertexArrays"))) == NULL) || r;
  r = ((glIsVertexArray = (PFNGLISVERTEXARRAYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsVertexArray"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_vertex_array_object */

#ifdef GL_ARB_vertex_blend

static GLboolean _glewInit_GL_ARB_vertex_blend (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glVertexBlendARB = (PFNGLVERTEXBLENDARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexBlendARB"))) == NULL) || r;
  r = ((glWeightPointerARB = (PFNGLWEIGHTPOINTERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightPointerARB"))) == NULL) || r;
  r = ((glWeightbvARB = (PFNGLWEIGHTBVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightbvARB"))) == NULL) || r;
  r = ((glWeightdvARB = (PFNGLWEIGHTDVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightdvARB"))) == NULL) || r;
  r = ((glWeightfvARB = (PFNGLWEIGHTFVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightfvARB"))) == NULL) || r;
  r = ((glWeightivARB = (PFNGLWEIGHTIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightivARB"))) == NULL) || r;
  r = ((glWeightsvARB = (PFNGLWEIGHTSVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightsvARB"))) == NULL) || r;
  r = ((glWeightubvARB = (PFNGLWEIGHTUBVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightubvARB"))) == NULL) || r;
  r = ((glWeightuivARB = (PFNGLWEIGHTUIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightuivARB"))) == NULL) || r;
  r = ((glWeightusvARB = (PFNGLWEIGHTUSVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWeightusvARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_vertex_blend */

#ifdef GL_ARB_vertex_buffer_object

static GLboolean _glewInit_GL_ARB_vertex_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindBufferARB = (PFNGLBINDBUFFERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferARB"))) == NULL) || r;
  r = ((glBufferDataARB = (PFNGLBUFFERDATAARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBufferDataARB"))) == NULL) || r;
  r = ((glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBufferSubDataARB"))) == NULL) || r;
  r = ((glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteBuffersARB"))) == NULL) || r;
  r = ((glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenBuffersARB"))) == NULL) || r;
  r = ((glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBufferParameterivARB"))) == NULL) || r;
  r = ((glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBufferPointervARB"))) == NULL) || r;
  r = ((glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBufferSubDataARB"))) == NULL) || r;
  r = ((glIsBufferARB = (PFNGLISBUFFERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsBufferARB"))) == NULL) || r;
  r = ((glMapBufferARB = (PFNGLMAPBUFFERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapBufferARB"))) == NULL) || r;
  r = ((glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUnmapBufferARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_vertex_buffer_object */

#ifdef GL_ARB_vertex_program

static GLboolean _glewInit_GL_ARB_vertex_program (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindProgramARB"))) == NULL) || r;
  r = ((glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteProgramsARB"))) == NULL) || r;
  r = ((glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDisableVertexAttribArrayARB"))) == NULL) || r;
  r = ((glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnableVertexAttribArrayARB"))) == NULL) || r;
  r = ((glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenProgramsARB"))) == NULL) || r;
  r = ((glGetProgramEnvParameterdvARB = (PFNGLGETPROGRAMENVPARAMETERDVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramEnvParameterdvARB"))) == NULL) || r;
  r = ((glGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramEnvParameterfvARB"))) == NULL) || r;
  r = ((glGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramLocalParameterdvARB"))) == NULL) || r;
  r = ((glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramLocalParameterfvARB"))) == NULL) || r;
  r = ((glGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramStringARB"))) == NULL) || r;
  r = ((glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramivARB"))) == NULL) || r;
  r = ((glGetVertexAttribPointervARB = (PFNGLGETVERTEXATTRIBPOINTERVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribPointervARB"))) == NULL) || r;
  r = ((glGetVertexAttribdvARB = (PFNGLGETVERTEXATTRIBDVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribdvARB"))) == NULL) || r;
  r = ((glGetVertexAttribfvARB = (PFNGLGETVERTEXATTRIBFVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribfvARB"))) == NULL) || r;
  r = ((glGetVertexAttribivARB = (PFNGLGETVERTEXATTRIBIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribivARB"))) == NULL) || r;
  r = ((glIsProgramARB = (PFNGLISPROGRAMARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsProgramARB"))) == NULL) || r;
  r = ((glProgramEnvParameter4dARB = (PFNGLPROGRAMENVPARAMETER4DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameter4dARB"))) == NULL) || r;
  r = ((glProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameter4dvARB"))) == NULL) || r;
  r = ((glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameter4fARB"))) == NULL) || r;
  r = ((glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameter4fvARB"))) == NULL) || r;
  r = ((glProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameter4dARB"))) == NULL) || r;
  r = ((glProgramLocalParameter4dvARB = (PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameter4dvARB"))) == NULL) || r;
  r = ((glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameter4fARB"))) == NULL) || r;
  r = ((glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameter4fvARB"))) == NULL) || r;
  r = ((glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramStringARB"))) == NULL) || r;
  r = ((glVertexAttrib1dARB = (PFNGLVERTEXATTRIB1DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1dARB"))) == NULL) || r;
  r = ((glVertexAttrib1dvARB = (PFNGLVERTEXATTRIB1DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1dvARB"))) == NULL) || r;
  r = ((glVertexAttrib1fARB = (PFNGLVERTEXATTRIB1FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1fARB"))) == NULL) || r;
  r = ((glVertexAttrib1fvARB = (PFNGLVERTEXATTRIB1FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1fvARB"))) == NULL) || r;
  r = ((glVertexAttrib1sARB = (PFNGLVERTEXATTRIB1SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1sARB"))) == NULL) || r;
  r = ((glVertexAttrib1svARB = (PFNGLVERTEXATTRIB1SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1svARB"))) == NULL) || r;
  r = ((glVertexAttrib2dARB = (PFNGLVERTEXATTRIB2DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2dARB"))) == NULL) || r;
  r = ((glVertexAttrib2dvARB = (PFNGLVERTEXATTRIB2DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2dvARB"))) == NULL) || r;
  r = ((glVertexAttrib2fARB = (PFNGLVERTEXATTRIB2FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2fARB"))) == NULL) || r;
  r = ((glVertexAttrib2fvARB = (PFNGLVERTEXATTRIB2FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2fvARB"))) == NULL) || r;
  r = ((glVertexAttrib2sARB = (PFNGLVERTEXATTRIB2SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2sARB"))) == NULL) || r;
  r = ((glVertexAttrib2svARB = (PFNGLVERTEXATTRIB2SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2svARB"))) == NULL) || r;
  r = ((glVertexAttrib3dARB = (PFNGLVERTEXATTRIB3DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3dARB"))) == NULL) || r;
  r = ((glVertexAttrib3dvARB = (PFNGLVERTEXATTRIB3DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3dvARB"))) == NULL) || r;
  r = ((glVertexAttrib3fARB = (PFNGLVERTEXATTRIB3FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3fARB"))) == NULL) || r;
  r = ((glVertexAttrib3fvARB = (PFNGLVERTEXATTRIB3FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3fvARB"))) == NULL) || r;
  r = ((glVertexAttrib3sARB = (PFNGLVERTEXATTRIB3SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3sARB"))) == NULL) || r;
  r = ((glVertexAttrib3svARB = (PFNGLVERTEXATTRIB3SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3svARB"))) == NULL) || r;
  r = ((glVertexAttrib4NbvARB = (PFNGLVERTEXATTRIB4NBVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4NbvARB"))) == NULL) || r;
  r = ((glVertexAttrib4NivARB = (PFNGLVERTEXATTRIB4NIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4NivARB"))) == NULL) || r;
  r = ((glVertexAttrib4NsvARB = (PFNGLVERTEXATTRIB4NSVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4NsvARB"))) == NULL) || r;
  r = ((glVertexAttrib4NubARB = (PFNGLVERTEXATTRIB4NUBARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4NubARB"))) == NULL) || r;
  r = ((glVertexAttrib4NubvARB = (PFNGLVERTEXATTRIB4NUBVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4NubvARB"))) == NULL) || r;
  r = ((glVertexAttrib4NuivARB = (PFNGLVERTEXATTRIB4NUIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4NuivARB"))) == NULL) || r;
  r = ((glVertexAttrib4NusvARB = (PFNGLVERTEXATTRIB4NUSVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4NusvARB"))) == NULL) || r;
  r = ((glVertexAttrib4bvARB = (PFNGLVERTEXATTRIB4BVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4bvARB"))) == NULL) || r;
  r = ((glVertexAttrib4dARB = (PFNGLVERTEXATTRIB4DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4dARB"))) == NULL) || r;
  r = ((glVertexAttrib4dvARB = (PFNGLVERTEXATTRIB4DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4dvARB"))) == NULL) || r;
  r = ((glVertexAttrib4fARB = (PFNGLVERTEXATTRIB4FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4fARB"))) == NULL) || r;
  r = ((glVertexAttrib4fvARB = (PFNGLVERTEXATTRIB4FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4fvARB"))) == NULL) || r;
  r = ((glVertexAttrib4ivARB = (PFNGLVERTEXATTRIB4IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4ivARB"))) == NULL) || r;
  r = ((glVertexAttrib4sARB = (PFNGLVERTEXATTRIB4SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4sARB"))) == NULL) || r;
  r = ((glVertexAttrib4svARB = (PFNGLVERTEXATTRIB4SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4svARB"))) == NULL) || r;
  r = ((glVertexAttrib4ubvARB = (PFNGLVERTEXATTRIB4UBVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4ubvARB"))) == NULL) || r;
  r = ((glVertexAttrib4uivARB = (PFNGLVERTEXATTRIB4UIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4uivARB"))) == NULL) || r;
  r = ((glVertexAttrib4usvARB = (PFNGLVERTEXATTRIB4USVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4usvARB"))) == NULL) || r;
  r = ((glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribPointerARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_vertex_program */

#ifdef GL_ARB_vertex_shader

static GLboolean _glewInit_GL_ARB_vertex_shader (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindAttribLocationARB"))) == NULL) || r;
  r = ((glGetActiveAttribARB = (PFNGLGETACTIVEATTRIBARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetActiveAttribARB"))) == NULL) || r;
  r = ((glGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetAttribLocationARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_vertex_shader */

#ifdef GL_ARB_window_pos

static GLboolean _glewInit_GL_ARB_window_pos (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glWindowPos2dARB = (PFNGLWINDOWPOS2DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2dARB"))) == NULL) || r;
  r = ((glWindowPos2dvARB = (PFNGLWINDOWPOS2DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2dvARB"))) == NULL) || r;
  r = ((glWindowPos2fARB = (PFNGLWINDOWPOS2FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2fARB"))) == NULL) || r;
  r = ((glWindowPos2fvARB = (PFNGLWINDOWPOS2FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2fvARB"))) == NULL) || r;
  r = ((glWindowPos2iARB = (PFNGLWINDOWPOS2IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2iARB"))) == NULL) || r;
  r = ((glWindowPos2ivARB = (PFNGLWINDOWPOS2IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2ivARB"))) == NULL) || r;
  r = ((glWindowPos2sARB = (PFNGLWINDOWPOS2SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2sARB"))) == NULL) || r;
  r = ((glWindowPos2svARB = (PFNGLWINDOWPOS2SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2svARB"))) == NULL) || r;
  r = ((glWindowPos3dARB = (PFNGLWINDOWPOS3DARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3dARB"))) == NULL) || r;
  r = ((glWindowPos3dvARB = (PFNGLWINDOWPOS3DVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3dvARB"))) == NULL) || r;
  r = ((glWindowPos3fARB = (PFNGLWINDOWPOS3FARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3fARB"))) == NULL) || r;
  r = ((glWindowPos3fvARB = (PFNGLWINDOWPOS3FVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3fvARB"))) == NULL) || r;
  r = ((glWindowPos3iARB = (PFNGLWINDOWPOS3IARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3iARB"))) == NULL) || r;
  r = ((glWindowPos3ivARB = (PFNGLWINDOWPOS3IVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3ivARB"))) == NULL) || r;
  r = ((glWindowPos3sARB = (PFNGLWINDOWPOS3SARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3sARB"))) == NULL) || r;
  r = ((glWindowPos3svARB = (PFNGLWINDOWPOS3SVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3svARB"))) == NULL) || r;

  return r;
}

#endif /* GL_ARB_window_pos */

#ifdef GL_ATIX_point_sprites

#endif /* GL_ATIX_point_sprites */

#ifdef GL_ATIX_texture_env_combine3

#endif /* GL_ATIX_texture_env_combine3 */

#ifdef GL_ATIX_texture_env_route

#endif /* GL_ATIX_texture_env_route */

#ifdef GL_ATIX_vertex_shader_output_point_size

#endif /* GL_ATIX_vertex_shader_output_point_size */

#ifdef GL_ATI_draw_buffers

static GLboolean _glewInit_GL_ATI_draw_buffers (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawBuffersATI = (PFNGLDRAWBUFFERSATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawBuffersATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_draw_buffers */

#ifdef GL_ATI_element_array

static GLboolean _glewInit_GL_ATI_element_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawElementArrayATI = (PFNGLDRAWELEMENTARRAYATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawElementArrayATI"))) == NULL) || r;
  r = ((glDrawRangeElementArrayATI = (PFNGLDRAWRANGEELEMENTARRAYATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawRangeElementArrayATI"))) == NULL) || r;
  r = ((glElementPointerATI = (PFNGLELEMENTPOINTERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glElementPointerATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_element_array */

#ifdef GL_ATI_envmap_bumpmap

static GLboolean _glewInit_GL_ATI_envmap_bumpmap (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetTexBumpParameterfvATI = (PFNGLGETTEXBUMPPARAMETERFVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexBumpParameterfvATI"))) == NULL) || r;
  r = ((glGetTexBumpParameterivATI = (PFNGLGETTEXBUMPPARAMETERIVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexBumpParameterivATI"))) == NULL) || r;
  r = ((glTexBumpParameterfvATI = (PFNGLTEXBUMPPARAMETERFVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexBumpParameterfvATI"))) == NULL) || r;
  r = ((glTexBumpParameterivATI = (PFNGLTEXBUMPPARAMETERIVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexBumpParameterivATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_envmap_bumpmap */

#ifdef GL_ATI_fragment_shader

static GLboolean _glewInit_GL_ATI_fragment_shader (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAlphaFragmentOp1ATI = (PFNGLALPHAFRAGMENTOP1ATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAlphaFragmentOp1ATI"))) == NULL) || r;
  r = ((glAlphaFragmentOp2ATI = (PFNGLALPHAFRAGMENTOP2ATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAlphaFragmentOp2ATI"))) == NULL) || r;
  r = ((glAlphaFragmentOp3ATI = (PFNGLALPHAFRAGMENTOP3ATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAlphaFragmentOp3ATI"))) == NULL) || r;
  r = ((glBeginFragmentShaderATI = (PFNGLBEGINFRAGMENTSHADERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginFragmentShaderATI"))) == NULL) || r;
  r = ((glBindFragmentShaderATI = (PFNGLBINDFRAGMENTSHADERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindFragmentShaderATI"))) == NULL) || r;
  r = ((glColorFragmentOp1ATI = (PFNGLCOLORFRAGMENTOP1ATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorFragmentOp1ATI"))) == NULL) || r;
  r = ((glColorFragmentOp2ATI = (PFNGLCOLORFRAGMENTOP2ATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorFragmentOp2ATI"))) == NULL) || r;
  r = ((glColorFragmentOp3ATI = (PFNGLCOLORFRAGMENTOP3ATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorFragmentOp3ATI"))) == NULL) || r;
  r = ((glDeleteFragmentShaderATI = (PFNGLDELETEFRAGMENTSHADERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteFragmentShaderATI"))) == NULL) || r;
  r = ((glEndFragmentShaderATI = (PFNGLENDFRAGMENTSHADERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndFragmentShaderATI"))) == NULL) || r;
  r = ((glGenFragmentShadersATI = (PFNGLGENFRAGMENTSHADERSATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenFragmentShadersATI"))) == NULL) || r;
  r = ((glPassTexCoordATI = (PFNGLPASSTEXCOORDATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPassTexCoordATI"))) == NULL) || r;
  r = ((glSampleMapATI = (PFNGLSAMPLEMAPATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSampleMapATI"))) == NULL) || r;
  r = ((glSetFragmentShaderConstantATI = (PFNGLSETFRAGMENTSHADERCONSTANTATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSetFragmentShaderConstantATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_fragment_shader */

#ifdef GL_ATI_map_object_buffer

static GLboolean _glewInit_GL_ATI_map_object_buffer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glMapObjectBufferATI = (PFNGLMAPOBJECTBUFFERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapObjectBufferATI"))) == NULL) || r;
  r = ((glUnmapObjectBufferATI = (PFNGLUNMAPOBJECTBUFFERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUnmapObjectBufferATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_map_object_buffer */

#ifdef GL_ATI_pn_triangles

static GLboolean _glewInit_GL_ATI_pn_triangles (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPNTrianglesfATI = (PFNGLPNTRIANGLESFATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPNTrianglesfATI"))) == NULL) || r;
  r = ((glPNTrianglesiATI = (PFNGLPNTRIANGLESIATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPNTrianglesiATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_pn_triangles */

#ifdef GL_ATI_separate_stencil

static GLboolean _glewInit_GL_ATI_separate_stencil (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glStencilFuncSeparateATI = (PFNGLSTENCILFUNCSEPARATEATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStencilFuncSeparateATI"))) == NULL) || r;
  r = ((glStencilOpSeparateATI = (PFNGLSTENCILOPSEPARATEATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStencilOpSeparateATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_separate_stencil */

#ifdef GL_ATI_shader_texture_lod

#endif /* GL_ATI_shader_texture_lod */

#ifdef GL_ATI_text_fragment_shader

#endif /* GL_ATI_text_fragment_shader */

#ifdef GL_ATI_texture_compression_3dc

#endif /* GL_ATI_texture_compression_3dc */

#ifdef GL_ATI_texture_float

#endif /* GL_ATI_texture_float */

#ifdef GL_ATI_texture_mirror_once

#endif /* GL_ATI_texture_mirror_once */

#ifdef GL_ATI_vertex_array_object

static GLboolean _glewInit_GL_ATI_vertex_array_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glArrayObjectATI = (PFNGLARRAYOBJECTATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glArrayObjectATI"))) == NULL) || r;
  r = ((glFreeObjectBufferATI = (PFNGLFREEOBJECTBUFFERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFreeObjectBufferATI"))) == NULL) || r;
  r = ((glGetArrayObjectfvATI = (PFNGLGETARRAYOBJECTFVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetArrayObjectfvATI"))) == NULL) || r;
  r = ((glGetArrayObjectivATI = (PFNGLGETARRAYOBJECTIVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetArrayObjectivATI"))) == NULL) || r;
  r = ((glGetObjectBufferfvATI = (PFNGLGETOBJECTBUFFERFVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetObjectBufferfvATI"))) == NULL) || r;
  r = ((glGetObjectBufferivATI = (PFNGLGETOBJECTBUFFERIVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetObjectBufferivATI"))) == NULL) || r;
  r = ((glGetVariantArrayObjectfvATI = (PFNGLGETVARIANTARRAYOBJECTFVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVariantArrayObjectfvATI"))) == NULL) || r;
  r = ((glGetVariantArrayObjectivATI = (PFNGLGETVARIANTARRAYOBJECTIVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVariantArrayObjectivATI"))) == NULL) || r;
  r = ((glIsObjectBufferATI = (PFNGLISOBJECTBUFFERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsObjectBufferATI"))) == NULL) || r;
  r = ((glNewObjectBufferATI = (PFNGLNEWOBJECTBUFFERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNewObjectBufferATI"))) == NULL) || r;
  r = ((glUpdateObjectBufferATI = (PFNGLUPDATEOBJECTBUFFERATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUpdateObjectBufferATI"))) == NULL) || r;
  r = ((glVariantArrayObjectATI = (PFNGLVARIANTARRAYOBJECTATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantArrayObjectATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_vertex_array_object */

#ifdef GL_ATI_vertex_attrib_array_object

static GLboolean _glewInit_GL_ATI_vertex_attrib_array_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetVertexAttribArrayObjectfvATI = (PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribArrayObjectfvATI"))) == NULL) || r;
  r = ((glGetVertexAttribArrayObjectivATI = (PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribArrayObjectivATI"))) == NULL) || r;
  r = ((glVertexAttribArrayObjectATI = (PFNGLVERTEXATTRIBARRAYOBJECTATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribArrayObjectATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_vertex_attrib_array_object */

#ifdef GL_ATI_vertex_streams

static GLboolean _glewInit_GL_ATI_vertex_streams (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClientActiveVertexStreamATI = (PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClientActiveVertexStreamATI"))) == NULL) || r;
  r = ((glNormalStream3bATI = (PFNGLNORMALSTREAM3BATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3bATI"))) == NULL) || r;
  r = ((glNormalStream3bvATI = (PFNGLNORMALSTREAM3BVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3bvATI"))) == NULL) || r;
  r = ((glNormalStream3dATI = (PFNGLNORMALSTREAM3DATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3dATI"))) == NULL) || r;
  r = ((glNormalStream3dvATI = (PFNGLNORMALSTREAM3DVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3dvATI"))) == NULL) || r;
  r = ((glNormalStream3fATI = (PFNGLNORMALSTREAM3FATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3fATI"))) == NULL) || r;
  r = ((glNormalStream3fvATI = (PFNGLNORMALSTREAM3FVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3fvATI"))) == NULL) || r;
  r = ((glNormalStream3iATI = (PFNGLNORMALSTREAM3IATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3iATI"))) == NULL) || r;
  r = ((glNormalStream3ivATI = (PFNGLNORMALSTREAM3IVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3ivATI"))) == NULL) || r;
  r = ((glNormalStream3sATI = (PFNGLNORMALSTREAM3SATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3sATI"))) == NULL) || r;
  r = ((glNormalStream3svATI = (PFNGLNORMALSTREAM3SVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalStream3svATI"))) == NULL) || r;
  r = ((glVertexBlendEnvfATI = (PFNGLVERTEXBLENDENVFATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexBlendEnvfATI"))) == NULL) || r;
  r = ((glVertexBlendEnviATI = (PFNGLVERTEXBLENDENVIATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexBlendEnviATI"))) == NULL) || r;
  r = ((glVertexStream2dATI = (PFNGLVERTEXSTREAM2DATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream2dATI"))) == NULL) || r;
  r = ((glVertexStream2dvATI = (PFNGLVERTEXSTREAM2DVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream2dvATI"))) == NULL) || r;
  r = ((glVertexStream2fATI = (PFNGLVERTEXSTREAM2FATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream2fATI"))) == NULL) || r;
  r = ((glVertexStream2fvATI = (PFNGLVERTEXSTREAM2FVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream2fvATI"))) == NULL) || r;
  r = ((glVertexStream2iATI = (PFNGLVERTEXSTREAM2IATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream2iATI"))) == NULL) || r;
  r = ((glVertexStream2ivATI = (PFNGLVERTEXSTREAM2IVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream2ivATI"))) == NULL) || r;
  r = ((glVertexStream2sATI = (PFNGLVERTEXSTREAM2SATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream2sATI"))) == NULL) || r;
  r = ((glVertexStream2svATI = (PFNGLVERTEXSTREAM2SVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream2svATI"))) == NULL) || r;
  r = ((glVertexStream3dATI = (PFNGLVERTEXSTREAM3DATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream3dATI"))) == NULL) || r;
  r = ((glVertexStream3dvATI = (PFNGLVERTEXSTREAM3DVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream3dvATI"))) == NULL) || r;
  r = ((glVertexStream3fATI = (PFNGLVERTEXSTREAM3FATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream3fATI"))) == NULL) || r;
  r = ((glVertexStream3fvATI = (PFNGLVERTEXSTREAM3FVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream3fvATI"))) == NULL) || r;
  r = ((glVertexStream3iATI = (PFNGLVERTEXSTREAM3IATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream3iATI"))) == NULL) || r;
  r = ((glVertexStream3ivATI = (PFNGLVERTEXSTREAM3IVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream3ivATI"))) == NULL) || r;
  r = ((glVertexStream3sATI = (PFNGLVERTEXSTREAM3SATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream3sATI"))) == NULL) || r;
  r = ((glVertexStream3svATI = (PFNGLVERTEXSTREAM3SVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream3svATI"))) == NULL) || r;
  r = ((glVertexStream4dATI = (PFNGLVERTEXSTREAM4DATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream4dATI"))) == NULL) || r;
  r = ((glVertexStream4dvATI = (PFNGLVERTEXSTREAM4DVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream4dvATI"))) == NULL) || r;
  r = ((glVertexStream4fATI = (PFNGLVERTEXSTREAM4FATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream4fATI"))) == NULL) || r;
  r = ((glVertexStream4fvATI = (PFNGLVERTEXSTREAM4FVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream4fvATI"))) == NULL) || r;
  r = ((glVertexStream4iATI = (PFNGLVERTEXSTREAM4IATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream4iATI"))) == NULL) || r;
  r = ((glVertexStream4ivATI = (PFNGLVERTEXSTREAM4IVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream4ivATI"))) == NULL) || r;
  r = ((glVertexStream4sATI = (PFNGLVERTEXSTREAM4SATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream4sATI"))) == NULL) || r;
  r = ((glVertexStream4svATI = (PFNGLVERTEXSTREAM4SVATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexStream4svATI"))) == NULL) || r;

  return r;
}

#endif /* GL_ATI_vertex_streams */

#ifdef GL_EXT_422_pixels

#endif /* GL_EXT_422_pixels */

#ifdef GL_EXT_Cg_shader

#endif /* GL_EXT_Cg_shader */

#ifdef GL_EXT_abgr

#endif /* GL_EXT_abgr */

#ifdef GL_EXT_bgra

#endif /* GL_EXT_bgra */

#ifdef GL_EXT_bindable_uniform

static GLboolean _glewInit_GL_EXT_bindable_uniform (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetUniformBufferSizeEXT = (PFNGLGETUNIFORMBUFFERSIZEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformBufferSizeEXT"))) == NULL) || r;
  r = ((glGetUniformOffsetEXT = (PFNGLGETUNIFORMOFFSETEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformOffsetEXT"))) == NULL) || r;
  r = ((glUniformBufferEXT = (PFNGLUNIFORMBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniformBufferEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_bindable_uniform */

#ifdef GL_EXT_blend_color

static GLboolean _glewInit_GL_EXT_blend_color (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendColorEXT = (PFNGLBLENDCOLOREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendColorEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_blend_color */

#ifdef GL_EXT_blend_equation_separate

static GLboolean _glewInit_GL_EXT_blend_equation_separate (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquationSeparateEXT = (PFNGLBLENDEQUATIONSEPARATEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendEquationSeparateEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_blend_equation_separate */

#ifdef GL_EXT_blend_func_separate

static GLboolean _glewInit_GL_EXT_blend_func_separate (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendFuncSeparateEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_blend_func_separate */

#ifdef GL_EXT_blend_logic_op

#endif /* GL_EXT_blend_logic_op */

#ifdef GL_EXT_blend_minmax

static GLboolean _glewInit_GL_EXT_blend_minmax (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquationEXT = (PFNGLBLENDEQUATIONEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlendEquationEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_blend_minmax */

#ifdef GL_EXT_blend_subtract

#endif /* GL_EXT_blend_subtract */

#ifdef GL_EXT_clip_volume_hint

#endif /* GL_EXT_clip_volume_hint */

#ifdef GL_EXT_cmyka

#endif /* GL_EXT_cmyka */

#ifdef GL_EXT_color_subtable

static GLboolean _glewInit_GL_EXT_color_subtable (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorSubTableEXT = (PFNGLCOLORSUBTABLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorSubTableEXT"))) == NULL) || r;
  r = ((glCopyColorSubTableEXT = (PFNGLCOPYCOLORSUBTABLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyColorSubTableEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_color_subtable */

#ifdef GL_EXT_compiled_vertex_array

static GLboolean _glewInit_GL_EXT_compiled_vertex_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glLockArraysEXT = (PFNGLLOCKARRAYSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLockArraysEXT"))) == NULL) || r;
  r = ((glUnlockArraysEXT = (PFNGLUNLOCKARRAYSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUnlockArraysEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_compiled_vertex_array */

#ifdef GL_EXT_convolution

static GLboolean _glewInit_GL_EXT_convolution (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glConvolutionFilter1DEXT = (PFNGLCONVOLUTIONFILTER1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionFilter1DEXT"))) == NULL) || r;
  r = ((glConvolutionFilter2DEXT = (PFNGLCONVOLUTIONFILTER2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionFilter2DEXT"))) == NULL) || r;
  r = ((glConvolutionParameterfEXT = (PFNGLCONVOLUTIONPARAMETERFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionParameterfEXT"))) == NULL) || r;
  r = ((glConvolutionParameterfvEXT = (PFNGLCONVOLUTIONPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionParameterfvEXT"))) == NULL) || r;
  r = ((glConvolutionParameteriEXT = (PFNGLCONVOLUTIONPARAMETERIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionParameteriEXT"))) == NULL) || r;
  r = ((glConvolutionParameterivEXT = (PFNGLCONVOLUTIONPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glConvolutionParameterivEXT"))) == NULL) || r;
  r = ((glCopyConvolutionFilter1DEXT = (PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyConvolutionFilter1DEXT"))) == NULL) || r;
  r = ((glCopyConvolutionFilter2DEXT = (PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyConvolutionFilter2DEXT"))) == NULL) || r;
  r = ((glGetConvolutionFilterEXT = (PFNGLGETCONVOLUTIONFILTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetConvolutionFilterEXT"))) == NULL) || r;
  r = ((glGetConvolutionParameterfvEXT = (PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetConvolutionParameterfvEXT"))) == NULL) || r;
  r = ((glGetConvolutionParameterivEXT = (PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetConvolutionParameterivEXT"))) == NULL) || r;
  r = ((glGetSeparableFilterEXT = (PFNGLGETSEPARABLEFILTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetSeparableFilterEXT"))) == NULL) || r;
  r = ((glSeparableFilter2DEXT = (PFNGLSEPARABLEFILTER2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSeparableFilter2DEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_convolution */

#ifdef GL_EXT_coordinate_frame

static GLboolean _glewInit_GL_EXT_coordinate_frame (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBinormalPointerEXT = (PFNGLBINORMALPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBinormalPointerEXT"))) == NULL) || r;
  r = ((glTangentPointerEXT = (PFNGLTANGENTPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTangentPointerEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_coordinate_frame */

#ifdef GL_EXT_copy_texture

static GLboolean _glewInit_GL_EXT_copy_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyTexImage1DEXT = (PFNGLCOPYTEXIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexImage1DEXT"))) == NULL) || r;
  r = ((glCopyTexImage2DEXT = (PFNGLCOPYTEXIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexImage2DEXT"))) == NULL) || r;
  r = ((glCopyTexSubImage1DEXT = (PFNGLCOPYTEXSUBIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexSubImage1DEXT"))) == NULL) || r;
  r = ((glCopyTexSubImage2DEXT = (PFNGLCOPYTEXSUBIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexSubImage2DEXT"))) == NULL) || r;
  r = ((glCopyTexSubImage3DEXT = (PFNGLCOPYTEXSUBIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTexSubImage3DEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_copy_texture */

#ifdef GL_EXT_cull_vertex

static GLboolean _glewInit_GL_EXT_cull_vertex (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCullParameterdvEXT = (PFNGLCULLPARAMETERDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCullParameterdvEXT"))) == NULL) || r;
  r = ((glCullParameterfvEXT = (PFNGLCULLPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCullParameterfvEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_cull_vertex */

#ifdef GL_EXT_depth_bounds_test

static GLboolean _glewInit_GL_EXT_depth_bounds_test (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDepthBoundsEXT = (PFNGLDEPTHBOUNDSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDepthBoundsEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_depth_bounds_test */

#ifdef GL_EXT_direct_state_access

static GLboolean _glewInit_GL_EXT_direct_state_access (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindMultiTextureEXT = (PFNGLBINDMULTITEXTUREEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindMultiTextureEXT"))) == NULL) || r;
  r = ((glCheckNamedFramebufferStatusEXT = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCheckNamedFramebufferStatusEXT"))) == NULL) || r;
  r = ((glClientAttribDefaultEXT = (PFNGLCLIENTATTRIBDEFAULTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClientAttribDefaultEXT"))) == NULL) || r;
  r = ((glCompressedMultiTexImage1DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedMultiTexImage1DEXT"))) == NULL) || r;
  r = ((glCompressedMultiTexImage2DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedMultiTexImage2DEXT"))) == NULL) || r;
  r = ((glCompressedMultiTexImage3DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedMultiTexImage3DEXT"))) == NULL) || r;
  r = ((glCompressedMultiTexSubImage1DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedMultiTexSubImage1DEXT"))) == NULL) || r;
  r = ((glCompressedMultiTexSubImage2DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedMultiTexSubImage2DEXT"))) == NULL) || r;
  r = ((glCompressedMultiTexSubImage3DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedMultiTexSubImage3DEXT"))) == NULL) || r;
  r = ((glCompressedTextureImage1DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTextureImage1DEXT"))) == NULL) || r;
  r = ((glCompressedTextureImage2DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTextureImage2DEXT"))) == NULL) || r;
  r = ((glCompressedTextureImage3DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTextureImage3DEXT"))) == NULL) || r;
  r = ((glCompressedTextureSubImage1DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTextureSubImage1DEXT"))) == NULL) || r;
  r = ((glCompressedTextureSubImage2DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTextureSubImage2DEXT"))) == NULL) || r;
  r = ((glCompressedTextureSubImage3DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCompressedTextureSubImage3DEXT"))) == NULL) || r;
  r = ((glCopyMultiTexImage1DEXT = (PFNGLCOPYMULTITEXIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyMultiTexImage1DEXT"))) == NULL) || r;
  r = ((glCopyMultiTexImage2DEXT = (PFNGLCOPYMULTITEXIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyMultiTexImage2DEXT"))) == NULL) || r;
  r = ((glCopyMultiTexSubImage1DEXT = (PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyMultiTexSubImage1DEXT"))) == NULL) || r;
  r = ((glCopyMultiTexSubImage2DEXT = (PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyMultiTexSubImage2DEXT"))) == NULL) || r;
  r = ((glCopyMultiTexSubImage3DEXT = (PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyMultiTexSubImage3DEXT"))) == NULL) || r;
  r = ((glCopyTextureImage1DEXT = (PFNGLCOPYTEXTUREIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTextureImage1DEXT"))) == NULL) || r;
  r = ((glCopyTextureImage2DEXT = (PFNGLCOPYTEXTUREIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTextureImage2DEXT"))) == NULL) || r;
  r = ((glCopyTextureSubImage1DEXT = (PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTextureSubImage1DEXT"))) == NULL) || r;
  r = ((glCopyTextureSubImage2DEXT = (PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTextureSubImage2DEXT"))) == NULL) || r;
  r = ((glCopyTextureSubImage3DEXT = (PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyTextureSubImage3DEXT"))) == NULL) || r;
  r = ((glDisableClientStateIndexedEXT = (PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDisableClientStateIndexedEXT"))) == NULL) || r;
  r = ((glEnableClientStateIndexedEXT = (PFNGLENABLECLIENTSTATEINDEXEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnableClientStateIndexedEXT"))) == NULL) || r;
  r = ((glFramebufferDrawBufferEXT = (PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferDrawBufferEXT"))) == NULL) || r;
  r = ((glFramebufferDrawBuffersEXT = (PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferDrawBuffersEXT"))) == NULL) || r;
  r = ((glFramebufferReadBufferEXT = (PFNGLFRAMEBUFFERREADBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferReadBufferEXT"))) == NULL) || r;
  r = ((glGenerateMultiTexMipmapEXT = (PFNGLGENERATEMULTITEXMIPMAPEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenerateMultiTexMipmapEXT"))) == NULL) || r;
  r = ((glGenerateTextureMipmapEXT = (PFNGLGENERATETEXTUREMIPMAPEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenerateTextureMipmapEXT"))) == NULL) || r;
  r = ((glGetCompressedMultiTexImageEXT = (PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCompressedMultiTexImageEXT"))) == NULL) || r;
  r = ((glGetCompressedTextureImageEXT = (PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCompressedTextureImageEXT"))) == NULL) || r;
  r = ((glGetDoubleIndexedvEXT = (PFNGLGETDOUBLEINDEXEDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetDoubleIndexedvEXT"))) == NULL) || r;
  r = ((glGetFloatIndexedvEXT = (PFNGLGETFLOATINDEXEDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFloatIndexedvEXT"))) == NULL) || r;
  r = ((glGetFramebufferParameterivEXT = (PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFramebufferParameterivEXT"))) == NULL) || r;
  r = ((glGetMultiTexEnvfvEXT = (PFNGLGETMULTITEXENVFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexEnvfvEXT"))) == NULL) || r;
  r = ((glGetMultiTexEnvivEXT = (PFNGLGETMULTITEXENVIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexEnvivEXT"))) == NULL) || r;
  r = ((glGetMultiTexGendvEXT = (PFNGLGETMULTITEXGENDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexGendvEXT"))) == NULL) || r;
  r = ((glGetMultiTexGenfvEXT = (PFNGLGETMULTITEXGENFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexGenfvEXT"))) == NULL) || r;
  r = ((glGetMultiTexGenivEXT = (PFNGLGETMULTITEXGENIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexGenivEXT"))) == NULL) || r;
  r = ((glGetMultiTexImageEXT = (PFNGLGETMULTITEXIMAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexImageEXT"))) == NULL) || r;
  r = ((glGetMultiTexLevelParameterfvEXT = (PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexLevelParameterfvEXT"))) == NULL) || r;
  r = ((glGetMultiTexLevelParameterivEXT = (PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexLevelParameterivEXT"))) == NULL) || r;
  r = ((glGetMultiTexParameterIivEXT = (PFNGLGETMULTITEXPARAMETERIIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexParameterIivEXT"))) == NULL) || r;
  r = ((glGetMultiTexParameterIuivEXT = (PFNGLGETMULTITEXPARAMETERIUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexParameterIuivEXT"))) == NULL) || r;
  r = ((glGetMultiTexParameterfvEXT = (PFNGLGETMULTITEXPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexParameterfvEXT"))) == NULL) || r;
  r = ((glGetMultiTexParameterivEXT = (PFNGLGETMULTITEXPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultiTexParameterivEXT"))) == NULL) || r;
  r = ((glGetNamedBufferParameterivEXT = (PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedBufferParameterivEXT"))) == NULL) || r;
  r = ((glGetNamedBufferPointervEXT = (PFNGLGETNAMEDBUFFERPOINTERVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedBufferPointervEXT"))) == NULL) || r;
  r = ((glGetNamedBufferSubDataEXT = (PFNGLGETNAMEDBUFFERSUBDATAEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedBufferSubDataEXT"))) == NULL) || r;
  r = ((glGetNamedFramebufferAttachmentParameterivEXT = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedFramebufferAttachmentParameterivEXT"))) == NULL) || r;
  r = ((glGetNamedProgramLocalParameterIivEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedProgramLocalParameterIivEXT"))) == NULL) || r;
  r = ((glGetNamedProgramLocalParameterIuivEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedProgramLocalParameterIuivEXT"))) == NULL) || r;
  r = ((glGetNamedProgramLocalParameterdvEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedProgramLocalParameterdvEXT"))) == NULL) || r;
  r = ((glGetNamedProgramLocalParameterfvEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedProgramLocalParameterfvEXT"))) == NULL) || r;
  r = ((glGetNamedProgramStringEXT = (PFNGLGETNAMEDPROGRAMSTRINGEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedProgramStringEXT"))) == NULL) || r;
  r = ((glGetNamedProgramivEXT = (PFNGLGETNAMEDPROGRAMIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedProgramivEXT"))) == NULL) || r;
  r = ((glGetNamedRenderbufferParameterivEXT = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetNamedRenderbufferParameterivEXT"))) == NULL) || r;
  r = ((glGetPointerIndexedvEXT = (PFNGLGETPOINTERINDEXEDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPointerIndexedvEXT"))) == NULL) || r;
  r = ((glGetTextureImageEXT = (PFNGLGETTEXTUREIMAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTextureImageEXT"))) == NULL) || r;
  r = ((glGetTextureLevelParameterfvEXT = (PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTextureLevelParameterfvEXT"))) == NULL) || r;
  r = ((glGetTextureLevelParameterivEXT = (PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTextureLevelParameterivEXT"))) == NULL) || r;
  r = ((glGetTextureParameterIivEXT = (PFNGLGETTEXTUREPARAMETERIIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTextureParameterIivEXT"))) == NULL) || r;
  r = ((glGetTextureParameterIuivEXT = (PFNGLGETTEXTUREPARAMETERIUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTextureParameterIuivEXT"))) == NULL) || r;
  r = ((glGetTextureParameterfvEXT = (PFNGLGETTEXTUREPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTextureParameterfvEXT"))) == NULL) || r;
  r = ((glGetTextureParameterivEXT = (PFNGLGETTEXTUREPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTextureParameterivEXT"))) == NULL) || r;
  r = ((glMapNamedBufferEXT = (PFNGLMAPNAMEDBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapNamedBufferEXT"))) == NULL) || r;
  r = ((glMatrixFrustumEXT = (PFNGLMATRIXFRUSTUMEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixFrustumEXT"))) == NULL) || r;
  r = ((glMatrixLoadIdentityEXT = (PFNGLMATRIXLOADIDENTITYEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixLoadIdentityEXT"))) == NULL) || r;
  r = ((glMatrixLoadTransposedEXT = (PFNGLMATRIXLOADTRANSPOSEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixLoadTransposedEXT"))) == NULL) || r;
  r = ((glMatrixLoadTransposefEXT = (PFNGLMATRIXLOADTRANSPOSEFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixLoadTransposefEXT"))) == NULL) || r;
  r = ((glMatrixLoaddEXT = (PFNGLMATRIXLOADDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixLoaddEXT"))) == NULL) || r;
  r = ((glMatrixLoadfEXT = (PFNGLMATRIXLOADFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixLoadfEXT"))) == NULL) || r;
  r = ((glMatrixMultTransposedEXT = (PFNGLMATRIXMULTTRANSPOSEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixMultTransposedEXT"))) == NULL) || r;
  r = ((glMatrixMultTransposefEXT = (PFNGLMATRIXMULTTRANSPOSEFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixMultTransposefEXT"))) == NULL) || r;
  r = ((glMatrixMultdEXT = (PFNGLMATRIXMULTDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixMultdEXT"))) == NULL) || r;
  r = ((glMatrixMultfEXT = (PFNGLMATRIXMULTFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixMultfEXT"))) == NULL) || r;
  r = ((glMatrixOrthoEXT = (PFNGLMATRIXORTHOEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixOrthoEXT"))) == NULL) || r;
  r = ((glMatrixPopEXT = (PFNGLMATRIXPOPEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixPopEXT"))) == NULL) || r;
  r = ((glMatrixPushEXT = (PFNGLMATRIXPUSHEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixPushEXT"))) == NULL) || r;
  r = ((glMatrixRotatedEXT = (PFNGLMATRIXROTATEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixRotatedEXT"))) == NULL) || r;
  r = ((glMatrixRotatefEXT = (PFNGLMATRIXROTATEFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixRotatefEXT"))) == NULL) || r;
  r = ((glMatrixScaledEXT = (PFNGLMATRIXSCALEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixScaledEXT"))) == NULL) || r;
  r = ((glMatrixScalefEXT = (PFNGLMATRIXSCALEFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixScalefEXT"))) == NULL) || r;
  r = ((glMatrixTranslatedEXT = (PFNGLMATRIXTRANSLATEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixTranslatedEXT"))) == NULL) || r;
  r = ((glMatrixTranslatefEXT = (PFNGLMATRIXTRANSLATEFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMatrixTranslatefEXT"))) == NULL) || r;
  r = ((glMultiTexBufferEXT = (PFNGLMULTITEXBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexBufferEXT"))) == NULL) || r;
  r = ((glMultiTexCoordPointerEXT = (PFNGLMULTITEXCOORDPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoordPointerEXT"))) == NULL) || r;
  r = ((glMultiTexEnvfEXT = (PFNGLMULTITEXENVFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexEnvfEXT"))) == NULL) || r;
  r = ((glMultiTexEnvfvEXT = (PFNGLMULTITEXENVFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexEnvfvEXT"))) == NULL) || r;
  r = ((glMultiTexEnviEXT = (PFNGLMULTITEXENVIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexEnviEXT"))) == NULL) || r;
  r = ((glMultiTexEnvivEXT = (PFNGLMULTITEXENVIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexEnvivEXT"))) == NULL) || r;
  r = ((glMultiTexGendEXT = (PFNGLMULTITEXGENDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexGendEXT"))) == NULL) || r;
  r = ((glMultiTexGendvEXT = (PFNGLMULTITEXGENDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexGendvEXT"))) == NULL) || r;
  r = ((glMultiTexGenfEXT = (PFNGLMULTITEXGENFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexGenfEXT"))) == NULL) || r;
  r = ((glMultiTexGenfvEXT = (PFNGLMULTITEXGENFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexGenfvEXT"))) == NULL) || r;
  r = ((glMultiTexGeniEXT = (PFNGLMULTITEXGENIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexGeniEXT"))) == NULL) || r;
  r = ((glMultiTexGenivEXT = (PFNGLMULTITEXGENIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexGenivEXT"))) == NULL) || r;
  r = ((glMultiTexImage1DEXT = (PFNGLMULTITEXIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexImage1DEXT"))) == NULL) || r;
  r = ((glMultiTexImage2DEXT = (PFNGLMULTITEXIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexImage2DEXT"))) == NULL) || r;
  r = ((glMultiTexImage3DEXT = (PFNGLMULTITEXIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexImage3DEXT"))) == NULL) || r;
  r = ((glMultiTexParameterIivEXT = (PFNGLMULTITEXPARAMETERIIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexParameterIivEXT"))) == NULL) || r;
  r = ((glMultiTexParameterIuivEXT = (PFNGLMULTITEXPARAMETERIUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexParameterIuivEXT"))) == NULL) || r;
  r = ((glMultiTexParameterfEXT = (PFNGLMULTITEXPARAMETERFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexParameterfEXT"))) == NULL) || r;
  r = ((glMultiTexParameterfvEXT = (PFNGLMULTITEXPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexParameterfvEXT"))) == NULL) || r;
  r = ((glMultiTexParameteriEXT = (PFNGLMULTITEXPARAMETERIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexParameteriEXT"))) == NULL) || r;
  r = ((glMultiTexParameterivEXT = (PFNGLMULTITEXPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexParameterivEXT"))) == NULL) || r;
  r = ((glMultiTexRenderbufferEXT = (PFNGLMULTITEXRENDERBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexRenderbufferEXT"))) == NULL) || r;
  r = ((glMultiTexSubImage1DEXT = (PFNGLMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexSubImage1DEXT"))) == NULL) || r;
  r = ((glMultiTexSubImage2DEXT = (PFNGLMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexSubImage2DEXT"))) == NULL) || r;
  r = ((glMultiTexSubImage3DEXT = (PFNGLMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexSubImage3DEXT"))) == NULL) || r;
  r = ((glNamedBufferDataEXT = (PFNGLNAMEDBUFFERDATAEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedBufferDataEXT"))) == NULL) || r;
  r = ((glNamedBufferSubDataEXT = (PFNGLNAMEDBUFFERSUBDATAEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedBufferSubDataEXT"))) == NULL) || r;
  r = ((glNamedFramebufferRenderbufferEXT = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedFramebufferRenderbufferEXT"))) == NULL) || r;
  r = ((glNamedFramebufferTexture1DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedFramebufferTexture1DEXT"))) == NULL) || r;
  r = ((glNamedFramebufferTexture2DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedFramebufferTexture2DEXT"))) == NULL) || r;
  r = ((glNamedFramebufferTexture3DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedFramebufferTexture3DEXT"))) == NULL) || r;
  r = ((glNamedFramebufferTextureEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedFramebufferTextureEXT"))) == NULL) || r;
  r = ((glNamedFramebufferTextureFaceEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedFramebufferTextureFaceEXT"))) == NULL) || r;
  r = ((glNamedFramebufferTextureLayerEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedFramebufferTextureLayerEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameter4dEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameter4dEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameter4dvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameter4dvEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameter4fEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameter4fEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameter4fvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameter4fvEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameterI4iEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameterI4iEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameterI4ivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameterI4ivEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameterI4uiEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameterI4uiEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameterI4uivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameterI4uivEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParameters4fvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParameters4fvEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParametersI4ivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParametersI4ivEXT"))) == NULL) || r;
  r = ((glNamedProgramLocalParametersI4uivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramLocalParametersI4uivEXT"))) == NULL) || r;
  r = ((glNamedProgramStringEXT = (PFNGLNAMEDPROGRAMSTRINGEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedProgramStringEXT"))) == NULL) || r;
  r = ((glNamedRenderbufferStorageEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedRenderbufferStorageEXT"))) == NULL) || r;
  r = ((glNamedRenderbufferStorageMultisampleCoverageEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedRenderbufferStorageMultisampleCoverageEXT"))) == NULL) || r;
  r = ((glNamedRenderbufferStorageMultisampleEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNamedRenderbufferStorageMultisampleEXT"))) == NULL) || r;
  r = ((glProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform1fEXT"))) == NULL) || r;
  r = ((glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform1fvEXT"))) == NULL) || r;
  r = ((glProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform1iEXT"))) == NULL) || r;
  r = ((glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform1ivEXT"))) == NULL) || r;
  r = ((glProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform1uiEXT"))) == NULL) || r;
  r = ((glProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform1uivEXT"))) == NULL) || r;
  r = ((glProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform2fEXT"))) == NULL) || r;
  r = ((glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform2fvEXT"))) == NULL) || r;
  r = ((glProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform2iEXT"))) == NULL) || r;
  r = ((glProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform2ivEXT"))) == NULL) || r;
  r = ((glProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform2uiEXT"))) == NULL) || r;
  r = ((glProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform2uivEXT"))) == NULL) || r;
  r = ((glProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform3fEXT"))) == NULL) || r;
  r = ((glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform3fvEXT"))) == NULL) || r;
  r = ((glProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform3iEXT"))) == NULL) || r;
  r = ((glProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform3ivEXT"))) == NULL) || r;
  r = ((glProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform3uiEXT"))) == NULL) || r;
  r = ((glProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform3uivEXT"))) == NULL) || r;
  r = ((glProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform4fEXT"))) == NULL) || r;
  r = ((glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform4fvEXT"))) == NULL) || r;
  r = ((glProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform4iEXT"))) == NULL) || r;
  r = ((glProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform4ivEXT"))) == NULL) || r;
  r = ((glProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform4uiEXT"))) == NULL) || r;
  r = ((glProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniform4uivEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix2fvEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix2x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix2x3fvEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix2x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix2x4fvEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix3fvEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix3x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix3x2fvEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix3x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix3x4fvEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix4fvEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix4x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix4x2fvEXT"))) == NULL) || r;
  r = ((glProgramUniformMatrix4x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramUniformMatrix4x3fvEXT"))) == NULL) || r;
  r = ((glPushClientAttribDefaultEXT = (PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPushClientAttribDefaultEXT"))) == NULL) || r;
  r = ((glTextureBufferEXT = (PFNGLTEXTUREBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureBufferEXT"))) == NULL) || r;
  r = ((glTextureImage1DEXT = (PFNGLTEXTUREIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureImage1DEXT"))) == NULL) || r;
  r = ((glTextureImage2DEXT = (PFNGLTEXTUREIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureImage2DEXT"))) == NULL) || r;
  r = ((glTextureImage3DEXT = (PFNGLTEXTUREIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureImage3DEXT"))) == NULL) || r;
  r = ((glTextureParameterIivEXT = (PFNGLTEXTUREPARAMETERIIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureParameterIivEXT"))) == NULL) || r;
  r = ((glTextureParameterIuivEXT = (PFNGLTEXTUREPARAMETERIUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureParameterIuivEXT"))) == NULL) || r;
  r = ((glTextureParameterfEXT = (PFNGLTEXTUREPARAMETERFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureParameterfEXT"))) == NULL) || r;
  r = ((glTextureParameterfvEXT = (PFNGLTEXTUREPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureParameterfvEXT"))) == NULL) || r;
  r = ((glTextureParameteriEXT = (PFNGLTEXTUREPARAMETERIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureParameteriEXT"))) == NULL) || r;
  r = ((glTextureParameterivEXT = (PFNGLTEXTUREPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureParameterivEXT"))) == NULL) || r;
  r = ((glTextureRenderbufferEXT = (PFNGLTEXTURERENDERBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureRenderbufferEXT"))) == NULL) || r;
  r = ((glTextureSubImage1DEXT = (PFNGLTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureSubImage1DEXT"))) == NULL) || r;
  r = ((glTextureSubImage2DEXT = (PFNGLTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureSubImage2DEXT"))) == NULL) || r;
  r = ((glTextureSubImage3DEXT = (PFNGLTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureSubImage3DEXT"))) == NULL) || r;
  r = ((glUnmapNamedBufferEXT = (PFNGLUNMAPNAMEDBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUnmapNamedBufferEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_direct_state_access */

#ifdef GL_EXT_draw_buffers2

static GLboolean _glewInit_GL_EXT_draw_buffers2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorMaskIndexedEXT = (PFNGLCOLORMASKINDEXEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorMaskIndexedEXT"))) == NULL) || r;
  r = ((glDisableIndexedEXT = (PFNGLDISABLEINDEXEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDisableIndexedEXT"))) == NULL) || r;
  r = ((glEnableIndexedEXT = (PFNGLENABLEINDEXEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnableIndexedEXT"))) == NULL) || r;
  r = ((glGetBooleanIndexedvEXT = (PFNGLGETBOOLEANINDEXEDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetBooleanIndexedvEXT"))) == NULL) || r;
  r = ((glGetIntegerIndexedvEXT = (PFNGLGETINTEGERINDEXEDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetIntegerIndexedvEXT"))) == NULL) || r;
  r = ((glIsEnabledIndexedEXT = (PFNGLISENABLEDINDEXEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsEnabledIndexedEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_draw_buffers2 */

#ifdef GL_EXT_draw_instanced

static GLboolean _glewInit_GL_EXT_draw_instanced (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawArraysInstancedEXT"))) == NULL) || r;
  r = ((glDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawElementsInstancedEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_draw_instanced */

#ifdef GL_EXT_draw_range_elements

static GLboolean _glewInit_GL_EXT_draw_range_elements (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawRangeElementsEXT = (PFNGLDRAWRANGEELEMENTSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawRangeElementsEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_draw_range_elements */

#ifdef GL_EXT_fog_coord

static GLboolean _glewInit_GL_EXT_fog_coord (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFogCoordPointerEXT = (PFNGLFOGCOORDPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordPointerEXT"))) == NULL) || r;
  r = ((glFogCoorddEXT = (PFNGLFOGCOORDDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoorddEXT"))) == NULL) || r;
  r = ((glFogCoorddvEXT = (PFNGLFOGCOORDDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoorddvEXT"))) == NULL) || r;
  r = ((glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordfEXT"))) == NULL) || r;
  r = ((glFogCoordfvEXT = (PFNGLFOGCOORDFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordfvEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_fog_coord */

#ifdef GL_EXT_fragment_lighting

static GLboolean _glewInit_GL_EXT_fragment_lighting (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFragmentColorMaterialEXT = (PFNGLFRAGMENTCOLORMATERIALEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentColorMaterialEXT"))) == NULL) || r;
  r = ((glFragmentLightModelfEXT = (PFNGLFRAGMENTLIGHTMODELFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightModelfEXT"))) == NULL) || r;
  r = ((glFragmentLightModelfvEXT = (PFNGLFRAGMENTLIGHTMODELFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightModelfvEXT"))) == NULL) || r;
  r = ((glFragmentLightModeliEXT = (PFNGLFRAGMENTLIGHTMODELIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightModeliEXT"))) == NULL) || r;
  r = ((glFragmentLightModelivEXT = (PFNGLFRAGMENTLIGHTMODELIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightModelivEXT"))) == NULL) || r;
  r = ((glFragmentLightfEXT = (PFNGLFRAGMENTLIGHTFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightfEXT"))) == NULL) || r;
  r = ((glFragmentLightfvEXT = (PFNGLFRAGMENTLIGHTFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightfvEXT"))) == NULL) || r;
  r = ((glFragmentLightiEXT = (PFNGLFRAGMENTLIGHTIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightiEXT"))) == NULL) || r;
  r = ((glFragmentLightivEXT = (PFNGLFRAGMENTLIGHTIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightivEXT"))) == NULL) || r;
  r = ((glFragmentMaterialfEXT = (PFNGLFRAGMENTMATERIALFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentMaterialfEXT"))) == NULL) || r;
  r = ((glFragmentMaterialfvEXT = (PFNGLFRAGMENTMATERIALFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentMaterialfvEXT"))) == NULL) || r;
  r = ((glFragmentMaterialiEXT = (PFNGLFRAGMENTMATERIALIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentMaterialiEXT"))) == NULL) || r;
  r = ((glFragmentMaterialivEXT = (PFNGLFRAGMENTMATERIALIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentMaterialivEXT"))) == NULL) || r;
  r = ((glGetFragmentLightfvEXT = (PFNGLGETFRAGMENTLIGHTFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragmentLightfvEXT"))) == NULL) || r;
  r = ((glGetFragmentLightivEXT = (PFNGLGETFRAGMENTLIGHTIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragmentLightivEXT"))) == NULL) || r;
  r = ((glGetFragmentMaterialfvEXT = (PFNGLGETFRAGMENTMATERIALFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragmentMaterialfvEXT"))) == NULL) || r;
  r = ((glGetFragmentMaterialivEXT = (PFNGLGETFRAGMENTMATERIALIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragmentMaterialivEXT"))) == NULL) || r;
  r = ((glLightEnviEXT = (PFNGLLIGHTENVIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLightEnviEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_fragment_lighting */

#ifdef GL_EXT_framebuffer_blit

static GLboolean _glewInit_GL_EXT_framebuffer_blit (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBlitFramebufferEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_blit */

#ifdef GL_EXT_framebuffer_multisample

static GLboolean _glewInit_GL_EXT_framebuffer_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRenderbufferStorageMultisampleEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_multisample */

#ifdef GL_EXT_framebuffer_object

static GLboolean _glewInit_GL_EXT_framebuffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindFramebufferEXT"))) == NULL) || r;
  r = ((glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindRenderbufferEXT"))) == NULL) || r;
  r = ((glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCheckFramebufferStatusEXT"))) == NULL) || r;
  r = ((glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteFramebuffersEXT"))) == NULL) || r;
  r = ((glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteRenderbuffersEXT"))) == NULL) || r;
  r = ((glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferRenderbufferEXT"))) == NULL) || r;
  r = ((glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTexture1DEXT"))) == NULL) || r;
  r = ((glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTexture2DEXT"))) == NULL) || r;
  r = ((glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTexture3DEXT"))) == NULL) || r;
  r = ((glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenFramebuffersEXT"))) == NULL) || r;
  r = ((glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenRenderbuffersEXT"))) == NULL) || r;
  r = ((glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenerateMipmapEXT"))) == NULL) || r;
  r = ((glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFramebufferAttachmentParameterivEXT"))) == NULL) || r;
  r = ((glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetRenderbufferParameterivEXT"))) == NULL) || r;
  r = ((glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsFramebufferEXT"))) == NULL) || r;
  r = ((glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsRenderbufferEXT"))) == NULL) || r;
  r = ((glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRenderbufferStorageEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_object */

#ifdef GL_EXT_framebuffer_sRGB

#endif /* GL_EXT_framebuffer_sRGB */

#ifdef GL_EXT_geometry_shader4

static GLboolean _glewInit_GL_EXT_geometry_shader4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFramebufferTextureEXT = (PFNGLFRAMEBUFFERTEXTUREEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTextureEXT"))) == NULL) || r;
  r = ((glFramebufferTextureFaceEXT = (PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTextureFaceEXT"))) == NULL) || r;
  r = ((glFramebufferTextureLayerEXT = (PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFramebufferTextureLayerEXT"))) == NULL) || r;
  r = ((glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramParameteriEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_geometry_shader4 */

#ifdef GL_EXT_gpu_program_parameters

static GLboolean _glewInit_GL_EXT_gpu_program_parameters (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProgramEnvParameters4fvEXT = (PFNGLPROGRAMENVPARAMETERS4FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameters4fvEXT"))) == NULL) || r;
  r = ((glProgramLocalParameters4fvEXT = (PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameters4fvEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_gpu_program_parameters */

#ifdef GL_EXT_gpu_shader4

static GLboolean _glewInit_GL_EXT_gpu_shader4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFragDataLocationEXT = (PFNGLBINDFRAGDATALOCATIONEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindFragDataLocationEXT"))) == NULL) || r;
  r = ((glGetFragDataLocationEXT = (PFNGLGETFRAGDATALOCATIONEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragDataLocationEXT"))) == NULL) || r;
  r = ((glGetUniformuivEXT = (PFNGLGETUNIFORMUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetUniformuivEXT"))) == NULL) || r;
  r = ((glGetVertexAttribIivEXT = (PFNGLGETVERTEXATTRIBIIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribIivEXT"))) == NULL) || r;
  r = ((glGetVertexAttribIuivEXT = (PFNGLGETVERTEXATTRIBIUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribIuivEXT"))) == NULL) || r;
  r = ((glUniform1uiEXT = (PFNGLUNIFORM1UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1uiEXT"))) == NULL) || r;
  r = ((glUniform1uivEXT = (PFNGLUNIFORM1UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform1uivEXT"))) == NULL) || r;
  r = ((glUniform2uiEXT = (PFNGLUNIFORM2UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2uiEXT"))) == NULL) || r;
  r = ((glUniform2uivEXT = (PFNGLUNIFORM2UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform2uivEXT"))) == NULL) || r;
  r = ((glUniform3uiEXT = (PFNGLUNIFORM3UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3uiEXT"))) == NULL) || r;
  r = ((glUniform3uivEXT = (PFNGLUNIFORM3UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform3uivEXT"))) == NULL) || r;
  r = ((glUniform4uiEXT = (PFNGLUNIFORM4UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4uiEXT"))) == NULL) || r;
  r = ((glUniform4uivEXT = (PFNGLUNIFORM4UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glUniform4uivEXT"))) == NULL) || r;
  r = ((glVertexAttribI1iEXT = (PFNGLVERTEXATTRIBI1IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI1iEXT"))) == NULL) || r;
  r = ((glVertexAttribI1ivEXT = (PFNGLVERTEXATTRIBI1IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI1ivEXT"))) == NULL) || r;
  r = ((glVertexAttribI1uiEXT = (PFNGLVERTEXATTRIBI1UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI1uiEXT"))) == NULL) || r;
  r = ((glVertexAttribI1uivEXT = (PFNGLVERTEXATTRIBI1UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI1uivEXT"))) == NULL) || r;
  r = ((glVertexAttribI2iEXT = (PFNGLVERTEXATTRIBI2IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI2iEXT"))) == NULL) || r;
  r = ((glVertexAttribI2ivEXT = (PFNGLVERTEXATTRIBI2IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI2ivEXT"))) == NULL) || r;
  r = ((glVertexAttribI2uiEXT = (PFNGLVERTEXATTRIBI2UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI2uiEXT"))) == NULL) || r;
  r = ((glVertexAttribI2uivEXT = (PFNGLVERTEXATTRIBI2UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI2uivEXT"))) == NULL) || r;
  r = ((glVertexAttribI3iEXT = (PFNGLVERTEXATTRIBI3IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI3iEXT"))) == NULL) || r;
  r = ((glVertexAttribI3ivEXT = (PFNGLVERTEXATTRIBI3IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI3ivEXT"))) == NULL) || r;
  r = ((glVertexAttribI3uiEXT = (PFNGLVERTEXATTRIBI3UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI3uiEXT"))) == NULL) || r;
  r = ((glVertexAttribI3uivEXT = (PFNGLVERTEXATTRIBI3UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI3uivEXT"))) == NULL) || r;
  r = ((glVertexAttribI4bvEXT = (PFNGLVERTEXATTRIBI4BVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4bvEXT"))) == NULL) || r;
  r = ((glVertexAttribI4iEXT = (PFNGLVERTEXATTRIBI4IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4iEXT"))) == NULL) || r;
  r = ((glVertexAttribI4ivEXT = (PFNGLVERTEXATTRIBI4IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4ivEXT"))) == NULL) || r;
  r = ((glVertexAttribI4svEXT = (PFNGLVERTEXATTRIBI4SVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4svEXT"))) == NULL) || r;
  r = ((glVertexAttribI4ubvEXT = (PFNGLVERTEXATTRIBI4UBVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4ubvEXT"))) == NULL) || r;
  r = ((glVertexAttribI4uiEXT = (PFNGLVERTEXATTRIBI4UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4uiEXT"))) == NULL) || r;
  r = ((glVertexAttribI4uivEXT = (PFNGLVERTEXATTRIBI4UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4uivEXT"))) == NULL) || r;
  r = ((glVertexAttribI4usvEXT = (PFNGLVERTEXATTRIBI4USVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribI4usvEXT"))) == NULL) || r;
  r = ((glVertexAttribIPointerEXT = (PFNGLVERTEXATTRIBIPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribIPointerEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_gpu_shader4 */

#ifdef GL_EXT_histogram

static GLboolean _glewInit_GL_EXT_histogram (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetHistogramEXT = (PFNGLGETHISTOGRAMEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetHistogramEXT"))) == NULL) || r;
  r = ((glGetHistogramParameterfvEXT = (PFNGLGETHISTOGRAMPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetHistogramParameterfvEXT"))) == NULL) || r;
  r = ((glGetHistogramParameterivEXT = (PFNGLGETHISTOGRAMPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetHistogramParameterivEXT"))) == NULL) || r;
  r = ((glGetMinmaxEXT = (PFNGLGETMINMAXEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMinmaxEXT"))) == NULL) || r;
  r = ((glGetMinmaxParameterfvEXT = (PFNGLGETMINMAXPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMinmaxParameterfvEXT"))) == NULL) || r;
  r = ((glGetMinmaxParameterivEXT = (PFNGLGETMINMAXPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMinmaxParameterivEXT"))) == NULL) || r;
  r = ((glHistogramEXT = (PFNGLHISTOGRAMEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glHistogramEXT"))) == NULL) || r;
  r = ((glMinmaxEXT = (PFNGLMINMAXEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMinmaxEXT"))) == NULL) || r;
  r = ((glResetHistogramEXT = (PFNGLRESETHISTOGRAMEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glResetHistogramEXT"))) == NULL) || r;
  r = ((glResetMinmaxEXT = (PFNGLRESETMINMAXEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glResetMinmaxEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_histogram */

#ifdef GL_EXT_index_array_formats

#endif /* GL_EXT_index_array_formats */

#ifdef GL_EXT_index_func

static GLboolean _glewInit_GL_EXT_index_func (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glIndexFuncEXT = (PFNGLINDEXFUNCEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexFuncEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_index_func */

#ifdef GL_EXT_index_material

static GLboolean _glewInit_GL_EXT_index_material (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glIndexMaterialEXT = (PFNGLINDEXMATERIALEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexMaterialEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_index_material */

#ifdef GL_EXT_index_texture

#endif /* GL_EXT_index_texture */

#ifdef GL_EXT_light_texture

static GLboolean _glewInit_GL_EXT_light_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glApplyTextureEXT = (PFNGLAPPLYTEXTUREEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glApplyTextureEXT"))) == NULL) || r;
  r = ((glTextureLightEXT = (PFNGLTEXTURELIGHTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureLightEXT"))) == NULL) || r;
  r = ((glTextureMaterialEXT = (PFNGLTEXTUREMATERIALEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureMaterialEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_light_texture */

#ifdef GL_EXT_misc_attribute

#endif /* GL_EXT_misc_attribute */

#ifdef GL_EXT_multi_draw_arrays

static GLboolean _glewInit_GL_EXT_multi_draw_arrays (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiDrawArraysEXT"))) == NULL) || r;
  r = ((glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiDrawElementsEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_multi_draw_arrays */

#ifdef GL_EXT_multisample

static GLboolean _glewInit_GL_EXT_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSampleMaskEXT = (PFNGLSAMPLEMASKEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSampleMaskEXT"))) == NULL) || r;
  r = ((glSamplePatternEXT = (PFNGLSAMPLEPATTERNEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSamplePatternEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_multisample */

#ifdef GL_EXT_packed_depth_stencil

#endif /* GL_EXT_packed_depth_stencil */

#ifdef GL_EXT_packed_float

#endif /* GL_EXT_packed_float */

#ifdef GL_EXT_packed_pixels

#endif /* GL_EXT_packed_pixels */

#ifdef GL_EXT_paletted_texture

static GLboolean _glewInit_GL_EXT_paletted_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorTableEXT = (PFNGLCOLORTABLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorTableEXT"))) == NULL) || r;
  r = ((glGetColorTableEXT = (PFNGLGETCOLORTABLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTableEXT"))) == NULL) || r;
  r = ((glGetColorTableParameterfvEXT = (PFNGLGETCOLORTABLEPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTableParameterfvEXT"))) == NULL) || r;
  r = ((glGetColorTableParameterivEXT = (PFNGLGETCOLORTABLEPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTableParameterivEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_paletted_texture */

#ifdef GL_EXT_pixel_buffer_object

#endif /* GL_EXT_pixel_buffer_object */

#ifdef GL_EXT_pixel_transform

static GLboolean _glewInit_GL_EXT_pixel_transform (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetPixelTransformParameterfvEXT = (PFNGLGETPIXELTRANSFORMPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPixelTransformParameterfvEXT"))) == NULL) || r;
  r = ((glGetPixelTransformParameterivEXT = (PFNGLGETPIXELTRANSFORMPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPixelTransformParameterivEXT"))) == NULL) || r;
  r = ((glPixelTransformParameterfEXT = (PFNGLPIXELTRANSFORMPARAMETERFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelTransformParameterfEXT"))) == NULL) || r;
  r = ((glPixelTransformParameterfvEXT = (PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelTransformParameterfvEXT"))) == NULL) || r;
  r = ((glPixelTransformParameteriEXT = (PFNGLPIXELTRANSFORMPARAMETERIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelTransformParameteriEXT"))) == NULL) || r;
  r = ((glPixelTransformParameterivEXT = (PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelTransformParameterivEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_pixel_transform */

#ifdef GL_EXT_pixel_transform_color_table

#endif /* GL_EXT_pixel_transform_color_table */

#ifdef GL_EXT_point_parameters

static GLboolean _glewInit_GL_EXT_point_parameters (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPointParameterfEXT = (PFNGLPOINTPARAMETERFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameterfEXT"))) == NULL) || r;
  r = ((glPointParameterfvEXT = (PFNGLPOINTPARAMETERFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameterfvEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_point_parameters */

#ifdef GL_EXT_polygon_offset

static GLboolean _glewInit_GL_EXT_polygon_offset (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPolygonOffsetEXT = (PFNGLPOLYGONOFFSETEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPolygonOffsetEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_polygon_offset */

#ifdef GL_EXT_rescale_normal

#endif /* GL_EXT_rescale_normal */

#ifdef GL_EXT_scene_marker

static GLboolean _glewInit_GL_EXT_scene_marker (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginSceneEXT = (PFNGLBEGINSCENEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginSceneEXT"))) == NULL) || r;
  r = ((glEndSceneEXT = (PFNGLENDSCENEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndSceneEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_scene_marker */

#ifdef GL_EXT_secondary_color

static GLboolean _glewInit_GL_EXT_secondary_color (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSecondaryColor3bEXT = (PFNGLSECONDARYCOLOR3BEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3bEXT"))) == NULL) || r;
  r = ((glSecondaryColor3bvEXT = (PFNGLSECONDARYCOLOR3BVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3bvEXT"))) == NULL) || r;
  r = ((glSecondaryColor3dEXT = (PFNGLSECONDARYCOLOR3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3dEXT"))) == NULL) || r;
  r = ((glSecondaryColor3dvEXT = (PFNGLSECONDARYCOLOR3DVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3dvEXT"))) == NULL) || r;
  r = ((glSecondaryColor3fEXT = (PFNGLSECONDARYCOLOR3FEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3fEXT"))) == NULL) || r;
  r = ((glSecondaryColor3fvEXT = (PFNGLSECONDARYCOLOR3FVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3fvEXT"))) == NULL) || r;
  r = ((glSecondaryColor3iEXT = (PFNGLSECONDARYCOLOR3IEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3iEXT"))) == NULL) || r;
  r = ((glSecondaryColor3ivEXT = (PFNGLSECONDARYCOLOR3IVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3ivEXT"))) == NULL) || r;
  r = ((glSecondaryColor3sEXT = (PFNGLSECONDARYCOLOR3SEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3sEXT"))) == NULL) || r;
  r = ((glSecondaryColor3svEXT = (PFNGLSECONDARYCOLOR3SVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3svEXT"))) == NULL) || r;
  r = ((glSecondaryColor3ubEXT = (PFNGLSECONDARYCOLOR3UBEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3ubEXT"))) == NULL) || r;
  r = ((glSecondaryColor3ubvEXT = (PFNGLSECONDARYCOLOR3UBVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3ubvEXT"))) == NULL) || r;
  r = ((glSecondaryColor3uiEXT = (PFNGLSECONDARYCOLOR3UIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3uiEXT"))) == NULL) || r;
  r = ((glSecondaryColor3uivEXT = (PFNGLSECONDARYCOLOR3UIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3uivEXT"))) == NULL) || r;
  r = ((glSecondaryColor3usEXT = (PFNGLSECONDARYCOLOR3USEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3usEXT"))) == NULL) || r;
  r = ((glSecondaryColor3usvEXT = (PFNGLSECONDARYCOLOR3USVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3usvEXT"))) == NULL) || r;
  r = ((glSecondaryColorPointerEXT = (PFNGLSECONDARYCOLORPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColorPointerEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_secondary_color */

#ifdef GL_EXT_separate_specular_color

#endif /* GL_EXT_separate_specular_color */

#ifdef GL_EXT_shadow_funcs

#endif /* GL_EXT_shadow_funcs */

#ifdef GL_EXT_shared_texture_palette

#endif /* GL_EXT_shared_texture_palette */

#ifdef GL_EXT_stencil_clear_tag

#endif /* GL_EXT_stencil_clear_tag */

#ifdef GL_EXT_stencil_two_side

static GLboolean _glewInit_GL_EXT_stencil_two_side (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveStencilFaceEXT = (PFNGLACTIVESTENCILFACEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glActiveStencilFaceEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_stencil_two_side */

#ifdef GL_EXT_stencil_wrap

#endif /* GL_EXT_stencil_wrap */

#ifdef GL_EXT_subtexture

static GLboolean _glewInit_GL_EXT_subtexture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexSubImage1DEXT = (PFNGLTEXSUBIMAGE1DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexSubImage1DEXT"))) == NULL) || r;
  r = ((glTexSubImage2DEXT = (PFNGLTEXSUBIMAGE2DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexSubImage2DEXT"))) == NULL) || r;
  r = ((glTexSubImage3DEXT = (PFNGLTEXSUBIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexSubImage3DEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_subtexture */

#ifdef GL_EXT_texture

#endif /* GL_EXT_texture */

#ifdef GL_EXT_texture3D

static GLboolean _glewInit_GL_EXT_texture3D (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexImage3DEXT = (PFNGLTEXIMAGE3DEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexImage3DEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_texture3D */

#ifdef GL_EXT_texture_array

#endif /* GL_EXT_texture_array */

#ifdef GL_EXT_texture_buffer_object

static GLboolean _glewInit_GL_EXT_texture_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexBufferEXT = (PFNGLTEXBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexBufferEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_texture_buffer_object */

#ifdef GL_EXT_texture_compression_dxt1

#endif /* GL_EXT_texture_compression_dxt1 */

#ifdef GL_EXT_texture_compression_latc

#endif /* GL_EXT_texture_compression_latc */

#ifdef GL_EXT_texture_compression_rgtc

#endif /* GL_EXT_texture_compression_rgtc */

#ifdef GL_EXT_texture_compression_s3tc

#endif /* GL_EXT_texture_compression_s3tc */

#ifdef GL_EXT_texture_cube_map

#endif /* GL_EXT_texture_cube_map */

#ifdef GL_EXT_texture_edge_clamp

#endif /* GL_EXT_texture_edge_clamp */

#ifdef GL_EXT_texture_env

#endif /* GL_EXT_texture_env */

#ifdef GL_EXT_texture_env_add

#endif /* GL_EXT_texture_env_add */

#ifdef GL_EXT_texture_env_combine

#endif /* GL_EXT_texture_env_combine */

#ifdef GL_EXT_texture_env_dot3

#endif /* GL_EXT_texture_env_dot3 */

#ifdef GL_EXT_texture_filter_anisotropic

#endif /* GL_EXT_texture_filter_anisotropic */

#ifdef GL_EXT_texture_integer

static GLboolean _glewInit_GL_EXT_texture_integer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClearColorIiEXT = (PFNGLCLEARCOLORIIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearColorIiEXT"))) == NULL) || r;
  r = ((glClearColorIuiEXT = (PFNGLCLEARCOLORIUIEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearColorIuiEXT"))) == NULL) || r;
  r = ((glGetTexParameterIivEXT = (PFNGLGETTEXPARAMETERIIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexParameterIivEXT"))) == NULL) || r;
  r = ((glGetTexParameterIuivEXT = (PFNGLGETTEXPARAMETERIUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexParameterIuivEXT"))) == NULL) || r;
  r = ((glTexParameterIivEXT = (PFNGLTEXPARAMETERIIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexParameterIivEXT"))) == NULL) || r;
  r = ((glTexParameterIuivEXT = (PFNGLTEXPARAMETERIUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexParameterIuivEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_texture_integer */

#ifdef GL_EXT_texture_lod_bias

#endif /* GL_EXT_texture_lod_bias */

#ifdef GL_EXT_texture_mirror_clamp

#endif /* GL_EXT_texture_mirror_clamp */

#ifdef GL_EXT_texture_object

static GLboolean _glewInit_GL_EXT_texture_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAreTexturesResidentEXT = (PFNGLARETEXTURESRESIDENTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAreTexturesResidentEXT"))) == NULL) || r;
  r = ((glBindTextureEXT = (PFNGLBINDTEXTUREEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindTextureEXT"))) == NULL) || r;
  r = ((glDeleteTexturesEXT = (PFNGLDELETETEXTURESEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteTexturesEXT"))) == NULL) || r;
  r = ((glGenTexturesEXT = (PFNGLGENTEXTURESEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenTexturesEXT"))) == NULL) || r;
  r = ((glIsTextureEXT = (PFNGLISTEXTUREEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsTextureEXT"))) == NULL) || r;
  r = ((glPrioritizeTexturesEXT = (PFNGLPRIORITIZETEXTURESEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPrioritizeTexturesEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_texture_object */

#ifdef GL_EXT_texture_perturb_normal

static GLboolean _glewInit_GL_EXT_texture_perturb_normal (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTextureNormalEXT = (PFNGLTEXTURENORMALEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureNormalEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_texture_perturb_normal */

#ifdef GL_EXT_texture_rectangle

#endif /* GL_EXT_texture_rectangle */

#ifdef GL_EXT_texture_sRGB

#endif /* GL_EXT_texture_sRGB */

#ifdef GL_EXT_texture_shared_exponent

#endif /* GL_EXT_texture_shared_exponent */

#ifdef GL_EXT_texture_swizzle

#endif /* GL_EXT_texture_swizzle */

#ifdef GL_EXT_timer_query

static GLboolean _glewInit_GL_EXT_timer_query (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetQueryObjecti64vEXT = (PFNGLGETQUERYOBJECTI64VEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetQueryObjecti64vEXT"))) == NULL) || r;
  r = ((glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetQueryObjectui64vEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_timer_query */

#ifdef GL_EXT_transform_feedback

static GLboolean _glewInit_GL_EXT_transform_feedback (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginTransformFeedbackEXT = (PFNGLBEGINTRANSFORMFEEDBACKEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginTransformFeedbackEXT"))) == NULL) || r;
  r = ((glBindBufferBaseEXT = (PFNGLBINDBUFFERBASEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferBaseEXT"))) == NULL) || r;
  r = ((glBindBufferOffsetEXT = (PFNGLBINDBUFFEROFFSETEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferOffsetEXT"))) == NULL) || r;
  r = ((glBindBufferRangeEXT = (PFNGLBINDBUFFERRANGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferRangeEXT"))) == NULL) || r;
  r = ((glEndTransformFeedbackEXT = (PFNGLENDTRANSFORMFEEDBACKEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndTransformFeedbackEXT"))) == NULL) || r;
  r = ((glGetTransformFeedbackVaryingEXT = (PFNGLGETTRANSFORMFEEDBACKVARYINGEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTransformFeedbackVaryingEXT"))) == NULL) || r;
  r = ((glTransformFeedbackVaryingsEXT = (PFNGLTRANSFORMFEEDBACKVARYINGSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTransformFeedbackVaryingsEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_transform_feedback */

#ifdef GL_EXT_vertex_array

static GLboolean _glewInit_GL_EXT_vertex_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glArrayElementEXT = (PFNGLARRAYELEMENTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glArrayElementEXT"))) == NULL) || r;
  r = ((glColorPointerEXT = (PFNGLCOLORPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorPointerEXT"))) == NULL) || r;
  r = ((glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawArraysEXT"))) == NULL) || r;
  r = ((glEdgeFlagPointerEXT = (PFNGLEDGEFLAGPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEdgeFlagPointerEXT"))) == NULL) || r;
  r = ((glGetPointervEXT = (PFNGLGETPOINTERVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetPointervEXT"))) == NULL) || r;
  r = ((glIndexPointerEXT = (PFNGLINDEXPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexPointerEXT"))) == NULL) || r;
  r = ((glNormalPointerEXT = (PFNGLNORMALPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalPointerEXT"))) == NULL) || r;
  r = ((glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoordPointerEXT"))) == NULL) || r;
  r = ((glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexPointerEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_vertex_array */

#ifdef GL_EXT_vertex_array_bgra

#endif /* GL_EXT_vertex_array_bgra */

#ifdef GL_EXT_vertex_shader

static GLboolean _glewInit_GL_EXT_vertex_shader (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginVertexShaderEXT = (PFNGLBEGINVERTEXSHADEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginVertexShaderEXT"))) == NULL) || r;
  r = ((glBindLightParameterEXT = (PFNGLBINDLIGHTPARAMETEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindLightParameterEXT"))) == NULL) || r;
  r = ((glBindMaterialParameterEXT = (PFNGLBINDMATERIALPARAMETEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindMaterialParameterEXT"))) == NULL) || r;
  r = ((glBindParameterEXT = (PFNGLBINDPARAMETEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindParameterEXT"))) == NULL) || r;
  r = ((glBindTexGenParameterEXT = (PFNGLBINDTEXGENPARAMETEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindTexGenParameterEXT"))) == NULL) || r;
  r = ((glBindTextureUnitParameterEXT = (PFNGLBINDTEXTUREUNITPARAMETEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindTextureUnitParameterEXT"))) == NULL) || r;
  r = ((glBindVertexShaderEXT = (PFNGLBINDVERTEXSHADEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindVertexShaderEXT"))) == NULL) || r;
  r = ((glDeleteVertexShaderEXT = (PFNGLDELETEVERTEXSHADEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteVertexShaderEXT"))) == NULL) || r;
  r = ((glDisableVariantClientStateEXT = (PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDisableVariantClientStateEXT"))) == NULL) || r;
  r = ((glEnableVariantClientStateEXT = (PFNGLENABLEVARIANTCLIENTSTATEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEnableVariantClientStateEXT"))) == NULL) || r;
  r = ((glEndVertexShaderEXT = (PFNGLENDVERTEXSHADEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndVertexShaderEXT"))) == NULL) || r;
  r = ((glExtractComponentEXT = (PFNGLEXTRACTCOMPONENTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glExtractComponentEXT"))) == NULL) || r;
  r = ((glGenSymbolsEXT = (PFNGLGENSYMBOLSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenSymbolsEXT"))) == NULL) || r;
  r = ((glGenVertexShadersEXT = (PFNGLGENVERTEXSHADERSEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenVertexShadersEXT"))) == NULL) || r;
  r = ((glGetInvariantBooleanvEXT = (PFNGLGETINVARIANTBOOLEANVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetInvariantBooleanvEXT"))) == NULL) || r;
  r = ((glGetInvariantFloatvEXT = (PFNGLGETINVARIANTFLOATVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetInvariantFloatvEXT"))) == NULL) || r;
  r = ((glGetInvariantIntegervEXT = (PFNGLGETINVARIANTINTEGERVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetInvariantIntegervEXT"))) == NULL) || r;
  r = ((glGetLocalConstantBooleanvEXT = (PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetLocalConstantBooleanvEXT"))) == NULL) || r;
  r = ((glGetLocalConstantFloatvEXT = (PFNGLGETLOCALCONSTANTFLOATVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetLocalConstantFloatvEXT"))) == NULL) || r;
  r = ((glGetLocalConstantIntegervEXT = (PFNGLGETLOCALCONSTANTINTEGERVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetLocalConstantIntegervEXT"))) == NULL) || r;
  r = ((glGetVariantBooleanvEXT = (PFNGLGETVARIANTBOOLEANVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVariantBooleanvEXT"))) == NULL) || r;
  r = ((glGetVariantFloatvEXT = (PFNGLGETVARIANTFLOATVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVariantFloatvEXT"))) == NULL) || r;
  r = ((glGetVariantIntegervEXT = (PFNGLGETVARIANTINTEGERVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVariantIntegervEXT"))) == NULL) || r;
  r = ((glGetVariantPointervEXT = (PFNGLGETVARIANTPOINTERVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVariantPointervEXT"))) == NULL) || r;
  r = ((glInsertComponentEXT = (PFNGLINSERTCOMPONENTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glInsertComponentEXT"))) == NULL) || r;
  r = ((glIsVariantEnabledEXT = (PFNGLISVARIANTENABLEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsVariantEnabledEXT"))) == NULL) || r;
  r = ((glSetInvariantEXT = (PFNGLSETINVARIANTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSetInvariantEXT"))) == NULL) || r;
  r = ((glSetLocalConstantEXT = (PFNGLSETLOCALCONSTANTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSetLocalConstantEXT"))) == NULL) || r;
  r = ((glShaderOp1EXT = (PFNGLSHADEROP1EXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glShaderOp1EXT"))) == NULL) || r;
  r = ((glShaderOp2EXT = (PFNGLSHADEROP2EXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glShaderOp2EXT"))) == NULL) || r;
  r = ((glShaderOp3EXT = (PFNGLSHADEROP3EXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glShaderOp3EXT"))) == NULL) || r;
  r = ((glSwizzleEXT = (PFNGLSWIZZLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSwizzleEXT"))) == NULL) || r;
  r = ((glVariantPointerEXT = (PFNGLVARIANTPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantPointerEXT"))) == NULL) || r;
  r = ((glVariantbvEXT = (PFNGLVARIANTBVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantbvEXT"))) == NULL) || r;
  r = ((glVariantdvEXT = (PFNGLVARIANTDVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantdvEXT"))) == NULL) || r;
  r = ((glVariantfvEXT = (PFNGLVARIANTFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantfvEXT"))) == NULL) || r;
  r = ((glVariantivEXT = (PFNGLVARIANTIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantivEXT"))) == NULL) || r;
  r = ((glVariantsvEXT = (PFNGLVARIANTSVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantsvEXT"))) == NULL) || r;
  r = ((glVariantubvEXT = (PFNGLVARIANTUBVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantubvEXT"))) == NULL) || r;
  r = ((glVariantuivEXT = (PFNGLVARIANTUIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantuivEXT"))) == NULL) || r;
  r = ((glVariantusvEXT = (PFNGLVARIANTUSVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVariantusvEXT"))) == NULL) || r;
  r = ((glWriteMaskEXT = (PFNGLWRITEMASKEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWriteMaskEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_vertex_shader */

#ifdef GL_EXT_vertex_weighting

static GLboolean _glewInit_GL_EXT_vertex_weighting (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glVertexWeightPointerEXT = (PFNGLVERTEXWEIGHTPOINTEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexWeightPointerEXT"))) == NULL) || r;
  r = ((glVertexWeightfEXT = (PFNGLVERTEXWEIGHTFEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexWeightfEXT"))) == NULL) || r;
  r = ((glVertexWeightfvEXT = (PFNGLVERTEXWEIGHTFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexWeightfvEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_EXT_vertex_weighting */

#ifdef GL_GREMEDY_frame_terminator

static GLboolean _glewInit_GL_GREMEDY_frame_terminator (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFrameTerminatorGREMEDY = (PFNGLFRAMETERMINATORGREMEDYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFrameTerminatorGREMEDY"))) == NULL) || r;

  return r;
}

#endif /* GL_GREMEDY_frame_terminator */

#ifdef GL_GREMEDY_string_marker

static GLboolean _glewInit_GL_GREMEDY_string_marker (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glStringMarkerGREMEDY"))) == NULL) || r;

  return r;
}

#endif /* GL_GREMEDY_string_marker */

#ifdef GL_HP_convolution_border_modes

#endif /* GL_HP_convolution_border_modes */

#ifdef GL_HP_image_transform

static GLboolean _glewInit_GL_HP_image_transform (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetImageTransformParameterfvHP = (PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetImageTransformParameterfvHP"))) == NULL) || r;
  r = ((glGetImageTransformParameterivHP = (PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetImageTransformParameterivHP"))) == NULL) || r;
  r = ((glImageTransformParameterfHP = (PFNGLIMAGETRANSFORMPARAMETERFHPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glImageTransformParameterfHP"))) == NULL) || r;
  r = ((glImageTransformParameterfvHP = (PFNGLIMAGETRANSFORMPARAMETERFVHPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glImageTransformParameterfvHP"))) == NULL) || r;
  r = ((glImageTransformParameteriHP = (PFNGLIMAGETRANSFORMPARAMETERIHPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glImageTransformParameteriHP"))) == NULL) || r;
  r = ((glImageTransformParameterivHP = (PFNGLIMAGETRANSFORMPARAMETERIVHPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glImageTransformParameterivHP"))) == NULL) || r;

  return r;
}

#endif /* GL_HP_image_transform */

#ifdef GL_HP_occlusion_test

#endif /* GL_HP_occlusion_test */

#ifdef GL_HP_texture_lighting

#endif /* GL_HP_texture_lighting */

#ifdef GL_IBM_cull_vertex

#endif /* GL_IBM_cull_vertex */

#ifdef GL_IBM_multimode_draw_arrays

static GLboolean _glewInit_GL_IBM_multimode_draw_arrays (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glMultiModeDrawArraysIBM = (PFNGLMULTIMODEDRAWARRAYSIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiModeDrawArraysIBM"))) == NULL) || r;
  r = ((glMultiModeDrawElementsIBM = (PFNGLMULTIMODEDRAWELEMENTSIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiModeDrawElementsIBM"))) == NULL) || r;

  return r;
}

#endif /* GL_IBM_multimode_draw_arrays */

#ifdef GL_IBM_rasterpos_clip

#endif /* GL_IBM_rasterpos_clip */

#ifdef GL_IBM_static_data

#endif /* GL_IBM_static_data */

#ifdef GL_IBM_texture_mirrored_repeat

#endif /* GL_IBM_texture_mirrored_repeat */

#ifdef GL_IBM_vertex_array_lists

static GLboolean _glewInit_GL_IBM_vertex_array_lists (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorPointerListIBM = (PFNGLCOLORPOINTERLISTIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorPointerListIBM"))) == NULL) || r;
  r = ((glEdgeFlagPointerListIBM = (PFNGLEDGEFLAGPOINTERLISTIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEdgeFlagPointerListIBM"))) == NULL) || r;
  r = ((glFogCoordPointerListIBM = (PFNGLFOGCOORDPOINTERLISTIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordPointerListIBM"))) == NULL) || r;
  r = ((glIndexPointerListIBM = (PFNGLINDEXPOINTERLISTIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIndexPointerListIBM"))) == NULL) || r;
  r = ((glNormalPointerListIBM = (PFNGLNORMALPOINTERLISTIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalPointerListIBM"))) == NULL) || r;
  r = ((glSecondaryColorPointerListIBM = (PFNGLSECONDARYCOLORPOINTERLISTIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColorPointerListIBM"))) == NULL) || r;
  r = ((glTexCoordPointerListIBM = (PFNGLTEXCOORDPOINTERLISTIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoordPointerListIBM"))) == NULL) || r;
  r = ((glVertexPointerListIBM = (PFNGLVERTEXPOINTERLISTIBMPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexPointerListIBM"))) == NULL) || r;

  return r;
}

#endif /* GL_IBM_vertex_array_lists */

#ifdef GL_INGR_color_clamp

#endif /* GL_INGR_color_clamp */

#ifdef GL_INGR_interlace_read

#endif /* GL_INGR_interlace_read */

#ifdef GL_INTEL_parallel_arrays

static GLboolean _glewInit_GL_INTEL_parallel_arrays (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorPointervINTEL = (PFNGLCOLORPOINTERVINTELPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorPointervINTEL"))) == NULL) || r;
  r = ((glNormalPointervINTEL = (PFNGLNORMALPOINTERVINTELPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormalPointervINTEL"))) == NULL) || r;
  r = ((glTexCoordPointervINTEL = (PFNGLTEXCOORDPOINTERVINTELPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoordPointervINTEL"))) == NULL) || r;
  r = ((glVertexPointervINTEL = (PFNGLVERTEXPOINTERVINTELPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexPointervINTEL"))) == NULL) || r;

  return r;
}

#endif /* GL_INTEL_parallel_arrays */

#ifdef GL_INTEL_texture_scissor

static GLboolean _glewInit_GL_INTEL_texture_scissor (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexScissorFuncINTEL = (PFNGLTEXSCISSORFUNCINTELPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexScissorFuncINTEL"))) == NULL) || r;
  r = ((glTexScissorINTEL = (PFNGLTEXSCISSORINTELPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexScissorINTEL"))) == NULL) || r;

  return r;
}

#endif /* GL_INTEL_texture_scissor */

#ifdef GL_KTX_buffer_region

static GLboolean _glewInit_GL_KTX_buffer_region (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBufferRegionEnabledEXT = (PFNGLBUFFERREGIONENABLEDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBufferRegionEnabledEXT"))) == NULL) || r;
  r = ((glDeleteBufferRegionEXT = (PFNGLDELETEBUFFERREGIONEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteBufferRegionEXT"))) == NULL) || r;
  r = ((glDrawBufferRegionEXT = (PFNGLDRAWBUFFERREGIONEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawBufferRegionEXT"))) == NULL) || r;
  r = ((glNewBufferRegionEXT = (PFNGLNEWBUFFERREGIONEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNewBufferRegionEXT"))) == NULL) || r;
  r = ((glReadBufferRegionEXT = (PFNGLREADBUFFERREGIONEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReadBufferRegionEXT"))) == NULL) || r;

  return r;
}

#endif /* GL_KTX_buffer_region */

#ifdef GL_MESAX_texture_stack

#endif /* GL_MESAX_texture_stack */

#ifdef GL_MESA_pack_invert

#endif /* GL_MESA_pack_invert */

#ifdef GL_MESA_resize_buffers

static GLboolean _glewInit_GL_MESA_resize_buffers (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glResizeBuffersMESA = (PFNGLRESIZEBUFFERSMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glResizeBuffersMESA"))) == NULL) || r;

  return r;
}

#endif /* GL_MESA_resize_buffers */

#ifdef GL_MESA_window_pos

static GLboolean _glewInit_GL_MESA_window_pos (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glWindowPos2dMESA = (PFNGLWINDOWPOS2DMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2dMESA"))) == NULL) || r;
  r = ((glWindowPos2dvMESA = (PFNGLWINDOWPOS2DVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2dvMESA"))) == NULL) || r;
  r = ((glWindowPos2fMESA = (PFNGLWINDOWPOS2FMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2fMESA"))) == NULL) || r;
  r = ((glWindowPos2fvMESA = (PFNGLWINDOWPOS2FVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2fvMESA"))) == NULL) || r;
  r = ((glWindowPos2iMESA = (PFNGLWINDOWPOS2IMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2iMESA"))) == NULL) || r;
  r = ((glWindowPos2ivMESA = (PFNGLWINDOWPOS2IVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2ivMESA"))) == NULL) || r;
  r = ((glWindowPos2sMESA = (PFNGLWINDOWPOS2SMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2sMESA"))) == NULL) || r;
  r = ((glWindowPos2svMESA = (PFNGLWINDOWPOS2SVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos2svMESA"))) == NULL) || r;
  r = ((glWindowPos3dMESA = (PFNGLWINDOWPOS3DMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3dMESA"))) == NULL) || r;
  r = ((glWindowPos3dvMESA = (PFNGLWINDOWPOS3DVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3dvMESA"))) == NULL) || r;
  r = ((glWindowPos3fMESA = (PFNGLWINDOWPOS3FMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3fMESA"))) == NULL) || r;
  r = ((glWindowPos3fvMESA = (PFNGLWINDOWPOS3FVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3fvMESA"))) == NULL) || r;
  r = ((glWindowPos3iMESA = (PFNGLWINDOWPOS3IMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3iMESA"))) == NULL) || r;
  r = ((glWindowPos3ivMESA = (PFNGLWINDOWPOS3IVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3ivMESA"))) == NULL) || r;
  r = ((glWindowPos3sMESA = (PFNGLWINDOWPOS3SMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3sMESA"))) == NULL) || r;
  r = ((glWindowPos3svMESA = (PFNGLWINDOWPOS3SVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos3svMESA"))) == NULL) || r;
  r = ((glWindowPos4dMESA = (PFNGLWINDOWPOS4DMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos4dMESA"))) == NULL) || r;
  r = ((glWindowPos4dvMESA = (PFNGLWINDOWPOS4DVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos4dvMESA"))) == NULL) || r;
  r = ((glWindowPos4fMESA = (PFNGLWINDOWPOS4FMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos4fMESA"))) == NULL) || r;
  r = ((glWindowPos4fvMESA = (PFNGLWINDOWPOS4FVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos4fvMESA"))) == NULL) || r;
  r = ((glWindowPos4iMESA = (PFNGLWINDOWPOS4IMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos4iMESA"))) == NULL) || r;
  r = ((glWindowPos4ivMESA = (PFNGLWINDOWPOS4IVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos4ivMESA"))) == NULL) || r;
  r = ((glWindowPos4sMESA = (PFNGLWINDOWPOS4SMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos4sMESA"))) == NULL) || r;
  r = ((glWindowPos4svMESA = (PFNGLWINDOWPOS4SVMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glWindowPos4svMESA"))) == NULL) || r;

  return r;
}

#endif /* GL_MESA_window_pos */

#ifdef GL_MESA_ycbcr_texture

#endif /* GL_MESA_ycbcr_texture */

#ifdef GL_NV_blend_square

#endif /* GL_NV_blend_square */

#ifdef GL_NV_conditional_render

static GLboolean _glewInit_GL_NV_conditional_render (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginConditionalRenderNV = (PFNGLBEGINCONDITIONALRENDERNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginConditionalRenderNV"))) == NULL) || r;
  r = ((glEndConditionalRenderNV = (PFNGLENDCONDITIONALRENDERNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndConditionalRenderNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_conditional_render */

#ifdef GL_NV_copy_depth_to_color

#endif /* GL_NV_copy_depth_to_color */

#ifdef GL_NV_depth_buffer_float

static GLboolean _glewInit_GL_NV_depth_buffer_float (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClearDepthdNV = (PFNGLCLEARDEPTHDNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearDepthdNV"))) == NULL) || r;
  r = ((glDepthBoundsdNV = (PFNGLDEPTHBOUNDSDNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDepthBoundsdNV"))) == NULL) || r;
  r = ((glDepthRangedNV = (PFNGLDEPTHRANGEDNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDepthRangedNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_depth_buffer_float */

#ifdef GL_NV_depth_clamp

#endif /* GL_NV_depth_clamp */

#ifdef GL_NV_depth_range_unclamped

#endif /* GL_NV_depth_range_unclamped */

#ifdef GL_NV_evaluators

static GLboolean _glewInit_GL_NV_evaluators (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glEvalMapsNV = (PFNGLEVALMAPSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEvalMapsNV"))) == NULL) || r;
  r = ((glGetMapAttribParameterfvNV = (PFNGLGETMAPATTRIBPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMapAttribParameterfvNV"))) == NULL) || r;
  r = ((glGetMapAttribParameterivNV = (PFNGLGETMAPATTRIBPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMapAttribParameterivNV"))) == NULL) || r;
  r = ((glGetMapControlPointsNV = (PFNGLGETMAPCONTROLPOINTSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMapControlPointsNV"))) == NULL) || r;
  r = ((glGetMapParameterfvNV = (PFNGLGETMAPPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMapParameterfvNV"))) == NULL) || r;
  r = ((glGetMapParameterivNV = (PFNGLGETMAPPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMapParameterivNV"))) == NULL) || r;
  r = ((glMapControlPointsNV = (PFNGLMAPCONTROLPOINTSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapControlPointsNV"))) == NULL) || r;
  r = ((glMapParameterfvNV = (PFNGLMAPPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapParameterfvNV"))) == NULL) || r;
  r = ((glMapParameterivNV = (PFNGLMAPPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMapParameterivNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_evaluators */

#ifdef GL_NV_explicit_multisample

static GLboolean _glewInit_GL_NV_explicit_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetMultisamplefvNV = (PFNGLGETMULTISAMPLEFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetMultisamplefvNV"))) == NULL) || r;
  r = ((glSampleMaskIndexedNV = (PFNGLSAMPLEMASKINDEXEDNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSampleMaskIndexedNV"))) == NULL) || r;
  r = ((glTexRenderbufferNV = (PFNGLTEXRENDERBUFFERNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexRenderbufferNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_explicit_multisample */

#ifdef GL_NV_fence

static GLboolean _glewInit_GL_NV_fence (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDeleteFencesNV = (PFNGLDELETEFENCESNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteFencesNV"))) == NULL) || r;
  r = ((glFinishFenceNV = (PFNGLFINISHFENCENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFinishFenceNV"))) == NULL) || r;
  r = ((glGenFencesNV = (PFNGLGENFENCESNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenFencesNV"))) == NULL) || r;
  r = ((glGetFenceivNV = (PFNGLGETFENCEIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFenceivNV"))) == NULL) || r;
  r = ((glIsFenceNV = (PFNGLISFENCENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsFenceNV"))) == NULL) || r;
  r = ((glSetFenceNV = (PFNGLSETFENCENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSetFenceNV"))) == NULL) || r;
  r = ((glTestFenceNV = (PFNGLTESTFENCENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTestFenceNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_fence */

#ifdef GL_NV_float_buffer

#endif /* GL_NV_float_buffer */

#ifdef GL_NV_fog_distance

#endif /* GL_NV_fog_distance */

#ifdef GL_NV_fragment_program

static GLboolean _glewInit_GL_NV_fragment_program (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetProgramNamedParameterdvNV = (PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramNamedParameterdvNV"))) == NULL) || r;
  r = ((glGetProgramNamedParameterfvNV = (PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramNamedParameterfvNV"))) == NULL) || r;
  r = ((glProgramNamedParameter4dNV = (PFNGLPROGRAMNAMEDPARAMETER4DNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramNamedParameter4dNV"))) == NULL) || r;
  r = ((glProgramNamedParameter4dvNV = (PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramNamedParameter4dvNV"))) == NULL) || r;
  r = ((glProgramNamedParameter4fNV = (PFNGLPROGRAMNAMEDPARAMETER4FNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramNamedParameter4fNV"))) == NULL) || r;
  r = ((glProgramNamedParameter4fvNV = (PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramNamedParameter4fvNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_fragment_program */

#ifdef GL_NV_fragment_program2

#endif /* GL_NV_fragment_program2 */

#ifdef GL_NV_fragment_program4

#endif /* GL_NV_fragment_program4 */

#ifdef GL_NV_fragment_program_option

#endif /* GL_NV_fragment_program_option */

#ifdef GL_NV_framebuffer_multisample_coverage

static GLboolean _glewInit_GL_NV_framebuffer_multisample_coverage (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glRenderbufferStorageMultisampleCoverageNV = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRenderbufferStorageMultisampleCoverageNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_framebuffer_multisample_coverage */

#ifdef GL_NV_geometry_program4

static GLboolean _glewInit_GL_NV_geometry_program4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProgramVertexLimitNV = (PFNGLPROGRAMVERTEXLIMITNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramVertexLimitNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_geometry_program4 */

#ifdef GL_NV_geometry_shader4

#endif /* GL_NV_geometry_shader4 */

#ifdef GL_NV_gpu_program4

static GLboolean _glewInit_GL_NV_gpu_program4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProgramEnvParameterI4iNV = (PFNGLPROGRAMENVPARAMETERI4INVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameterI4iNV"))) == NULL) || r;
  r = ((glProgramEnvParameterI4ivNV = (PFNGLPROGRAMENVPARAMETERI4IVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameterI4ivNV"))) == NULL) || r;
  r = ((glProgramEnvParameterI4uiNV = (PFNGLPROGRAMENVPARAMETERI4UINVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameterI4uiNV"))) == NULL) || r;
  r = ((glProgramEnvParameterI4uivNV = (PFNGLPROGRAMENVPARAMETERI4UIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParameterI4uivNV"))) == NULL) || r;
  r = ((glProgramEnvParametersI4ivNV = (PFNGLPROGRAMENVPARAMETERSI4IVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParametersI4ivNV"))) == NULL) || r;
  r = ((glProgramEnvParametersI4uivNV = (PFNGLPROGRAMENVPARAMETERSI4UIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramEnvParametersI4uivNV"))) == NULL) || r;
  r = ((glProgramLocalParameterI4iNV = (PFNGLPROGRAMLOCALPARAMETERI4INVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameterI4iNV"))) == NULL) || r;
  r = ((glProgramLocalParameterI4ivNV = (PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameterI4ivNV"))) == NULL) || r;
  r = ((glProgramLocalParameterI4uiNV = (PFNGLPROGRAMLOCALPARAMETERI4UINVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameterI4uiNV"))) == NULL) || r;
  r = ((glProgramLocalParameterI4uivNV = (PFNGLPROGRAMLOCALPARAMETERI4UIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParameterI4uivNV"))) == NULL) || r;
  r = ((glProgramLocalParametersI4ivNV = (PFNGLPROGRAMLOCALPARAMETERSI4IVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParametersI4ivNV"))) == NULL) || r;
  r = ((glProgramLocalParametersI4uivNV = (PFNGLPROGRAMLOCALPARAMETERSI4UIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramLocalParametersI4uivNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_gpu_program4 */

#ifdef GL_NV_half_float

static GLboolean _glewInit_GL_NV_half_float (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColor3hNV = (PFNGLCOLOR3HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3hNV"))) == NULL) || r;
  r = ((glColor3hvNV = (PFNGLCOLOR3HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3hvNV"))) == NULL) || r;
  r = ((glColor4hNV = (PFNGLCOLOR4HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4hNV"))) == NULL) || r;
  r = ((glColor4hvNV = (PFNGLCOLOR4HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4hvNV"))) == NULL) || r;
  r = ((glFogCoordhNV = (PFNGLFOGCOORDHNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordhNV"))) == NULL) || r;
  r = ((glFogCoordhvNV = (PFNGLFOGCOORDHVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogCoordhvNV"))) == NULL) || r;
  r = ((glMultiTexCoord1hNV = (PFNGLMULTITEXCOORD1HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1hNV"))) == NULL) || r;
  r = ((glMultiTexCoord1hvNV = (PFNGLMULTITEXCOORD1HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord1hvNV"))) == NULL) || r;
  r = ((glMultiTexCoord2hNV = (PFNGLMULTITEXCOORD2HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2hNV"))) == NULL) || r;
  r = ((glMultiTexCoord2hvNV = (PFNGLMULTITEXCOORD2HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord2hvNV"))) == NULL) || r;
  r = ((glMultiTexCoord3hNV = (PFNGLMULTITEXCOORD3HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3hNV"))) == NULL) || r;
  r = ((glMultiTexCoord3hvNV = (PFNGLMULTITEXCOORD3HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord3hvNV"))) == NULL) || r;
  r = ((glMultiTexCoord4hNV = (PFNGLMULTITEXCOORD4HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4hNV"))) == NULL) || r;
  r = ((glMultiTexCoord4hvNV = (PFNGLMULTITEXCOORD4HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glMultiTexCoord4hvNV"))) == NULL) || r;
  r = ((glNormal3hNV = (PFNGLNORMAL3HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3hNV"))) == NULL) || r;
  r = ((glNormal3hvNV = (PFNGLNORMAL3HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3hvNV"))) == NULL) || r;
  r = ((glSecondaryColor3hNV = (PFNGLSECONDARYCOLOR3HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3hNV"))) == NULL) || r;
  r = ((glSecondaryColor3hvNV = (PFNGLSECONDARYCOLOR3HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSecondaryColor3hvNV"))) == NULL) || r;
  r = ((glTexCoord1hNV = (PFNGLTEXCOORD1HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1hNV"))) == NULL) || r;
  r = ((glTexCoord1hvNV = (PFNGLTEXCOORD1HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord1hvNV"))) == NULL) || r;
  r = ((glTexCoord2hNV = (PFNGLTEXCOORD2HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2hNV"))) == NULL) || r;
  r = ((glTexCoord2hvNV = (PFNGLTEXCOORD2HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2hvNV"))) == NULL) || r;
  r = ((glTexCoord3hNV = (PFNGLTEXCOORD3HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3hNV"))) == NULL) || r;
  r = ((glTexCoord3hvNV = (PFNGLTEXCOORD3HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord3hvNV"))) == NULL) || r;
  r = ((glTexCoord4hNV = (PFNGLTEXCOORD4HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4hNV"))) == NULL) || r;
  r = ((glTexCoord4hvNV = (PFNGLTEXCOORD4HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4hvNV"))) == NULL) || r;
  r = ((glVertex2hNV = (PFNGLVERTEX2HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2hNV"))) == NULL) || r;
  r = ((glVertex2hvNV = (PFNGLVERTEX2HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex2hvNV"))) == NULL) || r;
  r = ((glVertex3hNV = (PFNGLVERTEX3HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3hNV"))) == NULL) || r;
  r = ((glVertex3hvNV = (PFNGLVERTEX3HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex3hvNV"))) == NULL) || r;
  r = ((glVertex4hNV = (PFNGLVERTEX4HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4hNV"))) == NULL) || r;
  r = ((glVertex4hvNV = (PFNGLVERTEX4HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertex4hvNV"))) == NULL) || r;
  r = ((glVertexAttrib1hNV = (PFNGLVERTEXATTRIB1HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1hNV"))) == NULL) || r;
  r = ((glVertexAttrib1hvNV = (PFNGLVERTEXATTRIB1HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1hvNV"))) == NULL) || r;
  r = ((glVertexAttrib2hNV = (PFNGLVERTEXATTRIB2HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2hNV"))) == NULL) || r;
  r = ((glVertexAttrib2hvNV = (PFNGLVERTEXATTRIB2HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2hvNV"))) == NULL) || r;
  r = ((glVertexAttrib3hNV = (PFNGLVERTEXATTRIB3HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3hNV"))) == NULL) || r;
  r = ((glVertexAttrib3hvNV = (PFNGLVERTEXATTRIB3HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3hvNV"))) == NULL) || r;
  r = ((glVertexAttrib4hNV = (PFNGLVERTEXATTRIB4HNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4hNV"))) == NULL) || r;
  r = ((glVertexAttrib4hvNV = (PFNGLVERTEXATTRIB4HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4hvNV"))) == NULL) || r;
  r = ((glVertexAttribs1hvNV = (PFNGLVERTEXATTRIBS1HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs1hvNV"))) == NULL) || r;
  r = ((glVertexAttribs2hvNV = (PFNGLVERTEXATTRIBS2HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs2hvNV"))) == NULL) || r;
  r = ((glVertexAttribs3hvNV = (PFNGLVERTEXATTRIBS3HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs3hvNV"))) == NULL) || r;
  r = ((glVertexAttribs4hvNV = (PFNGLVERTEXATTRIBS4HVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs4hvNV"))) == NULL) || r;
  r = ((glVertexWeighthNV = (PFNGLVERTEXWEIGHTHNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexWeighthNV"))) == NULL) || r;
  r = ((glVertexWeighthvNV = (PFNGLVERTEXWEIGHTHVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexWeighthvNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_half_float */

#ifdef GL_NV_light_max_exponent

#endif /* GL_NV_light_max_exponent */

#ifdef GL_NV_multisample_filter_hint

#endif /* GL_NV_multisample_filter_hint */

#ifdef GL_NV_occlusion_query

static GLboolean _glewInit_GL_NV_occlusion_query (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginOcclusionQueryNV = (PFNGLBEGINOCCLUSIONQUERYNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginOcclusionQueryNV"))) == NULL) || r;
  r = ((glDeleteOcclusionQueriesNV = (PFNGLDELETEOCCLUSIONQUERIESNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteOcclusionQueriesNV"))) == NULL) || r;
  r = ((glEndOcclusionQueryNV = (PFNGLENDOCCLUSIONQUERYNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndOcclusionQueryNV"))) == NULL) || r;
  r = ((glGenOcclusionQueriesNV = (PFNGLGENOCCLUSIONQUERIESNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenOcclusionQueriesNV"))) == NULL) || r;
  r = ((glGetOcclusionQueryivNV = (PFNGLGETOCCLUSIONQUERYIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetOcclusionQueryivNV"))) == NULL) || r;
  r = ((glGetOcclusionQueryuivNV = (PFNGLGETOCCLUSIONQUERYUIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetOcclusionQueryuivNV"))) == NULL) || r;
  r = ((glIsOcclusionQueryNV = (PFNGLISOCCLUSIONQUERYNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsOcclusionQueryNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_occlusion_query */

#ifdef GL_NV_packed_depth_stencil

#endif /* GL_NV_packed_depth_stencil */

#ifdef GL_NV_parameter_buffer_object

static GLboolean _glewInit_GL_NV_parameter_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProgramBufferParametersIivNV = (PFNGLPROGRAMBUFFERPARAMETERSIIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramBufferParametersIivNV"))) == NULL) || r;
  r = ((glProgramBufferParametersIuivNV = (PFNGLPROGRAMBUFFERPARAMETERSIUIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramBufferParametersIuivNV"))) == NULL) || r;
  r = ((glProgramBufferParametersfvNV = (PFNGLPROGRAMBUFFERPARAMETERSFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramBufferParametersfvNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_parameter_buffer_object */

#ifdef GL_NV_pixel_data_range

static GLboolean _glewInit_GL_NV_pixel_data_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushPixelDataRangeNV = (PFNGLFLUSHPIXELDATARANGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFlushPixelDataRangeNV"))) == NULL) || r;
  r = ((glPixelDataRangeNV = (PFNGLPIXELDATARANGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelDataRangeNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_pixel_data_range */

#ifdef GL_NV_point_sprite

static GLboolean _glewInit_GL_NV_point_sprite (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPointParameteriNV = (PFNGLPOINTPARAMETERINVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameteriNV"))) == NULL) || r;
  r = ((glPointParameterivNV = (PFNGLPOINTPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPointParameterivNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_point_sprite */

#ifdef GL_NV_present_video

static GLboolean _glewInit_GL_NV_present_video (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetVideoi64vNV = (PFNGLGETVIDEOI64VNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVideoi64vNV"))) == NULL) || r;
  r = ((glGetVideoivNV = (PFNGLGETVIDEOIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVideoivNV"))) == NULL) || r;
  r = ((glGetVideoui64vNV = (PFNGLGETVIDEOUI64VNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVideoui64vNV"))) == NULL) || r;
  r = ((glGetVideouivNV = (PFNGLGETVIDEOUIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVideouivNV"))) == NULL) || r;
  r = ((glPresentFrameDualFillNV = (PFNGLPRESENTFRAMEDUALFILLNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPresentFrameDualFillNV"))) == NULL) || r;
  r = ((glPresentFrameKeyedNV = (PFNGLPRESENTFRAMEKEYEDNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPresentFrameKeyedNV"))) == NULL) || r;
  r = ((glVideoParameterivNV = (PFNGLVIDEOPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVideoParameterivNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_present_video */

#ifdef GL_NV_primitive_restart

static GLboolean _glewInit_GL_NV_primitive_restart (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPrimitiveRestartIndexNV = (PFNGLPRIMITIVERESTARTINDEXNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPrimitiveRestartIndexNV"))) == NULL) || r;
  r = ((glPrimitiveRestartNV = (PFNGLPRIMITIVERESTARTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPrimitiveRestartNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_primitive_restart */

#ifdef GL_NV_register_combiners

static GLboolean _glewInit_GL_NV_register_combiners (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCombinerInputNV = (PFNGLCOMBINERINPUTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCombinerInputNV"))) == NULL) || r;
  r = ((glCombinerOutputNV = (PFNGLCOMBINEROUTPUTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCombinerOutputNV"))) == NULL) || r;
  r = ((glCombinerParameterfNV = (PFNGLCOMBINERPARAMETERFNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCombinerParameterfNV"))) == NULL) || r;
  r = ((glCombinerParameterfvNV = (PFNGLCOMBINERPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCombinerParameterfvNV"))) == NULL) || r;
  r = ((glCombinerParameteriNV = (PFNGLCOMBINERPARAMETERINVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCombinerParameteriNV"))) == NULL) || r;
  r = ((glCombinerParameterivNV = (PFNGLCOMBINERPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCombinerParameterivNV"))) == NULL) || r;
  r = ((glFinalCombinerInputNV = (PFNGLFINALCOMBINERINPUTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFinalCombinerInputNV"))) == NULL) || r;
  r = ((glGetCombinerInputParameterfvNV = (PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCombinerInputParameterfvNV"))) == NULL) || r;
  r = ((glGetCombinerInputParameterivNV = (PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCombinerInputParameterivNV"))) == NULL) || r;
  r = ((glGetCombinerOutputParameterfvNV = (PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCombinerOutputParameterfvNV"))) == NULL) || r;
  r = ((glGetCombinerOutputParameterivNV = (PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCombinerOutputParameterivNV"))) == NULL) || r;
  r = ((glGetFinalCombinerInputParameterfvNV = (PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFinalCombinerInputParameterfvNV"))) == NULL) || r;
  r = ((glGetFinalCombinerInputParameterivNV = (PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFinalCombinerInputParameterivNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_register_combiners */

#ifdef GL_NV_register_combiners2

static GLboolean _glewInit_GL_NV_register_combiners2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCombinerStageParameterfvNV = (PFNGLCOMBINERSTAGEPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCombinerStageParameterfvNV"))) == NULL) || r;
  r = ((glGetCombinerStageParameterfvNV = (PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetCombinerStageParameterfvNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_register_combiners2 */

#ifdef GL_NV_texgen_emboss

#endif /* GL_NV_texgen_emboss */

#ifdef GL_NV_texgen_reflection

#endif /* GL_NV_texgen_reflection */

#ifdef GL_NV_texture_compression_vtc

#endif /* GL_NV_texture_compression_vtc */

#ifdef GL_NV_texture_env_combine4

#endif /* GL_NV_texture_env_combine4 */

#ifdef GL_NV_texture_expand_normal

#endif /* GL_NV_texture_expand_normal */

#ifdef GL_NV_texture_rectangle

#endif /* GL_NV_texture_rectangle */

#ifdef GL_NV_texture_shader

#endif /* GL_NV_texture_shader */

#ifdef GL_NV_texture_shader2

#endif /* GL_NV_texture_shader2 */

#ifdef GL_NV_texture_shader3

#endif /* GL_NV_texture_shader3 */

#ifdef GL_NV_transform_feedback

static GLboolean _glewInit_GL_NV_transform_feedback (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveVaryingNV = (PFNGLACTIVEVARYINGNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glActiveVaryingNV"))) == NULL) || r;
  r = ((glBeginTransformFeedbackNV = (PFNGLBEGINTRANSFORMFEEDBACKNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBeginTransformFeedbackNV"))) == NULL) || r;
  r = ((glBindBufferBaseNV = (PFNGLBINDBUFFERBASENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferBaseNV"))) == NULL) || r;
  r = ((glBindBufferOffsetNV = (PFNGLBINDBUFFEROFFSETNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferOffsetNV"))) == NULL) || r;
  r = ((glBindBufferRangeNV = (PFNGLBINDBUFFERRANGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindBufferRangeNV"))) == NULL) || r;
  r = ((glEndTransformFeedbackNV = (PFNGLENDTRANSFORMFEEDBACKNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glEndTransformFeedbackNV"))) == NULL) || r;
  r = ((glGetActiveVaryingNV = (PFNGLGETACTIVEVARYINGNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetActiveVaryingNV"))) == NULL) || r;
  r = ((glGetTransformFeedbackVaryingNV = (PFNGLGETTRANSFORMFEEDBACKVARYINGNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTransformFeedbackVaryingNV"))) == NULL) || r;
  r = ((glGetVaryingLocationNV = (PFNGLGETVARYINGLOCATIONNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVaryingLocationNV"))) == NULL) || r;
  r = ((glTransformFeedbackAttribsNV = (PFNGLTRANSFORMFEEDBACKATTRIBSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTransformFeedbackAttribsNV"))) == NULL) || r;
  r = ((glTransformFeedbackVaryingsNV = (PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTransformFeedbackVaryingsNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_transform_feedback */

#ifdef GL_NV_transform_feedback2

static GLboolean _glewInit_GL_NV_transform_feedback2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindTransformFeedbackNV = (PFNGLBINDTRANSFORMFEEDBACKNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindTransformFeedbackNV"))) == NULL) || r;
  r = ((glDeleteTransformFeedbacksNV = (PFNGLDELETETRANSFORMFEEDBACKSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteTransformFeedbacksNV"))) == NULL) || r;
  r = ((glDrawTransformFeedbackNV = (PFNGLDRAWTRANSFORMFEEDBACKNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDrawTransformFeedbackNV"))) == NULL) || r;
  r = ((glGenTransformFeedbacksNV = (PFNGLGENTRANSFORMFEEDBACKSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenTransformFeedbacksNV"))) == NULL) || r;
  r = ((glIsTransformFeedbackNV = (PFNGLISTRANSFORMFEEDBACKNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsTransformFeedbackNV"))) == NULL) || r;
  r = ((glPauseTransformFeedbackNV = (PFNGLPAUSETRANSFORMFEEDBACKNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPauseTransformFeedbackNV"))) == NULL) || r;
  r = ((glResumeTransformFeedbackNV = (PFNGLRESUMETRANSFORMFEEDBACKNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glResumeTransformFeedbackNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_transform_feedback2 */

#ifdef GL_NV_vertex_array_range

static GLboolean _glewInit_GL_NV_vertex_array_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushVertexArrayRangeNV = (PFNGLFLUSHVERTEXARRAYRANGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFlushVertexArrayRangeNV"))) == NULL) || r;
  r = ((glVertexArrayRangeNV = (PFNGLVERTEXARRAYRANGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexArrayRangeNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_vertex_array_range */

#ifdef GL_NV_vertex_array_range2

#endif /* GL_NV_vertex_array_range2 */

#ifdef GL_NV_vertex_program

static GLboolean _glewInit_GL_NV_vertex_program (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAreProgramsResidentNV = (PFNGLAREPROGRAMSRESIDENTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAreProgramsResidentNV"))) == NULL) || r;
  r = ((glBindProgramNV = (PFNGLBINDPROGRAMNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glBindProgramNV"))) == NULL) || r;
  r = ((glDeleteProgramsNV = (PFNGLDELETEPROGRAMSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteProgramsNV"))) == NULL) || r;
  r = ((glExecuteProgramNV = (PFNGLEXECUTEPROGRAMNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glExecuteProgramNV"))) == NULL) || r;
  r = ((glGenProgramsNV = (PFNGLGENPROGRAMSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenProgramsNV"))) == NULL) || r;
  r = ((glGetProgramParameterdvNV = (PFNGLGETPROGRAMPARAMETERDVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramParameterdvNV"))) == NULL) || r;
  r = ((glGetProgramParameterfvNV = (PFNGLGETPROGRAMPARAMETERFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramParameterfvNV"))) == NULL) || r;
  r = ((glGetProgramStringNV = (PFNGLGETPROGRAMSTRINGNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramStringNV"))) == NULL) || r;
  r = ((glGetProgramivNV = (PFNGLGETPROGRAMIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetProgramivNV"))) == NULL) || r;
  r = ((glGetTrackMatrixivNV = (PFNGLGETTRACKMATRIXIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTrackMatrixivNV"))) == NULL) || r;
  r = ((glGetVertexAttribPointervNV = (PFNGLGETVERTEXATTRIBPOINTERVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribPointervNV"))) == NULL) || r;
  r = ((glGetVertexAttribdvNV = (PFNGLGETVERTEXATTRIBDVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribdvNV"))) == NULL) || r;
  r = ((glGetVertexAttribfvNV = (PFNGLGETVERTEXATTRIBFVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribfvNV"))) == NULL) || r;
  r = ((glGetVertexAttribivNV = (PFNGLGETVERTEXATTRIBIVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetVertexAttribivNV"))) == NULL) || r;
  r = ((glIsProgramNV = (PFNGLISPROGRAMNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsProgramNV"))) == NULL) || r;
  r = ((glLoadProgramNV = (PFNGLLOADPROGRAMNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glLoadProgramNV"))) == NULL) || r;
  r = ((glProgramParameter4dNV = (PFNGLPROGRAMPARAMETER4DNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramParameter4dNV"))) == NULL) || r;
  r = ((glProgramParameter4dvNV = (PFNGLPROGRAMPARAMETER4DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramParameter4dvNV"))) == NULL) || r;
  r = ((glProgramParameter4fNV = (PFNGLPROGRAMPARAMETER4FNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramParameter4fNV"))) == NULL) || r;
  r = ((glProgramParameter4fvNV = (PFNGLPROGRAMPARAMETER4FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramParameter4fvNV"))) == NULL) || r;
  r = ((glProgramParameters4dvNV = (PFNGLPROGRAMPARAMETERS4DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramParameters4dvNV"))) == NULL) || r;
  r = ((glProgramParameters4fvNV = (PFNGLPROGRAMPARAMETERS4FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glProgramParameters4fvNV"))) == NULL) || r;
  r = ((glRequestResidentProgramsNV = (PFNGLREQUESTRESIDENTPROGRAMSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glRequestResidentProgramsNV"))) == NULL) || r;
  r = ((glTrackMatrixNV = (PFNGLTRACKMATRIXNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTrackMatrixNV"))) == NULL) || r;
  r = ((glVertexAttrib1dNV = (PFNGLVERTEXATTRIB1DNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1dNV"))) == NULL) || r;
  r = ((glVertexAttrib1dvNV = (PFNGLVERTEXATTRIB1DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1dvNV"))) == NULL) || r;
  r = ((glVertexAttrib1fNV = (PFNGLVERTEXATTRIB1FNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1fNV"))) == NULL) || r;
  r = ((glVertexAttrib1fvNV = (PFNGLVERTEXATTRIB1FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1fvNV"))) == NULL) || r;
  r = ((glVertexAttrib1sNV = (PFNGLVERTEXATTRIB1SNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1sNV"))) == NULL) || r;
  r = ((glVertexAttrib1svNV = (PFNGLVERTEXATTRIB1SVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib1svNV"))) == NULL) || r;
  r = ((glVertexAttrib2dNV = (PFNGLVERTEXATTRIB2DNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2dNV"))) == NULL) || r;
  r = ((glVertexAttrib2dvNV = (PFNGLVERTEXATTRIB2DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2dvNV"))) == NULL) || r;
  r = ((glVertexAttrib2fNV = (PFNGLVERTEXATTRIB2FNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2fNV"))) == NULL) || r;
  r = ((glVertexAttrib2fvNV = (PFNGLVERTEXATTRIB2FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2fvNV"))) == NULL) || r;
  r = ((glVertexAttrib2sNV = (PFNGLVERTEXATTRIB2SNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2sNV"))) == NULL) || r;
  r = ((glVertexAttrib2svNV = (PFNGLVERTEXATTRIB2SVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib2svNV"))) == NULL) || r;
  r = ((glVertexAttrib3dNV = (PFNGLVERTEXATTRIB3DNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3dNV"))) == NULL) || r;
  r = ((glVertexAttrib3dvNV = (PFNGLVERTEXATTRIB3DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3dvNV"))) == NULL) || r;
  r = ((glVertexAttrib3fNV = (PFNGLVERTEXATTRIB3FNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3fNV"))) == NULL) || r;
  r = ((glVertexAttrib3fvNV = (PFNGLVERTEXATTRIB3FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3fvNV"))) == NULL) || r;
  r = ((glVertexAttrib3sNV = (PFNGLVERTEXATTRIB3SNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3sNV"))) == NULL) || r;
  r = ((glVertexAttrib3svNV = (PFNGLVERTEXATTRIB3SVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib3svNV"))) == NULL) || r;
  r = ((glVertexAttrib4dNV = (PFNGLVERTEXATTRIB4DNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4dNV"))) == NULL) || r;
  r = ((glVertexAttrib4dvNV = (PFNGLVERTEXATTRIB4DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4dvNV"))) == NULL) || r;
  r = ((glVertexAttrib4fNV = (PFNGLVERTEXATTRIB4FNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4fNV"))) == NULL) || r;
  r = ((glVertexAttrib4fvNV = (PFNGLVERTEXATTRIB4FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4fvNV"))) == NULL) || r;
  r = ((glVertexAttrib4sNV = (PFNGLVERTEXATTRIB4SNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4sNV"))) == NULL) || r;
  r = ((glVertexAttrib4svNV = (PFNGLVERTEXATTRIB4SVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4svNV"))) == NULL) || r;
  r = ((glVertexAttrib4ubNV = (PFNGLVERTEXATTRIB4UBNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4ubNV"))) == NULL) || r;
  r = ((glVertexAttrib4ubvNV = (PFNGLVERTEXATTRIB4UBVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttrib4ubvNV"))) == NULL) || r;
  r = ((glVertexAttribPointerNV = (PFNGLVERTEXATTRIBPOINTERNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribPointerNV"))) == NULL) || r;
  r = ((glVertexAttribs1dvNV = (PFNGLVERTEXATTRIBS1DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs1dvNV"))) == NULL) || r;
  r = ((glVertexAttribs1fvNV = (PFNGLVERTEXATTRIBS1FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs1fvNV"))) == NULL) || r;
  r = ((glVertexAttribs1svNV = (PFNGLVERTEXATTRIBS1SVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs1svNV"))) == NULL) || r;
  r = ((glVertexAttribs2dvNV = (PFNGLVERTEXATTRIBS2DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs2dvNV"))) == NULL) || r;
  r = ((glVertexAttribs2fvNV = (PFNGLVERTEXATTRIBS2FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs2fvNV"))) == NULL) || r;
  r = ((glVertexAttribs2svNV = (PFNGLVERTEXATTRIBS2SVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs2svNV"))) == NULL) || r;
  r = ((glVertexAttribs3dvNV = (PFNGLVERTEXATTRIBS3DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs3dvNV"))) == NULL) || r;
  r = ((glVertexAttribs3fvNV = (PFNGLVERTEXATTRIBS3FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs3fvNV"))) == NULL) || r;
  r = ((glVertexAttribs3svNV = (PFNGLVERTEXATTRIBS3SVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs3svNV"))) == NULL) || r;
  r = ((glVertexAttribs4dvNV = (PFNGLVERTEXATTRIBS4DVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs4dvNV"))) == NULL) || r;
  r = ((glVertexAttribs4fvNV = (PFNGLVERTEXATTRIBS4FVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs4fvNV"))) == NULL) || r;
  r = ((glVertexAttribs4svNV = (PFNGLVERTEXATTRIBS4SVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs4svNV"))) == NULL) || r;
  r = ((glVertexAttribs4ubvNV = (PFNGLVERTEXATTRIBS4UBVNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glVertexAttribs4ubvNV"))) == NULL) || r;

  return r;
}

#endif /* GL_NV_vertex_program */

#ifdef GL_NV_vertex_program1_1

#endif /* GL_NV_vertex_program1_1 */

#ifdef GL_NV_vertex_program2

#endif /* GL_NV_vertex_program2 */

#ifdef GL_NV_vertex_program2_option

#endif /* GL_NV_vertex_program2_option */

#ifdef GL_NV_vertex_program3

#endif /* GL_NV_vertex_program3 */

#ifdef GL_NV_vertex_program4

#endif /* GL_NV_vertex_program4 */

#ifdef GL_OES_byte_coordinates

#endif /* GL_OES_byte_coordinates */

#ifdef GL_OES_compressed_paletted_texture

#endif /* GL_OES_compressed_paletted_texture */

#ifdef GL_OES_read_format

#endif /* GL_OES_read_format */

#ifdef GL_OES_single_precision

static GLboolean _glewInit_GL_OES_single_precision (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClearDepthfOES = (PFNGLCLEARDEPTHFOESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClearDepthfOES"))) == NULL) || r;
  r = ((glClipPlanefOES = (PFNGLCLIPPLANEFOESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glClipPlanefOES"))) == NULL) || r;
  r = ((glDepthRangefOES = (PFNGLDEPTHRANGEFOESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDepthRangefOES"))) == NULL) || r;
  r = ((glFrustumfOES = (PFNGLFRUSTUMFOESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFrustumfOES"))) == NULL) || r;
  r = ((glGetClipPlanefOES = (PFNGLGETCLIPPLANEFOESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetClipPlanefOES"))) == NULL) || r;
  r = ((glOrthofOES = (PFNGLORTHOFOESPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glOrthofOES"))) == NULL) || r;

  return r;
}

#endif /* GL_OES_single_precision */

#ifdef GL_OML_interlace

#endif /* GL_OML_interlace */

#ifdef GL_OML_resample

#endif /* GL_OML_resample */

#ifdef GL_OML_subsample

#endif /* GL_OML_subsample */

#ifdef GL_PGI_misc_hints

#endif /* GL_PGI_misc_hints */

#ifdef GL_PGI_vertex_hints

#endif /* GL_PGI_vertex_hints */

#ifdef GL_REND_screen_coordinates

#endif /* GL_REND_screen_coordinates */

#ifdef GL_S3_s3tc

#endif /* GL_S3_s3tc */

#ifdef GL_SGIS_color_range

#endif /* GL_SGIS_color_range */

#ifdef GL_SGIS_detail_texture

static GLboolean _glewInit_GL_SGIS_detail_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDetailTexFuncSGIS = (PFNGLDETAILTEXFUNCSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDetailTexFuncSGIS"))) == NULL) || r;
  r = ((glGetDetailTexFuncSGIS = (PFNGLGETDETAILTEXFUNCSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetDetailTexFuncSGIS"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIS_detail_texture */

#ifdef GL_SGIS_fog_function

static GLboolean _glewInit_GL_SGIS_fog_function (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFogFuncSGIS = (PFNGLFOGFUNCSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFogFuncSGIS"))) == NULL) || r;
  r = ((glGetFogFuncSGIS = (PFNGLGETFOGFUNCSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFogFuncSGIS"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIS_fog_function */

#ifdef GL_SGIS_generate_mipmap

#endif /* GL_SGIS_generate_mipmap */

#ifdef GL_SGIS_multisample

static GLboolean _glewInit_GL_SGIS_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSampleMaskSGIS = (PFNGLSAMPLEMASKSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSampleMaskSGIS"))) == NULL) || r;
  r = ((glSamplePatternSGIS = (PFNGLSAMPLEPATTERNSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSamplePatternSGIS"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIS_multisample */

#ifdef GL_SGIS_pixel_texture

#endif /* GL_SGIS_pixel_texture */

#ifdef GL_SGIS_point_line_texgen

#endif /* GL_SGIS_point_line_texgen */

#ifdef GL_SGIS_sharpen_texture

static GLboolean _glewInit_GL_SGIS_sharpen_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetSharpenTexFuncSGIS = (PFNGLGETSHARPENTEXFUNCSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetSharpenTexFuncSGIS"))) == NULL) || r;
  r = ((glSharpenTexFuncSGIS = (PFNGLSHARPENTEXFUNCSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSharpenTexFuncSGIS"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIS_sharpen_texture */

#ifdef GL_SGIS_texture4D

static GLboolean _glewInit_GL_SGIS_texture4D (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexImage4DSGIS = (PFNGLTEXIMAGE4DSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexImage4DSGIS"))) == NULL) || r;
  r = ((glTexSubImage4DSGIS = (PFNGLTEXSUBIMAGE4DSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexSubImage4DSGIS"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIS_texture4D */

#ifdef GL_SGIS_texture_border_clamp

#endif /* GL_SGIS_texture_border_clamp */

#ifdef GL_SGIS_texture_edge_clamp

#endif /* GL_SGIS_texture_edge_clamp */

#ifdef GL_SGIS_texture_filter4

static GLboolean _glewInit_GL_SGIS_texture_filter4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetTexFilterFuncSGIS = (PFNGLGETTEXFILTERFUNCSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetTexFilterFuncSGIS"))) == NULL) || r;
  r = ((glTexFilterFuncSGIS = (PFNGLTEXFILTERFUNCSGISPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexFilterFuncSGIS"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIS_texture_filter4 */

#ifdef GL_SGIS_texture_lod

#endif /* GL_SGIS_texture_lod */

#ifdef GL_SGIS_texture_select

#endif /* GL_SGIS_texture_select */

#ifdef GL_SGIX_async

static GLboolean _glewInit_GL_SGIX_async (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAsyncMarkerSGIX = (PFNGLASYNCMARKERSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAsyncMarkerSGIX"))) == NULL) || r;
  r = ((glDeleteAsyncMarkersSGIX = (PFNGLDELETEASYNCMARKERSSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glDeleteAsyncMarkersSGIX"))) == NULL) || r;
  r = ((glFinishAsyncSGIX = (PFNGLFINISHASYNCSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFinishAsyncSGIX"))) == NULL) || r;
  r = ((glGenAsyncMarkersSGIX = (PFNGLGENASYNCMARKERSSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGenAsyncMarkersSGIX"))) == NULL) || r;
  r = ((glIsAsyncMarkerSGIX = (PFNGLISASYNCMARKERSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glIsAsyncMarkerSGIX"))) == NULL) || r;
  r = ((glPollAsyncSGIX = (PFNGLPOLLASYNCSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPollAsyncSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_async */

#ifdef GL_SGIX_async_histogram

#endif /* GL_SGIX_async_histogram */

#ifdef GL_SGIX_async_pixel

#endif /* GL_SGIX_async_pixel */

#ifdef GL_SGIX_blend_alpha_minmax

#endif /* GL_SGIX_blend_alpha_minmax */

#ifdef GL_SGIX_clipmap

#endif /* GL_SGIX_clipmap */

#ifdef GL_SGIX_convolution_accuracy

#endif /* GL_SGIX_convolution_accuracy */

#ifdef GL_SGIX_depth_texture

#endif /* GL_SGIX_depth_texture */

#ifdef GL_SGIX_flush_raster

static GLboolean _glewInit_GL_SGIX_flush_raster (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushRasterSGIX = (PFNGLFLUSHRASTERSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFlushRasterSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_flush_raster */

#ifdef GL_SGIX_fog_offset

#endif /* GL_SGIX_fog_offset */

#ifdef GL_SGIX_fog_texture

static GLboolean _glewInit_GL_SGIX_fog_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTextureFogSGIX = (PFNGLTEXTUREFOGSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTextureFogSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_fog_texture */

#ifdef GL_SGIX_fragment_specular_lighting

static GLboolean _glewInit_GL_SGIX_fragment_specular_lighting (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFragmentColorMaterialSGIX = (PFNGLFRAGMENTCOLORMATERIALSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentColorMaterialSGIX"))) == NULL) || r;
  r = ((glFragmentLightModelfSGIX = (PFNGLFRAGMENTLIGHTMODELFSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightModelfSGIX"))) == NULL) || r;
  r = ((glFragmentLightModelfvSGIX = (PFNGLFRAGMENTLIGHTMODELFVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightModelfvSGIX"))) == NULL) || r;
  r = ((glFragmentLightModeliSGIX = (PFNGLFRAGMENTLIGHTMODELISGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightModeliSGIX"))) == NULL) || r;
  r = ((glFragmentLightModelivSGIX = (PFNGLFRAGMENTLIGHTMODELIVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightModelivSGIX"))) == NULL) || r;
  r = ((glFragmentLightfSGIX = (PFNGLFRAGMENTLIGHTFSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightfSGIX"))) == NULL) || r;
  r = ((glFragmentLightfvSGIX = (PFNGLFRAGMENTLIGHTFVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightfvSGIX"))) == NULL) || r;
  r = ((glFragmentLightiSGIX = (PFNGLFRAGMENTLIGHTISGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightiSGIX"))) == NULL) || r;
  r = ((glFragmentLightivSGIX = (PFNGLFRAGMENTLIGHTIVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentLightivSGIX"))) == NULL) || r;
  r = ((glFragmentMaterialfSGIX = (PFNGLFRAGMENTMATERIALFSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentMaterialfSGIX"))) == NULL) || r;
  r = ((glFragmentMaterialfvSGIX = (PFNGLFRAGMENTMATERIALFVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentMaterialfvSGIX"))) == NULL) || r;
  r = ((glFragmentMaterialiSGIX = (PFNGLFRAGMENTMATERIALISGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentMaterialiSGIX"))) == NULL) || r;
  r = ((glFragmentMaterialivSGIX = (PFNGLFRAGMENTMATERIALIVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFragmentMaterialivSGIX"))) == NULL) || r;
  r = ((glGetFragmentLightfvSGIX = (PFNGLGETFRAGMENTLIGHTFVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragmentLightfvSGIX"))) == NULL) || r;
  r = ((glGetFragmentLightivSGIX = (PFNGLGETFRAGMENTLIGHTIVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragmentLightivSGIX"))) == NULL) || r;
  r = ((glGetFragmentMaterialfvSGIX = (PFNGLGETFRAGMENTMATERIALFVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragmentMaterialfvSGIX"))) == NULL) || r;
  r = ((glGetFragmentMaterialivSGIX = (PFNGLGETFRAGMENTMATERIALIVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetFragmentMaterialivSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_fragment_specular_lighting */

#ifdef GL_SGIX_framezoom

static GLboolean _glewInit_GL_SGIX_framezoom (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFrameZoomSGIX = (PFNGLFRAMEZOOMSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFrameZoomSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_framezoom */

#ifdef GL_SGIX_interlace

#endif /* GL_SGIX_interlace */

#ifdef GL_SGIX_ir_instrument1

#endif /* GL_SGIX_ir_instrument1 */

#ifdef GL_SGIX_list_priority

#endif /* GL_SGIX_list_priority */

#ifdef GL_SGIX_pixel_texture

static GLboolean _glewInit_GL_SGIX_pixel_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPixelTexGenSGIX = (PFNGLPIXELTEXGENSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glPixelTexGenSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_pixel_texture */

#ifdef GL_SGIX_pixel_texture_bits

#endif /* GL_SGIX_pixel_texture_bits */

#ifdef GL_SGIX_reference_plane

static GLboolean _glewInit_GL_SGIX_reference_plane (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glReferencePlaneSGIX = (PFNGLREFERENCEPLANESGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReferencePlaneSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_reference_plane */

#ifdef GL_SGIX_resample

#endif /* GL_SGIX_resample */

#ifdef GL_SGIX_shadow

#endif /* GL_SGIX_shadow */

#ifdef GL_SGIX_shadow_ambient

#endif /* GL_SGIX_shadow_ambient */

#ifdef GL_SGIX_sprite

static GLboolean _glewInit_GL_SGIX_sprite (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSpriteParameterfSGIX = (PFNGLSPRITEPARAMETERFSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSpriteParameterfSGIX"))) == NULL) || r;
  r = ((glSpriteParameterfvSGIX = (PFNGLSPRITEPARAMETERFVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSpriteParameterfvSGIX"))) == NULL) || r;
  r = ((glSpriteParameteriSGIX = (PFNGLSPRITEPARAMETERISGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSpriteParameteriSGIX"))) == NULL) || r;
  r = ((glSpriteParameterivSGIX = (PFNGLSPRITEPARAMETERIVSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glSpriteParameterivSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_sprite */

#ifdef GL_SGIX_tag_sample_buffer

static GLboolean _glewInit_GL_SGIX_tag_sample_buffer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTagSampleBufferSGIX = (PFNGLTAGSAMPLEBUFFERSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTagSampleBufferSGIX"))) == NULL) || r;

  return r;
}

#endif /* GL_SGIX_tag_sample_buffer */

#ifdef GL_SGIX_texture_add_env

#endif /* GL_SGIX_texture_add_env */

#ifdef GL_SGIX_texture_coordinate_clamp

#endif /* GL_SGIX_texture_coordinate_clamp */

#ifdef GL_SGIX_texture_lod_bias

#endif /* GL_SGIX_texture_lod_bias */

#ifdef GL_SGIX_texture_multi_buffer

#endif /* GL_SGIX_texture_multi_buffer */

#ifdef GL_SGIX_texture_range

#endif /* GL_SGIX_texture_range */

#ifdef GL_SGIX_texture_scale_bias

#endif /* GL_SGIX_texture_scale_bias */

#ifdef GL_SGIX_vertex_preclip

#endif /* GL_SGIX_vertex_preclip */

#ifdef GL_SGIX_vertex_preclip_hint

#endif /* GL_SGIX_vertex_preclip_hint */

#ifdef GL_SGIX_ycrcb

#endif /* GL_SGIX_ycrcb */

#ifdef GL_SGI_color_matrix

#endif /* GL_SGI_color_matrix */

#ifdef GL_SGI_color_table

static GLboolean _glewInit_GL_SGI_color_table (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorTableParameterfvSGI = (PFNGLCOLORTABLEPARAMETERFVSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorTableParameterfvSGI"))) == NULL) || r;
  r = ((glColorTableParameterivSGI = (PFNGLCOLORTABLEPARAMETERIVSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorTableParameterivSGI"))) == NULL) || r;
  r = ((glColorTableSGI = (PFNGLCOLORTABLESGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColorTableSGI"))) == NULL) || r;
  r = ((glCopyColorTableSGI = (PFNGLCOPYCOLORTABLESGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glCopyColorTableSGI"))) == NULL) || r;
  r = ((glGetColorTableParameterfvSGI = (PFNGLGETCOLORTABLEPARAMETERFVSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTableParameterfvSGI"))) == NULL) || r;
  r = ((glGetColorTableParameterivSGI = (PFNGLGETCOLORTABLEPARAMETERIVSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTableParameterivSGI"))) == NULL) || r;
  r = ((glGetColorTableSGI = (PFNGLGETCOLORTABLESGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGetColorTableSGI"))) == NULL) || r;

  return r;
}

#endif /* GL_SGI_color_table */

#ifdef GL_SGI_texture_color_table

#endif /* GL_SGI_texture_color_table */

#ifdef GL_SUNX_constant_data

static GLboolean _glewInit_GL_SUNX_constant_data (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFinishTextureSUNX = (PFNGLFINISHTEXTURESUNXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glFinishTextureSUNX"))) == NULL) || r;

  return r;
}

#endif /* GL_SUNX_constant_data */

#ifdef GL_SUN_convolution_border_modes

#endif /* GL_SUN_convolution_border_modes */

#ifdef GL_SUN_global_alpha

static GLboolean _glewInit_GL_SUN_global_alpha (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGlobalAlphaFactorbSUN = (PFNGLGLOBALALPHAFACTORBSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGlobalAlphaFactorbSUN"))) == NULL) || r;
  r = ((glGlobalAlphaFactordSUN = (PFNGLGLOBALALPHAFACTORDSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGlobalAlphaFactordSUN"))) == NULL) || r;
  r = ((glGlobalAlphaFactorfSUN = (PFNGLGLOBALALPHAFACTORFSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGlobalAlphaFactorfSUN"))) == NULL) || r;
  r = ((glGlobalAlphaFactoriSUN = (PFNGLGLOBALALPHAFACTORISUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGlobalAlphaFactoriSUN"))) == NULL) || r;
  r = ((glGlobalAlphaFactorsSUN = (PFNGLGLOBALALPHAFACTORSSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGlobalAlphaFactorsSUN"))) == NULL) || r;
  r = ((glGlobalAlphaFactorubSUN = (PFNGLGLOBALALPHAFACTORUBSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGlobalAlphaFactorubSUN"))) == NULL) || r;
  r = ((glGlobalAlphaFactoruiSUN = (PFNGLGLOBALALPHAFACTORUISUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGlobalAlphaFactoruiSUN"))) == NULL) || r;
  r = ((glGlobalAlphaFactorusSUN = (PFNGLGLOBALALPHAFACTORUSSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glGlobalAlphaFactorusSUN"))) == NULL) || r;

  return r;
}

#endif /* GL_SUN_global_alpha */

#ifdef GL_SUN_mesh_array

#endif /* GL_SUN_mesh_array */

#ifdef GL_SUN_read_video_pixels

static GLboolean _glewInit_GL_SUN_read_video_pixels (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glReadVideoPixelsSUN = (PFNGLREADVIDEOPIXELSSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReadVideoPixelsSUN"))) == NULL) || r;

  return r;
}

#endif /* GL_SUN_read_video_pixels */

#ifdef GL_SUN_slice_accum

#endif /* GL_SUN_slice_accum */

#ifdef GL_SUN_triangle_list

static GLboolean _glewInit_GL_SUN_triangle_list (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glReplacementCodePointerSUN = (PFNGLREPLACEMENTCODEPOINTERSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodePointerSUN"))) == NULL) || r;
  r = ((glReplacementCodeubSUN = (PFNGLREPLACEMENTCODEUBSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeubSUN"))) == NULL) || r;
  r = ((glReplacementCodeubvSUN = (PFNGLREPLACEMENTCODEUBVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeubvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiSUN = (PFNGLREPLACEMENTCODEUISUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiSUN"))) == NULL) || r;
  r = ((glReplacementCodeuivSUN = (PFNGLREPLACEMENTCODEUIVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuivSUN"))) == NULL) || r;
  r = ((glReplacementCodeusSUN = (PFNGLREPLACEMENTCODEUSSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeusSUN"))) == NULL) || r;
  r = ((glReplacementCodeusvSUN = (PFNGLREPLACEMENTCODEUSVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeusvSUN"))) == NULL) || r;

  return r;
}

#endif /* GL_SUN_triangle_list */

#ifdef GL_SUN_vertex

static GLboolean _glewInit_GL_SUN_vertex (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColor3fVertex3fSUN = (PFNGLCOLOR3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3fVertex3fSUN"))) == NULL) || r;
  r = ((glColor3fVertex3fvSUN = (PFNGLCOLOR3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor3fVertex3fvSUN"))) == NULL) || r;
  r = ((glColor4fNormal3fVertex3fSUN = (PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4fNormal3fVertex3fSUN"))) == NULL) || r;
  r = ((glColor4fNormal3fVertex3fvSUN = (PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4fNormal3fVertex3fvSUN"))) == NULL) || r;
  r = ((glColor4ubVertex2fSUN = (PFNGLCOLOR4UBVERTEX2FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4ubVertex2fSUN"))) == NULL) || r;
  r = ((glColor4ubVertex2fvSUN = (PFNGLCOLOR4UBVERTEX2FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4ubVertex2fvSUN"))) == NULL) || r;
  r = ((glColor4ubVertex3fSUN = (PFNGLCOLOR4UBVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4ubVertex3fSUN"))) == NULL) || r;
  r = ((glColor4ubVertex3fvSUN = (PFNGLCOLOR4UBVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glColor4ubVertex3fvSUN"))) == NULL) || r;
  r = ((glNormal3fVertex3fSUN = (PFNGLNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3fVertex3fSUN"))) == NULL) || r;
  r = ((glNormal3fVertex3fvSUN = (PFNGLNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glNormal3fVertex3fvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiColor3fVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiColor3fVertex3fSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiColor3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiColor3fVertex3fvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiColor4fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiColor4fNormal3fVertex3fSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiColor4fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiColor4fNormal3fVertex3fvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiColor4ubVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiColor4ubVertex3fSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiColor4ubVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiColor4ubVertex3fvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiNormal3fVertex3fSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiNormal3fVertex3fvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiTexCoord2fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiTexCoord2fVertex3fSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiTexCoord2fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiTexCoord2fVertex3fvSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiVertex3fSUN = (PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiVertex3fSUN"))) == NULL) || r;
  r = ((glReplacementCodeuiVertex3fvSUN = (PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glReplacementCodeuiVertex3fvSUN"))) == NULL) || r;
  r = ((glTexCoord2fColor3fVertex3fSUN = (PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fColor3fVertex3fSUN"))) == NULL) || r;
  r = ((glTexCoord2fColor3fVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fColor3fVertex3fvSUN"))) == NULL) || r;
  r = ((glTexCoord2fColor4fNormal3fVertex3fSUN = (PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fColor4fNormal3fVertex3fSUN"))) == NULL) || r;
  r = ((glTexCoord2fColor4fNormal3fVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fColor4fNormal3fVertex3fvSUN"))) == NULL) || r;
  r = ((glTexCoord2fColor4ubVertex3fSUN = (PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fColor4ubVertex3fSUN"))) == NULL) || r;
  r = ((glTexCoord2fColor4ubVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fColor4ubVertex3fvSUN"))) == NULL) || r;
  r = ((glTexCoord2fNormal3fVertex3fSUN = (PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fNormal3fVertex3fSUN"))) == NULL) || r;
  r = ((glTexCoord2fNormal3fVertex3fvSUN = (PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fNormal3fVertex3fvSUN"))) == NULL) || r;
  r = ((glTexCoord2fVertex3fSUN = (PFNGLTEXCOORD2FVERTEX3FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fVertex3fSUN"))) == NULL) || r;
  r = ((glTexCoord2fVertex3fvSUN = (PFNGLTEXCOORD2FVERTEX3FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord2fVertex3fvSUN"))) == NULL) || r;
  r = ((glTexCoord4fColor4fNormal3fVertex4fSUN = (PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4fColor4fNormal3fVertex4fSUN"))) == NULL) || r;
  r = ((glTexCoord4fColor4fNormal3fVertex4fvSUN = (PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4fColor4fNormal3fVertex4fvSUN"))) == NULL) || r;
  r = ((glTexCoord4fVertex4fSUN = (PFNGLTEXCOORD4FVERTEX4FSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4fVertex4fSUN"))) == NULL) || r;
  r = ((glTexCoord4fVertex4fvSUN = (PFNGLTEXCOORD4FVERTEX4FVSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glTexCoord4fVertex4fvSUN"))) == NULL) || r;

  return r;
}

#endif /* GL_SUN_vertex */

#ifdef GL_WIN_phong_shading

#endif /* GL_WIN_phong_shading */

#ifdef GL_WIN_specular_fog

#endif /* GL_WIN_specular_fog */

#ifdef GL_WIN_swap_hint

static GLboolean _glewInit_GL_WIN_swap_hint (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAddSwapHintRectWIN = (PFNGLADDSWAPHINTRECTWINPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glAddSwapHintRectWIN"))) == NULL) || r;

  return r;
}

#endif /* GL_WIN_swap_hint */

/* ------------------------------------------------------------------------- */

/* 
 * Search for name in the extensions string. Use of strstr()
 * is not sufficient because extension names can be prefixes of
 * other extension names. Could use strtok() but the constant
 * string returned by glGetString might be in read-only memory.
 */
GLboolean glewGetExtension (const char* name)
{    
  GLubyte* p;
  GLubyte* end;
  GLuint len = _glewStrLen((const GLubyte*)name);
  p = (GLubyte*)glGetString(GL_EXTENSIONS);
  if (0 == p) return GL_FALSE;
  end = p + _glewStrLen(p);
  while (p < end)
  {
    GLuint n = _glewStrCLen(p, ' ');
    if (len == n && _glewStrSame((const GLubyte*)name, p, n)) return GL_TRUE;
    p += n+1;
  }
  return GL_FALSE;
}

/* ------------------------------------------------------------------------- */

#ifndef GLEW_MX
static
#endif
GLenum glewContextInit (GLEW_CONTEXT_ARG_DEF_LIST)
{
  const GLubyte* s;
  GLuint dot;
  GLint major, minor;

  /* Since we're still initializing, the `GetString' function pointer hasn't
   * been loaded yet. */
  __glewGetString = (PFNGLGETSTRINGPROC)glewGetProcAddress(
                      fqn_from_convention(glew_convention, "glGetString")
                    );
  /* query opengl version */
  s = __glewGetString(GL_VERSION);

  dot = _glewStrCLen(s, '.');
  if (dot == 0)
    return GLEW_ERROR_NO_GL_VERSION;
  
  major = s[dot-1]-'0';
  minor = s[dot+1]-'0';

  if (minor < 0 || minor > 9)
    minor = 0;
  if (major<0 || major>9)
    return GLEW_ERROR_NO_GL_VERSION;
  

  if (major == 1 && minor == 0)
  {
    return GLEW_ERROR_GL_VERSION_10_ONLY;
  }
  else
  {
    CONST_CAST(GLEW_VERSION_3_0) =                                ( major >= 3               ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_2_1) = GLEW_VERSION_3_0 == GL_TRUE || ( major == 2 && minor >= 1 ) ? GL_TRUE : GL_FALSE;    
    CONST_CAST(GLEW_VERSION_2_0) = GLEW_VERSION_2_1 == GL_TRUE || ( major == 2               ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_5) = GLEW_VERSION_2_0 == GL_TRUE || ( major == 1 && minor >= 5 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_4) = GLEW_VERSION_1_5 == GL_TRUE || ( major == 1 && minor >= 4 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_3) = GLEW_VERSION_1_4 == GL_TRUE || ( major == 1 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_2) = GLEW_VERSION_1_3 == GL_TRUE || ( major == 1 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_1) = GLEW_VERSION_1_2 == GL_TRUE || ( major == 1 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
  }
  /* initialize extensions */
#ifdef GL_VERSION_1_1
  if (glewExperimental || GLEW_VERSION_1_1) CONST_CAST(GLEW_VERSION_1_1) = !_glewInit_GL_VERSION_1_1(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_1 */
#ifdef GL_VERSION_1_2
  if (glewExperimental || GLEW_VERSION_1_2) CONST_CAST(GLEW_VERSION_1_2) = !_glewInit_GL_VERSION_1_2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_2 */
#ifdef GL_VERSION_1_3
  if (glewExperimental || GLEW_VERSION_1_3) CONST_CAST(GLEW_VERSION_1_3) = !_glewInit_GL_VERSION_1_3(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_3 */
#ifdef GL_VERSION_1_4
  if (glewExperimental || GLEW_VERSION_1_4) CONST_CAST(GLEW_VERSION_1_4) = !_glewInit_GL_VERSION_1_4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_4 */
#ifdef GL_VERSION_1_5
  if (glewExperimental || GLEW_VERSION_1_5) CONST_CAST(GLEW_VERSION_1_5) = !_glewInit_GL_VERSION_1_5(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_5 */
#ifdef GL_VERSION_2_0
  if (glewExperimental || GLEW_VERSION_2_0) CONST_CAST(GLEW_VERSION_2_0) = !_glewInit_GL_VERSION_2_0(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_2_0 */
#ifdef GL_VERSION_2_1
  if (glewExperimental || GLEW_VERSION_2_1) CONST_CAST(GLEW_VERSION_2_1) = !_glewInit_GL_VERSION_2_1(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_2_1 */
#ifdef GL_VERSION_3_0
  if (glewExperimental || GLEW_VERSION_3_0) CONST_CAST(GLEW_VERSION_3_0) = !_glewInit_GL_VERSION_3_0(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_3_0 */
#ifdef GL_3DFX_multisample
  CONST_CAST(GLEW_3DFX_multisample) = glewGetExtension("GL_3DFX_multisample");
#endif /* GL_3DFX_multisample */
#ifdef GL_3DFX_tbuffer
  CONST_CAST(GLEW_3DFX_tbuffer) = glewGetExtension("GL_3DFX_tbuffer");
  if (glewExperimental || GLEW_3DFX_tbuffer) CONST_CAST(GLEW_3DFX_tbuffer) = !_glewInit_GL_3DFX_tbuffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_3DFX_tbuffer */
#ifdef GL_3DFX_texture_compression_FXT1
  CONST_CAST(GLEW_3DFX_texture_compression_FXT1) = glewGetExtension("GL_3DFX_texture_compression_FXT1");
#endif /* GL_3DFX_texture_compression_FXT1 */
#ifdef GL_APPLE_client_storage
  CONST_CAST(GLEW_APPLE_client_storage) = glewGetExtension("GL_APPLE_client_storage");
#endif /* GL_APPLE_client_storage */
#ifdef GL_APPLE_element_array
  CONST_CAST(GLEW_APPLE_element_array) = glewGetExtension("GL_APPLE_element_array");
  if (glewExperimental || GLEW_APPLE_element_array) CONST_CAST(GLEW_APPLE_element_array) = !_glewInit_GL_APPLE_element_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_element_array */
#ifdef GL_APPLE_fence
  CONST_CAST(GLEW_APPLE_fence) = glewGetExtension("GL_APPLE_fence");
  if (glewExperimental || GLEW_APPLE_fence) CONST_CAST(GLEW_APPLE_fence) = !_glewInit_GL_APPLE_fence(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_fence */
#ifdef GL_APPLE_float_pixels
  CONST_CAST(GLEW_APPLE_float_pixels) = glewGetExtension("GL_APPLE_float_pixels");
#endif /* GL_APPLE_float_pixels */
#ifdef GL_APPLE_flush_buffer_range
  CONST_CAST(GLEW_APPLE_flush_buffer_range) = glewGetExtension("GL_APPLE_flush_buffer_range");
  if (glewExperimental || GLEW_APPLE_flush_buffer_range) CONST_CAST(GLEW_APPLE_flush_buffer_range) = !_glewInit_GL_APPLE_flush_buffer_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_flush_buffer_range */
#ifdef GL_APPLE_pixel_buffer
  CONST_CAST(GLEW_APPLE_pixel_buffer) = glewGetExtension("GL_APPLE_pixel_buffer");
#endif /* GL_APPLE_pixel_buffer */
#ifdef GL_APPLE_specular_vector
  CONST_CAST(GLEW_APPLE_specular_vector) = glewGetExtension("GL_APPLE_specular_vector");
#endif /* GL_APPLE_specular_vector */
#ifdef GL_APPLE_texture_range
  CONST_CAST(GLEW_APPLE_texture_range) = glewGetExtension("GL_APPLE_texture_range");
  if (glewExperimental || GLEW_APPLE_texture_range) CONST_CAST(GLEW_APPLE_texture_range) = !_glewInit_GL_APPLE_texture_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_texture_range */
#ifdef GL_APPLE_transform_hint
  CONST_CAST(GLEW_APPLE_transform_hint) = glewGetExtension("GL_APPLE_transform_hint");
#endif /* GL_APPLE_transform_hint */
#ifdef GL_APPLE_vertex_array_object
  CONST_CAST(GLEW_APPLE_vertex_array_object) = glewGetExtension("GL_APPLE_vertex_array_object");
  if (glewExperimental || GLEW_APPLE_vertex_array_object) CONST_CAST(GLEW_APPLE_vertex_array_object) = !_glewInit_GL_APPLE_vertex_array_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_vertex_array_object */
#ifdef GL_APPLE_vertex_array_range
  CONST_CAST(GLEW_APPLE_vertex_array_range) = glewGetExtension("GL_APPLE_vertex_array_range");
  if (glewExperimental || GLEW_APPLE_vertex_array_range) CONST_CAST(GLEW_APPLE_vertex_array_range) = !_glewInit_GL_APPLE_vertex_array_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_vertex_array_range */
#ifdef GL_APPLE_ycbcr_422
  CONST_CAST(GLEW_APPLE_ycbcr_422) = glewGetExtension("GL_APPLE_ycbcr_422");
#endif /* GL_APPLE_ycbcr_422 */
#ifdef GL_ARB_color_buffer_float
  CONST_CAST(GLEW_ARB_color_buffer_float) = glewGetExtension("GL_ARB_color_buffer_float");
  if (glewExperimental || GLEW_ARB_color_buffer_float) CONST_CAST(GLEW_ARB_color_buffer_float) = !_glewInit_GL_ARB_color_buffer_float(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_color_buffer_float */
#ifdef GL_ARB_depth_buffer_float
  CONST_CAST(GLEW_ARB_depth_buffer_float) = glewGetExtension("GL_ARB_depth_buffer_float");
#endif /* GL_ARB_depth_buffer_float */
#ifdef GL_ARB_depth_texture
  CONST_CAST(GLEW_ARB_depth_texture) = glewGetExtension("GL_ARB_depth_texture");
#endif /* GL_ARB_depth_texture */
#ifdef GL_ARB_draw_buffers
  CONST_CAST(GLEW_ARB_draw_buffers) = glewGetExtension("GL_ARB_draw_buffers");
  if (glewExperimental || GLEW_ARB_draw_buffers) CONST_CAST(GLEW_ARB_draw_buffers) = !_glewInit_GL_ARB_draw_buffers(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_draw_buffers */
#ifdef GL_ARB_draw_instanced
  CONST_CAST(GLEW_ARB_draw_instanced) = glewGetExtension("GL_ARB_draw_instanced");
  if (glewExperimental || GLEW_ARB_draw_instanced) CONST_CAST(GLEW_ARB_draw_instanced) = !_glewInit_GL_ARB_draw_instanced(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_draw_instanced */
#ifdef GL_ARB_fragment_program
  CONST_CAST(GLEW_ARB_fragment_program) = glewGetExtension("GL_ARB_fragment_program");
#endif /* GL_ARB_fragment_program */
#ifdef GL_ARB_fragment_program_shadow
  CONST_CAST(GLEW_ARB_fragment_program_shadow) = glewGetExtension("GL_ARB_fragment_program_shadow");
#endif /* GL_ARB_fragment_program_shadow */
#ifdef GL_ARB_fragment_shader
  CONST_CAST(GLEW_ARB_fragment_shader) = glewGetExtension("GL_ARB_fragment_shader");
#endif /* GL_ARB_fragment_shader */
#ifdef GL_ARB_framebuffer_object
  CONST_CAST(GLEW_ARB_framebuffer_object) = glewGetExtension("GL_ARB_framebuffer_object");
  if (glewExperimental || GLEW_ARB_framebuffer_object) CONST_CAST(GLEW_ARB_framebuffer_object) = !_glewInit_GL_ARB_framebuffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_framebuffer_object */
#ifdef GL_ARB_framebuffer_sRGB
  CONST_CAST(GLEW_ARB_framebuffer_sRGB) = glewGetExtension("GL_ARB_framebuffer_sRGB");
#endif /* GL_ARB_framebuffer_sRGB */
#ifdef GL_ARB_geometry_shader4
  CONST_CAST(GLEW_ARB_geometry_shader4) = glewGetExtension("GL_ARB_geometry_shader4");
  if (glewExperimental || GLEW_ARB_geometry_shader4) CONST_CAST(GLEW_ARB_geometry_shader4) = !_glewInit_GL_ARB_geometry_shader4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_geometry_shader4 */
#ifdef GL_ARB_half_float_pixel
  CONST_CAST(GLEW_ARB_half_float_pixel) = glewGetExtension("GL_ARB_half_float_pixel");
#endif /* GL_ARB_half_float_pixel */
#ifdef GL_ARB_half_float_vertex
  CONST_CAST(GLEW_ARB_half_float_vertex) = glewGetExtension("GL_ARB_half_float_vertex");
#endif /* GL_ARB_half_float_vertex */
#ifdef GL_ARB_imaging
  CONST_CAST(GLEW_ARB_imaging) = glewGetExtension("GL_ARB_imaging");
  if (glewExperimental || GLEW_ARB_imaging) CONST_CAST(GLEW_ARB_imaging) = !_glewInit_GL_ARB_imaging(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_imaging */
#ifdef GL_ARB_instanced_arrays
  CONST_CAST(GLEW_ARB_instanced_arrays) = glewGetExtension("GL_ARB_instanced_arrays");
  if (glewExperimental || GLEW_ARB_instanced_arrays) CONST_CAST(GLEW_ARB_instanced_arrays) = !_glewInit_GL_ARB_instanced_arrays(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_instanced_arrays */
#ifdef GL_ARB_map_buffer_range
  CONST_CAST(GLEW_ARB_map_buffer_range) = glewGetExtension("GL_ARB_map_buffer_range");
  if (glewExperimental || GLEW_ARB_map_buffer_range) CONST_CAST(GLEW_ARB_map_buffer_range) = !_glewInit_GL_ARB_map_buffer_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_map_buffer_range */
#ifdef GL_ARB_matrix_palette
  CONST_CAST(GLEW_ARB_matrix_palette) = glewGetExtension("GL_ARB_matrix_palette");
  if (glewExperimental || GLEW_ARB_matrix_palette) CONST_CAST(GLEW_ARB_matrix_palette) = !_glewInit_GL_ARB_matrix_palette(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_matrix_palette */
#ifdef GL_ARB_multisample
  CONST_CAST(GLEW_ARB_multisample) = glewGetExtension("GL_ARB_multisample");
  if (glewExperimental || GLEW_ARB_multisample) CONST_CAST(GLEW_ARB_multisample) = !_glewInit_GL_ARB_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_multisample */
#ifdef GL_ARB_multitexture
  CONST_CAST(GLEW_ARB_multitexture) = glewGetExtension("GL_ARB_multitexture");
  if (glewExperimental || GLEW_ARB_multitexture) CONST_CAST(GLEW_ARB_multitexture) = !_glewInit_GL_ARB_multitexture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_multitexture */
#ifdef GL_ARB_occlusion_query
  CONST_CAST(GLEW_ARB_occlusion_query) = glewGetExtension("GL_ARB_occlusion_query");
  if (glewExperimental || GLEW_ARB_occlusion_query) CONST_CAST(GLEW_ARB_occlusion_query) = !_glewInit_GL_ARB_occlusion_query(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_occlusion_query */
#ifdef GL_ARB_pixel_buffer_object
  CONST_CAST(GLEW_ARB_pixel_buffer_object) = glewGetExtension("GL_ARB_pixel_buffer_object");
#endif /* GL_ARB_pixel_buffer_object */
#ifdef GL_ARB_point_parameters
  CONST_CAST(GLEW_ARB_point_parameters) = glewGetExtension("GL_ARB_point_parameters");
  if (glewExperimental || GLEW_ARB_point_parameters) CONST_CAST(GLEW_ARB_point_parameters) = !_glewInit_GL_ARB_point_parameters(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_point_parameters */
#ifdef GL_ARB_point_sprite
  CONST_CAST(GLEW_ARB_point_sprite) = glewGetExtension("GL_ARB_point_sprite");
#endif /* GL_ARB_point_sprite */
#ifdef GL_ARB_shader_objects
  CONST_CAST(GLEW_ARB_shader_objects) = glewGetExtension("GL_ARB_shader_objects");
  if (glewExperimental || GLEW_ARB_shader_objects) CONST_CAST(GLEW_ARB_shader_objects) = !_glewInit_GL_ARB_shader_objects(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_shader_objects */
#ifdef GL_ARB_shading_language_100
  CONST_CAST(GLEW_ARB_shading_language_100) = glewGetExtension("GL_ARB_shading_language_100");
#endif /* GL_ARB_shading_language_100 */
#ifdef GL_ARB_shadow
  CONST_CAST(GLEW_ARB_shadow) = glewGetExtension("GL_ARB_shadow");
#endif /* GL_ARB_shadow */
#ifdef GL_ARB_shadow_ambient
  CONST_CAST(GLEW_ARB_shadow_ambient) = glewGetExtension("GL_ARB_shadow_ambient");
#endif /* GL_ARB_shadow_ambient */
#ifdef GL_ARB_texture_border_clamp
  CONST_CAST(GLEW_ARB_texture_border_clamp) = glewGetExtension("GL_ARB_texture_border_clamp");
#endif /* GL_ARB_texture_border_clamp */
#ifdef GL_ARB_texture_buffer_object
  CONST_CAST(GLEW_ARB_texture_buffer_object) = glewGetExtension("GL_ARB_texture_buffer_object");
  if (glewExperimental || GLEW_ARB_texture_buffer_object) CONST_CAST(GLEW_ARB_texture_buffer_object) = !_glewInit_GL_ARB_texture_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_texture_buffer_object */
#ifdef GL_ARB_texture_compression
  CONST_CAST(GLEW_ARB_texture_compression) = glewGetExtension("GL_ARB_texture_compression");
  if (glewExperimental || GLEW_ARB_texture_compression) CONST_CAST(GLEW_ARB_texture_compression) = !_glewInit_GL_ARB_texture_compression(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_texture_compression */
#ifdef GL_ARB_texture_compression_rgtc
  CONST_CAST(GLEW_ARB_texture_compression_rgtc) = glewGetExtension("GL_ARB_texture_compression_rgtc");
#endif /* GL_ARB_texture_compression_rgtc */
#ifdef GL_ARB_texture_cube_map
  CONST_CAST(GLEW_ARB_texture_cube_map) = glewGetExtension("GL_ARB_texture_cube_map");
#endif /* GL_ARB_texture_cube_map */
#ifdef GL_ARB_texture_env_add
  CONST_CAST(GLEW_ARB_texture_env_add) = glewGetExtension("GL_ARB_texture_env_add");
#endif /* GL_ARB_texture_env_add */
#ifdef GL_ARB_texture_env_combine
  CONST_CAST(GLEW_ARB_texture_env_combine) = glewGetExtension("GL_ARB_texture_env_combine");
#endif /* GL_ARB_texture_env_combine */
#ifdef GL_ARB_texture_env_crossbar
  CONST_CAST(GLEW_ARB_texture_env_crossbar) = glewGetExtension("GL_ARB_texture_env_crossbar");
#endif /* GL_ARB_texture_env_crossbar */
#ifdef GL_ARB_texture_env_dot3
  CONST_CAST(GLEW_ARB_texture_env_dot3) = glewGetExtension("GL_ARB_texture_env_dot3");
#endif /* GL_ARB_texture_env_dot3 */
#ifdef GL_ARB_texture_float
  CONST_CAST(GLEW_ARB_texture_float) = glewGetExtension("GL_ARB_texture_float");
#endif /* GL_ARB_texture_float */
#ifdef GL_ARB_texture_mirrored_repeat
  CONST_CAST(GLEW_ARB_texture_mirrored_repeat) = glewGetExtension("GL_ARB_texture_mirrored_repeat");
#endif /* GL_ARB_texture_mirrored_repeat */
#ifdef GL_ARB_texture_non_power_of_two
  CONST_CAST(GLEW_ARB_texture_non_power_of_two) = glewGetExtension("GL_ARB_texture_non_power_of_two");
#endif /* GL_ARB_texture_non_power_of_two */
#ifdef GL_ARB_texture_rectangle
  CONST_CAST(GLEW_ARB_texture_rectangle) = glewGetExtension("GL_ARB_texture_rectangle");
#endif /* GL_ARB_texture_rectangle */
#ifdef GL_ARB_texture_rg
  CONST_CAST(GLEW_ARB_texture_rg) = glewGetExtension("GL_ARB_texture_rg");
#endif /* GL_ARB_texture_rg */
#ifdef GL_ARB_transpose_matrix
  CONST_CAST(GLEW_ARB_transpose_matrix) = glewGetExtension("GL_ARB_transpose_matrix");
  if (glewExperimental || GLEW_ARB_transpose_matrix) CONST_CAST(GLEW_ARB_transpose_matrix) = !_glewInit_GL_ARB_transpose_matrix(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_transpose_matrix */
#ifdef GL_ARB_vertex_array_object
  CONST_CAST(GLEW_ARB_vertex_array_object) = glewGetExtension("GL_ARB_vertex_array_object");
  if (glewExperimental || GLEW_ARB_vertex_array_object) CONST_CAST(GLEW_ARB_vertex_array_object) = !_glewInit_GL_ARB_vertex_array_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_array_object */
#ifdef GL_ARB_vertex_blend
  CONST_CAST(GLEW_ARB_vertex_blend) = glewGetExtension("GL_ARB_vertex_blend");
  if (glewExperimental || GLEW_ARB_vertex_blend) CONST_CAST(GLEW_ARB_vertex_blend) = !_glewInit_GL_ARB_vertex_blend(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_blend */
#ifdef GL_ARB_vertex_buffer_object
  CONST_CAST(GLEW_ARB_vertex_buffer_object) = glewGetExtension("GL_ARB_vertex_buffer_object");
  if (glewExperimental || GLEW_ARB_vertex_buffer_object) CONST_CAST(GLEW_ARB_vertex_buffer_object) = !_glewInit_GL_ARB_vertex_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_buffer_object */
#ifdef GL_ARB_vertex_program
  CONST_CAST(GLEW_ARB_vertex_program) = glewGetExtension("GL_ARB_vertex_program");
  if (glewExperimental || GLEW_ARB_vertex_program) CONST_CAST(GLEW_ARB_vertex_program) = !_glewInit_GL_ARB_vertex_program(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_program */
#ifdef GL_ARB_vertex_shader
  CONST_CAST(GLEW_ARB_vertex_shader) = glewGetExtension("GL_ARB_vertex_shader");
  if (glewExperimental || GLEW_ARB_vertex_shader) CONST_CAST(GLEW_ARB_vertex_shader) = !_glewInit_GL_ARB_vertex_shader(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_shader */
#ifdef GL_ARB_window_pos
  CONST_CAST(GLEW_ARB_window_pos) = glewGetExtension("GL_ARB_window_pos");
  if (glewExperimental || GLEW_ARB_window_pos) CONST_CAST(GLEW_ARB_window_pos) = !_glewInit_GL_ARB_window_pos(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_window_pos */
#ifdef GL_ATIX_point_sprites
  CONST_CAST(GLEW_ATIX_point_sprites) = glewGetExtension("GL_ATIX_point_sprites");
#endif /* GL_ATIX_point_sprites */
#ifdef GL_ATIX_texture_env_combine3
  CONST_CAST(GLEW_ATIX_texture_env_combine3) = glewGetExtension("GL_ATIX_texture_env_combine3");
#endif /* GL_ATIX_texture_env_combine3 */
#ifdef GL_ATIX_texture_env_route
  CONST_CAST(GLEW_ATIX_texture_env_route) = glewGetExtension("GL_ATIX_texture_env_route");
#endif /* GL_ATIX_texture_env_route */
#ifdef GL_ATIX_vertex_shader_output_point_size
  CONST_CAST(GLEW_ATIX_vertex_shader_output_point_size) = glewGetExtension("GL_ATIX_vertex_shader_output_point_size");
#endif /* GL_ATIX_vertex_shader_output_point_size */
#ifdef GL_ATI_draw_buffers
  CONST_CAST(GLEW_ATI_draw_buffers) = glewGetExtension("GL_ATI_draw_buffers");
  if (glewExperimental || GLEW_ATI_draw_buffers) CONST_CAST(GLEW_ATI_draw_buffers) = !_glewInit_GL_ATI_draw_buffers(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_draw_buffers */
#ifdef GL_ATI_element_array
  CONST_CAST(GLEW_ATI_element_array) = glewGetExtension("GL_ATI_element_array");
  if (glewExperimental || GLEW_ATI_element_array) CONST_CAST(GLEW_ATI_element_array) = !_glewInit_GL_ATI_element_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_element_array */
#ifdef GL_ATI_envmap_bumpmap
  CONST_CAST(GLEW_ATI_envmap_bumpmap) = glewGetExtension("GL_ATI_envmap_bumpmap");
  if (glewExperimental || GLEW_ATI_envmap_bumpmap) CONST_CAST(GLEW_ATI_envmap_bumpmap) = !_glewInit_GL_ATI_envmap_bumpmap(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_envmap_bumpmap */
#ifdef GL_ATI_fragment_shader
  CONST_CAST(GLEW_ATI_fragment_shader) = glewGetExtension("GL_ATI_fragment_shader");
  if (glewExperimental || GLEW_ATI_fragment_shader) CONST_CAST(GLEW_ATI_fragment_shader) = !_glewInit_GL_ATI_fragment_shader(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_fragment_shader */
#ifdef GL_ATI_map_object_buffer
  CONST_CAST(GLEW_ATI_map_object_buffer) = glewGetExtension("GL_ATI_map_object_buffer");
  if (glewExperimental || GLEW_ATI_map_object_buffer) CONST_CAST(GLEW_ATI_map_object_buffer) = !_glewInit_GL_ATI_map_object_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_map_object_buffer */
#ifdef GL_ATI_pn_triangles
  CONST_CAST(GLEW_ATI_pn_triangles) = glewGetExtension("GL_ATI_pn_triangles");
  if (glewExperimental || GLEW_ATI_pn_triangles) CONST_CAST(GLEW_ATI_pn_triangles) = !_glewInit_GL_ATI_pn_triangles(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_pn_triangles */
#ifdef GL_ATI_separate_stencil
  CONST_CAST(GLEW_ATI_separate_stencil) = glewGetExtension("GL_ATI_separate_stencil");
  if (glewExperimental || GLEW_ATI_separate_stencil) CONST_CAST(GLEW_ATI_separate_stencil) = !_glewInit_GL_ATI_separate_stencil(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_separate_stencil */
#ifdef GL_ATI_shader_texture_lod
  CONST_CAST(GLEW_ATI_shader_texture_lod) = glewGetExtension("GL_ATI_shader_texture_lod");
#endif /* GL_ATI_shader_texture_lod */
#ifdef GL_ATI_text_fragment_shader
  CONST_CAST(GLEW_ATI_text_fragment_shader) = glewGetExtension("GL_ATI_text_fragment_shader");
#endif /* GL_ATI_text_fragment_shader */
#ifdef GL_ATI_texture_compression_3dc
  CONST_CAST(GLEW_ATI_texture_compression_3dc) = glewGetExtension("GL_ATI_texture_compression_3dc");
#endif /* GL_ATI_texture_compression_3dc */
#ifdef GL_ATI_texture_float
  CONST_CAST(GLEW_ATI_texture_float) = glewGetExtension("GL_ATI_texture_float");
#endif /* GL_ATI_texture_float */
#ifdef GL_ATI_texture_mirror_once
  CONST_CAST(GLEW_ATI_texture_mirror_once) = glewGetExtension("GL_ATI_texture_mirror_once");
#endif /* GL_ATI_texture_mirror_once */
#ifdef GL_ATI_vertex_array_object
  CONST_CAST(GLEW_ATI_vertex_array_object) = glewGetExtension("GL_ATI_vertex_array_object");
  if (glewExperimental || GLEW_ATI_vertex_array_object) CONST_CAST(GLEW_ATI_vertex_array_object) = !_glewInit_GL_ATI_vertex_array_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_vertex_array_object */
#ifdef GL_ATI_vertex_attrib_array_object
  CONST_CAST(GLEW_ATI_vertex_attrib_array_object) = glewGetExtension("GL_ATI_vertex_attrib_array_object");
  if (glewExperimental || GLEW_ATI_vertex_attrib_array_object) CONST_CAST(GLEW_ATI_vertex_attrib_array_object) = !_glewInit_GL_ATI_vertex_attrib_array_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_vertex_attrib_array_object */
#ifdef GL_ATI_vertex_streams
  CONST_CAST(GLEW_ATI_vertex_streams) = glewGetExtension("GL_ATI_vertex_streams");
  if (glewExperimental || GLEW_ATI_vertex_streams) CONST_CAST(GLEW_ATI_vertex_streams) = !_glewInit_GL_ATI_vertex_streams(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_vertex_streams */
#ifdef GL_EXT_422_pixels
  CONST_CAST(GLEW_EXT_422_pixels) = glewGetExtension("GL_EXT_422_pixels");
#endif /* GL_EXT_422_pixels */
#ifdef GL_EXT_Cg_shader
  CONST_CAST(GLEW_EXT_Cg_shader) = glewGetExtension("GL_EXT_Cg_shader");
#endif /* GL_EXT_Cg_shader */
#ifdef GL_EXT_abgr
  CONST_CAST(GLEW_EXT_abgr) = glewGetExtension("GL_EXT_abgr");
#endif /* GL_EXT_abgr */
#ifdef GL_EXT_bgra
  CONST_CAST(GLEW_EXT_bgra) = glewGetExtension("GL_EXT_bgra");
#endif /* GL_EXT_bgra */
#ifdef GL_EXT_bindable_uniform
  CONST_CAST(GLEW_EXT_bindable_uniform) = glewGetExtension("GL_EXT_bindable_uniform");
  if (glewExperimental || GLEW_EXT_bindable_uniform) CONST_CAST(GLEW_EXT_bindable_uniform) = !_glewInit_GL_EXT_bindable_uniform(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_bindable_uniform */
#ifdef GL_EXT_blend_color
  CONST_CAST(GLEW_EXT_blend_color) = glewGetExtension("GL_EXT_blend_color");
  if (glewExperimental || GLEW_EXT_blend_color) CONST_CAST(GLEW_EXT_blend_color) = !_glewInit_GL_EXT_blend_color(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_blend_color */
#ifdef GL_EXT_blend_equation_separate
  CONST_CAST(GLEW_EXT_blend_equation_separate) = glewGetExtension("GL_EXT_blend_equation_separate");
  if (glewExperimental || GLEW_EXT_blend_equation_separate) CONST_CAST(GLEW_EXT_blend_equation_separate) = !_glewInit_GL_EXT_blend_equation_separate(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_blend_equation_separate */
#ifdef GL_EXT_blend_func_separate
  CONST_CAST(GLEW_EXT_blend_func_separate) = glewGetExtension("GL_EXT_blend_func_separate");
  if (glewExperimental || GLEW_EXT_blend_func_separate) CONST_CAST(GLEW_EXT_blend_func_separate) = !_glewInit_GL_EXT_blend_func_separate(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_blend_func_separate */
#ifdef GL_EXT_blend_logic_op
  CONST_CAST(GLEW_EXT_blend_logic_op) = glewGetExtension("GL_EXT_blend_logic_op");
#endif /* GL_EXT_blend_logic_op */
#ifdef GL_EXT_blend_minmax
  CONST_CAST(GLEW_EXT_blend_minmax) = glewGetExtension("GL_EXT_blend_minmax");
  if (glewExperimental || GLEW_EXT_blend_minmax) CONST_CAST(GLEW_EXT_blend_minmax) = !_glewInit_GL_EXT_blend_minmax(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_blend_minmax */
#ifdef GL_EXT_blend_subtract
  CONST_CAST(GLEW_EXT_blend_subtract) = glewGetExtension("GL_EXT_blend_subtract");
#endif /* GL_EXT_blend_subtract */
#ifdef GL_EXT_clip_volume_hint
  CONST_CAST(GLEW_EXT_clip_volume_hint) = glewGetExtension("GL_EXT_clip_volume_hint");
#endif /* GL_EXT_clip_volume_hint */
#ifdef GL_EXT_cmyka
  CONST_CAST(GLEW_EXT_cmyka) = glewGetExtension("GL_EXT_cmyka");
#endif /* GL_EXT_cmyka */
#ifdef GL_EXT_color_subtable
  CONST_CAST(GLEW_EXT_color_subtable) = glewGetExtension("GL_EXT_color_subtable");
  if (glewExperimental || GLEW_EXT_color_subtable) CONST_CAST(GLEW_EXT_color_subtable) = !_glewInit_GL_EXT_color_subtable(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_color_subtable */
#ifdef GL_EXT_compiled_vertex_array
  CONST_CAST(GLEW_EXT_compiled_vertex_array) = glewGetExtension("GL_EXT_compiled_vertex_array");
  if (glewExperimental || GLEW_EXT_compiled_vertex_array) CONST_CAST(GLEW_EXT_compiled_vertex_array) = !_glewInit_GL_EXT_compiled_vertex_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_compiled_vertex_array */
#ifdef GL_EXT_convolution
  CONST_CAST(GLEW_EXT_convolution) = glewGetExtension("GL_EXT_convolution");
  if (glewExperimental || GLEW_EXT_convolution) CONST_CAST(GLEW_EXT_convolution) = !_glewInit_GL_EXT_convolution(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_convolution */
#ifdef GL_EXT_coordinate_frame
  CONST_CAST(GLEW_EXT_coordinate_frame) = glewGetExtension("GL_EXT_coordinate_frame");
  if (glewExperimental || GLEW_EXT_coordinate_frame) CONST_CAST(GLEW_EXT_coordinate_frame) = !_glewInit_GL_EXT_coordinate_frame(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_coordinate_frame */
#ifdef GL_EXT_copy_texture
  CONST_CAST(GLEW_EXT_copy_texture) = glewGetExtension("GL_EXT_copy_texture");
  if (glewExperimental || GLEW_EXT_copy_texture) CONST_CAST(GLEW_EXT_copy_texture) = !_glewInit_GL_EXT_copy_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_copy_texture */
#ifdef GL_EXT_cull_vertex
  CONST_CAST(GLEW_EXT_cull_vertex) = glewGetExtension("GL_EXT_cull_vertex");
  if (glewExperimental || GLEW_EXT_cull_vertex) CONST_CAST(GLEW_EXT_cull_vertex) = !_glewInit_GL_EXT_cull_vertex(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_cull_vertex */
#ifdef GL_EXT_depth_bounds_test
  CONST_CAST(GLEW_EXT_depth_bounds_test) = glewGetExtension("GL_EXT_depth_bounds_test");
  if (glewExperimental || GLEW_EXT_depth_bounds_test) CONST_CAST(GLEW_EXT_depth_bounds_test) = !_glewInit_GL_EXT_depth_bounds_test(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_depth_bounds_test */
#ifdef GL_EXT_direct_state_access
  CONST_CAST(GLEW_EXT_direct_state_access) = glewGetExtension("GL_EXT_direct_state_access");
  if (glewExperimental || GLEW_EXT_direct_state_access) CONST_CAST(GLEW_EXT_direct_state_access) = !_glewInit_GL_EXT_direct_state_access(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_direct_state_access */
#ifdef GL_EXT_draw_buffers2
  CONST_CAST(GLEW_EXT_draw_buffers2) = glewGetExtension("GL_EXT_draw_buffers2");
  if (glewExperimental || GLEW_EXT_draw_buffers2) CONST_CAST(GLEW_EXT_draw_buffers2) = !_glewInit_GL_EXT_draw_buffers2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_draw_buffers2 */
#ifdef GL_EXT_draw_instanced
  CONST_CAST(GLEW_EXT_draw_instanced) = glewGetExtension("GL_EXT_draw_instanced");
  if (glewExperimental || GLEW_EXT_draw_instanced) CONST_CAST(GLEW_EXT_draw_instanced) = !_glewInit_GL_EXT_draw_instanced(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_draw_instanced */
#ifdef GL_EXT_draw_range_elements
  CONST_CAST(GLEW_EXT_draw_range_elements) = glewGetExtension("GL_EXT_draw_range_elements");
  if (glewExperimental || GLEW_EXT_draw_range_elements) CONST_CAST(GLEW_EXT_draw_range_elements) = !_glewInit_GL_EXT_draw_range_elements(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_draw_range_elements */
#ifdef GL_EXT_fog_coord
  CONST_CAST(GLEW_EXT_fog_coord) = glewGetExtension("GL_EXT_fog_coord");
  if (glewExperimental || GLEW_EXT_fog_coord) CONST_CAST(GLEW_EXT_fog_coord) = !_glewInit_GL_EXT_fog_coord(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_fog_coord */
#ifdef GL_EXT_fragment_lighting
  CONST_CAST(GLEW_EXT_fragment_lighting) = glewGetExtension("GL_EXT_fragment_lighting");
  if (glewExperimental || GLEW_EXT_fragment_lighting) CONST_CAST(GLEW_EXT_fragment_lighting) = !_glewInit_GL_EXT_fragment_lighting(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_fragment_lighting */
#ifdef GL_EXT_framebuffer_blit
  CONST_CAST(GLEW_EXT_framebuffer_blit) = glewGetExtension("GL_EXT_framebuffer_blit");
  if (glewExperimental || GLEW_EXT_framebuffer_blit) CONST_CAST(GLEW_EXT_framebuffer_blit) = !_glewInit_GL_EXT_framebuffer_blit(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_framebuffer_blit */
#ifdef GL_EXT_framebuffer_multisample
  CONST_CAST(GLEW_EXT_framebuffer_multisample) = glewGetExtension("GL_EXT_framebuffer_multisample");
  if (glewExperimental || GLEW_EXT_framebuffer_multisample) CONST_CAST(GLEW_EXT_framebuffer_multisample) = !_glewInit_GL_EXT_framebuffer_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_framebuffer_multisample */
#ifdef GL_EXT_framebuffer_object
  CONST_CAST(GLEW_EXT_framebuffer_object) = glewGetExtension("GL_EXT_framebuffer_object");
  if (glewExperimental || GLEW_EXT_framebuffer_object) CONST_CAST(GLEW_EXT_framebuffer_object) = !_glewInit_GL_EXT_framebuffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_framebuffer_object */
#ifdef GL_EXT_framebuffer_sRGB
  CONST_CAST(GLEW_EXT_framebuffer_sRGB) = glewGetExtension("GL_EXT_framebuffer_sRGB");
#endif /* GL_EXT_framebuffer_sRGB */
#ifdef GL_EXT_geometry_shader4
  CONST_CAST(GLEW_EXT_geometry_shader4) = glewGetExtension("GL_EXT_geometry_shader4");
  if (glewExperimental || GLEW_EXT_geometry_shader4) CONST_CAST(GLEW_EXT_geometry_shader4) = !_glewInit_GL_EXT_geometry_shader4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_geometry_shader4 */
#ifdef GL_EXT_gpu_program_parameters
  CONST_CAST(GLEW_EXT_gpu_program_parameters) = glewGetExtension("GL_EXT_gpu_program_parameters");
  if (glewExperimental || GLEW_EXT_gpu_program_parameters) CONST_CAST(GLEW_EXT_gpu_program_parameters) = !_glewInit_GL_EXT_gpu_program_parameters(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_gpu_program_parameters */
#ifdef GL_EXT_gpu_shader4
  CONST_CAST(GLEW_EXT_gpu_shader4) = glewGetExtension("GL_EXT_gpu_shader4");
  if (glewExperimental || GLEW_EXT_gpu_shader4) CONST_CAST(GLEW_EXT_gpu_shader4) = !_glewInit_GL_EXT_gpu_shader4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_gpu_shader4 */
#ifdef GL_EXT_histogram
  CONST_CAST(GLEW_EXT_histogram) = glewGetExtension("GL_EXT_histogram");
  if (glewExperimental || GLEW_EXT_histogram) CONST_CAST(GLEW_EXT_histogram) = !_glewInit_GL_EXT_histogram(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_histogram */
#ifdef GL_EXT_index_array_formats
  CONST_CAST(GLEW_EXT_index_array_formats) = glewGetExtension("GL_EXT_index_array_formats");
#endif /* GL_EXT_index_array_formats */
#ifdef GL_EXT_index_func
  CONST_CAST(GLEW_EXT_index_func) = glewGetExtension("GL_EXT_index_func");
  if (glewExperimental || GLEW_EXT_index_func) CONST_CAST(GLEW_EXT_index_func) = !_glewInit_GL_EXT_index_func(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_index_func */
#ifdef GL_EXT_index_material
  CONST_CAST(GLEW_EXT_index_material) = glewGetExtension("GL_EXT_index_material");
  if (glewExperimental || GLEW_EXT_index_material) CONST_CAST(GLEW_EXT_index_material) = !_glewInit_GL_EXT_index_material(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_index_material */
#ifdef GL_EXT_index_texture
  CONST_CAST(GLEW_EXT_index_texture) = glewGetExtension("GL_EXT_index_texture");
#endif /* GL_EXT_index_texture */
#ifdef GL_EXT_light_texture
  CONST_CAST(GLEW_EXT_light_texture) = glewGetExtension("GL_EXT_light_texture");
  if (glewExperimental || GLEW_EXT_light_texture) CONST_CAST(GLEW_EXT_light_texture) = !_glewInit_GL_EXT_light_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_light_texture */
#ifdef GL_EXT_misc_attribute
  CONST_CAST(GLEW_EXT_misc_attribute) = glewGetExtension("GL_EXT_misc_attribute");
#endif /* GL_EXT_misc_attribute */
#ifdef GL_EXT_multi_draw_arrays
  CONST_CAST(GLEW_EXT_multi_draw_arrays) = glewGetExtension("GL_EXT_multi_draw_arrays");
  if (glewExperimental || GLEW_EXT_multi_draw_arrays) CONST_CAST(GLEW_EXT_multi_draw_arrays) = !_glewInit_GL_EXT_multi_draw_arrays(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_multi_draw_arrays */
#ifdef GL_EXT_multisample
  CONST_CAST(GLEW_EXT_multisample) = glewGetExtension("GL_EXT_multisample");
  if (glewExperimental || GLEW_EXT_multisample) CONST_CAST(GLEW_EXT_multisample) = !_glewInit_GL_EXT_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_multisample */
#ifdef GL_EXT_packed_depth_stencil
  CONST_CAST(GLEW_EXT_packed_depth_stencil) = glewGetExtension("GL_EXT_packed_depth_stencil");
#endif /* GL_EXT_packed_depth_stencil */
#ifdef GL_EXT_packed_float
  CONST_CAST(GLEW_EXT_packed_float) = glewGetExtension("GL_EXT_packed_float");
#endif /* GL_EXT_packed_float */
#ifdef GL_EXT_packed_pixels
  CONST_CAST(GLEW_EXT_packed_pixels) = glewGetExtension("GL_EXT_packed_pixels");
#endif /* GL_EXT_packed_pixels */
#ifdef GL_EXT_paletted_texture
  CONST_CAST(GLEW_EXT_paletted_texture) = glewGetExtension("GL_EXT_paletted_texture");
  if (glewExperimental || GLEW_EXT_paletted_texture) CONST_CAST(GLEW_EXT_paletted_texture) = !_glewInit_GL_EXT_paletted_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_paletted_texture */
#ifdef GL_EXT_pixel_buffer_object
  CONST_CAST(GLEW_EXT_pixel_buffer_object) = glewGetExtension("GL_EXT_pixel_buffer_object");
#endif /* GL_EXT_pixel_buffer_object */
#ifdef GL_EXT_pixel_transform
  CONST_CAST(GLEW_EXT_pixel_transform) = glewGetExtension("GL_EXT_pixel_transform");
  if (glewExperimental || GLEW_EXT_pixel_transform) CONST_CAST(GLEW_EXT_pixel_transform) = !_glewInit_GL_EXT_pixel_transform(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_pixel_transform */
#ifdef GL_EXT_pixel_transform_color_table
  CONST_CAST(GLEW_EXT_pixel_transform_color_table) = glewGetExtension("GL_EXT_pixel_transform_color_table");
#endif /* GL_EXT_pixel_transform_color_table */
#ifdef GL_EXT_point_parameters
  CONST_CAST(GLEW_EXT_point_parameters) = glewGetExtension("GL_EXT_point_parameters");
  if (glewExperimental || GLEW_EXT_point_parameters) CONST_CAST(GLEW_EXT_point_parameters) = !_glewInit_GL_EXT_point_parameters(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_point_parameters */
#ifdef GL_EXT_polygon_offset
  CONST_CAST(GLEW_EXT_polygon_offset) = glewGetExtension("GL_EXT_polygon_offset");
  if (glewExperimental || GLEW_EXT_polygon_offset) CONST_CAST(GLEW_EXT_polygon_offset) = !_glewInit_GL_EXT_polygon_offset(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_polygon_offset */
#ifdef GL_EXT_rescale_normal
  CONST_CAST(GLEW_EXT_rescale_normal) = glewGetExtension("GL_EXT_rescale_normal");
#endif /* GL_EXT_rescale_normal */
#ifdef GL_EXT_scene_marker
  CONST_CAST(GLEW_EXT_scene_marker) = glewGetExtension("GL_EXT_scene_marker");
  if (glewExperimental || GLEW_EXT_scene_marker) CONST_CAST(GLEW_EXT_scene_marker) = !_glewInit_GL_EXT_scene_marker(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_scene_marker */
#ifdef GL_EXT_secondary_color
  CONST_CAST(GLEW_EXT_secondary_color) = glewGetExtension("GL_EXT_secondary_color");
  if (glewExperimental || GLEW_EXT_secondary_color) CONST_CAST(GLEW_EXT_secondary_color) = !_glewInit_GL_EXT_secondary_color(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_secondary_color */
#ifdef GL_EXT_separate_specular_color
  CONST_CAST(GLEW_EXT_separate_specular_color) = glewGetExtension("GL_EXT_separate_specular_color");
#endif /* GL_EXT_separate_specular_color */
#ifdef GL_EXT_shadow_funcs
  CONST_CAST(GLEW_EXT_shadow_funcs) = glewGetExtension("GL_EXT_shadow_funcs");
#endif /* GL_EXT_shadow_funcs */
#ifdef GL_EXT_shared_texture_palette
  CONST_CAST(GLEW_EXT_shared_texture_palette) = glewGetExtension("GL_EXT_shared_texture_palette");
#endif /* GL_EXT_shared_texture_palette */
#ifdef GL_EXT_stencil_clear_tag
  CONST_CAST(GLEW_EXT_stencil_clear_tag) = glewGetExtension("GL_EXT_stencil_clear_tag");
#endif /* GL_EXT_stencil_clear_tag */
#ifdef GL_EXT_stencil_two_side
  CONST_CAST(GLEW_EXT_stencil_two_side) = glewGetExtension("GL_EXT_stencil_two_side");
  if (glewExperimental || GLEW_EXT_stencil_two_side) CONST_CAST(GLEW_EXT_stencil_two_side) = !_glewInit_GL_EXT_stencil_two_side(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_stencil_two_side */
#ifdef GL_EXT_stencil_wrap
  CONST_CAST(GLEW_EXT_stencil_wrap) = glewGetExtension("GL_EXT_stencil_wrap");
#endif /* GL_EXT_stencil_wrap */
#ifdef GL_EXT_subtexture
  CONST_CAST(GLEW_EXT_subtexture) = glewGetExtension("GL_EXT_subtexture");
  if (glewExperimental || GLEW_EXT_subtexture) CONST_CAST(GLEW_EXT_subtexture) = !_glewInit_GL_EXT_subtexture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_subtexture */
#ifdef GL_EXT_texture
  CONST_CAST(GLEW_EXT_texture) = glewGetExtension("GL_EXT_texture");
#endif /* GL_EXT_texture */
#ifdef GL_EXT_texture3D
  CONST_CAST(GLEW_EXT_texture3D) = glewGetExtension("GL_EXT_texture3D");
  if (glewExperimental || GLEW_EXT_texture3D) CONST_CAST(GLEW_EXT_texture3D) = !_glewInit_GL_EXT_texture3D(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture3D */
#ifdef GL_EXT_texture_array
  CONST_CAST(GLEW_EXT_texture_array) = glewGetExtension("GL_EXT_texture_array");
#endif /* GL_EXT_texture_array */
#ifdef GL_EXT_texture_buffer_object
  CONST_CAST(GLEW_EXT_texture_buffer_object) = glewGetExtension("GL_EXT_texture_buffer_object");
  if (glewExperimental || GLEW_EXT_texture_buffer_object) CONST_CAST(GLEW_EXT_texture_buffer_object) = !_glewInit_GL_EXT_texture_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_buffer_object */
#ifdef GL_EXT_texture_compression_dxt1
  CONST_CAST(GLEW_EXT_texture_compression_dxt1) = glewGetExtension("GL_EXT_texture_compression_dxt1");
#endif /* GL_EXT_texture_compression_dxt1 */
#ifdef GL_EXT_texture_compression_latc
  CONST_CAST(GLEW_EXT_texture_compression_latc) = glewGetExtension("GL_EXT_texture_compression_latc");
#endif /* GL_EXT_texture_compression_latc */
#ifdef GL_EXT_texture_compression_rgtc
  CONST_CAST(GLEW_EXT_texture_compression_rgtc) = glewGetExtension("GL_EXT_texture_compression_rgtc");
#endif /* GL_EXT_texture_compression_rgtc */
#ifdef GL_EXT_texture_compression_s3tc
  CONST_CAST(GLEW_EXT_texture_compression_s3tc) = glewGetExtension("GL_EXT_texture_compression_s3tc");
#endif /* GL_EXT_texture_compression_s3tc */
#ifdef GL_EXT_texture_cube_map
  CONST_CAST(GLEW_EXT_texture_cube_map) = glewGetExtension("GL_EXT_texture_cube_map");
#endif /* GL_EXT_texture_cube_map */
#ifdef GL_EXT_texture_edge_clamp
  CONST_CAST(GLEW_EXT_texture_edge_clamp) = glewGetExtension("GL_EXT_texture_edge_clamp");
#endif /* GL_EXT_texture_edge_clamp */
#ifdef GL_EXT_texture_env
  CONST_CAST(GLEW_EXT_texture_env) = glewGetExtension("GL_EXT_texture_env");
#endif /* GL_EXT_texture_env */
#ifdef GL_EXT_texture_env_add
  CONST_CAST(GLEW_EXT_texture_env_add) = glewGetExtension("GL_EXT_texture_env_add");
#endif /* GL_EXT_texture_env_add */
#ifdef GL_EXT_texture_env_combine
  CONST_CAST(GLEW_EXT_texture_env_combine) = glewGetExtension("GL_EXT_texture_env_combine");
#endif /* GL_EXT_texture_env_combine */
#ifdef GL_EXT_texture_env_dot3
  CONST_CAST(GLEW_EXT_texture_env_dot3) = glewGetExtension("GL_EXT_texture_env_dot3");
#endif /* GL_EXT_texture_env_dot3 */
#ifdef GL_EXT_texture_filter_anisotropic
  CONST_CAST(GLEW_EXT_texture_filter_anisotropic) = glewGetExtension("GL_EXT_texture_filter_anisotropic");
#endif /* GL_EXT_texture_filter_anisotropic */
#ifdef GL_EXT_texture_integer
  CONST_CAST(GLEW_EXT_texture_integer) = glewGetExtension("GL_EXT_texture_integer");
  if (glewExperimental || GLEW_EXT_texture_integer) CONST_CAST(GLEW_EXT_texture_integer) = !_glewInit_GL_EXT_texture_integer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_integer */
#ifdef GL_EXT_texture_lod_bias
  CONST_CAST(GLEW_EXT_texture_lod_bias) = glewGetExtension("GL_EXT_texture_lod_bias");
#endif /* GL_EXT_texture_lod_bias */
#ifdef GL_EXT_texture_mirror_clamp
  CONST_CAST(GLEW_EXT_texture_mirror_clamp) = glewGetExtension("GL_EXT_texture_mirror_clamp");
#endif /* GL_EXT_texture_mirror_clamp */
#ifdef GL_EXT_texture_object
  CONST_CAST(GLEW_EXT_texture_object) = glewGetExtension("GL_EXT_texture_object");
  if (glewExperimental || GLEW_EXT_texture_object) CONST_CAST(GLEW_EXT_texture_object) = !_glewInit_GL_EXT_texture_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_object */
#ifdef GL_EXT_texture_perturb_normal
  CONST_CAST(GLEW_EXT_texture_perturb_normal) = glewGetExtension("GL_EXT_texture_perturb_normal");
  if (glewExperimental || GLEW_EXT_texture_perturb_normal) CONST_CAST(GLEW_EXT_texture_perturb_normal) = !_glewInit_GL_EXT_texture_perturb_normal(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_perturb_normal */
#ifdef GL_EXT_texture_rectangle
  CONST_CAST(GLEW_EXT_texture_rectangle) = glewGetExtension("GL_EXT_texture_rectangle");
#endif /* GL_EXT_texture_rectangle */
#ifdef GL_EXT_texture_sRGB
  CONST_CAST(GLEW_EXT_texture_sRGB) = glewGetExtension("GL_EXT_texture_sRGB");
#endif /* GL_EXT_texture_sRGB */
#ifdef GL_EXT_texture_shared_exponent
  CONST_CAST(GLEW_EXT_texture_shared_exponent) = glewGetExtension("GL_EXT_texture_shared_exponent");
#endif /* GL_EXT_texture_shared_exponent */
#ifdef GL_EXT_texture_swizzle
  CONST_CAST(GLEW_EXT_texture_swizzle) = glewGetExtension("GL_EXT_texture_swizzle");
#endif /* GL_EXT_texture_swizzle */
#ifdef GL_EXT_timer_query
  CONST_CAST(GLEW_EXT_timer_query) = glewGetExtension("GL_EXT_timer_query");
  if (glewExperimental || GLEW_EXT_timer_query) CONST_CAST(GLEW_EXT_timer_query) = !_glewInit_GL_EXT_timer_query(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_timer_query */
#ifdef GL_EXT_transform_feedback
  CONST_CAST(GLEW_EXT_transform_feedback) = glewGetExtension("GL_EXT_transform_feedback");
  if (glewExperimental || GLEW_EXT_transform_feedback) CONST_CAST(GLEW_EXT_transform_feedback) = !_glewInit_GL_EXT_transform_feedback(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_transform_feedback */
#ifdef GL_EXT_vertex_array
  CONST_CAST(GLEW_EXT_vertex_array) = glewGetExtension("GL_EXT_vertex_array");
  if (glewExperimental || GLEW_EXT_vertex_array) CONST_CAST(GLEW_EXT_vertex_array) = !_glewInit_GL_EXT_vertex_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_vertex_array */
#ifdef GL_EXT_vertex_array_bgra
  CONST_CAST(GLEW_EXT_vertex_array_bgra) = glewGetExtension("GL_EXT_vertex_array_bgra");
#endif /* GL_EXT_vertex_array_bgra */
#ifdef GL_EXT_vertex_shader
  CONST_CAST(GLEW_EXT_vertex_shader) = glewGetExtension("GL_EXT_vertex_shader");
  if (glewExperimental || GLEW_EXT_vertex_shader) CONST_CAST(GLEW_EXT_vertex_shader) = !_glewInit_GL_EXT_vertex_shader(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_vertex_shader */
#ifdef GL_EXT_vertex_weighting
  CONST_CAST(GLEW_EXT_vertex_weighting) = glewGetExtension("GL_EXT_vertex_weighting");
  if (glewExperimental || GLEW_EXT_vertex_weighting) CONST_CAST(GLEW_EXT_vertex_weighting) = !_glewInit_GL_EXT_vertex_weighting(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_vertex_weighting */
#ifdef GL_GREMEDY_frame_terminator
  CONST_CAST(GLEW_GREMEDY_frame_terminator) = glewGetExtension("GL_GREMEDY_frame_terminator");
  if (glewExperimental || GLEW_GREMEDY_frame_terminator) CONST_CAST(GLEW_GREMEDY_frame_terminator) = !_glewInit_GL_GREMEDY_frame_terminator(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_GREMEDY_frame_terminator */
#ifdef GL_GREMEDY_string_marker
  CONST_CAST(GLEW_GREMEDY_string_marker) = glewGetExtension("GL_GREMEDY_string_marker");
  if (glewExperimental || GLEW_GREMEDY_string_marker) CONST_CAST(GLEW_GREMEDY_string_marker) = !_glewInit_GL_GREMEDY_string_marker(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_GREMEDY_string_marker */
#ifdef GL_HP_convolution_border_modes
  CONST_CAST(GLEW_HP_convolution_border_modes) = glewGetExtension("GL_HP_convolution_border_modes");
#endif /* GL_HP_convolution_border_modes */
#ifdef GL_HP_image_transform
  CONST_CAST(GLEW_HP_image_transform) = glewGetExtension("GL_HP_image_transform");
  if (glewExperimental || GLEW_HP_image_transform) CONST_CAST(GLEW_HP_image_transform) = !_glewInit_GL_HP_image_transform(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_HP_image_transform */
#ifdef GL_HP_occlusion_test
  CONST_CAST(GLEW_HP_occlusion_test) = glewGetExtension("GL_HP_occlusion_test");
#endif /* GL_HP_occlusion_test */
#ifdef GL_HP_texture_lighting
  CONST_CAST(GLEW_HP_texture_lighting) = glewGetExtension("GL_HP_texture_lighting");
#endif /* GL_HP_texture_lighting */
#ifdef GL_IBM_cull_vertex
  CONST_CAST(GLEW_IBM_cull_vertex) = glewGetExtension("GL_IBM_cull_vertex");
#endif /* GL_IBM_cull_vertex */
#ifdef GL_IBM_multimode_draw_arrays
  CONST_CAST(GLEW_IBM_multimode_draw_arrays) = glewGetExtension("GL_IBM_multimode_draw_arrays");
  if (glewExperimental || GLEW_IBM_multimode_draw_arrays) CONST_CAST(GLEW_IBM_multimode_draw_arrays) = !_glewInit_GL_IBM_multimode_draw_arrays(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_IBM_multimode_draw_arrays */
#ifdef GL_IBM_rasterpos_clip
  CONST_CAST(GLEW_IBM_rasterpos_clip) = glewGetExtension("GL_IBM_rasterpos_clip");
#endif /* GL_IBM_rasterpos_clip */
#ifdef GL_IBM_static_data
  CONST_CAST(GLEW_IBM_static_data) = glewGetExtension("GL_IBM_static_data");
#endif /* GL_IBM_static_data */
#ifdef GL_IBM_texture_mirrored_repeat
  CONST_CAST(GLEW_IBM_texture_mirrored_repeat) = glewGetExtension("GL_IBM_texture_mirrored_repeat");
#endif /* GL_IBM_texture_mirrored_repeat */
#ifdef GL_IBM_vertex_array_lists
  CONST_CAST(GLEW_IBM_vertex_array_lists) = glewGetExtension("GL_IBM_vertex_array_lists");
  if (glewExperimental || GLEW_IBM_vertex_array_lists) CONST_CAST(GLEW_IBM_vertex_array_lists) = !_glewInit_GL_IBM_vertex_array_lists(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_IBM_vertex_array_lists */
#ifdef GL_INGR_color_clamp
  CONST_CAST(GLEW_INGR_color_clamp) = glewGetExtension("GL_INGR_color_clamp");
#endif /* GL_INGR_color_clamp */
#ifdef GL_INGR_interlace_read
  CONST_CAST(GLEW_INGR_interlace_read) = glewGetExtension("GL_INGR_interlace_read");
#endif /* GL_INGR_interlace_read */
#ifdef GL_INTEL_parallel_arrays
  CONST_CAST(GLEW_INTEL_parallel_arrays) = glewGetExtension("GL_INTEL_parallel_arrays");
  if (glewExperimental || GLEW_INTEL_parallel_arrays) CONST_CAST(GLEW_INTEL_parallel_arrays) = !_glewInit_GL_INTEL_parallel_arrays(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_INTEL_parallel_arrays */
#ifdef GL_INTEL_texture_scissor
  CONST_CAST(GLEW_INTEL_texture_scissor) = glewGetExtension("GL_INTEL_texture_scissor");
  if (glewExperimental || GLEW_INTEL_texture_scissor) CONST_CAST(GLEW_INTEL_texture_scissor) = !_glewInit_GL_INTEL_texture_scissor(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_INTEL_texture_scissor */
#ifdef GL_KTX_buffer_region
  CONST_CAST(GLEW_KTX_buffer_region) = glewGetExtension("GL_KTX_buffer_region");
  if (glewExperimental || GLEW_KTX_buffer_region) CONST_CAST(GLEW_KTX_buffer_region) = !_glewInit_GL_KTX_buffer_region(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_KTX_buffer_region */
#ifdef GL_MESAX_texture_stack
  CONST_CAST(GLEW_MESAX_texture_stack) = glewGetExtension("GL_MESAX_texture_stack");
#endif /* GL_MESAX_texture_stack */
#ifdef GL_MESA_pack_invert
  CONST_CAST(GLEW_MESA_pack_invert) = glewGetExtension("GL_MESA_pack_invert");
#endif /* GL_MESA_pack_invert */
#ifdef GL_MESA_resize_buffers
  CONST_CAST(GLEW_MESA_resize_buffers) = glewGetExtension("GL_MESA_resize_buffers");
  if (glewExperimental || GLEW_MESA_resize_buffers) CONST_CAST(GLEW_MESA_resize_buffers) = !_glewInit_GL_MESA_resize_buffers(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_MESA_resize_buffers */
#ifdef GL_MESA_window_pos
  CONST_CAST(GLEW_MESA_window_pos) = glewGetExtension("GL_MESA_window_pos");
  if (glewExperimental || GLEW_MESA_window_pos) CONST_CAST(GLEW_MESA_window_pos) = !_glewInit_GL_MESA_window_pos(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_MESA_window_pos */
#ifdef GL_MESA_ycbcr_texture
  CONST_CAST(GLEW_MESA_ycbcr_texture) = glewGetExtension("GL_MESA_ycbcr_texture");
#endif /* GL_MESA_ycbcr_texture */
#ifdef GL_NV_blend_square
  CONST_CAST(GLEW_NV_blend_square) = glewGetExtension("GL_NV_blend_square");
#endif /* GL_NV_blend_square */
#ifdef GL_NV_conditional_render
  CONST_CAST(GLEW_NV_conditional_render) = glewGetExtension("GL_NV_conditional_render");
  if (glewExperimental || GLEW_NV_conditional_render) CONST_CAST(GLEW_NV_conditional_render) = !_glewInit_GL_NV_conditional_render(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_conditional_render */
#ifdef GL_NV_copy_depth_to_color
  CONST_CAST(GLEW_NV_copy_depth_to_color) = glewGetExtension("GL_NV_copy_depth_to_color");
#endif /* GL_NV_copy_depth_to_color */
#ifdef GL_NV_depth_buffer_float
  CONST_CAST(GLEW_NV_depth_buffer_float) = glewGetExtension("GL_NV_depth_buffer_float");
  if (glewExperimental || GLEW_NV_depth_buffer_float) CONST_CAST(GLEW_NV_depth_buffer_float) = !_glewInit_GL_NV_depth_buffer_float(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_depth_buffer_float */
#ifdef GL_NV_depth_clamp
  CONST_CAST(GLEW_NV_depth_clamp) = glewGetExtension("GL_NV_depth_clamp");
#endif /* GL_NV_depth_clamp */
#ifdef GL_NV_depth_range_unclamped
  CONST_CAST(GLEW_NV_depth_range_unclamped) = glewGetExtension("GL_NV_depth_range_unclamped");
#endif /* GL_NV_depth_range_unclamped */
#ifdef GL_NV_evaluators
  CONST_CAST(GLEW_NV_evaluators) = glewGetExtension("GL_NV_evaluators");
  if (glewExperimental || GLEW_NV_evaluators) CONST_CAST(GLEW_NV_evaluators) = !_glewInit_GL_NV_evaluators(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_evaluators */
#ifdef GL_NV_explicit_multisample
  CONST_CAST(GLEW_NV_explicit_multisample) = glewGetExtension("GL_NV_explicit_multisample");
  if (glewExperimental || GLEW_NV_explicit_multisample) CONST_CAST(GLEW_NV_explicit_multisample) = !_glewInit_GL_NV_explicit_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_explicit_multisample */
#ifdef GL_NV_fence
  CONST_CAST(GLEW_NV_fence) = glewGetExtension("GL_NV_fence");
  if (glewExperimental || GLEW_NV_fence) CONST_CAST(GLEW_NV_fence) = !_glewInit_GL_NV_fence(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_fence */
#ifdef GL_NV_float_buffer
  CONST_CAST(GLEW_NV_float_buffer) = glewGetExtension("GL_NV_float_buffer");
#endif /* GL_NV_float_buffer */
#ifdef GL_NV_fog_distance
  CONST_CAST(GLEW_NV_fog_distance) = glewGetExtension("GL_NV_fog_distance");
#endif /* GL_NV_fog_distance */
#ifdef GL_NV_fragment_program
  CONST_CAST(GLEW_NV_fragment_program) = glewGetExtension("GL_NV_fragment_program");
  if (glewExperimental || GLEW_NV_fragment_program) CONST_CAST(GLEW_NV_fragment_program) = !_glewInit_GL_NV_fragment_program(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_fragment_program */
#ifdef GL_NV_fragment_program2
  CONST_CAST(GLEW_NV_fragment_program2) = glewGetExtension("GL_NV_fragment_program2");
#endif /* GL_NV_fragment_program2 */
#ifdef GL_NV_fragment_program4
  CONST_CAST(GLEW_NV_fragment_program4) = glewGetExtension("GL_NV_fragment_program4");
#endif /* GL_NV_fragment_program4 */
#ifdef GL_NV_fragment_program_option
  CONST_CAST(GLEW_NV_fragment_program_option) = glewGetExtension("GL_NV_fragment_program_option");
#endif /* GL_NV_fragment_program_option */
#ifdef GL_NV_framebuffer_multisample_coverage
  CONST_CAST(GLEW_NV_framebuffer_multisample_coverage) = glewGetExtension("GL_NV_framebuffer_multisample_coverage");
  if (glewExperimental || GLEW_NV_framebuffer_multisample_coverage) CONST_CAST(GLEW_NV_framebuffer_multisample_coverage) = !_glewInit_GL_NV_framebuffer_multisample_coverage(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_framebuffer_multisample_coverage */
#ifdef GL_NV_geometry_program4
  CONST_CAST(GLEW_NV_geometry_program4) = glewGetExtension("GL_NV_geometry_program4");
  if (glewExperimental || GLEW_NV_geometry_program4) CONST_CAST(GLEW_NV_geometry_program4) = !_glewInit_GL_NV_geometry_program4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_geometry_program4 */
#ifdef GL_NV_geometry_shader4
  CONST_CAST(GLEW_NV_geometry_shader4) = glewGetExtension("GL_NV_geometry_shader4");
#endif /* GL_NV_geometry_shader4 */
#ifdef GL_NV_gpu_program4
  CONST_CAST(GLEW_NV_gpu_program4) = glewGetExtension("GL_NV_gpu_program4");
  if (glewExperimental || GLEW_NV_gpu_program4) CONST_CAST(GLEW_NV_gpu_program4) = !_glewInit_GL_NV_gpu_program4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_gpu_program4 */
#ifdef GL_NV_half_float
  CONST_CAST(GLEW_NV_half_float) = glewGetExtension("GL_NV_half_float");
  if (glewExperimental || GLEW_NV_half_float) CONST_CAST(GLEW_NV_half_float) = !_glewInit_GL_NV_half_float(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_half_float */
#ifdef GL_NV_light_max_exponent
  CONST_CAST(GLEW_NV_light_max_exponent) = glewGetExtension("GL_NV_light_max_exponent");
#endif /* GL_NV_light_max_exponent */
#ifdef GL_NV_multisample_filter_hint
  CONST_CAST(GLEW_NV_multisample_filter_hint) = glewGetExtension("GL_NV_multisample_filter_hint");
#endif /* GL_NV_multisample_filter_hint */
#ifdef GL_NV_occlusion_query
  CONST_CAST(GLEW_NV_occlusion_query) = glewGetExtension("GL_NV_occlusion_query");
  if (glewExperimental || GLEW_NV_occlusion_query) CONST_CAST(GLEW_NV_occlusion_query) = !_glewInit_GL_NV_occlusion_query(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_occlusion_query */
#ifdef GL_NV_packed_depth_stencil
  CONST_CAST(GLEW_NV_packed_depth_stencil) = glewGetExtension("GL_NV_packed_depth_stencil");
#endif /* GL_NV_packed_depth_stencil */
#ifdef GL_NV_parameter_buffer_object
  CONST_CAST(GLEW_NV_parameter_buffer_object) = glewGetExtension("GL_NV_parameter_buffer_object");
  if (glewExperimental || GLEW_NV_parameter_buffer_object) CONST_CAST(GLEW_NV_parameter_buffer_object) = !_glewInit_GL_NV_parameter_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_parameter_buffer_object */
#ifdef GL_NV_pixel_data_range
  CONST_CAST(GLEW_NV_pixel_data_range) = glewGetExtension("GL_NV_pixel_data_range");
  if (glewExperimental || GLEW_NV_pixel_data_range) CONST_CAST(GLEW_NV_pixel_data_range) = !_glewInit_GL_NV_pixel_data_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_pixel_data_range */
#ifdef GL_NV_point_sprite
  CONST_CAST(GLEW_NV_point_sprite) = glewGetExtension("GL_NV_point_sprite");
  if (glewExperimental || GLEW_NV_point_sprite) CONST_CAST(GLEW_NV_point_sprite) = !_glewInit_GL_NV_point_sprite(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_point_sprite */
#ifdef GL_NV_present_video
  CONST_CAST(GLEW_NV_present_video) = glewGetExtension("GL_NV_present_video");
  if (glewExperimental || GLEW_NV_present_video) CONST_CAST(GLEW_NV_present_video) = !_glewInit_GL_NV_present_video(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_present_video */
#ifdef GL_NV_primitive_restart
  CONST_CAST(GLEW_NV_primitive_restart) = glewGetExtension("GL_NV_primitive_restart");
  if (glewExperimental || GLEW_NV_primitive_restart) CONST_CAST(GLEW_NV_primitive_restart) = !_glewInit_GL_NV_primitive_restart(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_primitive_restart */
#ifdef GL_NV_register_combiners
  CONST_CAST(GLEW_NV_register_combiners) = glewGetExtension("GL_NV_register_combiners");
  if (glewExperimental || GLEW_NV_register_combiners) CONST_CAST(GLEW_NV_register_combiners) = !_glewInit_GL_NV_register_combiners(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_register_combiners */
#ifdef GL_NV_register_combiners2
  CONST_CAST(GLEW_NV_register_combiners2) = glewGetExtension("GL_NV_register_combiners2");
  if (glewExperimental || GLEW_NV_register_combiners2) CONST_CAST(GLEW_NV_register_combiners2) = !_glewInit_GL_NV_register_combiners2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_register_combiners2 */
#ifdef GL_NV_texgen_emboss
  CONST_CAST(GLEW_NV_texgen_emboss) = glewGetExtension("GL_NV_texgen_emboss");
#endif /* GL_NV_texgen_emboss */
#ifdef GL_NV_texgen_reflection
  CONST_CAST(GLEW_NV_texgen_reflection) = glewGetExtension("GL_NV_texgen_reflection");
#endif /* GL_NV_texgen_reflection */
#ifdef GL_NV_texture_compression_vtc
  CONST_CAST(GLEW_NV_texture_compression_vtc) = glewGetExtension("GL_NV_texture_compression_vtc");
#endif /* GL_NV_texture_compression_vtc */
#ifdef GL_NV_texture_env_combine4
  CONST_CAST(GLEW_NV_texture_env_combine4) = glewGetExtension("GL_NV_texture_env_combine4");
#endif /* GL_NV_texture_env_combine4 */
#ifdef GL_NV_texture_expand_normal
  CONST_CAST(GLEW_NV_texture_expand_normal) = glewGetExtension("GL_NV_texture_expand_normal");
#endif /* GL_NV_texture_expand_normal */
#ifdef GL_NV_texture_rectangle
  CONST_CAST(GLEW_NV_texture_rectangle) = glewGetExtension("GL_NV_texture_rectangle");
#endif /* GL_NV_texture_rectangle */
#ifdef GL_NV_texture_shader
  CONST_CAST(GLEW_NV_texture_shader) = glewGetExtension("GL_NV_texture_shader");
#endif /* GL_NV_texture_shader */
#ifdef GL_NV_texture_shader2
  CONST_CAST(GLEW_NV_texture_shader2) = glewGetExtension("GL_NV_texture_shader2");
#endif /* GL_NV_texture_shader2 */
#ifdef GL_NV_texture_shader3
  CONST_CAST(GLEW_NV_texture_shader3) = glewGetExtension("GL_NV_texture_shader3");
#endif /* GL_NV_texture_shader3 */
#ifdef GL_NV_transform_feedback
  CONST_CAST(GLEW_NV_transform_feedback) = glewGetExtension("GL_NV_transform_feedback");
  if (glewExperimental || GLEW_NV_transform_feedback) CONST_CAST(GLEW_NV_transform_feedback) = !_glewInit_GL_NV_transform_feedback(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_transform_feedback */
#ifdef GL_NV_transform_feedback2
  CONST_CAST(GLEW_NV_transform_feedback2) = glewGetExtension("GL_NV_transform_feedback2");
  if (glewExperimental || GLEW_NV_transform_feedback2) CONST_CAST(GLEW_NV_transform_feedback2) = !_glewInit_GL_NV_transform_feedback2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_transform_feedback2 */
#ifdef GL_NV_vertex_array_range
  CONST_CAST(GLEW_NV_vertex_array_range) = glewGetExtension("GL_NV_vertex_array_range");
  if (glewExperimental || GLEW_NV_vertex_array_range) CONST_CAST(GLEW_NV_vertex_array_range) = !_glewInit_GL_NV_vertex_array_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_vertex_array_range */
#ifdef GL_NV_vertex_array_range2
  CONST_CAST(GLEW_NV_vertex_array_range2) = glewGetExtension("GL_NV_vertex_array_range2");
#endif /* GL_NV_vertex_array_range2 */
#ifdef GL_NV_vertex_program
  CONST_CAST(GLEW_NV_vertex_program) = glewGetExtension("GL_NV_vertex_program");
  if (glewExperimental || GLEW_NV_vertex_program) CONST_CAST(GLEW_NV_vertex_program) = !_glewInit_GL_NV_vertex_program(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_vertex_program */
#ifdef GL_NV_vertex_program1_1
  CONST_CAST(GLEW_NV_vertex_program1_1) = glewGetExtension("GL_NV_vertex_program1_1");
#endif /* GL_NV_vertex_program1_1 */
#ifdef GL_NV_vertex_program2
  CONST_CAST(GLEW_NV_vertex_program2) = glewGetExtension("GL_NV_vertex_program2");
#endif /* GL_NV_vertex_program2 */
#ifdef GL_NV_vertex_program2_option
  CONST_CAST(GLEW_NV_vertex_program2_option) = glewGetExtension("GL_NV_vertex_program2_option");
#endif /* GL_NV_vertex_program2_option */
#ifdef GL_NV_vertex_program3
  CONST_CAST(GLEW_NV_vertex_program3) = glewGetExtension("GL_NV_vertex_program3");
#endif /* GL_NV_vertex_program3 */
#ifdef GL_NV_vertex_program4
  CONST_CAST(GLEW_NV_vertex_program4) = glewGetExtension("GL_NV_vertex_program4");
#endif /* GL_NV_vertex_program4 */
#ifdef GL_OES_byte_coordinates
  CONST_CAST(GLEW_OES_byte_coordinates) = glewGetExtension("GL_OES_byte_coordinates");
#endif /* GL_OES_byte_coordinates */
#ifdef GL_OES_compressed_paletted_texture
  CONST_CAST(GLEW_OES_compressed_paletted_texture) = glewGetExtension("GL_OES_compressed_paletted_texture");
#endif /* GL_OES_compressed_paletted_texture */
#ifdef GL_OES_read_format
  CONST_CAST(GLEW_OES_read_format) = glewGetExtension("GL_OES_read_format");
#endif /* GL_OES_read_format */
#ifdef GL_OES_single_precision
  CONST_CAST(GLEW_OES_single_precision) = glewGetExtension("GL_OES_single_precision");
  if (glewExperimental || GLEW_OES_single_precision) CONST_CAST(GLEW_OES_single_precision) = !_glewInit_GL_OES_single_precision(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_OES_single_precision */
#ifdef GL_OML_interlace
  CONST_CAST(GLEW_OML_interlace) = glewGetExtension("GL_OML_interlace");
#endif /* GL_OML_interlace */
#ifdef GL_OML_resample
  CONST_CAST(GLEW_OML_resample) = glewGetExtension("GL_OML_resample");
#endif /* GL_OML_resample */
#ifdef GL_OML_subsample
  CONST_CAST(GLEW_OML_subsample) = glewGetExtension("GL_OML_subsample");
#endif /* GL_OML_subsample */
#ifdef GL_PGI_misc_hints
  CONST_CAST(GLEW_PGI_misc_hints) = glewGetExtension("GL_PGI_misc_hints");
#endif /* GL_PGI_misc_hints */
#ifdef GL_PGI_vertex_hints
  CONST_CAST(GLEW_PGI_vertex_hints) = glewGetExtension("GL_PGI_vertex_hints");
#endif /* GL_PGI_vertex_hints */
#ifdef GL_REND_screen_coordinates
  CONST_CAST(GLEW_REND_screen_coordinates) = glewGetExtension("GL_REND_screen_coordinates");
#endif /* GL_REND_screen_coordinates */
#ifdef GL_S3_s3tc
  CONST_CAST(GLEW_S3_s3tc) = glewGetExtension("GL_S3_s3tc");
#endif /* GL_S3_s3tc */
#ifdef GL_SGIS_color_range
  CONST_CAST(GLEW_SGIS_color_range) = glewGetExtension("GL_SGIS_color_range");
#endif /* GL_SGIS_color_range */
#ifdef GL_SGIS_detail_texture
  CONST_CAST(GLEW_SGIS_detail_texture) = glewGetExtension("GL_SGIS_detail_texture");
  if (glewExperimental || GLEW_SGIS_detail_texture) CONST_CAST(GLEW_SGIS_detail_texture) = !_glewInit_GL_SGIS_detail_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_detail_texture */
#ifdef GL_SGIS_fog_function
  CONST_CAST(GLEW_SGIS_fog_function) = glewGetExtension("GL_SGIS_fog_function");
  if (glewExperimental || GLEW_SGIS_fog_function) CONST_CAST(GLEW_SGIS_fog_function) = !_glewInit_GL_SGIS_fog_function(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_fog_function */
#ifdef GL_SGIS_generate_mipmap
  CONST_CAST(GLEW_SGIS_generate_mipmap) = glewGetExtension("GL_SGIS_generate_mipmap");
#endif /* GL_SGIS_generate_mipmap */
#ifdef GL_SGIS_multisample
  CONST_CAST(GLEW_SGIS_multisample) = glewGetExtension("GL_SGIS_multisample");
  if (glewExperimental || GLEW_SGIS_multisample) CONST_CAST(GLEW_SGIS_multisample) = !_glewInit_GL_SGIS_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_multisample */
#ifdef GL_SGIS_pixel_texture
  CONST_CAST(GLEW_SGIS_pixel_texture) = glewGetExtension("GL_SGIS_pixel_texture");
#endif /* GL_SGIS_pixel_texture */
#ifdef GL_SGIS_point_line_texgen
  CONST_CAST(GLEW_SGIS_point_line_texgen) = glewGetExtension("GL_SGIS_point_line_texgen");
#endif /* GL_SGIS_point_line_texgen */
#ifdef GL_SGIS_sharpen_texture
  CONST_CAST(GLEW_SGIS_sharpen_texture) = glewGetExtension("GL_SGIS_sharpen_texture");
  if (glewExperimental || GLEW_SGIS_sharpen_texture) CONST_CAST(GLEW_SGIS_sharpen_texture) = !_glewInit_GL_SGIS_sharpen_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_sharpen_texture */
#ifdef GL_SGIS_texture4D
  CONST_CAST(GLEW_SGIS_texture4D) = glewGetExtension("GL_SGIS_texture4D");
  if (glewExperimental || GLEW_SGIS_texture4D) CONST_CAST(GLEW_SGIS_texture4D) = !_glewInit_GL_SGIS_texture4D(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_texture4D */
#ifdef GL_SGIS_texture_border_clamp
  CONST_CAST(GLEW_SGIS_texture_border_clamp) = glewGetExtension("GL_SGIS_texture_border_clamp");
#endif /* GL_SGIS_texture_border_clamp */
#ifdef GL_SGIS_texture_edge_clamp
  CONST_CAST(GLEW_SGIS_texture_edge_clamp) = glewGetExtension("GL_SGIS_texture_edge_clamp");
#endif /* GL_SGIS_texture_edge_clamp */
#ifdef GL_SGIS_texture_filter4
  CONST_CAST(GLEW_SGIS_texture_filter4) = glewGetExtension("GL_SGIS_texture_filter4");
  if (glewExperimental || GLEW_SGIS_texture_filter4) CONST_CAST(GLEW_SGIS_texture_filter4) = !_glewInit_GL_SGIS_texture_filter4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_texture_filter4 */
#ifdef GL_SGIS_texture_lod
  CONST_CAST(GLEW_SGIS_texture_lod) = glewGetExtension("GL_SGIS_texture_lod");
#endif /* GL_SGIS_texture_lod */
#ifdef GL_SGIS_texture_select
  CONST_CAST(GLEW_SGIS_texture_select) = glewGetExtension("GL_SGIS_texture_select");
#endif /* GL_SGIS_texture_select */
#ifdef GL_SGIX_async
  CONST_CAST(GLEW_SGIX_async) = glewGetExtension("GL_SGIX_async");
  if (glewExperimental || GLEW_SGIX_async) CONST_CAST(GLEW_SGIX_async) = !_glewInit_GL_SGIX_async(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_async */
#ifdef GL_SGIX_async_histogram
  CONST_CAST(GLEW_SGIX_async_histogram) = glewGetExtension("GL_SGIX_async_histogram");
#endif /* GL_SGIX_async_histogram */
#ifdef GL_SGIX_async_pixel
  CONST_CAST(GLEW_SGIX_async_pixel) = glewGetExtension("GL_SGIX_async_pixel");
#endif /* GL_SGIX_async_pixel */
#ifdef GL_SGIX_blend_alpha_minmax
  CONST_CAST(GLEW_SGIX_blend_alpha_minmax) = glewGetExtension("GL_SGIX_blend_alpha_minmax");
#endif /* GL_SGIX_blend_alpha_minmax */
#ifdef GL_SGIX_clipmap
  CONST_CAST(GLEW_SGIX_clipmap) = glewGetExtension("GL_SGIX_clipmap");
#endif /* GL_SGIX_clipmap */
#ifdef GL_SGIX_convolution_accuracy
  CONST_CAST(GLEW_SGIX_convolution_accuracy) = glewGetExtension("GL_SGIX_convolution_accuracy");
#endif /* GL_SGIX_convolution_accuracy */
#ifdef GL_SGIX_depth_texture
  CONST_CAST(GLEW_SGIX_depth_texture) = glewGetExtension("GL_SGIX_depth_texture");
#endif /* GL_SGIX_depth_texture */
#ifdef GL_SGIX_flush_raster
  CONST_CAST(GLEW_SGIX_flush_raster) = glewGetExtension("GL_SGIX_flush_raster");
  if (glewExperimental || GLEW_SGIX_flush_raster) CONST_CAST(GLEW_SGIX_flush_raster) = !_glewInit_GL_SGIX_flush_raster(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_flush_raster */
#ifdef GL_SGIX_fog_offset
  CONST_CAST(GLEW_SGIX_fog_offset) = glewGetExtension("GL_SGIX_fog_offset");
#endif /* GL_SGIX_fog_offset */
#ifdef GL_SGIX_fog_texture
  CONST_CAST(GLEW_SGIX_fog_texture) = glewGetExtension("GL_SGIX_fog_texture");
  if (glewExperimental || GLEW_SGIX_fog_texture) CONST_CAST(GLEW_SGIX_fog_texture) = !_glewInit_GL_SGIX_fog_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_fog_texture */
#ifdef GL_SGIX_fragment_specular_lighting
  CONST_CAST(GLEW_SGIX_fragment_specular_lighting) = glewGetExtension("GL_SGIX_fragment_specular_lighting");
  if (glewExperimental || GLEW_SGIX_fragment_specular_lighting) CONST_CAST(GLEW_SGIX_fragment_specular_lighting) = !_glewInit_GL_SGIX_fragment_specular_lighting(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_fragment_specular_lighting */
#ifdef GL_SGIX_framezoom
  CONST_CAST(GLEW_SGIX_framezoom) = glewGetExtension("GL_SGIX_framezoom");
  if (glewExperimental || GLEW_SGIX_framezoom) CONST_CAST(GLEW_SGIX_framezoom) = !_glewInit_GL_SGIX_framezoom(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_framezoom */
#ifdef GL_SGIX_interlace
  CONST_CAST(GLEW_SGIX_interlace) = glewGetExtension("GL_SGIX_interlace");
#endif /* GL_SGIX_interlace */
#ifdef GL_SGIX_ir_instrument1
  CONST_CAST(GLEW_SGIX_ir_instrument1) = glewGetExtension("GL_SGIX_ir_instrument1");
#endif /* GL_SGIX_ir_instrument1 */
#ifdef GL_SGIX_list_priority
  CONST_CAST(GLEW_SGIX_list_priority) = glewGetExtension("GL_SGIX_list_priority");
#endif /* GL_SGIX_list_priority */
#ifdef GL_SGIX_pixel_texture
  CONST_CAST(GLEW_SGIX_pixel_texture) = glewGetExtension("GL_SGIX_pixel_texture");
  if (glewExperimental || GLEW_SGIX_pixel_texture) CONST_CAST(GLEW_SGIX_pixel_texture) = !_glewInit_GL_SGIX_pixel_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_pixel_texture */
#ifdef GL_SGIX_pixel_texture_bits
  CONST_CAST(GLEW_SGIX_pixel_texture_bits) = glewGetExtension("GL_SGIX_pixel_texture_bits");
#endif /* GL_SGIX_pixel_texture_bits */
#ifdef GL_SGIX_reference_plane
  CONST_CAST(GLEW_SGIX_reference_plane) = glewGetExtension("GL_SGIX_reference_plane");
  if (glewExperimental || GLEW_SGIX_reference_plane) CONST_CAST(GLEW_SGIX_reference_plane) = !_glewInit_GL_SGIX_reference_plane(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_reference_plane */
#ifdef GL_SGIX_resample
  CONST_CAST(GLEW_SGIX_resample) = glewGetExtension("GL_SGIX_resample");
#endif /* GL_SGIX_resample */
#ifdef GL_SGIX_shadow
  CONST_CAST(GLEW_SGIX_shadow) = glewGetExtension("GL_SGIX_shadow");
#endif /* GL_SGIX_shadow */
#ifdef GL_SGIX_shadow_ambient
  CONST_CAST(GLEW_SGIX_shadow_ambient) = glewGetExtension("GL_SGIX_shadow_ambient");
#endif /* GL_SGIX_shadow_ambient */
#ifdef GL_SGIX_sprite
  CONST_CAST(GLEW_SGIX_sprite) = glewGetExtension("GL_SGIX_sprite");
  if (glewExperimental || GLEW_SGIX_sprite) CONST_CAST(GLEW_SGIX_sprite) = !_glewInit_GL_SGIX_sprite(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_sprite */
#ifdef GL_SGIX_tag_sample_buffer
  CONST_CAST(GLEW_SGIX_tag_sample_buffer) = glewGetExtension("GL_SGIX_tag_sample_buffer");
  if (glewExperimental || GLEW_SGIX_tag_sample_buffer) CONST_CAST(GLEW_SGIX_tag_sample_buffer) = !_glewInit_GL_SGIX_tag_sample_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_tag_sample_buffer */
#ifdef GL_SGIX_texture_add_env
  CONST_CAST(GLEW_SGIX_texture_add_env) = glewGetExtension("GL_SGIX_texture_add_env");
#endif /* GL_SGIX_texture_add_env */
#ifdef GL_SGIX_texture_coordinate_clamp
  CONST_CAST(GLEW_SGIX_texture_coordinate_clamp) = glewGetExtension("GL_SGIX_texture_coordinate_clamp");
#endif /* GL_SGIX_texture_coordinate_clamp */
#ifdef GL_SGIX_texture_lod_bias
  CONST_CAST(GLEW_SGIX_texture_lod_bias) = glewGetExtension("GL_SGIX_texture_lod_bias");
#endif /* GL_SGIX_texture_lod_bias */
#ifdef GL_SGIX_texture_multi_buffer
  CONST_CAST(GLEW_SGIX_texture_multi_buffer) = glewGetExtension("GL_SGIX_texture_multi_buffer");
#endif /* GL_SGIX_texture_multi_buffer */
#ifdef GL_SGIX_texture_range
  CONST_CAST(GLEW_SGIX_texture_range) = glewGetExtension("GL_SGIX_texture_range");
#endif /* GL_SGIX_texture_range */
#ifdef GL_SGIX_texture_scale_bias
  CONST_CAST(GLEW_SGIX_texture_scale_bias) = glewGetExtension("GL_SGIX_texture_scale_bias");
#endif /* GL_SGIX_texture_scale_bias */
#ifdef GL_SGIX_vertex_preclip
  CONST_CAST(GLEW_SGIX_vertex_preclip) = glewGetExtension("GL_SGIX_vertex_preclip");
#endif /* GL_SGIX_vertex_preclip */
#ifdef GL_SGIX_vertex_preclip_hint
  CONST_CAST(GLEW_SGIX_vertex_preclip_hint) = glewGetExtension("GL_SGIX_vertex_preclip_hint");
#endif /* GL_SGIX_vertex_preclip_hint */
#ifdef GL_SGIX_ycrcb
  CONST_CAST(GLEW_SGIX_ycrcb) = glewGetExtension("GL_SGIX_ycrcb");
#endif /* GL_SGIX_ycrcb */
#ifdef GL_SGI_color_matrix
  CONST_CAST(GLEW_SGI_color_matrix) = glewGetExtension("GL_SGI_color_matrix");
#endif /* GL_SGI_color_matrix */
#ifdef GL_SGI_color_table
  CONST_CAST(GLEW_SGI_color_table) = glewGetExtension("GL_SGI_color_table");
  if (glewExperimental || GLEW_SGI_color_table) CONST_CAST(GLEW_SGI_color_table) = !_glewInit_GL_SGI_color_table(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGI_color_table */
#ifdef GL_SGI_texture_color_table
  CONST_CAST(GLEW_SGI_texture_color_table) = glewGetExtension("GL_SGI_texture_color_table");
#endif /* GL_SGI_texture_color_table */
#ifdef GL_SUNX_constant_data
  CONST_CAST(GLEW_SUNX_constant_data) = glewGetExtension("GL_SUNX_constant_data");
  if (glewExperimental || GLEW_SUNX_constant_data) CONST_CAST(GLEW_SUNX_constant_data) = !_glewInit_GL_SUNX_constant_data(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUNX_constant_data */
#ifdef GL_SUN_convolution_border_modes
  CONST_CAST(GLEW_SUN_convolution_border_modes) = glewGetExtension("GL_SUN_convolution_border_modes");
#endif /* GL_SUN_convolution_border_modes */
#ifdef GL_SUN_global_alpha
  CONST_CAST(GLEW_SUN_global_alpha) = glewGetExtension("GL_SUN_global_alpha");
  if (glewExperimental || GLEW_SUN_global_alpha) CONST_CAST(GLEW_SUN_global_alpha) = !_glewInit_GL_SUN_global_alpha(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUN_global_alpha */
#ifdef GL_SUN_mesh_array
  CONST_CAST(GLEW_SUN_mesh_array) = glewGetExtension("GL_SUN_mesh_array");
#endif /* GL_SUN_mesh_array */
#ifdef GL_SUN_read_video_pixels
  CONST_CAST(GLEW_SUN_read_video_pixels) = glewGetExtension("GL_SUN_read_video_pixels");
  if (glewExperimental || GLEW_SUN_read_video_pixels) CONST_CAST(GLEW_SUN_read_video_pixels) = !_glewInit_GL_SUN_read_video_pixels(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUN_read_video_pixels */
#ifdef GL_SUN_slice_accum
  CONST_CAST(GLEW_SUN_slice_accum) = glewGetExtension("GL_SUN_slice_accum");
#endif /* GL_SUN_slice_accum */
#ifdef GL_SUN_triangle_list
  CONST_CAST(GLEW_SUN_triangle_list) = glewGetExtension("GL_SUN_triangle_list");
  if (glewExperimental || GLEW_SUN_triangle_list) CONST_CAST(GLEW_SUN_triangle_list) = !_glewInit_GL_SUN_triangle_list(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUN_triangle_list */
#ifdef GL_SUN_vertex
  CONST_CAST(GLEW_SUN_vertex) = glewGetExtension("GL_SUN_vertex");
  if (glewExperimental || GLEW_SUN_vertex) CONST_CAST(GLEW_SUN_vertex) = !_glewInit_GL_SUN_vertex(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUN_vertex */
#ifdef GL_WIN_phong_shading
  CONST_CAST(GLEW_WIN_phong_shading) = glewGetExtension("GL_WIN_phong_shading");
#endif /* GL_WIN_phong_shading */
#ifdef GL_WIN_specular_fog
  CONST_CAST(GLEW_WIN_specular_fog) = glewGetExtension("GL_WIN_specular_fog");
#endif /* GL_WIN_specular_fog */
#ifdef GL_WIN_swap_hint
  CONST_CAST(GLEW_WIN_swap_hint) = glewGetExtension("GL_WIN_swap_hint");
  if (glewExperimental || GLEW_WIN_swap_hint) CONST_CAST(GLEW_WIN_swap_hint) = !_glewInit_GL_WIN_swap_hint(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_WIN_swap_hint */

  return GLEW_OK;
}


#if defined(_WIN32)

#if !defined(GLEW_MX)

PFNWGLSETSTEREOEMITTERSTATE3DLPROC __wglewSetStereoEmitterState3DL = NULL;

PFNWGLCREATEBUFFERREGIONARBPROC __wglewCreateBufferRegionARB = NULL;
PFNWGLDELETEBUFFERREGIONARBPROC __wglewDeleteBufferRegionARB = NULL;
PFNWGLRESTOREBUFFERREGIONARBPROC __wglewRestoreBufferRegionARB = NULL;
PFNWGLSAVEBUFFERREGIONARBPROC __wglewSaveBufferRegionARB = NULL;

PFNWGLCREATECONTEXTATTRIBSARBPROC __wglewCreateContextAttribsARB = NULL;

PFNWGLGETEXTENSIONSSTRINGARBPROC __wglewGetExtensionsStringARB = NULL;

PFNWGLGETCURRENTREADDCARBPROC __wglewGetCurrentReadDCARB = NULL;
PFNWGLMAKECONTEXTCURRENTARBPROC __wglewMakeContextCurrentARB = NULL;

PFNWGLCREATEPBUFFERARBPROC __wglewCreatePbufferARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC __wglewDestroyPbufferARB = NULL;
PFNWGLGETPBUFFERDCARBPROC __wglewGetPbufferDCARB = NULL;
PFNWGLQUERYPBUFFERARBPROC __wglewQueryPbufferARB = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC __wglewReleasePbufferDCARB = NULL;

PFNWGLCHOOSEPIXELFORMATARBPROC __wglewChoosePixelFormatARB = NULL;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC __wglewGetPixelFormatAttribfvARB = NULL;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC __wglewGetPixelFormatAttribivARB = NULL;

PFNWGLBINDTEXIMAGEARBPROC __wglewBindTexImageARB = NULL;
PFNWGLRELEASETEXIMAGEARBPROC __wglewReleaseTexImageARB = NULL;
PFNWGLSETPBUFFERATTRIBARBPROC __wglewSetPbufferAttribARB = NULL;

PFNWGLBINDDISPLAYCOLORTABLEEXTPROC __wglewBindDisplayColorTableEXT = NULL;
PFNWGLCREATEDISPLAYCOLORTABLEEXTPROC __wglewCreateDisplayColorTableEXT = NULL;
PFNWGLDESTROYDISPLAYCOLORTABLEEXTPROC __wglewDestroyDisplayColorTableEXT = NULL;
PFNWGLLOADDISPLAYCOLORTABLEEXTPROC __wglewLoadDisplayColorTableEXT = NULL;

PFNWGLGETEXTENSIONSSTRINGEXTPROC __wglewGetExtensionsStringEXT = NULL;

PFNWGLGETCURRENTREADDCEXTPROC __wglewGetCurrentReadDCEXT = NULL;
PFNWGLMAKECONTEXTCURRENTEXTPROC __wglewMakeContextCurrentEXT = NULL;

PFNWGLCREATEPBUFFEREXTPROC __wglewCreatePbufferEXT = NULL;
PFNWGLDESTROYPBUFFEREXTPROC __wglewDestroyPbufferEXT = NULL;
PFNWGLGETPBUFFERDCEXTPROC __wglewGetPbufferDCEXT = NULL;
PFNWGLQUERYPBUFFEREXTPROC __wglewQueryPbufferEXT = NULL;
PFNWGLRELEASEPBUFFERDCEXTPROC __wglewReleasePbufferDCEXT = NULL;

PFNWGLCHOOSEPIXELFORMATEXTPROC __wglewChoosePixelFormatEXT = NULL;
PFNWGLGETPIXELFORMATATTRIBFVEXTPROC __wglewGetPixelFormatAttribfvEXT = NULL;
PFNWGLGETPIXELFORMATATTRIBIVEXTPROC __wglewGetPixelFormatAttribivEXT = NULL;

PFNWGLGETSWAPINTERVALEXTPROC __wglewGetSwapIntervalEXT = NULL;
PFNWGLSWAPINTERVALEXTPROC __wglewSwapIntervalEXT = NULL;

PFNWGLGETDIGITALVIDEOPARAMETERSI3DPROC __wglewGetDigitalVideoParametersI3D = NULL;
PFNWGLSETDIGITALVIDEOPARAMETERSI3DPROC __wglewSetDigitalVideoParametersI3D = NULL;

PFNWGLGETGAMMATABLEI3DPROC __wglewGetGammaTableI3D = NULL;
PFNWGLGETGAMMATABLEPARAMETERSI3DPROC __wglewGetGammaTableParametersI3D = NULL;
PFNWGLSETGAMMATABLEI3DPROC __wglewSetGammaTableI3D = NULL;
PFNWGLSETGAMMATABLEPARAMETERSI3DPROC __wglewSetGammaTableParametersI3D = NULL;

PFNWGLDISABLEGENLOCKI3DPROC __wglewDisableGenlockI3D = NULL;
PFNWGLENABLEGENLOCKI3DPROC __wglewEnableGenlockI3D = NULL;
PFNWGLGENLOCKSAMPLERATEI3DPROC __wglewGenlockSampleRateI3D = NULL;
PFNWGLGENLOCKSOURCEDELAYI3DPROC __wglewGenlockSourceDelayI3D = NULL;
PFNWGLGENLOCKSOURCEEDGEI3DPROC __wglewGenlockSourceEdgeI3D = NULL;
PFNWGLGENLOCKSOURCEI3DPROC __wglewGenlockSourceI3D = NULL;
PFNWGLGETGENLOCKSAMPLERATEI3DPROC __wglewGetGenlockSampleRateI3D = NULL;
PFNWGLGETGENLOCKSOURCEDELAYI3DPROC __wglewGetGenlockSourceDelayI3D = NULL;
PFNWGLGETGENLOCKSOURCEEDGEI3DPROC __wglewGetGenlockSourceEdgeI3D = NULL;
PFNWGLGETGENLOCKSOURCEI3DPROC __wglewGetGenlockSourceI3D = NULL;
PFNWGLISENABLEDGENLOCKI3DPROC __wglewIsEnabledGenlockI3D = NULL;
PFNWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC __wglewQueryGenlockMaxSourceDelayI3D = NULL;

PFNWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC __wglewAssociateImageBufferEventsI3D = NULL;
PFNWGLCREATEIMAGEBUFFERI3DPROC __wglewCreateImageBufferI3D = NULL;
PFNWGLDESTROYIMAGEBUFFERI3DPROC __wglewDestroyImageBufferI3D = NULL;
PFNWGLRELEASEIMAGEBUFFEREVENTSI3DPROC __wglewReleaseImageBufferEventsI3D = NULL;

PFNWGLDISABLEFRAMELOCKI3DPROC __wglewDisableFrameLockI3D = NULL;
PFNWGLENABLEFRAMELOCKI3DPROC __wglewEnableFrameLockI3D = NULL;
PFNWGLISENABLEDFRAMELOCKI3DPROC __wglewIsEnabledFrameLockI3D = NULL;
PFNWGLQUERYFRAMELOCKMASTERI3DPROC __wglewQueryFrameLockMasterI3D = NULL;

PFNWGLBEGINFRAMETRACKINGI3DPROC __wglewBeginFrameTrackingI3D = NULL;
PFNWGLENDFRAMETRACKINGI3DPROC __wglewEndFrameTrackingI3D = NULL;
PFNWGLGETFRAMEUSAGEI3DPROC __wglewGetFrameUsageI3D = NULL;
PFNWGLQUERYFRAMETRACKINGI3DPROC __wglewQueryFrameTrackingI3D = NULL;

PFNWGLCREATEAFFINITYDCNVPROC __wglewCreateAffinityDCNV = NULL;
PFNWGLDELETEDCNVPROC __wglewDeleteDCNV = NULL;
PFNWGLENUMGPUDEVICESNVPROC __wglewEnumGpuDevicesNV = NULL;
PFNWGLENUMGPUSFROMAFFINITYDCNVPROC __wglewEnumGpusFromAffinityDCNV = NULL;
PFNWGLENUMGPUSNVPROC __wglewEnumGpusNV = NULL;

PFNWGLBINDVIDEODEVICENVPROC __wglewBindVideoDeviceNV = NULL;
PFNWGLENUMERATEVIDEODEVICESNVPROC __wglewEnumerateVideoDevicesNV = NULL;
PFNWGLQUERYCURRENTCONTEXTNVPROC __wglewQueryCurrentContextNV = NULL;

PFNWGLBINDSWAPBARRIERNVPROC __wglewBindSwapBarrierNV = NULL;
PFNWGLJOINSWAPGROUPNVPROC __wglewJoinSwapGroupNV = NULL;
PFNWGLQUERYFRAMECOUNTNVPROC __wglewQueryFrameCountNV = NULL;
PFNWGLQUERYMAXSWAPGROUPSNVPROC __wglewQueryMaxSwapGroupsNV = NULL;
PFNWGLQUERYSWAPGROUPNVPROC __wglewQuerySwapGroupNV = NULL;
PFNWGLRESETFRAMECOUNTNVPROC __wglewResetFrameCountNV = NULL;

PFNWGLALLOCATEMEMORYNVPROC __wglewAllocateMemoryNV = NULL;
PFNWGLFREEMEMORYNVPROC __wglewFreeMemoryNV = NULL;

PFNWGLBINDVIDEOIMAGENVPROC __wglewBindVideoImageNV = NULL;
PFNWGLGETVIDEODEVICENVPROC __wglewGetVideoDeviceNV = NULL;
PFNWGLGETVIDEOINFONVPROC __wglewGetVideoInfoNV = NULL;
PFNWGLRELEASEVIDEODEVICENVPROC __wglewReleaseVideoDeviceNV = NULL;
PFNWGLRELEASEVIDEOIMAGENVPROC __wglewReleaseVideoImageNV = NULL;
PFNWGLSENDPBUFFERTOVIDEONVPROC __wglewSendPbufferToVideoNV = NULL;

PFNWGLGETMSCRATEOMLPROC __wglewGetMscRateOML = NULL;
PFNWGLGETSYNCVALUESOMLPROC __wglewGetSyncValuesOML = NULL;
PFNWGLSWAPBUFFERSMSCOMLPROC __wglewSwapBuffersMscOML = NULL;
PFNWGLSWAPLAYERBUFFERSMSCOMLPROC __wglewSwapLayerBuffersMscOML = NULL;
PFNWGLWAITFORMSCOMLPROC __wglewWaitForMscOML = NULL;
PFNWGLWAITFORSBCOMLPROC __wglewWaitForSbcOML = NULL;
GLboolean __WGLEW_3DFX_multisample = GL_FALSE;
GLboolean __WGLEW_3DL_stereo_control = GL_FALSE;
GLboolean __WGLEW_ARB_buffer_region = GL_FALSE;
GLboolean __WGLEW_ARB_create_context = GL_FALSE;
GLboolean __WGLEW_ARB_extensions_string = GL_FALSE;
GLboolean __WGLEW_ARB_framebuffer_sRGB = GL_FALSE;
GLboolean __WGLEW_ARB_make_current_read = GL_FALSE;
GLboolean __WGLEW_ARB_multisample = GL_FALSE;
GLboolean __WGLEW_ARB_pbuffer = GL_FALSE;
GLboolean __WGLEW_ARB_pixel_format = GL_FALSE;
GLboolean __WGLEW_ARB_pixel_format_float = GL_FALSE;
GLboolean __WGLEW_ARB_render_texture = GL_FALSE;
GLboolean __WGLEW_ATI_pixel_format_float = GL_FALSE;
GLboolean __WGLEW_ATI_render_texture_rectangle = GL_FALSE;
GLboolean __WGLEW_EXT_depth_float = GL_FALSE;
GLboolean __WGLEW_EXT_display_color_table = GL_FALSE;
GLboolean __WGLEW_EXT_extensions_string = GL_FALSE;
GLboolean __WGLEW_EXT_framebuffer_sRGB = GL_FALSE;
GLboolean __WGLEW_EXT_make_current_read = GL_FALSE;
GLboolean __WGLEW_EXT_multisample = GL_FALSE;
GLboolean __WGLEW_EXT_pbuffer = GL_FALSE;
GLboolean __WGLEW_EXT_pixel_format = GL_FALSE;
GLboolean __WGLEW_EXT_pixel_format_packed_float = GL_FALSE;
GLboolean __WGLEW_EXT_swap_control = GL_FALSE;
GLboolean __WGLEW_I3D_digital_video_control = GL_FALSE;
GLboolean __WGLEW_I3D_gamma = GL_FALSE;
GLboolean __WGLEW_I3D_genlock = GL_FALSE;
GLboolean __WGLEW_I3D_image_buffer = GL_FALSE;
GLboolean __WGLEW_I3D_swap_frame_lock = GL_FALSE;
GLboolean __WGLEW_I3D_swap_frame_usage = GL_FALSE;
GLboolean __WGLEW_NV_float_buffer = GL_FALSE;
GLboolean __WGLEW_NV_gpu_affinity = GL_FALSE;
GLboolean __WGLEW_NV_present_video = GL_FALSE;
GLboolean __WGLEW_NV_render_depth_texture = GL_FALSE;
GLboolean __WGLEW_NV_render_texture_rectangle = GL_FALSE;
GLboolean __WGLEW_NV_swap_group = GL_FALSE;
GLboolean __WGLEW_NV_vertex_array_range = GL_FALSE;
GLboolean __WGLEW_NV_video_output = GL_FALSE;
GLboolean __WGLEW_OML_sync_control = GL_FALSE;

#endif /* !GLEW_MX */

#ifdef WGL_3DFX_multisample

#endif /* WGL_3DFX_multisample */

#ifdef WGL_3DL_stereo_control

static GLboolean _glewInit_WGL_3DL_stereo_control (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglSetStereoEmitterState3DL = (PFNWGLSETSTEREOEMITTERSTATE3DLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSetStereoEmitterState3DL"))) == NULL) || r;

  return r;
}

#endif /* WGL_3DL_stereo_control */

#ifdef WGL_ARB_buffer_region

static GLboolean _glewInit_WGL_ARB_buffer_region (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreateBufferRegionARB = (PFNWGLCREATEBUFFERREGIONARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglCreateBufferRegionARB"))) == NULL) || r;
  r = ((wglDeleteBufferRegionARB = (PFNWGLDELETEBUFFERREGIONARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglDeleteBufferRegionARB"))) == NULL) || r;
  r = ((wglRestoreBufferRegionARB = (PFNWGLRESTOREBUFFERREGIONARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglRestoreBufferRegionARB"))) == NULL) || r;
  r = ((wglSaveBufferRegionARB = (PFNWGLSAVEBUFFERREGIONARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSaveBufferRegionARB"))) == NULL) || r;

  return r;
}

#endif /* WGL_ARB_buffer_region */

#ifdef WGL_ARB_create_context

static GLboolean _glewInit_WGL_ARB_create_context (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglCreateContextAttribsARB"))) == NULL) || r;

  return r;
}

#endif /* WGL_ARB_create_context */

#ifdef WGL_ARB_extensions_string

static GLboolean _glewInit_WGL_ARB_extensions_string (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetExtensionsStringARB"))) == NULL) || r;

  return r;
}

#endif /* WGL_ARB_extensions_string */

#ifdef WGL_ARB_framebuffer_sRGB

#endif /* WGL_ARB_framebuffer_sRGB */

#ifdef WGL_ARB_make_current_read

static GLboolean _glewInit_WGL_ARB_make_current_read (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetCurrentReadDCARB = (PFNWGLGETCURRENTREADDCARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetCurrentReadDCARB"))) == NULL) || r;
  r = ((wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglMakeContextCurrentARB"))) == NULL) || r;

  return r;
}

#endif /* WGL_ARB_make_current_read */

#ifdef WGL_ARB_multisample

#endif /* WGL_ARB_multisample */

#ifdef WGL_ARB_pbuffer

static GLboolean _glewInit_WGL_ARB_pbuffer (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglCreatePbufferARB"))) == NULL) || r;
  r = ((wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglDestroyPbufferARB"))) == NULL) || r;
  r = ((wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetPbufferDCARB"))) == NULL) || r;
  r = ((wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQueryPbufferARB"))) == NULL) || r;
  r = ((wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglReleasePbufferDCARB"))) == NULL) || r;

  return r;
}

#endif /* WGL_ARB_pbuffer */

#ifdef WGL_ARB_pixel_format

static GLboolean _glewInit_WGL_ARB_pixel_format (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglChoosePixelFormatARB"))) == NULL) || r;
  r = ((wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetPixelFormatAttribfvARB"))) == NULL) || r;
  r = ((wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetPixelFormatAttribivARB"))) == NULL) || r;

  return r;
}

#endif /* WGL_ARB_pixel_format */

#ifdef WGL_ARB_pixel_format_float

#endif /* WGL_ARB_pixel_format_float */

#ifdef WGL_ARB_render_texture

static GLboolean _glewInit_WGL_ARB_render_texture (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindTexImageARB = (PFNWGLBINDTEXIMAGEARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglBindTexImageARB"))) == NULL) || r;
  r = ((wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglReleaseTexImageARB"))) == NULL) || r;
  r = ((wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSetPbufferAttribARB"))) == NULL) || r;

  return r;
}

#endif /* WGL_ARB_render_texture */

#ifdef WGL_ATI_pixel_format_float

#endif /* WGL_ATI_pixel_format_float */

#ifdef WGL_ATI_render_texture_rectangle

#endif /* WGL_ATI_render_texture_rectangle */

#ifdef WGL_EXT_depth_float

#endif /* WGL_EXT_depth_float */

#ifdef WGL_EXT_display_color_table

static GLboolean _glewInit_WGL_EXT_display_color_table (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindDisplayColorTableEXT = (PFNWGLBINDDISPLAYCOLORTABLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglBindDisplayColorTableEXT"))) == NULL) || r;
  r = ((wglCreateDisplayColorTableEXT = (PFNWGLCREATEDISPLAYCOLORTABLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglCreateDisplayColorTableEXT"))) == NULL) || r;
  r = ((wglDestroyDisplayColorTableEXT = (PFNWGLDESTROYDISPLAYCOLORTABLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglDestroyDisplayColorTableEXT"))) == NULL) || r;
  r = ((wglLoadDisplayColorTableEXT = (PFNWGLLOADDISPLAYCOLORTABLEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglLoadDisplayColorTableEXT"))) == NULL) || r;

  return r;
}

#endif /* WGL_EXT_display_color_table */

#ifdef WGL_EXT_extensions_string

static GLboolean _glewInit_WGL_EXT_extensions_string (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetExtensionsStringEXT"))) == NULL) || r;

  return r;
}

#endif /* WGL_EXT_extensions_string */

#ifdef WGL_EXT_framebuffer_sRGB

#endif /* WGL_EXT_framebuffer_sRGB */

#ifdef WGL_EXT_make_current_read

static GLboolean _glewInit_WGL_EXT_make_current_read (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetCurrentReadDCEXT = (PFNWGLGETCURRENTREADDCEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetCurrentReadDCEXT"))) == NULL) || r;
  r = ((wglMakeContextCurrentEXT = (PFNWGLMAKECONTEXTCURRENTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglMakeContextCurrentEXT"))) == NULL) || r;

  return r;
}

#endif /* WGL_EXT_make_current_read */

#ifdef WGL_EXT_multisample

#endif /* WGL_EXT_multisample */

#ifdef WGL_EXT_pbuffer

static GLboolean _glewInit_WGL_EXT_pbuffer (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreatePbufferEXT = (PFNWGLCREATEPBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglCreatePbufferEXT"))) == NULL) || r;
  r = ((wglDestroyPbufferEXT = (PFNWGLDESTROYPBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglDestroyPbufferEXT"))) == NULL) || r;
  r = ((wglGetPbufferDCEXT = (PFNWGLGETPBUFFERDCEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetPbufferDCEXT"))) == NULL) || r;
  r = ((wglQueryPbufferEXT = (PFNWGLQUERYPBUFFEREXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQueryPbufferEXT"))) == NULL) || r;
  r = ((wglReleasePbufferDCEXT = (PFNWGLRELEASEPBUFFERDCEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglReleasePbufferDCEXT"))) == NULL) || r;

  return r;
}

#endif /* WGL_EXT_pbuffer */

#ifdef WGL_EXT_pixel_format

static GLboolean _glewInit_WGL_EXT_pixel_format (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglChoosePixelFormatEXT = (PFNWGLCHOOSEPIXELFORMATEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglChoosePixelFormatEXT"))) == NULL) || r;
  r = ((wglGetPixelFormatAttribfvEXT = (PFNWGLGETPIXELFORMATATTRIBFVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetPixelFormatAttribfvEXT"))) == NULL) || r;
  r = ((wglGetPixelFormatAttribivEXT = (PFNWGLGETPIXELFORMATATTRIBIVEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetPixelFormatAttribivEXT"))) == NULL) || r;

  return r;
}

#endif /* WGL_EXT_pixel_format */

#ifdef WGL_EXT_pixel_format_packed_float

#endif /* WGL_EXT_pixel_format_packed_float */

#ifdef WGL_EXT_swap_control

static GLboolean _glewInit_WGL_EXT_swap_control (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetSwapIntervalEXT"))) == NULL) || r;
  r = ((wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSwapIntervalEXT"))) == NULL) || r;

  return r;
}

#endif /* WGL_EXT_swap_control */

#ifdef WGL_I3D_digital_video_control

static GLboolean _glewInit_WGL_I3D_digital_video_control (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetDigitalVideoParametersI3D = (PFNWGLGETDIGITALVIDEOPARAMETERSI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetDigitalVideoParametersI3D"))) == NULL) || r;
  r = ((wglSetDigitalVideoParametersI3D = (PFNWGLSETDIGITALVIDEOPARAMETERSI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSetDigitalVideoParametersI3D"))) == NULL) || r;

  return r;
}

#endif /* WGL_I3D_digital_video_control */

#ifdef WGL_I3D_gamma

static GLboolean _glewInit_WGL_I3D_gamma (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetGammaTableI3D = (PFNWGLGETGAMMATABLEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetGammaTableI3D"))) == NULL) || r;
  r = ((wglGetGammaTableParametersI3D = (PFNWGLGETGAMMATABLEPARAMETERSI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetGammaTableParametersI3D"))) == NULL) || r;
  r = ((wglSetGammaTableI3D = (PFNWGLSETGAMMATABLEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSetGammaTableI3D"))) == NULL) || r;
  r = ((wglSetGammaTableParametersI3D = (PFNWGLSETGAMMATABLEPARAMETERSI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSetGammaTableParametersI3D"))) == NULL) || r;

  return r;
}

#endif /* WGL_I3D_gamma */

#ifdef WGL_I3D_genlock

static GLboolean _glewInit_WGL_I3D_genlock (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglDisableGenlockI3D = (PFNWGLDISABLEGENLOCKI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglDisableGenlockI3D"))) == NULL) || r;
  r = ((wglEnableGenlockI3D = (PFNWGLENABLEGENLOCKI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglEnableGenlockI3D"))) == NULL) || r;
  r = ((wglGenlockSampleRateI3D = (PFNWGLGENLOCKSAMPLERATEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGenlockSampleRateI3D"))) == NULL) || r;
  r = ((wglGenlockSourceDelayI3D = (PFNWGLGENLOCKSOURCEDELAYI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGenlockSourceDelayI3D"))) == NULL) || r;
  r = ((wglGenlockSourceEdgeI3D = (PFNWGLGENLOCKSOURCEEDGEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGenlockSourceEdgeI3D"))) == NULL) || r;
  r = ((wglGenlockSourceI3D = (PFNWGLGENLOCKSOURCEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGenlockSourceI3D"))) == NULL) || r;
  r = ((wglGetGenlockSampleRateI3D = (PFNWGLGETGENLOCKSAMPLERATEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetGenlockSampleRateI3D"))) == NULL) || r;
  r = ((wglGetGenlockSourceDelayI3D = (PFNWGLGETGENLOCKSOURCEDELAYI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetGenlockSourceDelayI3D"))) == NULL) || r;
  r = ((wglGetGenlockSourceEdgeI3D = (PFNWGLGETGENLOCKSOURCEEDGEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetGenlockSourceEdgeI3D"))) == NULL) || r;
  r = ((wglGetGenlockSourceI3D = (PFNWGLGETGENLOCKSOURCEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetGenlockSourceI3D"))) == NULL) || r;
  r = ((wglIsEnabledGenlockI3D = (PFNWGLISENABLEDGENLOCKI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglIsEnabledGenlockI3D"))) == NULL) || r;
  r = ((wglQueryGenlockMaxSourceDelayI3D = (PFNWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQueryGenlockMaxSourceDelayI3D"))) == NULL) || r;

  return r;
}

#endif /* WGL_I3D_genlock */

#ifdef WGL_I3D_image_buffer

static GLboolean _glewInit_WGL_I3D_image_buffer (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglAssociateImageBufferEventsI3D = (PFNWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglAssociateImageBufferEventsI3D"))) == NULL) || r;
  r = ((wglCreateImageBufferI3D = (PFNWGLCREATEIMAGEBUFFERI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglCreateImageBufferI3D"))) == NULL) || r;
  r = ((wglDestroyImageBufferI3D = (PFNWGLDESTROYIMAGEBUFFERI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglDestroyImageBufferI3D"))) == NULL) || r;
  r = ((wglReleaseImageBufferEventsI3D = (PFNWGLRELEASEIMAGEBUFFEREVENTSI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglReleaseImageBufferEventsI3D"))) == NULL) || r;

  return r;
}

#endif /* WGL_I3D_image_buffer */

#ifdef WGL_I3D_swap_frame_lock

static GLboolean _glewInit_WGL_I3D_swap_frame_lock (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglDisableFrameLockI3D = (PFNWGLDISABLEFRAMELOCKI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglDisableFrameLockI3D"))) == NULL) || r;
  r = ((wglEnableFrameLockI3D = (PFNWGLENABLEFRAMELOCKI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglEnableFrameLockI3D"))) == NULL) || r;
  r = ((wglIsEnabledFrameLockI3D = (PFNWGLISENABLEDFRAMELOCKI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglIsEnabledFrameLockI3D"))) == NULL) || r;
  r = ((wglQueryFrameLockMasterI3D = (PFNWGLQUERYFRAMELOCKMASTERI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQueryFrameLockMasterI3D"))) == NULL) || r;

  return r;
}

#endif /* WGL_I3D_swap_frame_lock */

#ifdef WGL_I3D_swap_frame_usage

static GLboolean _glewInit_WGL_I3D_swap_frame_usage (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBeginFrameTrackingI3D = (PFNWGLBEGINFRAMETRACKINGI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglBeginFrameTrackingI3D"))) == NULL) || r;
  r = ((wglEndFrameTrackingI3D = (PFNWGLENDFRAMETRACKINGI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglEndFrameTrackingI3D"))) == NULL) || r;
  r = ((wglGetFrameUsageI3D = (PFNWGLGETFRAMEUSAGEI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetFrameUsageI3D"))) == NULL) || r;
  r = ((wglQueryFrameTrackingI3D = (PFNWGLQUERYFRAMETRACKINGI3DPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQueryFrameTrackingI3D"))) == NULL) || r;

  return r;
}

#endif /* WGL_I3D_swap_frame_usage */

#ifdef WGL_NV_float_buffer

#endif /* WGL_NV_float_buffer */

#ifdef WGL_NV_gpu_affinity

static GLboolean _glewInit_WGL_NV_gpu_affinity (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreateAffinityDCNV = (PFNWGLCREATEAFFINITYDCNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglCreateAffinityDCNV"))) == NULL) || r;
  r = ((wglDeleteDCNV = (PFNWGLDELETEDCNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglDeleteDCNV"))) == NULL) || r;
  r = ((wglEnumGpuDevicesNV = (PFNWGLENUMGPUDEVICESNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglEnumGpuDevicesNV"))) == NULL) || r;
  r = ((wglEnumGpusFromAffinityDCNV = (PFNWGLENUMGPUSFROMAFFINITYDCNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglEnumGpusFromAffinityDCNV"))) == NULL) || r;
  r = ((wglEnumGpusNV = (PFNWGLENUMGPUSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglEnumGpusNV"))) == NULL) || r;

  return r;
}

#endif /* WGL_NV_gpu_affinity */

#ifdef WGL_NV_present_video

static GLboolean _glewInit_WGL_NV_present_video (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindVideoDeviceNV = (PFNWGLBINDVIDEODEVICENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglBindVideoDeviceNV"))) == NULL) || r;
  r = ((wglEnumerateVideoDevicesNV = (PFNWGLENUMERATEVIDEODEVICESNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglEnumerateVideoDevicesNV"))) == NULL) || r;
  r = ((wglQueryCurrentContextNV = (PFNWGLQUERYCURRENTCONTEXTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQueryCurrentContextNV"))) == NULL) || r;

  return r;
}

#endif /* WGL_NV_present_video */

#ifdef WGL_NV_render_depth_texture

#endif /* WGL_NV_render_depth_texture */

#ifdef WGL_NV_render_texture_rectangle

#endif /* WGL_NV_render_texture_rectangle */

#ifdef WGL_NV_swap_group

static GLboolean _glewInit_WGL_NV_swap_group (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindSwapBarrierNV = (PFNWGLBINDSWAPBARRIERNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglBindSwapBarrierNV"))) == NULL) || r;
  r = ((wglJoinSwapGroupNV = (PFNWGLJOINSWAPGROUPNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglJoinSwapGroupNV"))) == NULL) || r;
  r = ((wglQueryFrameCountNV = (PFNWGLQUERYFRAMECOUNTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQueryFrameCountNV"))) == NULL) || r;
  r = ((wglQueryMaxSwapGroupsNV = (PFNWGLQUERYMAXSWAPGROUPSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQueryMaxSwapGroupsNV"))) == NULL) || r;
  r = ((wglQuerySwapGroupNV = (PFNWGLQUERYSWAPGROUPNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglQuerySwapGroupNV"))) == NULL) || r;
  r = ((wglResetFrameCountNV = (PFNWGLRESETFRAMECOUNTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglResetFrameCountNV"))) == NULL) || r;

  return r;
}

#endif /* WGL_NV_swap_group */

#ifdef WGL_NV_vertex_array_range

static GLboolean _glewInit_WGL_NV_vertex_array_range (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglAllocateMemoryNV = (PFNWGLALLOCATEMEMORYNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglAllocateMemoryNV"))) == NULL) || r;
  r = ((wglFreeMemoryNV = (PFNWGLFREEMEMORYNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglFreeMemoryNV"))) == NULL) || r;

  return r;
}

#endif /* WGL_NV_vertex_array_range */

#ifdef WGL_NV_video_output

static GLboolean _glewInit_WGL_NV_video_output (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindVideoImageNV = (PFNWGLBINDVIDEOIMAGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglBindVideoImageNV"))) == NULL) || r;
  r = ((wglGetVideoDeviceNV = (PFNWGLGETVIDEODEVICENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetVideoDeviceNV"))) == NULL) || r;
  r = ((wglGetVideoInfoNV = (PFNWGLGETVIDEOINFONVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetVideoInfoNV"))) == NULL) || r;
  r = ((wglReleaseVideoDeviceNV = (PFNWGLRELEASEVIDEODEVICENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglReleaseVideoDeviceNV"))) == NULL) || r;
  r = ((wglReleaseVideoImageNV = (PFNWGLRELEASEVIDEOIMAGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglReleaseVideoImageNV"))) == NULL) || r;
  r = ((wglSendPbufferToVideoNV = (PFNWGLSENDPBUFFERTOVIDEONVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSendPbufferToVideoNV"))) == NULL) || r;

  return r;
}

#endif /* WGL_NV_video_output */

#ifdef WGL_OML_sync_control

static GLboolean _glewInit_WGL_OML_sync_control (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetMscRateOML = (PFNWGLGETMSCRATEOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetMscRateOML"))) == NULL) || r;
  r = ((wglGetSyncValuesOML = (PFNWGLGETSYNCVALUESOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglGetSyncValuesOML"))) == NULL) || r;
  r = ((wglSwapBuffersMscOML = (PFNWGLSWAPBUFFERSMSCOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSwapBuffersMscOML"))) == NULL) || r;
  r = ((wglSwapLayerBuffersMscOML = (PFNWGLSWAPLAYERBUFFERSMSCOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglSwapLayerBuffersMscOML"))) == NULL) || r;
  r = ((wglWaitForMscOML = (PFNWGLWAITFORMSCOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglWaitForMscOML"))) == NULL) || r;
  r = ((wglWaitForSbcOML = (PFNWGLWAITFORSBCOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "wglWaitForSbcOML"))) == NULL) || r;

  return r;
}

#endif /* WGL_OML_sync_control */

/* ------------------------------------------------------------------------- */

static PFNWGLGETEXTENSIONSSTRINGARBPROC _wglewGetExtensionsStringARB = NULL;
static PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglewGetExtensionsStringEXT = NULL;

GLboolean wglewGetExtension (const char* name)
{    
  GLubyte* p;
  GLubyte* end;
  GLuint len = _glewStrLen((const GLubyte*)name);
  if (_wglewGetExtensionsStringARB == NULL)
    if (_wglewGetExtensionsStringEXT == NULL)
      return GL_FALSE;
    else
      p = (GLubyte*)_wglewGetExtensionsStringEXT();
  else
    p = (GLubyte*)_wglewGetExtensionsStringARB(wglGetCurrentDC());
  if (0 == p) return GL_FALSE;
  end = p + _glewStrLen(p);
  while (p < end)
  {
    GLuint n = _glewStrCLen(p, ' ');
    if (len == n && _glewStrSame((const GLubyte*)name, p, n)) return GL_TRUE;
    p += n+1;
  }
  return GL_FALSE;
}

GLenum wglewContextInit (WGLEW_CONTEXT_ARG_DEF_LIST)
{
  GLboolean crippled;
  /* find wgl extension string query functions */
  _wglewGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringARB");
  _wglewGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringEXT");
  /* initialize extensions */
  crippled = _wglewGetExtensionsStringARB == NULL && _wglewGetExtensionsStringEXT == NULL;
#ifdef WGL_3DFX_multisample
  CONST_CAST(WGLEW_3DFX_multisample) = wglewGetExtension("WGL_3DFX_multisample");
#endif /* WGL_3DFX_multisample */
#ifdef WGL_3DL_stereo_control
  CONST_CAST(WGLEW_3DL_stereo_control) = wglewGetExtension("WGL_3DL_stereo_control");
  if (glewExperimental || WGLEW_3DL_stereo_control|| crippled) CONST_CAST(WGLEW_3DL_stereo_control)= !_glewInit_WGL_3DL_stereo_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_3DL_stereo_control */
#ifdef WGL_ARB_buffer_region
  CONST_CAST(WGLEW_ARB_buffer_region) = wglewGetExtension("WGL_ARB_buffer_region");
  if (glewExperimental || WGLEW_ARB_buffer_region|| crippled) CONST_CAST(WGLEW_ARB_buffer_region)= !_glewInit_WGL_ARB_buffer_region(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_buffer_region */
#ifdef WGL_ARB_create_context
  CONST_CAST(WGLEW_ARB_create_context) = wglewGetExtension("WGL_ARB_create_context");
  if (glewExperimental || WGLEW_ARB_create_context|| crippled) CONST_CAST(WGLEW_ARB_create_context)= !_glewInit_WGL_ARB_create_context(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_create_context */
#ifdef WGL_ARB_extensions_string
  CONST_CAST(WGLEW_ARB_extensions_string) = wglewGetExtension("WGL_ARB_extensions_string");
  if (glewExperimental || WGLEW_ARB_extensions_string|| crippled) CONST_CAST(WGLEW_ARB_extensions_string)= !_glewInit_WGL_ARB_extensions_string(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_extensions_string */
#ifdef WGL_ARB_framebuffer_sRGB
  CONST_CAST(WGLEW_ARB_framebuffer_sRGB) = wglewGetExtension("WGL_ARB_framebuffer_sRGB");
#endif /* WGL_ARB_framebuffer_sRGB */
#ifdef WGL_ARB_make_current_read
  CONST_CAST(WGLEW_ARB_make_current_read) = wglewGetExtension("WGL_ARB_make_current_read");
  if (glewExperimental || WGLEW_ARB_make_current_read|| crippled) CONST_CAST(WGLEW_ARB_make_current_read)= !_glewInit_WGL_ARB_make_current_read(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_make_current_read */
#ifdef WGL_ARB_multisample
  CONST_CAST(WGLEW_ARB_multisample) = wglewGetExtension("WGL_ARB_multisample");
#endif /* WGL_ARB_multisample */
#ifdef WGL_ARB_pbuffer
  CONST_CAST(WGLEW_ARB_pbuffer) = wglewGetExtension("WGL_ARB_pbuffer");
  if (glewExperimental || WGLEW_ARB_pbuffer|| crippled) CONST_CAST(WGLEW_ARB_pbuffer)= !_glewInit_WGL_ARB_pbuffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_pbuffer */
#ifdef WGL_ARB_pixel_format
  CONST_CAST(WGLEW_ARB_pixel_format) = wglewGetExtension("WGL_ARB_pixel_format");
  if (glewExperimental || WGLEW_ARB_pixel_format|| crippled) CONST_CAST(WGLEW_ARB_pixel_format)= !_glewInit_WGL_ARB_pixel_format(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_pixel_format */
#ifdef WGL_ARB_pixel_format_float
  CONST_CAST(WGLEW_ARB_pixel_format_float) = wglewGetExtension("WGL_ARB_pixel_format_float");
#endif /* WGL_ARB_pixel_format_float */
#ifdef WGL_ARB_render_texture
  CONST_CAST(WGLEW_ARB_render_texture) = wglewGetExtension("WGL_ARB_render_texture");
  if (glewExperimental || WGLEW_ARB_render_texture|| crippled) CONST_CAST(WGLEW_ARB_render_texture)= !_glewInit_WGL_ARB_render_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_render_texture */
#ifdef WGL_ATI_pixel_format_float
  CONST_CAST(WGLEW_ATI_pixel_format_float) = wglewGetExtension("WGL_ATI_pixel_format_float");
#endif /* WGL_ATI_pixel_format_float */
#ifdef WGL_ATI_render_texture_rectangle
  CONST_CAST(WGLEW_ATI_render_texture_rectangle) = wglewGetExtension("WGL_ATI_render_texture_rectangle");
#endif /* WGL_ATI_render_texture_rectangle */
#ifdef WGL_EXT_depth_float
  CONST_CAST(WGLEW_EXT_depth_float) = wglewGetExtension("WGL_EXT_depth_float");
#endif /* WGL_EXT_depth_float */
#ifdef WGL_EXT_display_color_table
  CONST_CAST(WGLEW_EXT_display_color_table) = wglewGetExtension("WGL_EXT_display_color_table");
  if (glewExperimental || WGLEW_EXT_display_color_table|| crippled) CONST_CAST(WGLEW_EXT_display_color_table)= !_glewInit_WGL_EXT_display_color_table(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_display_color_table */
#ifdef WGL_EXT_extensions_string
  CONST_CAST(WGLEW_EXT_extensions_string) = wglewGetExtension("WGL_EXT_extensions_string");
  if (glewExperimental || WGLEW_EXT_extensions_string|| crippled) CONST_CAST(WGLEW_EXT_extensions_string)= !_glewInit_WGL_EXT_extensions_string(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_extensions_string */
#ifdef WGL_EXT_framebuffer_sRGB
  CONST_CAST(WGLEW_EXT_framebuffer_sRGB) = wglewGetExtension("WGL_EXT_framebuffer_sRGB");
#endif /* WGL_EXT_framebuffer_sRGB */
#ifdef WGL_EXT_make_current_read
  CONST_CAST(WGLEW_EXT_make_current_read) = wglewGetExtension("WGL_EXT_make_current_read");
  if (glewExperimental || WGLEW_EXT_make_current_read|| crippled) CONST_CAST(WGLEW_EXT_make_current_read)= !_glewInit_WGL_EXT_make_current_read(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_make_current_read */
#ifdef WGL_EXT_multisample
  CONST_CAST(WGLEW_EXT_multisample) = wglewGetExtension("WGL_EXT_multisample");
#endif /* WGL_EXT_multisample */
#ifdef WGL_EXT_pbuffer
  CONST_CAST(WGLEW_EXT_pbuffer) = wglewGetExtension("WGL_EXT_pbuffer");
  if (glewExperimental || WGLEW_EXT_pbuffer|| crippled) CONST_CAST(WGLEW_EXT_pbuffer)= !_glewInit_WGL_EXT_pbuffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_pbuffer */
#ifdef WGL_EXT_pixel_format
  CONST_CAST(WGLEW_EXT_pixel_format) = wglewGetExtension("WGL_EXT_pixel_format");
  if (glewExperimental || WGLEW_EXT_pixel_format|| crippled) CONST_CAST(WGLEW_EXT_pixel_format)= !_glewInit_WGL_EXT_pixel_format(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_pixel_format */
#ifdef WGL_EXT_pixel_format_packed_float
  CONST_CAST(WGLEW_EXT_pixel_format_packed_float) = wglewGetExtension("WGL_EXT_pixel_format_packed_float");
#endif /* WGL_EXT_pixel_format_packed_float */
#ifdef WGL_EXT_swap_control
  CONST_CAST(WGLEW_EXT_swap_control) = wglewGetExtension("WGL_EXT_swap_control");
  if (glewExperimental || WGLEW_EXT_swap_control|| crippled) CONST_CAST(WGLEW_EXT_swap_control)= !_glewInit_WGL_EXT_swap_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_swap_control */
#ifdef WGL_I3D_digital_video_control
  CONST_CAST(WGLEW_I3D_digital_video_control) = wglewGetExtension("WGL_I3D_digital_video_control");
  if (glewExperimental || WGLEW_I3D_digital_video_control|| crippled) CONST_CAST(WGLEW_I3D_digital_video_control)= !_glewInit_WGL_I3D_digital_video_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_digital_video_control */
#ifdef WGL_I3D_gamma
  CONST_CAST(WGLEW_I3D_gamma) = wglewGetExtension("WGL_I3D_gamma");
  if (glewExperimental || WGLEW_I3D_gamma|| crippled) CONST_CAST(WGLEW_I3D_gamma)= !_glewInit_WGL_I3D_gamma(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_gamma */
#ifdef WGL_I3D_genlock
  CONST_CAST(WGLEW_I3D_genlock) = wglewGetExtension("WGL_I3D_genlock");
  if (glewExperimental || WGLEW_I3D_genlock|| crippled) CONST_CAST(WGLEW_I3D_genlock)= !_glewInit_WGL_I3D_genlock(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_genlock */
#ifdef WGL_I3D_image_buffer
  CONST_CAST(WGLEW_I3D_image_buffer) = wglewGetExtension("WGL_I3D_image_buffer");
  if (glewExperimental || WGLEW_I3D_image_buffer|| crippled) CONST_CAST(WGLEW_I3D_image_buffer)= !_glewInit_WGL_I3D_image_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_image_buffer */
#ifdef WGL_I3D_swap_frame_lock
  CONST_CAST(WGLEW_I3D_swap_frame_lock) = wglewGetExtension("WGL_I3D_swap_frame_lock");
  if (glewExperimental || WGLEW_I3D_swap_frame_lock|| crippled) CONST_CAST(WGLEW_I3D_swap_frame_lock)= !_glewInit_WGL_I3D_swap_frame_lock(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_swap_frame_lock */
#ifdef WGL_I3D_swap_frame_usage
  CONST_CAST(WGLEW_I3D_swap_frame_usage) = wglewGetExtension("WGL_I3D_swap_frame_usage");
  if (glewExperimental || WGLEW_I3D_swap_frame_usage|| crippled) CONST_CAST(WGLEW_I3D_swap_frame_usage)= !_glewInit_WGL_I3D_swap_frame_usage(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_swap_frame_usage */
#ifdef WGL_NV_float_buffer
  CONST_CAST(WGLEW_NV_float_buffer) = wglewGetExtension("WGL_NV_float_buffer");
#endif /* WGL_NV_float_buffer */
#ifdef WGL_NV_gpu_affinity
  CONST_CAST(WGLEW_NV_gpu_affinity) = wglewGetExtension("WGL_NV_gpu_affinity");
  if (glewExperimental || WGLEW_NV_gpu_affinity|| crippled) CONST_CAST(WGLEW_NV_gpu_affinity)= !_glewInit_WGL_NV_gpu_affinity(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_gpu_affinity */
#ifdef WGL_NV_present_video
  CONST_CAST(WGLEW_NV_present_video) = wglewGetExtension("WGL_NV_present_video");
  if (glewExperimental || WGLEW_NV_present_video|| crippled) CONST_CAST(WGLEW_NV_present_video)= !_glewInit_WGL_NV_present_video(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_present_video */
#ifdef WGL_NV_render_depth_texture
  CONST_CAST(WGLEW_NV_render_depth_texture) = wglewGetExtension("WGL_NV_render_depth_texture");
#endif /* WGL_NV_render_depth_texture */
#ifdef WGL_NV_render_texture_rectangle
  CONST_CAST(WGLEW_NV_render_texture_rectangle) = wglewGetExtension("WGL_NV_render_texture_rectangle");
#endif /* WGL_NV_render_texture_rectangle */
#ifdef WGL_NV_swap_group
  CONST_CAST(WGLEW_NV_swap_group) = wglewGetExtension("WGL_NV_swap_group");
  if (glewExperimental || WGLEW_NV_swap_group|| crippled) CONST_CAST(WGLEW_NV_swap_group)= !_glewInit_WGL_NV_swap_group(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_swap_group */
#ifdef WGL_NV_vertex_array_range
  CONST_CAST(WGLEW_NV_vertex_array_range) = wglewGetExtension("WGL_NV_vertex_array_range");
  if (glewExperimental || WGLEW_NV_vertex_array_range|| crippled) CONST_CAST(WGLEW_NV_vertex_array_range)= !_glewInit_WGL_NV_vertex_array_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_vertex_array_range */
#ifdef WGL_NV_video_output
  CONST_CAST(WGLEW_NV_video_output) = wglewGetExtension("WGL_NV_video_output");
  if (glewExperimental || WGLEW_NV_video_output|| crippled) CONST_CAST(WGLEW_NV_video_output)= !_glewInit_WGL_NV_video_output(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_video_output */
#ifdef WGL_OML_sync_control
  CONST_CAST(WGLEW_OML_sync_control) = wglewGetExtension("WGL_OML_sync_control");
  if (glewExperimental || WGLEW_OML_sync_control|| crippled) CONST_CAST(WGLEW_OML_sync_control)= !_glewInit_WGL_OML_sync_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_OML_sync_control */

  return GLEW_OK;
}

#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX)

PFNGLXCHOOSEVISUALPROC __glewXChooseVisual = NULL;
PFNGLXCOPYCONTEXTPROC __glewXCopyContext = NULL;
PFNGLXCREATECONTEXTPROC __glewXCreateContext = NULL;
PFNGLXCREATEGLXPIXMAPPROC __glewXCreateGLXPixmap = NULL;
PFNGLXDESTROYCONTEXTPROC __glewXDestroyContext = NULL;
PFNGLXDESTROYGLXPIXMAPPROC __glewXDestroyGLXPixmap = NULL;
PFNGLXGETCONFIGPROC __glewXGetConfig = NULL;
PFNGLXGETCURRENTCONTEXTPROC __glewXGetCurrentContext = NULL;
PFNGLXGETCURRENTDRAWABLEPROC __glewXGetCurrentDrawable = NULL;
PFNGLXISDIRECTPROC __glewXIsDirect = NULL;
PFNGLXMAKECURRENTPROC __glewXMakeCurrent = NULL;
PFNGLXQUERYEXTENSIONPROC __glewXQueryExtension = NULL;
PFNGLXQUERYVERSIONPROC __glewXQueryVersion = NULL;
PFNGLXSWAPBUFFERSPROC __glewXSwapBuffers = NULL;
PFNGLXUSEXFONTPROC __glewXUseXFont = NULL;
PFNGLXWAITGLPROC __glewXWaitGL = NULL;
PFNGLXWAITXPROC __glewXWaitX = NULL;

PFNGLXGETCLIENTSTRINGPROC __glewXGetClientString = NULL;
PFNGLXQUERYEXTENSIONSSTRINGPROC __glewXQueryExtensionsString = NULL;
PFNGLXQUERYSERVERSTRINGPROC __glewXQueryServerString = NULL;

PFNGLXGETCURRENTDISPLAYPROC __glewXGetCurrentDisplay = NULL;

PFNGLXCHOOSEFBCONFIGPROC __glewXChooseFBConfig = NULL;
PFNGLXCREATENEWCONTEXTPROC __glewXCreateNewContext = NULL;
PFNGLXCREATEPBUFFERPROC __glewXCreatePbuffer = NULL;
PFNGLXCREATEPIXMAPPROC __glewXCreatePixmap = NULL;
PFNGLXCREATEWINDOWPROC __glewXCreateWindow = NULL;
PFNGLXDESTROYPBUFFERPROC __glewXDestroyPbuffer = NULL;
PFNGLXDESTROYPIXMAPPROC __glewXDestroyPixmap = NULL;
PFNGLXDESTROYWINDOWPROC __glewXDestroyWindow = NULL;
PFNGLXGETCURRENTREADDRAWABLEPROC __glewXGetCurrentReadDrawable = NULL;
PFNGLXGETFBCONFIGATTRIBPROC __glewXGetFBConfigAttrib = NULL;
PFNGLXGETFBCONFIGSPROC __glewXGetFBConfigs = NULL;
PFNGLXGETSELECTEDEVENTPROC __glewXGetSelectedEvent = NULL;
PFNGLXGETVISUALFROMFBCONFIGPROC __glewXGetVisualFromFBConfig = NULL;
PFNGLXMAKECONTEXTCURRENTPROC __glewXMakeContextCurrent = NULL;
PFNGLXQUERYCONTEXTPROC __glewXQueryContext = NULL;
PFNGLXQUERYDRAWABLEPROC __glewXQueryDrawable = NULL;
PFNGLXSELECTEVENTPROC __glewXSelectEvent = NULL;

PFNGLXCREATECONTEXTATTRIBSARBPROC __glewXCreateContextAttribsARB = NULL;

PFNGLXBINDTEXIMAGEATIPROC __glewXBindTexImageATI = NULL;
PFNGLXDRAWABLEATTRIBATIPROC __glewXDrawableAttribATI = NULL;
PFNGLXRELEASETEXIMAGEATIPROC __glewXReleaseTexImageATI = NULL;

PFNGLXFREECONTEXTEXTPROC __glewXFreeContextEXT = NULL;
PFNGLXGETCONTEXTIDEXTPROC __glewXGetContextIDEXT = NULL;
PFNGLXIMPORTCONTEXTEXTPROC __glewXImportContextEXT = NULL;
PFNGLXQUERYCONTEXTINFOEXTPROC __glewXQueryContextInfoEXT = NULL;

PFNGLXBINDTEXIMAGEEXTPROC __glewXBindTexImageEXT = NULL;
PFNGLXRELEASETEXIMAGEEXTPROC __glewXReleaseTexImageEXT = NULL;

PFNGLXGETAGPOFFSETMESAPROC __glewXGetAGPOffsetMESA = NULL;

PFNGLXCOPYSUBBUFFERMESAPROC __glewXCopySubBufferMESA = NULL;

PFNGLXCREATEGLXPIXMAPMESAPROC __glewXCreateGLXPixmapMESA = NULL;

PFNGLXRELEASEBUFFERSMESAPROC __glewXReleaseBuffersMESA = NULL;

PFNGLXSET3DFXMODEMESAPROC __glewXSet3DfxModeMESA = NULL;

PFNGLXBINDVIDEODEVICENVPROC __glewXBindVideoDeviceNV = NULL;
PFNGLXENUMERATEVIDEODEVICESNVPROC __glewXEnumerateVideoDevicesNV = NULL;

PFNGLXBINDSWAPBARRIERNVPROC __glewXBindSwapBarrierNV = NULL;
PFNGLXJOINSWAPGROUPNVPROC __glewXJoinSwapGroupNV = NULL;
PFNGLXQUERYFRAMECOUNTNVPROC __glewXQueryFrameCountNV = NULL;
PFNGLXQUERYMAXSWAPGROUPSNVPROC __glewXQueryMaxSwapGroupsNV = NULL;
PFNGLXQUERYSWAPGROUPNVPROC __glewXQuerySwapGroupNV = NULL;
PFNGLXRESETFRAMECOUNTNVPROC __glewXResetFrameCountNV = NULL;

PFNGLXALLOCATEMEMORYNVPROC __glewXAllocateMemoryNV = NULL;
PFNGLXFREEMEMORYNVPROC __glewXFreeMemoryNV = NULL;

PFNGLXBINDVIDEOIMAGENVPROC __glewXBindVideoImageNV = NULL;
PFNGLXGETVIDEODEVICENVPROC __glewXGetVideoDeviceNV = NULL;
PFNGLXGETVIDEOINFONVPROC __glewXGetVideoInfoNV = NULL;
PFNGLXRELEASEVIDEODEVICENVPROC __glewXReleaseVideoDeviceNV = NULL;
PFNGLXRELEASEVIDEOIMAGENVPROC __glewXReleaseVideoImageNV = NULL;
PFNGLXSENDPBUFFERTOVIDEONVPROC __glewXSendPbufferToVideoNV = NULL;

#ifdef GLX_OML_sync_control
PFNGLXGETMSCRATEOMLPROC __glewXGetMscRateOML = NULL;
PFNGLXGETSYNCVALUESOMLPROC __glewXGetSyncValuesOML = NULL;
PFNGLXSWAPBUFFERSMSCOMLPROC __glewXSwapBuffersMscOML = NULL;
PFNGLXWAITFORMSCOMLPROC __glewXWaitForMscOML = NULL;
PFNGLXWAITFORSBCOMLPROC __glewXWaitForSbcOML = NULL;
#endif

PFNGLXCHOOSEFBCONFIGSGIXPROC __glewXChooseFBConfigSGIX = NULL;
PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC __glewXCreateContextWithConfigSGIX = NULL;
PFNGLXCREATEGLXPIXMAPWITHCONFIGSGIXPROC __glewXCreateGLXPixmapWithConfigSGIX = NULL;
PFNGLXGETFBCONFIGATTRIBSGIXPROC __glewXGetFBConfigAttribSGIX = NULL;
PFNGLXGETFBCONFIGFROMVISUALSGIXPROC __glewXGetFBConfigFromVisualSGIX = NULL;
PFNGLXGETVISUALFROMFBCONFIGSGIXPROC __glewXGetVisualFromFBConfigSGIX = NULL;

PFNGLXBINDHYPERPIPESGIXPROC __glewXBindHyperpipeSGIX = NULL;
PFNGLXDESTROYHYPERPIPECONFIGSGIXPROC __glewXDestroyHyperpipeConfigSGIX = NULL;
PFNGLXHYPERPIPEATTRIBSGIXPROC __glewXHyperpipeAttribSGIX = NULL;
PFNGLXHYPERPIPECONFIGSGIXPROC __glewXHyperpipeConfigSGIX = NULL;
PFNGLXQUERYHYPERPIPEATTRIBSGIXPROC __glewXQueryHyperpipeAttribSGIX = NULL;
PFNGLXQUERYHYPERPIPEBESTATTRIBSGIXPROC __glewXQueryHyperpipeBestAttribSGIX = NULL;
PFNGLXQUERYHYPERPIPECONFIGSGIXPROC __glewXQueryHyperpipeConfigSGIX = NULL;
PFNGLXQUERYHYPERPIPENETWORKSGIXPROC __glewXQueryHyperpipeNetworkSGIX = NULL;

PFNGLXCREATEGLXPBUFFERSGIXPROC __glewXCreateGLXPbufferSGIX = NULL;
PFNGLXDESTROYGLXPBUFFERSGIXPROC __glewXDestroyGLXPbufferSGIX = NULL;
PFNGLXGETSELECTEDEVENTSGIXPROC __glewXGetSelectedEventSGIX = NULL;
PFNGLXQUERYGLXPBUFFERSGIXPROC __glewXQueryGLXPbufferSGIX = NULL;
PFNGLXSELECTEVENTSGIXPROC __glewXSelectEventSGIX = NULL;

PFNGLXBINDSWAPBARRIERSGIXPROC __glewXBindSwapBarrierSGIX = NULL;
PFNGLXQUERYMAXSWAPBARRIERSSGIXPROC __glewXQueryMaxSwapBarriersSGIX = NULL;

PFNGLXJOINSWAPGROUPSGIXPROC __glewXJoinSwapGroupSGIX = NULL;

PFNGLXBINDCHANNELTOWINDOWSGIXPROC __glewXBindChannelToWindowSGIX = NULL;
PFNGLXCHANNELRECTSGIXPROC __glewXChannelRectSGIX = NULL;
PFNGLXCHANNELRECTSYNCSGIXPROC __glewXChannelRectSyncSGIX = NULL;
PFNGLXQUERYCHANNELDELTASSGIXPROC __glewXQueryChannelDeltasSGIX = NULL;
PFNGLXQUERYCHANNELRECTSGIXPROC __glewXQueryChannelRectSGIX = NULL;

PFNGLXCUSHIONSGIPROC __glewXCushionSGI = NULL;

PFNGLXGETCURRENTREADDRAWABLESGIPROC __glewXGetCurrentReadDrawableSGI = NULL;
PFNGLXMAKECURRENTREADSGIPROC __glewXMakeCurrentReadSGI = NULL;

PFNGLXSWAPINTERVALSGIPROC __glewXSwapIntervalSGI = NULL;

PFNGLXGETVIDEOSYNCSGIPROC __glewXGetVideoSyncSGI = NULL;
PFNGLXWAITVIDEOSYNCSGIPROC __glewXWaitVideoSyncSGI = NULL;

PFNGLXGETTRANSPARENTINDEXSUNPROC __glewXGetTransparentIndexSUN = NULL;

PFNGLXGETVIDEORESIZESUNPROC __glewXGetVideoResizeSUN = NULL;
PFNGLXVIDEORESIZESUNPROC __glewXVideoResizeSUN = NULL;

#if !defined(GLEW_MX)
GLboolean __GLXEW_VERSION_1_0 = GL_FALSE;
GLboolean __GLXEW_VERSION_1_1 = GL_FALSE;
GLboolean __GLXEW_VERSION_1_2 = GL_FALSE;
GLboolean __GLXEW_VERSION_1_3 = GL_FALSE;
GLboolean __GLXEW_VERSION_1_4 = GL_FALSE;
GLboolean __GLXEW_3DFX_multisample = GL_FALSE;
GLboolean __GLXEW_ARB_create_context = GL_FALSE;
GLboolean __GLXEW_ARB_fbconfig_float = GL_FALSE;
GLboolean __GLXEW_ARB_framebuffer_sRGB = GL_FALSE;
GLboolean __GLXEW_ARB_get_proc_address = GL_FALSE;
GLboolean __GLXEW_ARB_multisample = GL_FALSE;
GLboolean __GLXEW_ATI_pixel_format_float = GL_FALSE;
GLboolean __GLXEW_ATI_render_texture = GL_FALSE;
GLboolean __GLXEW_EXT_fbconfig_packed_float = GL_FALSE;
GLboolean __GLXEW_EXT_framebuffer_sRGB = GL_FALSE;
GLboolean __GLXEW_EXT_import_context = GL_FALSE;
GLboolean __GLXEW_EXT_scene_marker = GL_FALSE;
GLboolean __GLXEW_EXT_texture_from_pixmap = GL_FALSE;
GLboolean __GLXEW_EXT_visual_info = GL_FALSE;
GLboolean __GLXEW_EXT_visual_rating = GL_FALSE;
GLboolean __GLXEW_MESA_agp_offset = GL_FALSE;
GLboolean __GLXEW_MESA_copy_sub_buffer = GL_FALSE;
GLboolean __GLXEW_MESA_pixmap_colormap = GL_FALSE;
GLboolean __GLXEW_MESA_release_buffers = GL_FALSE;
GLboolean __GLXEW_MESA_set_3dfx_mode = GL_FALSE;
GLboolean __GLXEW_NV_float_buffer = GL_FALSE;
GLboolean __GLXEW_NV_present_video = GL_FALSE;
GLboolean __GLXEW_NV_swap_group = GL_FALSE;
GLboolean __GLXEW_NV_vertex_array_range = GL_FALSE;
GLboolean __GLXEW_NV_video_output = GL_FALSE;
GLboolean __GLXEW_OML_swap_method = GL_FALSE;
#ifdef GLX_OML_sync_control
GLboolean __GLXEW_OML_sync_control = GL_FALSE;
#endif
GLboolean __GLXEW_SGIS_blended_overlay = GL_FALSE;
GLboolean __GLXEW_SGIS_color_range = GL_FALSE;
GLboolean __GLXEW_SGIS_multisample = GL_FALSE;
GLboolean __GLXEW_SGIS_shared_multisample = GL_FALSE;
GLboolean __GLXEW_SGIX_fbconfig = GL_FALSE;
GLboolean __GLXEW_SGIX_hyperpipe = GL_FALSE;
GLboolean __GLXEW_SGIX_pbuffer = GL_FALSE;
GLboolean __GLXEW_SGIX_swap_barrier = GL_FALSE;
GLboolean __GLXEW_SGIX_swap_group = GL_FALSE;
GLboolean __GLXEW_SGIX_video_resize = GL_FALSE;
GLboolean __GLXEW_SGIX_visual_select_group = GL_FALSE;
GLboolean __GLXEW_SGI_cushion = GL_FALSE;
GLboolean __GLXEW_SGI_make_current_read = GL_FALSE;
GLboolean __GLXEW_SGI_swap_control = GL_FALSE;
GLboolean __GLXEW_SGI_video_sync = GL_FALSE;
GLboolean __GLXEW_SUN_get_transparent_index = GL_FALSE;
GLboolean __GLXEW_SUN_video_resize = GL_FALSE;

#endif /* !GLEW_MX */

#ifdef GLX_VERSION_1_0

static GLboolean _glewInit_GLX_VERSION_1_0 (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXChooseVisual = (PFNGLXCHOOSEVISUALPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXChooseVisual"))) == NULL) || r;
  r = ((glXCopyContext = (PFNGLXCOPYCONTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCopyContext"))) == NULL) || r;
  r = ((glXCreateContext = (PFNGLXCREATECONTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateContext"))) == NULL) || r;
  r = ((glXCreateGLXPixmap = (PFNGLXCREATEGLXPIXMAPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateGLXPixmap"))) == NULL) || r;
  r = ((glXDestroyContext = (PFNGLXDESTROYCONTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXDestroyContext"))) == NULL) || r;
  r = ((glXDestroyGLXPixmap = (PFNGLXDESTROYGLXPIXMAPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXDestroyGLXPixmap"))) == NULL) || r;
  r = ((glXGetConfig = (PFNGLXGETCONFIGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetConfig"))) == NULL) || r;
  r = ((glXGetCurrentContext = (PFNGLXGETCURRENTCONTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetCurrentContext"))) == NULL) || r;
  r = ((glXGetCurrentDrawable = (PFNGLXGETCURRENTDRAWABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetCurrentDrawable"))) == NULL) || r;
  r = ((glXIsDirect = (PFNGLXISDIRECTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXIsDirect"))) == NULL) || r;
  r = ((glXMakeCurrent = (PFNGLXMAKECURRENTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXMakeCurrent"))) == NULL) || r;
  r = ((glXQueryExtension = (PFNGLXQUERYEXTENSIONPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryExtension"))) == NULL) || r;
  r = ((glXQueryVersion = (PFNGLXQUERYVERSIONPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryVersion"))) == NULL) || r;
  r = ((glXSwapBuffers = (PFNGLXSWAPBUFFERSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXSwapBuffers"))) == NULL) || r;
  r = ((glXUseXFont = (PFNGLXUSEXFONTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXUseXFont"))) == NULL) || r;
  r = ((glXWaitGL = (PFNGLXWAITGLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXWaitGL"))) == NULL) || r;
  r = ((glXWaitX = (PFNGLXWAITXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXWaitX"))) == NULL) || r;

  return r;
}

#endif /* GLX_VERSION_1_0 */

#ifdef GLX_VERSION_1_1

static GLboolean _glewInit_GLX_VERSION_1_1 (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetClientString = (PFNGLXGETCLIENTSTRINGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetClientString"))) == NULL) || r;
  r = ((glXQueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryExtensionsString"))) == NULL) || r;
  r = ((glXQueryServerString = (PFNGLXQUERYSERVERSTRINGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryServerString"))) == NULL) || r;

  return r;
}

#endif /* GLX_VERSION_1_1 */

#ifdef GLX_VERSION_1_2

static GLboolean _glewInit_GLX_VERSION_1_2 (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetCurrentDisplay = (PFNGLXGETCURRENTDISPLAYPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetCurrentDisplay"))) == NULL) || r;

  return r;
}

#endif /* GLX_VERSION_1_2 */

#ifdef GLX_VERSION_1_3

static GLboolean _glewInit_GLX_VERSION_1_3 (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXChooseFBConfig"))) == NULL) || r;
  r = ((glXCreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateNewContext"))) == NULL) || r;
  r = ((glXCreatePbuffer = (PFNGLXCREATEPBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreatePbuffer"))) == NULL) || r;
  r = ((glXCreatePixmap = (PFNGLXCREATEPIXMAPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreatePixmap"))) == NULL) || r;
  r = ((glXCreateWindow = (PFNGLXCREATEWINDOWPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateWindow"))) == NULL) || r;
  r = ((glXDestroyPbuffer = (PFNGLXDESTROYPBUFFERPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXDestroyPbuffer"))) == NULL) || r;
  r = ((glXDestroyPixmap = (PFNGLXDESTROYPIXMAPPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXDestroyPixmap"))) == NULL) || r;
  r = ((glXDestroyWindow = (PFNGLXDESTROYWINDOWPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXDestroyWindow"))) == NULL) || r;
  r = ((glXGetCurrentReadDrawable = (PFNGLXGETCURRENTREADDRAWABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetCurrentReadDrawable"))) == NULL) || r;
  r = ((glXGetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetFBConfigAttrib"))) == NULL) || r;
  r = ((glXGetFBConfigs = (PFNGLXGETFBCONFIGSPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetFBConfigs"))) == NULL) || r;
  r = ((glXGetSelectedEvent = (PFNGLXGETSELECTEDEVENTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetSelectedEvent"))) == NULL) || r;
  r = ((glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetVisualFromFBConfig"))) == NULL) || r;
  r = ((glXMakeContextCurrent = (PFNGLXMAKECONTEXTCURRENTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXMakeContextCurrent"))) == NULL) || r;
  r = ((glXQueryContext = (PFNGLXQUERYCONTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryContext"))) == NULL) || r;
  r = ((glXQueryDrawable = (PFNGLXQUERYDRAWABLEPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryDrawable"))) == NULL) || r;
  r = ((glXSelectEvent = (PFNGLXSELECTEVENTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXSelectEvent"))) == NULL) || r;

  return r;
}

#endif /* GLX_VERSION_1_3 */

#ifdef GLX_VERSION_1_4

#endif /* GLX_VERSION_1_4 */

#ifdef GLX_3DFX_multisample

#endif /* GLX_3DFX_multisample */

#ifdef GLX_ARB_create_context

static GLboolean _glewInit_GLX_ARB_create_context (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateContextAttribsARB"))) == NULL) || r;

  return r;
}

#endif /* GLX_ARB_create_context */

#ifdef GLX_ARB_fbconfig_float

#endif /* GLX_ARB_fbconfig_float */

#ifdef GLX_ARB_framebuffer_sRGB

#endif /* GLX_ARB_framebuffer_sRGB */

#ifdef GLX_ARB_get_proc_address

#endif /* GLX_ARB_get_proc_address */

#ifdef GLX_ARB_multisample

#endif /* GLX_ARB_multisample */

#ifdef GLX_ATI_pixel_format_float

#endif /* GLX_ATI_pixel_format_float */

#ifdef GLX_ATI_render_texture

static GLboolean _glewInit_GLX_ATI_render_texture (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindTexImageATI = (PFNGLXBINDTEXIMAGEATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXBindTexImageATI"))) == NULL) || r;
  r = ((glXDrawableAttribATI = (PFNGLXDRAWABLEATTRIBATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXDrawableAttribATI"))) == NULL) || r;
  r = ((glXReleaseTexImageATI = (PFNGLXRELEASETEXIMAGEATIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXReleaseTexImageATI"))) == NULL) || r;

  return r;
}

#endif /* GLX_ATI_render_texture */

#ifdef GLX_EXT_fbconfig_packed_float

#endif /* GLX_EXT_fbconfig_packed_float */

#ifdef GLX_EXT_framebuffer_sRGB

#endif /* GLX_EXT_framebuffer_sRGB */

#ifdef GLX_EXT_import_context

static GLboolean _glewInit_GLX_EXT_import_context (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXFreeContextEXT = (PFNGLXFREECONTEXTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXFreeContextEXT"))) == NULL) || r;
  r = ((glXGetContextIDEXT = (PFNGLXGETCONTEXTIDEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetContextIDEXT"))) == NULL) || r;
  r = ((glXImportContextEXT = (PFNGLXIMPORTCONTEXTEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXImportContextEXT"))) == NULL) || r;
  r = ((glXQueryContextInfoEXT = (PFNGLXQUERYCONTEXTINFOEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryContextInfoEXT"))) == NULL) || r;

  return r;
}

#endif /* GLX_EXT_import_context */

#ifdef GLX_EXT_scene_marker

#endif /* GLX_EXT_scene_marker */

#ifdef GLX_EXT_texture_from_pixmap

static GLboolean _glewInit_GLX_EXT_texture_from_pixmap (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindTexImageEXT = (PFNGLXBINDTEXIMAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXBindTexImageEXT"))) == NULL) || r;
  r = ((glXReleaseTexImageEXT = (PFNGLXRELEASETEXIMAGEEXTPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXReleaseTexImageEXT"))) == NULL) || r;

  return r;
}

#endif /* GLX_EXT_texture_from_pixmap */

#ifdef GLX_EXT_visual_info

#endif /* GLX_EXT_visual_info */

#ifdef GLX_EXT_visual_rating

#endif /* GLX_EXT_visual_rating */

#ifdef GLX_MESA_agp_offset

static GLboolean _glewInit_GLX_MESA_agp_offset (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetAGPOffsetMESA = (PFNGLXGETAGPOFFSETMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetAGPOffsetMESA"))) == NULL) || r;

  return r;
}

#endif /* GLX_MESA_agp_offset */

#ifdef GLX_MESA_copy_sub_buffer

static GLboolean _glewInit_GLX_MESA_copy_sub_buffer (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCopySubBufferMESA = (PFNGLXCOPYSUBBUFFERMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCopySubBufferMESA"))) == NULL) || r;

  return r;
}

#endif /* GLX_MESA_copy_sub_buffer */

#ifdef GLX_MESA_pixmap_colormap

static GLboolean _glewInit_GLX_MESA_pixmap_colormap (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCreateGLXPixmapMESA = (PFNGLXCREATEGLXPIXMAPMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateGLXPixmapMESA"))) == NULL) || r;

  return r;
}

#endif /* GLX_MESA_pixmap_colormap */

#ifdef GLX_MESA_release_buffers

static GLboolean _glewInit_GLX_MESA_release_buffers (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXReleaseBuffersMESA = (PFNGLXRELEASEBUFFERSMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXReleaseBuffersMESA"))) == NULL) || r;

  return r;
}

#endif /* GLX_MESA_release_buffers */

#ifdef GLX_MESA_set_3dfx_mode

static GLboolean _glewInit_GLX_MESA_set_3dfx_mode (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXSet3DfxModeMESA = (PFNGLXSET3DFXMODEMESAPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXSet3DfxModeMESA"))) == NULL) || r;

  return r;
}

#endif /* GLX_MESA_set_3dfx_mode */

#ifdef GLX_NV_float_buffer

#endif /* GLX_NV_float_buffer */

#ifdef GLX_NV_present_video

static GLboolean _glewInit_GLX_NV_present_video (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindVideoDeviceNV = (PFNGLXBINDVIDEODEVICENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXBindVideoDeviceNV"))) == NULL) || r;
  r = ((glXEnumerateVideoDevicesNV = (PFNGLXENUMERATEVIDEODEVICESNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXEnumerateVideoDevicesNV"))) == NULL) || r;

  return r;
}

#endif /* GLX_NV_present_video */

#ifdef GLX_NV_swap_group

static GLboolean _glewInit_GLX_NV_swap_group (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindSwapBarrierNV = (PFNGLXBINDSWAPBARRIERNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXBindSwapBarrierNV"))) == NULL) || r;
  r = ((glXJoinSwapGroupNV = (PFNGLXJOINSWAPGROUPNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXJoinSwapGroupNV"))) == NULL) || r;
  r = ((glXQueryFrameCountNV = (PFNGLXQUERYFRAMECOUNTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryFrameCountNV"))) == NULL) || r;
  r = ((glXQueryMaxSwapGroupsNV = (PFNGLXQUERYMAXSWAPGROUPSNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryMaxSwapGroupsNV"))) == NULL) || r;
  r = ((glXQuerySwapGroupNV = (PFNGLXQUERYSWAPGROUPNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQuerySwapGroupNV"))) == NULL) || r;
  r = ((glXResetFrameCountNV = (PFNGLXRESETFRAMECOUNTNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXResetFrameCountNV"))) == NULL) || r;

  return r;
}

#endif /* GLX_NV_swap_group */

#ifdef GLX_NV_vertex_array_range

static GLboolean _glewInit_GLX_NV_vertex_array_range (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXAllocateMemoryNV = (PFNGLXALLOCATEMEMORYNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXAllocateMemoryNV"))) == NULL) || r;
  r = ((glXFreeMemoryNV = (PFNGLXFREEMEMORYNVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXFreeMemoryNV"))) == NULL) || r;

  return r;
}

#endif /* GLX_NV_vertex_array_range */

#ifdef GLX_NV_video_output

static GLboolean _glewInit_GLX_NV_video_output (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindVideoImageNV = (PFNGLXBINDVIDEOIMAGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXBindVideoImageNV"))) == NULL) || r;
  r = ((glXGetVideoDeviceNV = (PFNGLXGETVIDEODEVICENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetVideoDeviceNV"))) == NULL) || r;
  r = ((glXGetVideoInfoNV = (PFNGLXGETVIDEOINFONVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetVideoInfoNV"))) == NULL) || r;
  r = ((glXReleaseVideoDeviceNV = (PFNGLXRELEASEVIDEODEVICENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXReleaseVideoDeviceNV"))) == NULL) || r;
  r = ((glXReleaseVideoImageNV = (PFNGLXRELEASEVIDEOIMAGENVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXReleaseVideoImageNV"))) == NULL) || r;
  r = ((glXSendPbufferToVideoNV = (PFNGLXSENDPBUFFERTOVIDEONVPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXSendPbufferToVideoNV"))) == NULL) || r;

  return r;
}

#endif /* GLX_NV_video_output */

#ifdef GLX_OML_swap_method

#endif /* GLX_OML_swap_method */

#if defined(GLX_OML_sync_control) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <inttypes.h>

static GLboolean _glewInit_GLX_OML_sync_control (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetMscRateOML = (PFNGLXGETMSCRATEOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetMscRateOML"))) == NULL) || r;
  r = ((glXGetSyncValuesOML = (PFNGLXGETSYNCVALUESOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetSyncValuesOML"))) == NULL) || r;
  r = ((glXSwapBuffersMscOML = (PFNGLXSWAPBUFFERSMSCOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXSwapBuffersMscOML"))) == NULL) || r;
  r = ((glXWaitForMscOML = (PFNGLXWAITFORMSCOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXWaitForMscOML"))) == NULL) || r;
  r = ((glXWaitForSbcOML = (PFNGLXWAITFORSBCOMLPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXWaitForSbcOML"))) == NULL) || r;

  return r;
}

#endif /* GLX_OML_sync_control */

#ifdef GLX_SGIS_blended_overlay

#endif /* GLX_SGIS_blended_overlay */

#ifdef GLX_SGIS_color_range

#endif /* GLX_SGIS_color_range */

#ifdef GLX_SGIS_multisample

#endif /* GLX_SGIS_multisample */

#ifdef GLX_SGIS_shared_multisample

#endif /* GLX_SGIS_shared_multisample */

#ifdef GLX_SGIX_fbconfig

static GLboolean _glewInit_GLX_SGIX_fbconfig (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXChooseFBConfigSGIX = (PFNGLXCHOOSEFBCONFIGSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXChooseFBConfigSGIX"))) == NULL) || r;
  r = ((glXCreateContextWithConfigSGIX = (PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateContextWithConfigSGIX"))) == NULL) || r;
  r = ((glXCreateGLXPixmapWithConfigSGIX = (PFNGLXCREATEGLXPIXMAPWITHCONFIGSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateGLXPixmapWithConfigSGIX"))) == NULL) || r;
  r = ((glXGetFBConfigAttribSGIX = (PFNGLXGETFBCONFIGATTRIBSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetFBConfigAttribSGIX"))) == NULL) || r;
  r = ((glXGetFBConfigFromVisualSGIX = (PFNGLXGETFBCONFIGFROMVISUALSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetFBConfigFromVisualSGIX"))) == NULL) || r;
  r = ((glXGetVisualFromFBConfigSGIX = (PFNGLXGETVISUALFROMFBCONFIGSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetVisualFromFBConfigSGIX"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGIX_fbconfig */

#ifdef GLX_SGIX_hyperpipe

static GLboolean _glewInit_GLX_SGIX_hyperpipe (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindHyperpipeSGIX = (PFNGLXBINDHYPERPIPESGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXBindHyperpipeSGIX"))) == NULL) || r;
  r = ((glXDestroyHyperpipeConfigSGIX = (PFNGLXDESTROYHYPERPIPECONFIGSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXDestroyHyperpipeConfigSGIX"))) == NULL) || r;
  r = ((glXHyperpipeAttribSGIX = (PFNGLXHYPERPIPEATTRIBSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXHyperpipeAttribSGIX"))) == NULL) || r;
  r = ((glXHyperpipeConfigSGIX = (PFNGLXHYPERPIPECONFIGSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXHyperpipeConfigSGIX"))) == NULL) || r;
  r = ((glXQueryHyperpipeAttribSGIX = (PFNGLXQUERYHYPERPIPEATTRIBSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryHyperpipeAttribSGIX"))) == NULL) || r;
  r = ((glXQueryHyperpipeBestAttribSGIX = (PFNGLXQUERYHYPERPIPEBESTATTRIBSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryHyperpipeBestAttribSGIX"))) == NULL) || r;
  r = ((glXQueryHyperpipeConfigSGIX = (PFNGLXQUERYHYPERPIPECONFIGSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryHyperpipeConfigSGIX"))) == NULL) || r;
  r = ((glXQueryHyperpipeNetworkSGIX = (PFNGLXQUERYHYPERPIPENETWORKSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryHyperpipeNetworkSGIX"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGIX_hyperpipe */

#ifdef GLX_SGIX_pbuffer

static GLboolean _glewInit_GLX_SGIX_pbuffer (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCreateGLXPbufferSGIX = (PFNGLXCREATEGLXPBUFFERSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCreateGLXPbufferSGIX"))) == NULL) || r;
  r = ((glXDestroyGLXPbufferSGIX = (PFNGLXDESTROYGLXPBUFFERSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXDestroyGLXPbufferSGIX"))) == NULL) || r;
  r = ((glXGetSelectedEventSGIX = (PFNGLXGETSELECTEDEVENTSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetSelectedEventSGIX"))) == NULL) || r;
  r = ((glXQueryGLXPbufferSGIX = (PFNGLXQUERYGLXPBUFFERSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryGLXPbufferSGIX"))) == NULL) || r;
  r = ((glXSelectEventSGIX = (PFNGLXSELECTEVENTSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXSelectEventSGIX"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGIX_pbuffer */

#ifdef GLX_SGIX_swap_barrier

static GLboolean _glewInit_GLX_SGIX_swap_barrier (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindSwapBarrierSGIX = (PFNGLXBINDSWAPBARRIERSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXBindSwapBarrierSGIX"))) == NULL) || r;
  r = ((glXQueryMaxSwapBarriersSGIX = (PFNGLXQUERYMAXSWAPBARRIERSSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryMaxSwapBarriersSGIX"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGIX_swap_barrier */

#ifdef GLX_SGIX_swap_group

static GLboolean _glewInit_GLX_SGIX_swap_group (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXJoinSwapGroupSGIX = (PFNGLXJOINSWAPGROUPSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXJoinSwapGroupSGIX"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGIX_swap_group */

#ifdef GLX_SGIX_video_resize

static GLboolean _glewInit_GLX_SGIX_video_resize (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindChannelToWindowSGIX = (PFNGLXBINDCHANNELTOWINDOWSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXBindChannelToWindowSGIX"))) == NULL) || r;
  r = ((glXChannelRectSGIX = (PFNGLXCHANNELRECTSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXChannelRectSGIX"))) == NULL) || r;
  r = ((glXChannelRectSyncSGIX = (PFNGLXCHANNELRECTSYNCSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXChannelRectSyncSGIX"))) == NULL) || r;
  r = ((glXQueryChannelDeltasSGIX = (PFNGLXQUERYCHANNELDELTASSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryChannelDeltasSGIX"))) == NULL) || r;
  r = ((glXQueryChannelRectSGIX = (PFNGLXQUERYCHANNELRECTSGIXPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXQueryChannelRectSGIX"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGIX_video_resize */

#ifdef GLX_SGIX_visual_select_group

#endif /* GLX_SGIX_visual_select_group */

#ifdef GLX_SGI_cushion

static GLboolean _glewInit_GLX_SGI_cushion (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCushionSGI = (PFNGLXCUSHIONSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXCushionSGI"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGI_cushion */

#ifdef GLX_SGI_make_current_read

static GLboolean _glewInit_GLX_SGI_make_current_read (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetCurrentReadDrawableSGI = (PFNGLXGETCURRENTREADDRAWABLESGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetCurrentReadDrawableSGI"))) == NULL) || r;
  r = ((glXMakeCurrentReadSGI = (PFNGLXMAKECURRENTREADSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXMakeCurrentReadSGI"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGI_make_current_read */

#ifdef GLX_SGI_swap_control

static GLboolean _glewInit_GLX_SGI_swap_control (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXSwapIntervalSGI"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGI_swap_control */

#ifdef GLX_SGI_video_sync

static GLboolean _glewInit_GLX_SGI_video_sync (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetVideoSyncSGI = (PFNGLXGETVIDEOSYNCSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetVideoSyncSGI"))) == NULL) || r;
  r = ((glXWaitVideoSyncSGI = (PFNGLXWAITVIDEOSYNCSGIPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXWaitVideoSyncSGI"))) == NULL) || r;

  return r;
}

#endif /* GLX_SGI_video_sync */

#ifdef GLX_SUN_get_transparent_index

static GLboolean _glewInit_GLX_SUN_get_transparent_index (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetTransparentIndexSUN = (PFNGLXGETTRANSPARENTINDEXSUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetTransparentIndexSUN"))) == NULL) || r;

  return r;
}

#endif /* GLX_SUN_get_transparent_index */

#ifdef GLX_SUN_video_resize

static GLboolean _glewInit_GLX_SUN_video_resize (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetVideoResizeSUN = (PFNGLXGETVIDEORESIZESUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXGetVideoResizeSUN"))) == NULL) || r;
  r = ((glXVideoResizeSUN = (PFNGLXVIDEORESIZESUNPROC)glewGetProcAddress(fqn_from_convention(glew_convention, "glXVideoResizeSUN"))) == NULL) || r;

  return r;
}

#endif /* GLX_SUN_video_resize */

/* ------------------------------------------------------------------------ */

GLboolean glxewGetExtension (const char* name)
{    
  GLubyte* p;
  GLubyte* end;
  GLuint len;

  if (glXGetCurrentDisplay == NULL) return GL_FALSE;
  len = _glewStrLen((const GLubyte*)name);
  p = (GLubyte*)glXGetClientString(glXGetCurrentDisplay(), GLX_EXTENSIONS);
  if (0 == p) return GL_FALSE;
  end = p + _glewStrLen(p);
  while (p < end)
  {
    GLuint n = _glewStrCLen(p, ' ');
    if (len == n && _glewStrSame((const GLubyte*)name, p, n)) return GL_TRUE;
    p += n+1;
  }
  return GL_FALSE;
}

GLenum glxewContextInit (GLXEW_CONTEXT_ARG_DEF_LIST)
{
  int major, minor;

  /* Initialize GLX versions. */
  if(_glewInit_GLX_VERSION_1_0(GLEW_CONTEXT_ARG_VAR_INIT)) {
    return GLEW_ERROR_NO_GLX;
  }
  if (_glewInit_GLX_VERSION_1_1(GLEW_CONTEXT_ARG_VAR_INIT)) {
    return GLEW_ERROR_GLX_VERSION_10_ONLY;
  }
  if (_glewInit_GLX_VERSION_1_2(GLEW_CONTEXT_ARG_VAR_INIT)) {
    return GLEW_ERROR_GLX_VERSION_11_ONLY;
  }

  /* initialize flags */
  CONST_CAST(GLXEW_VERSION_1_0) = GL_TRUE;
  CONST_CAST(GLXEW_VERSION_1_1) = GL_TRUE;
  CONST_CAST(GLXEW_VERSION_1_2) = GL_TRUE;
  CONST_CAST(GLXEW_VERSION_1_3) = GL_TRUE;
  CONST_CAST(GLXEW_VERSION_1_4) = GL_TRUE;
  /* query GLX version */
  glXQueryVersion(glXGetCurrentDisplay(), &major, &minor);
  if (major == 1 && minor <= 3)
  {
    switch (minor)
    {
      case 3:
      CONST_CAST(GLXEW_VERSION_1_4) = GL_FALSE;
      break;
      case 2:
      CONST_CAST(GLXEW_VERSION_1_4) = GL_FALSE;
      CONST_CAST(GLXEW_VERSION_1_3) = GL_FALSE;
      break;
      default:
      return GLEW_ERROR_GLX_VERSION_11_ONLY;
      break;
    }
  }
  /* initialize extensions */
#ifdef GLX_VERSION_1_3
  if (glewExperimental || GLXEW_VERSION_1_3) CONST_CAST(GLXEW_VERSION_1_3) = !_glewInit_GLX_VERSION_1_3(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_VERSION_1_3 */
#ifdef GLX_3DFX_multisample
  CONST_CAST(GLXEW_3DFX_multisample) = glxewGetExtension("GLX_3DFX_multisample");
#endif /* GLX_3DFX_multisample */
#ifdef GLX_ARB_create_context
  CONST_CAST(GLXEW_ARB_create_context) = glxewGetExtension("GLX_ARB_create_context");
  if (glewExperimental || GLXEW_ARB_create_context) CONST_CAST(GLXEW_ARB_create_context) = !_glewInit_GLX_ARB_create_context(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_ARB_create_context */
#ifdef GLX_ARB_fbconfig_float
  CONST_CAST(GLXEW_ARB_fbconfig_float) = glxewGetExtension("GLX_ARB_fbconfig_float");
#endif /* GLX_ARB_fbconfig_float */
#ifdef GLX_ARB_framebuffer_sRGB
  CONST_CAST(GLXEW_ARB_framebuffer_sRGB) = glxewGetExtension("GLX_ARB_framebuffer_sRGB");
#endif /* GLX_ARB_framebuffer_sRGB */
#ifdef GLX_ARB_get_proc_address
  CONST_CAST(GLXEW_ARB_get_proc_address) = glxewGetExtension("GLX_ARB_get_proc_address");
#endif /* GLX_ARB_get_proc_address */
#ifdef GLX_ARB_multisample
  CONST_CAST(GLXEW_ARB_multisample) = glxewGetExtension("GLX_ARB_multisample");
#endif /* GLX_ARB_multisample */
#ifdef GLX_ATI_pixel_format_float
  CONST_CAST(GLXEW_ATI_pixel_format_float) = glxewGetExtension("GLX_ATI_pixel_format_float");
#endif /* GLX_ATI_pixel_format_float */
#ifdef GLX_ATI_render_texture
  CONST_CAST(GLXEW_ATI_render_texture) = glxewGetExtension("GLX_ATI_render_texture");
  if (glewExperimental || GLXEW_ATI_render_texture) CONST_CAST(GLXEW_ATI_render_texture) = !_glewInit_GLX_ATI_render_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_ATI_render_texture */
#ifdef GLX_EXT_fbconfig_packed_float
  CONST_CAST(GLXEW_EXT_fbconfig_packed_float) = glxewGetExtension("GLX_EXT_fbconfig_packed_float");
#endif /* GLX_EXT_fbconfig_packed_float */
#ifdef GLX_EXT_framebuffer_sRGB
  CONST_CAST(GLXEW_EXT_framebuffer_sRGB) = glxewGetExtension("GLX_EXT_framebuffer_sRGB");
#endif /* GLX_EXT_framebuffer_sRGB */
#ifdef GLX_EXT_import_context
  CONST_CAST(GLXEW_EXT_import_context) = glxewGetExtension("GLX_EXT_import_context");
  if (glewExperimental || GLXEW_EXT_import_context) CONST_CAST(GLXEW_EXT_import_context) = !_glewInit_GLX_EXT_import_context(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_EXT_import_context */
#ifdef GLX_EXT_scene_marker
  CONST_CAST(GLXEW_EXT_scene_marker) = glxewGetExtension("GLX_EXT_scene_marker");
#endif /* GLX_EXT_scene_marker */
#ifdef GLX_EXT_texture_from_pixmap
  CONST_CAST(GLXEW_EXT_texture_from_pixmap) = glxewGetExtension("GLX_EXT_texture_from_pixmap");
  if (glewExperimental || GLXEW_EXT_texture_from_pixmap) CONST_CAST(GLXEW_EXT_texture_from_pixmap) = !_glewInit_GLX_EXT_texture_from_pixmap(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_EXT_texture_from_pixmap */
#ifdef GLX_EXT_visual_info
  CONST_CAST(GLXEW_EXT_visual_info) = glxewGetExtension("GLX_EXT_visual_info");
#endif /* GLX_EXT_visual_info */
#ifdef GLX_EXT_visual_rating
  CONST_CAST(GLXEW_EXT_visual_rating) = glxewGetExtension("GLX_EXT_visual_rating");
#endif /* GLX_EXT_visual_rating */
#ifdef GLX_MESA_agp_offset
  CONST_CAST(GLXEW_MESA_agp_offset) = glxewGetExtension("GLX_MESA_agp_offset");
  if (glewExperimental || GLXEW_MESA_agp_offset) CONST_CAST(GLXEW_MESA_agp_offset) = !_glewInit_GLX_MESA_agp_offset(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_agp_offset */
#ifdef GLX_MESA_copy_sub_buffer
  CONST_CAST(GLXEW_MESA_copy_sub_buffer) = glxewGetExtension("GLX_MESA_copy_sub_buffer");
  if (glewExperimental || GLXEW_MESA_copy_sub_buffer) CONST_CAST(GLXEW_MESA_copy_sub_buffer) = !_glewInit_GLX_MESA_copy_sub_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_copy_sub_buffer */
#ifdef GLX_MESA_pixmap_colormap
  CONST_CAST(GLXEW_MESA_pixmap_colormap) = glxewGetExtension("GLX_MESA_pixmap_colormap");
  if (glewExperimental || GLXEW_MESA_pixmap_colormap) CONST_CAST(GLXEW_MESA_pixmap_colormap) = !_glewInit_GLX_MESA_pixmap_colormap(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_pixmap_colormap */
#ifdef GLX_MESA_release_buffers
  CONST_CAST(GLXEW_MESA_release_buffers) = glxewGetExtension("GLX_MESA_release_buffers");
  if (glewExperimental || GLXEW_MESA_release_buffers) CONST_CAST(GLXEW_MESA_release_buffers) = !_glewInit_GLX_MESA_release_buffers(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_release_buffers */
#ifdef GLX_MESA_set_3dfx_mode
  CONST_CAST(GLXEW_MESA_set_3dfx_mode) = glxewGetExtension("GLX_MESA_set_3dfx_mode");
  if (glewExperimental || GLXEW_MESA_set_3dfx_mode) CONST_CAST(GLXEW_MESA_set_3dfx_mode) = !_glewInit_GLX_MESA_set_3dfx_mode(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_set_3dfx_mode */
#ifdef GLX_NV_float_buffer
  CONST_CAST(GLXEW_NV_float_buffer) = glxewGetExtension("GLX_NV_float_buffer");
#endif /* GLX_NV_float_buffer */
#ifdef GLX_NV_present_video
  CONST_CAST(GLXEW_NV_present_video) = glxewGetExtension("GLX_NV_present_video");
  if (glewExperimental || GLXEW_NV_present_video) CONST_CAST(GLXEW_NV_present_video) = !_glewInit_GLX_NV_present_video(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_present_video */
#ifdef GLX_NV_swap_group
  CONST_CAST(GLXEW_NV_swap_group) = glxewGetExtension("GLX_NV_swap_group");
  if (glewExperimental || GLXEW_NV_swap_group) CONST_CAST(GLXEW_NV_swap_group) = !_glewInit_GLX_NV_swap_group(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_swap_group */
#ifdef GLX_NV_vertex_array_range
  CONST_CAST(GLXEW_NV_vertex_array_range) = glxewGetExtension("GLX_NV_vertex_array_range");
  if (glewExperimental || GLXEW_NV_vertex_array_range) CONST_CAST(GLXEW_NV_vertex_array_range) = !_glewInit_GLX_NV_vertex_array_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_vertex_array_range */
#ifdef GLX_NV_video_output
  CONST_CAST(GLXEW_NV_video_output) = glxewGetExtension("GLX_NV_video_output");
  if (glewExperimental || GLXEW_NV_video_output) CONST_CAST(GLXEW_NV_video_output) = !_glewInit_GLX_NV_video_output(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_video_output */
#ifdef GLX_OML_swap_method
  CONST_CAST(GLXEW_OML_swap_method) = glxewGetExtension("GLX_OML_swap_method");
#endif /* GLX_OML_swap_method */
#if defined(GLX_OML_sync_control) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <inttypes.h>
  CONST_CAST(GLXEW_OML_sync_control) = glxewGetExtension("GLX_OML_sync_control");
  if (glewExperimental || GLXEW_OML_sync_control) CONST_CAST(GLXEW_OML_sync_control) = !_glewInit_GLX_OML_sync_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_OML_sync_control */
#ifdef GLX_SGIS_blended_overlay
  CONST_CAST(GLXEW_SGIS_blended_overlay) = glxewGetExtension("GLX_SGIS_blended_overlay");
#endif /* GLX_SGIS_blended_overlay */
#ifdef GLX_SGIS_color_range
  CONST_CAST(GLXEW_SGIS_color_range) = glxewGetExtension("GLX_SGIS_color_range");
#endif /* GLX_SGIS_color_range */
#ifdef GLX_SGIS_multisample
  CONST_CAST(GLXEW_SGIS_multisample) = glxewGetExtension("GLX_SGIS_multisample");
#endif /* GLX_SGIS_multisample */
#ifdef GLX_SGIS_shared_multisample
  CONST_CAST(GLXEW_SGIS_shared_multisample) = glxewGetExtension("GLX_SGIS_shared_multisample");
#endif /* GLX_SGIS_shared_multisample */
#ifdef GLX_SGIX_fbconfig
  CONST_CAST(GLXEW_SGIX_fbconfig) = glxewGetExtension("GLX_SGIX_fbconfig");
  if (glewExperimental || GLXEW_SGIX_fbconfig) CONST_CAST(GLXEW_SGIX_fbconfig) = !_glewInit_GLX_SGIX_fbconfig(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_fbconfig */
#ifdef GLX_SGIX_hyperpipe
  CONST_CAST(GLXEW_SGIX_hyperpipe) = glxewGetExtension("GLX_SGIX_hyperpipe");
  if (glewExperimental || GLXEW_SGIX_hyperpipe) CONST_CAST(GLXEW_SGIX_hyperpipe) = !_glewInit_GLX_SGIX_hyperpipe(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_hyperpipe */
#ifdef GLX_SGIX_pbuffer
  CONST_CAST(GLXEW_SGIX_pbuffer) = glxewGetExtension("GLX_SGIX_pbuffer");
  if (glewExperimental || GLXEW_SGIX_pbuffer) CONST_CAST(GLXEW_SGIX_pbuffer) = !_glewInit_GLX_SGIX_pbuffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_pbuffer */
#ifdef GLX_SGIX_swap_barrier
  CONST_CAST(GLXEW_SGIX_swap_barrier) = glxewGetExtension("GLX_SGIX_swap_barrier");
  if (glewExperimental || GLXEW_SGIX_swap_barrier) CONST_CAST(GLXEW_SGIX_swap_barrier) = !_glewInit_GLX_SGIX_swap_barrier(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_swap_barrier */
#ifdef GLX_SGIX_swap_group
  CONST_CAST(GLXEW_SGIX_swap_group) = glxewGetExtension("GLX_SGIX_swap_group");
  if (glewExperimental || GLXEW_SGIX_swap_group) CONST_CAST(GLXEW_SGIX_swap_group) = !_glewInit_GLX_SGIX_swap_group(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_swap_group */
#ifdef GLX_SGIX_video_resize
  CONST_CAST(GLXEW_SGIX_video_resize) = glxewGetExtension("GLX_SGIX_video_resize");
  if (glewExperimental || GLXEW_SGIX_video_resize) CONST_CAST(GLXEW_SGIX_video_resize) = !_glewInit_GLX_SGIX_video_resize(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_video_resize */
#ifdef GLX_SGIX_visual_select_group
  CONST_CAST(GLXEW_SGIX_visual_select_group) = glxewGetExtension("GLX_SGIX_visual_select_group");
#endif /* GLX_SGIX_visual_select_group */
#ifdef GLX_SGI_cushion
  CONST_CAST(GLXEW_SGI_cushion) = glxewGetExtension("GLX_SGI_cushion");
  if (glewExperimental || GLXEW_SGI_cushion) CONST_CAST(GLXEW_SGI_cushion) = !_glewInit_GLX_SGI_cushion(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGI_cushion */
#ifdef GLX_SGI_make_current_read
  CONST_CAST(GLXEW_SGI_make_current_read) = glxewGetExtension("GLX_SGI_make_current_read");
  if (glewExperimental || GLXEW_SGI_make_current_read) CONST_CAST(GLXEW_SGI_make_current_read) = !_glewInit_GLX_SGI_make_current_read(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGI_make_current_read */
#ifdef GLX_SGI_swap_control
  CONST_CAST(GLXEW_SGI_swap_control) = glxewGetExtension("GLX_SGI_swap_control");
  if (glewExperimental || GLXEW_SGI_swap_control) CONST_CAST(GLXEW_SGI_swap_control) = !_glewInit_GLX_SGI_swap_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGI_swap_control */
#ifdef GLX_SGI_video_sync
  CONST_CAST(GLXEW_SGI_video_sync) = glxewGetExtension("GLX_SGI_video_sync");
  if (glewExperimental || GLXEW_SGI_video_sync) CONST_CAST(GLXEW_SGI_video_sync) = !_glewInit_GLX_SGI_video_sync(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGI_video_sync */
#ifdef GLX_SUN_get_transparent_index
  CONST_CAST(GLXEW_SUN_get_transparent_index) = glxewGetExtension("GLX_SUN_get_transparent_index");
  if (glewExperimental || GLXEW_SUN_get_transparent_index) CONST_CAST(GLXEW_SUN_get_transparent_index) = !_glewInit_GLX_SUN_get_transparent_index(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SUN_get_transparent_index */
#ifdef GLX_SUN_video_resize
  CONST_CAST(GLXEW_SUN_video_resize) = glxewGetExtension("GLX_SUN_video_resize");
  if (glewExperimental || GLXEW_SUN_video_resize) CONST_CAST(GLXEW_SUN_video_resize) = !_glewInit_GLX_SUN_video_resize(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SUN_video_resize */

  return GLEW_OK;
}

#endif /* !__APPLE__ || GLEW_APPLE_GLX */

/* ------------------------------------------------------------------------ */

const GLubyte* glewGetErrorString (GLenum e)
{
  static const GLubyte* _glewErrorString[] =
  {
    (const GLubyte*)"No error",
    (const GLubyte*)"Missing GL version",
    (const GLubyte*)"GL 1.1 and up are not supported",
    (const GLubyte*)"GLX 1.2 and up are not supported",
    (const GLubyte*)"Unknown error"
  };
  const int max_error = sizeof(_glewErrorString)/sizeof(*_glewErrorString) - 1;
  return _glewErrorString[(int)e > max_error ? max_error : (int)e];
}

const GLubyte* glewGetString (GLenum name)
{
  static const GLubyte* _glewString[] =
  {
    (const GLubyte*)NULL,
    (const GLubyte*)"1.5.1",
    (const GLubyte*)"1",
    (const GLubyte*)"5",
    (const GLubyte*)"1"
  };
  const int max_string = sizeof(_glewString)/sizeof(*_glewString) - 1;
  return _glewString[(int)name > max_string ? 0 : (int)name];
}

/* ------------------------------------------------------------------------ */

GLboolean glewExperimental = GL_FALSE;

#if !defined(GLEW_MX)

#if defined(_WIN32)
extern GLenum wglewContextInit (void);
#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX) /* _UNIX */
extern GLenum glxewContextInit (void);
#endif /* _WIN32 */

GLenum glewInitLibrary (const char *name, enum GL_Name_Convention conv)
{
  glew_gl_lib = name;
  glew_convention = conv;

  return glewInit();
}

GLenum glewInit ()
{
  GLenum r;

  _glewInitFunctionLoader();
  if(dso_handle == NULL) {
    return GLEW_ERROR_NO_GLX;
  }

  r = glewContextInit();
#if defined(_WIN32)
  return wglewContextInit();
#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX) /* _UNIX */
  /* Initialize GLX .  If we're using a `system' GL, then that's the only
   * init we can/want to do.  Otherwise, it's allowed to fail, since we can
   * load GL function pointers through other means. */
  if(glew_convention == GLEW_NAME_CONVENTION_GL) {
    GLenum ctx_init = glxewContextInit();
    return ctx_init;
  }
  return r;
#else
  return r;
#endif /* _WIN32 */
}

#endif /* !GLEW_MX */
#ifdef GLEW_MX
GLboolean glewContextIsSupported (GLEWContext* ctx, const char* name)
#else
GLboolean glewIsSupported (const char* name)
#endif
{
  GLubyte* pos = (GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"GL_", 3))
    {
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"VERSION_", 8))
      {
#ifdef GL_VERSION_1_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_1", 3))
        {
          ret = GLEW_VERSION_1_1;
          continue;
        }
#endif
#ifdef GL_VERSION_1_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_2", 3))
        {
          ret = GLEW_VERSION_1_2;
          continue;
        }
#endif
#ifdef GL_VERSION_1_3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_3", 3))
        {
          ret = GLEW_VERSION_1_3;
          continue;
        }
#endif
#ifdef GL_VERSION_1_4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_4", 3))
        {
          ret = GLEW_VERSION_1_4;
          continue;
        }
#endif
#ifdef GL_VERSION_1_5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_5", 3))
        {
          ret = GLEW_VERSION_1_5;
          continue;
        }
#endif
#ifdef GL_VERSION_2_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"2_0", 3))
        {
          ret = GLEW_VERSION_2_0;
          continue;
        }
#endif
#ifdef GL_VERSION_2_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"2_1", 3))
        {
          ret = GLEW_VERSION_2_1;
          continue;
        }
#endif
#ifdef GL_VERSION_3_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_0", 3))
        {
          ret = GLEW_VERSION_3_0;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"3DFX_", 5))
      {
#ifdef GL_3DFX_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLEW_3DFX_multisample;
          continue;
        }
#endif
#ifdef GL_3DFX_tbuffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"tbuffer", 7))
        {
          ret = GLEW_3DFX_tbuffer;
          continue;
        }
#endif
#ifdef GL_3DFX_texture_compression_FXT1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_FXT1", 24))
        {
          ret = GLEW_3DFX_texture_compression_FXT1;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"APPLE_", 6))
      {
#ifdef GL_APPLE_client_storage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"client_storage", 14))
        {
          ret = GLEW_APPLE_client_storage;
          continue;
        }
#endif
#ifdef GL_APPLE_element_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"element_array", 13))
        {
          ret = GLEW_APPLE_element_array;
          continue;
        }
#endif
#ifdef GL_APPLE_fence
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fence", 5))
        {
          ret = GLEW_APPLE_fence;
          continue;
        }
#endif
#ifdef GL_APPLE_float_pixels
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"float_pixels", 12))
        {
          ret = GLEW_APPLE_float_pixels;
          continue;
        }
#endif
#ifdef GL_APPLE_flush_buffer_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"flush_buffer_range", 18))
        {
          ret = GLEW_APPLE_flush_buffer_range;
          continue;
        }
#endif
#ifdef GL_APPLE_pixel_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_buffer", 12))
        {
          ret = GLEW_APPLE_pixel_buffer;
          continue;
        }
#endif
#ifdef GL_APPLE_specular_vector
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"specular_vector", 15))
        {
          ret = GLEW_APPLE_specular_vector;
          continue;
        }
#endif
#ifdef GL_APPLE_texture_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_range", 13))
        {
          ret = GLEW_APPLE_texture_range;
          continue;
        }
#endif
#ifdef GL_APPLE_transform_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_hint", 14))
        {
          ret = GLEW_APPLE_transform_hint;
          continue;
        }
#endif
#ifdef GL_APPLE_vertex_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_object", 19))
        {
          ret = GLEW_APPLE_vertex_array_object;
          continue;
        }
#endif
#ifdef GL_APPLE_vertex_array_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range", 18))
        {
          ret = GLEW_APPLE_vertex_array_range;
          continue;
        }
#endif
#ifdef GL_APPLE_ycbcr_422
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ycbcr_422", 9))
        {
          ret = GLEW_APPLE_ycbcr_422;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ARB_", 4))
      {
#ifdef GL_ARB_color_buffer_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_buffer_float", 18))
        {
          ret = GLEW_ARB_color_buffer_float;
          continue;
        }
#endif
#ifdef GL_ARB_depth_buffer_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_buffer_float", 18))
        {
          ret = GLEW_ARB_depth_buffer_float;
          continue;
        }
#endif
#ifdef GL_ARB_depth_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_texture", 13))
        {
          ret = GLEW_ARB_depth_texture;
          continue;
        }
#endif
#ifdef GL_ARB_draw_buffers
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_buffers", 12))
        {
          ret = GLEW_ARB_draw_buffers;
          continue;
        }
#endif
#ifdef GL_ARB_draw_instanced
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_instanced", 14))
        {
          ret = GLEW_ARB_draw_instanced;
          continue;
        }
#endif
#ifdef GL_ARB_fragment_program
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program", 16))
        {
          ret = GLEW_ARB_fragment_program;
          continue;
        }
#endif
#ifdef GL_ARB_fragment_program_shadow
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program_shadow", 23))
        {
          ret = GLEW_ARB_fragment_program_shadow;
          continue;
        }
#endif
#ifdef GL_ARB_fragment_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_shader", 15))
        {
          ret = GLEW_ARB_fragment_shader;
          continue;
        }
#endif
#ifdef GL_ARB_framebuffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_object", 18))
        {
          ret = GLEW_ARB_framebuffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLEW_ARB_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GL_ARB_geometry_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_shader4", 16))
        {
          ret = GLEW_ARB_geometry_shader4;
          continue;
        }
#endif
#ifdef GL_ARB_half_float_pixel
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"half_float_pixel", 16))
        {
          ret = GLEW_ARB_half_float_pixel;
          continue;
        }
#endif
#ifdef GL_ARB_half_float_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"half_float_vertex", 17))
        {
          ret = GLEW_ARB_half_float_vertex;
          continue;
        }
#endif
#ifdef GL_ARB_imaging
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"imaging", 7))
        {
          ret = GLEW_ARB_imaging;
          continue;
        }
#endif
#ifdef GL_ARB_instanced_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"instanced_arrays", 16))
        {
          ret = GLEW_ARB_instanced_arrays;
          continue;
        }
#endif
#ifdef GL_ARB_map_buffer_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"map_buffer_range", 16))
        {
          ret = GLEW_ARB_map_buffer_range;
          continue;
        }
#endif
#ifdef GL_ARB_matrix_palette
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"matrix_palette", 14))
        {
          ret = GLEW_ARB_matrix_palette;
          continue;
        }
#endif
#ifdef GL_ARB_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLEW_ARB_multisample;
          continue;
        }
#endif
#ifdef GL_ARB_multitexture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multitexture", 12))
        {
          ret = GLEW_ARB_multitexture;
          continue;
        }
#endif
#ifdef GL_ARB_occlusion_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"occlusion_query", 15))
        {
          ret = GLEW_ARB_occlusion_query;
          continue;
        }
#endif
#ifdef GL_ARB_pixel_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_buffer_object", 19))
        {
          ret = GLEW_ARB_pixel_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_point_parameters
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_parameters", 16))
        {
          ret = GLEW_ARB_point_parameters;
          continue;
        }
#endif
#ifdef GL_ARB_point_sprite
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_sprite", 12))
        {
          ret = GLEW_ARB_point_sprite;
          continue;
        }
#endif
#ifdef GL_ARB_shader_objects
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_objects", 14))
        {
          ret = GLEW_ARB_shader_objects;
          continue;
        }
#endif
#ifdef GL_ARB_shading_language_100
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shading_language_100", 20))
        {
          ret = GLEW_ARB_shading_language_100;
          continue;
        }
#endif
#ifdef GL_ARB_shadow
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow", 6))
        {
          ret = GLEW_ARB_shadow;
          continue;
        }
#endif
#ifdef GL_ARB_shadow_ambient
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow_ambient", 14))
        {
          ret = GLEW_ARB_shadow_ambient;
          continue;
        }
#endif
#ifdef GL_ARB_texture_border_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_border_clamp", 20))
        {
          ret = GLEW_ARB_texture_border_clamp;
          continue;
        }
#endif
#ifdef GL_ARB_texture_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_buffer_object", 21))
        {
          ret = GLEW_ARB_texture_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_texture_compression
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression", 19))
        {
          ret = GLEW_ARB_texture_compression;
          continue;
        }
#endif
#ifdef GL_ARB_texture_compression_rgtc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_rgtc", 24))
        {
          ret = GLEW_ARB_texture_compression_rgtc;
          continue;
        }
#endif
#ifdef GL_ARB_texture_cube_map
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_cube_map", 16))
        {
          ret = GLEW_ARB_texture_cube_map;
          continue;
        }
#endif
#ifdef GL_ARB_texture_env_add
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_add", 15))
        {
          ret = GLEW_ARB_texture_env_add;
          continue;
        }
#endif
#ifdef GL_ARB_texture_env_combine
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine", 19))
        {
          ret = GLEW_ARB_texture_env_combine;
          continue;
        }
#endif
#ifdef GL_ARB_texture_env_crossbar
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_crossbar", 20))
        {
          ret = GLEW_ARB_texture_env_crossbar;
          continue;
        }
#endif
#ifdef GL_ARB_texture_env_dot3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_dot3", 16))
        {
          ret = GLEW_ARB_texture_env_dot3;
          continue;
        }
#endif
#ifdef GL_ARB_texture_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_float", 13))
        {
          ret = GLEW_ARB_texture_float;
          continue;
        }
#endif
#ifdef GL_ARB_texture_mirrored_repeat
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_mirrored_repeat", 23))
        {
          ret = GLEW_ARB_texture_mirrored_repeat;
          continue;
        }
#endif
#ifdef GL_ARB_texture_non_power_of_two
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_non_power_of_two", 24))
        {
          ret = GLEW_ARB_texture_non_power_of_two;
          continue;
        }
#endif
#ifdef GL_ARB_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rectangle", 17))
        {
          ret = GLEW_ARB_texture_rectangle;
          continue;
        }
#endif
#ifdef GL_ARB_texture_rg
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rg", 10))
        {
          ret = GLEW_ARB_texture_rg;
          continue;
        }
#endif
#ifdef GL_ARB_transpose_matrix
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transpose_matrix", 16))
        {
          ret = GLEW_ARB_transpose_matrix;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_object", 19))
        {
          ret = GLEW_ARB_vertex_array_object;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_blend
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_blend", 12))
        {
          ret = GLEW_ARB_vertex_blend;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_buffer_object", 20))
        {
          ret = GLEW_ARB_vertex_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_program
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program", 14))
        {
          ret = GLEW_ARB_vertex_program;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_shader", 13))
        {
          ret = GLEW_ARB_vertex_shader;
          continue;
        }
#endif
#ifdef GL_ARB_window_pos
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"window_pos", 10))
        {
          ret = GLEW_ARB_window_pos;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ATIX_", 5))
      {
#ifdef GL_ATIX_point_sprites
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_sprites", 13))
        {
          ret = GLEW_ATIX_point_sprites;
          continue;
        }
#endif
#ifdef GL_ATIX_texture_env_combine3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine3", 20))
        {
          ret = GLEW_ATIX_texture_env_combine3;
          continue;
        }
#endif
#ifdef GL_ATIX_texture_env_route
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_route", 17))
        {
          ret = GLEW_ATIX_texture_env_route;
          continue;
        }
#endif
#ifdef GL_ATIX_vertex_shader_output_point_size
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_shader_output_point_size", 31))
        {
          ret = GLEW_ATIX_vertex_shader_output_point_size;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ATI_", 4))
      {
#ifdef GL_ATI_draw_buffers
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_buffers", 12))
        {
          ret = GLEW_ATI_draw_buffers;
          continue;
        }
#endif
#ifdef GL_ATI_element_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"element_array", 13))
        {
          ret = GLEW_ATI_element_array;
          continue;
        }
#endif
#ifdef GL_ATI_envmap_bumpmap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"envmap_bumpmap", 14))
        {
          ret = GLEW_ATI_envmap_bumpmap;
          continue;
        }
#endif
#ifdef GL_ATI_fragment_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_shader", 15))
        {
          ret = GLEW_ATI_fragment_shader;
          continue;
        }
#endif
#ifdef GL_ATI_map_object_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"map_object_buffer", 17))
        {
          ret = GLEW_ATI_map_object_buffer;
          continue;
        }
#endif
#ifdef GL_ATI_pn_triangles
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pn_triangles", 12))
        {
          ret = GLEW_ATI_pn_triangles;
          continue;
        }
#endif
#ifdef GL_ATI_separate_stencil
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"separate_stencil", 16))
        {
          ret = GLEW_ATI_separate_stencil;
          continue;
        }
#endif
#ifdef GL_ATI_shader_texture_lod
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_texture_lod", 18))
        {
          ret = GLEW_ATI_shader_texture_lod;
          continue;
        }
#endif
#ifdef GL_ATI_text_fragment_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"text_fragment_shader", 20))
        {
          ret = GLEW_ATI_text_fragment_shader;
          continue;
        }
#endif
#ifdef GL_ATI_texture_compression_3dc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_3dc", 23))
        {
          ret = GLEW_ATI_texture_compression_3dc;
          continue;
        }
#endif
#ifdef GL_ATI_texture_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_float", 13))
        {
          ret = GLEW_ATI_texture_float;
          continue;
        }
#endif
#ifdef GL_ATI_texture_mirror_once
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_mirror_once", 19))
        {
          ret = GLEW_ATI_texture_mirror_once;
          continue;
        }
#endif
#ifdef GL_ATI_vertex_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_object", 19))
        {
          ret = GLEW_ATI_vertex_array_object;
          continue;
        }
#endif
#ifdef GL_ATI_vertex_attrib_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_attrib_array_object", 26))
        {
          ret = GLEW_ATI_vertex_attrib_array_object;
          continue;
        }
#endif
#ifdef GL_ATI_vertex_streams
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_streams", 14))
        {
          ret = GLEW_ATI_vertex_streams;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"EXT_", 4))
      {
#ifdef GL_EXT_422_pixels
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"422_pixels", 10))
        {
          ret = GLEW_EXT_422_pixels;
          continue;
        }
#endif
#ifdef GL_EXT_Cg_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"Cg_shader", 9))
        {
          ret = GLEW_EXT_Cg_shader;
          continue;
        }
#endif
#ifdef GL_EXT_abgr
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"abgr", 4))
        {
          ret = GLEW_EXT_abgr;
          continue;
        }
#endif
#ifdef GL_EXT_bgra
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"bgra", 4))
        {
          ret = GLEW_EXT_bgra;
          continue;
        }
#endif
#ifdef GL_EXT_bindable_uniform
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"bindable_uniform", 16))
        {
          ret = GLEW_EXT_bindable_uniform;
          continue;
        }
#endif
#ifdef GL_EXT_blend_color
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_color", 11))
        {
          ret = GLEW_EXT_blend_color;
          continue;
        }
#endif
#ifdef GL_EXT_blend_equation_separate
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_equation_separate", 23))
        {
          ret = GLEW_EXT_blend_equation_separate;
          continue;
        }
#endif
#ifdef GL_EXT_blend_func_separate
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_func_separate", 19))
        {
          ret = GLEW_EXT_blend_func_separate;
          continue;
        }
#endif
#ifdef GL_EXT_blend_logic_op
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_logic_op", 14))
        {
          ret = GLEW_EXT_blend_logic_op;
          continue;
        }
#endif
#ifdef GL_EXT_blend_minmax
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_minmax", 12))
        {
          ret = GLEW_EXT_blend_minmax;
          continue;
        }
#endif
#ifdef GL_EXT_blend_subtract
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_subtract", 14))
        {
          ret = GLEW_EXT_blend_subtract;
          continue;
        }
#endif
#ifdef GL_EXT_clip_volume_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"clip_volume_hint", 16))
        {
          ret = GLEW_EXT_clip_volume_hint;
          continue;
        }
#endif
#ifdef GL_EXT_cmyka
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cmyka", 5))
        {
          ret = GLEW_EXT_cmyka;
          continue;
        }
#endif
#ifdef GL_EXT_color_subtable
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_subtable", 14))
        {
          ret = GLEW_EXT_color_subtable;
          continue;
        }
#endif
#ifdef GL_EXT_compiled_vertex_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"compiled_vertex_array", 21))
        {
          ret = GLEW_EXT_compiled_vertex_array;
          continue;
        }
#endif
#ifdef GL_EXT_convolution
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"convolution", 11))
        {
          ret = GLEW_EXT_convolution;
          continue;
        }
#endif
#ifdef GL_EXT_coordinate_frame
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"coordinate_frame", 16))
        {
          ret = GLEW_EXT_coordinate_frame;
          continue;
        }
#endif
#ifdef GL_EXT_copy_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_texture", 12))
        {
          ret = GLEW_EXT_copy_texture;
          continue;
        }
#endif
#ifdef GL_EXT_cull_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cull_vertex", 11))
        {
          ret = GLEW_EXT_cull_vertex;
          continue;
        }
#endif
#ifdef GL_EXT_depth_bounds_test
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_bounds_test", 17))
        {
          ret = GLEW_EXT_depth_bounds_test;
          continue;
        }
#endif
#ifdef GL_EXT_direct_state_access
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"direct_state_access", 19))
        {
          ret = GLEW_EXT_direct_state_access;
          continue;
        }
#endif
#ifdef GL_EXT_draw_buffers2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_buffers2", 13))
        {
          ret = GLEW_EXT_draw_buffers2;
          continue;
        }
#endif
#ifdef GL_EXT_draw_instanced
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_instanced", 14))
        {
          ret = GLEW_EXT_draw_instanced;
          continue;
        }
#endif
#ifdef GL_EXT_draw_range_elements
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_range_elements", 19))
        {
          ret = GLEW_EXT_draw_range_elements;
          continue;
        }
#endif
#ifdef GL_EXT_fog_coord
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_coord", 9))
        {
          ret = GLEW_EXT_fog_coord;
          continue;
        }
#endif
#ifdef GL_EXT_fragment_lighting
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_lighting", 17))
        {
          ret = GLEW_EXT_fragment_lighting;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_blit
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_blit", 16))
        {
          ret = GLEW_EXT_framebuffer_blit;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_multisample", 23))
        {
          ret = GLEW_EXT_framebuffer_multisample;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_object", 18))
        {
          ret = GLEW_EXT_framebuffer_object;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLEW_EXT_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GL_EXT_geometry_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_shader4", 16))
        {
          ret = GLEW_EXT_geometry_shader4;
          continue;
        }
#endif
#ifdef GL_EXT_gpu_program_parameters
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_program_parameters", 22))
        {
          ret = GLEW_EXT_gpu_program_parameters;
          continue;
        }
#endif
#ifdef GL_EXT_gpu_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader4", 11))
        {
          ret = GLEW_EXT_gpu_shader4;
          continue;
        }
#endif
#ifdef GL_EXT_histogram
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"histogram", 9))
        {
          ret = GLEW_EXT_histogram;
          continue;
        }
#endif
#ifdef GL_EXT_index_array_formats
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"index_array_formats", 19))
        {
          ret = GLEW_EXT_index_array_formats;
          continue;
        }
#endif
#ifdef GL_EXT_index_func
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"index_func", 10))
        {
          ret = GLEW_EXT_index_func;
          continue;
        }
#endif
#ifdef GL_EXT_index_material
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"index_material", 14))
        {
          ret = GLEW_EXT_index_material;
          continue;
        }
#endif
#ifdef GL_EXT_index_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"index_texture", 13))
        {
          ret = GLEW_EXT_index_texture;
          continue;
        }
#endif
#ifdef GL_EXT_light_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"light_texture", 13))
        {
          ret = GLEW_EXT_light_texture;
          continue;
        }
#endif
#ifdef GL_EXT_misc_attribute
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"misc_attribute", 14))
        {
          ret = GLEW_EXT_misc_attribute;
          continue;
        }
#endif
#ifdef GL_EXT_multi_draw_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multi_draw_arrays", 17))
        {
          ret = GLEW_EXT_multi_draw_arrays;
          continue;
        }
#endif
#ifdef GL_EXT_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLEW_EXT_multisample;
          continue;
        }
#endif
#ifdef GL_EXT_packed_depth_stencil
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_depth_stencil", 20))
        {
          ret = GLEW_EXT_packed_depth_stencil;
          continue;
        }
#endif
#ifdef GL_EXT_packed_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_float", 12))
        {
          ret = GLEW_EXT_packed_float;
          continue;
        }
#endif
#ifdef GL_EXT_packed_pixels
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_pixels", 13))
        {
          ret = GLEW_EXT_packed_pixels;
          continue;
        }
#endif
#ifdef GL_EXT_paletted_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"paletted_texture", 16))
        {
          ret = GLEW_EXT_paletted_texture;
          continue;
        }
#endif
#ifdef GL_EXT_pixel_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_buffer_object", 19))
        {
          ret = GLEW_EXT_pixel_buffer_object;
          continue;
        }
#endif
#ifdef GL_EXT_pixel_transform
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_transform", 15))
        {
          ret = GLEW_EXT_pixel_transform;
          continue;
        }
#endif
#ifdef GL_EXT_pixel_transform_color_table
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_transform_color_table", 27))
        {
          ret = GLEW_EXT_pixel_transform_color_table;
          continue;
        }
#endif
#ifdef GL_EXT_point_parameters
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_parameters", 16))
        {
          ret = GLEW_EXT_point_parameters;
          continue;
        }
#endif
#ifdef GL_EXT_polygon_offset
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"polygon_offset", 14))
        {
          ret = GLEW_EXT_polygon_offset;
          continue;
        }
#endif
#ifdef GL_EXT_rescale_normal
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"rescale_normal", 14))
        {
          ret = GLEW_EXT_rescale_normal;
          continue;
        }
#endif
#ifdef GL_EXT_scene_marker
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"scene_marker", 12))
        {
          ret = GLEW_EXT_scene_marker;
          continue;
        }
#endif
#ifdef GL_EXT_secondary_color
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"secondary_color", 15))
        {
          ret = GLEW_EXT_secondary_color;
          continue;
        }
#endif
#ifdef GL_EXT_separate_specular_color
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"separate_specular_color", 23))
        {
          ret = GLEW_EXT_separate_specular_color;
          continue;
        }
#endif
#ifdef GL_EXT_shadow_funcs
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow_funcs", 12))
        {
          ret = GLEW_EXT_shadow_funcs;
          continue;
        }
#endif
#ifdef GL_EXT_shared_texture_palette
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shared_texture_palette", 22))
        {
          ret = GLEW_EXT_shared_texture_palette;
          continue;
        }
#endif
#ifdef GL_EXT_stencil_clear_tag
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"stencil_clear_tag", 17))
        {
          ret = GLEW_EXT_stencil_clear_tag;
          continue;
        }
#endif
#ifdef GL_EXT_stencil_two_side
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"stencil_two_side", 16))
        {
          ret = GLEW_EXT_stencil_two_side;
          continue;
        }
#endif
#ifdef GL_EXT_stencil_wrap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"stencil_wrap", 12))
        {
          ret = GLEW_EXT_stencil_wrap;
          continue;
        }
#endif
#ifdef GL_EXT_subtexture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"subtexture", 10))
        {
          ret = GLEW_EXT_subtexture;
          continue;
        }
#endif
#ifdef GL_EXT_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture", 7))
        {
          ret = GLEW_EXT_texture;
          continue;
        }
#endif
#ifdef GL_EXT_texture3D
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture3D", 9))
        {
          ret = GLEW_EXT_texture3D;
          continue;
        }
#endif
#ifdef GL_EXT_texture_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_array", 13))
        {
          ret = GLEW_EXT_texture_array;
          continue;
        }
#endif
#ifdef GL_EXT_texture_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_buffer_object", 21))
        {
          ret = GLEW_EXT_texture_buffer_object;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_dxt1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_dxt1", 24))
        {
          ret = GLEW_EXT_texture_compression_dxt1;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_latc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_latc", 24))
        {
          ret = GLEW_EXT_texture_compression_latc;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_rgtc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_rgtc", 24))
        {
          ret = GLEW_EXT_texture_compression_rgtc;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_s3tc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_s3tc", 24))
        {
          ret = GLEW_EXT_texture_compression_s3tc;
          continue;
        }
#endif
#ifdef GL_EXT_texture_cube_map
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_cube_map", 16))
        {
          ret = GLEW_EXT_texture_cube_map;
          continue;
        }
#endif
#ifdef GL_EXT_texture_edge_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_edge_clamp", 18))
        {
          ret = GLEW_EXT_texture_edge_clamp;
          continue;
        }
#endif
#ifdef GL_EXT_texture_env
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env", 11))
        {
          ret = GLEW_EXT_texture_env;
          continue;
        }
#endif
#ifdef GL_EXT_texture_env_add
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_add", 15))
        {
          ret = GLEW_EXT_texture_env_add;
          continue;
        }
#endif
#ifdef GL_EXT_texture_env_combine
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine", 19))
        {
          ret = GLEW_EXT_texture_env_combine;
          continue;
        }
#endif
#ifdef GL_EXT_texture_env_dot3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_dot3", 16))
        {
          ret = GLEW_EXT_texture_env_dot3;
          continue;
        }
#endif
#ifdef GL_EXT_texture_filter_anisotropic
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_filter_anisotropic", 26))
        {
          ret = GLEW_EXT_texture_filter_anisotropic;
          continue;
        }
#endif
#ifdef GL_EXT_texture_integer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_integer", 15))
        {
          ret = GLEW_EXT_texture_integer;
          continue;
        }
#endif
#ifdef GL_EXT_texture_lod_bias
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_lod_bias", 16))
        {
          ret = GLEW_EXT_texture_lod_bias;
          continue;
        }
#endif
#ifdef GL_EXT_texture_mirror_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_mirror_clamp", 20))
        {
          ret = GLEW_EXT_texture_mirror_clamp;
          continue;
        }
#endif
#ifdef GL_EXT_texture_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_object", 14))
        {
          ret = GLEW_EXT_texture_object;
          continue;
        }
#endif
#ifdef GL_EXT_texture_perturb_normal
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_perturb_normal", 22))
        {
          ret = GLEW_EXT_texture_perturb_normal;
          continue;
        }
#endif
#ifdef GL_EXT_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rectangle", 17))
        {
          ret = GLEW_EXT_texture_rectangle;
          continue;
        }
#endif
#ifdef GL_EXT_texture_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_sRGB", 12))
        {
          ret = GLEW_EXT_texture_sRGB;
          continue;
        }
#endif
#ifdef GL_EXT_texture_shared_exponent
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_shared_exponent", 23))
        {
          ret = GLEW_EXT_texture_shared_exponent;
          continue;
        }
#endif
#ifdef GL_EXT_texture_swizzle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_swizzle", 15))
        {
          ret = GLEW_EXT_texture_swizzle;
          continue;
        }
#endif
#ifdef GL_EXT_timer_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"timer_query", 11))
        {
          ret = GLEW_EXT_timer_query;
          continue;
        }
#endif
#ifdef GL_EXT_transform_feedback
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback", 18))
        {
          ret = GLEW_EXT_transform_feedback;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array", 12))
        {
          ret = GLEW_EXT_vertex_array;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_array_bgra
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_bgra", 17))
        {
          ret = GLEW_EXT_vertex_array_bgra;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_shader", 13))
        {
          ret = GLEW_EXT_vertex_shader;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_weighting
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_weighting", 16))
        {
          ret = GLEW_EXT_vertex_weighting;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"GREMEDY_", 8))
      {
#ifdef GL_GREMEDY_frame_terminator
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"frame_terminator", 16))
        {
          ret = GLEW_GREMEDY_frame_terminator;
          continue;
        }
#endif
#ifdef GL_GREMEDY_string_marker
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"string_marker", 13))
        {
          ret = GLEW_GREMEDY_string_marker;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"HP_", 3))
      {
#ifdef GL_HP_convolution_border_modes
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"convolution_border_modes", 24))
        {
          ret = GLEW_HP_convolution_border_modes;
          continue;
        }
#endif
#ifdef GL_HP_image_transform
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"image_transform", 15))
        {
          ret = GLEW_HP_image_transform;
          continue;
        }
#endif
#ifdef GL_HP_occlusion_test
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"occlusion_test", 14))
        {
          ret = GLEW_HP_occlusion_test;
          continue;
        }
#endif
#ifdef GL_HP_texture_lighting
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_lighting", 16))
        {
          ret = GLEW_HP_texture_lighting;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"IBM_", 4))
      {
#ifdef GL_IBM_cull_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cull_vertex", 11))
        {
          ret = GLEW_IBM_cull_vertex;
          continue;
        }
#endif
#ifdef GL_IBM_multimode_draw_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multimode_draw_arrays", 21))
        {
          ret = GLEW_IBM_multimode_draw_arrays;
          continue;
        }
#endif
#ifdef GL_IBM_rasterpos_clip
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"rasterpos_clip", 14))
        {
          ret = GLEW_IBM_rasterpos_clip;
          continue;
        }
#endif
#ifdef GL_IBM_static_data
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"static_data", 11))
        {
          ret = GLEW_IBM_static_data;
          continue;
        }
#endif
#ifdef GL_IBM_texture_mirrored_repeat
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_mirrored_repeat", 23))
        {
          ret = GLEW_IBM_texture_mirrored_repeat;
          continue;
        }
#endif
#ifdef GL_IBM_vertex_array_lists
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_lists", 18))
        {
          ret = GLEW_IBM_vertex_array_lists;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"INGR_", 5))
      {
#ifdef GL_INGR_color_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_clamp", 11))
        {
          ret = GLEW_INGR_color_clamp;
          continue;
        }
#endif
#ifdef GL_INGR_interlace_read
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"interlace_read", 14))
        {
          ret = GLEW_INGR_interlace_read;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"INTEL_", 6))
      {
#ifdef GL_INTEL_parallel_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"parallel_arrays", 15))
        {
          ret = GLEW_INTEL_parallel_arrays;
          continue;
        }
#endif
#ifdef GL_INTEL_texture_scissor
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_scissor", 15))
        {
          ret = GLEW_INTEL_texture_scissor;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"KTX_", 4))
      {
#ifdef GL_KTX_buffer_region
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"buffer_region", 13))
        {
          ret = GLEW_KTX_buffer_region;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"MESAX_", 6))
      {
#ifdef GL_MESAX_texture_stack
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_stack", 13))
        {
          ret = GLEW_MESAX_texture_stack;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"MESA_", 5))
      {
#ifdef GL_MESA_pack_invert
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pack_invert", 11))
        {
          ret = GLEW_MESA_pack_invert;
          continue;
        }
#endif
#ifdef GL_MESA_resize_buffers
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"resize_buffers", 14))
        {
          ret = GLEW_MESA_resize_buffers;
          continue;
        }
#endif
#ifdef GL_MESA_window_pos
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"window_pos", 10))
        {
          ret = GLEW_MESA_window_pos;
          continue;
        }
#endif
#ifdef GL_MESA_ycbcr_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ycbcr_texture", 13))
        {
          ret = GLEW_MESA_ycbcr_texture;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NV_", 3))
      {
#ifdef GL_NV_blend_square
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_square", 12))
        {
          ret = GLEW_NV_blend_square;
          continue;
        }
#endif
#ifdef GL_NV_conditional_render
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"conditional_render", 18))
        {
          ret = GLEW_NV_conditional_render;
          continue;
        }
#endif
#ifdef GL_NV_copy_depth_to_color
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_depth_to_color", 19))
        {
          ret = GLEW_NV_copy_depth_to_color;
          continue;
        }
#endif
#ifdef GL_NV_depth_buffer_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_buffer_float", 18))
        {
          ret = GLEW_NV_depth_buffer_float;
          continue;
        }
#endif
#ifdef GL_NV_depth_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_clamp", 11))
        {
          ret = GLEW_NV_depth_clamp;
          continue;
        }
#endif
#ifdef GL_NV_depth_range_unclamped
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_range_unclamped", 21))
        {
          ret = GLEW_NV_depth_range_unclamped;
          continue;
        }
#endif
#ifdef GL_NV_evaluators
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"evaluators", 10))
        {
          ret = GLEW_NV_evaluators;
          continue;
        }
#endif
#ifdef GL_NV_explicit_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"explicit_multisample", 20))
        {
          ret = GLEW_NV_explicit_multisample;
          continue;
        }
#endif
#ifdef GL_NV_fence
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fence", 5))
        {
          ret = GLEW_NV_fence;
          continue;
        }
#endif
#ifdef GL_NV_float_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"float_buffer", 12))
        {
          ret = GLEW_NV_float_buffer;
          continue;
        }
#endif
#ifdef GL_NV_fog_distance
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_distance", 12))
        {
          ret = GLEW_NV_fog_distance;
          continue;
        }
#endif
#ifdef GL_NV_fragment_program
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program", 16))
        {
          ret = GLEW_NV_fragment_program;
          continue;
        }
#endif
#ifdef GL_NV_fragment_program2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program2", 17))
        {
          ret = GLEW_NV_fragment_program2;
          continue;
        }
#endif
#ifdef GL_NV_fragment_program4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program4", 17))
        {
          ret = GLEW_NV_fragment_program4;
          continue;
        }
#endif
#ifdef GL_NV_fragment_program_option
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program_option", 23))
        {
          ret = GLEW_NV_fragment_program_option;
          continue;
        }
#endif
#ifdef GL_NV_framebuffer_multisample_coverage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_multisample_coverage", 32))
        {
          ret = GLEW_NV_framebuffer_multisample_coverage;
          continue;
        }
#endif
#ifdef GL_NV_geometry_program4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_program4", 17))
        {
          ret = GLEW_NV_geometry_program4;
          continue;
        }
#endif
#ifdef GL_NV_geometry_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_shader4", 16))
        {
          ret = GLEW_NV_geometry_shader4;
          continue;
        }
#endif
#ifdef GL_NV_gpu_program4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_program4", 12))
        {
          ret = GLEW_NV_gpu_program4;
          continue;
        }
#endif
#ifdef GL_NV_half_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"half_float", 10))
        {
          ret = GLEW_NV_half_float;
          continue;
        }
#endif
#ifdef GL_NV_light_max_exponent
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"light_max_exponent", 18))
        {
          ret = GLEW_NV_light_max_exponent;
          continue;
        }
#endif
#ifdef GL_NV_multisample_filter_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample_filter_hint", 23))
        {
          ret = GLEW_NV_multisample_filter_hint;
          continue;
        }
#endif
#ifdef GL_NV_occlusion_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"occlusion_query", 15))
        {
          ret = GLEW_NV_occlusion_query;
          continue;
        }
#endif
#ifdef GL_NV_packed_depth_stencil
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_depth_stencil", 20))
        {
          ret = GLEW_NV_packed_depth_stencil;
          continue;
        }
#endif
#ifdef GL_NV_parameter_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"parameter_buffer_object", 23))
        {
          ret = GLEW_NV_parameter_buffer_object;
          continue;
        }
#endif
#ifdef GL_NV_pixel_data_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_data_range", 16))
        {
          ret = GLEW_NV_pixel_data_range;
          continue;
        }
#endif
#ifdef GL_NV_point_sprite
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_sprite", 12))
        {
          ret = GLEW_NV_point_sprite;
          continue;
        }
#endif
#ifdef GL_NV_present_video
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"present_video", 13))
        {
          ret = GLEW_NV_present_video;
          continue;
        }
#endif
#ifdef GL_NV_primitive_restart
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"primitive_restart", 17))
        {
          ret = GLEW_NV_primitive_restart;
          continue;
        }
#endif
#ifdef GL_NV_register_combiners
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"register_combiners", 18))
        {
          ret = GLEW_NV_register_combiners;
          continue;
        }
#endif
#ifdef GL_NV_register_combiners2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"register_combiners2", 19))
        {
          ret = GLEW_NV_register_combiners2;
          continue;
        }
#endif
#ifdef GL_NV_texgen_emboss
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texgen_emboss", 13))
        {
          ret = GLEW_NV_texgen_emboss;
          continue;
        }
#endif
#ifdef GL_NV_texgen_reflection
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texgen_reflection", 17))
        {
          ret = GLEW_NV_texgen_reflection;
          continue;
        }
#endif
#ifdef GL_NV_texture_compression_vtc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_vtc", 23))
        {
          ret = GLEW_NV_texture_compression_vtc;
          continue;
        }
#endif
#ifdef GL_NV_texture_env_combine4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine4", 20))
        {
          ret = GLEW_NV_texture_env_combine4;
          continue;
        }
#endif
#ifdef GL_NV_texture_expand_normal
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_expand_normal", 21))
        {
          ret = GLEW_NV_texture_expand_normal;
          continue;
        }
#endif
#ifdef GL_NV_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rectangle", 17))
        {
          ret = GLEW_NV_texture_rectangle;
          continue;
        }
#endif
#ifdef GL_NV_texture_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_shader", 14))
        {
          ret = GLEW_NV_texture_shader;
          continue;
        }
#endif
#ifdef GL_NV_texture_shader2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_shader2", 15))
        {
          ret = GLEW_NV_texture_shader2;
          continue;
        }
#endif
#ifdef GL_NV_texture_shader3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_shader3", 15))
        {
          ret = GLEW_NV_texture_shader3;
          continue;
        }
#endif
#ifdef GL_NV_transform_feedback
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback", 18))
        {
          ret = GLEW_NV_transform_feedback;
          continue;
        }
#endif
#ifdef GL_NV_transform_feedback2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback2", 19))
        {
          ret = GLEW_NV_transform_feedback2;
          continue;
        }
#endif
#ifdef GL_NV_vertex_array_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range", 18))
        {
          ret = GLEW_NV_vertex_array_range;
          continue;
        }
#endif
#ifdef GL_NV_vertex_array_range2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range2", 19))
        {
          ret = GLEW_NV_vertex_array_range2;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program", 14))
        {
          ret = GLEW_NV_vertex_program;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program1_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program1_1", 17))
        {
          ret = GLEW_NV_vertex_program1_1;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program2", 15))
        {
          ret = GLEW_NV_vertex_program2;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program2_option
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program2_option", 22))
        {
          ret = GLEW_NV_vertex_program2_option;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program3", 15))
        {
          ret = GLEW_NV_vertex_program3;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program4", 15))
        {
          ret = GLEW_NV_vertex_program4;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"OES_", 4))
      {
#ifdef GL_OES_byte_coordinates
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"byte_coordinates", 16))
        {
          ret = GLEW_OES_byte_coordinates;
          continue;
        }
#endif
#ifdef GL_OES_compressed_paletted_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"compressed_paletted_texture", 27))
        {
          ret = GLEW_OES_compressed_paletted_texture;
          continue;
        }
#endif
#ifdef GL_OES_read_format
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"read_format", 11))
        {
          ret = GLEW_OES_read_format;
          continue;
        }
#endif
#ifdef GL_OES_single_precision
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"single_precision", 16))
        {
          ret = GLEW_OES_single_precision;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"OML_", 4))
      {
#ifdef GL_OML_interlace
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"interlace", 9))
        {
          ret = GLEW_OML_interlace;
          continue;
        }
#endif
#ifdef GL_OML_resample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"resample", 8))
        {
          ret = GLEW_OML_resample;
          continue;
        }
#endif
#ifdef GL_OML_subsample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"subsample", 9))
        {
          ret = GLEW_OML_subsample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"PGI_", 4))
      {
#ifdef GL_PGI_misc_hints
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"misc_hints", 10))
        {
          ret = GLEW_PGI_misc_hints;
          continue;
        }
#endif
#ifdef GL_PGI_vertex_hints
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_hints", 12))
        {
          ret = GLEW_PGI_vertex_hints;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"REND_", 5))
      {
#ifdef GL_REND_screen_coordinates
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"screen_coordinates", 18))
        {
          ret = GLEW_REND_screen_coordinates;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"S3_", 3))
      {
#ifdef GL_S3_s3tc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"s3tc", 4))
        {
          ret = GLEW_S3_s3tc;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGIS_", 5))
      {
#ifdef GL_SGIS_color_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_range", 11))
        {
          ret = GLEW_SGIS_color_range;
          continue;
        }
#endif
#ifdef GL_SGIS_detail_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"detail_texture", 14))
        {
          ret = GLEW_SGIS_detail_texture;
          continue;
        }
#endif
#ifdef GL_SGIS_fog_function
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_function", 12))
        {
          ret = GLEW_SGIS_fog_function;
          continue;
        }
#endif
#ifdef GL_SGIS_generate_mipmap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"generate_mipmap", 15))
        {
          ret = GLEW_SGIS_generate_mipmap;
          continue;
        }
#endif
#ifdef GL_SGIS_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLEW_SGIS_multisample;
          continue;
        }
#endif
#ifdef GL_SGIS_pixel_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_texture", 13))
        {
          ret = GLEW_SGIS_pixel_texture;
          continue;
        }
#endif
#ifdef GL_SGIS_point_line_texgen
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_line_texgen", 17))
        {
          ret = GLEW_SGIS_point_line_texgen;
          continue;
        }
#endif
#ifdef GL_SGIS_sharpen_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sharpen_texture", 15))
        {
          ret = GLEW_SGIS_sharpen_texture;
          continue;
        }
#endif
#ifdef GL_SGIS_texture4D
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture4D", 9))
        {
          ret = GLEW_SGIS_texture4D;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_border_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_border_clamp", 20))
        {
          ret = GLEW_SGIS_texture_border_clamp;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_edge_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_edge_clamp", 18))
        {
          ret = GLEW_SGIS_texture_edge_clamp;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_filter4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_filter4", 15))
        {
          ret = GLEW_SGIS_texture_filter4;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_lod
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_lod", 11))
        {
          ret = GLEW_SGIS_texture_lod;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_select
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_select", 14))
        {
          ret = GLEW_SGIS_texture_select;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGIX_", 5))
      {
#ifdef GL_SGIX_async
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"async", 5))
        {
          ret = GLEW_SGIX_async;
          continue;
        }
#endif
#ifdef GL_SGIX_async_histogram
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"async_histogram", 15))
        {
          ret = GLEW_SGIX_async_histogram;
          continue;
        }
#endif
#ifdef GL_SGIX_async_pixel
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"async_pixel", 11))
        {
          ret = GLEW_SGIX_async_pixel;
          continue;
        }
#endif
#ifdef GL_SGIX_blend_alpha_minmax
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_alpha_minmax", 18))
        {
          ret = GLEW_SGIX_blend_alpha_minmax;
          continue;
        }
#endif
#ifdef GL_SGIX_clipmap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"clipmap", 7))
        {
          ret = GLEW_SGIX_clipmap;
          continue;
        }
#endif
#ifdef GL_SGIX_convolution_accuracy
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"convolution_accuracy", 20))
        {
          ret = GLEW_SGIX_convolution_accuracy;
          continue;
        }
#endif
#ifdef GL_SGIX_depth_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_texture", 13))
        {
          ret = GLEW_SGIX_depth_texture;
          continue;
        }
#endif
#ifdef GL_SGIX_flush_raster
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"flush_raster", 12))
        {
          ret = GLEW_SGIX_flush_raster;
          continue;
        }
#endif
#ifdef GL_SGIX_fog_offset
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_offset", 10))
        {
          ret = GLEW_SGIX_fog_offset;
          continue;
        }
#endif
#ifdef GL_SGIX_fog_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_texture", 11))
        {
          ret = GLEW_SGIX_fog_texture;
          continue;
        }
#endif
#ifdef GL_SGIX_fragment_specular_lighting
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_specular_lighting", 26))
        {
          ret = GLEW_SGIX_fragment_specular_lighting;
          continue;
        }
#endif
#ifdef GL_SGIX_framezoom
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framezoom", 9))
        {
          ret = GLEW_SGIX_framezoom;
          continue;
        }
#endif
#ifdef GL_SGIX_interlace
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"interlace", 9))
        {
          ret = GLEW_SGIX_interlace;
          continue;
        }
#endif
#ifdef GL_SGIX_ir_instrument1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ir_instrument1", 14))
        {
          ret = GLEW_SGIX_ir_instrument1;
          continue;
        }
#endif
#ifdef GL_SGIX_list_priority
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"list_priority", 13))
        {
          ret = GLEW_SGIX_list_priority;
          continue;
        }
#endif
#ifdef GL_SGIX_pixel_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_texture", 13))
        {
          ret = GLEW_SGIX_pixel_texture;
          continue;
        }
#endif
#ifdef GL_SGIX_pixel_texture_bits
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_texture_bits", 18))
        {
          ret = GLEW_SGIX_pixel_texture_bits;
          continue;
        }
#endif
#ifdef GL_SGIX_reference_plane
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"reference_plane", 15))
        {
          ret = GLEW_SGIX_reference_plane;
          continue;
        }
#endif
#ifdef GL_SGIX_resample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"resample", 8))
        {
          ret = GLEW_SGIX_resample;
          continue;
        }
#endif
#ifdef GL_SGIX_shadow
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow", 6))
        {
          ret = GLEW_SGIX_shadow;
          continue;
        }
#endif
#ifdef GL_SGIX_shadow_ambient
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow_ambient", 14))
        {
          ret = GLEW_SGIX_shadow_ambient;
          continue;
        }
#endif
#ifdef GL_SGIX_sprite
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sprite", 6))
        {
          ret = GLEW_SGIX_sprite;
          continue;
        }
#endif
#ifdef GL_SGIX_tag_sample_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"tag_sample_buffer", 17))
        {
          ret = GLEW_SGIX_tag_sample_buffer;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_add_env
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_add_env", 15))
        {
          ret = GLEW_SGIX_texture_add_env;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_coordinate_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_coordinate_clamp", 24))
        {
          ret = GLEW_SGIX_texture_coordinate_clamp;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_lod_bias
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_lod_bias", 16))
        {
          ret = GLEW_SGIX_texture_lod_bias;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_multi_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_multi_buffer", 20))
        {
          ret = GLEW_SGIX_texture_multi_buffer;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_range", 13))
        {
          ret = GLEW_SGIX_texture_range;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_scale_bias
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_scale_bias", 18))
        {
          ret = GLEW_SGIX_texture_scale_bias;
          continue;
        }
#endif
#ifdef GL_SGIX_vertex_preclip
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_preclip", 14))
        {
          ret = GLEW_SGIX_vertex_preclip;
          continue;
        }
#endif
#ifdef GL_SGIX_vertex_preclip_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_preclip_hint", 19))
        {
          ret = GLEW_SGIX_vertex_preclip_hint;
          continue;
        }
#endif
#ifdef GL_SGIX_ycrcb
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ycrcb", 5))
        {
          ret = GLEW_SGIX_ycrcb;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGI_", 4))
      {
#ifdef GL_SGI_color_matrix
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_matrix", 12))
        {
          ret = GLEW_SGI_color_matrix;
          continue;
        }
#endif
#ifdef GL_SGI_color_table
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_table", 11))
        {
          ret = GLEW_SGI_color_table;
          continue;
        }
#endif
#ifdef GL_SGI_texture_color_table
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_color_table", 19))
        {
          ret = GLEW_SGI_texture_color_table;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SUNX_", 5))
      {
#ifdef GL_SUNX_constant_data
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"constant_data", 13))
        {
          ret = GLEW_SUNX_constant_data;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SUN_", 4))
      {
#ifdef GL_SUN_convolution_border_modes
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"convolution_border_modes", 24))
        {
          ret = GLEW_SUN_convolution_border_modes;
          continue;
        }
#endif
#ifdef GL_SUN_global_alpha
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"global_alpha", 12))
        {
          ret = GLEW_SUN_global_alpha;
          continue;
        }
#endif
#ifdef GL_SUN_mesh_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"mesh_array", 10))
        {
          ret = GLEW_SUN_mesh_array;
          continue;
        }
#endif
#ifdef GL_SUN_read_video_pixels
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"read_video_pixels", 17))
        {
          ret = GLEW_SUN_read_video_pixels;
          continue;
        }
#endif
#ifdef GL_SUN_slice_accum
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"slice_accum", 11))
        {
          ret = GLEW_SUN_slice_accum;
          continue;
        }
#endif
#ifdef GL_SUN_triangle_list
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"triangle_list", 13))
        {
          ret = GLEW_SUN_triangle_list;
          continue;
        }
#endif
#ifdef GL_SUN_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex", 6))
        {
          ret = GLEW_SUN_vertex;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"WIN_", 4))
      {
#ifdef GL_WIN_phong_shading
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"phong_shading", 13))
        {
          ret = GLEW_WIN_phong_shading;
          continue;
        }
#endif
#ifdef GL_WIN_specular_fog
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"specular_fog", 12))
        {
          ret = GLEW_WIN_specular_fog;
          continue;
        }
#endif
#ifdef GL_WIN_swap_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_hint", 9))
        {
          ret = GLEW_WIN_swap_hint;
          continue;
        }
#endif
      }
    }
    ret = (len == 0);
  }
  return ret;
}

#if defined(_WIN32)

#if defined(GLEW_MX)
GLboolean wglewContextIsSupported (WGLEWContext* ctx, const char* name)
#else
GLboolean wglewIsSupported (const char* name)
#endif
{
  GLubyte* pos = (GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"WGL_", 4))
    {
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"3DFX_", 5))
      {
#ifdef WGL_3DFX_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = WGLEW_3DFX_multisample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"3DL_", 4))
      {
#ifdef WGL_3DL_stereo_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"stereo_control", 14))
        {
          ret = WGLEW_3DL_stereo_control;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ARB_", 4))
      {
#ifdef WGL_ARB_buffer_region
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"buffer_region", 13))
        {
          ret = WGLEW_ARB_buffer_region;
          continue;
        }
#endif
#ifdef WGL_ARB_create_context
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context", 14))
        {
          ret = WGLEW_ARB_create_context;
          continue;
        }
#endif
#ifdef WGL_ARB_extensions_string
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"extensions_string", 17))
        {
          ret = WGLEW_ARB_extensions_string;
          continue;
        }
#endif
#ifdef WGL_ARB_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = WGLEW_ARB_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef WGL_ARB_make_current_read
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"make_current_read", 17))
        {
          ret = WGLEW_ARB_make_current_read;
          continue;
        }
#endif
#ifdef WGL_ARB_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = WGLEW_ARB_multisample;
          continue;
        }
#endif
#ifdef WGL_ARB_pbuffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pbuffer", 7))
        {
          ret = WGLEW_ARB_pbuffer;
          continue;
        }
#endif
#ifdef WGL_ARB_pixel_format
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format", 12))
        {
          ret = WGLEW_ARB_pixel_format;
          continue;
        }
#endif
#ifdef WGL_ARB_pixel_format_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format_float", 18))
        {
          ret = WGLEW_ARB_pixel_format_float;
          continue;
        }
#endif
#ifdef WGL_ARB_render_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_texture", 14))
        {
          ret = WGLEW_ARB_render_texture;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ATI_", 4))
      {
#ifdef WGL_ATI_pixel_format_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format_float", 18))
        {
          ret = WGLEW_ATI_pixel_format_float;
          continue;
        }
#endif
#ifdef WGL_ATI_render_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_texture_rectangle", 24))
        {
          ret = WGLEW_ATI_render_texture_rectangle;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"EXT_", 4))
      {
#ifdef WGL_EXT_depth_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_float", 11))
        {
          ret = WGLEW_EXT_depth_float;
          continue;
        }
#endif
#ifdef WGL_EXT_display_color_table
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"display_color_table", 19))
        {
          ret = WGLEW_EXT_display_color_table;
          continue;
        }
#endif
#ifdef WGL_EXT_extensions_string
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"extensions_string", 17))
        {
          ret = WGLEW_EXT_extensions_string;
          continue;
        }
#endif
#ifdef WGL_EXT_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = WGLEW_EXT_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef WGL_EXT_make_current_read
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"make_current_read", 17))
        {
          ret = WGLEW_EXT_make_current_read;
          continue;
        }
#endif
#ifdef WGL_EXT_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = WGLEW_EXT_multisample;
          continue;
        }
#endif
#ifdef WGL_EXT_pbuffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pbuffer", 7))
        {
          ret = WGLEW_EXT_pbuffer;
          continue;
        }
#endif
#ifdef WGL_EXT_pixel_format
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format", 12))
        {
          ret = WGLEW_EXT_pixel_format;
          continue;
        }
#endif
#ifdef WGL_EXT_pixel_format_packed_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format_packed_float", 25))
        {
          ret = WGLEW_EXT_pixel_format_packed_float;
          continue;
        }
#endif
#ifdef WGL_EXT_swap_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_control", 12))
        {
          ret = WGLEW_EXT_swap_control;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"I3D_", 4))
      {
#ifdef WGL_I3D_digital_video_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"digital_video_control", 21))
        {
          ret = WGLEW_I3D_digital_video_control;
          continue;
        }
#endif
#ifdef WGL_I3D_gamma
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gamma", 5))
        {
          ret = WGLEW_I3D_gamma;
          continue;
        }
#endif
#ifdef WGL_I3D_genlock
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"genlock", 7))
        {
          ret = WGLEW_I3D_genlock;
          continue;
        }
#endif
#ifdef WGL_I3D_image_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"image_buffer", 12))
        {
          ret = WGLEW_I3D_image_buffer;
          continue;
        }
#endif
#ifdef WGL_I3D_swap_frame_lock
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_frame_lock", 15))
        {
          ret = WGLEW_I3D_swap_frame_lock;
          continue;
        }
#endif
#ifdef WGL_I3D_swap_frame_usage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_frame_usage", 16))
        {
          ret = WGLEW_I3D_swap_frame_usage;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NV_", 3))
      {
#ifdef WGL_NV_float_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"float_buffer", 12))
        {
          ret = WGLEW_NV_float_buffer;
          continue;
        }
#endif
#ifdef WGL_NV_gpu_affinity
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_affinity", 12))
        {
          ret = WGLEW_NV_gpu_affinity;
          continue;
        }
#endif
#ifdef WGL_NV_present_video
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"present_video", 13))
        {
          ret = WGLEW_NV_present_video;
          continue;
        }
#endif
#ifdef WGL_NV_render_depth_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_depth_texture", 20))
        {
          ret = WGLEW_NV_render_depth_texture;
          continue;
        }
#endif
#ifdef WGL_NV_render_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_texture_rectangle", 24))
        {
          ret = WGLEW_NV_render_texture_rectangle;
          continue;
        }
#endif
#ifdef WGL_NV_swap_group
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_group", 10))
        {
          ret = WGLEW_NV_swap_group;
          continue;
        }
#endif
#ifdef WGL_NV_vertex_array_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range", 18))
        {
          ret = WGLEW_NV_vertex_array_range;
          continue;
        }
#endif
#ifdef WGL_NV_video_output
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_output", 12))
        {
          ret = WGLEW_NV_video_output;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"OML_", 4))
      {
#ifdef WGL_OML_sync_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sync_control", 12))
        {
          ret = WGLEW_OML_sync_control;
          continue;
        }
#endif
      }
    }
    ret = (len == 0);
  }
  return ret;
}

#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX)

#if defined(GLEW_MX)
GLboolean glxewContextIsSupported (GLXEWContext* ctx, const char* name)
#else
GLboolean glxewIsSupported (const char* name)
#endif
{
  GLubyte* pos = (GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if(_glewStrSame1(&pos, &len, (const GLubyte*)"GLX_", 4))
    {
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"VERSION_", 8))
      {
#ifdef GLX_VERSION_1_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_0", 3))
        {
          ret = GLXEW_VERSION_1_0;
          continue;
        }
#endif
#ifdef GLX_VERSION_1_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_1", 3))
        {
          ret = GLXEW_VERSION_1_1;
          continue;
        }
#endif
#ifdef GLX_VERSION_1_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_2", 3))
        {
          ret = GLXEW_VERSION_1_2;
          continue;
        }
#endif
#ifdef GLX_VERSION_1_3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_3", 3))
        {
          ret = GLXEW_VERSION_1_3;
          continue;
        }
#endif
#ifdef GLX_VERSION_1_4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_4", 3))
        {
          ret = GLXEW_VERSION_1_4;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"3DFX_", 5))
      {
#ifdef GLX_3DFX_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLXEW_3DFX_multisample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ARB_", 4))
      {
#ifdef GLX_ARB_create_context
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context", 14))
        {
          ret = GLXEW_ARB_create_context;
          continue;
        }
#endif
#ifdef GLX_ARB_fbconfig_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fbconfig_float", 14))
        {
          ret = GLXEW_ARB_fbconfig_float;
          continue;
        }
#endif
#ifdef GLX_ARB_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLXEW_ARB_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GLX_ARB_get_proc_address
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"get_proc_address", 16))
        {
          ret = GLXEW_ARB_get_proc_address;
          continue;
        }
#endif
#ifdef GLX_ARB_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLXEW_ARB_multisample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ATI_", 4))
      {
#ifdef GLX_ATI_pixel_format_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format_float", 18))
        {
          ret = GLXEW_ATI_pixel_format_float;
          continue;
        }
#endif
#ifdef GLX_ATI_render_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_texture", 14))
        {
          ret = GLXEW_ATI_render_texture;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"EXT_", 4))
      {
#ifdef GLX_EXT_fbconfig_packed_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fbconfig_packed_float", 21))
        {
          ret = GLXEW_EXT_fbconfig_packed_float;
          continue;
        }
#endif
#ifdef GLX_EXT_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLXEW_EXT_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GLX_EXT_import_context
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"import_context", 14))
        {
          ret = GLXEW_EXT_import_context;
          continue;
        }
#endif
#ifdef GLX_EXT_scene_marker
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"scene_marker", 12))
        {
          ret = GLXEW_EXT_scene_marker;
          continue;
        }
#endif
#ifdef GLX_EXT_texture_from_pixmap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_from_pixmap", 19))
        {
          ret = GLXEW_EXT_texture_from_pixmap;
          continue;
        }
#endif
#ifdef GLX_EXT_visual_info
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"visual_info", 11))
        {
          ret = GLXEW_EXT_visual_info;
          continue;
        }
#endif
#ifdef GLX_EXT_visual_rating
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"visual_rating", 13))
        {
          ret = GLXEW_EXT_visual_rating;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"MESA_", 5))
      {
#ifdef GLX_MESA_agp_offset
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"agp_offset", 10))
        {
          ret = GLXEW_MESA_agp_offset;
          continue;
        }
#endif
#ifdef GLX_MESA_copy_sub_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_sub_buffer", 15))
        {
          ret = GLXEW_MESA_copy_sub_buffer;
          continue;
        }
#endif
#ifdef GLX_MESA_pixmap_colormap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixmap_colormap", 15))
        {
          ret = GLXEW_MESA_pixmap_colormap;
          continue;
        }
#endif
#ifdef GLX_MESA_release_buffers
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"release_buffers", 15))
        {
          ret = GLXEW_MESA_release_buffers;
          continue;
        }
#endif
#ifdef GLX_MESA_set_3dfx_mode
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"set_3dfx_mode", 13))
        {
          ret = GLXEW_MESA_set_3dfx_mode;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NV_", 3))
      {
#ifdef GLX_NV_float_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"float_buffer", 12))
        {
          ret = GLXEW_NV_float_buffer;
          continue;
        }
#endif
#ifdef GLX_NV_present_video
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"present_video", 13))
        {
          ret = GLXEW_NV_present_video;
          continue;
        }
#endif
#ifdef GLX_NV_swap_group
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_group", 10))
        {
          ret = GLXEW_NV_swap_group;
          continue;
        }
#endif
#ifdef GLX_NV_vertex_array_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range", 18))
        {
          ret = GLXEW_NV_vertex_array_range;
          continue;
        }
#endif
#ifdef GLX_NV_video_output
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_output", 12))
        {
          ret = GLXEW_NV_video_output;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"OML_", 4))
      {
#ifdef GLX_OML_swap_method
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_method", 11))
        {
          ret = GLXEW_OML_swap_method;
          continue;
        }
#endif
#if defined(GLX_OML_sync_control) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <inttypes.h>
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sync_control", 12))
        {
          ret = GLXEW_OML_sync_control;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGIS_", 5))
      {
#ifdef GLX_SGIS_blended_overlay
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blended_overlay", 15))
        {
          ret = GLXEW_SGIS_blended_overlay;
          continue;
        }
#endif
#ifdef GLX_SGIS_color_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_range", 11))
        {
          ret = GLXEW_SGIS_color_range;
          continue;
        }
#endif
#ifdef GLX_SGIS_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLXEW_SGIS_multisample;
          continue;
        }
#endif
#ifdef GLX_SGIS_shared_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shared_multisample", 18))
        {
          ret = GLXEW_SGIS_shared_multisample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGIX_", 5))
      {
#ifdef GLX_SGIX_fbconfig
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fbconfig", 8))
        {
          ret = GLXEW_SGIX_fbconfig;
          continue;
        }
#endif
#ifdef GLX_SGIX_hyperpipe
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"hyperpipe", 9))
        {
          ret = GLXEW_SGIX_hyperpipe;
          continue;
        }
#endif
#ifdef GLX_SGIX_pbuffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pbuffer", 7))
        {
          ret = GLXEW_SGIX_pbuffer;
          continue;
        }
#endif
#ifdef GLX_SGIX_swap_barrier
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_barrier", 12))
        {
          ret = GLXEW_SGIX_swap_barrier;
          continue;
        }
#endif
#ifdef GLX_SGIX_swap_group
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_group", 10))
        {
          ret = GLXEW_SGIX_swap_group;
          continue;
        }
#endif
#ifdef GLX_SGIX_video_resize
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_resize", 12))
        {
          ret = GLXEW_SGIX_video_resize;
          continue;
        }
#endif
#ifdef GLX_SGIX_visual_select_group
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"visual_select_group", 19))
        {
          ret = GLXEW_SGIX_visual_select_group;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGI_", 4))
      {
#ifdef GLX_SGI_cushion
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cushion", 7))
        {
          ret = GLXEW_SGI_cushion;
          continue;
        }
#endif
#ifdef GLX_SGI_make_current_read
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"make_current_read", 17))
        {
          ret = GLXEW_SGI_make_current_read;
          continue;
        }
#endif
#ifdef GLX_SGI_swap_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_control", 12))
        {
          ret = GLXEW_SGI_swap_control;
          continue;
        }
#endif
#ifdef GLX_SGI_video_sync
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_sync", 10))
        {
          ret = GLXEW_SGI_video_sync;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SUN_", 4))
      {
#ifdef GLX_SUN_get_transparent_index
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"get_transparent_index", 21))
        {
          ret = GLXEW_SUN_get_transparent_index;
          continue;
        }
#endif
#ifdef GLX_SUN_video_resize
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_resize", 12))
        {
          ret = GLXEW_SUN_video_resize;
          continue;
        }
#endif
      }
    }
    ret = (len == 0);
  }
  return ret;
}

#endif /* _WIN32 */
