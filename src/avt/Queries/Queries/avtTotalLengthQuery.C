// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtTotalLengthQuery.C                              //
// ************************************************************************* //

#include <avtTotalLengthQuery.h>

#include <avtEdgeLength.h>
#include <avtSourceFromAVTDataset.h>
#include <avtWarpFilter.h>

#include <InvalidDimensionsException.h>


using     std::string;

// ****************************************************************************
//  Method: avtTotalLengthQuery constructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalLengthFilter,
//          now deprecated.
//
//  Programmer: Hank Childs 
//  Creation:   November 3, 2006
//
// ****************************************************************************

avtTotalLengthQuery::avtTotalLengthQuery() : avtSummationQuery()
{
    string      varname = "length";
    string      sum_type = "Length";    

    length_expr      = new avtEdgeLength;
    length_expr->SetOutputVariableName(varname.c_str());

    SetVariableName(varname);
    SetSumType(sum_type);
    SumGhostValues(false);

    // avtEdgeLength will return -1 for cells that are not lines.
    // So turn off the summing of negative values.
    SumOnlyPositiveValues(true);
}


// ****************************************************************************
//  Method: avtTotalLengthQuery destructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalLengthFilter,
//          now deprecated.
//
//  Programmer: Hank Childs 
//  Creation:   November 3, 2006 
//
//  Modifications:
//
// ****************************************************************************

avtTotalLengthQuery::~avtTotalLengthQuery()
{
    if (length_expr != NULL)
    {
        delete length_expr;
        length_expr = NULL;
    }
}


// ****************************************************************************
//  Method: avtTotalLengthQuery::VerifyInput 
//
//  Purpose:  Throw an exception for invalid input. 
//
//  Programmer: Hank Childs 
//  Creation:   November 3, 2006
//
// ****************************************************************************

void 
avtTotalLengthQuery::VerifyInput()
{
    //
    //  Since base class performs general input verification for all queries,
    //  make sure to call it's verify method first.
    //
    avtDataObjectQuery::VerifyInput();

    if  (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1)
    {
        EXCEPTION2(InvalidDimensionsException, "Length query", 
                   "1D lines");
    }
    SetUnits(GetInput()->GetInfo().GetAttributes().GetXUnits());
}



// ****************************************************************************
//  Method: avtTotalLengthQuery::ApplyFilters 
//
//  Purpose:  Apply any avt filters necessary for completion of this query.
//
//  Programmer: Hank Childs 
//  Creation:   November 3, 2006 
//
// ****************************************************************************

avtDataObject_p 
avtTotalLengthQuery::ApplyFilters(avtDataObject_p inData)
{
    avtContract_p contract = 
        inData->GetOriginatingSource()->GetGeneralContract();

    if (timeVarying) 
    { 
        avtDataRequest_p oldSpec = inData->GetOriginatingSource()->
            GetGeneralContract()->GetDataRequest();

        avtDataRequest_p newDS = new 
            avtDataRequest(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        contract = new avtContract(newDS, contract->GetPipelineIndex());
    }

    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    //
    // If we have a function/curve, then elevate it away from a 1D rect
    // grid.
    //
    avtWarpFilter warper;
    if (inData->GetInfo().GetAttributes().GetSpatialDimension() == 1)
    {
        warper.SetInput(dob);
        dob = warper.GetOutput();
    }

    length_expr->SetInput(dob);
    avtDataObject_p objOut = length_expr->GetOutput();
    objOut->Update(contract);

    return objOut;
}


