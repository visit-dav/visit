// ************************************************************************* //
//                                avtStreamer.C                              //
// ************************************************************************* //

#include <vtkDataSet.h>

#include <vtkDataSetWriter.h>

#include <avtDataTree.h>
#include <avtParallel.h>
#include <avtStreamer.h>

bool avtStreamer::debugDump = false;


// ****************************************************************************
//  Method: avtStreamer constructor
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
// ****************************************************************************

avtStreamer::avtStreamer()
{
    lastDataset = NULL;
}


// ****************************************************************************
//  Method: avtStreamer destructor
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
// ****************************************************************************

avtStreamer::~avtStreamer()
{
    if (lastDataset != NULL)
    {
        lastDataset->Delete();
        lastDataset = NULL;
    }
}


// ****************************************************************************
//  Method: avtStreamer::ManageMemory
//
//  Purpose:
//      Is a resting spot for the last dataset a filter has processed.  Many
//      filters have a problem of what to do with their datasets when they
//      return from ExecuteData, since they have bumped the reference count
//      and decrementing it before returning would destruct the data before
//      it is returned.  This is a mechanism to store it so the derived types
//      can forget about it.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu May  6 11:35:15 PDT 2004
//    Made sure not to delete the last reference to something if it was the
//    thing we were about to add a reference to.  In other words, if the
//    new dataset is the same as the old one, noop.
//
// ****************************************************************************

void
avtStreamer::ManageMemory(vtkDataSet *ds)
{
    if (ds == lastDataset)
        return;

    if (lastDataset != NULL)
    {
        lastDataset->Delete();
    }

    lastDataset = ds;
    if (lastDataset != NULL)
    {
        lastDataset->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtStreamer::ReleaseData
//
//  Purpose:
//      Releases any problem-size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
avtStreamer::ReleaseData(void)
{
    avtDataTreeStreamer::ReleaseData();
    ManageMemory(NULL);  // Cleans out any stored datasets.
}


// ****************************************************************************
//  Method: avtStreamer::ExecuteDataTree
//
//  Purpose:
//      Defines the pure virtual function executedomaintree.  
//      Serves as a wrapper for the ExecuteDomain method.
//
//  Arguments:
//      ds      The vtkDataSet to pass to the derived type.
//      dom     The domain number of the input dataset.
//      label   The label associated with this datset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 9, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Made this method return avtDataTree.
//
//    Kathleen Bonnell, Wed Sep 19 13:35:35 PDT 2001 
//    Added string argument so that labels will get passed on. 
// 
//    Hank Childs, Fri Oct 19 10:56:55 PDT 2001
//    Allow for derived types to return NULL.
//
//    Hank Childs, Wed Sep 11 09:17:46 PDT 2002
//    Pass the label down to the derived types as well.
//
//    Hank Childs, Mon Jun 27 10:02:55 PDT 2005
//    Choose better file names when doing a "-dump" in parallel.
//
//    Hank Childs, Tue Jul  5 09:41:28 PDT 2005
//    Fix cut-n-paste bug with last change.
//
// ****************************************************************************

avtDataTree_p
avtStreamer::ExecuteDataTree(vtkDataSet* ds, int dom, std::string label)
{
    char name[1024];
    static int times = 0;

    if (debugDump)
    {
        if (PAR_Size() > 1)
        {
            int rank = PAR_Rank();
            sprintf(name, "before_%s%d.%d.vtk", GetType(), times, rank);
        }
        else
            sprintf(name, "before_%s%d.vtk", GetType(), times);
        vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
        wrtr->SetInput(ds);
        wrtr->SetFileName(name);
        wrtr->Write();
    }

    vtkDataSet *out_ds = ExecuteData(ds, dom, label);
    if (out_ds == NULL)
    {
        return NULL;
    }

    if (debugDump)
    {
        if (PAR_Size() > 1)
        {
            int rank = PAR_Rank();
            sprintf(name, "after_%s%d.%d.vtk", GetType(), times, rank);
        }
        else
            sprintf(name, "after_%s%d.vtk", GetType(), times++);
        vtkDataSetWriter *wrtr2 = vtkDataSetWriter::New();
        wrtr2->SetInput(out_ds);
        wrtr2->SetFileName(name);
        wrtr2->Write();
    }

    return new avtDataTree(out_ds, dom, label);
}
