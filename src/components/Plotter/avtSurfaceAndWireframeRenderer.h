// ************************************************************************* //
//                     avtSurfaceAndWireframeRenderer.h                      //
// ************************************************************************* //

#ifndef AVT_SURFACE_AND_WIREFRAME_RENDERER_H
#define AVT_SURFACE_AND_WIREFRAME_RENDERER_H

#include <plotter_exports.h>

#include <avtCustomRenderer.h>

class vtkLookupTable;
class vtkPolyData;
class vtkProperty;
class vtkUnsignedCharArray;


typedef struct lec { float r, g, b; } LEC;

// ****************************************************************************
//  Class: avtSurfaceAndWireframeRenderer
//
//  Purpose:
//    An implementation of an avtCustomRenderer for a surface plot.
//    Allows both surface and wireframe to be rendered at the same time
//    from the same dataset. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2001. 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Aug 22 15:06:29 PDT 2001
//    Add methods whereby users could specify which types
//    of cells are drawn, by turning them on/off. Default is all on.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001 
//    Removed method SetLUTColors.  This functionality now resides
//    in avtLookupTable.
// 
//    Hank Childs, Tue Sep 18 10:41:30 PDT 2001
//    Make destructor virtual.
//
//    Kathleen Bonnell, Wed Sep 26 17:47:41 PDT 2001 
//    Add ability to turn on/off both surface-associated primitives
//    and edge-associated primitives. 
//
//    Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001 
//    Colors represented by vtkUnsignedCharArray instead of vtkScalars, 
//    to match VTK 4.0 API. 
//
//    Hank Childs, Tue Apr 23 19:17:44 PDT 2002
//    Inherit from custom renderer.  Split class into derived types for
//    rendering method.
//
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002
//    Added flags to keep track of modifications to certain members for each
//    input dataset.  Also added ImmediateModeRendering flags for future use. 
//    
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//    Added methods in support of lighting.  Added members lastAmbient and
//    lastDiffuse to keep track of lighting coefficients. 
//    
//    Kathleen Bonnell, Wed Aug 14 12:54:01 PDT 2002  
//    Added option to allow lighting to be ignored. 
//
//    Kathleen Bonnell, Tue Nov 26 15:39:16 PST 2002
//    Removed unused members lastAmbient, lastDiffuse.  Added new member
//    lutColorsChanged and method LUTColorsChanged to set it.  Added
//    GetScalarVisibility.
//
//    Kathleen Bonnell, Thu Aug  7 08:29:31 PDT 2003 
//    Moved immediateModeRendering and Set/Get methods to parent class. 
//    
//    Kathleen Bonnell, Tue Aug 26 14:47:57 PDT 2003 
//    Added ReleaseGraphicsResources(), lastWindowSize, and setupModified.
//    
// ****************************************************************************

class PLOTTER_API avtSurfaceAndWireframeRenderer : public avtCustomRenderer
{
  public:
                              avtSurfaceAndWireframeRenderer();
    virtual                  ~avtSurfaceAndWireframeRenderer();

    static avtSurfaceAndWireframeRenderer *
                              New();

    virtual void              Render(vtkDataSet *);

    void                      SetLookupTable(vtkLookupTable *lut);
    void                      CreateDefaultLookupTable(void);

    void                      SetProperty(vtkProperty *);
    void                      SetScalarRange(float, float);
                                             
    void                      ResolveTopologyOn();
    void                      ResolveTopologyOff();
    void                      ScalarVisibilityOn();
    void                      ScalarVisibilityOff();
    bool                      GetScalarVisibility() 
                                  { return scalarVisibility; };
    void                      SurfaceVertsOn();
    void                      SurfaceVertsOff();
    void                      SurfaceLinesOn();
    void                      SurfaceLinesOff();
    void                      SurfaceStripsOn();
    void                      SurfaceStripsOff();
    void                      SurfacePolysOn();
    void                      SurfacePolysOff();
    void                      EdgeLinesOn();
    void                      EdgeLinesOff();
    void                      EdgeStripsOn();
    void                      EdgeStripsOff();
    void                      EdgePolysOn();
    void                      EdgePolysOff();

    virtual void              GlobalLightingOn();
    virtual void              GlobalLightingOff();
    virtual void              GlobalSetAmbientCoefficient(const float);

    void                      IgnoreLighting(const bool);
    void                      LUTColorsChanged(const bool);

  protected:
    
    vtkPolyData              *input;
    vtkUnsignedCharArray     *Colors;
    vtkLookupTable           *LUT;
    vtkProperty              *prop;
    bool                      scalarVisibility;
    bool                      resolveTopology;
    bool                      scalarRange;
    float                     range[2];
    bool                      drawSurfaceVerts; 
    bool                      drawSurfaceLines; 
    bool                      drawSurfaceStrips; 
    bool                      drawSurfacePolys; 
    bool                      drawEdgeLines; 
    bool                      drawEdgeStrips; 
    bool                      drawEdgePolys; 

    bool                      ignoreLighting;
    bool                      lutColorsChanged;

    void                      Initialize();
    virtual void              SetupGraphicsLibrary() = 0;
    void                      Draw();
    virtual void              DrawEdges() = 0;
    virtual void              DrawSurface() = 0;
    void                      SetColors();
    virtual void              ReleaseGraphicsResources() = 0;

    std::vector<bool>          surfaceModified; 
    std::vector<bool>          edgesModified; 
    std::vector<bool>          setupModified; 
    std::vector<unsigned long> propMTime; 
    std::vector<int>           lastRep; 
    std::vector<int>           lastInterp; 
    std::vector<LEC>           lastEdgeColor; 
    std::vector<vtkDataSet*>   inputs;         
    int                        inputNum; 
    int                        lastWindowSize[2]; 
};


typedef ref_ptr<avtSurfaceAndWireframeRenderer>
                                              avtSurfaceAndWireframeRenderer_p;


#endif


