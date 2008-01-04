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

#include <GL/glu.h>

void mgluBeginCurve ( GLUnurbs* nurb )
{}

void mgluBeginPolygon ( GLUtesselator* tess )
{}

void mgluBeginSurface ( GLUnurbs* nurb )
{}

void mgluBeginTrim ( GLUnurbs* nurb )
{}

GLint mgluBuild1DMipmapLevels ( GLenum target, GLint internalFormat,
                                GLsizei width, GLenum format, GLenum type,
                                GLint level, GLint base, GLint max,
                                const void *data )
{}

GLint mgluBuild1DMipmaps ( GLenum target, GLint internalFormat,
                           GLsizei width, GLenum format, GLenum type,
                           const void *data )
{}

GLint mgluBuild2DMipmapLevels ( GLenum target, GLint internalFormat,
                                GLsizei width, GLsizei height, GLenum format,
                                GLenum type, GLint level, GLint base,
                                GLint max, const void *data )
{}

GLint mgluBuild2DMipmaps ( GLenum target, GLint internalFormat,
                           GLsizei width, GLsizei height, GLenum format,
                           GLenum type, const void *data )
{}

GLint mgluBuild3DMipmapLevels ( GLenum target, GLint internalFormat,
                                GLsizei width, GLsizei height, GLsizei depth,
                                GLenum format, GLenum type, GLint level,
                                GLint base, GLint max, const void *data )
{}

GLint mgluBuild3DMipmaps ( GLenum target, GLint internalFormat,
                           GLsizei width, GLsizei height, GLsizei depth,
                           GLenum format, GLenum type, const void *data )
{}

GLboolean mgluCheckExtension ( const GLubyte *extName,
                               const GLubyte *extString )
{}

void mgluCylinder ( GLUquadric* quad, GLdouble base, GLdouble top,
                    GLdouble height, GLint slices, GLint stacks )
{}

void mgluDeleteNurbsRenderer ( GLUnurbs* nurb )
{}

void mgluDeleteQuadric ( GLUquadric* quad )
{}

void mgluDeleteTess ( GLUtesselator* tess )
{}

void mgluDisk ( GLUquadric* quad, GLdouble inner, GLdouble outer,
                GLint slices, GLint loops )
{}

void mgluEndCurve ( GLUnurbs* nurb )
{}

void mgluEndPolygon ( GLUtesselator* tess )
{}

void mgluEndSurface ( GLUnurbs* nurb )
{}

void mgluEndTrim ( GLUnurbs* nurb )
{}

const GLubyte * mgluErrorString ( GLenum error )
{}

void mgluGetNurbsProperty ( GLUnurbs* nurb, GLenum property, GLfloat* data )
{}

const GLubyte * mgluGetString ( GLenum name )
{}

void mgluGetTessProperty ( GLUtesselator* tess, GLenum which, GLdouble* data )
{}

void mgluLoadSamplingMatrices ( GLUnurbs* nurb, const GLfloat *model,
                                const GLfloat *perspective, const GLint *view )
{}

void mgluLookAt ( GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
                  GLdouble centerX, GLdouble centerY, GLdouble centerZ,
                  GLdouble upX, GLdouble upY, GLdouble upZ )
{}

GLUnurbs* mgluNewNurbsRenderer ( void )
{}

GLUquadric* mgluNewQuadric ( void )
{}

GLUtesselator* mgluNewTess ( void )
{}

void mgluNextContour ( GLUtesselator* tess, GLenum type )
{}

void mgluNurbsCallback ( GLUnurbs* nurb, GLenum which,
                         _GLUfuncptr CallBackFunc )
{}

void mgluNurbsCallbackData ( GLUnurbs* nurb, GLvoid* userData )
{}

void mgluNurbsCallbackDataEXT ( GLUnurbs* nurb, GLvoid* userData )
{}

void mgluNurbsCurve ( GLUnurbs* nurb, GLint knotCount, GLfloat *knots,
                      GLint stride, GLfloat *control, GLint order, GLenum type )
{}

void mgluNurbsProperty ( GLUnurbs* nurb, GLenum property, GLfloat value )
{}

void mgluNurbsSurface ( GLUnurbs* nurb, GLint sKnotCount, GLfloat* sKnots,
                        GLint tKnotCount, GLfloat* tKnots, GLint sStride,
                        GLint tStride, GLfloat* control, GLint sOrder,
                        GLint tOrder, GLenum type )
{}

void mgluOrtho2D ( GLdouble left, GLdouble right,
                   GLdouble bottom, GLdouble top )
{}

void mgluPartialDisk ( GLUquadric* quad, GLdouble inner, GLdouble outer,
                       GLint slices, GLint loops, GLdouble start,
                       GLdouble sweep )
{}

void mgluPerspective ( GLdouble fovy, GLdouble aspect,
                       GLdouble zNear, GLdouble zFar )
{}

void mgluPickMatrix ( GLdouble x, GLdouble y, GLdouble delX, GLdouble delY,
                      GLint *viewport )
{}

GLint mgluProject ( GLdouble objX, GLdouble objY, GLdouble objZ,
                    const GLdouble *model, const GLdouble *proj,
                    const GLint *view,
                    GLdouble* winX, GLdouble* winY, GLdouble* winZ )
{}

void mgluPwlCurve ( GLUnurbs* nurb, GLint count, GLfloat* data,
                    GLint stride, GLenum type )
{}

void mgluQuadricCallback ( GLUquadric* quad, GLenum which,
                           _GLUfuncptr CallBackFunc )
{}

void mgluQuadricDrawStyle ( GLUquadric* quad, GLenum draw )
{}

void mgluQuadricNormals ( GLUquadric* quad, GLenum normal )
{}

void mgluQuadricOrientation ( GLUquadric* quad, GLenum orientation )
{}

void mgluQuadricTexture ( GLUquadric* quad, GLboolean texture )
{}

GLint mgluScaleImage ( GLenum format, GLsizei wIn, GLsizei hIn,
                       GLenum typeIn, const void *dataIn,
                       GLsizei wOut, GLsizei hOut, GLenum typeOut,
                       GLvoid* dataOut )
{}

void mgluSphere ( GLUquadric* quad, GLdouble radius, GLint slices,
                  GLint stacks )
{}

void mgluTessBeginContour ( GLUtesselator* tess )
{}

void mgluTessBeginPolygon ( GLUtesselator* tess, GLvoid* data )
{}

void mgluTessCallback ( GLUtesselator* tess, GLenum which,
                        _GLUfuncptr CallBackFunc )
{}

void mgluTessEndContour ( GLUtesselator* tess )
{}

void mgluTessEndPolygon ( GLUtesselator* tess )
{}

void mgluTessNormal ( GLUtesselator* tess,
                      GLdouble valueX, GLdouble valueY, GLdouble valueZ )
{}

void mgluTessProperty ( GLUtesselator* tess, GLenum which, GLdouble data )
{}

void mgluTessVertex ( GLUtesselator* tess, GLdouble *location, GLvoid* data )
{}

GLint mgluUnProject ( GLdouble winX, GLdouble winY, GLdouble winZ,
                      const GLdouble *model, const GLdouble *proj,
                      const GLint *view,
                      GLdouble* objX, GLdouble* objY, GLdouble* objZ )
{}

GLint mgluUnProject4 ( GLdouble winX, GLdouble winY, GLdouble winZ,
                       GLdouble clipW, const GLdouble *model,
                       const GLdouble *proj, const GLint *view,
                       GLdouble nearVal, GLdouble farVal,
                       GLdouble* objX, GLdouble* objY, GLdouble* objZ,
                       GLdouble* objW )
{}
