// ************************************************************************* //
//                             avtScatterFilter.h                            //
// ************************************************************************* //

#ifndef AVT_SCATTER_FILTER_H
#define AVT_SCATTER_FILTER_H

#include <avtStreamer.h>
#include <ScatterAttributes.h>

// ****************************************************************************
//  Class: avtScatterFilter
//
//  Purpose:
//      A filter that combines multiple scalar fields into a point mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:31:23 PST 2004
//
//  Modifications:
//
// ****************************************************************************

class avtScatterFilter : public avtStreamer
{
  public:
                               avtScatterFilter(const std::string &v,
                                                const ScatterAttributes &);
    virtual                   ~avtScatterFilter();

    virtual const char        *GetType(void)  { return "avtScatterFilter"; };
    virtual const char        *GetDescription(void)  
                                   { return "Creating point mesh"; };

protected:
    struct DataInput
    {
        vtkDataArray *data;
        bool          useMin;
        bool          useMax;
        float         min;
        float         max;
        int           scale;
        float         skew;
    };

    std::string                variableName;
    ScatterAttributes          atts;

    virtual vtkDataSet        *ExecuteData(vtkDataSet *, int, std::string);
    virtual void               RefashionDataObjectInfo(void);
    //virtual avtPipelineSpecification_p     
                               //PerformRestriction(avtPipelineSpecification_p spec);
    vtkDataArray              *GetDataArray(vtkDataSet *inDS,
                                            const std::string &name,
                                            avtCentering targetCentering,
                                            bool &deleteArray);
    vtkDataArray              *Recenter(vtkDataSet *ds, vtkDataArray *arr, 
                                        avtCentering cent) const;
    vtkDataSet                *PointMeshFromVariables(DataInput *d1,
                                                      DataInput *d2,
                                                      DataInput *d3);
    int                        CountSpatialDimensions() const;

};


#endif


