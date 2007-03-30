// ************************************************************************* //
//                          avtSourceFromDataset.C                           //
// ************************************************************************* //

#include <avtSourceFromDataset.h>

#include <vtkDataSet.h>

#include <avtDataset.h>
#include <avtSILRestrictionTraverser.h>

#include <BadDomainException.h>


using     std::vector;


// ****************************************************************************
//  Method: avtSourceFromDataset constructor
//
//  Arguments:
//      d     An array of vtkDataSets that make up multiple domains.
//      nd    The number of datasets in d.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jun  1 16:40:06 PDT 2001
//    Overhauled method, removed previous comments.
//
// ****************************************************************************

avtSourceFromDataset::avtSourceFromDataset(vtkDataSet **d, int nd) 
{
    nDataset = nd;
    datasets = new vtkDataSet*[nDataset];
    for (int i = 0 ; i < nDataset ; i++)
    {
        datasets[i] = d[i];
        if (datasets[i] != NULL)
        {
            datasets[i]->Register(NULL);
        }
    }
}


// ****************************************************************************
//  Method: avtSourceFromDataset destructor
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jun  1 16:40:06 PDT 2001
//    Overhauled method, removed previous comments.
//
// ****************************************************************************

avtSourceFromDataset::~avtSourceFromDataset()
{
    if (datasets != NULL)
    {
        for (int i = 0 ; i < nDataset ; i++)
        {
            if (datasets[i] != NULL)
            {
                datasets[i]->Delete();
            }
        }
        delete [] datasets;
    }
}


// ****************************************************************************
//  Method: avtSourceFromDataset::FetchDataset
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
//  Creation:   June 1, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 16:35:30 PST 2002
//    Used SIL restriction traverser.
//
//    Mark C. Miller, 17Mar03
//    eliminated use of "spec" argument and SIL restrction. 
//    the data specification abstraction doesn't make much sense here because
//    we're basically dealing with raw VTK dataset objects clothed as AVT
//    datasets
// ****************************************************************************

bool
avtSourceFromDataset::FetchDataset(avtDataSpecification_p spec,
                                   avtDataTree_p &tree)
{
    int  i;

    //
    // It needs to have "chunk ids".  Just make some up so we can meet the
    // interface.
    //
    int *chunks = new int[nDataset];
    for (i = 0 ; i < nDataset ; i++)
        chunks[i] = i; 
    
    tree = new avtDataTree(nDataset, datasets, chunks);
    delete [] chunks;

    return false;
}
