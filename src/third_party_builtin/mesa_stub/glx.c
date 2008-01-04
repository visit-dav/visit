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

XVisualInfo* mglXChooseVisual( Display *dpy, int screen,
                               int *attribList )
{}

GLXContext mglXCreateContext( Display *dpy, XVisualInfo *vis,
                              GLXContext shareList, Bool direct )
{}

void mglXDestroyContext( Display *dpy, GLXContext ctx )
{}

Bool mglXMakeCurrent( Display *dpy, GLXDrawable drawable,
                      GLXContext ctx)
{}

void mglXCopyContext( Display *dpy, GLXContext src, GLXContext dst,
                      unsigned long mask )
{}

void mglXSwapBuffers( Display *dpy, GLXDrawable drawable )
{}

GLXPixmap mglXCreateGLXPixmap( Display *dpy, XVisualInfo *visual,
                               Pixmap pixmap )
{}

void mglXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap )
{}

Bool mglXQueryExtension( Display *dpy, int *errorb, int *event )
{}

Bool mglXQueryVersion( Display *dpy, int *maj, int *min )
{}

Bool mglXIsDirect( Display *dpy, GLXContext ctx )
{}

int mglXGetConfig( Display *dpy, XVisualInfo *visual,
                   int attrib, int *value )
{}

GLXContext mglXGetCurrentContext( void )
{}

GLXDrawable mglXGetCurrentDrawable( void )
{}

void mglXWaitGL( void )
{}

void mglXWaitX( void )
{}

void mglXUseXFont( Font font, int first, int count, int list )
{}


/* GLX 1.1 and later */
const char *mglXQueryExtensionsString( Display *dpy, int screen )
{}

const char *mglXQueryServerString( Display *dpy, int screen, int name )
{}

const char *mglXGetClientString( Display *dpy, int name )
{}


/* GLX 1.2 and later */
Display *mglXGetCurrentDisplay( void )
{}


/* GLX 1.3 and later */
GLXFBConfig *mglXChooseFBConfig( Display *dpy, int screen,
                                 const int *attribList, int *nitems )
{}

int mglXGetFBConfigAttrib( Display *dpy, GLXFBConfig config,
                           int attribute, int *value )
{}

GLXFBConfig *mglXGetFBConfigs( Display *dpy, int screen,
                               int *nelements )
{}

XVisualInfo *mglXGetVisualFromFBConfig( Display *dpy,
                                        GLXFBConfig config )
{}

GLXWindow mglXCreateWindow( Display *dpy, GLXFBConfig config,
                            Window win, const int *attribList )
{}

void mglXDestroyWindow( Display *dpy, GLXWindow window )
{}

GLXPixmap mglXCreatePixmap( Display *dpy, GLXFBConfig config,
                            Pixmap pixmap, const int *attribList )
{}

void mglXDestroyPixmap( Display *dpy, GLXPixmap pixmap )
{}

GLXPbuffer mglXCreatePbuffer( Display *dpy, GLXFBConfig config,
                              const int *attribList )
{}

void mglXDestroyPbuffer( Display *dpy, GLXPbuffer pbuf )
{}

void mglXQueryDrawable( Display *dpy, GLXDrawable draw, int attribute,
                        unsigned int *value )
{}

GLXContext mglXCreateNewContext( Display *dpy, GLXFBConfig config,
                                 int renderType, GLXContext shareList,
                                 Bool direct )
{}

Bool mglXMakeContextCurrent( Display *dpy, GLXDrawable draw,
                             GLXDrawable read, GLXContext ctx )
{}

GLXDrawable mglXGetCurrentReadDrawable( void )
{}

int mglXQueryContext( Display *dpy, GLXContext ctx, int attribute,
                      int *value )
{}

void mglXSelectEvent( Display *dpy, GLXDrawable drawable,
                      unsigned long mask )
{}

void mglXGetSelectedEvent( Display *dpy, GLXDrawable drawable,
                           unsigned long *mask )
{}


/* GLX 1.4 and later */
void (*mglXGetProcAddress( const GLubyte *procname ))()
{}



/**
 ** The following aren't in mglxext.h yet.
 **/

void *mglXAllocateMemoryNV( GLsizei size, GLfloat readfreq, GLfloat writefreq,
                            GLfloat priority )
{}

void mglXFreeMemoryNV( GLvoid *pointer )
{}

GLuint mglXGetAGPOffsetMESA( const GLvoid *pointer )
{}
