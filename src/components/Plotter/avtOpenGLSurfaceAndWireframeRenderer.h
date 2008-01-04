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

// ************************************************************************* //
//                   avtOpenGLSurfaceAndWireframeRenderer.h                  //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_SURFACE_AND_WIREFRAME_RENDERER_H
#define AVT_OPEN_GL_SURFACE_AND_WIREFRAME_RENDERER_H

#include <plotter_exports.h>

#include <avtSurfaceAndWireframeRenderer.h>


// ****************************************************************************
//  Class: avtOpenGLSurfaceAndWireframeRenderer
//
//  Purpose:
//      This is a surface and wireframe renderer that actually knows how to
//      perform OpenGL calls.
//
//  Programmer: Hank Childs 
//  Creation:   April 23, 2002
//
//  Modifications:
//    Brad Whitlock, Fri May 10 14:02:21 PST 2002
//    Moved the ctor, dtor bodies to the C file to get it to work on Windows.
//
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002    
//    Added secondary draw and setup methods, so that display lists could be
//    supported.  Added members to keep track of display list indices. 
//
//    Kathleen Bonnell, Tue Aug 26 13:54:32 PDT 2003 
//    Added ReleaseGraphicsResources. 
//    
// ****************************************************************************

class PLOTTER_API avtOpenGLSurfaceAndWireframeRenderer 
    : public avtSurfaceAndWireframeRenderer
{
  public:
                              avtOpenGLSurfaceAndWireframeRenderer();
    virtual                  ~avtOpenGLSurfaceAndWireframeRenderer();


  protected:
    virtual void              SetupGraphicsLibrary();
    virtual void              DrawEdges();
    virtual void              DrawSurface();
    virtual void              ReleaseGraphicsResources();

   private:
    void                      SetupGraphicsLibrary2();
    void                      DrawEdges2();
    void                      DrawSurface2();

    std::vector<int>          surfaceListId;
    std::vector<int>          edgesListId;
    std::vector<int>          setupListId;
};

#endif
