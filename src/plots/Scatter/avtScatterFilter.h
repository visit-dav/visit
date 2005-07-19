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
//    Brad Whitlock, Mon Jul 18 11:53:27 PDT 2005
//    Added overrides of PreExecute, PerformRestriction, and added new 
//    method PopulateDataInputs. Also added extents members.
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
    double                     xExtents[2];
    double                     yExtents[2];
    double                     zExtents[2];
    double                     colorExtents[2];
    bool                       needXExtents;
    bool                       needYExtents;
    bool                       needZExtents;
    bool                       needColorExtents;

    virtual void               PreExecute(void);
    virtual vtkDataSet        *ExecuteData(vtkDataSet *, int, std::string);
    virtual void               RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p     
                               PerformRestriction(avtPipelineSpecification_p spec);
    vtkDataArray              *GetDataArray(vtkDataSet *inDS,
                                            const std::string &name,
                                            avtCentering targetCentering,
                                            bool &deleteArray);
    vtkDataArray              *Recenter(vtkDataSet *ds, vtkDataArray *arr, 
                                        avtCentering cent) const;
    vtkDataSet                *PointMeshFromVariables(DataInput *d1,
                                                      DataInput *d2,
                                                      DataInput *d3,
                                                      DataInput *d4, bool &);
    int                        CountSpatialDimensions() const;
    void                       PopulateDataInputs(DataInput *orderedArrays,
                                                  vtkDataArray **arr) const;
    void                       PopulateNames(const char **) const;
    bool                       NeedSpatialExtents() const;
};


#endif


