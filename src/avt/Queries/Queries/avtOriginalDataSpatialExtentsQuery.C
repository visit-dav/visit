/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                    avtOriginalDataSpatialExtentsQuery.C                   //
// ************************************************************************* //

#include <avtOriginalDataSpatialExtentsQuery.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>
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
    avtDataRequest_p dataRequest = inData->GetOriginatingSource()->
        GetGeneralContract()->GetDataRequest();

    string dbVar = ParsingExprList::GetRealVariable(
                       queryAtts.GetVariables()[0]);
    avtDataRequest_p new_dataRequest = new avtDataRequest(dataRequest,
                                                                dbVar.c_str());

    avtContract_p contract = 
        new avtContract(new_dataRequest, queryAtts.GetPipeIndex()); 

    avtDataObject_p retObj;
    CopyTo(retObj, inData);
    retObj->Update(contract);
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
//    Hank Childs, Fri Jun  9 14:43:27 PDT 2006
//    Remove unused variable.
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string 
//
// ****************************************************************************

void
avtOriginalDataSpatialExtentsQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    string floatFormat = queryAtts.GetFloatFormat();
    string format ="";
    UpdateProgress(0, 0);

    avtDataObject_p dob = ApplyFilters(GetInput());

    SetTypedInput(dob);

    avtDataset_p input = GetTypedInput();
    double extents[6] = {0., 0., 0., 0., 0., 0.};
    char msg[1024];

    avtDatasetExaminer::GetSpatialExtents(input, extents);
    UnifyMinMax(extents, 6);
    int dim = input->GetInfo().GetAttributes().GetSpatialDimension();
    if (dim == 1)
    {
        format = "The original extents are (" + floatFormat + ", " 
                                              + floatFormat + ")";
        SNPRINTF(msg, 1024,format.c_str(), 
                extents[0], extents[1]);
    }
    else if (dim == 2)
    {
        format = "The original extents are (" + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ")";
        SNPRINTF(msg, 1024, format.c_str(), 
            extents[0], extents[1], extents[2], extents[3]);
    }
    else if (dim == 3)
    {
        format = "The original extents are (" + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ", " 
                                              + floatFormat + ")";
       SNPRINTF(msg, 1024, format.c_str(),
        extents[0], extents[1], extents[2], extents[3], extents[4], extents[5]);
    }
    doubleVector d;
    for (int i = 0 ; i < 2*dim ; i++)
        d.push_back(extents[i]);
    qA->SetResultsMessage(msg);
    qA->SetResultsValue(d);

    UpdateProgress(1, 0);
}

