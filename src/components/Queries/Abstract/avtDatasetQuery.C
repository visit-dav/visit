// ************************************************************************* //
//                            avtDatasetQuery.C                              //
// ************************************************************************* //

#include <avtDatasetQuery.h>
#include <vtkDataSet.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>
#include <avtSourceFromAVTDataset.h>

#if defined(_WIN32)
// GetMessage is a reserved Win32 API function name that is really a
// macro. Since we use GetMessage as certain method names, we have to
// undefine the macro. Really, we should *NEVER* use the name GetMessage.
#ifdef GetMessage
#undef GetMessage
#endif
#endif

// ****************************************************************************
//  Method: avtDatasetQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Initialize new members currentNode, totalNodes.
//
// ****************************************************************************

avtDatasetQuery::avtDatasetQuery() : avtDatasetSink() 
{
    currentNode = 0;
    totalNodes = 0;
}


// ****************************************************************************
//  Method: avtDatasetQuery::PerformQuery
//
//  Purpose:
//    Perform  the requested query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Use recursive Execute on the input data tree.  Initialize queryAtts
//    member from argument.  Move creation of artificial pipeline to
//    the derived types that actually need one.
//  
// ****************************************************************************

void
avtDatasetQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;

    Init(); 

    UpdateProgress(0, 0);
    //
    // Allow derived types to apply any necessary filters.
    //
    avtDataObject_p dob = ApplyFilters(GetInput());

    //
    // Reset the input so that we have access to the data tree. 
    //
    SetTypedInput(dob);

    PreExecute();
    avtDataTree_p tree = GetInputDataTree();
    totalNodes = tree->GetNumberOfLeaves();
    Execute(tree);

    PostExecute();

    //
    // Retrieve the query results and set the message in the atts. 
    //
    queryAtts.SetResultsMessage(GetMessage());
    UpdateProgress(1, 0);

    *qA = queryAtts;
}


// ****************************************************************************
//  Method: avtDatasetQuery::Execute
//
//  Purpose:
//    Recursive method to traverse an avtDataTree, calling Execute on the leaf
//    nodes. 
//
//  Notes:
//    Adapted from avtFilter.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 15, 2002 
//
// ****************************************************************************
void
avtDatasetQuery::Execute(avtDataTree_p inDT)
{
    if (*inDT == NULL)
    {
        return;
    }
 
    int nc = inDT->GetNChildren();
 
    if (nc <= 0 && !inDT->HasData())
    {
        return;
    }
 
    if ( nc == 0 )
    {
        //
        // There is only one dataset to process.
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();
        int dom = inDT->GetDataRepresentation().GetDomain();
 
        //
        // Setting the source to NULL for the input will break the
        // pipeline.
        //
        in_ds->SetSource(NULL);
        Execute(in_ds, dom);
        currentNode++;
        UpdateProgress(currentNode, totalNodes);
    }
    else
    {
        //
        // There is more than one input dataset to process.
        // Recurse through the children.
        //
        for (int j = 0; j < nc; j++)
        {
            if (inDT->ChildIsPresent(j))
            {
                Execute(inDT->GetChild(j));
            }
        }
    }
}


// ****************************************************************************
//  Method: avtDatasetQuery::PreExecute
//
//  Purpose:
//    Method that allows derived types to perform necessary initialization 
//    before Execute.  Stub so that derived types do not have to define. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002 
//
// ****************************************************************************

void
avtDatasetQuery::PreExecute()
{
    ;
}

// ****************************************************************************
//  Method: avtDatasetQuery::PostExecute
//
//  Purpose:
//    Method that allows derived types to perform necessary cleanup  after
//    Excute.  Stub so that derived types do not have to define.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002 
//
// ****************************************************************************

void
avtDatasetQuery::PostExecute()
{
    ;
}

// ****************************************************************************
//  Method: avtDatasetQuery::ApplyFilters
//
//  Purpose:
//    Allow derived types to apply any necessary avtFilters.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002 
//
// ****************************************************************************

avtDataObject_p
avtDatasetQuery::ApplyFilters(avtDataObject_p dob)
{
    avtDataObject_p rv;
    CopyTo(rv, dob);
    return rv;
}


