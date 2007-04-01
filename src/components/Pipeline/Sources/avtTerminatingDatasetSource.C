// ************************************************************************* //
//                         avtTerminatingDatasetSource.C                     //
// ************************************************************************* //

#include <avtTerminatingDatasetSource.h>

#include <float.h>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkFieldData.h>

#include <avtCommonDataFunctions.h>
#include <avtDataAttributes.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtTerminatingDatasetSource constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTerminatingDatasetSource::avtTerminatingDatasetSource()
{
    ;
}


// ****************************************************************************
//  Method: avtTerminatingDatasetSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTerminatingDatasetSource::~avtTerminatingDatasetSource()
{
    ;
}


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
//    Hank Childs, Fri Jan  9 10:10:05 PST 2004
//    Change the arugments to the dataset verifier.
//
//    Hank Childs, Wed Feb 25 15:40:49 PST 2004
//    No longer determine extents, since that is done when the dataset is
//    populated (through MergeExtents).
//
// ****************************************************************************

bool
avtTerminatingDatasetSource::FetchData(avtDataSpecification_p spec)
{
    bool rv = false;
    rv = FetchDataset(spec, GetDataTree());
    avtDataTree_p tree = GetDataTree();

    if (!ArtificialPipeline())
    {
        int nleaves = 0;
        vtkDataSet **ds = tree->GetAllLeaves(nleaves);
        vector<int> domains;
        tree->GetAllDomainIds(domains);
        verifier.VerifyDatasets(nleaves, ds, domains);
        delete [] ds;
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
//    Hank Childs, Tue Jul 29 16:33:57 PDT 2003
//    Make use of cached bounds.
//
//    Hank Childs, Tue Feb 24 14:38:47 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 10:07:35 PST 2004 
//    DataExtents now always have only 2 components. 
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
    if (ds->GetFieldData()->GetArray("avtOriginalBounds") != NULL)
    {
        vtkDataArray *arr = ds->GetFieldData()->GetArray("avtOriginalBounds");
        for (int i = 0 ; i < 6 ; i++)
            bounds[i] = arr->GetTuple1(i);
    }
    else
    {
        ds->GetBounds(bounds);
    }

    double dbounds[6];
    dbounds[0] = bounds[0];
    dbounds[1] = bounds[1];
    dbounds[2] = bounds[2];
    dbounds[3] = bounds[3];
    dbounds[4] = bounds[4];
    dbounds[5] = bounds[5];
    atts.GetCumulativeTrueSpatialExtents()->Merge(dbounds);

    int nvars = atts.GetNumberOfVariables();
    double dextents[2];
    for (int i = 0 ; i < nvars ; i++)
    {
        const char *vname = atts.GetVariableName(i).c_str();
        if (atts.GetTrueDataExtents(vname)->HasExtents())
        {
            //
            // There is no point in walking through the data and determining
            // what the cumulative extents are -- we know them already.
            //
            continue;
        }

        GetDataRange(ds, dextents, vname);
        atts.GetCumulativeTrueDataExtents(vname)->Merge(dextents);
    }
}


