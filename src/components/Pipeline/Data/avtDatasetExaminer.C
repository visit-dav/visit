// ************************************************************************* //
//                           avtDatasetExaminer.C                            //
// ************************************************************************* //

#include <avtDatasetExaminer.h>

#include <float.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDatasetExaminer::GetNumberOfZones
//
//  Purpose:
//    Gets the number of zones in the data tree.
//
//  Returns:
//    The number of zones (cells) in the underlying vtk dataset of the
//    avtDataTree.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    January 04, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Apr  5 13:34:11 PDT 2001
//    Reflect data now stored as avtDataTree instead of array of
//    avtDomainTrees.  Use avtDataTree::Traverse method instead of
//    retrieving vtkDataSets individually.
//
//    Hank Childs, Fri Mar 15 17:18:00 PST 2002
//    Moved from class avtDataset.
//
// ****************************************************************************
 
int
avtDatasetExaminer::GetNumberOfZones(avtDataset_p &ds)
{
    avtDataTree_p dataTree = ds->dataTree;

    int numZones = 0;
    if (*dataTree != NULL)
    {
        bool dummy;
        dataTree->Traverse(CGetNumberOfZones, &numZones, dummy);
    }
    return numZones;
}


// ****************************************************************************
//  Method: avtDatasetExaminer::GetVariableList
//
//  Purpose:
//      Gets a list of variables the dataset is defined over.
//
//  Returns:     A list of variables the dataset is defined over.
//
//  Programmer:  Hank Childs
//  Creation:    November 14, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Mar 15 17:18:00 PST 2002
//    Moved from class avtDataset.
//
// ****************************************************************************
 
void
avtDatasetExaminer::GetVariableList(avtDataset_p &ds, VarList &vl)
{
    avtDataTree_p dataTree = ds->dataTree;

    bool success = false;
    if (*dataTree != NULL)
    {
        dataTree->Traverse(CGetVariableList, &vl, success);    }
 
    if (!success)
    {
        debug1 << "HIGHLY UNUSUAL: the data tree was not able to determine "
               << "its variable list." << endl;
        vl.nvars = 0;
    }
}


// ****************************************************************************
//  Method: avtDatasetExaminer::GetSpatialExtents
//
//  Purpose:
//      Gets the spatial extents of the dataset.
//
//  Arguments:
//      se        A place to put the spatial extents
//
//  Returns:      Whether or not the extents were obtained.
//
//  Programmer:   Hank Childs
//  Creation:     January 5, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  9 17:11:18 PST 2001
//    Added call to GetDomain, as domains are stored as avtDomainTree.
//
//    Hank Childs, Sun Mar 25 15:25:50 PST 2001
//    Added debug statement for case where there are no extents.
//
//    Kathleen Bonnell, Fri Apr 13 16:27:15 PDT 2001
//    Changed to utilize avtDataTree::Traverse method.
//
//    Hank Childs, Tue Jul 17 13:34:28 PDT 2001
//    Return whether or not we got the extents.
//
//    Hank Childs, Fri Sep  7 18:30:33 PDT 2001
//    Changed argument from float to double.  Also made the declaration be
//    a double * instead of a double[6], since some compilers struggle with
//    the (non-)distinction.
//
//    Hank Childs, Fri Mar 15 17:18:00 PST 2002
//    Moved from class avtDataset.
//
// ****************************************************************************
 
bool
avtDatasetExaminer::GetSpatialExtents(avtDataset_p &ds, double *se)
{
    avtDataTree_p dataTree = ds->dataTree;

    int   i;
 
    bool foundExtents = false;
    for (i = 0 ; i < 3 ; i++)
    {
        se[2*i + 0] = +DBL_MAX;
        se[2*i + 1] = -DBL_MAX;
    }
 
    if ( *dataTree != NULL )
    {
        dataTree->Traverse(CGetSpatialExtents, se, foundExtents);
    }
 
    if (!foundExtents)
    {
        debug1 << "Unable to determine spatial extents -- dataset needs an "
               << "update" << endl;
    }
 
    return foundExtents;
}


// ****************************************************************************
//  Method: avtDatasetExaminer::GetDataExtents
//
//  Purpose:
//      Gets the data extents of the dataset.
//
//  Arguments:
//      de        A place to put the data extents
//
//  Returns:      Whether or not the extents were obtained.
//
//  Programmer:   Hank Childs
//  Creation:     January 5, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  9 17:11:18 PST 2001
//    Added call to GetDomain, as domains are stored as avtDataTree.
//
//    Hank Childs, Sun Mar 25 15:25:50 PST 2001
//    Added debug statement for case where there are no extents.
//
//    Kathleen Bonnell, Fri Apr 13 16:27:15 PDT 2001
//    Changed to utilize avtDataTree::Traverse method.
//
//    Hank Childs, Tue Jul 17 13:34:28 PDT 2001
//    Return whether or not we got the extents.
//
//    Hank Childs, Fri Sep  7 18:30:33 PDT 2001
//    Changed argument from float to double.  Also made the declaration be
//    a double * instead of a double[2], since some compilers struggle with
//    the (non-)distinction.  Removed assumptions that we have scalar data.
//
//    Hank Childs, Fri Mar 15 17:18:00 PST 2002
//    Moved from class avtDataset.
//
// ****************************************************************************
 
bool
avtDatasetExaminer::GetDataExtents(avtDataset_p &ds, double *de)
{
    avtDataTree_p dataTree = ds->dataTree;

    bool foundExtents = false;
    int dim = ds->GetInfo().GetAttributes().GetVariableDimension();
    for (int i = 0 ; i < dim ; i++)
    {
        de[2*i]   = +DBL_MAX;
        de[2*i+1] = -DBL_MAX;
    }
 
    if ( *dataTree != NULL )
    {
        dataTree->Traverse(CGetDataExtents, de, foundExtents);
    }
 
    if (!foundExtents)
    {
        debug1 << "Unable to determine data extents -- dataset needs an "
               << "update" << endl;
    }
 
    return foundExtents;
}

// ****************************************************************************
//  Method: avtDatasetExaminer::GetDataMagnitudeExtents
//
//  Purpose:
//      Gets the data extents of the magnitude of the dataset.
//
//  Arguments:
//      de        A place to put the data extents
//
//  Returns:      Whether or not the extents were obtained.
//
//  Programmer:   Brad Whitlock
//  Creation:     Wed Dec 4 11:46:46 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
 
bool
avtDatasetExaminer::GetDataMagnitudeExtents(avtDataset_p &ds, double *de)
{
    avtDataTree_p dataTree = ds->dataTree;

    bool foundExtents = false;
    de[0] = +DBL_MAX;
    de[1] = -DBL_MAX;
 
    if ( *dataTree != NULL )
    {
        dataTree->Traverse(CGetDataMagnitudeExtents, de, foundExtents);
    }
 
    if (!foundExtents)
    {
        debug1 << "Unable to determine data magnitude extents -- dataset "
               << "needs an update" << endl;
    }
 
    return foundExtents;
}

// ****************************************************************************
//  Method: avtDatasetExaminer::GetNodeCenteredDataExtents
//
//  Purpose:
//      Gets the data extents associated with nodes of the dataset.
//
//  Arguments:
//      de        A place to put the data extents
//
//  Returns:      Whether or not the extents were obtained.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     March 27, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Apr 13 16:27:15 PDT 2001
//    Changed to utilize avtDataTree::Traverse method.
//
//    Hank Childs, Tue Jul 17 13:34:28 PDT 2001
//    Return whether or not we got the extents.
//
//    Hank Childs, Fri Sep  7 18:30:33 PDT 2001
//    Changed argument from float to double.  Also made the declaration be
//    a double * instead of a double[2], since some compilers struggle with
//    the (non-)distinction.  Removed assumptions that we have scalar data.
//
//    Hank Childs, Fri Mar 15 17:18:00 PST 2002
//    Moved from class avtDataset.
//
// ****************************************************************************
 
bool
avtDatasetExaminer::GetNodeCenteredDataExtents(avtDataset_p &ds, double *de)
{
    avtDataTree_p dataTree = ds->dataTree;

    bool foundExtents = false;
    int dim = ds->GetInfo().GetAttributes().GetVariableDimension();
    for (int i = 0 ; i < dim ; i++)
    {
        de[2*i]   = +DBL_MAX;
        de[2*i+1] = -DBL_MAX;
    }
 
    if ( *dataTree != NULL )
    {
       dataTree->Traverse(CGetNodeCenteredDataExtents, de, foundExtents);
    }
 
    if (!foundExtents)
    {
        debug1 << "Unable to determine node centered extents -- "
               << "either the dataset has no node-centered data, "
               << "or the dataset needs an update" << endl;
    }
 
    return foundExtents;
}


// ****************************************************************************
//  Method: avtDatasetExaminer::FindMaximum
//
//  Purpose:
//      Determines the minimum value and the point corresponding to that
//      maximum.
//
//  Arguments:
//      ds       A dataset.
//      pt       The point.
//      value    The value of the maximum.
//
//  Programmer:  Hank Childs
//  Creation:    March 15, 2002
//
// ****************************************************************************

void
avtDatasetExaminer::FindMaximum(avtDataset_p &ds, double *pt, double &value)
{
    FindExtremeArgs args;
    bool  success = false;
    args.value = -DBL_MAX;
    if ( *ds->dataTree != NULL )
    {
        ds->dataTree->Traverse(CFindMaximum, (void *) &args, success);
    }

    if (success)
    {
        value = args.value;
        pt[0] = args.point[0];
        pt[1] = args.point[1];
        pt[2] = args.point[2];
    }
    else
    {
        value = -DBL_MAX;
        pt[0] = 0.;
        pt[1] = 0.;
        pt[2] = 0.;
    }
}


// ****************************************************************************
//  Method: avtDatasetExaminer::FindMinimum
//
//  Purpose:
//      Determines the minimum value and the point corresponding to that
//      minimum.
//
//  Arguments:
//      ds       A dataset.
//      pt       The point.
//      value    The value of the minimum.
//
//  Programmer:  Hank Childs
//  Creation:    March 15, 2002
//
// ****************************************************************************

void
avtDatasetExaminer::FindMinimum(avtDataset_p &ds, double *pt, double &value)
{
    FindExtremeArgs args;
    bool  success = false;
    args.value = DBL_MAX;
    if ( *ds->dataTree != NULL )
    {
        ds->dataTree->Traverse(CFindMinimum, (void *) &args, success);
    }

    if (success)
    {
        value = args.value;
        pt[0] = args.point[0];
        pt[1] = args.point[1];
        pt[2] = args.point[2];
    }
    else
    {
        value = DBL_MAX;
        pt[0] = 0.;
        pt[1] = 0.;
        pt[2] = 0.;
    }
}


// ****************************************************************************
//  Method: avtDatasetExaminer::FindZone
//
//  Purpose:
//      Determines the location of a zone.
//
//  Arguments:
//      ds       A dataset.
//      dom      A domain number.
//      zone     A zone number.
//      pt       A location to put the point in.
//
//  Returns:     true if it located the zone, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    March 15, 2002
//
// ****************************************************************************

bool
avtDatasetExaminer::FindZone(avtDataset_p &ds, int dom, int zone, double *pt)
{
    LocateObjectArgs args;
    bool  success = false;
    args.domain = dom;
    args.index  = zone;
    if ( *ds->dataTree != NULL )
    {
        ds->dataTree->Traverse(CLocateZone, (void *) &args, success);
    }

    if (success)
    {
        pt[0] = args.point[0];
        pt[1] = args.point[1];
        pt[2] = args.point[2];
    }

    return success;
}


// ****************************************************************************
//  Method: avtDatasetExaminer::FindNode
//
//  Purpose:
//      Determines the location of a node.
//
//  Arguments:
//      ds       A dataset.
//      dom      A domain number.
//      zone     A zone number.
//      pt       A location to put the point in.
//
//  Returns:     true if it located the zone, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    March 15, 2002
//
// ****************************************************************************

bool
avtDatasetExaminer::FindNode(avtDataset_p &ds, int dom, int zone, double *pt)
{
    LocateObjectArgs args;
    bool  success = false;
    args.domain = dom;
    args.index  = zone;
    if ( *ds->dataTree != NULL )
    {
        ds->dataTree->Traverse(CLocateNode, (void *) &args, success);
    }

    if (success)
    {
        pt[0] = args.point[0];
        pt[1] = args.point[1];
        pt[2] = args.point[2];
    }

    return success;
}


