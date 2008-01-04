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

#include <GL/gl.h>

void mglClearIndex( GLfloat c )
{}

void mglClearColor( GLclampf red, GLclampf green, GLclampf blue,
                    GLclampf alpha )
{}

void mglClear( GLbitfield mask )
{}

void mglIndexMask( GLuint mask )
{}

void mglColorMask( GLboolean red, GLboolean green, GLboolean blue,
                   GLboolean alpha )
{}

void mglAlphaFunc( GLenum func, GLclampf ref )
{}

void mglBlendFunc( GLenum sfactor, GLenum dfactor )
{}

void mglLogicOp( GLenum opcode )
{}

void mglCullFace( GLenum mode )
{}

void mglFrontFace( GLenum mode )
{}

void mglPointSize( GLfloat size )
{}

void mglLineWidth( GLfloat width )
{}

void mglLineStipple( GLint factor, GLushort pattern )
{}

void mglPolygonMode( GLenum face, GLenum mode )
{}

void mglPolygonOffset( GLfloat factor, GLfloat units )
{}

void mglPolygonStipple( const GLubyte *mask )
{}

void mglGetPolygonStipple( GLubyte *mask )
{}

void mglEdgeFlag( GLboolean flag )
{}

void mglEdgeFlagv( const GLboolean *flag )
{}

void mglScissor( GLint x, GLint y, GLsizei width, GLsizei height)
{}

void mglClipPlane( GLenum plane, const GLdouble *equation )
{}

void mglGetClipPlane( GLenum plane, GLdouble *equation )
{}

void mglDrawBuffer( GLenum mode )
{}

void mglReadBuffer( GLenum mode )
{}

void mglEnable( GLenum cap )
{}

void mglDisable( GLenum cap )
{}

GLboolean mglIsEnabled( GLenum cap )
{}


void mglEnableClientState( GLenum cap )  /* 1.1 */
{}

void mglDisableClientState( GLenum cap )  /* 1.1 */
{}


void mglGetBooleanv( GLenum pname, GLboolean *params )
{}

void mglGetDoublev( GLenum pname, GLdouble *params )
{}

void mglGetFloatv( GLenum pname, GLfloat *params )
{}

void mglGetIntegerv( GLenum pname, GLint *params )
{}


void mglPushAttrib( GLbitfield mask )
{}

void mglPopAttrib( void )
{}


void mglPushClientAttrib( GLbitfield mask )  /* 1.1 */
{}

void mglPopClientAttrib( void )  /* 1.1 */
{}


GLint mglRenderMode( GLenum mode )
{}

GLenum mglGetError( void )
{}

const GLubyte* mglGetString( GLenum name )
{}

void mglFinish( void )
{}

void mglFlush( void )
{}

void mglHint( GLenum target, GLenum mode )
{}


/*
 * Depth Buffer
 */

void mglClearDepth( GLclampd depth )
{}

void mglDepthFunc( GLenum func )
{}

void mglDepthMask( GLboolean flag )
{}

void mglDepthRange( GLclampd near_val, GLclampd far_val )
{}


/*
 * Accumulation Buffer
 */

void mglClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{}

void mglAccum( GLenum op, GLfloat value )
{}


/*
 * Transformation
 */

void mglMatrixMode( GLenum mode )
{}

void mglOrtho( GLdouble left, GLdouble right,
               GLdouble bottom, GLdouble top,
               GLdouble near_val, GLdouble far_val )
{}

void mglFrustum( GLdouble left, GLdouble right,
                 GLdouble bottom, GLdouble top,
                 GLdouble near_val, GLdouble far_val )
{}

void mglViewport( GLint x, GLint y,
                  GLsizei width, GLsizei height )
{}

void mglPushMatrix( void )
{}

void mglPopMatrix( void )
{}

void mglLoadIdentity( void )
{}

void mglLoadMatrixd( const GLdouble *m )
{}
void mglLoadMatrixf( const GLfloat *m )
{}

void mglMultMatrixd( const GLdouble *m )
{}
void mglMultMatrixf( const GLfloat *m )
{}

void mglRotated( GLdouble angle,
                 GLdouble x, GLdouble y, GLdouble z )
{}
void mglRotatef( GLfloat angle,
                 GLfloat x, GLfloat y, GLfloat z )
{}

void mglScaled( GLdouble x, GLdouble y, GLdouble z )
{}
void mglScalef( GLfloat x, GLfloat y, GLfloat z )
{}

void mglTranslated( GLdouble x, GLdouble y, GLdouble z )
{}
void mglTranslatef( GLfloat x, GLfloat y, GLfloat z )
{}


/*
 * Display Lists
 */

GLboolean mglIsList( GLuint list )
{}

void mglDeleteLists( GLuint list, GLsizei range )
{}

GLuint mglGenLists( GLsizei range )
{}

void mglNewList( GLuint list, GLenum mode )
{}

void mglEndList( void )
{}

void mglCallList( GLuint list )
{}

void mglCallLists( GLsizei n, GLenum type,
                   const GLvoid *lists )
{}

void mglListBase( GLuint base )
{}


/*
 * Drawing Functions
 */

void mglBegin( GLenum mode )
{}

void mglEnd( void )
{}


void mglVertex2d( GLdouble x, GLdouble y )
{}
void mglVertex2f( GLfloat x, GLfloat y )
{}
void mglVertex2i( GLint x, GLint y )
{}
void mglVertex2s( GLshort x, GLshort y )
{}

void mglVertex3d( GLdouble x, GLdouble y, GLdouble z )
{}
void mglVertex3f( GLfloat x, GLfloat y, GLfloat z )
{}
void mglVertex3i( GLint x, GLint y, GLint z )
{}
void mglVertex3s( GLshort x, GLshort y, GLshort z )
{}

void mglVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{}
void mglVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{}
void mglVertex4i( GLint x, GLint y, GLint z, GLint w )
{}
void mglVertex4s( GLshort x, GLshort y, GLshort z, GLshort w )
{}

void mglVertex2dv( const GLdouble *v )
{}
void mglVertex2fv( const GLfloat *v )
{}
void mglVertex2iv( const GLint *v )
{}
void mglVertex2sv( const GLshort *v )
{}

void mglVertex3dv( const GLdouble *v )
{}
void mglVertex3fv( const GLfloat *v )
{}
void mglVertex3iv( const GLint *v )
{}
void mglVertex3sv( const GLshort *v )
{}

void mglVertex4dv( const GLdouble *v )
{}
void mglVertex4fv( const GLfloat *v )
{}
void mglVertex4iv( const GLint *v )
{}
void mglVertex4sv( const GLshort *v )
{}


void mglNormal3b( GLbyte nx, GLbyte ny, GLbyte nz )
{}
void mglNormal3d( GLdouble nx, GLdouble ny, GLdouble nz )
{}
void mglNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{}
void mglNormal3i( GLint nx, GLint ny, GLint nz )
{}
void mglNormal3s( GLshort nx, GLshort ny, GLshort nz )
{}

void mglNormal3bv( const GLbyte *v )
{}
void mglNormal3dv( const GLdouble *v )
{}
void mglNormal3fv( const GLfloat *v )
{}
void mglNormal3iv( const GLint *v )
{}
void mglNormal3sv( const GLshort *v )
{}


void mglIndexd( GLdouble c )
{}
void mglIndexf( GLfloat c )
{}
void mglIndexi( GLint c )
{}
void mglIndexs( GLshort c )
{}
void mglIndexub( GLubyte c )  /* 1.1 */
{}

void mglIndexdv( const GLdouble *c )
{}
void mglIndexfv( const GLfloat *c )
{}
void mglIndexiv( const GLint *c )
{}
void mglIndexsv( const GLshort *c )
{}
void mglIndexubv( const GLubyte *c )  /* 1.1 */
{}

void mglColor3b( GLbyte red, GLbyte green, GLbyte blue )
{}
void mglColor3d( GLdouble red, GLdouble green, GLdouble blue )
{}
void mglColor3f( GLfloat red, GLfloat green, GLfloat blue )
{}
void mglColor3i( GLint red, GLint green, GLint blue )
{}
void mglColor3s( GLshort red, GLshort green, GLshort blue )
{}
void mglColor3ub( GLubyte red, GLubyte green, GLubyte blue )
{}
void mglColor3ui( GLuint red, GLuint green, GLuint blue )
{}
void mglColor3us( GLushort red, GLushort green, GLushort blue )
{}

void mglColor4b( GLbyte red, GLbyte green,
                 GLbyte blue, GLbyte alpha )
{}
void mglColor4d( GLdouble red, GLdouble green,
                 GLdouble blue, GLdouble alpha )
{}
void mglColor4f( GLfloat red, GLfloat green,
                 GLfloat blue, GLfloat alpha )
{}
void mglColor4i( GLint red, GLint green,
                 GLint blue, GLint alpha )
{}
void mglColor4s( GLshort red, GLshort green,
                 GLshort blue, GLshort alpha )
{}
void mglColor4ub( GLubyte red, GLubyte green,
                  GLubyte blue, GLubyte alpha )
{}
void mglColor4ui( GLuint red, GLuint green,
                  GLuint blue, GLuint alpha )
{}
void mglColor4us( GLushort red, GLushort green,
                  GLushort blue, GLushort alpha )
{}


void mglColor3bv( const GLbyte *v )
{}
void mglColor3dv( const GLdouble *v )
{}
void mglColor3fv( const GLfloat *v )
{}
void mglColor3iv( const GLint *v )
{}
void mglColor3sv( const GLshort *v )
{}
void mglColor3ubv( const GLubyte *v )
{}
void mglColor3uiv( const GLuint *v )
{}
void mglColor3usv( const GLushort *v )
{}

void mglColor4bv( const GLbyte *v )
{}
void mglColor4dv( const GLdouble *v )
{}
void mglColor4fv( const GLfloat *v )
{}
void mglColor4iv( const GLint *v )
{}
void mglColor4sv( const GLshort *v )
{}
void mglColor4ubv( const GLubyte *v )
{}
void mglColor4uiv( const GLuint *v )
{}
void mglColor4usv( const GLushort *v )
{}


void mglTexCoord1d( GLdouble s )
{}
void mglTexCoord1f( GLfloat s )
{}
void mglTexCoord1i( GLint s )
{}
void mglTexCoord1s( GLshort s )
{}

void mglTexCoord2d( GLdouble s, GLdouble t )
{}
void mglTexCoord2f( GLfloat s, GLfloat t )
{}
void mglTexCoord2i( GLint s, GLint t )
{}
void mglTexCoord2s( GLshort s, GLshort t )
{}

void mglTexCoord3d( GLdouble s, GLdouble t, GLdouble r )
{}
void mglTexCoord3f( GLfloat s, GLfloat t, GLfloat r )
{}
void mglTexCoord3i( GLint s, GLint t, GLint r )
{}
void mglTexCoord3s( GLshort s, GLshort t, GLshort r )
{}

void mglTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{}
void mglTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{}
void mglTexCoord4i( GLint s, GLint t, GLint r, GLint q )
{}
void mglTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q )
{}

void mglTexCoord1dv( const GLdouble *v )
{}
void mglTexCoord1fv( const GLfloat *v )
{}
void mglTexCoord1iv( const GLint *v )
{}
void mglTexCoord1sv( const GLshort *v )
{}

void mglTexCoord2dv( const GLdouble *v )
{}
void mglTexCoord2fv( const GLfloat *v )
{}
void mglTexCoord2iv( const GLint *v )
{}
void mglTexCoord2sv( const GLshort *v )
{}

void mglTexCoord3dv( const GLdouble *v )
{}
void mglTexCoord3fv( const GLfloat *v )
{}
void mglTexCoord3iv( const GLint *v )
{}
void mglTexCoord3sv( const GLshort *v )
{}

void mglTexCoord4dv( const GLdouble *v )
{}
void mglTexCoord4fv( const GLfloat *v )
{}
void mglTexCoord4iv( const GLint *v )
{}
void mglTexCoord4sv( const GLshort *v )
{}


void mglRasterPos2d( GLdouble x, GLdouble y )
{}
void mglRasterPos2f( GLfloat x, GLfloat y )
{}
void mglRasterPos2i( GLint x, GLint y )
{}
void mglRasterPos2s( GLshort x, GLshort y )
{}

void mglRasterPos3d( GLdouble x, GLdouble y, GLdouble z )
{}
void mglRasterPos3f( GLfloat x, GLfloat y, GLfloat z )
{}
void mglRasterPos3i( GLint x, GLint y, GLint z )
{}
void mglRasterPos3s( GLshort x, GLshort y, GLshort z )
{}

void mglRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{}
void mglRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{}
void mglRasterPos4i( GLint x, GLint y, GLint z, GLint w )
{}
void mglRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w )
{}

void mglRasterPos2dv( const GLdouble *v )
{}
void mglRasterPos2fv( const GLfloat *v )
{}
void mglRasterPos2iv( const GLint *v )
{}
void mglRasterPos2sv( const GLshort *v )
{}

void mglRasterPos3dv( const GLdouble *v )
{}
void mglRasterPos3fv( const GLfloat *v )
{}
void mglRasterPos3iv( const GLint *v )
{}
void mglRasterPos3sv( const GLshort *v )
{}

void mglRasterPos4dv( const GLdouble *v )
{}
void mglRasterPos4fv( const GLfloat *v )
{}
void mglRasterPos4iv( const GLint *v )
{}
void mglRasterPos4sv( const GLshort *v )
{}


void mglRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{}
void mglRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{}
void mglRecti( GLint x1, GLint y1, GLint x2, GLint y2 )
{}
void mglRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{}


void mglRectdv( const GLdouble *v1, const GLdouble *v2 )
{}
void mglRectfv( const GLfloat *v1, const GLfloat *v2 )
{}
void mglRectiv( const GLint *v1, const GLint *v2 )
{}
void mglRectsv( const GLshort *v1, const GLshort *v2 )
{}


/*
 * Vertex Arrays  (1.1)
 */

void mglVertexPointer( GLint size, GLenum type,
                       GLsizei stride, const GLvoid *ptr )
{}

void mglNormalPointer( GLenum type, GLsizei stride,
                       const GLvoid *ptr )
{}

void mglColorPointer( GLint size, GLenum type,
                      GLsizei stride, const GLvoid *ptr )
{}

void mglIndexPointer( GLenum type, GLsizei stride,
                      const GLvoid *ptr )
{}

void mglTexCoordPointer( GLint size, GLenum type,
                         GLsizei stride, const GLvoid *ptr )
{}

void mglEdgeFlagPointer( GLsizei stride, const GLvoid *ptr )
{}

void mglGetPointerv( GLenum pname, GLvoid **params )
{}

void mglArrayElement( GLint i )
{}

void mglDrawArrays( GLenum mode, GLint first, GLsizei count )
{}

void mglDrawElements( GLenum mode, GLsizei count,
                      GLenum type, const GLvoid *indices )
{}

void mglInterleavedArrays( GLenum format, GLsizei stride,
                           const GLvoid *pointer )
{}

/*
 * Lighting
 */

void mglShadeModel( GLenum mode )
{}

void mglLightf( GLenum light, GLenum pname, GLfloat param )
{}
void mglLighti( GLenum light, GLenum pname, GLint param )
{}
void mglLightfv( GLenum light, GLenum pname,
                 const GLfloat *params )
{}
void mglLightiv( GLenum light, GLenum pname,
                 const GLint *params )
{}

void mglGetLightfv( GLenum light, GLenum pname,
                    GLfloat *params )
{}
void mglGetLightiv( GLenum light, GLenum pname,
                    GLint *params )
{}

void mglLightModelf( GLenum pname, GLfloat param )
{}
void mglLightModeli( GLenum pname, GLint param )
{}
void mglLightModelfv( GLenum pname, const GLfloat *params )
{}
void mglLightModeliv( GLenum pname, const GLint *params )
{}

void mglMaterialf( GLenum face, GLenum pname, GLfloat param )
{}
void mglMateriali( GLenum face, GLenum pname, GLint param )
{}
void mglMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{}
void mglMaterialiv( GLenum face, GLenum pname, const GLint *params )
{}

void mglGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{}
void mglGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{}

void mglColorMaterial( GLenum face, GLenum mode )
{}


/*
 * Raster functions
 */

void mglPixelZoom( GLfloat xfactor, GLfloat yfactor )
{}

void mglPixelStoref( GLenum pname, GLfloat param )
{}
void mglPixelStorei( GLenum pname, GLint param )
{}

void mglPixelTransferf( GLenum pname, GLfloat param )
{}
void mglPixelTransferi( GLenum pname, GLint param )
{}

void mglPixelMapfv( GLenum map, GLint mapsize,
                    const GLfloat *values )
{}
void mglPixelMapuiv( GLenum map, GLint mapsize,
                     const GLuint *values )
{}
void mglPixelMapusv( GLenum map, GLint mapsize,
                     const GLushort *values )
{}

void mglGetPixelMapfv( GLenum map, GLfloat *values )
{}
void mglGetPixelMapuiv( GLenum map, GLuint *values )
{}
void mglGetPixelMapusv( GLenum map, GLushort *values )
{}

void mglBitmap( GLsizei width, GLsizei height,
                GLfloat xorig, GLfloat yorig,
                GLfloat xmove, GLfloat ymove,
                const GLubyte *bitmap )
{}

void mglReadPixels( GLint x, GLint y,
                    GLsizei width, GLsizei height,
                    GLenum format, GLenum type,
                    GLvoid *pixels )
{}

void mglDrawPixels( GLsizei width, GLsizei height,
                    GLenum format, GLenum type,
                    const GLvoid *pixels )
{}

void mglCopyPixels( GLint x, GLint y,
                    GLsizei width, GLsizei height,
                    GLenum type )
{}

/*
 * Stenciling
 */

void mglStencilFunc( GLenum func, GLint ref, GLuint mask )
{}

void mglStencilMask( GLuint mask )
{}

void mglStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{}

void mglClearStencil( GLint s )
{}



/*
 * Texture mapping
 */

void mglTexGend( GLenum coord, GLenum pname, GLdouble param )
{}
void mglTexGenf( GLenum coord, GLenum pname, GLfloat param )
{}
void mglTexGeni( GLenum coord, GLenum pname, GLint param )
{}

void mglTexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{}
void mglTexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{}
void mglTexGeniv( GLenum coord, GLenum pname, const GLint *params )
{}

void mglGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{}
void mglGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{}
void mglGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{}


void mglTexEnvf( GLenum target, GLenum pname, GLfloat param )
{}
void mglTexEnvi( GLenum target, GLenum pname, GLint param )
{}

void mglTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{}
void mglTexEnviv( GLenum target, GLenum pname, const GLint *params )
{}

void mglGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{}
void mglGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{}


void mglTexParameterf( GLenum target, GLenum pname, GLfloat param )
{}
void mglTexParameteri( GLenum target, GLenum pname, GLint param )
{}

void mglTexParameterfv( GLenum target, GLenum pname,
                        const GLfloat *params )
{}
void mglTexParameteriv( GLenum target, GLenum pname,
                        const GLint *params )
{}

void mglGetTexParameterfv( GLenum target,
                           GLenum pname, GLfloat *params)
{}
void mglGetTexParameteriv( GLenum target,
                           GLenum pname, GLint *params )
{}

void mglGetTexLevelParameterfv( GLenum target, GLint level,
                                GLenum pname, GLfloat *params )
{}
void mglGetTexLevelParameteriv( GLenum target, GLint level,
                                GLenum pname, GLint *params )
{}


void mglTexImage1D( GLenum target, GLint level,
                    GLint internalFormat,
                    GLsizei width, GLint border,
                    GLenum format, GLenum type,
                    const GLvoid *pixels )
{}

void mglTexImage2D( GLenum target, GLint level,
                    GLint internalFormat,
                    GLsizei width, GLsizei height,
                    GLint border, GLenum format, GLenum type,
                    const GLvoid *pixels )
{}

void mglGetTexImage( GLenum target, GLint level,
                     GLenum format, GLenum type,
                     GLvoid *pixels )
{}


/* 1.1 functions */

void mglGenTextures( GLsizei n, GLuint *textures )
{}

void mglDeleteTextures( GLsizei n, const GLuint *textures)
{}

void mglBindTexture( GLenum target, GLuint texture )
{}

void mglPrioritizeTextures( GLsizei n,
                            const GLuint *textures,
                            const GLclampf *priorities )
{}

GLboolean mglAreTexturesResident( GLsizei n,
                                  const GLuint *textures,
                                  GLboolean *residences )
{}

GLboolean mglIsTexture( GLuint texture )
{}


void mglTexSubImage1D( GLenum target, GLint level,
                       GLint xoffset,
                       GLsizei width, GLenum format,
                       GLenum type, const GLvoid *pixels )
{}


void mglTexSubImage2D( GLenum target, GLint level,
                       GLint xoffset, GLint yoffset,
                       GLsizei width, GLsizei height,
                       GLenum format, GLenum type,
                       const GLvoid *pixels )
{}


void mglCopyTexImage1D( GLenum target, GLint level,
                        GLenum internalformat,
                        GLint x, GLint y,
                        GLsizei width, GLint border )
{}


void mglCopyTexImage2D( GLenum target, GLint level,
                        GLenum internalformat,
                        GLint x, GLint y,
                        GLsizei width, GLsizei height,
                        GLint border )
{}


void mglCopyTexSubImage1D( GLenum target, GLint level,
                           GLint xoffset, GLint x, GLint y,
                           GLsizei width )
{}


void mglCopyTexSubImage2D( GLenum target, GLint level,
                           GLint xoffset, GLint yoffset,
                           GLint x, GLint y,
                           GLsizei width, GLsizei height )
{}


/*
 * Evaluators
 */

void mglMap1d( GLenum target, GLdouble u1, GLdouble u2,
               GLint stride,
               GLint order, const GLdouble *points )
{}
void mglMap1f( GLenum target, GLfloat u1, GLfloat u2,
               GLint stride,
               GLint order, const GLfloat *points )
{}

void mglMap2d( GLenum target,
               GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
               GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
               const GLdouble *points )
{}
void mglMap2f( GLenum target,
               GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
               GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
               const GLfloat *points )
{}

void mglGetMapdv( GLenum target, GLenum query, GLdouble *v )
{}
void mglGetMapfv( GLenum target, GLenum query, GLfloat *v )
{}
void mglGetMapiv( GLenum target, GLenum query, GLint *v )
{}

void mglEvalCoord1d( GLdouble u )
{}
void mglEvalCoord1f( GLfloat u )
{}

void mglEvalCoord1dv( const GLdouble *u )
{}
void mglEvalCoord1fv( const GLfloat *u )
{}

void mglEvalCoord2d( GLdouble u, GLdouble v )
{}
void mglEvalCoord2f( GLfloat u, GLfloat v )
{}

void mglEvalCoord2dv( const GLdouble *u )
{}
void mglEvalCoord2fv( const GLfloat *u )
{}

void mglMapGrid1d( GLint un, GLdouble u1, GLdouble u2 )
{}
void mglMapGrid1f( GLint un, GLfloat u1, GLfloat u2 )
{}

void mglMapGrid2d( GLint un, GLdouble u1, GLdouble u2,
                   GLint vn, GLdouble v1, GLdouble v2 )
{}
void mglMapGrid2f( GLint un, GLfloat u1, GLfloat u2,
                   GLint vn, GLfloat v1, GLfloat v2 )
{}

void mglEvalPoint1( GLint i )
{}

void mglEvalPoint2( GLint i, GLint j )
{}

void mglEvalMesh1( GLenum mode, GLint i1, GLint i2 )
{}

void mglEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{}


/*
 * Fog
 */

void mglFogf( GLenum pname, GLfloat param )
{}

void mglFogi( GLenum pname, GLint param )
{}

void mglFogfv( GLenum pname, const GLfloat *params )
{}

void mglFogiv( GLenum pname, const GLint *params )
{}


/*
 * Selection and Feedback
 */

void mglFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer )
{}

void mglPassThrough( GLfloat token )
{}

void mglSelectBuffer( GLsizei size, GLuint *buffer )
{}

void mglInitNames( void )
{}

void mglLoadName( GLuint name )
{}

void mglPushName( GLuint name )
{}

void mglPopName( void )
{}



void mglDrawRangeElements( GLenum mode, GLuint start,
                           GLuint end, GLsizei count,
                           GLenum type, const GLvoid *indices )
{}

void mglTexImage3D( GLenum target, GLint level,
                    GLint internalFormat,
                    GLsizei width, GLsizei height,
                    GLsizei depth, GLint border,
                    GLenum format, GLenum type,
                    const GLvoid *pixels )
{}

void mglTexSubImage3D( GLenum target, GLint level,
                       GLint xoffset, GLint yoffset,
                       GLint zoffset, GLsizei width,
                       GLsizei height, GLsizei depth,
                       GLenum format,
                       GLenum type, const GLvoid *pixels)
{}

void mglCopyTexSubImage3D( GLenum target, GLint level,
                           GLint xoffset, GLint yoffset,
                           GLint zoffset, GLint x,
                           GLint y, GLsizei width,
                           GLsizei height )
{}

/*
 * GL_ARB_imaging
 */

void mglColorTable( GLenum target, GLenum internalformat,
                    GLsizei width, GLenum format,
                    GLenum type, const GLvoid *table )
{}

void mglColorSubTable( GLenum target,
                       GLsizei start, GLsizei count,
                       GLenum format, GLenum type,
                       const GLvoid *data )
{}

void mglColorTableParameteriv(GLenum target, GLenum pname,
                              const GLint *params)
{}

void mglColorTableParameterfv(GLenum target, GLenum pname,
                              const GLfloat *params)
{}

void mglCopyColorSubTable( GLenum target, GLsizei start,
                           GLint x, GLint y, GLsizei width )
{}

void mglCopyColorTable( GLenum target, GLenum internalformat,
                        GLint x, GLint y, GLsizei width )
{}

void mglGetColorTable( GLenum target, GLenum format,
                       GLenum type, GLvoid *table )
{}

void mglGetColorTableParameterfv( GLenum target, GLenum pname,
                                  GLfloat *params )
{}

void mglGetColorTableParameteriv( GLenum target, GLenum pname,
                                  GLint *params )
{}

void mglBlendEquation( GLenum mode )
{}

void mglBlendColor( GLclampf red, GLclampf green,
                    GLclampf blue, GLclampf alpha )
{}

void mglHistogram( GLenum target, GLsizei width,
       GLenum internalformat, GLboolean sink )
{}

void mglResetHistogram( GLenum target )
{}

void mglGetHistogram( GLenum target, GLboolean reset,
                      GLenum format, GLenum type,
                      GLvoid *values )
{}

void mglGetHistogramParameterfv( GLenum target, GLenum pname,
                                 GLfloat *params )
{}

void mglGetHistogramParameteriv( GLenum target, GLenum pname,
                                 GLint *params )
{}

void mglMinmax( GLenum target, GLenum internalformat,
                GLboolean sink )
{}

void mglResetMinmax( GLenum target )
{}

void mglGetMinmax( GLenum target, GLboolean reset,
                   GLenum format, GLenum types,
                   GLvoid *values )
{}

void mglGetMinmaxParameterfv( GLenum target, GLenum pname,
                              GLfloat *params )
{}

void mglGetMinmaxParameteriv( GLenum target, GLenum pname,
                              GLint *params )
{}

void mglConvolutionFilter1D( GLenum target,
                             GLenum internalformat, GLsizei width,
                             GLenum format, GLenum type,
                             const GLvoid *image )
{}

void mglConvolutionFilter2D( GLenum target,
                             GLenum internalformat, GLsizei width,
                             GLsizei height, GLenum format,
                             GLenum type, const GLvoid *image )
{}

void mglConvolutionParameterf( GLenum target, GLenum pname,
                               GLfloat params )
{}

void mglConvolutionParameterfv( GLenum target, GLenum pname,
                                const GLfloat *params )
{}

void mglConvolutionParameteri( GLenum target, GLenum pname,
                               GLint params )
{}

void mglConvolutionParameteriv( GLenum target, GLenum pname,
                                const GLint *params )
{}

void mglCopyConvolutionFilter1D( GLenum target,
                                 GLenum internalformat, GLint x, GLint y,
                                 GLsizei width )
{}

void mglCopyConvolutionFilter2D( GLenum target,
                                 GLenum internalformat, GLint x, GLint y,
                                 GLsizei width, GLsizei height)
{}

void mglGetConvolutionFilter( GLenum target, GLenum format,
                              GLenum type, GLvoid *image )
{}

void mglGetConvolutionParameterfv( GLenum target, GLenum pname,
                                   GLfloat *params )
{}

void mglGetConvolutionParameteriv( GLenum target, GLenum pname,
                                   GLint *params )
{}

void mglSeparableFilter2D( GLenum target,
                           GLenum internalformat, GLsizei width,
                           GLsizei height, GLenum format,
                           GLenum type,
                           const GLvoid *row, const GLvoid *column )
{}

void mglGetSeparableFilter( GLenum target, GLenum format,
                            GLenum type, GLvoid *row, GLvoid *column,
                            GLvoid *span )
{}

/*
 * OpenGL 1.3
 */

void mglActiveTexture( GLenum texture )
{}

void mglClientActiveTexture( GLenum texture )
{}

void mglCompressedTexImage1D( GLenum target, GLint level,
                              GLenum internalformat, GLsizei width,
                              GLint border, GLsizei imageSize,
                              const GLvoid *data )
{}

void mglCompressedTexImage2D( GLenum target, GLint level,
                              GLenum internalformat,
                              GLsizei width, GLsizei height,
                              GLint border, GLsizei imageSize,
                              const GLvoid *data )
{}

void mglCompressedTexImage3D( GLenum target, GLint level,
                              GLenum internalformat,
                              GLsizei width, GLsizei height,
                              GLsizei depth, GLint border,
                              GLsizei imageSize, const GLvoid *data )
{}

void mglCompressedTexSubImage1D( GLenum target, GLint level, GLint xoffset,
                                 GLsizei width, GLenum format,
                                 GLsizei imageSize, const GLvoid *data )
{}

void mglCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLsizei width, GLsizei height,
                                 GLenum format,
                                 GLsizei imageSize, const GLvoid *data )
{}

void mglCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLint zoffset,
                                 GLsizei width, GLsizei height,
                                 GLsizei depth, GLenum format,
                                 GLsizei imageSize, const GLvoid *data )
{}

void mglGetCompressedTexImage( GLenum target, GLint lod, GLvoid *img )
{}

void mglMultiTexCoord1d( GLenum target, GLdouble s )
{}

void mglMultiTexCoord1dv( GLenum target, const GLdouble *v )
{}

void mglMultiTexCoord1f( GLenum target, GLfloat s )
{}

void mglMultiTexCoord1fv( GLenum target, const GLfloat *v )
{}

void mglMultiTexCoord1i( GLenum target, GLint s )
{}

void mglMultiTexCoord1iv( GLenum target, const GLint *v )
{}

void mglMultiTexCoord1s( GLenum target, GLshort s )
{}

void mglMultiTexCoord1sv( GLenum target, const GLshort *v )
{}

void mglMultiTexCoord2d( GLenum target, GLdouble s, GLdouble t )
{}

void mglMultiTexCoord2dv( GLenum target, const GLdouble *v )
{}

void mglMultiTexCoord2f( GLenum target, GLfloat s, GLfloat t )
{}

void mglMultiTexCoord2fv( GLenum target, const GLfloat *v )
{}

void mglMultiTexCoord2i( GLenum target, GLint s, GLint t )
{}

void mglMultiTexCoord2iv( GLenum target, const GLint *v )
{}

void mglMultiTexCoord2s( GLenum target, GLshort s, GLshort t )
{}

void mglMultiTexCoord2sv( GLenum target, const GLshort *v )
{}

void mglMultiTexCoord3d( GLenum target, GLdouble s, GLdouble t, GLdouble r )
{}

void mglMultiTexCoord3dv( GLenum target, const GLdouble *v )
{}

void mglMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t, GLfloat r )
{}

void mglMultiTexCoord3fv( GLenum target, const GLfloat *v )
{}

void mglMultiTexCoord3i( GLenum target, GLint s, GLint t, GLint r )
{}

void mglMultiTexCoord3iv( GLenum target, const GLint *v )
{}

void mglMultiTexCoord3s( GLenum target, GLshort s, GLshort t, GLshort r )
{}

void mglMultiTexCoord3sv( GLenum target, const GLshort *v )
{}

void mglMultiTexCoord4d( GLenum target, GLdouble s, GLdouble t, GLdouble r,
                         GLdouble q )
{}

void mglMultiTexCoord4dv( GLenum target, const GLdouble *v )
{}

void mglMultiTexCoord4f( GLenum target, GLfloat s, GLfloat t, GLfloat r,
                         GLfloat q )
{}

void mglMultiTexCoord4fv( GLenum target, const GLfloat *v )
{}

void mglMultiTexCoord4i( GLenum target, GLint s, GLint t, GLint r, GLint q )
{}

void mglMultiTexCoord4iv( GLenum target, const GLint *v )
{}

void mglMultiTexCoord4s( GLenum target, GLshort s, GLshort t, GLshort r,
                         GLshort q )
{}

void mglMultiTexCoord4sv( GLenum target, const GLshort *v )
{}


void mglLoadTransposeMatrixd( const GLdouble m[16] )
{}

void mglLoadTransposeMatrixf( const GLfloat m[16] )
{}

void mglMultTransposeMatrixd( const GLdouble m[16] )
{}

void mglMultTransposeMatrixf( const GLfloat m[16] )
{}

void mglSampleCoverage( GLclampf value, GLboolean invert )
{}

/*
 * GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
 */

void mglActiveTextureARB( GLenum texture )
{}
void mglClientActiveTextureARB( GLenum texture )
{}
void mglMultiTexCoord1dARB( GLenum target, GLdouble s )
{}
void mglMultiTexCoord1dvARB( GLenum target, const GLdouble *v )
{}
void mglMultiTexCoord1fARB( GLenum target, GLfloat s )
{}
void mglMultiTexCoord1fvARB( GLenum target, const GLfloat *v )
{}
void mglMultiTexCoord1iARB( GLenum target, GLint s )
{}
void mglMultiTexCoord1ivARB( GLenum target, const GLint *v )
{}
void mglMultiTexCoord1sARB( GLenum target, GLshort s )
{}
void mglMultiTexCoord1svARB( GLenum target, const GLshort *v )
{}
void mglMultiTexCoord2dARB( GLenum target, GLdouble s, GLdouble t )
{}
void mglMultiTexCoord2dvARB( GLenum target, const GLdouble *v )
{}
void mglMultiTexCoord2fARB( GLenum target, GLfloat s, GLfloat t )
{}
void mglMultiTexCoord2fvARB( GLenum target, const GLfloat *v )
{}
void mglMultiTexCoord2iARB( GLenum target, GLint s, GLint t )
{}
void mglMultiTexCoord2ivARB( GLenum target, const GLint *v )
{}
void mglMultiTexCoord2sARB( GLenum target, GLshort s, GLshort t )
{}
void mglMultiTexCoord2svARB( GLenum target, const GLshort *v )
{}
void mglMultiTexCoord3dARB( GLenum target, GLdouble s, GLdouble t, GLdouble r )
{}
void mglMultiTexCoord3dvARB( GLenum target, const GLdouble *v )
{}
void mglMultiTexCoord3fARB( GLenum target, GLfloat s, GLfloat t, GLfloat r )
{}
void mglMultiTexCoord3fvARB( GLenum target, const GLfloat *v )
{}
void mglMultiTexCoord3iARB( GLenum target, GLint s, GLint t, GLint r )
{}
void mglMultiTexCoord3ivARB( GLenum target, const GLint *v )
{}
void mglMultiTexCoord3sARB( GLenum target, GLshort s, GLshort t, GLshort r )
{}
void mglMultiTexCoord3svARB( GLenum target, const GLshort *v )
{}
void mglMultiTexCoord4dARB( GLenum target, GLdouble s, GLdouble t, GLdouble r,
                            GLdouble q )
{}
void mglMultiTexCoord4dvARB( GLenum target, const GLdouble *v )
{}
void mglMultiTexCoord4fARB( GLenum target, GLfloat s, GLfloat t, GLfloat r,
                            GLfloat q )
{}
void mglMultiTexCoord4fvARB( GLenum target, const GLfloat *v )
{}
void mglMultiTexCoord4iARB( GLenum target, GLint s, GLint t, GLint r, GLint q )
{}
void mglMultiTexCoord4ivARB( GLenum target, const GLint *v )
{}
void mglMultiTexCoord4sARB( GLenum target, GLshort s, GLshort t, GLshort r,
                            GLshort q )
{}
void mglMultiTexCoord4svARB( GLenum target, const GLshort *v )
{}
