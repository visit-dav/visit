// ************************************************************************* //
//                                avtDataset.C                               //
// ************************************************************************* //


#include <avtDataset.h>

#include <float.h>

#include <avtCommonDataFunctions.h>
#include <avtCompactTreeFilter.h>
#include <avtDataRepresentation.h>
#include <avtDataSetWriter.h>
#include <avtSourceFromAVTDataset.h>


// ****************************************************************************
//  Method: avtDataset constructor
//
//  Arguments:
//      src     The upstream object for this dataset.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Tue May 22 20:56:19 PDT 2001
//    Rewrote to account for new base type.  Blew away antiquated comments.
//
// ****************************************************************************

avtDataset::avtDataset(avtDataObjectSource *src)
    : avtDataObject(src)
{
    dataTree = new avtDataTree();
}


// ****************************************************************************
//  Method: avtDataset constructor
//
//  Arguments:
//      ds      A vtk dataset.
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
// ****************************************************************************

avtDataset::avtDataset(vtkDataSet *ds)
    : avtDataObject(NULL)
{
    dataTree = new avtDataTree(ds, 0);
}

// ****************************************************************************
//  Method: avtDataset copy constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   23Apr03 
//
// ****************************************************************************

avtDataset::avtDataset(avtDataset_p in, bool dontCopyData)
   : avtDataObject(in->source)
{
   info.Copy(in->GetInfo());
   dataTree = new avtDataTree(in->dataTree, dontCopyData);
}



// ****************************************************************************
//  Method: avtDataset::GetNumberOfCells
//
//  Purpose:
//      Returns number of cells in the dataset. Will count only polygons if
//      polysOnly is true
//
//  Programmer:  Mark C. Miller 
//  Creation:    November 5, 2003 
//
// ****************************************************************************

int
avtDataset::GetNumberOfCells(bool polysOnly) const
{
   int topoDim = -1;

   // we only care about topoDim if we're counting polys-only
   if (polysOnly)
      topoDim = GetInfo().GetAttributes().GetTopologicalDimension();

   return dataTree->GetNumberOfCells(topoDim, polysOnly);
}

// ****************************************************************************
//  Method: avtDataset::SetDataTree
//
//  Purpose:
//      Sets the domain to be the argument.
//
//  Arguments:
//      dr       A representaiton of the data.
//
//  Programmer:  Hank Childs
//  Creation:    July 24, 2000
//
//    Jeremy Meredith, Tue Sep 26 09:22:26 PDT 2000
//    Made this take an avtDomain.
//
//    Kathleen Bonnell, Fri Feb  9 17:11:18 PST 2001 
//    Made this set domain as an avtDomainTree.
//
//    Kathleen Bonnell, Thu Apr  5 14:50:11 PDT 2001 
//    Renamed this method as SetDataTree to reflect that data
//    is no longer stored as an array of avtDomainTrees but as
//    a single avtDataTree.  Removed argument specifying domain number.
//
// ****************************************************************************

void
avtDataset::SetDataTree(avtDataRepresentation &dr)
{
    dataTree = new avtDataTree(dr);
}


// ****************************************************************************
//  Method: avtDataset::SetDataTree
//
//  Purpose:
//      Sets the data tree to be the argument.
//
//  Arguments:
//      dt       A data tree.
//
//  Programmer:  Hank Childs
//  Creation:    July 24, 2000
//
//    Jeremy Meredith, Tue Sep 26 09:22:26 PDT 2000
//    Made this take an avtDomain.
//
//    Kathleen Bonnell, Thu Feb  1 13:49:46 PST 2001 
//    Made this take an avtDomainTree.
//
//    Kathleen Bonnell, Thu Apr  5 14:00:43 PDT 2001 
//    Renamed this method as SetDataTree to reflect that data
//    is no longer stored as an array of avtDomainTrees but as
//    a single avtDataTree.  Removed argument specifying domain number. 
//
// ****************************************************************************

void
avtDataset::SetDataTree(avtDataTree_p dt)
{
    dataTree = dt;
}


// ****************************************************************************
//  Method: avtDataset::Instance
//
//  Purpose:
//      Creates an instance of an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2001
//
// ****************************************************************************

avtDataObject *
avtDataset::Instance(void)
{
    avtDataObjectSource *src = NULL;
    avtDataset *ds = new avtDataset(src);
    return ds;
}


// ****************************************************************************
//  Method: avtDataset::DerivedCopy
//
//  Purpose:
//      Copies the argument to this object.  
//
//  Arguments:
//      dob     The dataset to copy.
//
//  Notes:      This should only be called by the base class' copy 
//              routine (avtDataObject::Copy).
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jul 31 10:07:07 PDT 2001
//    Copy the data tree rather than sharing a reference.
//
//    Hank Childs, Wed Sep 11 09:19:41 PDT 2002
//    Do not assume that the data tree is non-NULL.
//
// ****************************************************************************

void
avtDataset::DerivedCopy(avtDataObject *dob)
{
    avtDataset *ds = (avtDataset *) dob;
    if (*(ds->dataTree) != NULL)
    {
        dataTree = new avtDataTree(ds->dataTree);
    }
    else
    {
        dataTree = NULL;
    }
}


// ****************************************************************************
//  Method: avtDataset::DerivedMerge
//
//  Purpose:
//      Merges the argument to this object.  
//
//  Arguments:
//      dob     The dataset to merge.
//
//  Notes:      This should only be called by the base class' merge 
//              routine (avtDataObject::Merge).
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Sep 11 09:19:41 PDT 2002
//    Do not assume that the data tree is non-NULL.
//
// ****************************************************************************

void
avtDataset::DerivedMerge(avtDataObject *dob)
{
    avtDataset *ds = (avtDataset *) dob;
    if (*(ds->dataTree) != NULL && *dataTree != NULL)
    {
        dataTree->Merge(ds->dataTree);
    }
    else if (*(ds->dataTree) != NULL)
    {
        dataTree = ds->dataTree;
    }
}


// ****************************************************************************
//  Method: avtDataset::WriteTreeStructure
//
//  Purpose:
//    Writes the data tree's structure to the given output stream. 
//
//  Arguments:
//    os       The output stream.
//    indent   The starting indent (optional).
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 18, 2001 
//
//  Modifications:
//
//    Hank Childs, Wed Sep 11 09:19:41 PDT 2002
//    Do not assume that the data tree is non-NULL.
//
// ****************************************************************************

void
avtDataset::WriteTreeStructure(ostream & os, int indent)
{
    if (*dataTree != NULL)
    {
        dataTree->WriteTreeStructure(os, indent);
    }
    else
    {
        os << "NULL data tree" << endl;
    }
}

// ****************************************************************************
//  Method: avtDataset::Compact
//
//  Purpose:
//      Compacts the dataset.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Oct 12 11:38:41 PDT 2001
//    Set flag for compact tree filter specifying that execution does not
//    depend on DLB.
//
// ****************************************************************************

void
avtDataset::Compact(void)
{
    //
    // Create an identical dataset.  We need an identical one because we can't
    // send 'this' into avtSourceFromAVTDataset (ref_ptr issues).
    //
    avtDataset_p ds = (avtDataset *) Clone();
    ds->SetSource(source);

    //
    // Make a fake-o pipeline.
    //
    avtSourceFromAVTDataset *src = new avtSourceFromAVTDataset(ds);

    //
    // The compact tree filter is what will do the magic.
    //
    avtCompactTreeFilter *cf = new avtCompactTreeFilter;
    cf->SetInput(src->GetOutput());
    cf->DLBDependentExecutionOFF();

    //
    // Force an execution.  This is ugly.
    //
    cf->Update(cf->GetGeneralPipelineSpecification());

    //
    // Now copy the compact filter's output's tree into 'this'.
    //
    avtDataset_p ds1 = cf->GetTypedOutput();
    SetDataTree(ds1->GetDataTree());

    //
    // Clean up memory.
    //
    delete src;
    delete cf;
}


// ****************************************************************************
//  Method: avtDataset::InstantiateWriter
//
//  Purpose:
//      Instantiates a writer that is appropriate for an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   October 1, 2001
//
// ****************************************************************************

avtDataObjectWriter *
avtDataset::InstantiateWriter(void)
{
    return new avtDataSetWriter;
}


// ****************************************************************************
//  Method: avtDataset::ReleaseData
//
//  Purpose:
//      Releases this dataset's data tree.
//
//  Programmer: Hank Childs
//  Creation:   November 5, 2001
//
// ****************************************************************************

void
avtDataset::ReleaseData(void)
{
    dataTree = NULL;
}


// ****************************************************************************
//  Method: avtDataset::SetActiveVariable
//
//  Purpose:
//      Sets the active variable of the dataset.
//
//  Arguments:
//      name    The name of the new active variable.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

void
avtDataset::SetActiveVariable(const char *name)
{
    if (*dataTree != NULL)
    {
        SetActiveVariableArgs args;
        args.varname = name;
        bool success;
        dataTree->Traverse(CSetActiveVariable, (void *) &args, success);
    }
}


