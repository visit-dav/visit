// ************************************************************************* //
//                    avtOriginalDataSpatialExtentsQuery.C                   //
// ************************************************************************* //

#include <avtOriginalDataSpatialExtentsQuery.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>
#include <avtTerminatingSource.h>
#include <ParsingExprList.h>
#include <snprintf.h>


// ****************************************************************************
//  Method: avtOriginalDataSpatialExtentsQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 31, 2006 
//
//  Modifications:
//
// ****************************************************************************

avtOriginalDataSpatialExtentsQuery::avtOriginalDataSpatialExtentsQuery() : avtDatasetQuery() 
{
}


// ****************************************************************************
//  Method: avtOriginalDataSpatialExtentsQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 31, 2006 
//
//  Modifications:
//
// ****************************************************************************
avtOriginalDataSpatialExtentsQuery::~avtOriginalDataSpatialExtentsQuery() 
{
}


// ****************************************************************************
//  Method: avtOriginalDataSpatialExtentsQuery::ApplyFilters
//
//  Purpose:
//    Makes sure the entire SIL is used. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 31, 2006 
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtOriginalDataSpatialExtentsQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataSpecification_p dspec = inData->GetTerminatingSource()->
        GetGeneralPipelineSpecification()->GetDataSpecification();

    string dbVar = ParsingExprList::GetRealVariable(
                       queryAtts.GetVariables()[0]);
    avtDataSpecification_p new_dspec = new avtDataSpecification(dspec,
                                                                dbVar.c_str());

    avtPipelineSpecification_p pspec = 
        new avtPipelineSpecification(new_dspec, queryAtts.GetPipeIndex()); 

    avtDataObject_p retObj;
    CopyTo(retObj, inData);
    retObj->Update(pspec);
    return retObj;
}


// ****************************************************************************
//  Method: avtOriginalDataSpatialExtentsQuery::PerformQuery
//
//  Purpose:
//    Perform the requested query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 31, 2006 
//
//  Modifications:
//
// ****************************************************************************

void
avtOriginalDataSpatialExtentsQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    UpdateProgress(0, 0);

    avtDataObject_p dob = ApplyFilters(GetInput());

    SetTypedInput(dob);

    avtDataset_p input = GetTypedInput();
    double extents[6] = {0., 0., 0., 0., 0., 0.};
    char msg[1024];

    bool success = avtDatasetExaminer::GetSpatialExtents(input, extents);
    UnifyMinMax(extents, 6);
    int dim = input->GetInfo().GetAttributes().GetSpatialDimension();
    if (dim == 1)
    {
        SNPRINTF(msg, 1024, "The original extents are (%g, %g)", 
                extents[0], extents[1]);
    }
    else if (dim == 2)
    {
        SNPRINTF(msg, 1024, "The original extents are (%g, %g, %g, %g)", 
            extents[0], extents[1], extents[2], extents[3]);
    }
    else if (dim == 3)
    {
       SNPRINTF(msg, 1024, "The original extents are (%g, %g, %g, %g, %g, %g)", 
        extents[0], extents[1], extents[2], extents[3], extents[4], extents[5]);
    }
    doubleVector d;
    for (int i = 0 ; i < 2*dim ; i++)
        d.push_back(extents[i]);
    qA->SetResultsMessage(msg);
    qA->SetResultsValue(d);

    UpdateProgress(1, 0);
}

