// ************************************************************************* //
//                        avtSourceFromAVTDataset.C                          //
// ************************************************************************* //

#include <avtSourceFromAVTDataset.h>

#include <avtCommonDataFunctions.h>
#include <avtDataset.h>
#include <avtSILRestrictionTraverser.h>

#include <TimingsManager.h>


using     std::vector;


avtDataObject_p   DataObjectFromDataset(avtDataset_p);


// ****************************************************************************
//  Method: avtSourceFromAVTDataset constructor
//
//  Arguments:
//      d     An avtdataset.
//
//  Programmer: Hank Childs
//  Creation:   June 19, 2001
//
// ****************************************************************************

avtSourceFromAVTDataset::avtSourceFromAVTDataset(avtDataset_p d)
    : avtInlinePipelineSource(DataObjectFromDataset(d))
{
    tree = d->GetDataTree();
    GetTypedOutput()->GetInfo().Copy(d->GetInfo());
}


// ****************************************************************************
//  Method: avtSourceFromAVTDataset::FetchDataset
//
//  Purpose:
//      Gets all of the domains specified in the domain list.
//
//  Arguments:
//      spec    A specification of which domains to use.
//      tree    The output data tree.
//
//  Returns:    Whether or not this dataset is different from the last time it
//              was updated.
//       
//  Programmer: Hank Childs
//  Creation:   June 19, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Sep 16 13:46:40 PDT 2001
//    Added code to prevent trees from getting unnecessarily large.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001
//    Restructured to make use of new tree method PruneTree. 
//
//    Jeremy Meredith, Hank Childs, Fri Jul 26 13:57:48 PDT 2002
//    Some use cases (e.g. calling this method on the viewer)
//    do not require a SIL restriction, so allow for this now.
//
//    Hank Childs, Fri Nov 22 16:34:04 PST 2002
//    Use the SIL restriction traverser.
//
// ****************************************************************************

bool
avtSourceFromAVTDataset::FetchDataset(avtDataSpecification_p spec,
                                      avtDataTree_p &outtree)
{
    int timingsHandle = visitTimer->StartTimer();

    vector<int> list;
    if (spec->GetSIL().useRestriction)
    {
        avtSILRestrictionTraverser trav(spec->GetRestriction());
        trav.GetDomainList(list);
        outtree = tree->PruneTree(list);
    }
    else
    {
        int dataChunk = spec->GetSIL().dataChunk;
        if (dataChunk >= 0)
        {
            list.push_back(dataChunk);
            outtree = tree->PruneTree(list);
        }
        else
        {
            outtree = tree;
        }
    }

    visitTimer->StopTimer(timingsHandle, "Fetching dataset AVT dataset");

    return false;
}


// ****************************************************************************
//  Function: DataObjectFromDataset
//
//  Purpose:
//      Creates a pointer typed to the base class.  This should happen free
//      with C++, but does not because of reference pointers.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 15:24:42 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtDataObject_p
DataObjectFromDataset(avtDataset_p ds)
{
    avtDataObject_p rv;
    CopyTo(rv, ds);

    return rv;
}


