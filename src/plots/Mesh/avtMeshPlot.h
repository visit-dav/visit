// ************************************************************************* //
//                               avtMeshPlot.h                               //
// ************************************************************************* //

#ifndef AVT_MESH_PLOT_H
#define AVT_MESH_PLOT_H

#include <LineAttributes.h>
#include <MeshAttributes.h>

#include <avtPlot.h>
#include <avtSurfaceAndWireframeRenderer.h>
#include <avtGhostZoneAndFacelistFilter.h>

class     vtkProperty;

class     avtMeshFilter;
class     avtPointToGlyphFilter;
class     avtUserDefinedMapper;
class     avtVariableLegend;
class     avtSmoothPolyDataFilter;


// ****************************************************************************
//  Method: avtMeshPlot
//
//  Purpose:
//      A concrete type of avtPlot for meshes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 21, 2001 
//
//  Modifications:
//
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Kathleen Bonnell, Fri Jun  15 14:15:43 PDT 2001
//    Added mesh filter. 
//
//    Kathleen Bonnell, Fri Jun 29 10:27:44 PDT 2001 
//    Added method to set opaque render mode. 
//
//    Hank Childs, Thu Jul 26 11:59:16 PDT 2001
//    Went back to inheriting from avtPlot, since this can be a line plot or
//    a surface plot.
//
//    Kathleen Bonnell, Wed Aug 22 15:22:55 PDT 2001 
//    Use avtUserDefinedMapper and avtSurfaceAndWireframeRenderer instead
//    of avtWireframeMapper to better draw opaque mode.  Modified arguments
//    to SetLineWidth and SetLineStyle.  Added member property.
//    
//    Kathleen Bonnell, Sep  5 12:13:54 PDT 2001
//    Add method to set the color for opaque mode.
//
//    Kathleen Bonnell, Wed Sep 26 15:14:07 PDT 2001 
//    Add methods to set the background/foreground colors.
//
//    Jeremy Meredith, Fri Nov  9 10:15:17 PST 2001
//    Added an Equivalent method.
//
//    Hank Childs, Sun Jun 23 21:07:55 PDT 2002
//    Added support for point meshes.
//
//    Jeremy Meredith, Mon Jul  8 18:38:04 PDT 2002
//    Added facelist filter before the mesh lines were calculated.
//
//    Hank Childs, Fri Jul 19 08:59:21 PDT 2002
//    Use the ghost zone and facelist filter instead of the normal facelist
//    filter.
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//    
//    Jeremy Meredith, Tue Dec 10 10:04:18 PST 2002
//    Added smooth poly data filter.
//
//    Kathleen Bonnell, Mon Mar 24 17:48:27 PST 2003
//    Added SetOpaqueColor with double* arg.  Added flag
//    wireframeRenderingIsInappropriate. 
//
//    Kathleen Bonnell, Wed Aug 27 15:45:45 PDT 2003 
//    Added SetOpaqueMeshIsAppropriate.
//    
// ****************************************************************************

class
avtMeshPlot : public avtPlot
{
  public:
                    avtMeshPlot();
    virtual        ~avtMeshPlot();

    static avtPlot *Create();

    virtual const char *GetName(void)  { return "MeshPlot"; };
    virtual void    ReleaseData(void);

    virtual void    SetAtts(const AttributeGroup*);
    virtual bool    Equivalent(const AttributeGroup*);

    void            SetLegend(bool);
    void            SetLineWidth(_LineWidth);
    void            SetLineStyle(_LineStyle);
    void            SetPointSize(float);
    void            SetRenderOpaque(bool);
    void            SetMeshColor(const unsigned char *);
    void            SetMeshColor(const double *);
    void            SetOpaqueColor(const unsigned char *, bool force = false);
    void            SetOpaqueColor(const double *, bool force = false);
    void            SetVarName(const char *);
    virtual bool    SetBackgroundColor(const double *);
    virtual bool    SetForegroundColor(const double *);

    virtual int     TargetTopologicalDimension(void);
    virtual const AttributeSubject 
                   *SetOpaqueMeshIsAppropriate(bool);

  protected:
    avtSurfaceAndWireframeRenderer_p renderer;
    avtUserDefinedMapper            *mapper;
    avtVariableLegend               *varLegend;
    avtGhostZoneAndFacelistFilter   *ghostAndFaceFilter;
    avtMeshFilter                   *filter;
    avtPointToGlyphFilter           *glyphPoints;
    vtkProperty                     *property;
    avtLegend_p                      varLegendRefPtr;
    avtSmoothPolyDataFilter         *smooth;
    double                           bgColor[3];
    double                           fgColor[3];
    bool                             wireframeRenderingIsInappropriate;
    MeshAttributes           atts;

    virtual avtMapper       *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual void             CustomizeMapper(avtDataObjectInformation &);
    virtual avtLegend_p      GetLegend(void) { return varLegendRefPtr; };
};


#endif


