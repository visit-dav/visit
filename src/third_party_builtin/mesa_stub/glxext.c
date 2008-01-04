/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <GL/glx.h>
#include <GL/glxext.h>

__GLXextFuncPtr mglXGetProcAddressARB ( const GLubyte *procName )
{}

int mglXSwapIntervalSGI ( int x )
{}

int mglXGetVideoSyncSGI ( unsigned int *x )
{}
int mglXWaitVideoSyncSGI ( int x, int y, unsigned int *z )
{}

Bool mglXMakeCurrentReadSGI ( Display *dpy, GLXDrawable draw, GLXDrawable read,
                              GLXContext ctx )
{}
GLXDrawable mglXGetCurrentReadDrawableSGI ( void )
{}

void mglXDestroyGLXVideoSourceSGIX ( Display *dpy, GLXVideoSourceSGIX x )
{}

Display * mglXGetCurrentDisplayEXT ( void )
{}

int mglXQueryContextInfoEXT ( Display *dpy, GLXContext ctx, int x, int *y )
{}

GLXContextID mglXGetContextIDEXT ( const GLXContext ctx )
{}

GLXContext mglXImportContextEXT ( Display *dpy, GLXContextID ctx )
{}

void mglXFreeContextEXT ( Display *dpy, GLXContext ctx )
{}

int mglXGetFBConfigAttribSGIX ( Display *dpy, GLXFBConfigSGIX x, int y, int *z )
{}

GLXPixmap mglXCreateGLXPixmapWithConfigSGIX ( Display *dpy, GLXFBConfigSGIX x,
                                              Pixmap pixmap )
{}

GLXContext mglXCreateContextWithConfigSGIX ( Display *dpy, GLXFBConfigSGIX x,
                                             int y, GLXContext ctx, Bool z )
{}

XVisualInfo * mglXGetVisualFromFBConfigSGIX ( Display *dpy, GLXFBConfigSGIX x )
{}

GLXFBConfigSGIX mglXGetFBConfigFromVisualSGIX ( Display *dpy, XVisualInfo *x )
{}

GLXPbufferSGIX mglXCreateGLXPbufferSGIX ( Display *dpy, GLXFBConfigSGIX x,
                                          unsigned int y, unsigned int z,
                                          int *a )
{}

void mglXDestroyGLXPbufferSGIX ( Display *dpy, GLXPbufferSGIX x )
{}

int mglXQueryGLXPbufferSGIX ( Display *dpy, GLXPbufferSGIX x, int y,
                              unsigned int *z )
{}

void mglXSelectEventSGIX ( Display *dpy, GLXDrawable draw, unsigned long x )
{}

void mglXGetSelectedEventSGIX ( Display *dpy, GLXDrawable draw,
                                unsigned long *x )
{}

void mglXCushionSGI ( Display *dpy, Window win, float x )
{}

int mglXBindChannelToWindowSGIX ( Display *dpy, int x, int y, Window win )
{}

int mglXChannelRectSGIX ( Display *dpy, int x, int y, int z, int a, int b,
                          int c )
{}

int mglXQueryChannelRectSGIX ( Display *dpy, int x, int y, int *z,
                               int *a, int *b, int *c )
{}

int mglXQueryChannelDeltasSGIX ( Display *dpy, int x, int y, int *z,
                                 int *a, int *b, int *c )
{}

int mglXChannelRectSyncSGIX ( Display *dpy, int x, int y, GLenum z )
{}


void mglXJoinSwapGroupSGIX ( Display *dpy, GLXDrawable x, GLXDrawable y )
{}

void mglXBindSwapBarrierSGIX ( Display *dpy, GLXDrawable draw, int x )
{}

Bool mglXQueryMaxSwapBarriersSGIX ( Display *dpy, int x, int *y )
{}

Status mglXGetTransparentIndexSUN ( Display *dpy, Window x, Window y, long *z )
{}

void mglXCopySubBufferMESA ( Display *dpy, GLXDrawable draw,
                             int x, int y, int z, int a )
{}

GLXPixmap mglXCreateGLXPixmapMESA ( Display *dpy, XVisualInfo *x,
                                    Pixmap pixmap, Colormap y )
{}

Bool mglXReleaseBuffersMESA ( Display *dpy, GLXDrawable draw )
{}

Bool mglXSet3DfxModeMESA ( int x )
{}

#if defined(__STDC_VERSION__)
#if __STDC_VERSION__ >= 199901L
/* Include ISO C99 integer types for OML_sync_control; need a better test */
#include <inttypes.h>

#ifndef GLX_OML_sync_control
#define GLX_OML_sync_control 1
#ifdef GLX_GLXEXT_PROTOTYPES
Bool mglXGetSyncValuesOML ( Display *dpy, GLXDrawable draw,
                            int64_t *x, int64_t *y, int64_t *z )
{}

Bool mglXGetMscRateOML ( Display *dpy, GLXDrawable draw,
                         int32_t *x, int32_t *y )
{}

int64_t mglXSwapBuffersMscOML ( Display *dpy, GLXDrawable draw,
                                int64_t x, int64_t y, int64_t z )
{}

Bool mglXWaitForMscOML ( Display *dpy, GLXDrawable draw,
                         int64_t x, int64_t y, int64_t z,
                         int64_t *a, int64_t *b, int64_t *c )
{}

Bool mglXWaitForSbcOML ( Display *dpy, GLXDrawable draw,
                         int64_t x, int64_t *y, int64_t *z, int64_t *a )
{}
#endif /* GLX_GLXEXT_PROTOTYPES */
#endif

#endif /* C99 version test */
#endif /* STDC test */
