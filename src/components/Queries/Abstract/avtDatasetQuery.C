// ************************************************************************* //
//                            avtDatasetQuery.C                              //
// ************************************************************************* //

#include <avtDatasetQuery.h>
#include <vtkDataSet.h>

#include <DebugStream.h>
#include <BadIndexException.h>
#include <ImproperUseException.h>
#include <NoInputException.h>
#include <avtSourceFromAVTDataset.h>


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
        avtDataSpecification_p oldSpec = dob->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec->GetVariable(), queryAtts.GetTimeStep(), 
                                 oldSpec->GetRestriction());

        if (!OriginalData()) 
        {
            newDS->GetRestriction()->SuspendCorrectnessChecking();
            newDS->GetRestriction()->TurnOnAll();
            for (int i = 0; i < silUseSet.size(); i++)
            {
               if (silUseSet[i] == 0)
                   newDS->GetRestriction()->TurnOffSet(i);
            }
            newDS->GetRestriction()->EnableCorrectnessChecking();
        }
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


// ****************************************************************************
//  Method: avtDatasetQuery::PerformQueryInTime
//
//  Purpose:
//    Perform the requested query at the give timesteps. 
//
//  Arguments:
//    qA         The attributes controlling this query.
//    timeSteps  The timespeps for this query.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 24, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr  2 08:51:17 PST 2004
//    Changed args. 
//
// ****************************************************************************

void
avtDatasetQuery::PerformQueryInTime(QueryAttributes *qA, const int startT ,
                                    const int endT, const int stride,
                                    const int timeType, doubleVector &times)
{
    queryAtts = *qA;
  
    int nFrames = (int) ceil((endT -startT)/stride) + 1; 

    //
    // Ensure that the specified endTime is included,
    // regardless of the stride.
    //
    int actualEnd = startT + nFrames *stride;
    if (actualEnd < endT)
        actualEnd = endT + stride;

    Init(nFrames);

    UpdateProgress(0, 0);
    doubleVector qRes;

    int i, count = 0;

    //
    //  For now, each processor process all times, because they have been 
    //  domain-balanced already.  Need to work out an alternate system 
    //  for balancing by time ...  perhaps when nDomains < nProcs.
    //
    avtDataObject_p origInput;
    avtDataObject_p input = GetInput();
    CopyTo(origInput, input);
    for (i =  startT; i <= actualEnd; i+=stride)
    {
        if (i <= endT)
            queryAtts.SetTimeStep(i);
        else 
            queryAtts.SetTimeStep(endT);

        avtDataObject_p dob = ApplyFilters(origInput);

        //
        // Reset the input so that we have access to the data tree. 
        //
        SetTypedInput(dob);

        //
        // Retrieve the correct time value that will be used for
        // the x-axis.
        //
        double tval;  
        switch(timeType)
        {
            case 0: // cycle
                tval = (double) GetInput()->GetInfo().GetAttributes().GetCycle();
                break;
            case 1: // time
                tval = GetInput()->GetInfo().GetAttributes().GetTime();
                break;
            case 2: // timestep
            default: // timestep
                if (i <= endT)
                    tval = (double)i;
                else 
                    tval = (double)endT;
                break;
        }
        times.push_back(tval);

        PreExecute();
        avtDataTree_p tree = GetInputDataTree();
        
        Execute(tree);
        UpdateProgress(count++ , nFrames);
        PostExecute();
      
        //
        // Store the query results ... currently only one result.
        //
        qRes.push_back(resValue[0]);
    }
    queryAtts.SetResultsValue(qRes);
    UpdateProgress(1, 0);
    *qA = queryAtts;
}

