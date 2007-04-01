// ************************************************************************* //
//                            avtDatasetExaminer.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_EXAMINER_H
#define AVT_DATASET_EXAMINER_H

#include <pipeline_exports.h>

#include <avtDataset.h>
#include <avtTypes.h>


class     vtkDataArray;


// ****************************************************************************
//  Class: avtDatasetExaminer
//
//  Purpose:
//      This is a collection of routines that walk through a dataset.  It is
//      separated out so that the avtDataset class does not get too messy.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 4 11:44:40 PDT 2002
//    I added GetDataMagnitudeExtents.
//
//    Kathleen Bonnell, Wed Feb 18 08:31:26 PST 2004
//    Added GetNumberOfNodes, another GetNumberOfZones.
//
//    Hank Childs, Tue Feb 24 17:33:45 PST 2004
//    Account for multiple variables.
//
// ****************************************************************************

class PIPELINE_API avtDatasetExaminer
{
  public:
    static int                GetNumberOfNodes(avtDataset_p &);
    static int                GetNumberOfZones(avtDataset_p &);
    static void               GetNumberOfZones(avtDataset_p &, int &, int &);
    static void               GetVariableList(avtDataset_p &, VarList &);
    static bool               GetSpatialExtents(avtDataset_p &, double *);
    static bool               GetDataExtents(avtDataset_p &, double *,
                                             const char * = NULL);
    static bool               GetDataMagnitudeExtents(avtDataset_p &, double*,
                                                      const char * = NULL);
    static void               FindMinimum(avtDataset_p &, double *, double &);
    static void               FindMaximum(avtDataset_p &, double *, double &);
    static bool               FindZone(avtDataset_p &, int, int, double *);
    static bool               FindNode(avtDataset_p &, int, int, double *);
    static vtkDataArray      *GetArray(avtDataset_p &, const char *, int,
                                       avtCentering &);
    static avtCentering       GetVariableCentering(avtDataset_p&, const char*);
};


#endif


