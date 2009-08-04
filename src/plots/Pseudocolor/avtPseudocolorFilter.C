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
//                           avtPseudocolorFilter.C                          //
// ************************************************************************* //

#include <avtPseudocolorFilter.h>


// ****************************************************************************
//  Method: avtPseudocolorFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtPseudocolorFilter::avtPseudocolorFilter()
{
    keepNodeZone = false;
}


// ****************************************************************************
//  Method: avtPseudocolorFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtPseudocolorFilter::~avtPseudocolorFilter()
{
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::SetPlotAtts
//
//  Purpose:    Sets the PseudcolorAttributes needed for this filter.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 10, 2040 
//
// ****************************************************************************

void
avtPseudocolorFilter::SetPlotAtts(const PseudocolorAttributes *atts)
{
    plotAtts = *atts;
}




// ****************************************************************************
//  Method: avtPseudocolorFilter::ExecuteData
//
//  Purpose:
//      Returns input. 
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtPseudocolorFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    return inDS;
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::UpdateDataObjectInfo
//
//  Purpose:  Sets flags in the pipeline.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPseudocolorFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::ModifyContract
//
//  Purpose:  Turns on Node/Zone numbers when appropriate. 
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   October 29, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 10 13:37:09 PDT 2005
//    Verify the existence of a valid variable before attempting to retrieve
//    its centering.
//
//    Kathleen Bonnell, Tue Jul 14 13:42:37 PDT 2009
//    Added test for MayRequireNodes for turning Node numbers on.
//
// ****************************************************************************

avtContract_p
avtPseudocolorFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = contract;

    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    string pointVar = plotAtts.GetPointSizeVar();
    avtDataRequest_p dataRequest = new avtDataRequest(
                                       contract->GetDataRequest());

    //
    // Find out if we need to add a secondary variable.
    //
    if (plotAtts.GetPointSizeVarEnabled() && 
        pointVar != "default" &&
        pointVar != "\0" &&
        pointVar != dataRequest->GetVariable() &&
        !dataRequest->HasSecondaryVariable(pointVar.c_str()))
    {
        rv->GetDataRequest()->AddSecondaryVariable(pointVar.c_str());
    }


    if (contract->GetDataRequest()->MayRequireZones() ||
        contract->GetDataRequest()->MayRequireNodes())
    {
        keepNodeZone = true;
        if (data.ValidActiveVariable())
        {
            if (data.GetCentering() == AVT_NODECENT)
            {
                rv->GetDataRequest()->TurnNodeNumbersOn();
            }
            else if (data.GetCentering() == AVT_ZONECENT)
            {
                rv->GetDataRequest()->TurnZoneNumbersOn();
            }
        }
        else 
        {
            // canot determine variable centering, so turn on both
            // node numbers and zone numbers.
            rv->GetDataRequest()->TurnNodeNumbersOn();
            rv->GetDataRequest()->TurnZoneNumbersOn();
        }
    }
    else
    {
        keepNodeZone = false;
    }

    return rv;
}
