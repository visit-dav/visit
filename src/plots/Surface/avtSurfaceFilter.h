// ************************************************************************* //
//                             avtSurfaceFilter.h                            //
// ************************************************************************* //

#ifndef AVT_SURFACE_FILTER_H
#define AVT_SURFACE_FILTER_H


#include <avtStreamer.h>
#include <SurfaceAttributes.h>


class vtkDataSet;
class vtkSurfaceFilter;
class vtkCellDataToPointData;
class vtkAppendPolyData;
class vtkUniqueFeatureEdges;
class vtkGeometryFilter;


// ****************************************************************************
//  Class: avtSurfaceFilter
//
//  Purpose:
//    A filter that turns a 2d dataset into a 3d dataset based upon
//    scaled point or cell data. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Mar 29 13:54:27 PST 2001 
//    Added method 'SkewTheValue'.
//
//    Kathleen Bonnell, Tue Apr 10 11:46:11 PDT 2001
//    Renamed ExecuteDomain as ExecuteData.
//
//    Hank Childs, Tue Jun 12 13:52:27 PDT 2001
//    Renamed several methods to reflect changes in the base class.
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Added filters to extract the edges, for wireframe rendering. 
//    Added PreExecute and PerformRestriction.  Added members min, max,
//    Ms, Bs.
//
//    Kathleen Bonnell, Fri Oct 10 10:48:24 PDT 2003
//    Added PostExecute. 
//
//    Mark C. Miller, Sun Feb 29 18:08:26 PST 2004
//    Added zValMin, zValMax data members
//
//    Mark C. Miller, Tue Mar  2 17:55:30 PST 2004
//    Removed zValMin, zValMax
//
// ****************************************************************************

class avtSurfaceFilter : public avtStreamer
{
  public:
                            avtSurfaceFilter(const AttributeGroup*);
    virtual                ~avtSurfaceFilter();

    static avtFilter       *Create(const AttributeGroup*);

    virtual const char     *GetType(void)  { return "avtSurfaceFilter"; };
    virtual const char     *GetDescription(void)
                            { return "Elevating 2D data by variable values"; };

    virtual void            ReleaseData(void);
    virtual bool            Equivalent(const AttributeGroup*);

  protected:
    SurfaceAttributes       atts;
    vtkSurfaceFilter       *filter;
    vtkCellDataToPointData *cd2pd;
    vtkGeometryFilter      *geoFilter;
    vtkAppendPolyData      *appendFilter;
    vtkUniqueFeatureEdges  *edgesFilter;
    double                  min;
    double                  max;
    double                  Ms;
    double                  Bs;
    bool                    stillNeedExtents;

    double                  SkewTheValue(const double);
    void                    CalculateScaleValues(double *, double*);

    virtual vtkDataSet     *ExecuteData(vtkDataSet *, int, std::string);
    virtual void            RefashionDataObjectInfo(void);
    virtual void            VerifyInput(void);
    virtual void            PreExecute(void);
    virtual void            PostExecute(void);
    virtual avtPipelineSpecification_p
                            PerformRestriction(avtPipelineSpecification_p);
};


#endif


