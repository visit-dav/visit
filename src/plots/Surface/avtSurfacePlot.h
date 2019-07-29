// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtSurfacePlot.h                             //
// ************************************************************************* //

#ifndef AVT_SURFACE_PLOT_H
#define AVT_SURFACE_PLOT_H


#include <LineAttributes.h>
#include <SurfaceAttributes.h>

#include <avtLegend.h>
#include <avtPlot.h>

class     vtkProperty;

class     avtLookupTable;
class     avtSurfaceFilter;
class     avtSurfaceMapper;
class     avtWireframeFilter;
class     avtVariableLegend;


// ****************************************************************************
//  Class:  avtSurfacePlot
//
//  Purpose:
//      A concrete type of avtPlot, this generates a surface plot from 2d data. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Mar 27 14:47:03 PST 2001
//    Inherited from avtSurfaceDataPlot instead of avtPlot and added GetName.
//
//    Kathleen Bonnell, Fri Mar 30 08:56:47 PDT 2001 
//    Added methods SetLineWidth, SetScaling, SetMin, SetMinOff, 
//    SetMax, SetMaxOff 
//
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Brad Whitlock, Fri Jun 15 14:22:57 PST 2001
//    Added override of the SetColorTable method.
//
//    Kathleen Bonnell, Thu Jun 21 17:33:08 PDT 2001 
//    Added method SetLineStyle.
//
//    Kathleen Bonnell, Tue Aug 21 10:06:05 PDT 2001 
//    Use avtSurfaceAndWireframeRenderer instead of avtVariableMapper. 
//    Modified parameters to SetLineWidth, SetLineStyle.  Removed
//    methods related to setting min/max. Add methods SetRepresentation,
//    SetSurfaceAttributes, SetWireframeAttributes.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Add avtLookupTable in place of vtkLookupTable. 
//    
//    Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//    Added method SetLimitsMode. 
//    
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//    
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004 
//    Added avtWireframeFilter.
//
//    Brad Whitlock, Wed Dec 15 10:48:35 PDT 2004
//    Removed SetVarName.
//
//    Kathleen Biagas, Mon Jul 18 17:07:22 MST 2016 
//    Remove avtWireframeFilter. Add surface-plot specific mapper, removed
//    use of avtSurfaceAndWireframeRenderer. (All for VTK-8).
//
// ****************************************************************************

class avtSurfacePlot : public avtSurfaceDataPlot
{
  public:
                                avtSurfacePlot();
    virtual                    ~avtSurfacePlot();

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual void                ReleaseData(void);
    virtual bool                SetColorTable(const char *ctName);

    virtual const char         *GetName(void)  { return "SurfacePlot"; };

    void                        SetLegend(bool);
    void                        SetScaling(const int, const double);
    void                        SetSurfaceAttributes(bool);
    void                        SetWireframeAttributes(bool);
    void                        SetLimitsMode(int);

  protected:
    avtSurfaceMapper               *mapper;
    avtVariableLegend              *varLegend;
    avtLegend_p                     varLegendRefPtr;
    avtLookupTable                 *avtLUT;
    avtSurfaceFilter               *surfaceFilter;
    SurfaceAttributes               atts;
    bool                            colorsInitialized;

    virtual avtMapperBase      *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior();
    virtual avtLegend_p         GetLegend(void){ return varLegendRefPtr; };
};


#endif


