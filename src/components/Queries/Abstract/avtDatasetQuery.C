// ************************************************************************* //
//                            avtDatasetQuery.C                              //
// ************************************************************************* //

#include <avtDatasetQuery.h>

#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>

#include <vtkDataSet.h>

#include <BadIndexException.h>
#include <DebugStream.h>



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
//    Kathleen Bonnell, Fri Jul 11 16:19:32 PDT 2003 
//    Initialize value. 
//
// ****************************************************************************

avtDatasetQuery::avtDatasetQuery() : avtDatasetSink() 
{
    currentNode = 0;
    totalNodes = 0;
    // always have 1 value.
    resValue.push_back(0.);
}


// ****************************************************************************
//  Method: avtDatasetQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetQuery::~avtDatasetQuery()
{
    ;
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
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Set queryAtts results value. 
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
    queryAtts.SetResultsMessage(resMsg);
    queryAtts.SetResultsValue(resValue);

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
        if (!timeVarying)
        {
            currentNode++;
            UpdateProgress(currentNode, totalNodes);
        }
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
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 15:52:54 PST 2004
//    Allow for time-varying case. 
//
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
//    Kathleen Bonnell, Mon Jan  3 15:12:19 PST 2005 
//    Modify timeVarying case. 
//
// ****************************************************************************

avtDataObject_p
avtDatasetQuery::ApplyFilters(avtDataObject_p dob)
{
    if (!timeVarying)
    {
        avtDataObject_p rv;
        CopyTo(rv, dob);
        return rv;
    }
    else 
    {
        avtPipelineSpecification_p orig_pspec = dob->GetTerminatingSource()->
            GetGeneralPipelineSpecification();
        
        avtDataSpecification_p oldSpec = orig_pspec->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        avtPipelineSpecification_p pspec = 
            new avtPipelineSpecification(newDS, queryAtts.GetPipeIndex());

        avtDataObject_p rv;
        CopyTo(rv, dob);
        rv->Update(pspec);
        return rv;
    }
}

// ****************************************************************************
//  Method: avtDatasetQuery::SetResultValue
//
//  Purpose:
//    Set the result value for the specified index. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 12, 2003 
//
// ****************************************************************************

void
avtDatasetQuery::SetResultValue(const double &d, const int i)
{
    if (i < 0 || i >= resValue.size())
        EXCEPTION2(BadIndexException, i, resValue.size()-1)

    resValue[i] = d;
}


// ****************************************************************************
//  Method: avtDatasetQuery::GetResultValue
//
//  Purpose:
//    Return  the result value for the specified index. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 12, 2003 
//
// ****************************************************************************

double
avtDatasetQuery::GetResultValue(const int i)
{
    if (i < 0 || i >= resValue.size())
        EXCEPTION2(BadIndexException,i, resValue.size()-1)

    return resValue[i];
}

