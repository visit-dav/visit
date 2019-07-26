// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtScatterFilter.h                            //
// ************************************************************************* //

#ifndef AVT_SCATTER_FILTER_H
#define AVT_SCATTER_FILTER_H

#include <avtDataTreeIterator.h>

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
//    Added overrides of PreExecute, ModifyContract, and added new 
//    method PopulateDataInputs. Also added extents members.
//
//    Cyrus Harrison, Tue Aug 17 11:52:24 PDT 2010
//    Added PostExecute method to:
//      1) Set legend if a color var is selected.
//      (This logic was moved out of ExecuteData to prevent a parallel hang
//       when there are more procs than chunks to process.)
//      2) Set proper spatial extents.
//
//    Cyrus Harrison, Thu Aug 19 13:35:08 PDT 2010
//    Changes to support using var1 from atts.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents name.
//
//    Kathleen Biagas, Thu Mar  1 14:49:50 MST 2012
//    Add keepNodeZone and dataArray (origNodes) arg to PointMeshFromVariables.
//
//    Eric Brugger, Tue Aug 19 11:13:13 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Kathleen Biagas, Fri Mar 20 11:44:28 PDT 2015
//    Make 'PointMeshFromVariables' a templated method to support double
//    precision.
//
// ****************************************************************************

class avtScatterFilter : public avtDataTreeIterator
{
  public:
                               avtScatterFilter(const ScatterAttributes &);
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

    ScatterAttributes          atts;
    double                     xExtents[2];
    double                     yExtents[2];
    double                     zExtents[2];
    double                     colorExtents[2];
    bool                       needXExtents;
    bool                       needYExtents;
    bool                       needZExtents;
    bool                       needColorExtents;
    bool                       keepNodeZone;

    doubleVector               thisProcsSpatialExtents;

    virtual void               PreExecute(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void               PostExecute(void);
    virtual void               UpdateDataObjectInfo(void);
    virtual avtContract_p     
                               ModifyContract(avtContract_p spec);
    vtkDataArray              *GetDataArray(vtkDataSet *inDS,
                                            const std::string &name,
                                            avtCentering targetCentering,
                                            bool &deleteArray);
    vtkDataArray              *Recenter(vtkDataSet *ds, vtkDataArray *arr, 
                                        avtCentering cent) const;

    template <typename T>
    vtkDataSet                *PointMeshFromVariables(DataInput *d1,
                                                      DataInput *d2,
                                                      DataInput *d3,
                                                      DataInput *d4, bool &,
                                                      vtkDataArray *,
                                                      int);
    int                        CountSpatialDimensions() const;
    void                       PopulateDataInputs(DataInput *orderedArrays,
                                                  vtkDataArray **arr) const;
    void                       PopulateNames(const char **) const;
    bool                       NeedSpatialExtents() const;
};


#endif


