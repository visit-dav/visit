// ************************************************************************* //
//                            avtDatasetExaminer.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_EXAMINER
#define AVT_DATASET_EXAMINER
#include <pipeline_exports.h>


#include <avtDataset.h>


class     vtkCell;


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
// ****************************************************************************

class PIPELINE_API avtDatasetExaminer
{
  public:
    static int                GetNumberOfZones(avtDataset_p &);
    static void               GetVariableList(avtDataset_p &, VarList &);
    static bool               GetSpatialExtents(avtDataset_p &, double *);
    static bool               GetDataExtents(avtDataset_p &, double *);
    static bool               GetDataMagnitudeExtents(avtDataset_p &, double *);
    static bool               GetNodeCenteredDataExtents(avtDataset_p &,
                                                         double *);
    static void               FindMinimum(avtDataset_p &, double *, double &);
    static void               FindMaximum(avtDataset_p &, double *, double &);
    static bool               FindZone(avtDataset_p &, int, int, double *);
    static bool               FindNode(avtDataset_p &, int, int, double *);
};


#endif


