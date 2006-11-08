/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

    if (timeVarying) 
    { 
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        pspec = new avtPipelineSpecification(newDS, pspec->GetPipelineIndex());
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
    objOut->Update(pspec);

    return objOut;
}


