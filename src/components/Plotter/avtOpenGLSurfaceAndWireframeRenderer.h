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
