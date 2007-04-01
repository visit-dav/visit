// ************************************************************************* //
//                             avtWireframeFilter.h                            //
// ************************************************************************* //

#ifndef AVT_WIREFRAME_FILTER_H
#define AVT_WIREFRAME_FILTER_H


#include <avtStreamer.h>
#include <SurfaceAttributes.h>


class vtkDataSet;
class vtkAppendPolyData;
class vtkUniqueFeatureEdges;
class vtkGeometryFilter;


// ****************************************************************************
//  Class: avtWireframeFilter
//
//  Purpose:
//    A filter that turns a 2d dataset into a 3d dataset based upon
//    scaled point or cell data. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 24, 2004
//
//  Modifications:
//
// ****************************************************************************

class avtWireframeFilter : public avtStreamer
{
  public:
                            avtWireframeFilter(const AttributeGroup*);
    virtual                ~avtWireframeFilter();

    static avtFilter       *Create(const AttributeGroup*);

    virtual const char     *GetType(void)  { return "avtWireframeFilter"; };
    virtual const char     *GetDescription(void)
                            { return "Creating wireframe of surface."; };

    virtual void            ReleaseData(void);
    virtual bool            Equivalent(const AttributeGroup*);

  protected:
    SurfaceAttributes       atts;
    vtkGeometryFilter      *geoFilter;
    vtkAppendPolyData      *appendFilter;
    vtkUniqueFeatureEdges  *edgesFilter;

    virtual vtkDataSet     *ExecuteData(vtkDataSet *, int, std::string);
    virtual void            PostExecute(void);
};


#endif


