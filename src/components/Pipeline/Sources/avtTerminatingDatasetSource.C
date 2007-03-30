// ************************************************************************* //
//                         avtTerminatingDatasetSource.C                     //
// ************************************************************************* //

#include <avtTerminatingDatasetSource.h>

#include <float.h>

#include <vtkDataSet.h>

#include <avtCommonDataFunctions.h>
#include <avtDataAttributes.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtTerminatingDatasetSource::FetchData
//
//  Purpose:
//      Defines FetchData, a method that is called when a terminating data
//      object does an Update.  This layer defines how the information obtained
//      (a data tree) should be associated with the data object.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jul 17 10:10:25 PDT 2001
//    Better handling of an empty data specification.
//
//    Hank Childs, Tue Sep  4 15:25:38 PDT 2001
//    Swap out extent mechanism to formally handle the notion of cumulative
//    extents.
//
//    Hank Childs, Fri Sep  7 17:58:58 PDT 2001
//    Used doubles instead of floats when getting VTK extents.
//
//    Hank Childs, Thu Oct 18 13:32:04 PDT 2001
//    Since VTK assumes vectors are 3D, allocate enough memory to make sure
//    it does do an array bounds write.
//
//    Hank Childs, Thu Oct 18 16:17:22 PDT 2001
//    Perform a dataset verification phase.
//
//    Hank Childs, Tue Nov 13 12:05:57 PST 2001
//    Only perform dataset verification if this is a "real" terminating source.
//
//    Hank Childs, Thu Feb  7 11:51:46 PST 2002
//    Still execute even if we have an empty SIL.  This will allow databases
//    to do collective communication routines.
//
// ****************************************************************************

bool
avtTerminatingDatasetSource::FetchData(avtDataSpecification_p spec)
{
    bool rv = false;
    rv = FetchDataset(spec, GetDataTree());
    avtDataTree_p tree = GetDataTree();

    int   i;
    bool foundExtents = false;
    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();

    if (!ArtificialPipeline())
    {
        //
        // Get the spatial extents and merge them back in.
        //
        double se[6];
        for (i = 0 ; i < 3 ; i++)
        {
            se[2*i + 0] = +DBL_MAX;
            se[2*i + 1] = -DBL_MAX;
        }
        tree->Traverse(CGetSpatialExtents, se, foundExtents);
        if (foundExtents)
        {
            atts.GetCumulativeTrueSpatialExtents()->Merge(se);
        }
    
        //
        // Get the data extents and merge them back in.
        //
        double de[6];
        for (i = 0 ; i < 3 ; i++)
        {
            de[2*i + 0] = +DBL_MAX;
            de[2*i + 1] = -DBL_MAX;
        }
        tree->Traverse(CGetDataExtents, de, foundExtents);
        if (foundExtents)
        {
            atts.GetCumulativeTrueDataExtents()->Merge(de);
        }

        verifier.VerifyDataTree(tree);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtTerminatingDatasetSource::MergeExtents
//
//  Purpose:
//      Merges extents into the output.  This is used by databases that
//      do subselection before handing back the data tree (like a material
//      select) and want to give extents.
//
//  Arguments:
//      ds      The VTK dataset to merge into the extents.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep  7 17:58:58 PDT 2001
//    Better support for vector variables.
//
//    Hank Childs, Thu Oct 11 13:11:26 PDT 2001
//    Allow NULL input.
//
//    Hank Childs, Thu Oct 18 13:32:04 PDT 2001
//    Since VTK assumes vectors are 3D, allocate enough memory to make sure
//    it does do an array bounds write.
//
// ****************************************************************************

void
avtTerminatingDatasetSource::MergeExtents(vtkDataSet *ds)
{
    if (ds == NULL)
    {
        return;
    }

    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();

    float bounds[6];
    ds->GetBounds(bounds);

    double dbounds[6];
    dbounds[0] = bounds[0];
    dbounds[1] = bounds[1];
    dbounds[2] = bounds[2];
    dbounds[3] = bounds[3];
    dbounds[4] = bounds[4];
    dbounds[5] = bounds[5];
    atts.GetCumulativeTrueSpatialExtents()->Merge(dbounds);

    int dim = GetOutput()->GetInfo().GetAttributes().GetVariableDimension();
    if (dim == 2)
    {
        // VTK will treat 2D vectors as 3D, so overallocate a little.
        dim = 3;
    }
    double *dextents = new double[2*dim];
    GetDataRange(ds, dextents, dim);

    atts.GetCumulativeTrueDataExtents()->Merge(dextents);

    delete [] dextents;
}


